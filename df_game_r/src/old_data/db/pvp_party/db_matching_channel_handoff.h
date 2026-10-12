#ifndef OLDSRV_OLD_DATA_DB_PVP_PARTY_DB_MATCHING_CHANNEL_HANDOFF_H_
#define OLDSRV_OLD_DATA_DB_PVP_PARTY_DB_MATCHING_CHANNEL_HANDOFF_H_

#include "old_data/db/core/taiwan_interface_db_dispatcher.h"

#include <stddef.h>
#include <stdint.h>

class Stream;

namespace matching_channel_handoff_db {

enum {
    // Feature-owned Taiwan extension.  Shared enum wiring names value 25;
    // retaining the literal here lets the feature TU verify before that
    // shared registry edit is applied.
    kTaiwanInnerCode = 25,
    kMaximumServerGroupCount = 18,
    kTicketLifetimeSeconds = 90,
    // Official policy is 5s ready + 300s play.  Keep 115s for settlement,
    // room reset and DB-thread completion after OPERATION_ACTIVATE_COHORT.
    kActiveCohortLifetimeSeconds = 420,
    kChannelMatchingBound = 1,
    kZombieMatchingMode = 15,
};

enum Operation {
    OPERATION_RESERVE = 1,
    OPERATION_ARM = 2,
    OPERATION_CLAIM = 3,
    OPERATION_CANCEL = 4,
    // 140828 project-owned cohort lifecycle.  These values are not claimed
    // to be surviving official inner operations; they extend the private
    // Taiwan inner-25 contract used by this reconstruction.
    OPERATION_ROUTE_COHORT = 5,
    OPERATION_ABORT_COHORT = 6,
    OPERATION_ACTIVATE_COHORT = 7,
    OPERATION_COMPLETE_COHORT = 8,
};

enum CohortFlag {
    COHORT_FLAG_ALL_RESERVED = 1u << 0,
    COHORT_FLAG_ALL_ARMED = 1u << 1,
    COHORT_FLAG_ALL_CLAIMED = 1u << 2,
    COHORT_FLAG_ALL = COHORT_FLAG_ALL_RESERVED |
                      COHORT_FLAG_ALL_ARMED |
                      COHORT_FLAG_ALL_CLAIMED,
};

enum State {
    STATE_NONE = 0,
    STATE_RESERVED = 1,
    STATE_ARMED = 2,
    STATE_CLAIMED = 3,
    STATE_CANCELED = 4,
    STATE_EXPIRED = 5,
};

enum Result {
    RESULT_SUCCESS = 0,
    RESULT_BUSY = 1,
    RESULT_STALE = 2,
    RESULT_WRONG_TARGET = 3,
    RESULT_REPLAY = 4,
    RESULT_NOT_OWNER = 5,
    RESULT_DATABASE_FAILURE = 255,
};

// Fixed Taiwan outer-846/inner-25 record.  The five NOTI606 fields are kept
// verbatim.  For the 140828 project-owned extension, field1 is the durable
// cohort_id allocated by OPERATION_ROUTE_COHORT.  CMD521 still echoes only
// field0 and field4; target identity and the remaining writer fields are
// server-side authorization evidence.
struct Message {
    uint32_t account_id;
    uint32_t charac_no;
    uint32_t session_token;
    uint32_t request_token;
    uint32_t handoff_id;
    uint32_t field1;
    uint32_t endpoint_key;
    uint32_t endpoint_port_like;
    uint32_t field4;
    int32_t source_server_group;
    int32_t source_channel;
    int32_t target_server_group;
    int32_t target_channel;
    uint8_t field0;
    uint8_t matching_bound;
    uint16_t matching_mode;
    uint8_t operation;
    uint8_t result;
    uint8_t state;
    uint8_t source_cleaned;
    uint8_t reply_required;
    uint8_t cohort_slot;   // +0x3d, zero based within the 5/6-member cohort
    uint8_t cohort_size;   // +0x3e, exactly 5 or 6 once a cohort is allocated
    uint8_t cohort_flags;  // +0x3f, CohortFlag barrier snapshot
};

static_assert(sizeof(Message) == 64u,
              "matching channel Taiwan payload must remain 64 bytes");
static_assert(offsetof(Message, cohort_slot) == 0x3du,
              "cohort_slot must remain at Message+0x3d");
static_assert(offsetof(Message, cohort_size) == 0x3eu,
              "cohort_size must remain at Message+0x3e");
static_assert(offsetof(Message, cohort_flags) == 0x3fu,
              "cohort_flags must remain at Message+0x3f");

class DBMatchingChannelHandoff : public TaiwanInterfaceDBDispatcher {
public:
    DBMatchingChannelHandoff();
    virtual ~DBMatchingChannelHandoff();

    virtual int dispatch(int signal, int user_idx, Stream* stream);

    static bool QueueRequest(int user_idx, const Message& request);
};

#if defined(__i386__)
static_assert(sizeof(DBMatchingChannelHandoff) == 0x08u,
              "matching handoff DB owner must preserve base-only ABI");
#endif

}  // namespace matching_channel_handoff_db

#endif  // OLDSRV_OLD_DATA_DB_PVP_PARTY_DB_MATCHING_CHANNEL_HANDOFF_H_
