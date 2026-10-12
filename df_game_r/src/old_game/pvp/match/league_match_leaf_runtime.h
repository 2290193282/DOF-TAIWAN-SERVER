#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_LEAGUE_MATCH_LEAF_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_LEAGUE_MATCH_LEAF_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>

class CUser;

// IDA only exposes the numeric values consumed by CLeagueMatch::addBadge:
// green = 0, red = 1, blue = 2.  The fixed underlying type is part of the
// CUser badge ABI.
enum ENUM_BADGE_TYPE : int {
    ENUM_BADGE_TYPE_GREEN = 0,
    ENUM_BADGE_TYPE_RED = 1,
    ENUM_BADGE_TYPE_BLUE = 2,
};

// IDA unit: 99_misc_global/PlayerInfo.
// The reset body deliberately leaves both padding regions untouched.
struct PlayerInfo {
    uint32_t account_id;          // IDA +0x00
    float rating;                 // IDA +0x04
    CUser* user;                  // IDA +0x08
    uint8_t active;               // IDA +0x0c
    uint8_t padding_0d_0f[3];
    uint32_t school_no;           // IDA +0x10
    int16_t daily_badges[3];      // IDA +0x14
    uint8_t padding_1a_1b[2];

    // IDA_EA: 0x85687D4
    // IDA_NAME: _ZN10PlayerInfo5resetEv
    // DEMANGLED: PlayerInfo::reset(void)
    bool reset();
};

// IDA_EA: 0x855C952
// IDA_NAME: _Z13CompareRatingRK10PlayerInfoS1_
// DEMANGLED: CompareRating(PlayerInfo const&,PlayerInfo const&)
bool CompareRating(const PlayerInfo& left, const PlayerInfo& right);

// IDA unit: 99_misc_global/TeamInfo.
// CLeagueMatch embeds one instance at +0x1c and another at +0x8c.
struct TeamInfo {
    PlayerInfo players[4];

    // IDA_EA: 0x855C9AE
    // IDA_NAME: _ZN8TeamInfo7refreshEv
    // DEMANGLED: TeamInfo::refresh(void)
    bool refresh();

    // IDA_EA: 0x855CB16
    // IDA_NAME: _ZN8TeamInfo5resetEv
    // DEMANGLED: TeamInfo::reset(void)
    bool reset();

    // IDA_EA: 0x855CB54
    // IDA_NAME: _ZN8TeamInfo3addEP5CUser
    // DEMANGLED: TeamInfo::add(CUser *)
    int add(CUser* user);

    // IDA_EA: 0x855CC9C
    // IDA_NAME: _ZN8TeamInfo3delEP5CUser
    // DEMANGLED: TeamInfo::del(CUser *)
    int del(CUser* user);

    // IDA_EA: 0x855CD26
    // IDA_NAME: _ZNK8TeamInfo4sizeEv
    // DEMANGLED: TeamInfo::size(void)const
    int size() const;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(PlayerInfo) == 0x1cu,
              "PlayerInfo IDA ABI size must stay 0x1c");
static_assert(offsetof(PlayerInfo, rating) == 0x04u,
              "PlayerInfo::rating must stay at +0x04");
static_assert(offsetof(PlayerInfo, user) == 0x08u,
              "PlayerInfo::user must stay at +0x08");
static_assert(offsetof(PlayerInfo, active) == 0x0cu,
              "PlayerInfo::active must stay at +0x0c");
static_assert(offsetof(PlayerInfo, school_no) == 0x10u,
              "PlayerInfo::school_no must stay at +0x10");
static_assert(offsetof(PlayerInfo, daily_badges) == 0x14u,
              "PlayerInfo::daily_badges must stay at +0x14");
static_assert(sizeof(TeamInfo) == 0x70u,
              "TeamInfo IDA ABI size must stay 0x70");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_LEAGUE_MATCH_LEAF_RUNTIME_H_
