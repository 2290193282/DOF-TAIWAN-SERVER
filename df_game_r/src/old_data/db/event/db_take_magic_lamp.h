#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_TAKE_MAGIC_LAMP_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_TAKE_MAGIC_LAMP_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>

namespace magic_lamp {

enum {
    // RECONSTRUCTED_0828: slot 838 is unused in both restored 852-entry
    // DB/Inter routing tables.  It is an internal compatibility signal, not
    // a client packet id.
    kDbSignal = 838,
    kDbHandle = 9,
    kMaximumServerGroupCount = 18,
};

enum ClaimResult {
    CLAIM_GRANTED = 0,
    CLAIM_RETRY_PENDING = 1,
    CLAIM_PENDING_RECENT = 2,
    CLAIM_ALREADY_SUBMITTED = 3,
    CLAIM_DATABASE_FAILURE = 4,
};

enum ClaimOperation {
    CLAIM_OPERATION_RESERVE = 1,
    CLAIM_OPERATION_MARK_MAIL_PERSISTED = 2,
};

// Fixed DB_Q / DISPATCH_Q payload.  Identity fields are echoed so the game
// thread can reject a result routed to a stale or different character.
struct ClaimMessage {
    uint32_t account_id;
    uint32_t request_charac_no;
    uint32_t reward_charac_no;
    uint32_t reward_item_id;
    int32_t request_server_group;
    int32_t reward_server_group;
    uint8_t operation;
    uint8_t result;
    uint8_t reserved[2];
};

static_assert(sizeof(ClaimMessage) == 28u,
              "CMD723 internal DB claim payload must remain 28 bytes");

class DB_TakeMagicLamp : public DBDispatcher {
public:
    DB_TakeMagicLamp();
    virtual ~DB_TakeMagicLamp();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    static bool makeReserveRequest(int user_idx,
                                   uint32_t account_id,
                                   uint32_t charac_no,
                                   uint32_t reward_item_id,
                                   int server_group);

    static bool makeMarkMailPersistedRequest(uint32_t account_id,
                                             uint32_t reward_charac_no,
                                             uint32_t reward_item_id,
                                             int server_group);
};

}  // namespace magic_lamp_0828

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_TAKE_MAGIC_LAMP_0828_H_
