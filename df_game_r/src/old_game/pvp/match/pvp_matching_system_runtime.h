#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PVP_MATCHING_SYSTEM_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PVP_MATCHING_SYSTEM_RUNTIME_H_

#include "old_game/pvp/match/fair_pvp_match_interface.h"

// IDA unit: 99_misc_global/CMatchingSystem.  These six values select the
// fixed six-entry owner table; their ordinal is the table index in the old
// code at 0x85DFBA0.
enum ENUM_PVP_MATCHING_TYPE : int {
    ENUM_PVP_MATCHING_NORMAL = 0,
    ENUM_PVP_MATCHING_LADDER = 1,
    ENUM_PVP_MATCHING_LEAGUE = 2,
    ENUM_PVP_MATCHING_TOURNAMENT = 3,
    ENUM_PVP_MATCHING_ONLINE_PRELIMINARY = 4,
    ENUM_PVP_MATCHING_FAIR_PVP = 5
};

class CMatchingSystem {
public:
    // IDA_EA: 0x85DFAC6
    CMatchingSystem();
    // IDA_EA: 0x85DFAF4
    ~CMatchingSystem();
    // IDA_EA: 0x85DFBA0
    IMatch* GetMatchingSystem(ENUM_PVP_MATCHING_TYPE type);

private:
    IMatch* matching_systems_[6];
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CMatchingSystem) == 0x18u,
              "CMatchingSystem IDA ABI size must stay 0x18");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PVP_MATCHING_SYSTEM_RUNTIME_H_
