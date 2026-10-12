#ifndef OLDSRV_OLD_DATA_DB_CORE_DB_RUNTIME_PAYLOADS_H_
#define OLDSRV_OLD_DATA_DB_CORE_DB_RUNTIME_PAYLOADS_H_

#include "old_game/player/core/character_load_mercenary.h"
#include "old_game/economy/inventory/inventory.h"

#include <stdint.h>

// IDA wire records shared by the DB dispatchers and their game-runtime
// producers.  These declarations intentionally describe only the fields the
// original dispatchers exchange; the producer-side game state remains owned
// by its runtime module.
#pragma pack(push, 1)

// IDA: SIG_OBJECT_BRING_UP, consumed by DB_SaveObjectBringUp @0x8446A1E.
struct SIG_OBJECT_BRING_UP {
    uint32_t m_id;                 // +0x00
    uint32_t charac_no;            // +0x04
    uint32_t server_no;            // +0x08
    uint8_t use_material_count;    // +0x0C
    uint8_t give_reward_flag;      // +0x0D
    uint8_t reserved_0e[2];        // +0x0E
};

// IDA: SIG_MERCENARY_DB_DATA, consumed by DB_SaveMercenaryData @0x84458C6.
// The wire buffer reserves 36 records (0xBD4 total); the original dispatcher
// accepts at most 30 populated records.
struct SIG_MERCENARY_DB_DATA {
    uint32_t count;                                // +0x0000
    CHARAC_LOAD_MERCENARY mercenary[36];            // +0x0004
};

namespace private_store {

// IDA: PrivateStoreItem is a 0x4c-byte persisted extension of the canonical
// 0x3d-byte Inven_Item ABI.  Its first 0x3d bytes are copied verbatim by the
// private-store runtime; fields below are the offsets used in the saved-store
// protocol and packet builders.
struct PrivateStoreItem {
    // IDA_EA: 0x823457C
    // IDA_NAME: _ZN13private_store16PrivateStoreItemC2Ev
    // DEMANGLED: private_store::PrivateStoreItem::PrivateStoreItem(void)
    PrivateStoreItem();

    // IDA_EA: 0x823459A
    // IDA_NAME: _ZN13private_store16PrivateStoreItem5ResetEv
    // DEMANGLED: private_store::PrivateStoreItem::Reset(void)
    PrivateStoreItem* Reset();

    // IDA_EA: 0x85CB662 / 0x85CB66E.  These are out-of-line old-server
    // methods (rather than compiler-synthesised conveniences) and are used
    // by the private-store packet and state machines.
    int GetState();
    PrivateStoreItem* operator=(const PrivateStoreItem& rhs);

    Inven_Item item;                // +0x00
    uint8_t state;                  // +0x3D
    uint8_t reserved_3e[2];         // +0x3E
    int32_t price;                  // +0x40
    uint16_t store_slot;            // +0x44
    uint8_t inventory_space;        // +0x46
    uint8_t reserved_47;            // +0x47
    uint16_t inventory_slot;        // +0x48
    uint16_t reserved_4a;           // +0x4A
};

// IDA: private_store::SIG_PRIVATE_STORE, used by DB_SavePrivateStore and
// DB_LoadPrivateStore.  The fourteen item slots are serialized directly into
// the store table's blob column.
struct SIG_PRIVATE_STORE {
    uint32_t charac_no;             // +0x000
    uint8_t loaded;                 // +0x004
    uint8_t reserved_005[3];        // +0x005
    PrivateStoreItem item[14];      // +0x008
    int32_t use_doll;               // +0x430
};

}  // namespace private_store

#pragma pack(pop)

static_assert(sizeof(SIG_OBJECT_BRING_UP) == 0x10u,
              "SIG_OBJECT_BRING_UP IDA ABI size must stay 0x10");
static_assert(sizeof(SIG_MERCENARY_DB_DATA) == 0xBD4u,
              "SIG_MERCENARY_DB_DATA IDA ABI size must stay 0xBD4");
static_assert(sizeof(private_store::PrivateStoreItem) == 0x4Cu,
              "private_store::PrivateStoreItem IDA ABI size must stay 0x4C");
static_assert(sizeof(private_store::SIG_PRIVATE_STORE) == 0x434u,
              "private_store::SIG_PRIVATE_STORE IDA ABI size must stay 0x434");

#endif  // OLDSRV_OLD_DATA_DB_CORE_DB_RUNTIME_PAYLOADS_H_
