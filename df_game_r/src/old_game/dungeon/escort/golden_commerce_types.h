#ifndef OLDSRV_OLD_GAME_DUNGEON_ESCORT_GOLDEN_COMMERCE_TYPES_H_
#define OLDSRV_OLD_GAME_DUNGEON_ESCORT_GOLDEN_COMMERCE_TYPES_H_

#include <stdint.h>

namespace golden_commerce {

enum {
    // Taiwan outer signal 846.  Inner 27 is feature-owned until the root
    // integration installs the concrete DB and Inter dispatchers together.
    kTaiwanInnerCode = 27,
    kMaximumServerGroupCount = 18,
    kOfficialDungeonWest = 3054,
    kOfficialDungeonSouth = 3056,
    kOfficialDungeonEast = 3057,
    kOfficialCartWest = 61666,
    kOfficialCartSouth = 61664,
    kOfficialCartEast = 61667,
    kMaximumRewardGold = 10000000,
};

enum RewardOperation {
    REWARD_OPERATION_CLAIM = 1,
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

inline uint32_t OfficialCartForDungeon(int dungeon_index) {
    switch (dungeon_index) {
        case kOfficialDungeonWest: return kOfficialCartWest;
        case kOfficialDungeonSouth: return kOfficialCartSouth;
        case kOfficialDungeonEast: return kOfficialCartEast;
        default: return 0u;
    }
}

inline uint32_t OfficialCargoBaseValue(uint32_t item_id) {
    switch (item_id) {
        case 10000910u: return 50000u;
        case 10000911u: return 100000u;
        case 10000912u: return 1000000u;
        case 10000913u: return 10000000u;
        case 10001574u: return 100000u;
        case 10001575u: return 200000u;
        default: return 0u;
    }
}

inline bool IsCargoAllowedInDungeon(int dungeon_index, uint32_t item_id) {
    switch (dungeon_index) {
        case kOfficialDungeonWest:
            return item_id == 10000910u || item_id == 10000911u;
        case kOfficialDungeonSouth:
            return item_id == 10001574u || item_id == 10000911u ||
                   item_id == 10000912u;
        case kOfficialDungeonEast:
            return item_id == 10001575u || item_id == 10000911u ||
                   item_id == 10000912u || item_id == 10000913u;
        default:
            return false;
    }
}

inline uint8_t RankIndexForPercent(uint8_t percent) {
    if (percent >= 91u) return 0u;
    if (percent >= 61u) return 1u;
    if (percent >= 47u) return 2u;
    if (percent >= 36u) return 3u;
    if (percent >= 11u) return 4u;
    return 5u;
}

inline uint32_t RewardGoldFor(uint32_t base_value, uint8_t percent) {
    return static_cast<uint32_t>(
        (static_cast<uint64_t>(base_value) * percent) / 100u);
}

inline uint32_t ResultTokenFor(uint32_t run_token_lo,
                               uint32_t charac_no,
                               uint32_t cargo_item_id,
                               uint8_t percent) {
    uint32_t value = run_token_lo ^ charac_no ^ cargo_item_id ^
                     (static_cast<uint32_t>(percent) << 24u) ^
                     0x47434D52u;  // "GCMR"
    return value == 0u ? 1u : value;
}

// Pointer-free DB_Q/DISPATCH_Q record.  All reward inputs are captured by
// the authoritative game state; the DB owner independently revalidates the
// official dungeon/cart/cargo matrix and the integer payout formula.
struct RewardClaimMessage {
    uint32_t account_id;
    uint32_t charac_no;
    uint32_t run_token_hi;
    uint32_t run_token_lo;
    uint32_t request_token;
    uint32_t cargo_item_id;
    uint32_t cargo_base_value;
    uint32_t reward_gold;
    uint32_t server_result_token;
    uint32_t cart_monster_id;
    int32_t dungeon_index;
    int32_t server_group;
    int32_t reply_user_idx;
    uint16_t member_unique_id;
    uint16_t cart_unique_id;
    uint8_t operation;
    uint8_t authoritative_percent;
    uint8_t rank_index;
    uint8_t result_code;
    uint8_t claim_state;
    uint8_t replayed;
    uint8_t reserved[2];
    uint32_t delivery_key_hi;
    uint32_t delivery_key_lo;
};

static_assert(sizeof(RewardClaimMessage) == 72u,
              "Golden Commerce Taiwan payload must remain 72 bytes");

}  // namespace golden_commerce_0828

#endif  // OLDSRV_OLD_GAME_DUNGEON_ESCORT_GOLDEN_COMMERCE_0828_TYPES_H_
