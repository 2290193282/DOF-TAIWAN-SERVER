#ifndef OLDSRV_OLD_GAME_DUNGEON_RAID_DUAL_RAID_TYPES_H_
#define OLDSRV_OLD_GAME_DUNGEON_RAID_DUAL_RAID_TYPES_H_

#include <stdint.h>

namespace dual_raid {

enum {
    // Taiwan outer signal 846.  Inner 23 is deliberately feature-owned until
    // the root integration change registers both DB and Inter owners.
    kTaiwanInnerCode = 23,
    kMaximumServerGroupCount = 18,
    kTeamCount = 2,
    kMembersPerTeam = 3,
    kOfficialRewardItemId = 10000373,
    kOfficialClearRewardCount = 3,
    kOfficialFailRewardCount = 1,
    kMaximumRewardExp = 1000000,
};

enum RewardOperation {
    REWARD_OPERATION_CLAIM = 1,
};

enum RewardResult {
    REWARD_RESULT_SUCCESS = 0,
    REWARD_RESULT_ALREADY_DELIVERED = 1,
    REWARD_RESULT_IDENTITY = 2,
    REWARD_RESULT_STATE = 3,
    REWARD_RESULT_POLICY = 4,
    REWARD_RESULT_DATABASE = 255,
};

// Fixed DB_Q/DISPATCH_Q record.  It contains no pointers and uses split raid
// token words so its layout is identical on the legacy i386 target and on a
// host-side verifier.
struct RewardClaimMessage {
    uint32_t account_id;
    uint32_t charac_no;
    uint32_t raid_token_hi;
    uint32_t raid_token_lo;
    uint32_t request_token;
    uint32_t reward_item_id;
    uint32_t reward_item_count;
    uint32_t reward_exp;
    int32_t server_group;
    int32_t reply_user_idx;
    uint32_t delivery_key_hi;
    uint32_t delivery_key_lo;
    uint8_t operation;
    uint8_t team_index;
    uint8_t clear_result;
    uint8_t result_code;
    uint8_t reserved[4];
};

static_assert(sizeof(RewardClaimMessage) == 56u,
              "Dual Raid reward queue record must remain 56 bytes");

}  // namespace dual_raid_0828

#endif  // OLDSRV_OLD_GAME_DUNGEON_RAID_DUAL_RAID_0828_TYPES_H_
