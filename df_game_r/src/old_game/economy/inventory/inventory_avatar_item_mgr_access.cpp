#include "old_game/economy/inventory/inventory.h"

#include "old_game/appearance/pet/creature.h"

// IDA_EA: 0x8110C6C
// IDA_NAME: _ZN10CInventory17GetAvatarItemMgrWEv
// DEMANGLED: CInventory::GetAvatarItemMgrW(void)
WongWork::CAvatarItemMgr* CInventory::GetAvatarItemMgrW() {
    return &avatar_item_manager_;
}

// IDA_EA: 0x80DD576
// IDA_NAME: _ZNK10CInventory17GetAvatarItemMgrREv
// DEMANGLED: CInventory::GetAvatarItemMgrR(void)const
const WongWork::CAvatarItemMgr* CInventory::GetAvatarItemMgrR() const {
    return &avatar_item_manager_;
}
