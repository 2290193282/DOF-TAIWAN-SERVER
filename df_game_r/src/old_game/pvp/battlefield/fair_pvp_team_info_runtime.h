#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_FAIR_PVP_TEAM_INFO_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_FAIR_PVP_TEAM_INFO_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>

#include "old_data/PVF/pvp_mission/fair_pvp_stat_script.h"

class CUser;
class PacketGuard;

namespace fair_pvp {

// IDA unit: 09_dungeon_pvp_party/fair_pvp/PlayerInfoThird.
// The two-byte CharacterKind is deliberately retained between the presence
// marker and the grade.  CFairMatch reads these records directly while it
// validates rematch teams.
struct PlayerInfoThird {
    CUser* user;                  // +0x00
    bool present;                 // +0x04
    CharacterKind character_kind; // +0x05
    int pvp_grade;                // +0x08
    float rating;                 // +0x0c

    // IDA_EA: 0x8568FDA
    // IDA_NAME: _ZN8fair_pvp15PlayerInfoThirdC2Ev
    // DEMANGLED: fair_pvp::PlayerInfoThird::PlayerInfoThird(void)
    PlayerInfoThird();

    // IDA_EA: 0x8568900
    // IDA_NAME: _ZN8fair_pvp15PlayerInfoThird5resetEv
    // DEMANGLED: fair_pvp::PlayerInfoThird::reset(void)
    PlayerInfoThird* reset();
};

// IDA unit: 09_dungeon_pvp_party/fair_pvp/TeamInfoThird.
// The leading rematch flag/state are reset by the member functions, but the
// original constructor only constructs the four embedded player records.
struct TeamInfoThird {
    bool rematch_requested;       // +0x00
    int rematch_state;            // +0x04
    PlayerInfoThird players[4];   // +0x08

    // IDA_EA: 0x8568FF0
    // IDA_NAME: _ZN8fair_pvp13TeamInfoThirdC2Ev
    // DEMANGLED: fair_pvp::TeamInfoThird::TeamInfoThird(void)
    TeamInfoThird();

    // IDA_EA: 0x85624DE
    // IDA_NAME: _ZN8fair_pvp13TeamInfoThird7refreshEv
    // DEMANGLED: fair_pvp::TeamInfoThird::refresh(void)
    TeamInfoThird* refresh();

    // IDA_EA: 0x8562582
    // IDA_NAME: _ZN8fair_pvp13TeamInfoThird5resetEv
    // DEMANGLED: fair_pvp::TeamInfoThird::reset(void)
    TeamInfoThird* reset();

    // IDA_EA: 0x85625C6
    // IDA_NAME: _ZN8fair_pvp13TeamInfoThird3addEP5CUser
    // DEMANGLED: fair_pvp::TeamInfoThird::add(CUser *)
    int add(CUser* user);

    // IDA_EA: 0x85626AA
    // IDA_NAME: _ZN8fair_pvp13TeamInfoThird3delEP5CUser
    // DEMANGLED: fair_pvp::TeamInfoThird::del(CUser *)
    int del(CUser* user);

    // IDA_EA: 0x856270C
    // IDA_NAME: _ZNK8fair_pvp13TeamInfoThird4sizeEv
    // DEMANGLED: fair_pvp::TeamInfoThird::size(void)const
    int size() const;

    // IDA_EA: 0x856274C
    // IDA_NAME: _ZN8fair_pvp13TeamInfoThird10SendToTeamER11PacketGuard
    // DEMANGLED: fair_pvp::TeamInfoThird::SendToTeam(PacketGuard &)
    bool SendToTeam(PacketGuard& packet);

    // IDA_EA: 0x85627B2
    // IDA_NAME: _ZN8fair_pvp13TeamInfoThird13GetTeamRatingEv
    // DEMANGLED: fair_pvp::TeamInfoThird::GetTeamRating(void)
    float GetTeamRating();
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(PlayerInfoThird) == 0x10u,
              "PlayerInfoThird IDA ABI size must stay 0x10");
static_assert(offsetof(PlayerInfoThird, character_kind) == 0x05u,
              "PlayerInfoThird::character_kind must stay at +0x05");
static_assert(offsetof(PlayerInfoThird, pvp_grade) == 0x08u,
              "PlayerInfoThird::pvp_grade must stay at +0x08");
static_assert(sizeof(TeamInfoThird) == 0x48u,
              "TeamInfoThird IDA ABI size must stay 0x48");
static_assert(offsetof(TeamInfoThird, players) == 0x08u,
              "TeamInfoThird::players must stay at +0x08");
#endif

}  // namespace fair_pvp

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_FAIR_PVP_TEAM_INFO_RUNTIME_H_
