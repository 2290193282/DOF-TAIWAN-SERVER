#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_COLLECT_BOX_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_COLLECT_BOX_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stddef.h>
#include <stdint.h>


class Stream;

namespace collect_box_db {

enum {
    // Slot 825 was empty in both frozen 852-entry registries.  Only the
    // dedicated queue-6 task owns the SQL dispatcher; replies use the same
    // numeric slot through DISPATCH_Q.
    kDbSignal = 825,
    kMaximumBeadCount = 15,
    kResultSuccess = 0,
    kResultDatabaseFailure = 1,
    kResultInvalidState = 2,
    kResultPendingRecovery = 3,
    kResultInventoryNotDurable = 4,
    kResultRequest = 0xff,
    kPhaseNone = 0,
    kPhaseReserved = 1,
    kPhaseCommitted = 2,
    kPhaseCancelled = 3,
};

enum Operation {
    kOperationSelect = 1,
    kOperationPrepareAdd = 2,
    kOperationPrepareRemove = 3,
    kOperationPrepareExtend = 4,
    kOperationFinalize = 5,
    kOperationCancel = 6,
};

// Pointer-free queue6/DISPATCH_Q record.  For mutation requests the two
// slot/total pairs are an exact proof of the authoritative DB3 inventory
// image before and after one CollectBox item transition.  `saga_operation`
// remains the immutable PREPARE kind while `operation` advances through the
// FINALIZE/CANCEL protocol.
struct Message {
    int32_t user_idx;
    uint32_t account_id;
    uint32_t charac_no;
    int32_t server_group;
    uint32_t session_token;
    uint32_t request_token;
    uint32_t saga_id;
    uint32_t item_id;
    uint32_t expiry_at;
    uint32_t slot_count_before;
    uint32_t total_count_before;
    uint32_t slot_count_after;
    uint32_t total_count_after;
    uint16_t source_slot;
    uint16_t bead_index;
    uint16_t client_command;
    uint8_t operation;
    uint8_t saga_operation;
    uint8_t result;
    uint8_t phase;
    uint8_t box_id;
    uint8_t unlimited;
    uint8_t bead_count;
    uint8_t client_reply_expected;
    uint8_t reserved[2];
    uint32_t bead_items[kMaximumBeadCount];
};

static_assert(offsetof(Message, bead_items) == 68u &&
                  sizeof(Message) == 128u,
              "CollectBox queue message ABI drift");

class DB_CollectBox : public DBDispatcher {
public:
    DB_CollectBox();
    virtual ~DB_CollectBox();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    static bool Queue(int user_idx, const Message& request);
};

}  // namespace collect_box_db_0828

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_COLLECT_BOX_0828_H_
