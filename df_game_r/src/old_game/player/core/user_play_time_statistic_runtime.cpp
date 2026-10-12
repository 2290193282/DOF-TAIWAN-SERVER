#include "old_game/player/core/user_play_time_statistic_runtime.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

// IDA_EA: 0x86102C0
// IDA_NAME: _ZN25CUserPlayTimeStatisticMgrC2Ev
// DEMANGLED: CUserPlayTimeStatisticMgr::CUserPlayTimeStatisticMgr(void)
CUserPlayTimeStatisticMgr::CUserPlayTimeStatisticMgr() : entries_() {}

// IDA_EA: 0x86107EC
// IDA_NAME: _ZN25CUserPlayTimeStatisticMgrD2Ev
// DEMANGLED: CUserPlayTimeStatisticMgr::~CUserPlayTimeStatisticMgr()
CUserPlayTimeStatisticMgr::~CUserPlayTimeStatisticMgr() {}

// IDA_EA: 0x860CC38
// IDA_NAME: _ZN25CUserPlayTimeStatisticMgr18UpdateUserPlayTimeEji
// DEMANGLED: CUserPlayTimeStatisticMgr::UpdateUserPlayTime(unsigned int,int)
int CUserPlayTimeStatisticMgr::UpdateUserPlayTime(uint32_t account_id,
                                                   int play_time) {
    std::map<uint32_t, stUserPlayInfo_t>::iterator found =
        entries_.find(account_id);
    if (found != entries_.end()) {
        found->second.play_time += play_time;
        ++found->second.play_count;
    } else {
        stUserPlayInfo_t info;
        info.play_time = play_time;
        info.play_count = 1;
        entries_.insert(std::make_pair(account_id, info));
    }
    return 1;
}

// IDA_EA: 0x860CD14
// IDA_NAME: _ZN25CUserPlayTimeStatisticMgr18InsertUserPlayTimeEv
// DEMANGLED: CUserPlayTimeStatisticMgr::InsertUserPlayTime(void)
int CUserPlayTimeStatisticMgr::InsertUserPlayTime() {
    if (entries_.empty()) {
        return 1;
    }

    CStreamGuard stream_guard(0, false);
    SIG_USER_PLAY_TIME_STATISTIC* signal = 0;
    int record_count = 0;

    for (std::map<uint32_t, stUserPlayInfo_t>::const_iterator entry =
             entries_.begin();
         entry != entries_.end();
         ++entry) {
        if (!stream_guard.Get()) {
            stream_guard.Set(
                GlobalData::s_stream_pool->Acquire("Statistics.cpp", 586));
            stream_guard << 182;
            stream_guard << -1;
            signal = stream_guard->GetInBuffer<SIG_USER_PLAY_TIME_STATISTIC>();
            std::memset(signal, 0, sizeof(*signal));
        }

        signal->records[record_count].account_id = entry->first;
        signal->records[record_count].play_time = entry->second.play_time;
        signal->records[record_count].play_count = entry->second.play_count;
        ++record_count;

        if (record_count > 19) {
            signal->record_count = record_count;
            GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, stream_guard);
            stream_guard.Set(0);
            record_count = 0;
        }
    }

    if (record_count > 0) {
        signal->record_count = record_count;
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, stream_guard);
        stream_guard.Set(0);
    }
    entries_.clear();
    return 1;
}

// IDA_EA: 0x860CBA5
// IDA_NAME: _Z35GetInstanceUserPlayTimeStatisticMgrv
// DEMANGLED: GetInstanceUserPlayTimeStatisticMgr(void)
CUserPlayTimeStatisticMgr* GetInstanceUserPlayTimeStatisticMgr() {
    static CUserPlayTimeStatisticMgr instance;
    return &instance;
}
