#ifndef OLDSRV_OLD_DATA_DB_CHARACTER_DB_CLONE_TITLE_H_
#define OLDSRV_OLD_DATA_DB_CHARACTER_DB_CLONE_TITLE_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>


namespace clone_title_db {

// RECONSTRUCTED_0828: slot 822 is unused in both original 852-entry
// DBTask/InterDispatcher tables.  It is private to the clone-title sidecar.
enum {
    kDbSignal = 822,
    kOperationLoad = 1,
    kOperationSave = 2,
    kResultOk = 0,
    kResultFailed = 1,
    kReplyBackground = 0,
    kReplyCommand = 1,
};

// Fixed NATIVE_0828_DB_Q / DISPATCH_Q payload.  Account and character
// identity prevent a delayed response from being applied to a recycled CUser
// session.
struct Message {
    uint32_t account_id;
    uint32_t charac_no;
    uint32_t item_id;
    int32_t server_group;
    uint8_t operation;
    uint8_t result;
    uint8_t reply_kind;
    uint8_t reserved;
};

static_assert(sizeof(Message) == 20u,
              "clone-title DB message must remain a fixed 20-byte ABI");

class DB_CloneTitle : public DBDispatcher {
public:
    DB_CloneTitle();
    virtual ~DB_CloneTitle();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    static bool Queue(int user_idx, const Message& request);
};

}  // namespace clone_title_db_0828

#endif  // OLDSRV_OLD_DATA_DB_CHARACTER_DB_CLONE_TITLE_0828_H_
