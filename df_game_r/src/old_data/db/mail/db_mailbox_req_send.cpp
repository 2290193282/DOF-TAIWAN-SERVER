#include "old_data/db/mail/db_mailbox_req_send.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/security/policy/checksum.h"
#include "common/manager/system_time.h"

#include <stdint.h>
#include <string.h>
#include <time.h>


extern int GetIdentityFromDB(MySQL* mysql)
    __asm__("_Z17GetIdentityFromDBP5MySQL");

namespace {

enum {
    kDbSignalMailboxSend = 128,
    kLetterStatNormal = 1,
};

unsigned int g_last_letter_id = 0;

bool IsTrackedContextValid(
    const SIG_MAILBOX_PERSISTENCE_CONTEXT& context,
    const SIG_MAILBOX_SEND_NEW_MAIL::Request& request) {
    const bool known_consumer =
        context.consumer ==
            MAILBOX_PERSISTENCE_CONSUMER_MAGIC_LAMP ||
        context.consumer ==
            MAILBOX_PERSISTENCE_CONSUMER_FACEBOOK_LIKE ||
        context.consumer ==
            MAILBOX_PERSISTENCE_CONSUMER_DECIDE_LEVELUP;
    return context.magic == MAILBOX_PERSISTENCE_CONTEXT_MAGIC &&
           context.version == MAILBOX_PERSISTENCE_CONTEXT_VERSION &&
           known_consumer &&
           MailboxPersistenceCorrelationValid(context) &&
           context.receiver_charac_no != 0u &&
           context.receiver_charac_no ==
               static_cast<uint32_t>(request.receiver_charac_no) &&
           context.reward_item_id != 0u &&
           context.reward_item_id == request.item_id &&
           context.server_group == request.server_group;
}

bool SendResultInternal(
    int result,
    int receive_charac_no,
    ENUM_SERVER_GROUP server_group,
    const SIG_MAILBOX_PERSISTENCE_CONTEXT* persistence) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!pool || !queues) {
        return false;
    }
    Stream* const stream = pool->Acquire("DBThread.cpp", 21919);
    if (!stream) {
        return false;
    }
    CStreamGuard guard(stream, true);
    *guard << kDbSignalMailboxSend;
    // Signal 128's original completion is intentionally monitor-side and is
    // therefore not tied to a live CUser.  The tracked tail supplies the
    // durable-operation correlation instead.
    *guard << -1;

    SIG_MAILBOX_SEND_NEW_MAIL* payload = 0;
    if (persistence) {
        SIG_MAILBOX_SEND_NEW_MAIL_TRACKED* const tracked =
            reinterpret_cast<SIG_MAILBOX_SEND_NEW_MAIL_TRACKED*>(
                guard.GetInBufferUntyped(static_cast<int>(
                    sizeof(SIG_MAILBOX_SEND_NEW_MAIL_TRACKED))));
        if (!tracked) {
            return false;
        }
        memset(tracked, 0, sizeof(*tracked));
        tracked->context = *persistence;
        payload = &tracked->mail;
    } else {
        payload = guard.GetInBuffer<SIG_MAILBOX_SEND_NEW_MAIL>();
        if (!payload) {
            return false;
        }
        memset(payload, 0, sizeof(*payload));
    }
    payload->result.result = result;
    payload->result.receiver_charac_no = receive_charac_no;
    payload->result.server_group = server_group;
    return queues->put(MsgQueueMgr::DISPATCH_Q, guard) == 1;
}

size_t BoundedStringLength(const char* text, size_t capacity) {
    if (!text) return capacity;
    size_t length = 0u;
    while (length < capacity && text[length] != '\0') ++length;
    return length;
}

bool ComputeTrackedMailPayloadSha256(
    const SIG_MAILBOX_SEND_NEW_MAIL::Request& request,
    uint8_t digest[32]) {
    if (!digest) return false;
    CNChecksumSha256 checksum;
    return checksum.MakeChecksumFromMemory(
               reinterpret_cast<const unsigned char*>(&request),
               static_cast<int>(sizeof(request)), digest, 32) ==
           CHECKSUM_RESULT_OK;
}

int ComputeMailExpiration(const SIG_MAILBOX_SEND_NEW_MAIL::Request& request) {
    time_t timer = GlobalData::system_time()
                       ? static_cast<time_t>(
                             GlobalData::system_time()->getCurSec())
                       : time(0);
    struct tm local_time;
    localtime_r(&timer, &local_time);
    ++local_time.tm_hour;
    local_time.tm_min = 0;
    local_time.tm_sec = 0;
    int expire_time = static_cast<int>(mktime(&local_time));

    if (request.db_work) {
        expire_time = request.occ_time - 1296000;
    } else if (static_cast<uint32_t>(request.occ_time) > 14u) {
        if (static_cast<uint32_t>(request.occ_time) > 15u) {
            expire_time += 86400 * request.occ_time - 1296000;
        }
    } else {
        expire_time -= 86400 * request.occ_time;
    }
    return expire_time;
}

bool IsCanonicalDecideMailRequest(
    const SIG_MAILBOX_SEND_NEW_MAIL::Request& request) {
    const size_t sender_length = BoundedStringLength(
        request.sender_charac_name, sizeof(request.sender_charac_name));
    const size_t text_length =
        BoundedStringLength(request.letter_text, sizeof(request.letter_text));
    return sender_length < sizeof(request.sender_charac_name) &&
           text_length < sizeof(request.letter_text) &&
           text_length == request.letter_text_length &&
           request.receiver_charac_no > 0 && request.item_id != 0u &&
           request.gold == 0 && request.old_letter_id == 0 &&
           request.db_work == 0;
}

bool FindExactDecideLetter(
    MySQL* database,
    const SIG_MAILBOX_SEND_NEW_MAIL::Request& request,
    const uint8_t delivery_token[16],
    const uint8_t payload_sha256[32],
    bool& found,
    int& letter_id) {
    found = false;
    letter_id = 0;
    if (!database || !delivery_token || !payload_sha256) return false;
    if (request.letter_text_length == 0u) {
        found = true;
        return true;
    }

    char* const token = database->blob_to_str(
        0, const_cast<uint8_t*>(delivery_token), 16);
    if (!token ||
        !database->set_query(
            "seLect letter_id,native_delivery_payload_sha256_0828,"
            "send_charac_no,send_charac_name,letter_text from letter where "
            "charac_no=%u and native_delivery_token_0828='%s' limit 2",
            static_cast<unsigned int>(request.receiver_charac_no), token) ||
        !database->exec(true)) {
        return false;
    }
    if (database->get_n_rows() == 0) return true;
    if (database->get_n_rows() != 1 || !database->fetch()) return false;

    uint8_t stored_sha256[32];
    int stored_sender_charac_no = 0;
    char stored_sender_name[sizeof(request.sender_charac_name) + 1u];
    char stored_letter_text[sizeof(request.letter_text) + 1u];
    memset(stored_sender_name, 0, sizeof(stored_sender_name));
    memset(stored_letter_text, 0, sizeof(stored_letter_text));
    const size_t expected_sender_length = strlen(request.sender_charac_name);
    const size_t expected_text_length = request.letter_text_length;
    if (!database->get_int(0, letter_id) || letter_id <= 0 ||
        database->get_binary_length(1) != 32 ||
        !database->get_binary(1, stored_sha256,
                              static_cast<int>(sizeof(stored_sha256))) ||
        !database->get_int(2, stored_sender_charac_no) ||
        stored_sender_charac_no != request.sender_charac_no ||
        database->get_binary_length(3) !=
            static_cast<int>(expected_sender_length) ||
        !database->get_str(3, stored_sender_name,
                           static_cast<int>(sizeof(stored_sender_name))) ||
        memcmp(stored_sender_name, request.sender_charac_name,
               expected_sender_length + 1u) != 0 ||
        database->get_binary_length(4) !=
            static_cast<int>(expected_text_length) ||
        !database->get_str(4, stored_letter_text,
                           static_cast<int>(sizeof(stored_letter_text))) ||
        memcmp(stored_letter_text, request.letter_text,
               expected_text_length + 1u) != 0 ||
        memcmp(stored_sha256, payload_sha256, sizeof(stored_sha256)) != 0) {
        // Do not trust the feature-owned digest column by itself.  The exact
        // receiver (WHERE), sender and body must also be the post-image bound
        // by the fixed Request digest.  Letter stat/reg_date are deliberately
        // excluded because reading a delivered mail mutates stat and the
        // legacy consumer derives reg_date from its processing time.
        return false;
    }
    found = true;
    return true;
}

bool EnsureExactDecideLetter(
    MySQL* database,
    const SIG_MAILBOX_SEND_NEW_MAIL::Request& request,
    const uint8_t delivery_token[16],
    const uint8_t payload_sha256[32],
    int expire_time,
    int& letter_id) {
    bool found = false;
    if (!FindExactDecideLetter(database, request, delivery_token,
                               payload_sha256, found, letter_id)) {
        return false;
    }
    if (found) return true;

    char escaped_text[508];
    char escaped_name[62];
    memset(escaped_text, 0, sizeof(escaped_text));
    memset(escaped_name, 0, sizeof(escaped_name));
    database->escape_string(escaped_text, request.letter_text);
    database->escape_string(escaped_name, request.sender_charac_name);
    char* const token = database->blob_to_str(
        0, const_cast<uint8_t*>(delivery_token), 16);
    char* const payload_sha = database->blob_to_str(
        1, const_cast<uint8_t*>(payload_sha256), 32);
    if (!token || !payload_sha ||
        !database->set_query(
            "inSert into letter(charac_no,send_charac_no,send_charac_name,"
            "letter_text,reg_date,stat,native_delivery_token_0828,"
            "native_delivery_payload_sha256_0828) values("
            "%d,%d,'%s','%s',from_unixtime(%d),%d,'%s','%s')",
            request.receiver_charac_no, request.sender_charac_no,
            escaped_name, escaped_text, expire_time, kLetterStatNormal,
            token, payload_sha)) {
        return false;
    }
    // letter is MyISAM: a lost response or duplicate-key result is resolved
    // only by the authoritative token row, never by exec()/affected rows.
    (void)database->exec(true);
    found = false;
    letter_id = 0;
    return FindExactDecideLetter(database, request, delivery_token,
                                 payload_sha256, found, letter_id) &&
           found;
}

bool FindExactDecidePostal(
    MySQL* database,
    const SIG_MAILBOX_SEND_NEW_MAIL::Request& request,
    const uint8_t delivery_token[16],
    const uint8_t payload_sha256[32],
    bool& found) {
    found = false;
    if (!database || !delivery_token || !payload_sha256) return false;
    char* const token = database->blob_to_str(
        0, const_cast<uint8_t*>(delivery_token), 16);
    if (!token ||
        !database->set_query(
            "seLect item_id,letter_id,native_delivery_payload_sha256_0828,"
            "send_charac_no,seal_flag,add_info,endurance,upgrade,"
            "amplify_option,amplify_value,gold,send_charac_name,"
            "creature_flag,avata_flag,extend_info,item_guid,random_option,"
            "chronicle_value,seperate_upgrade,unlimit_flag from postal "
            "where receive_charac_no=%u and "
            "native_delivery_token_0828='%s' limit 2",
            static_cast<unsigned int>(request.receiver_charac_no), token) ||
        !database->exec(true)) {
        return false;
    }
    if (database->get_n_rows() == 0) return true;
    if (database->get_n_rows() != 1 || !database->fetch()) return false;

    unsigned int stored_item_id = 0u;
    int stored_letter_id = 0;
    uint8_t stored_sha256[32];
    int stored_sender_charac_no = 0;
    unsigned int stored_seal_flag = 0u;
    unsigned int stored_add_info = 0u;
    unsigned int stored_endurance = 0u;
    unsigned int stored_upgrade = 0u;
    unsigned int stored_amplify_option = 0u;
    unsigned int stored_amplify_value = 0u;
    int stored_gold = 0;
    char stored_sender_name[sizeof(request.sender_charac_name) + 1u];
    unsigned int stored_creature_flag = 0u;
    unsigned int stored_avatar_flag = 0u;
    unsigned int stored_extend_info = 0u;
    uint8_t stored_item_guid[sizeof(request.item_guid)];
    uint8_t stored_random_option[sizeof(request.random_option)];
    unsigned int stored_chronicle_value = 0u;
    unsigned int stored_separate_upgrade = 0u;
    unsigned int stored_unlimit_flag = 0u;
    memset(stored_sender_name, 0, sizeof(stored_sender_name));
    memset(stored_item_guid, 0, sizeof(stored_item_guid));
    memset(stored_random_option, 0, sizeof(stored_random_option));
    const size_t expected_sender_length = strlen(request.sender_charac_name);
    if (!database->get_uint(0, stored_item_id) ||
        stored_item_id != request.item_id ||
        !database->get_int(1, stored_letter_id) ||
        database->get_binary_length(2) != 32 ||
        !database->get_binary(2, stored_sha256,
                              static_cast<int>(sizeof(stored_sha256))) ||
        !database->get_int(3, stored_sender_charac_no) ||
        stored_sender_charac_no != request.sender_charac_no ||
        !database->get_uint(4, stored_seal_flag) ||
        stored_seal_flag != request.seal_flag ||
        !database->get_uint(5, stored_add_info) ||
        stored_add_info != request.add_info ||
        !database->get_uint(6, stored_endurance) ||
        stored_endurance != request.endurance ||
        !database->get_uint(7, stored_upgrade) ||
        stored_upgrade != request.upgrade ||
        !database->get_uint(8, stored_amplify_option) ||
        stored_amplify_option != request.amplify_option ||
        !database->get_uint(9, stored_amplify_value) ||
        stored_amplify_value != request.amplify_value ||
        !database->get_int(10, stored_gold) ||
        stored_gold != request.gold ||
        database->get_binary_length(11) !=
            static_cast<int>(expected_sender_length) ||
        !database->get_str(11, stored_sender_name,
                           static_cast<int>(sizeof(stored_sender_name))) ||
        memcmp(stored_sender_name, request.sender_charac_name,
               expected_sender_length + 1u) != 0 ||
        !database->get_uint(12, stored_creature_flag) ||
        stored_creature_flag != request.creature_flag ||
        !database->get_uint(13, stored_avatar_flag) ||
        stored_avatar_flag != request.avatar_flag ||
        !database->get_uint(14, stored_extend_info) ||
        stored_extend_info != request.extend_info ||
        database->get_binary_length(15) !=
            static_cast<int>(sizeof(stored_item_guid)) ||
        !database->get_binary(15, stored_item_guid,
                              static_cast<int>(sizeof(stored_item_guid))) ||
        memcmp(stored_item_guid, request.item_guid,
               sizeof(stored_item_guid)) != 0 ||
        database->get_binary_length(16) !=
            static_cast<int>(sizeof(stored_random_option)) ||
        !database->get_binary(
            16, stored_random_option,
            static_cast<int>(sizeof(stored_random_option))) ||
        memcmp(stored_random_option, request.random_option,
               sizeof(stored_random_option)) != 0 ||
        !database->get_uint(17, stored_chronicle_value) ||
        stored_chronicle_value != request.chronicle_value ||
        !database->get_uint(18, stored_separate_upgrade) ||
        stored_separate_upgrade != request.separate_upgrade ||
        !database->get_uint(19, stored_unlimit_flag) ||
        stored_unlimit_flag != request.unlimit_flag ||
        memcmp(stored_sha256, payload_sha256, sizeof(stored_sha256)) != 0) {
        // A token collision, altered replay, or corrupted physical post-image
        // is never acknowledged.  The fixed digest is supplemental evidence;
        // every immutable postal field projected from Request is re-read.
        // occ_time is excluded because the legacy consumer converts its
        // relative request value to an insertion-time expiration timestamp.
        return false;
    }
    bool exact_letter = false;
    int exact_letter_id = 0;
    if (!FindExactDecideLetter(database, request, delivery_token,
                               payload_sha256, exact_letter,
                               exact_letter_id) ||
        !exact_letter || exact_letter_id != stored_letter_id) {
        // A postal row is not complete until its foreign id names the exact
        // deterministic letter post-image (or both ids are zero).
        return false;
    }
    found = true;
    return true;
}

bool InsertTrackedDecideMailData(
    MySQL* database,
    const SIG_MAILBOX_SEND_NEW_MAIL* payload,
    const uint8_t delivery_token[16],
    const uint8_t payload_sha256[32]) {
    if (!database || !payload || !delivery_token || !payload_sha256) {
        return false;
    }
    const SIG_MAILBOX_SEND_NEW_MAIL::Request& request = payload->request;
    if (!IsCanonicalDecideMailRequest(request)) return false;

    const int expire_time = ComputeMailExpiration(request);
    int letter_id = 0;
    if (!EnsureExactDecideLetter(database, request, delivery_token,
                                 payload_sha256, expire_time, letter_id)) {
        return false;
    }

    char escaped_name[61];
    memset(escaped_name, 0, sizeof(escaped_name));
    database->escape_string(escaped_name, request.sender_charac_name);
    char* const item_guid = database->blob_to_str(
        0, const_cast<uint8_t*>(request.item_guid), sizeof(request.item_guid));
    char* const random_option = database->blob_to_str(
        1, const_cast<uint8_t*>(request.random_option),
        sizeof(request.random_option));
    char* const token = database->blob_to_str(
        2, const_cast<uint8_t*>(delivery_token), 16);
    char* const payload_sha = database->blob_to_str(
        3, const_cast<uint8_t*>(payload_sha256), 32);
    if (!item_guid || !random_option || !token || !payload_sha ||
        !database->set_query(
            "inSert into postal(occ_time,send_charac_no,"
            "receive_charac_no,seal_flag,item_id,add_info,endurance,upgrade,"
            "amplify_option,amplify_value,gold,send_charac_name,creature_flag,"
            "avata_flag,letter_id,extend_info,item_guid,random_option,"
            "chronicle_value,seperate_upgrade,unlimit_flag,"
            "native_delivery_token_0828,"
            "native_delivery_payload_sha256_0828) values("
            "from_unixtime(%d),%d,%d,%d,%u,%u,%u,%u,%u,%u,%d,'%s',"
            "%u,%u,%d,%u,'%s','%s',%u,%u,%u,'%s','%s')",
            expire_time, request.sender_charac_no,
            request.receiver_charac_no,
            static_cast<unsigned int>(request.seal_flag), request.item_id,
            request.add_info,
            static_cast<unsigned int>(request.endurance),
            static_cast<unsigned int>(request.upgrade),
            static_cast<unsigned int>(request.amplify_option),
            static_cast<unsigned int>(request.amplify_value), request.gold,
            escaped_name,
            static_cast<unsigned int>(request.creature_flag),
            static_cast<unsigned int>(request.avatar_flag), letter_id,
            request.extend_info, item_guid, random_option,
            request.chronicle_value,
            static_cast<unsigned int>(request.separate_upgrade),
            static_cast<unsigned int>(request.unlimit_flag), token,
            payload_sha)) {
        return false;
    }
    return database->exec(true);
}

bool RecordExactDecideMailJournal(
    MySQL* database,
    const SIG_MAILBOX_SEND_NEW_MAIL::Request& request,
    const uint8_t delivery_token[16],
    const uint8_t payload_sha256[32]) {
    if (!database || !delivery_token || !payload_sha256) return false;
    char* token = database->blob_to_str(
        0, const_cast<uint8_t*>(delivery_token), 16);
    char* payload_sha = database->blob_to_str(
        1, const_cast<uint8_t*>(payload_sha256), 32);
    if (!token || !payload_sha ||
        !database->set_query(
            "inSert ignore into decide_levelup_mail_delivery_0828("
            "receiver_charac_no,delivery_token,reward_item_id,"
            "payload_sha256,created_at) values(%u,'%s',%u,'%s',now())",
            static_cast<unsigned int>(request.receiver_charac_no), token,
            request.item_id, payload_sha)) {
        return false;
    }
    // An uncertain INSERT result is resolved by the authoritative row below;
    // neither exec acceptance nor affected-row count is a durable ACK.
    (void)database->exec(true);

    token = database->blob_to_str(
        0, const_cast<uint8_t*>(delivery_token), 16);
    if (!token ||
        !database->set_query(
            "seLect reward_item_id,payload_sha256 from "
            "decide_levelup_mail_delivery_0828 where "
            "receiver_charac_no=%u and delivery_token='%s' limit 2",
            static_cast<unsigned int>(request.receiver_charac_no), token) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch()) {
        return false;
    }
    unsigned int stored_item_id = 0u;
    uint8_t stored_sha256[32];
    return database->get_uint(0, stored_item_id) &&
           stored_item_id == request.item_id &&
           database->get_binary_length(1) == 32 &&
           database->get_binary(1, stored_sha256,
                                static_cast<int>(sizeof(stored_sha256))) &&
           memcmp(stored_sha256, payload_sha256, sizeof(stored_sha256)) == 0;
}

}  // namespace

// IDA_EA: 0x844E32E
// IDA_NAME: _ZN19DB_MailBox_Req_SendC2Ev
// DEMANGLED: DB_MailBox_Req_Send::DB_MailBox_Req_Send(void)
DB_MailBox_Req_Send::DB_MailBox_Req_Send() {}

// IDA_EA: 0x845F484
// IDA_NAME: _ZN19DB_MailBox_Req_SendD2Ev
// DEMANGLED: DB_MailBox_Req_Send::~DB_MailBox_Req_Send()
// IDA_EA: 0x845F4B4
// IDA_NAME: _ZN19DB_MailBox_Req_SendD0Ev
// DEMANGLED: DB_MailBox_Req_Send::~DB_MailBox_Req_Send() deleting destructor
DB_MailBox_Req_Send::~DB_MailBox_Req_Send() {}

// IDA_EA: 0x841F15A
// IDA_NAME: _ZN19DB_MailBox_Req_Send12InsertLetterEP5MySQLiiPKcS3_Ril16ENUM_LETTER_STAT
// DEMANGLED: DB_MailBox_Req_Send::InsertLetter(MySQL *,int,int,char const*,char const*,int &,long,ENUM_LETTER_STAT)
bool DB_MailBox_Req_Send::InsertLetter(MySQL* db_handle,
                                        int receive_charac_no,
                                        int send_charac_no,
                                        const char* send_charac_name,
                                        const char* letter_text,
                                        int& letter_id,
                                        long reg_time,
                                        ENUM_LETTER_STAT stat) {
    if (strlen(letter_text) > 0xAAu) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "static bool DB_MailBox_Req_Send::InsertLetter(MySQL*, int, int, const char*, const char*, int&, time_t, ENUM_LETTER_STAT)",
            0x54BC,
            "send_charac_no : %d, recv_charac_no : %d, letter_size : %d",
            send_charac_no,
            receive_charac_no,
            static_cast<int>(strlen(letter_text)));
        return false;
    }

    char escaped_text[508];
    char escaped_name[62];
    memset(escaped_text, 0, sizeof(escaped_text));
    memset(escaped_name, 0, sizeof(escaped_name));
    db_handle->escape_string(escaped_text, letter_text);
    db_handle->escape_string(escaped_name, send_charac_name);
    db_handle->set_query(
        "inSert into letter(charac_no,send_charac_no,send_charac_name,letter_text,reg_date,stat) values(%d,%d,'%s','%s',from_unixtime(%d),%d)",
        receive_charac_no,
        send_charac_no,
        escaped_name,
        escaped_text,
        static_cast<int>(reg_time),
        stat);
    if (db_handle->exec(true) != true) {
        return false;
    }
    letter_id = GetIdentityFromDB(db_handle);
    return true;
}

// IDA_EA: 0x841F2FA
// IDA_NAME: _ZN19DB_MailBox_Req_Send14insertMailDataEP5MySQLPK25SIG_MAILBOX_SEND_NEW_MAIL
// DEMANGLED: DB_MailBox_Req_Send::insertMailData(MySQL *,SIG_MAILBOX_SEND_NEW_MAIL const*)
bool DB_MailBox_Req_Send::insertMailData(MySQL* db_handle,
                                          const SIG_MAILBOX_SEND_NEW_MAIL* payload) {
    const SIG_MAILBOX_SEND_NEW_MAIL::Request& request = payload->request;
    time_t timer = GlobalData::system_time()
                       ? static_cast<time_t>(GlobalData::system_time()->getCurSec())
                       : time(0);
    struct tm local_time;
    localtime_r(&timer, &local_time);
    ++local_time.tm_hour;
    local_time.tm_min = 0;
    local_time.tm_sec = 0;
    int expire_time = static_cast<int>(mktime(&local_time));

    if (request.db_work) {
        expire_time = request.occ_time - 1296000;
    } else if (static_cast<uint32_t>(request.occ_time) > 14u) {
        if (static_cast<uint32_t>(request.occ_time) > 15u) {
            expire_time += 86400 * request.occ_time - 1296000;
        }
    } else {
        expire_time -= 86400 * request.occ_time;
    }

    int letter_id = request.old_letter_id;
    if (letter_id && letter_id != g_last_letter_id) {
        db_handle->set_query("upDate letter set stat =%d where letter_id = %d",
                             kLetterStatNormal,
                             letter_id);
        db_handle->exec(true);
    }
    g_last_letter_id = letter_id;

    if (request.letter_text_length && !letter_id) {
        if (InsertLetter(db_handle,
                         request.receiver_charac_no,
                         request.sender_charac_no,
                         request.sender_charac_name,
                         request.letter_text,
                         letter_id,
                         expire_time,
                         static_cast<ENUM_LETTER_STAT>(kLetterStatNormal)) !=
            true) {
            LogManager::logFormat(
                1,
                "DBThread.cpp",
                "static bool DB_MailBox_Req_Send::insertMailData(MySQL*, const SIG_MAILBOX_SEND_NEW_MAIL*)",
                0x550C,
                "InsertLetter() error");
        }
        if (!request.item_id && !request.gold) {
            return true;
        }
    }

    char send_charac_name[88];
    memset(send_charac_name, 0, sizeof(send_charac_name));
    strcpy(send_charac_name, request.sender_charac_name);
    char* item_guid =
        db_handle->blob_to_str(0,
                               const_cast<uint8_t*>(request.item_guid),
                               sizeof(request.item_guid));
    char* random_option =
        db_handle->blob_to_str(1,
                               const_cast<uint8_t*>(request.random_option),
                               sizeof(request.random_option));
    db_handle->set_query(
        "inSert into postal(occ_time,send_charac_no,receive_charac_no,seal_flag,item_id,add_info,endurance,upgrade,amplify_option,amplify_value,gold,send_charac_name,creature_flag,avata_flag,letter_id,extend_info,item_guid,random_option,chronicle_value,seperate_upgrade,unlimit_flag) values(from_unixtime(%d),%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,'%s',%d,%d,%d,%d,'%s','%s',%u,%d,%d)",
        expire_time,
        request.sender_charac_no,
        request.receiver_charac_no,
        request.seal_flag,
        request.item_id,
        request.add_info,
        request.endurance,
        request.upgrade,
        request.amplify_option,
        request.amplify_value,
        request.gold,
        send_charac_name,
        request.creature_flag,
        request.avatar_flag,
        letter_id,
        request.extend_info,
        item_guid,
        random_option,
        request.chronicle_value,
        request.separate_upgrade,
        request.unlimit_flag);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x841F75A
// IDA_NAME: _ZN19DB_MailBox_Req_Send10sendResultEiii17ENUM_SERVER_GROUP
// DEMANGLED: DB_MailBox_Req_Send::sendResult(int,int,int,ENUM_SERVER_GROUP)
bool DB_MailBox_Req_Send::sendResult(int user_idx,
                                      int result,
                                      int receive_charac_no,
                                      ENUM_SERVER_GROUP server_group) {
    (void)user_idx;
    // IDA_EVIDENCE: the legacy completion helper submits its response and
    // returns true unconditionally.  Preserve that contract; tracked CMD723
    // requests call SendResultInternal directly so they can observe whether
    // the correlated persistence ACK was actually queued.
    (void)SendResultInternal(result, receive_charac_no, server_group, 0);
    return true;
}

// IDA_EA: 0x841F84E
// IDA_NAME: _ZN19DB_MailBox_Req_Send8dispatchEiiP6Stream
// DEMANGLED: DB_MailBox_Req_Send::dispatch(int,int,Stream *)
bool DB_MailBox_Req_Send::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;

    if (!stream) {
        return false;
    }
    const int request_size = stream->length();
    const bool tracked_request =
        request_size ==
        static_cast<int>(sizeof(SIG_MAILBOX_SEND_NEW_MAIL_TRACKED));
    if (request_size != static_cast<int>(sizeof(SIG_MAILBOX_SEND_NEW_MAIL)) &&
        !tracked_request) {
        return false;
    }
    SIG_MAILBOX_SEND_NEW_MAIL* payload =
        stream->GetOutBuffer<SIG_MAILBOX_SEND_NEW_MAIL>();
    if (!payload) {
        return false;
    }

    SIG_MAILBOX_PERSISTENCE_CONTEXT persistence;
    const SIG_MAILBOX_PERSISTENCE_CONTEXT* persistence_ptr = 0;
    if (tracked_request) {
        memset(&persistence, 0, sizeof(persistence));
        if (!stream->get_binary(&persistence,
                                static_cast<int>(sizeof(persistence))) ||
            stream->length() != 0 ||
            !IsTrackedContextValid(persistence, payload->request)) {
            return false;
        }
        persistence_ptr = &persistence;
    }

    if (!GlobalData::s_db_mgr) {
        return false;
    }
    const ENUM_SERVER_GROUP server_group =
        static_cast<ENUM_SERVER_GROUP>(payload->request.server_group);
    // GetDBHandle deliberately falls back to DB_HANDLE_LOG when an active
    // global DB lacks a DB3 group mapping.  That legacy behavior is retained
    // for ordinary mail.  A tracked reward must fail closed instead: writing
    // a postal INSERT into the log database and acknowledging it would be a
    // false persistence success for the requested character shard.
    MySQL* const db_handle =
        tracked_request && GlobalData::s_db_mgr->isActiveGlobalDB()
            ? GlobalData::s_db_mgr->GetDBHandleServerGroup(DB_HANDLE_3,
                                                           server_group)
            : GlobalData::s_db_mgr->GetDBHandle(DB_HANDLE_3, server_group);
    if (!db_handle) {
        return false;
    }
    bool persisted = false;
    if (persistence_ptr &&
        persistence_ptr->consumer ==
            MAILBOX_PERSISTENCE_CONSUMER_DECIDE_LEVELUP) {
        uint8_t payload_sha256[32];
        bool exact_postal = false;
        const bool canonical =
            IsCanonicalDecideMailRequest(payload->request) &&
            ComputeTrackedMailPayloadSha256(payload->request,
                                            payload_sha256);
        if (canonical &&
            FindExactDecidePostal(
                db_handle, payload->request,
                persistence_ptr->delivery_token, payload_sha256,
                exact_postal)) {
            if (!exact_postal) {
                // postal is legacy MyISAM.  Its receiver+128-bit token unique
                // key is the delivery atom: after any INSERT outcome, re-read
                // the authoritative row and require the exact fixed payload
                // SHA before producing a tracked ACK.
                (void)InsertTrackedDecideMailData(
                    db_handle, payload, persistence_ptr->delivery_token,
                    payload_sha256);
                exact_postal = false;
                if (!FindExactDecidePostal(
                        db_handle, payload->request,
                        persistence_ptr->delivery_token, payload_sha256,
                        exact_postal)) {
                    exact_postal = false;
                }
            }
            if (exact_postal) {
                persisted = RecordExactDecideMailJournal(
                    db_handle, payload->request,
                    persistence_ptr->delivery_token, payload_sha256);
            }
        }
    } else {
        persisted = insertMailData(db_handle, payload) == true;
    }
    if (persistence_ptr) {
        // A tracked failure must also be returned: the consumer keeps its
        // reservation in state 0 and can retry after its lease.  Crucially,
        // this result is produced after exec(true), never on DB_Q acceptance.
        const bool result_queued = SendResultInternal(
            persisted ? 0 : -1,
            payload->request.receiver_charac_no,
            server_group,
            persistence_ptr);
        return persisted && result_queued;
    }
    if (!persisted) {
        return false;
    }
    return sendResult(
        user_idx,
        0,
        payload->request.receiver_charac_no,
        server_group);
}
