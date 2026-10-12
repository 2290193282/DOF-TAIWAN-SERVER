#ifndef OLDSRV_GLOBAL_ITEM_UPGRADE_PUBLIC_ABI_H_
#define OLDSRV_GLOBAL_ITEM_UPGRADE_PUBLIC_ABI_H_

#include <stddef.h>
#include <stdint.h>

#include "old_game/economy/equipment/item_amplifier_abi.h"

namespace WongWork {

// IDA_EA: 0x80FC834
// IDA_NAME: _ZN8WongWork12IItemUpgradeC2Ev
// DEMANGLED: WongWork::IItemUpgrade::IItemUpgrade(void)
//
// IDA_EA: 0x80FC842
// IDA_NAME: _ZN8WongWork12IItemUpgradeD2Ev
// DEMANGLED: WongWork::IItemUpgrade::~IItemUpgrade(void)
// The old object starts with a C++ vptr.  The first observed vtable slot is
// the item-upgrade procedure used by item_upgrade_system @ 0x80FC850.
struct IItemUpgradeAbi {
    void* vptr;
};

struct IItemUpgradeVTableAbi {
    void* item_upgrade_system_slot;
};

// Declaration-only old-symbol entries. Their definitions are intentionally not
// synthesized here: the base lifecycle/vtable setup and derived constructors
// must be restored from their own IDA functions.
// IDA_EA: 0x85461C8
// IDA_NAME: _ZN8WongWork12CItemUpgradeC2Ev
// DEMANGLED: WongWork::CItemUpgrade::CItemUpgrade(void)
// The constructor replaces the base vptr with its derived vtable after
// constructing the upgrade table. The target table keeps the restored prefix
// and appends four native [table type] vectors. The vtable address is not a
// source-level constant and must be restored by the owning constructor body.
struct CItemUpgradeAbi {
    IItemUpgradeAbi interface_;
    upgrade_table_t upgrade_table_;
    void* random_;
    uint32_t random_limit_;
};

// IDA_EA: 0x811DFC0
// IDA_NAME: _ZN8WongWork21CItemUpgrade_SeparateC2Ev
// DEMANGLED: WongWork::CItemUpgrade_Separate::CItemUpgrade_Separate(void)
// GenuineDamageUpgradeTable is B-owned.  Its confirmed 0x188-byte public
// footprint is preserved here without manufacturing a duplicate parser type.
struct CItemUpgradeSeparateAbi {
    IItemUpgradeAbi interface_;
    uint8_t genuine_upgrade_table_[0x188];
    void* random_;
    uint32_t random_limit_;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(IItemUpgradeAbi) == 0x04u,
              "IItemUpgrade ABI size must stay 0x04");
static_assert(offsetof(CItemUpgradeAbi, upgrade_table_) == 0x04u,
              "CItemUpgrade table offset must stay 0x04");
static_assert(offsetof(CItemUpgradeAbi, random_) == 0x520u,
              "typed-table CItemUpgrade random offset must stay 0x520");
static_assert(sizeof(CItemUpgradeAbi) == 0x528u,
              "typed-table CItemUpgrade ABI size must stay 0x528");
static_assert(offsetof(CItemUpgradeSeparateAbi, genuine_upgrade_table_) ==
                  0x04u,
              "CItemUpgrade_Separate table offset must stay 0x04");
static_assert(offsetof(CItemUpgradeSeparateAbi, random_) == 0x18Cu,
              "CItemUpgrade_Separate random offset must stay 0x18C");
static_assert(sizeof(CItemUpgradeSeparateAbi) == 0x194u,
              "CItemUpgrade_Separate ABI size must stay 0x194");
#endif

}  // namespace WongWork

#endif  // OLDSRV_GLOBAL_ITEM_UPGRADE_PUBLIC_ABI_H_
