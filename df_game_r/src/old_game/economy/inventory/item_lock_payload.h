#ifndef OLDSRV_OLD_GAME_INVENTORY_ITEM_LOCK_PAYLOAD_H_
#define OLDSRV_OLD_GAME_INVENTORY_ITEM_LOCK_PAYLOAD_H_

#include "old_data/db/inventory/load_inventory_payload.h"

#include <stddef.h>
#include <stdint.h>


// IDA_EA: CStreamGuard::GetInBuffer<SIG_ITEM_LOCK_UPDATE> @0x8543238,
// Stream::GetInBuffer<SIG_ITEM_LOCK_UPDATE> @0x8543836, and
// Stream::GetOutBuffer<SIG_ITEM_LOCK_UPDATE> @0x8453768 all use 0x324 bytes.
struct SIG_ITEM_LOCK_UPDATE {
    uint32_t charac_no;                 // +0x000
    stItemLockInfo item_locks[100];     // +0x004
};

static_assert(sizeof(SIG_ITEM_LOCK_UPDATE) == 0x324u,
              "SIG_ITEM_LOCK_UPDATE IDA ABI size must stay 0x324");
static_assert(offsetof(SIG_ITEM_LOCK_UPDATE, item_locks) == 0x04u,
              "SIG_ITEM_LOCK_UPDATE item_locks offset must stay 0x04");

#endif  // OLDSRV_OLD_GAME_INVENTORY_ITEM_LOCK_PAYLOAD_H_
