#include "old_game/economy/inventory/inventory.h"


// IDA_EA: 0x82898C0
// IDA_NAME: _ZN13CAccountCargo9ResetSlotEi
// DEMANGLED: CAccountCargo::ResetSlot(int)
Inven_Item* CAccountCargo::ResetSlot(int slot) {
    if (!CheckValidSlot(slot)) {
        return 0;
    }
    return items_[slot].reset();
}

// IDA_EA: 0x82898F8
// IDA_NAME: _ZN13CAccountCargo7GetSlotEi
// DEMANGLED: CAccountCargo::GetSlot(int)
Inven_Item CAccountCargo::GetSlot(int slot) {
    if (!CheckValidSlot(slot)) {
        return Inven_Item();
    }
    return items_[slot];
}

// IDA_EA: 0x8289E3C
// IDA_NAME: _ZN13CAccountCargo10DeleteItemEii
// DEMANGLED: CAccountCargo::DeleteItem(int,int)
int CAccountCargo::DeleteItem(int slot, int count) {
    if (!CheckValidSlot(slot)) {
        return 0;
    }

    Inven_Item& item = items_[slot];
    if (item.isEmpty() || count <= 0) {
        return 0;
    }

    if (item.isEquipableItemType()) {
        ResetSlot(slot);
    } else if (item.get_add_info() < count) {
        return 0;
    } else if (item.get_add_info() == count) {
        ResetSlot(slot);
    } else {
        item.set_add_info(item.get_add_info() - count);
    }
    altered_ = 1;
    return 1;
}
