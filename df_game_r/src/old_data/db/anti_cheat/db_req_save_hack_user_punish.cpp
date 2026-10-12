#include "old_data/db/anti_cheat/db_req_save_hack_user_punish.h"

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



#pragma pack(push, 1)
struct SIG_SAVE_HACK_USER_PUNISH {
    uint32_t m_id;          // +0
    uint32_t value;         // +4
    char ip[16];            // +8
};
#pragma pack(pop)

typedef char SIG_SAVE_HACK_USER_PUNISH_Size_Must_Be_24[
    (sizeof(SIG_SAVE_HACK_USER_PUNISH) == 24) ? 1 : -1];

// IDA_EA: 0x844E57A
// IDA_NAME: _ZN24DB_ReqSaveHackUserPunishC2Ev
// DEMANGLED: DB_ReqSaveHackUserPunish::DB_ReqSaveHackUserPunish(void)
DB_ReqSaveHackUserPunish::DB_ReqSaveHackUserPunish() {}

// IDA_EA: 0x845ED82
// IDA_NAME: _ZN24DB_ReqSaveHackUserPunishD2Ev
// DEMANGLED: DB_ReqSaveHackUserPunish::~DB_ReqSaveHackUserPunish()
// IDA_EA: 0x845EDB2
// IDA_NAME: _ZN24DB_ReqSaveHackUserPunishD0Ev
// DEMANGLED: DB_ReqSaveHackUserPunish::~DB_ReqSaveHackUserPunish() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_ReqSaveHackUserPunish::~DB_ReqSaveHackUserPunish() {}

// IDA_EA: 0x8425374
// IDA_NAME: _ZN24DB_ReqSaveHackUserPunish8dispatchEiiP6Stream
// DEMANGLED: DB_ReqSaveHackUserPunish::dispatch(int,int,Stream *)
bool DB_ReqSaveHackUserPunish::dispatch(int signal, int user_idx,
                                        Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_SAVE_HACK_USER_PUNISH payload;
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }

    GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0));
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1),static_cast<ENUM_SERVER_GROUP>( 0));
    const char* account_id = NumberToString(payload.m_id, 0);
    db_handle->set_query(
        "upDate member_punish_info set occ_time=now(), start_time=now(), end_time='9999-12-31 23:59:59', punish_value=0, apply_flag=2 where m_id=%s and punish_type=%d",
        account_id,
        2);

    bool need_insert = true;
    if (db_handle->exec(true) == true) {
        if (db_handle->getAffectedRowCount()) {
            need_insert = false;
        }
    }

    if (need_insert) {
        account_id = NumberToString(payload.m_id, 0);
        db_handle->set_query(
            "inSert into member_punish_info(m_id, punish_type, occ_time, start_time, end_time, punish_value, apply_flag) values(%s, %d, now(), now(), '9999-12-31 23:59:59', 0, 2)",
            account_id,
            2);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x8425494
// IDA_NAME: _ZN24DB_ReqSaveHackUserPunish11makeRequestEjjPKc
// DEMANGLED: DB_ReqSaveHackUserPunish::makeRequest(unsigned int,unsigned int,char const*)
void DB_ReqSaveHackUserPunish::makeRequest(unsigned int m_id,
                                           unsigned int value,
                                           const char* src) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 25348);
    CStreamGuard guard(stream, true);
    *guard << 181;
    *guard << -1;

    SIG_SAVE_HACK_USER_PUNISH* payload =
        static_cast<SIG_SAVE_HACK_USER_PUNISH*>(
            guard->GetInBufferUntyped(sizeof(SIG_SAVE_HACK_USER_PUNISH)));
    payload->m_id = m_id;
    payload->value = value;
    if (src) {
        strncpy(payload->ip, src, 0x10u);
    } else {
        memcpy(payload->ip, "000.000.000.000", 0x10u);
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
