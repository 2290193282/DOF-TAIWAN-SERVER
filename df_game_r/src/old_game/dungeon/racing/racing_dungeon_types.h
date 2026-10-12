#ifndef OLDSRV_OLD_GAME_DUNGEON_RACING_RACING_DUNGEON_TYPES_H_
#define OLDSRV_OLD_GAME_DUNGEON_RACING_RACING_DUNGEON_TYPES_H_

#include <stdint.h>

namespace racing_dungeon {

enum {
    // Taiwan outer signal 846.  Inner 24 remains feature-owned until the
    // root integration change installs both concrete dispatcher objects.
    kTaiwanInnerCode = 24,
    kMaximumServerGroupCount = 18,
    kMaximumRacers = 4,
    kMinimumRacers = 3,
    // 0828 NOTI466 handler sub_581480 passes 0xBEF to the ordinary
    // select-dungeon path.  The dormant official DNFRider.dgn also binds
    // each of its four maps back to dungeon 3055.
    kOfficialDungeonIndex = 3055,
    kOfficialRewardItemId = 2749803,
    kOfficialInvitationItemId = 2682825,
    kOfficialRequiredFatigue = 1,
    kOfficialEntryConditionCount = 20,
    kMaximumFinishTimeMs = 60 * 60 * 1000,
};

enum RewardOperation {
    REWARD_OPERATION_RESERVE_ENTRY = 1,
    REWARD_OPERATION_CANCEL_ENTRY = 2,
    REWARD_OPERATION_CLAIM = 3,
};

enum RewardResult {
    REWARD_RESULT_SUCCESS = 0,
    REWARD_RESULT_ALREADY_DELIVERED = 1,
    REWARD_RESULT_IDENTITY = 2,
    REWARD_RESULT_POLICY = 3,
    REWARD_RESULT_COLLISION = 4,
    REWARD_RESULT_DATABASE = 255,
};

enum RewardClaimState {
    REWARD_CLAIM_PENDING = 0,
    REWARD_CLAIM_DELIVERED = 1,
};

// Pointer-free DB_Q/DISPATCH_Q record.  The split session/delivery words and
// explicit tail make the record identical on the legacy i386 target and in
// host-side wire verification builds.
struct RewardClaimMessage {
    uint32_t account_id;
    uint32_t charac_no;
    uint32_t session_token_hi;
    uint32_t session_token_lo;
    uint32_t entry_token_hi;
    uint32_t entry_token_lo;
    uint32_t request_token;
    uint32_t finish_time_ms;
    uint32_t reward_item_id;
    int32_t server_group;
    int32_t reply_user_idx;
    uint32_t delivery_key_hi;
    uint32_t delivery_key_lo;
    uint32_t activity_day;
    uint16_t entry_ordinal;
    uint16_t member_unique_id;
    uint8_t operation;
    uint8_t finish_rank;
    uint8_t reward_count;
    uint8_t result_code;
    uint8_t claim_state;
    uint8_t replayed;
    uint8_t reserved[2];
};

static_assert(sizeof(RewardClaimMessage) == 68u,
              "Racing Taiwan payload must remain 68 bytes");

}  // namespace racing_dungeon_0828

#endif  // OLDSRV_OLD_GAME_DUNGEON_RACING_RACING_DUNGEON_0828_TYPES_H_
