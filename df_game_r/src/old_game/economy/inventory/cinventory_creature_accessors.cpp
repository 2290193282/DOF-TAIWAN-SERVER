#include "old_game/economy/inventory/inventory.h"

// IDA_EA: 0x80DD568
// IDA_NAME: _ZN10CInventory15GetCreatureMgrWEv
// DEMANGLED: CInventory::GetCreatureMgrW(void)
user_creature::CCreatureMgr* CInventory::GetCreatureMgrW() {
    return &creature_manager_;
}

// IDA_EA: 0x81B7408
// IDA_NAME: _ZNK10CInventory15GetCreatureMgrREv
// DEMANGLED: CInventory::GetCreatureMgrR(void)const
const user_creature::CCreatureMgr* CInventory::GetCreatureMgrR() const {
    return &creature_manager_;
}
