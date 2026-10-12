#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"

#include "common/manager/global_data.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

// IDA_EA: 0x860EDF4
// IDA_NAME: _ZN26CLevelDungeonPlayStatistic14InsertDatabaseEv
// DEMANGLED: CLevelDungeonPlayStatistic::InsertDatabase(void)
void CLevelDungeonPlayStatistic::InsertDatabase() {
    if (entries.empty()) {
        return;
    }

    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire("Statistics.cpp", 1385), true);
    guard << 263;
    guard << -1;
    // The original serializes the map size through a signed byte local.
    guard << static_cast<char>(entries.size());
    for (std::map<int, stDungeonPlayInfo>::const_iterator entry = entries.begin();
         entry != entries.end();
         ++entry) {
        guard << entry->first;
        guard << entry->second.play_count;
        guard << entry->second.play_value;
    }

    GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, guard);
    entries.clear();
}

// IDA_EA: 0x860F110
// IDA_NAME: _ZN20CDungeonExpStatistic14InsertDatabaseEv
// DEMANGLED: CDungeonExpStatistic::InsertDatabase(void)
void CDungeonExpStatistic::InsertDatabase() {
    if (entries.empty()) {
        return;
    }

    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire("Statistics.cpp", 1436), true);
    guard << 278;
    guard << -1;
    // This is likewise the original one-byte count, not an aligned int.
    guard << static_cast<char>(entries.size());
    for (std::map<int, stDungeonExp>::const_iterator entry = entries.begin();
         entry != entries.end();
         ++entry) {
        guard << entry->first;
        guard << entry->second.exp_value;
        guard << entry->second.bonus_value;
        guard << entry->second.play_count;
    }

    GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, guard);
    entries.clear();
}
