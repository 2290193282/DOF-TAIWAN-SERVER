#include "old_data/db/auth_security/db_security_card_issue.h"

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



extern unsigned int generateSecurityCardQuestion(char* out)
    __asm__("_Z28generateSecurityCardQuestionPc");
extern int turnOffSecurityFlag(MySQL* db, unsigned int account_id)
    __asm__("_Z19turnOffSecurityFlagP5MySQLj");

#pragma pack(push, 1)
struct SIG_SECURITY_CARD_ISSUE_LOCAL {
    uint32_t account_id;      // +0
    char server_key[33];      // +4
    char phone[12];           // +37
    char cert_key[6];         // +49
    char card[39];            // +55
    char q_pos[5];            // +94
    char phone_hash[41];      // +99
    int32_t error;            // +140
};
#pragma pack(pop)

typedef char SIG_SECURITY_CARD_ISSUE_LOCAL_Size_Must_Be_144[
    (sizeof(SIG_SECURITY_CARD_ISSUE_LOCAL) == 144) ? 1 : -1];

// IDA_EA: 0x844E8C2
// IDA_NAME: _ZN20DB_SecurityCardIssueC2Ev
// DEMANGLED: DB_SecurityCardIssue::DB_SecurityCardIssue(void)
DB_SecurityCardIssue::DB_SecurityCardIssue() {}

// IDA_EA: 0x845E45E
// IDA_NAME: _ZN20DB_SecurityCardIssueD2Ev
// DEMANGLED: DB_SecurityCardIssue::~DB_SecurityCardIssue()
// IDA_EA: 0x845E48E
// IDA_NAME: _ZN20DB_SecurityCardIssueD0Ev
// DEMANGLED: DB_SecurityCardIssue::~DB_SecurityCardIssue() deleting destructor
DB_SecurityCardIssue::~DB_SecurityCardIssue() {}

// IDA_EA: 0x842AF94
// IDA_NAME: _ZN20DB_SecurityCardIssue8dispatchEiiP6Stream
// DEMANGLED: DB_SecurityCardIssue::dispatch(int,int,Stream *)
bool DB_SecurityCardIssue::dispatch(int signal,
                                    int user_idx,
                                    Stream* stream) {
    if (!DBDispatcher::dispatch(signal, user_idx, stream)) {
        return false;
    }

    SIG_SECURITY_CARD_ISSUE_LOCAL payload;
    memset(&payload, 0, sizeof(payload));
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect apply_flag from member_security_card where phone='%s' and apply_flag=1",
        payload.phone_hash);
    if (db_handle->exec(true) != true || db_handle->get_n_rows() > 4) {
        _sendResult(user_idx, payload.phone, 0, 0, 0,
                    static_cast<ENUM_ERROR>(119));
        return false;
    }

    db_handle->set_query(
        "seLect re_issue_cnt,unix_timestamp(last_issue_time) from member_security_card where m_id=%s",
        NumberToString(payload.account_id, 0));
    if (db_handle->exec(true) != true) {
        _sendResult(user_idx, payload.phone, 0, 0, 0,
                    static_cast<ENUM_ERROR>(1));
        return false;
    }

    char q_pos[5];
    memset(q_pos, 0, sizeof(q_pos));
    generateSecurityCardQuestion(q_pos);
    strncpy(payload.q_pos, q_pos, 5u);

    if (db_handle->get_n_rows()) {
        unsigned int reissue_count = 0;
        unsigned int last_issue_time = 0;
        if (db_handle->fetch() != true) {
            _sendResult(user_idx, payload.phone, 0, 0, 0,
                        static_cast<ENUM_ERROR>(1));
            return false;
        }
        db_handle->get_uint(0, reissue_count);
        db_handle->get_uint(1, last_issue_time);

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
                _sendResult(user_idx, payload.phone, 0, 0, 0,
                            static_cast<ENUM_ERROR>(120));
                return false;
            }
        } else {
            reissue_count = 0;
        }

        db_handle->set_query(
            "upDate member_security_card set phone='%s',cert_key='%s',server_key='%s',card='%s',fail_cnt=0,re_issue_cnt=%d,last_issue_time=now(),validity_time=0,apply_flag=0,q_pos='%s' where m_id=%s",
            payload.phone_hash,
            payload.cert_key,
            payload.server_key,
            payload.card,
            reissue_count,
            q_pos,
            NumberToString(payload.account_id, 0));
        db_handle->exec(true);
    } else {
        db_handle->set_query(
            "inSert into member_security_card(m_id,occ_time,phone,cert_key,server_key,card,fail_cnt,re_issue_cnt,last_issue_time,validity_time,apply_flag,q_pos) values(%s,now(),'%s','%s','%s','%s',0,0,now(),0,0,'%s')",
            NumberToString(payload.account_id, 0),
            payload.phone_hash,
            payload.cert_key,
            payload.server_key,
            payload.card,
            q_pos);
        db_handle->exec(true);
    }

    turnOffSecurityFlag(db_handle, payload.account_id);
    _sendResult(user_idx,
                payload.phone,
                payload.server_key,
                payload.card,
                payload.q_pos,
                static_cast<ENUM_ERROR>(0));
    return true;
}

// IDA_EA: 0x842B410
// IDA_NAME: _ZN20DB_SecurityCardIssue11_sendResultEiPKcS1_S1_S1_10ENUM_ERROR
// DEMANGLED: DB_SecurityCardIssue::_sendResult(int,char const*,char const*,char const*,char const*,ENUM_ERROR)
void DB_SecurityCardIssue::_sendResult(int user_idx,
                                       const char* phone,
                                       const char* server_key,
                                       const char* card,
                                       const char* q_pos,
                                       ENUM_ERROR error) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 28140);
    CStreamGuard guard(stream, true);
    *guard << 253;
    *guard << user_idx;

    SIG_SECURITY_CARD_ISSUE_LOCAL* payload =
        static_cast<SIG_SECURITY_CARD_ISSUE_LOCAL*>(
            guard->GetInBufferUntyped(sizeof(SIG_SECURITY_CARD_ISSUE_LOCAL)));
    memset(payload, 0, sizeof(*payload));
    payload->error = error;
    if (server_key) {
        strncpy(payload->server_key, server_key, 0x21u);
    }
    if (card) {
        strncpy(payload->card, card, 0x27u);
    }
    if (q_pos) {
        strncpy(payload->q_pos, q_pos, 5u);
    }
    if (phone) {
        strncpy(payload->phone, phone, 0xCu);
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
}

// IDA_EA: 0x842B57A
// IDA_NAME: _ZN20DB_SecurityCardIssue11makeRequestEijPKcS1_S1_S1_S1_
// DEMANGLED: DB_SecurityCardIssue::makeRequest(int,unsigned int,char const*,char const*,char const*,char const*,char const*)
void DB_SecurityCardIssue::makeRequest(int user_idx,
                                       unsigned int account_id,
                                       const char* cert_key,
                                       const char* server_key,
                                       const char* phone,
                                       const char* phone_hash,
                                       const char* card) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 28170);
    CStreamGuard guard(stream, true);
    *guard << 253;
    *guard << user_idx;

    SIG_SECURITY_CARD_ISSUE_LOCAL* payload =
        static_cast<SIG_SECURITY_CARD_ISSUE_LOCAL*>(
            guard->GetInBufferUntyped(sizeof(SIG_SECURITY_CARD_ISSUE_LOCAL)));
    memset(payload, 0, sizeof(*payload));
    payload->account_id = account_id;
    if (cert_key) {
        strncpy(payload->cert_key, cert_key, 6u);
    }
    if (server_key) {
        strncpy(payload->server_key, server_key, 0x21u);
    }
    if (phone) {
        strncpy(payload->phone, phone, 0xCu);
    }
    if (card) {
        strncpy(payload->card, card, 0x27u);
    }
    if (phone_hash) {
        strncpy(payload->phone_hash, phone_hash, 0x29u);
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
