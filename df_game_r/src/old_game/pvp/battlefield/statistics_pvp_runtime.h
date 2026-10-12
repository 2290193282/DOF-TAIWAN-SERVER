#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_STATISTICS_PVP_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_STATISTICS_PVP_RUNTIME_H_

#include <stddef.h>

#include <map>

#include "infra/concurrent/thread/mutex.h"

// IDA unit: 09_dungeon_pvp_party/StatisticsPvP.
//
// GameWorld embeds this service at +0xec.  The original object is a Mutex
// base followed immediately by the room-result map at +0x18.
class StatisticsPvP : public Mutex {
public:
    struct PvP_Key {
        bool operator<(const PvP_Key& rhs) const;

        int room_number;
        int play_type;
    };

    struct PvP_Room {
        int room_number;
        int play_type;
        int occurrence_count;
        int participant_count;
    };

    StatisticsPvP();
    ~StatisticsPvP();

    // IDA_EA: 0x860C582
    // IDA_NAME: _ZN13StatisticsPvP5resetEv
    // DEMANGLED: StatisticsPvP::reset(void)
    int reset();

    // IDA_EA: 0x860C5DA
    // IDA_NAME: _ZN13StatisticsPvP16incPvPRoomResultEiii
    // DEMANGLED: StatisticsPvP::incPvPRoomResult(int,int,int)
    int incPvPRoomResult(int room_number,
                         int play_type,
                         int participant_count);

    // IDA_EA: 0x844D524
    // IDA_NAME: _ZN13StatisticsPvP4lockEv
    int lock();

    // IDA_EA: 0x844D538
    // IDA_NAME: _ZN13StatisticsPvP6unlockEv
    int unlock();

    // Public solely so cross-unit ABI audits and the DB statistics consumer
    // can inspect the exact typed service boundary.
    std::map<PvP_Key, PvP_Room> room_results_;  // +0x18
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(StatisticsPvP::PvP_Key) == 0x08u,
              "StatisticsPvP::PvP_Key must stay 0x08 bytes");
static_assert(sizeof(StatisticsPvP::PvP_Room) == 0x10u,
              "StatisticsPvP::PvP_Room must stay 0x10 bytes");
static_assert(offsetof(StatisticsPvP, room_results_) == 0x18u,
              "StatisticsPvP result map must stay at +0x18");
static_assert(sizeof(StatisticsPvP) == 0x30u,
              "StatisticsPvP service must stay 0x30 bytes");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_STATISTICS_PVP_RUNTIME_H_
