#ifndef OLDSRV_OLD_GAME_USERINFO_TITLE_BOOK_DISPATCHER_MESSAGES_H_
#define OLDSRV_OLD_GAME_USERINFO_TITLE_BOOK_DISPATCHER_MESSAGES_H_

#include <stdint.h>

#include "infra/network/packet/message_base.h"

// CMD412 and CMD413 raw template storage is not constructed.  These named
// types describe only their proven 13-byte C2S base plus five int reads.
#pragma pack(push, 1)
struct MSG_TITLEBOOK_PUT : public MSG_BASE {
    int item_space;
    int source_slot;
    int item_id;
    int category;
    int title_slot;
};

struct MSG_TITLEBOOK_GET : public MSG_BASE {
    int item_space;
    int output_slot;
    int item_id;
    int category;
    int title_slot;
};
#pragma pack(pop)

static_assert(sizeof(MSG_TITLEBOOK_PUT) == 0x21u,
              "CMD412 must retain the 13-byte C2S header plus five ints");
static_assert(sizeof(MSG_TITLEBOOK_GET) == 0x21u,
              "CMD413 must retain the 13-byte C2S header plus five ints");

// CMD424 raw template storage is not constructed.  This named type documents
// only the 15-byte read/process ABI and deliberately has no C2 identity.
#pragma pack(push, 1)
struct MSG_TITLEBOOK_OTHERUSER : public MSG_BASE {
    uint16_t target_uid;
};
#pragma pack(pop)

static_assert(sizeof(MSG_TITLEBOOK_OTHERUSER) == 0x0fu,
              "CMD424 must retain the 13-byte C2S header plus target uid");

#endif  // OLDSRV_OLD_GAME_USERINFO_TITLE_BOOK_DISPATCHER_MESSAGES_H_
