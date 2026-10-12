#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_ONLINE_PRELIMINARY_MATCH_TYPES_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_ONLINE_PRELIMINARY_MATCH_TYPES_H_

#include <stddef.h>
#include <stdint.h>

class CUser;

// IDA unit: 99_misc_global/PlayerInfoSecond.
// COnlinePreliminary stores this exact record four times in each team block.
// The old reset body writes only user (+0x00) and occupied (+0x04), leaving
// the three tail padding bytes untouched.
struct PlayerInfoSecond {
    CUser* user;       // IDA +0x00
    uint8_t occupied;  // IDA +0x04
    uint8_t padding_05_07[3];

    // IDA_EA: 0x8568850
    // IDA_NAME: _ZN16PlayerInfoSecond5resetEv
    // DEMANGLED: PlayerInfoSecond::reset(void)
    PlayerInfoSecond* reset();
};

// IDA unit: 99_misc_global/TeamInfoSecond.
// This is the real 0x28-byte team member embedded at COnlinePreliminary
// +0x1c and +0x44. It carries an online-preliminary team id, its current
// Elo rating, and four explicitly occupied player slots.
struct TeamInfoSecond {
    int32_t online_preliminary_team_id;  // IDA +0x00
    float rating;                        // IDA +0x04
    PlayerInfoSecond players[4];         // IDA +0x08

    // IDA_EA: 0x855FCF6
    // IDA_NAME: _ZN14TeamInfoSecond7refreshEv
    // DEMANGLED: TeamInfoSecond::refresh(void)
    void refresh();

    // IDA_EA: 0x855FDC0
    // IDA_NAME: _ZN14TeamInfoSecond5resetEv
    // DEMANGLED: TeamInfoSecond::reset(void)
    bool reset();

    // IDA_EA: 0x855FDFE
    // IDA_NAME: _ZN14TeamInfoSecond3addEP5CUser
    // DEMANGLED: TeamInfoSecond::add(CUser *)
    int add(CUser* user);

    // IDA_EA: 0x855FE54
    // IDA_NAME: _ZN14TeamInfoSecond3delEP5CUser
    // DEMANGLED: TeamInfoSecond::del(CUser *)
    int del(CUser* user);

    // IDA_EA: 0x855FEAA
    // IDA_NAME: _ZNK14TeamInfoSecond4sizeEv
    // DEMANGLED: TeamInfoSecond::size(void)const
    int size() const;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(PlayerInfoSecond) == 0x08u,
              "PlayerInfoSecond IDA ABI size must stay 0x08");
static_assert(offsetof(PlayerInfoSecond, occupied) == 0x04u,
              "PlayerInfoSecond::occupied must stay at +0x04");
static_assert(sizeof(TeamInfoSecond) == 0x28u,
              "TeamInfoSecond IDA ABI size must stay 0x28");
static_assert(offsetof(TeamInfoSecond, rating) == 0x04u,
              "TeamInfoSecond::rating must stay at +0x04");
static_assert(offsetof(TeamInfoSecond, players) == 0x08u,
              "TeamInfoSecond::players must stay at +0x08");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_ONLINE_PRELIMINARY_MATCH_TYPES_H_
