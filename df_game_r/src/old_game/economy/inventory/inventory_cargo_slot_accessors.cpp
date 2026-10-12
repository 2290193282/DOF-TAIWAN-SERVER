#include "old_game/economy/equipment/expand_equipslot_runtime.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"

#include <string.h>


// IDA_EA: 0x822D6EE
// IDA_NAME: _ZN10CInventory22set_inventory_capacityEi
// DEMANGLED: CInventory::set_inventory_capacity(int)
CInventory* CInventory::set_inventory_capacity(int capacity) {
    inventory_capacity_ = capacity;
    return this;
}

// IDA_EA: 0x84FB918
// IDA_NAME: _ZNK10CInventory12GetInvenSlotEii
// DEMANGLED: CInventory::GetInvenSlot(int,int)const
Inven_Item CInventory::GetInvenSlot(int inventory_type, int slot) const {
    Inven_Item empty;
    Inven_Item out;
    if (CheckValidSlot(inventory_type, slot) == 1 &&
        stackable_items_ && avatar_items_ && creature_items_) {
        switch (inventory_type) {
            case 0:
                memcpy(&out, &equipment_items_[slot], sizeof(out));
                return out;
            case 1:
                memcpy(&out, &stackable_items_[slot], sizeof(out));
                return out;
            case 2:
                memcpy(&out, &avatar_items_[slot], sizeof(out));
                return out;
            case 3:
                memcpy(&out, &creature_items_[slot], sizeof(out));
                return out;
            case 5: {
                CUser* user = parent_;
                CExpandEquipslot* expand = reinterpret_cast<CExpandEquipslot*>(
                    user->GetCharacExpandData(
                        static_cast<ENUM_CHARAC_EXPAND_TYPE>(9)));
                if (expand) {
                    return expand->GetInvenSlot(
                        static_cast<INVEN_TYPE>(inventory_type), slot);
                }
                break;
            }
            default:
                break;
        }
    }
    return empty;
}


// IDA_EA: 0x822D708
// IDA_NAME: _ZNK6CCargo11GetCapacityEv
// DEMANGLED: CCargo::GetCapacity(void)const
int CCargo::GetCapacity() const {
    return capacity_;
}

// IDA_EA: 0x850C656
// IDA_NAME: _ZN6CCargo12UpgradeCargoEi
// DEMANGLED: CCargo::UpgradeCargo(int)
int CCargo::UpgradeCargo(int capacity) {
    if (!capacity_ || capacity_ > capacity) {
        return 0;
    }
    if (capacity_ >= capacity) {
        return 0;
    }
    capacity_ = capacity;
    return 1;
}

// IDA_EA: 0x850C698
// IDA_NAME: _ZNK6CCargo12isUpgradableEi
// DEMANGLED: CCargo::isUpgradable(int)const
int CCargo::isUpgradable(int capacity) const {
    const int current = GetCapacity();
    if (!current || current > capacity) {
        return 0;
    }
    if (current < capacity) {
        return capacity <= 152;
    }
    return 0;
}
