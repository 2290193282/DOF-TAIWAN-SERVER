#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_EVENT_ALL_USER_GIFT_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_EVENT_ALL_USER_GIFT_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>

namespace event_all_user_gift {

enum {
    // RECONSTRUCTED_0828: slot 837 has no direct DBTask registration and no
    // InterDispatcher registration in the restored 852-entry tables.  It is
    // an internal DB_Q/DISPATCH_Q route, not a client packet id.
    kDbSignal = 837,
    kEventDbHandle = 9,
    kMaximumSelectionGroupCount = 8,
};

enum Operation {
    OPERATION_QUERY_ELIGIBILITY = 1,
    OPERATION_RESERVE_CLAIM = 2,
    OPERATION_RELEASE_CLAIM = 3,
};

enum Result {
    RESULT_DB_ERROR = 1,
    RESULT_UNCLAIMED = 2,
    RESULT_ALREADY_CLAIMED = 3,
    RESULT_RESERVED = 4,
    RESULT_RELEASED = 5,
};

#pragma pack(push, 1)
struct Payload {
    uint32_t account_id;
    uint32_t charac_no;
    int32_t origin_user_idx;
    int32_t server_group;
    uint8_t operation;
    uint8_t result;
    uint8_t job;
    uint8_t selection_count;
    uint16_t selected_indices[kMaximumSelectionGroupCount];
    uint32_t item_ids[kMaximumSelectionGroupCount];
    uint32_t item_counts[kMaximumSelectionGroupCount];
};
#pragma pack(pop)

static_assert(sizeof(Payload) == 100u,
              "0828 all-user-gift DB payload must remain 100 bytes");

class DB_EventAllUserGift : public DBDispatcher {
public:
    DB_EventAllUserGift();
    virtual ~DB_EventAllUserGift();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

void ClearPayload(Payload& payload);

// Eligibility uses the live CUser UID so a stale login reply is discarded by
// the canonical InterDispatcher.  Reserve/release use -1 and carry the
// original UID inside Payload: this lets the result owner release a reservation
// even when the character disconnects before the DB reply arrives.
bool Queue(int dispatch_user_idx, const Payload& payload);

}  // namespace event_all_user_gift_0828

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_EVENT_ALL_USER_GIFT_0828_H_
