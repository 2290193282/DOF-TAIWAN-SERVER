#include "old_data/db/auth_security/db_security_card_retransfer.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <stdint.h>
#include <string.h>
#include <time.h>



namespace {

enum {
    kDbSignalSecurityCardRetransfer = 312,
    kDbSecurityCardHandle = 6,
    kDbMemberHandle = 1,
    kErrorGeneral = 1,
    kErrorTooManyReissue = 120,
    kErrorPhoneMismatch = 125,
};

#pragma pack(push, 1)
struct SIG_SECURITY_CARD_RETRANSFER_LOCAL {
    uint32_t account_id;       // +0x00
    char server_key[33];       // +0x04
    char cert_key[6];          // +0x25
    char card[39];             // +0x2b
    char phone[12];            // +0x52
    char current_phone_hash[41]; // +0x5e
    uint8_t reserved_0x87;     // +0x87
    uint32_t error;            // +0x88
};
#pragma pack(pop)

typedef char SIG_SECURITY_CARD_RETRANSFER_LOCAL_Size_Must_Be_140[
    (sizeof(SIG_SECURITY_CARD_RETRANSFER_LOCAL) == 140) ? 1 : -1];

static void send_error(int user_idx, const SIG_SECURITY_CARD_RETRANSFER_LOCAL& payload, int error) {
    DB_SecurityCardRetransfer::_sendResult(user_idx,
                                           const_cast<char*>(payload.phone),
                                           payload.server_key,
                                           payload.card,
                                           static_cast<ENUM_ERROR>(error));
}

}  // namespace

// IDA_EA: 0x844E96A
// IDA_NAME: _ZN25DB_SecurityCardRetransferC2Ev
// DEMANGLED: DB_SecurityCardRetransfer::DB_SecurityCardRetransfer(void)
DB_SecurityCardRetransfer::DB_SecurityCardRetransfer() {}

// IDA_EA: 0x845E28A
// IDA_NAME: _ZN25DB_SecurityCardRetransferD2Ev
// DEMANGLED: DB_SecurityCardRetransfer::~DB_SecurityCardRetransfer()
// IDA_EA: 0x845E2BA
// IDA_NAME: _ZN25DB_SecurityCardRetransferD0Ev
// DEMANGLED: DB_SecurityCardRetransfer::~DB_SecurityCardRetransfer() deleting destructor
DB_SecurityCardRetransfer::~DB_SecurityCardRetransfer() {}

// IDA_EA: 0x842C362
// IDA_NAME: _ZN25DB_SecurityCardRetransfer8dispatchEiiP6Stream
// DEMANGLED: DB_SecurityCardRetransfer::dispatch(int,int,Stream *)
bool DB_SecurityCardRetransfer::dispatch(int signal,
                                         int user_idx,
                                         Stream* stream) {
    if (!DBDispatcher::dispatch(signal, user_idx, stream)) {
        return false;
    }

    SIG_SECURITY_CARD_RETRANSFER_LOCAL payload;
    memset(&payload, 0, sizeof(payload));
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kDbSecurityCardHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect re_issue_cnt,unix_timestamp(last_issue_time), phone from member_security_card where m_id=%s",
        NumberToString(payload.account_id, 0));
    if (db_handle->exec(true) != true || db_handle->fetch() != true) {
        send_error(user_idx, payload, kErrorGeneral);
        return false;
    }

    unsigned int reissue_count = 0;
    unsigned int last_issue_time = 0;
    char stored_phone[41];
    memset(stored_phone, 0, sizeof(stored_phone));
    db_handle->get_uint(0, reissue_count);
    db_handle->get_uint(1, last_issue_time);
    db_handle->get_str(2, stored_phone, sizeof(stored_phone));

    if (strncmp(payload.current_phone_hash, stored_phone, sizeof(stored_phone))) {
        send_error(user_idx, payload, kErrorPhoneMismatch);
        return false;
    }

    time_t current_time = GlobalData::system_time()
                              ? GlobalData::system_time()->getCurSec()
                              : time(0);
    time_t last_time = static_cast<time_t>(last_issue_time);
    struct tm last_tm;
    struct tm cur_tm;
    localtime_r(&last_time, &last_tm);
    localtime_r(&current_time, &cur_tm);
    if (last_tm.tm_yday == cur_tm.tm_yday) {
        if (++reissue_count > 3) {
            send_error(user_idx, payload, kErrorTooManyReissue);
            return false;
        }
    } else {
        reissue_count = 0;
    }

    db_handle->set_query(
        "upDate member_security_card set cert_key='%s',server_key='%s',card='%s',fail_cnt=0,cancel_cnt=0,re_issue_cnt=%d,last_issue_time=now(),validity_time=0,apply_flag=1,cert_flag=0,cert_cnt=0 where m_id=%s",
        payload.cert_key,
        payload.server_key,
        payload.card,
        reissue_count,
        NumberToString(payload.account_id, 0));
    if (db_handle->exec(true) != true) {
        send_error(user_idx, payload, kErrorGeneral);
        return false;
    }

    _sendResult(user_idx,
                payload.phone,
                payload.server_key,
                payload.card,
                static_cast<ENUM_ERROR>(0));

    MySQL* member_db = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kDbMemberHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    member_db->set_query(
        "upDate member_security_grade set security_card_reg=now() where m_id=%s",
        NumberToString(payload.account_id, 0));
    bool need_insert = true;
    if (member_db->exec(true) == true && member_db->getAffectedRowCount()) {
        need_insert = false;
    }
    if (need_insert) {
        member_db->set_query(
            "inSert into member_security_grade(m_id, security_card_reg) values(%s, now())",
            NumberToString(payload.account_id, 0));
        if (member_db->exec(true) != true) {
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x842C7BE
// IDA_NAME: _ZN25DB_SecurityCardRetransfer11_sendResultEiPcPKcS2_10ENUM_ERROR
// DEMANGLED: DB_SecurityCardRetransfer::_sendResult(int,char *,char const*,char const*,ENUM_ERROR)
void DB_SecurityCardRetransfer::_sendResult(int user_idx,
                                            char* phone,
                                            const char* server_key,
                                            const char* card,
                                            ENUM_ERROR error) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 28626);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalSecurityCardRetransfer;
    *guard << user_idx;

    SIG_SECURITY_CARD_RETRANSFER_LOCAL* payload =
        static_cast<SIG_SECURITY_CARD_RETRANSFER_LOCAL*>(
            guard->GetInBufferUntyped(sizeof(SIG_SECURITY_CARD_RETRANSFER_LOCAL)));
    memset(payload, 0, sizeof(*payload));
    payload->error = error;
    if (server_key) {
        strncpy(payload->server_key, server_key, 0x21u);
    }
    if (card) {
        strncpy(payload->card, card, 0x27u);
    }
    if (phone) {
        strncpy(payload->phone, phone, 0xCu);
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
}

// IDA_EA: 0x842C1F6
// IDA_NAME: _ZN25DB_SecurityCardRetransfer11makeRequestEiPcS0_jPKcS2_S2_
// DEMANGLED: DB_SecurityCardRetransfer::makeRequest(int,char *,char *,unsigned int,char const*,char const*,char const*)
void DB_SecurityCardRetransfer::makeRequest(int user_idx,
                                            const char* phone,
                                            const char* current_phone_hash,
                                            unsigned int account_id,
                                            const char* cert_key,
                                            const char* server_key,
                                            const char* card) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 28472);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalSecurityCardRetransfer;
    *guard << user_idx;

    SIG_SECURITY_CARD_RETRANSFER_LOCAL* payload =
        static_cast<SIG_SECURITY_CARD_RETRANSFER_LOCAL*>(
            guard->GetInBufferUntyped(sizeof(SIG_SECURITY_CARD_RETRANSFER_LOCAL)));
    memset(payload, 0, sizeof(*payload));
    payload->account_id = account_id;
    if (cert_key) {
        strncpy(payload->cert_key, cert_key, 6u);
    }
    if (server_key) {
        strncpy(payload->server_key, server_key, 0x21u);
    }
    if (card) {
        strncpy(payload->card, card, 0x27u);
    }
    if (phone) {
        strncpy(payload->phone, phone, 0xCu);
    }
    if (current_phone_hash) {
        strncpy(payload->current_phone_hash, current_phone_hash, 0x29u);
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
