#ifndef OLDSRV_OLD_GAME_EVENT_TOURNAMENT_REWARD_TYPE_H_
#define OLDSRV_OLD_GAME_EVENT_TOURNAMENT_REWARD_TYPE_H_

#include <stdint.h>

// IDA unit: 09_dungeon_pvp_party/CTournamentDungeonReward.  The archive
// passes 0 and 1 for the two reward rows and 2 for the aggregate query.  The
// enumerator spellings are recovery-local; the named enum itself is required
// by the original member-function ABI.
enum ENUM_TOURNAMENT_REWARD_CARD_TYPE : int {
    ENUM_TOURNAMENT_REWARD_CARD_TYPE_0 = 0,
    ENUM_TOURNAMENT_REWARD_CARD_TYPE_1 = 1,
    ENUM_TOURNAMENT_REWARD_CARD_TYPE_ALL = 2,
};

// IDA unit: 11_event_runtime/TournamentRewardType.  The type participates in
// CBattle_Field's embedded CTournamentDungeonReward layout, so keep it in a
// small cycle-free header shared by the event and dungeon domains.
struct TournamentRewardType {
    // IDA_EA: 0x8285546
    // IDA_NAME: _ZN20TournamentRewardTypeC2Ev
    TournamentRewardType();

    // IDA_EA: 0x8284750
    // IDA_NAME: _ZN20TournamentRewardType5ClearEv
    TournamentRewardType* Clear();

    int32_t first_dword;
    int32_t second_dword;
    int8_t reward_card_type;
    uint8_t reserved_09[3];
};

static_assert(sizeof(TournamentRewardType) == 0x0c,
              "TournamentRewardType IDA ABI size must stay 0x0c");

#endif  // OLDSRV_OLD_GAME_EVENT_TOURNAMENT_REWARD_TYPE_H_
