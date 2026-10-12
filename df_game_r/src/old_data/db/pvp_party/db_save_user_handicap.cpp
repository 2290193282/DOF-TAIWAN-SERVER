#include "old_data/db/pvp_party/db_save_user_handicap.h"

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
struct SIG_UPDATE_USER_HANDICAP {
    uint32_t event_id;       // +0, preserved old field written by caller.
    uint32_t account_id;     // +4
    uint32_t punish_type;    // +8
    uint32_t punish_value;   // +12
    uint32_t duration_days;  // +16
    uint32_t src;            // +20
};
#pragma pack(pop)

typedef char SIG_UPDATE_USER_HANDICAP_Size_Must_Be_24[
    (sizeof(SIG_UPDATE_USER_HANDICAP) == 24) ? 1 : -1];

// IDA_EA: 0x8427DFC
// IDA_NAME: _ZN19DB_SaveUserHandicap12_getDBHandleEv
// DEMANGLED: DB_SaveUserHandicap::_getDBHandle(void)
MySQL* DB_SaveUserHandicap::_getDBHandle() {
    return GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1),static_cast<ENUM_SERVER_GROUP>( 0));
}

// IDA_EA: 0x8427E28
// IDA_NAME: _ZN19DB_SaveUserHandicap30_getDBHandleForDevideAccountDBEv
// DEMANGLED: DB_SaveUserHandicap::_getDBHandleForDevideAccountDB(void)
MySQL* DB_SaveUserHandicap::_getDBHandleForDevideAccountDB() {
    return GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1),static_cast<ENUM_SERVER_GROUP>( 0));
}

// IDA_EA: 0x844E702
// IDA_NAME: _ZN19DB_SaveUserHandicapC2Ev
// DEMANGLED: DB_SaveUserHandicap::DB_SaveUserHandicap(void)
DB_SaveUserHandicap::DB_SaveUserHandicap() {}

// IDA_EA: 0x845E8F0
// IDA_NAME: _ZN19DB_SaveUserHandicapD2Ev
// DEMANGLED: DB_SaveUserHandicap::~DB_SaveUserHandicap()
// IDA_EA: 0x845E920
// IDA_NAME: _ZN19DB_SaveUserHandicapD0Ev
// DEMANGLED: DB_SaveUserHandicap::~DB_SaveUserHandicap() deleting destructor
DB_SaveUserHandicap::~DB_SaveUserHandicap() {}

// IDA_EA: 0x8427E54
// IDA_NAME: _ZN19DB_SaveUserHandicap8dispatchEiiP6Stream
// DEMANGLED: DB_SaveUserHandicap::dispatch(int,int,Stream *)
bool DB_SaveUserHandicap::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_UPDATE_USER_HANDICAP payload;
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }

    MySQL* db_handle = _getDBHandle();
    if (payload.duration_days) {
        db_handle->set_query(
            "upDate member_punish_info set occ_time=now(), start_time=now(), end_time=adddate(now(),interval %d day), punish_value=%d, apply_flag=2 where m_id=%s and punish_type=%d",
            payload.duration_days,
            payload.punish_value,
            NumberToString(payload.account_id, 0),
            payload.punish_type);
    } else {
        db_handle->set_query(
            "upDate member_punish_info set occ_time=now(), start_time=now(), end_time='9999-12-31 23:59:59', punish_value=%d, apply_flag=2 where m_id=%s and punish_type=%d",
            payload.punish_value,
            NumberToString(payload.account_id, 0),
            payload.punish_type);
    }

    bool need_insert = true;
    if (db_handle->exec(true) == true && db_handle->getAffectedRowCount()) {
        need_insert = false;
    }

    if (need_insert) {
        if (payload.duration_days) {
            db_handle->set_query(
                "inSert into member_punish_info(m_id, occ_time, start_time, end_time, punish_type, punish_value, apply_flag) values(%s, now(), now(), adddate(now(),interval %d day), %d, %d, 2)",
                NumberToString(payload.account_id, 0),
                payload.duration_days,
                payload.punish_type,
                payload.punish_value);
        } else {
            db_handle->set_query(
                "inSert into member_punish_info(m_id, occ_time, start_time, end_time, punish_type, punish_value, apply_flag) values(%s, now(), now(),'9999-12-31 23:59:59', %d, %d, 2)",
                NumberToString(payload.account_id, 0),
                payload.punish_type,
                payload.punish_value);
        }
        db_handle->exec(true);
    }

    return true;
}

// IDA_EA: 0x842801A
// IDA_NAME: _ZN19DB_SaveUserHandicap11makeRequestEjiiiii
// DEMANGLED: DB_SaveUserHandicap::makeRequest(unsigned int,int,int,int,int,int)
void DB_SaveUserHandicap::makeRequest(unsigned int account_id,
                                      int event_id,
                                      int punish_type,
                                      int punish_value,
                                      int duration_days,
                                      int src) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 27018);
    CStreamGuard guard(stream, true);
    *guard << 206;
    *guard << -1;

    SIG_UPDATE_USER_HANDICAP* payload =
        static_cast<SIG_UPDATE_USER_HANDICAP*>(
            guard->GetInBufferUntyped(sizeof(SIG_UPDATE_USER_HANDICAP)));
    if (payload) {
        memset(payload, 0, sizeof(SIG_UPDATE_USER_HANDICAP));
        payload->event_id = static_cast<uint32_t>(event_id);
        payload->account_id = account_id;
        payload->punish_type = static_cast<uint32_t>(punish_type);
        payload->punish_value = static_cast<uint32_t>(punish_value);
        payload->duration_days = static_cast<uint32_t>(duration_days);
        payload->src = static_cast<uint32_t>(src);
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
