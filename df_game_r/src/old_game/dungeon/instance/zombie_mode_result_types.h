#ifndef OLDSRV_OLD_GAME_DUNGEON_INSTANCE_ZOMBIE_MODE_RESULT_TYPES_H_
#define OLDSRV_OLD_GAME_DUNGEON_INSTANCE_ZOMBIE_MODE_RESULT_TYPES_H_

#include <stdint.h>

namespace zombie_mode {

enum {
    // Taiwan outer signal 846.  Inner 26 remains feature-owned until the
    // shared Taiwan DB/Inter maps are wired by the root integration change.
    kTaiwanInnerCode = 26,
    kMaximumServerGroupCount = 18,
    kMaximumSeats = 8,
    kMinimumPlayers = 5,
    kMaximumPlayers = 6,
    kInitialZombieCount = 2,
    kDailyFullRewardLimit = 4,
    kRewardItemId = 10000687,
    kImmunityItemId = 690001565,
    kMaximumCharacterLevel = 85,
    kResultDeliveryContinuationMagic = 0x5a52434c,
    kResultContinuationTelemetryTick = 1,
    kResultContinuationDeliveryTimeout = 2,
};

enum Operation {
    OPERATION_BEGIN_ENTRY = 1,
    OPERATION_SETTLE = 2,
    OPERATION_COMMIT = 3,
    OPERATION_RECOVER = 4,
    // Private queue-6 continuation emitted only by legacy DB_Q signal 828
    // after canonical charac_info/stat/skill apply-and-verify completes.
    OPERATION_FINALIZE = 5,
};

enum Result {
    RESULT_SUCCESS = 0,
    RESULT_NOT_FOUND = 1,
    RESULT_IDENTITY = 2,
    RESULT_POLICY = 3,
    RESULT_STALE_CHARACTER = 4,
    RESULT_DATABASE = 255,
};

enum ClaimState {
    CLAIM_STATE_RESERVED = 0,
    CLAIM_STATE_TARGET_STORED = 1,
    CLAIM_STATE_COMMITTED = 2,
};

enum Role {
    ROLE_HUMAN = 0,
    ROLE_ZOMBIE = 1,
};

enum Outcome {
    OUTCOME_HUMAN_WIN = 0,
    OUTCOME_ZOMBIE_WIN = 1,
    OUTCOME_DRAW = 2,
};

// These are the three absolute legacy rows mutated by canonical rewardExp.
// Persisting their complete target makes a MyISAM retry overwrite the same
// values instead of adding EXP twice after a worker/process crash.
struct CharacterSnapshot {
    uint32_t level;
    uint32_t experience;
    uint32_t remain_sp_first;
    uint32_t remain_sp_second;
    uint32_t remain_sfp_first;
    uint32_t remain_sfp_second;
};

// Pointer-free Taiwan DB_Q/DISPATCH_Q record.  The explicit scalar order is
// identical on the target i386 ABI and on host-side verification builds.
struct Message {
    uint32_t account_id;
    uint32_t charac_no;
    uint32_t round_token_hi;
    uint32_t round_token_lo;
    uint32_t request_token;
    int32_t server_group;
    int32_t reply_user_idx;

    uint32_t damage;
    uint32_t attacks;
    uint32_t serious_attacks;
    uint32_t killed_humans;
    uint32_t killed_zombies;
    uint32_t survival_seconds;
    uint32_t total_score;

    uint32_t reward_item_id;
    uint32_t reward_exp;
    uint32_t delivery_key_hi;
    uint32_t delivery_key_lo;

    CharacterSnapshot baseline;
    CharacterSnapshot target;

    uint16_t seat;
    uint16_t daily_ordinal;
    uint16_t reward_item_count;
    uint8_t operation;
    uint8_t result;
    uint8_t claim_state;
    uint8_t replayed;
    uint8_t role;
    uint8_t outcome;
    uint8_t rank;
    uint8_t limited_reward;
    uint8_t immunity_at_entry;
    uint8_t last_survivor;
    uint8_t telemetry_consistent;
    uint8_t reserved[3];
};

// Pointer-free MainLoop -> DISPATCH_Q continuation.  The round token is the
// room generation; room_index plus generation_token reject pool reuse.
struct ResultDeliveryContinuation {
    uint32_t magic;
    uint32_t round_token_hi;
    uint32_t round_token_lo;
    uint32_t now_ms;
    int32_t room_index;
    uint32_t generation_token;
    uint32_t action;
};

static_assert(sizeof(CharacterSnapshot) == 24u,
              "Zombie result character snapshot must remain 24 bytes");
static_assert(sizeof(Message) == 140u,
              "Zombie result Taiwan inner record must remain 140 bytes");
static_assert(sizeof(ResultDeliveryContinuation) == 28u,
              "Zombie result continuation must remain 28 bytes");

}  // namespace zombie_mode_0828

#endif  // OLDSRV_OLD_GAME_DUNGEON_INSTANCE_ZOMBIE_MODE_RESULT_0828_TYPES_H_
