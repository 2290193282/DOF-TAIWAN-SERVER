#ifndef OLDSRV_OLD_DATA_DB_INVENTORY_ACCOUNT_CARGO_PAYLOAD_H_
#define OLDSRV_OLD_DATA_DB_INVENTORY_ACCOUNT_CARGO_PAYLOAD_H_

#include <stddef.h>
#include <stdint.h>

#include "old_game/economy/inventory/inventory.h"


// Canonical DBTask/InterDispatcher payloads for account cargo.  The 140828
// inventory record is 0x3d bytes, so the existing 56-slot cargo body remains
// 0xd58 bytes and the complete load result remains 0xd60 bytes.
#pragma pack(push, 1)

struct SIG_CREATE_ACCOUNT_CARGO_RESULT {
    uint8_t result;
    uint8_t reserved_01;
    uint16_t reserved_02;
    uint32_t capacity;
};

struct SIG_UPGRADE_ACCOUNT_CARGO_RESULT {
    uint8_t result;
    uint8_t reserved_01;
    uint16_t reserved_02;
    uint32_t capacity;
};

struct SIG_ACCOUNT_CARGO_DATA {
    uint32_t capacity;
    uint32_t money;
    Inven_Item items[56];
};

#pragma pack(pop)

static_assert(sizeof(SIG_CREATE_ACCOUNT_CARGO_RESULT) == 0x08u &&
                  offsetof(SIG_CREATE_ACCOUNT_CARGO_RESULT, capacity) == 0x04u,
              "create-account-cargo result ABI changed");
static_assert(sizeof(SIG_UPGRADE_ACCOUNT_CARGO_RESULT) == 0x08u &&
                  offsetof(SIG_UPGRADE_ACCOUNT_CARGO_RESULT, capacity) == 0x04u,
              "upgrade-account-cargo result ABI changed");
static_assert(sizeof(SIG_ACCOUNT_CARGO_DATA) == 0x0d60u &&
                  offsetof(SIG_ACCOUNT_CARGO_DATA, items) == 0x08u,
              "140828 account-cargo payload ABI changed");

#endif  // OLDSRV_OLD_DATA_DB_INVENTORY_ACCOUNT_CARGO_PAYLOAD_H_
