#include "old_data/db/mail/db_multi_mail_dispatchers.h"

#include "common/manager/db_common_public_abi.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_data/db/core/mysql.h"
#include "old_data/db/mail/db_mailbox_req_send.h"
#include "old_data/db/mail/mailbox_payload.h"
#include "old_game/economy/mail/mailbox_system_multi_mail_payload.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <time.h>

namespace {

// IDA 0x843BC5D stores the full immediate 0x1f5 before the int stream
// write.  The old decompiler output retained only its low byte (245).
const int kInterSignalMultiMailBoxReqSend = 501;

int NextHourExpiry() {
    time_t timer =
        static_cast<time_t>(GlobalData::s_systemTime_.getCurSec());
    tm local_time;
    localtime_r(&timer, &local_time);
    ++local_time.tm_hour;
    local_time.tm_min = 0;
    local_time.tm_sec = 0;
    return static_cast<int>(mktime(&local_time));
}

}  // namespace

// IDA_EA: 0x844F492 / 0x845C460
DB_Multi_MailBox_Req_Send::DB_Multi_MailBox_Req_Send() {}
DB_Multi_MailBox_Req_Send::~DB_Multi_MailBox_Req_Send() {}

// IDA_EA: 0x843BADA
// IDA_NAME: _ZN25DB_Multi_MailBox_Req_Send8dispatchEiiP6Stream
// DEMANGLED: DB_Multi_MailBox_Req_Send::dispatch(int,int,Stream *)
bool DB_Multi_MailBox_Req_Send::dispatch(int signal,
                                         int user_idx,
                                         Stream* stream) {
    (void)signal;
    SIG_MAILBOX_CHECK_RECEIVER* const payload =
        stream->GetOutBuffer<SIG_MAILBOX_CHECK_RECEIVER>();
    const int expiry_time = NextHourExpiry();
    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(3), static_cast<ENUM_SERVER_GROUP>(0));

    // 0x843BADA consumes the active MultiMail request's named contents field.
    // Its enclosing SIG offset is +0xDB; the identity reference remains the
    // separately named +0x3FBD record word below.
    const char* const letter_text =
        payload->message.payload.multi_mailbox_send.contents;
    if (DB_MailBox_Req_Send::InsertLetter(
            db_handle,
            payload->receiver_charac_no,
            payload->sender_charac_no,
            payload->sender_charac_name,
            letter_text,
            payload->multi_mail_text_length,
            expiry_time,
            static_cast<ENUM_LETTER_STAT>(0)) != true) {
        // IDA 0x843BB66 logs the persistence failure, but 0x843BB8E still
        // sends the original check-receiver record back to game dispatch.
        // The reply releases the per-package send transaction.
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "virtual bool DB_Multi_MailBox_Req_Send::dispatch(int, int, Stream*)",
            0x8E53,
            "InsertLetter() error");
    }
    _sendResult(user_idx, payload);
    return true;
}

// IDA_EA: 0x843BC16
// IDA_NAME: _ZN25DB_Multi_MailBox_Req_Send11_sendResultEiP26SIG_MAILBOX_CHECK_RECEIVER
// DEMANGLED: DB_Multi_MailBox_Req_Send::_sendResult(int,SIG_MAILBOX_CHECK_RECEIVER *)
bool DB_Multi_MailBox_Req_Send::_sendResult(
    int user_idx,
    SIG_MAILBOX_CHECK_RECEIVER* payload) {
    Stream* const out =
        GlobalData::big_stream_pool()->Acquire("DBThread.cpp", 36455);
    CStreamGuard guard(out, true);
    guard << kInterSignalMultiMailBoxReqSend;
    cMyTrace trace(
        "bool DB_Multi_MailBox_Req_Send::_sendResult(int, SIG_MAILBOX_CHECK_RECEIVER*)",
        36458,
        0);
    trace("E_MULTI_MAILBOX_REQ_SEND : %d", kInterSignalMultiMailBoxReqSend);
    guard << user_idx;
    SIG_MAILBOX_CHECK_RECEIVER* const result =
        guard.GetInBuffer<SIG_MAILBOX_CHECK_RECEIVER>();
    std::memcpy(result, payload, sizeof(*result));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x844F99A / 0x845B830
DB_MailBox_Req_System_Multi_Mail::DB_MailBox_Req_System_Multi_Mail() {}
DB_MailBox_Req_System_Multi_Mail::~DB_MailBox_Req_System_Multi_Mail() {}

// IDA_EA: 0x844346A
// IDA_NAME: _ZN32DB_MailBox_Req_System_Multi_Mail8dispatchEiiP6Stream
// DEMANGLED: DB_MailBox_Req_System_Multi_Mail::dispatch(int,int,Stream *)
bool DB_MailBox_Req_System_Multi_Mail::dispatch(int signal,
                                                int user_idx,
                                                Stream* stream) {
    (void)signal;
    (void)user_idx;
    SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL* const payload =
        stream->GetOutBuffer<SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL>();
    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(3),
        static_cast<ENUM_SERVER_GROUP>(payload->server_group));

    int expiry_time = NextHourExpiry();
    // The old two-level branch intentionally leaves exactly 15 days at the
    // next-hour default; preserve it instead of simplifying the boundary.
    if (payload->occurrence_days > 14u) {
        if (payload->occurrence_days > 15u) {
            expiry_time = static_cast<int>(86400u * payload->occurrence_days) +
                          expiry_time - 1296000;
        }
    } else {
        expiry_time -= static_cast<int>(86400u * payload->occurrence_days);
    }
    payload->expiry_time = expiry_time;

    if (_InsertLetter(db_handle, payload) != true) {
        return false;
    }
    if (_InsertPostal(db_handle, payload) != true) {
        return false;
    }
    sendResult(payload->charac_no,
               static_cast<ENUM_SERVER_GROUP>(payload->server_group));
    return true;
}

// IDA_EA: 0x84435D4
// IDA_NAME: _ZN32DB_MailBox_Req_System_Multi_Mail13_InsertLetterEP5MySQLP34SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL
// DEMANGLED: DB_MailBox_Req_System_Multi_Mail::_InsertLetter(MySQL *,SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL *)
bool DB_MailBox_Req_System_Multi_Mail::_InsertLetter(
    MySQL* db_handle,
    SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL* payload) {
    char escaped_text[508];
    char escaped_title[61];
    std::memset(escaped_text, 0, sizeof(escaped_text));
    std::memset(escaped_title, 0, sizeof(escaped_title));
    db_handle->escape_string(escaped_text, payload->letter_text);
    db_handle->escape_string(escaped_title, payload->title);
    db_handle->set_query(
        "inSert into letter(charac_no,send_charac_no,send_charac_name,"
        "letter_text,reg_date,stat) values(%d,%d,'%s','%s',from_"
        "unixtime(%d),%d)",
        payload->charac_no,
        payload->sender_charac_no,
        escaped_title,
        escaped_text,
        payload->expiry_time,
        payload->letter_stat);
    if (db_handle->exec(true) != true) {
        return false;
    }
    payload->letter_id = static_cast<int32_t>(
        WongWork::DBCommon::GetIdentity(db_handle));
    return true;
}

// IDA_EA: 0x844372E
// IDA_NAME: _ZN32DB_MailBox_Req_System_Multi_Mail13_InsertPostalEP5MySQLP34SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL
// DEMANGLED: DB_MailBox_Req_System_Multi_Mail::_InsertPostal(MySQL *,SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL *)
bool DB_MailBox_Req_System_Multi_Mail::_InsertPostal(
    MySQL* db_handle,
    SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL* payload) {
    std::string values;
    values.clear();
    for (unsigned int index = 0; payload->item_count > index; ++index) {
        if (index) {
            // 0x8443796 changes the one-time gold sender field in place.
            payload->sender = 0;
        }

        const Inven_Item& item = payload->items[index];
        uint8_t amplify_option = 0;
        uint16_t amplify_value = 0;
        item.amplify_option_.getValues(amplify_option, amplify_value);
        const int separate_upgrade =
            item.upgrade_separate_.GetUpgradeSeparate();
        char* const random_option = db_handle->blob_to_str(
            1,
            const_cast<RandomOption*>(
                &item.chronicle_random_option_.random_option_),
            sizeof(item.chronicle_random_option_.random_option_));
        char* const item_guid = db_handle->blob_to_str(
            0,
            const_cast<itemGloballyUniqueIdentifier_t*>(
                &item.globally_unique_id_),
            sizeof(item.globally_unique_id_));

        char row[1024];
        std::memset(row, 0, sizeof(row));
        std::sprintf(
            row,
            "(from_unixtime(%d),%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,'%s',%d,"
            "%d,%d,%d,'%s','%s',%u,%d,%d)",
            payload->expiry_time,
            payload->sender_charac_no,
            payload->charac_no,
            static_cast<int>(item.seal_or_state_),
            item.item_id_,
            item.add_info_,
            static_cast<int>(item.avatar_select_),
            item.GetItemAttr(),
            static_cast<int>(amplify_option),
            static_cast<int>(amplify_value),
            static_cast<int>(payload->sender),
            payload->title,
            item.IsCreatureItemType() ? 1 : 0,
            item.isAvatarItemType() ? 1 : 0,
            payload->letter_id,
            item.extend_info_,
            item_guid,
            random_option,
            item.chronicle_random_option_.value_,
            separate_upgrade,
            static_cast<int>(payload->unlimit_flag));
        values.append(row);
        if (payload->item_count - 1u > index) {
            values.append(",");
        }
    }

    db_handle->set_query(
        "inSert into postal(occ_time,send_charac_no,receive_charac_no,"
        "seal_flag,item_id,add_info,endurance,upgrade ,amplify_option,"
        "amplify_value,gold,send_charac_name,creature_flag,avata_flag,"
        "letter_id,extend_info,item_guid,random_option,chronicle_value,"
        "seperate_upgrade,unlimit_flag) values %s",
        values.c_str());
    if (db_handle->exec(true) == true) {
        return true;
    }

    cMyTrace trace(
        "bool DB_MailBox_Req_System_Multi_Mail::_InsertPostal(MySQL*, SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL*)",
        42421,
        5);
    trace("Insert Postal Fail - inQuery(%s)", values.c_str());
    return false;
}

// IDA_EA: 0x8443B6E
// IDA_NAME: _ZN32DB_MailBox_Req_System_Multi_Mail10sendResultEi17ENUM_SERVER_GROUP
// DEMANGLED: DB_MailBox_Req_System_Multi_Mail::sendResult(int,ENUM_SERVER_GROUP)
bool DB_MailBox_Req_System_Multi_Mail::sendResult(
    int charac_no,
    ENUM_SERVER_GROUP server_group) {
    Stream* const out =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 42430);
    CStreamGuard guard(out, true);
    guard << 128;
    guard << -1;
    SIG_MAILBOX_SEND_NEW_MAIL* const result =
        guard.GetInBuffer<SIG_MAILBOX_SEND_NEW_MAIL>();
    result->result.result = 0;
    result->result.receiver_charac_no = charac_no;
    result->result.server_group = static_cast<int32_t>(server_group);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}
