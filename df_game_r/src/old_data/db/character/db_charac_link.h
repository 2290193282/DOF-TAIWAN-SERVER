#ifndef OLDSRV_OLD_DATA_DB_CHARACTER_DB_CHARAC_LINK_H_
#define OLDSRV_OLD_DATA_DB_CHARACTER_DB_CHARAC_LINK_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stddef.h>
#include <stdint.h>


namespace charac_link_db {

// Signal 824 was empty in both frozen 852-entry tables.  Its producer and DB
// consumer belong exclusively to NATIVE_0828_DB_Q; only the reply is routed
// through the existing DISPATCH_Q / InterDispatcher envelope.
enum {
    kDbSignal = 824,
    kOperationLoad = 1,
    kOperationConnect = 2,
    kOperationDisconnect = 3,
    kResultOk = 0,
    kResultFailed = 1,
    kResultRequest = 0xff,
    kMaximumLinkRecords = 36,
};

struct LinkRecord {
    uint32_t charac_no;
    uint32_t link_charac_no;
};

// Pointer-free request/reply ABI.  The outer stream also carries user_idx;
// retaining it here lets the Inter owner reject a delayed reply delivered to
// a recycled CUser slot.  session_token identifies the current character-list
// session and request_token identifies its one in-flight operation.
struct Message {
    int32_t user_idx;
    uint32_t account_id;
    uint32_t first_charac_no;
    uint32_t second_charac_no;
    int32_t server_group;
    uint32_t session_token;
    uint32_t request_token;
    uint8_t operation;
    uint8_t result;
    uint8_t first_slot;
    uint8_t second_slot;
    uint8_t first_grow_type;
    uint8_t second_grow_type;
    uint8_t record_count;
    uint8_t reserved;
    LinkRecord records[kMaximumLinkRecords];
};

static_assert(sizeof(LinkRecord) == 8u,
              "character-link record must remain an eight-byte ABI");
static_assert(offsetof(Message, records) == 36u &&
                  sizeof(Message) == 324u,
              "character-link DB message ABI drift");

class DB_CharacLink : public DBDispatcher {
public:
    DB_CharacLink();
    virtual ~DB_CharacLink();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    static bool Queue(int user_idx, const Message& request);
};

}  // namespace charac_link_db_0828

#endif  // OLDSRV_OLD_DATA_DB_CHARACTER_DB_CHARAC_LINK_0828_H_
