#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_EXP_KEEPING_EVENT_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_EXP_KEEPING_EVENT_H_

#include "old_data/db/core/taiwan_interface_db_dispatcher.h"

#include <stdint.h>

class Stream;

namespace exp_keeping {

enum {
    // The restored Taiwan family already owns outer DB/Inter signal 846.
    // Code 19 extends only its internal map and does not move the physical
    // 852-pointer outer tables.
    kTaiwanInnerCode = TaiwanInternalPack::EXP_KEEPING_EVENT,
    kEventId = 262,
    kResetHour = 6,
    kDailyExperienceCap = 30000000,
    kMinimumKeepingLevelExclusive = 1,
    kMinimumRewardLevelExclusive = 35,
    kMaximumCharacterLevel = 85,
    kFemaleFighterJob = 1,
    kMaximumServerGroupCount = 18,
};

enum Operation {
    OPERATION_LOAD = 1,
    OPERATION_ACCUMULATE = 2,
    OPERATION_RESERVE = 3,
    OPERATION_COMMIT = 4,
    // Private queue-6 continuation emitted only by legacy DB_Q signal 828
    // after canonical charac_info/stat/skill apply-and-verify completes.
    OPERATION_FINALIZE = 5,
};

enum Result {
    RESULT_SUCCESS = 0,
    RESULT_EVENT_CLOSED = 1,
    RESULT_NOT_ELIGIBLE = 2,
    RESULT_NOTHING_TO_CLAIM = 3,
    RESULT_PENDING = 4,
    RESULT_STALE_CHARACTER = 5,
    RESULT_DATABASE_FAILURE = 255,
};

enum ClaimState {
    CLAIM_STATE_NONE = 0,
    CLAIM_STATE_RESERVED = 1,
    CLAIM_STATE_TARGET_STORED = 2,
    CLAIM_STATE_COMMITTED = 3,
};

// Avoid native uint64_t alignment in this cross-thread record.  The target
// i386 ABI and host-side verification builds therefore share one fixed shape.
struct SplitToken {
    uint32_t low;
    uint32_t high;
};

// The three legacy tables touched by an EXP level-up are MyISAM.  Keeping
// their scalar result as an absolute snapshot lets the DB worker replay a
// partially applied claim deterministically before acknowledging CMD594.
struct CharacterSnapshot {
    uint32_t level;
    uint32_t experience;
    uint32_t remain_sp_first;
    uint32_t remain_sp_second;
    uint32_t remain_sfp_first;
    uint32_t remain_sfp_second;
};

struct Message {
    uint32_t account_id;
    uint32_t charac_no;
    uint32_t session_token;
    uint32_t request_token;
    int32_t server_group;
    uint32_t event_id;
    uint32_t activity_day;
    uint32_t kept_experience;
    uint32_t source_experience;
    uint32_t source_dungeon;
    SplitToken source_token;
    SplitToken claim_token;
    CharacterSnapshot baseline;
    CharacterSnapshot target;
    uint8_t operation;
    uint8_t result;
    uint8_t reward_available;
    uint8_t claim_state;
    uint8_t replayed;
    // Level at the authoritative monster producer immediately before the
    // ordinary EXP mutation.  This preserves the valid level-84 kill that
    // itself advances the fighter to the 85 cap.
    uint8_t source_level;
    uint8_t reserved[2];
};

static_assert(sizeof(SplitToken) == 8u,
              "CMD594 split token must remain eight bytes");
static_assert(sizeof(CharacterSnapshot) == 24u,
              "CMD594 character snapshot must remain 24 bytes");
static_assert(sizeof(Message) == 112u,
              "CMD594 Taiwan inner payload must remain 112 bytes");

class DBExpKeepingEvent : public TaiwanInterfaceDBDispatcher {
public:
    DBExpKeepingEvent();
    virtual ~DBExpKeepingEvent();

    virtual int dispatch(int signal, int user_idx, Stream* stream);

    static bool QueueRequest(int user_idx, const Message& request);
};

#if defined(__i386__)
static_assert(sizeof(DBExpKeepingEvent) == 0x08u,
              "CMD594 Taiwan DB owner must preserve the base-only ABI");
#endif

}  // namespace exp_keeping_0828

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_EXP_KEEPING_EVENT_0828_H_
