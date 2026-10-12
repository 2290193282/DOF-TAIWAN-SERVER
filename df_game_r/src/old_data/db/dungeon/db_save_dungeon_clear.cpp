#include "old_data/db/dungeon/db_save_dungeon_clear.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

// IDA_EA: 0x8419FEC
// IDA_NAME: _ZN19DB_SaveDungeonClearC2Ev
// DEMANGLED: DB_SaveDungeonClear::DB_SaveDungeonClear(void)
DB_SaveDungeonClear::DB_SaveDungeonClear() {}

// IDA_EA: 0x841A008
// IDA_NAME: _ZN19DB_SaveDungeonClearD2Ev
// DEMANGLED: DB_SaveDungeonClear::~DB_SaveDungeonClear()
// IDA_EA: 0x841A038
// IDA_NAME: _ZN19DB_SaveDungeonClearD0Ev
// DEMANGLED: DB_SaveDungeonClear::~DB_SaveDungeonClear() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_SaveDungeonClear::~DB_SaveDungeonClear() {}

// IDA_EA: 0x841A056
// IDA_NAME: _ZN19DB_SaveDungeonClear8dispatchEiiP6Stream
// DEMANGLED: DB_SaveDungeonClear::dispatch(int,int,Stream *)
bool DB_SaveDungeonClear::dispatch(int signal,
                                   int user_idx,
                                   Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_SAVE_DUNGEON_CLEAR* payload =
        stream->GetOutBuffer<SIG_SAVE_DUNGEON_CLEAR>();
    if (!payload) {
        return false;
    }
    return SaveDungeonClear(payload) == true;
}

// IDA_EA: 0x841A0A6
// IDA_NAME: _ZN19DB_SaveDungeonClear16SaveDungeonClearEP22SIG_SAVE_DUNGEON_CLEAR
// DEMANGLED: DB_SaveDungeonClear::SaveDungeonClear(SIG_SAVE_DUNGEON_CLEAR *)
bool DB_SaveDungeonClear::SaveDungeonClear(SIG_SAVE_DUNGEON_CLEAR* payload_data) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    const char* member_id =
        NumberToString(payload_data->account_id, 0);
    db_handle->set_query("upDate member_dungeon set dungeon='%s' where m_id=%s",
                         payload_data->dungeon,
                         member_id);

    bool need_insert = true;
    if (db_handle->exec(true) == true) {
        if (db_handle->getAffectedRowCount()) {
            need_insert = false;
        }
    }
    if (need_insert) {
        member_id = NumberToString(payload_data->account_id, 0);
        db_handle->set_query(
            "inSert into member_dungeon (m_id,dungeon) values (%s,'%s')",
            member_id,
            payload_data->dungeon);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x841A1A2
// IDA_NAME: _ZN19DB_SaveDungeonClear10SendResultEiiP22SIG_SAVE_DUNGEON_CLEAR
// DEMANGLED: DB_SaveDungeonClear::SendResult(int,int,SIG_SAVE_DUNGEON_CLEAR *)
bool DB_SaveDungeonClear::SendResult(int signal,
                                     int user_idx,
                                     SIG_SAVE_DUNGEON_CLEAR* payload) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 19491);
    CStreamGuard guard(stream, true);
    *guard << signal;
    *guard << user_idx;
    guard->put_binary(payload, sizeof(*payload));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}
