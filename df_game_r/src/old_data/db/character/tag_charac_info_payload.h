#ifndef OLDSRV_DB_TAG_CHARAC_INFO_PAYLOAD_H_
#define OLDSRV_DB_TAG_CHARAC_INFO_PAYLOAD_H_

#include "old_game/player/core/character_tag_data.h"
#include <stddef.h>
#include <stdint.h>

// IDA_EA: Stream::GetOutBuffer<SIG_REQUEST_TAG_CHARAC_INFO> @0x8454514,
// Stream::GetInBuffer<...> @0x823A984, and CStreamGuard::GetInBuffer<...>
// @0x823744E. The target clear-avatar slot extends the typed inventory tail.
struct SIG_REQUEST_TAG_CHARAC_INFO {
    uint32_t charac_no;                    // +0x0000
    uint8_t striker_skill_index;           // +0x0004
    TAG_CHARAC_DATA tag_charac_data;       // +0x0005
    TAG_CHARAC_INVEN_DATA inventory;       // +0x007d
    CHARAC_QUEST_SHOP_DATA quest_shop;     // +0x412d
};

static_assert(sizeof(TAG_CHARAC_INVEN_DATA) == 0x40b0u,
              "TAG inventory payload size must stay 0x40b0");
// The final packed quest-shop byte ends at +0x41d7.  The containing request
// retains the native four-byte alignment inherited from charac_no, so Stream
// advances one final pad byte and the physical extent is 0x41d8.
static_assert(sizeof(SIG_REQUEST_TAG_CHARAC_INFO) == 0x41d8u,
              "SIG_REQUEST_TAG_CHARAC_INFO target ABI size must stay 0x41d8");
static_assert(offsetof(SIG_REQUEST_TAG_CHARAC_INFO, tag_charac_data) == 5u,
              "TAG request charac data offset must stay 5");
static_assert(offsetof(SIG_REQUEST_TAG_CHARAC_INFO, inventory) == 125u,
              "TAG request inventory offset must stay 125");
static_assert(offsetof(SIG_REQUEST_TAG_CHARAC_INFO, quest_shop) == 16685u,
              "TAG request quest-shop offset must stay 16685");


#endif  // OLDSRV_DB_TAG_CHARAC_INFO_PAYLOAD_H_
