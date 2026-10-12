#include "old_game/pvp/battlefield/statistics_pvp_runtime.h"

#include <utility>

#include "infra/concurrent/thread/guard_mutex.h"

// IDA_EA: 0x860C4F6
// IDA_NAME: _ZN13StatisticsPvPC2Ev
// DEMANGLED: StatisticsPvP::StatisticsPvP(void)
StatisticsPvP::StatisticsPvP()
    : Mutex(),
      room_results_() {}

// IDA_EA: 0x860C53C
// IDA_NAME: _ZN13StatisticsPvPD2Ev
// DEMANGLED: StatisticsPvP::~StatisticsPvP()
StatisticsPvP::~StatisticsPvP() {}

// IDA_EA: 0x844D524
// IDA_NAME: _ZN13StatisticsPvP4lockEv
// DEMANGLED: StatisticsPvP::lock(void)
int StatisticsPvP::lock() {
    return Mutex::lock();
}

// IDA_EA: 0x844D538
// IDA_NAME: _ZN13StatisticsPvP6unlockEv
// DEMANGLED: StatisticsPvP::unlock(void)
int StatisticsPvP::unlock() {
    return Mutex::unlock();
}

// IDA_EA: 0x860C582
// IDA_NAME: _ZN13StatisticsPvP5resetEv
// DEMANGLED: StatisticsPvP::reset(void)
int StatisticsPvP::reset() {
    Guard<Mutex> guard(this);
    room_results_.clear();
    return 0;
}

// IDA_EA: 0x860C5DA
// IDA_NAME: _ZN13StatisticsPvP16incPvPRoomResultEiii
// DEMANGLED: StatisticsPvP::incPvPRoomResult(int,int,int)
int StatisticsPvP::incPvPRoomResult(int room_number,
                                    int play_type,
                                    int participant_count) {
    Guard<Mutex> guard(this);

    PvP_Key key = {room_number, play_type};
    std::map<PvP_Key, PvP_Room>::iterator found =
        room_results_.find(key);
    if (found == room_results_.end()) {
        PvP_Room room = {
            room_number,
            play_type,
            1,
            participant_count,
        };
        room_results_.insert(std::make_pair(key, room));
    } else {
        ++found->second.occurrence_count;
        found->second.participant_count += participant_count;
    }
    return 0;
}
