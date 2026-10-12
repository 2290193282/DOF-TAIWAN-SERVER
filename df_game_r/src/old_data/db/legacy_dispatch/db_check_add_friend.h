#ifndef OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_DB_CHECK_ADD_FRIEND_H_
#define OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_DB_CHECK_ADD_FRIEND_H_

#include <stddef.h>
#include <stdint.h>

#include "old_data/db/core/db_dispatcher.h"

// IDA stream payload used by DB_CheckAddFriend.  The final physical byte is
// required by the 0x24-byte Stream contract, but has no direct read/write in
// the producer, DB dispatcher, or Inter receiver closure.
struct SIG_CHECK_ADD_FRIEND {
    int32_t result;                       // +0x00
    uint8_t check_name;                   // +0x04
    char character_name[30];              // +0x05
    uint8_t trailing_byte_unobserved;     // +0x23
};

static_assert(sizeof(SIG_CHECK_ADD_FRIEND) == 0x24u,
              "SIG_CHECK_ADD_FRIEND IDA ABI size must stay 0x24");
static_assert(offsetof(SIG_CHECK_ADD_FRIEND, check_name) == 0x04u,
              "SIG_CHECK_ADD_FRIEND check_name offset must stay 0x04");
static_assert(offsetof(SIG_CHECK_ADD_FRIEND, character_name) == 0x05u,
              "SIG_CHECK_ADD_FRIEND character_name offset must stay 0x05");
static_assert(offsetof(SIG_CHECK_ADD_FRIEND, trailing_byte_unobserved) ==
                  0x23u,
              "SIG_CHECK_ADD_FRIEND trailing byte offset must stay 0x23");

class DB_CheckAddFriend : public DBDispatcher {
public:
    DB_CheckAddFriend();
    virtual ~DB_CheckAddFriend();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_DB_CHECK_ADD_FRIEND_H_
