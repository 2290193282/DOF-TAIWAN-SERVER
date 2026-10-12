#include "old_data/db/event/db_save_pc_room_play_time_event.h"

#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_data/db/core/mysql.h"
#include "old_data/PVF/data/data_manager.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <string>


namespace {

enum {
    kDbSignalSavePcRoomPlayTimeEvent = 0x2FF,
    kPcRoomPlayTimeEventType = 92,
    kPcRoomPlayTimeDbHandle = 9,
};

}  // namespace

// IDA_EA: 0x844FCE2
// IDA_NAME: _ZN26DB_SavePcRoomPlayTimeEventC2Ev
// DEMANGLED: DB_SavePcRoomPlayTimeEvent::DB_SavePcRoomPlayTimeEvent(void)
DB_SavePcRoomPlayTimeEvent::DB_SavePcRoomPlayTimeEvent() {}

// IDA_EA: 0x845B092
// IDA_NAME: _ZN26DB_SavePcRoomPlayTimeEventD2Ev
// DEMANGLED: DB_SavePcRoomPlayTimeEvent::~DB_SavePcRoomPlayTimeEvent()
// IDA_EA: 0x845B0C2
// IDA_NAME: _ZN26DB_SavePcRoomPlayTimeEventD0Ev
// DEMANGLED: DB_SavePcRoomPlayTimeEvent::~DB_SavePcRoomPlayTimeEvent()
// deleting destructor
DB_SavePcRoomPlayTimeEvent::~DB_SavePcRoomPlayTimeEvent() {}

// IDA_EA: 0x8447992
// IDA_NAME: _ZN26DB_SavePcRoomPlayTimeEvent8dispatchEiiP6Stream
// DEMANGLED: DB_SavePcRoomPlayTimeEvent::dispatch(int,int,Stream *)
bool DB_SavePcRoomPlayTimeEvent::dispatch(int signal,
                                          int user_idx,
                                          Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(
        kPcRoomPlayTimeDbHandle),static_cast<ENUM_SERVER_GROUP>(
        0));
    SIG_SAVE_PC_ROOM_PLAY_TIME* request =
        stream->GetOutBuffer<SIG_SAVE_PC_ROOM_PLAY_TIME>();
    const std::string table_name(
        *G_CDataManager()
             ->get_event_script_mng()
             ->get_db_table_name(kPcRoomPlayTimeEventType));
    db_handle->set_query(
        "upDate %s set play_time = %u, reward_cnt = %u, occ_date = now() where m_id = %s",
        ::toTString(table_name),
        request->play_time,
        request->reward_index,
        NumberToString(request->account_id, 0));
    if (db_handle->exec(true) != true) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "virtual bool DB_SavePcRoomPlayTimeEvent::dispatch(int, int, Stream*)",
            45659,
            "<PCROOM_PLAY_TIME_EVENT> update_error m_id(%d), total_time(%d), reward_cnt(%d)",
            request->account_id,
            request->play_time,
            request->reward_index);
        return false;
    }
    if (db_handle->getAffectedRowCount()) {
        return true;
    }

    db_handle->set_query(
        "inSert into %s(m_id, play_time, reward_cnt, occ_date) values(%s,%u,%u,now())",
        ::toTString(table_name),
        NumberToString(request->account_id, 0),
        request->play_time,
        request->reward_index);
    if (db_handle->exec(true) == true) {
        return true;
    }

    LogManager::logFormat(
        1,
        "DBThread.cpp",
        "virtual bool DB_SavePcRoomPlayTimeEvent::dispatch(int, int, Stream*)",
        45670,
        "<PCROOM_PLAY_TIME_EVENT> insert_error m_id(%d), total_time(%d), reward_cnt(%d)",
        request->account_id,
        request->play_time,
        request->reward_index);
    return false;
}

// IDA_EA: 0x8447BD0
// IDA_NAME: _ZN26DB_SavePcRoomPlayTimeEvent11makeRequestEjjh
// DEMANGLED: DB_SavePcRoomPlayTimeEvent::makeRequest(unsigned int,unsigned int,unsigned char)
void DB_SavePcRoomPlayTimeEvent::makeRequest(unsigned int account_id,
                                             unsigned int total_play_time,
                                             unsigned char reward_count) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 45680);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalSavePcRoomPlayTimeEvent;
    *guard << -1;
    SIG_SAVE_PC_ROOM_PLAY_TIME* request =
        guard->GetInBuffer<SIG_SAVE_PC_ROOM_PLAY_TIME>();
    request->account_id = account_id;
    request->play_time = total_play_time;
    request->reward_index = reward_count;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
