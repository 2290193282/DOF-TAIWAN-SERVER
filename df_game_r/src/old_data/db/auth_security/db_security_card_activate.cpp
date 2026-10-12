#include "old_data/db/auth_security/db_security_card_activate.h"

#include "old_data/db/auth_security/db_cancel_resting_user_restrict.h"
#include "old_data/db/auth_security/db_check_secu_reward.h"
#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>
#include <string.h>



extern int turnOnSecurityFlag(MySQL* db, unsigned int account_id)
    __asm__("_Z18turnOnSecurityFlagP5MySQLj");
extern int saveSecurityCardHistory(MySQL* db,
                                   unsigned int account_id,
                                   int modify_type)
    __asm__("_Z23saveSecurityCardHistoryP5MySQLj26ENUM_SECURITYCARD_MOD_TYPE");
#pragma pack(push, 1)
struct SIG_SECURITY_CARD_ACTIVATE_LOCAL {
    uint32_t account_id;       // +0
    char cert_key[6];          // +4
    uint8_t activated;         // +10
    uint8_t reissue_history;   // +11
};
#pragma pack(pop)

typedef char SIG_SECURITY_CARD_ACTIVATE_LOCAL_Size_Must_Be_12[
    (sizeof(SIG_SECURITY_CARD_ACTIVATE_LOCAL) == 12) ? 1 : -1];

// IDA_EA: 0x844E8FA
// IDA_NAME: _ZN23DB_SecurityCardActivateC2Ev
// DEMANGLED: DB_SecurityCardActivate::DB_SecurityCardActivate(void)
DB_SecurityCardActivate::DB_SecurityCardActivate() {}

// IDA_EA: 0x845E3C2
// IDA_NAME: _ZN23DB_SecurityCardActivateD2Ev
// DEMANGLED: DB_SecurityCardActivate::~DB_SecurityCardActivate()
// IDA_EA: 0x845E3F2
// IDA_NAME: _ZN23DB_SecurityCardActivateD0Ev
// DEMANGLED: DB_SecurityCardActivate::~DB_SecurityCardActivate() deleting destructor
DB_SecurityCardActivate::~DB_SecurityCardActivate() {}

// IDA_EA: 0x842B86C
// IDA_NAME: _ZN23DB_SecurityCardActivate8dispatchEiiP6Stream
// DEMANGLED: DB_SecurityCardActivate::dispatch(int,int,Stream *)
bool DB_SecurityCardActivate::dispatch(int signal,
                                       int user_idx,
                                       Stream* stream) {
    (void)signal;

    SIG_SECURITY_CARD_ACTIVATE_LOCAL payload;
    memset(&payload, 0, sizeof(payload));
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "upDate member_security_card set apply_flag=1, cert_flag=1, cert_cnt = 0 where m_id=%s and cert_key='%s'",
        NumberToString(payload.account_id, 0),
        payload.cert_key);
    db_handle->exec(true);

    payload.activated = 0;
    if (db_handle->getAffectedRowCount() == 1) {
        payload.activated = 1;
        turnOnSecurityFlag(db_handle, payload.account_id);
    }

    DB_CancelRestingUserRestrict::makeRequest(payload.account_id, user_idx);
    if (payload.reissue_history) {
        saveSecurityCardHistory(db_handle, payload.account_id, 1);
    } else {
        saveSecurityCardHistory(db_handle, payload.account_id, 0);
    }

    MySQL* member_db = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1),static_cast<ENUM_SERVER_GROUP>( 0));
    if (payload.activated) {
        member_db->set_query(
            "upDate member_security_grade set security_card_reg=now() where m_id=%s",
            NumberToString(payload.account_id, 0));
        bool need_insert = true;
        if (member_db->exec(true) == true &&
            member_db->getAffectedRowCount()) {
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
        DB_CheckSecuReward::makeRequest(user_idx, payload.account_id, 0);
    }

    Stream* out_stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 28297);
    CStreamGuard guard(out_stream, true);
    *guard << 255;
    *guard << user_idx;

    SIG_SECURITY_CARD_ACTIVATE_LOCAL* out_payload =
        static_cast<SIG_SECURITY_CARD_ACTIVATE_LOCAL*>(
            guard->GetInBufferUntyped(
                sizeof(SIG_SECURITY_CARD_ACTIVATE_LOCAL)));
    memset(out_payload, 0, sizeof(*out_payload));
    out_payload->activated = payload.activated;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x842BB5C
// IDA_NAME: _ZN23DB_SecurityCardActivate11makeRequestEijPKcb
// DEMANGLED: DB_SecurityCardActivate::makeRequest(int,unsigned int,char const*,bool)
void DB_SecurityCardActivate::makeRequest(int user_idx,
                                          unsigned int account_id,
                                          const char* cert_key,
                                          bool reissue_history) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 28313);
    CStreamGuard guard(stream, true);
    *guard << 255;
    *guard << user_idx;

    SIG_SECURITY_CARD_ACTIVATE_LOCAL* payload =
        static_cast<SIG_SECURITY_CARD_ACTIVATE_LOCAL*>(
            guard->GetInBufferUntyped(
                sizeof(SIG_SECURITY_CARD_ACTIVATE_LOCAL)));
    memset(payload, 0, sizeof(*payload));
    payload->account_id = account_id;
    if (cert_key) {
        strncpy(payload->cert_key, cert_key, 6u);
    }
    payload->reissue_history = reissue_history ? 1 : 0;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
