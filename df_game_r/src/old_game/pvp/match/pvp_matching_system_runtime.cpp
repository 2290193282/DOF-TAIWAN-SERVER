#include "old_game/pvp/match/pvp_matching_system_runtime.h"

#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/pvp/match/fair_pvp_match_runtime.h"
#include "old_game/pvp/arena/fair_pvp_waiting_room_runtime.h"
#include "old_game/pvp/match/league_match_static_state_runtime.h"
#include "old_game/pvp/match/league_match_room_list_runtime.h"
#include "old_game/pvp/match/online_preliminary_match_runtime.h"

namespace {

// The three small families have no instance members beyond IMatch in the old
// IDB (each allocation is exactly four bytes).  Their distinct virtual
// destructors are retained so CMatchingSystem's D2 virtual-delete loop uses
// the original object ownership rule.
class CNormalMatch : public IMatch {
public:
    CNormalMatch() : IMatch() {}
    virtual ~CNormalMatch() {}
};

class CLadderMatch : public IMatch {
public:
    CLadderMatch() : IMatch() {}
    virtual ~CLadderMatch() {}
};

class CTournamentMatch : public IMatch {
public:
    CTournamentMatch() : IMatch() {}
    virtual ~CTournamentMatch() {}
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CNormalMatch) == 0x04u,
              "CNormalMatch IDA allocation must stay 0x04");
static_assert(sizeof(CLadderMatch) == 0x04u,
              "CLadderMatch IDA allocation must stay 0x04");
static_assert(sizeof(CTournamentMatch) == 0x04u,
              "CTournamentMatch IDA allocation must stay 0x04");
#endif

}  // namespace

// IDA_EA: 0x85DFAC6
// IDA_NAME: _ZN15CMatchingSystemC2Ev
CMatchingSystem::CMatchingSystem()
    : matching_systems_{0, 0, 0, 0, 0, 0} {
}

// IDA_EA: 0x85DFAF4
// IDA_NAME: _ZN15CMatchingSystemD2Ev
CMatchingSystem::~CMatchingSystem() {
    for (int index = 0; index != 6; ++index) {
        delete matching_systems_[index];
    }
}

// IDA_EA: 0x85DFBA0
// IDA_NAME: _ZN15CMatchingSystem17GetMatchingSystemE22ENUM_PVP_MATCHING_TYPE
IMatch* CMatchingSystem::GetMatchingSystem(ENUM_PVP_MATCHING_TYPE type) {
    if (type < ENUM_PVP_MATCHING_NORMAL || type > ENUM_PVP_MATCHING_FAIR_PVP) {
        return 0;
    }

    IMatch*& match = matching_systems_[static_cast<int>(type)];
    if (match) {
        return match;
    }

    switch (type) {
    case ENUM_PVP_MATCHING_NORMAL:
        match = new CNormalMatch();
        break;
    case ENUM_PVP_MATCHING_LADDER:
        match = new CLadderMatch();
        break;
    case ENUM_PVP_MATCHING_LEAGUE:
        // The target's case 2 allocates the complete 0xfc CLeagueMatch;
        // keep its true vtable, static waiting-room ownership and team
        // storage rather than a lifecycle-only surrogate.
        match = new CLeagueMatch();
        break;
    case ENUM_PVP_MATCHING_TOURNAMENT:
        match = new CTournamentMatch();
        break;
    case ENUM_PVP_MATCHING_ONLINE_PRELIMINARY:
        match = new online_preliminary::COnlinePreliminary();
        break;
    case ENUM_PVP_MATCHING_FAIR_PVP:
        match = new fair_pvp::CFairMatch();
        break;
    }
    return match;
}
