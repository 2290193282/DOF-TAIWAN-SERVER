#include "old_game/dungeon/instance/dungeon_entrance_log.h"

#include <stdint.h>

#include "common/manager/global_data.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

// IDA_EA: 0x86103FE
// IDA_NAME: _ZN19CDungeonEntranceLogC2Ev
// DEMANGLED: CDungeonEntranceLog::CDungeonEntranceLog(void)
CDungeonEntranceLog::CDungeonEntranceLog()
    : Base() {}

// IDA_EA: 0x861089A
// IDA_NAME: _ZN19CDungeonEntranceLogD2Ev
// DEMANGLED: CDungeonEntranceLog::~CDungeonEntranceLog()
CDungeonEntranceLog::~CDungeonEntranceLog() {}

// IDA_EA: 0x860E324
// IDA_NAME: _ZN19CDungeonEntranceLog24DecrementDungeonEntranceEib
// DEMANGLED: CDungeonEntranceLog::DecrementDungeonEntrance(int,bool)
int CDungeonEntranceLog::DecrementDungeonEntrance(int dungeon_index,
                                                  bool /*tower*/) {
    Base::iterator it = find(dungeon_index);
    if (it == end()) {
        return 0;
    }
    --it->second.normal_active_count;
    return static_cast<int>(reinterpret_cast<uintptr_t>(&*it));
}

// IDA_EA: 0x860E38E
// IDA_NAME: _ZN19CDungeonEntranceLog26InsertDungeonEntrance_HourEv
// DEMANGLED: CDungeonEntranceLog::InsertDungeonEntrance_Hour(void)
int CDungeonEntranceLog::InsertDungeonEntrance_Hour() {
    // Hex-Rays types its scratch as char[4], but the actual call edge is
    // CStreamGuard::operator<<(int) @ 0x80C8C56. The count is four bytes.
    const int count = static_cast<int>(size());
    if (count > 0) {
        CStreamGuard guard(
            GlobalData::s_stream_pool->Acquire("Statistics.cpp", 1205), true);
        guard << 225;
        guard << -1;
        guard << count;

        for (Base::const_iterator entry = begin(); entry != end(); ++entry) {
            guard << entry->first;
            guard << entry->second.normal_entrance_hour;
            guard << entry->second.normal_active_count;
        }
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, guard);

        for (Base::iterator entry = begin(); entry != end(); ++entry) {
            entry->second.normal_entrance_hour = 0;
        }
    }
    return 1;
}

// IDA_EA: 0x860E59C
// IDA_NAME: _ZN19CDungeonEntranceLog21InsertDungeonEntranceEv
// DEMANGLED: CDungeonEntranceLog::InsertDungeonEntrance(void)
int CDungeonEntranceLog::InsertDungeonEntrance() {
    // The raw callee is likewise the signed-int stream overload.
    const int count = static_cast<int>(size());
    if (count > 0) {
        CStreamGuard guard(
            GlobalData::s_stream_pool->Acquire("Statistics.cpp", 1242), true);
        guard << 224;
        guard << -1;
        guard << count;

        for (Base::const_iterator entry = begin(); entry != end(); ++entry) {
            guard << entry->first;
            guard << entry->second.normal_entrance_total;
            guard << entry->second.tower_entrance_total;
        }
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, guard);
        clear();
    }
    return 1;
}
