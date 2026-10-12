#include "old_game/economy/inventory/inventory.h"

#include "common/manager/user_history_log_abi.h"
#include "old_game/player/core/cuser.h"

#include <cstring>
#include <limits.h>
#include <stdint.h>


extern int checkStackableLimit(unsigned long item_id, int stack_count)
    __asm__("_Z19checkStackableLimitmi");

namespace {

const int kCargoSlotCount = 152;

unsigned long ItemId(const Inven_Item& item) {
    return static_cast<unsigned long>(static_cast<uint32_t>(item.item_id_));
}

int ItemKey(const Inven_Item& item) {
    return item.add_info_;
}

void CopyItem(Inven_Item* destination, const Inven_Item& source) {
    std::memcpy(destination, &source, sizeof(Inven_Item));
}

}  // namespace


// IDA_EA: 0x86949DA
// IDA_NAME: _ZN6CCargoC2Ev
// DEMANGLED: CCargo::CCargo(void)
CCargo::CCargo() {
    cargo_ = 0;
    reset();
}

// IDA_EA: 0x86949F8
// IDA_NAME: _ZN6CCargoD2Ev
// DEMANGLED: CCargo::~CCargo()
CCargo::~CCargo() {}

// IDA_EA: 0x84E99F0
// IDA_NAME: _ZNK6CCargo12GetItemCountEv
// DEMANGLED: CCargo::GetItemCount(void)const
int CCargo::GetItemCount() const {
    return item_count_;
}

// IDA_EA: 0x850B116
// IDA_NAME: _ZN6CCargo9set_cargoEPv
// DEMANGLED: CCargo::set_cargo(void *)
int CCargo::set_cargo(void* source) {
    if (capacity_ <= 0) {
        return 0;
    }
    cargo_sort_flag_ = false;
    std::memcpy(cargo_, source, kCargoSlotCount * sizeof(Inven_Item));
    _GetItemCount();
    return 1;
}

// IDA_EA: 0x850B164
// IDA_NAME: _ZNK6CCargo9get_cargoEPv
// DEMANGLED: CCargo::get_cargo(void *)const
int CCargo::get_cargo(void* destination) const {
    if (capacity_ <= 0) {
        return 0;
    }
    std::memcpy(destination, cargo_, kCargoSlotCount * sizeof(Inven_Item));
    return 1;
}

// IDA_EA: 0x850B2B4
// IDA_NAME: _ZNK6CCargo14get_cargo_slotEi
// DEMANGLED: CCargo::get_cargo_slot(int)const
Inven_Item CCargo::get_cargo_slot(int slot) const {
    Inven_Item item;
    if (IsOperatorable(slot)) {
        std::memmove(&item, &cargo_[slot], sizeof(Inven_Item));
    } else {
        std::memset(&item, 0, sizeof(Inven_Item));
    }
    return item;
}

// IDA_EA: 0x850B390
// IDA_NAME: _ZNK6CCargo11GetCargoRefEi
// DEMANGLED: CCargo::GetCargoRef(int)const
const Inven_Item* CCargo::GetCargoRef(int slot) const {
    return IsOperatorable(slot) ? &cargo_[slot] : 0;
}

// IDA_EA: 0x850B3C8
// IDA_NAME: _ZN6CCargo11GetCargoRefEi
// DEMANGLED: CCargo::GetCargoRef(int)
Inven_Item* CCargo::GetCargoRef(int slot) {
    return IsOperatorable(slot) ? &cargo_[slot] : 0;
}

// IDA_EA: 0x850B400
// IDA_NAME: _ZN6CCargo11insert_itemERK10Inven_Item
// DEMANGLED: CCargo::insert_item(Inven_Item const&)
int CCargo::insert_item(const Inven_Item& item) {
    if (!item.isEquipableItemType()) {
        for (int slot = 0; slot < kCargoSlotCount; ++slot) {
            if (_AddStackItem(item, slot) >= 0) {
                return slot;
            }
        }
    }

    for (int slot = 0; slot < kCargoSlotCount; ++slot) {
        if (!ItemId(cargo_[slot])) {
            _AddItem(item, slot);
            return slot;
        }
    }
    return -1;
}

// IDA_EA: 0x850BB00
// IDA_NAME: _ZN6CCargo11update_itemEiRK10Inven_Item
// DEMANGLED: CCargo::update_item(int,Inven_Item const&)
CCargo* CCargo::update_item(int slot, const Inven_Item& item) {
    const bool operatorable = IsOperatorable(slot);
    if (operatorable) {
        Inven_Item* const destination = &cargo_[slot];
        if (ItemId(*destination)) {
            CopyItem(destination, item);
            return reinterpret_cast<CCargo*>(static_cast<uintptr_t>(
                item.reserved_capacity_.value_2_));
        }
        return _AddItem(item, slot);
    }
    return reinterpret_cast<CCargo*>(static_cast<uintptr_t>(1u));
}

// IDA_EA: 0x850B4B0
// IDA_NAME: _ZNK6CCargo16_tryAddStackItemERK10Inven_Itemi
// DEMANGLED: CCargo::_tryAddStackItem(Inven_Item const&,int)const
int CCargo::_tryAddStackItem(const Inven_Item& item, int slot) const {
    const Inven_Item* const current = &cargo_[slot];
    if (ItemId(*current) != ItemId(item) ||
        INT_MAX - item.get_add_info() < ItemKey(*current)) {
        return -1;
    }

    return checkStackableLimit(ItemId(*current),
                               current->get_add_info() + item.get_add_info())
               ? slot
               : -10;
}

// IDA_EA: 0x850B570
// IDA_NAME: _ZNK6CCargo24tryInsertItemSpecialSlotERK10Inven_Itemi
// DEMANGLED: CCargo::tryInsertItemSpecialSlot(Inven_Item const&,int)const
int CCargo::tryInsertItemSpecialSlot(const Inven_Item& item, int slot) const {
    if (!IsOperatorable(slot)) {
        return -2;
    }

    if (!item.isEquipableItemType()) {
        if (ItemId(cargo_[slot]) == ItemId(item)) {
            const int result = _tryAddStackItem(item, slot);
            if (result >= 0) {
                return slot;
            }
            if (result == -10) {
                return -1;
            }
        } else {
            for (int candidate = 0; candidate < kCargoSlotCount; ++candidate) {
                const int result = _tryAddStackItem(item, candidate);
                if (result >= 0) {
                    return candidate;
                }
                if (result == -10) {
                    return -1;
                }
            }
        }
    }

    return ItemId(cargo_[slot]) ? -1 : slot;
}

// IDA_EA: 0x850B672
// IDA_NAME: _ZN6CCargo24insert_item_special_slotER10Inven_Itemi
// DEMANGLED: CCargo::insert_item_special_slot(Inven_Item &,int)
int CCargo::insert_item_special_slot(Inven_Item& item, int slot) {
    if (!IsOperatorable(slot)) {
        return -2;
    }

    if (!item.isEquipableItemType()) {
        if (ItemId(cargo_[slot]) == ItemId(item)) {
            const int result = _AddStackItem(item, slot);
            if (result >= 0) {
                return slot;
            }
            if (result == -10) {
                return -1;
            }
        } else {
            for (int candidate = 0; candidate < kCargoSlotCount; ++candidate) {
                const int result = _tryAddStackItem(item, candidate);
                if (result >= 0) {
                    return candidate;
                }
                if (result == -10) {
                    return -1;
                }
            }
        }
    }

    if (ItemId(cargo_[slot])) {
        return -1;
    }
    _AddItem(item, slot);
    return slot;
}

// IDA_EA: 0x850B904
// IDA_NAME: _ZN6CCargo9move_itemEii
// DEMANGLED: CCargo::move_item(int,int)
int CCargo::move_item(int source_slot, int destination_slot) {
    if (!IsOperatorable(source_slot) || !IsOperatorable(destination_slot)) {
        return 0;
    }

    Inven_Item* const source = &cargo_[source_slot];
    Inven_Item* const destination = &cargo_[destination_slot];
    if (!ItemId(*source) && !ItemId(*destination)) {
        return 0;
    }

    Inven_Item item;
    CopyItem(&item, *source);
    CopyItem(source, *destination);
    CopyItem(destination, item);
    cargo_sort_flag_ = false;
    return 1;
}

// IDA_EA: 0x850B1A0
// IDA_NAME: _ZN6CCargo10_ResetSlotEi14eItemDelReason
// DEMANGLED: CCargo::_ResetSlot(int,eItemDelReason)
CCargo* CCargo::_ResetSlot(int slot, eItemDelReason reason) {
    Inven_Item item;
    CopyItem(&item, cargo_[slot]);
    cargo_[slot].reset();

    cUserHistoryLog* const history = CUser_history_log(owner_);
    if (item.isEquipableItemType()) {
        history->ItemDelCargo(static_cast<int>(ItemId(item)), 1, 0, reason);
    } else {
        history->ItemDelCargo(static_cast<int>(ItemId(item)),
                              item.get_add_info(),
                              0,
                              reason);
    }
    --item_count_;
    return this;
}

// IDA_EA: 0x850B794
// IDA_NAME: _ZN6CCargo11delete_itemEii14eItemDelReason
// DEMANGLED: CCargo::delete_item(int,int,eItemDelReason)
int CCargo::delete_item(int slot, int count, eItemDelReason reason) {
    if (!IsOperatorable(slot)) {
        return 0;
    }

    Inven_Item* const item = &cargo_[slot];
    if (!ItemId(*item) || count <= 0) {
        return 0;
    }

    cargo_sort_flag_ = false;
    if (item->isEquipableItemType() || item->get_add_info() == count) {
        _ResetSlot(slot, reason);
    } else {
        const int remaining_count = item->get_add_info() - count;
        if (remaining_count <= 0) {
            return 0;
        }

        item->add_info_ = remaining_count;
        CUser_history_log(owner_)->ItemDelCargo(
            static_cast<int>(ItemId(*item)), count, remaining_count, reason);
    }
    return 1;
}

// IDA_EA: 0x850BBCA
// IDA_NAME: _ZNK6CCargo16check_slot_emptyEi
// DEMANGLED: CCargo::check_slot_empty(int)const
bool CCargo::check_slot_empty(int slot) const {
    return IsOperatorable(slot) && ItemId(cargo_[slot]) == 0;
}

// IDA_EA: 0x850BC14
// IDA_NAME: _ZNK6CCargo16check_item_existEi
// DEMANGLED: CCargo::check_item_exist(int)const
int CCargo::check_item_exist(int item_id) const {
    for (int slot = 0; slot < kCargoSlotCount; ++slot) {
        if (ItemId(cargo_[slot]) ==
            static_cast<unsigned int>(item_id)) {
            return slot;
        }
    }
    return -1;
}

// IDA_EA: 0x850BC5A
// IDA_NAME: _ZN6CCargo12_CompareSlotEPKvS1_
// DEMANGLED: CCargo::_CompareSlot(void const*,void const*)
int CCargo::_CompareSlot(const void* left, const void* right) {
    const Inven_Item& left_item = *reinterpret_cast<const Inven_Item*>(left);
    const Inven_Item& right_item = *reinterpret_cast<const Inven_Item*>(right);
    const int left_type = ItemId(left_item) ? left_item.item_category_ : 9999;
    const int right_type = ItemId(right_item) ? right_item.item_category_ : 9999;

    if (left_type < right_type) {
        return -1;
    }
    if (left_type > right_type) {
        return 1;
    }
    if (left_type == 9999 || ItemId(left_item) == ItemId(right_item)) {
        return 0;
    }
    return ItemId(left_item) < ItemId(right_item) ? -1 : 1;
}

// IDA_EA: 0x850C406
// IDA_NAME: _ZNK6CCargo14get_empty_slotEv
// DEMANGLED: CCargo::get_empty_slot(void)const
int CCargo::get_empty_slot() const {
    int slot = 0;
    while (slot < kCargoSlotCount && ItemId(cargo_[slot])) {
        ++slot;
    }
    return IsOperatorable(slot) ? slot : -1;
}

// IDA_EA: 0x850C462
// IDA_NAME: _ZN6CCargo13_GetItemCountEv
// DEMANGLED: CCargo::_GetItemCount(void)
int CCargo::_GetItemCount() {
    item_count_ = 0;
    for (int slot = 0; slot < kCargoSlotCount; ++slot) {
        if (ItemId(cargo_[slot])) {
            ++item_count_;
        }
    }
    return item_count_;
}

// IDA_EA: 0x850C4BA
// IDA_NAME: _ZN6CCargo8_AddItemERK10Inven_Itemi
// DEMANGLED: CCargo::_AddItem(Inven_Item const&,int)
CCargo* CCargo::_AddItem(const Inven_Item& item, int slot) {
    CopyItem(&cargo_[slot], item);
    ++item_count_;
    cargo_sort_flag_ = false;
    return this;
}

// IDA_EA: 0x850C546
// IDA_NAME: _ZN6CCargo13_AddStackItemERK10Inven_Itemi
// DEMANGLED: CCargo::_AddStackItem(Inven_Item const&,int)
int CCargo::_AddStackItem(const Inven_Item& item, int slot) {
    Inven_Item* const current = &cargo_[slot];
    if (ItemId(*current) != ItemId(item)) {
        return -1;
    }

    cargo_sort_flag_ = false;
    if (INT_MAX - item.get_add_info() >= ItemKey(*current)) {
        const int combined = current->get_add_info() + item.get_add_info();
        if (!checkStackableLimit(ItemId(*current), combined)) {
            return -10;
        }
        current->add_info_ = combined;
    } else {
        current->add_info_ = 0;
    }
    return slot;
}

// IDA_EA: 0x850C6E0
// IDA_NAME: _ZNK6CCargo12GetCargoDataEiR10Inven_Item
// DEMANGLED: CCargo::GetCargoData(int,Inven_Item &)const
int CCargo::GetCargoData(int item_id, Inven_Item& item) const {
    for (int slot = 0; slot < kCargoSlotCount; ++slot) {
        const Inven_Item* const current = &cargo_[slot];
        if (ItemId(*current) == static_cast<unsigned int>(item_id)) {
            CopyItem(&item, *current);
            return slot;
        }
    }
    return -1;
}

// IDA_EA: 0x850C79E
// IDA_NAME: _ZN6CCargo18calcCargoItemCountEv
// DEMANGLED: CCargo::calcCargoItemCount(void)
int CCargo::calcCargoItemCount() {
    return _GetItemCount();
}

// IDA_EA: 0x850C7B2
// IDA_NAME: _ZN6CCargo17_IsExistEquipItemEii
// DEMANGLED: CCargo::_IsExistEquipItem(int,int)
int CCargo::_IsExistEquipItem(int item_id, int item_key) {
    for (int slot = 0; slot < kCargoSlotCount; ++slot) {
        const Inven_Item* const item = &cargo_[slot];
        if (item->isEquipableItemType() &&
            ItemId(*item) == static_cast<unsigned int>(item_id) &&
            ItemKey(*item) == item_key) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x850D39C
// IDA_NAME: _ZNK6CCargo14IsOperatorableEi
// DEMANGLED: CCargo::IsOperatorable(int)const
bool CCargo::IsOperatorable(int slot) const {
    return slot >= 0 && capacity_ > slot &&
           slot < kCargoSlotCount;
}

// IDA_EA: 0x8694A0C
// IDA_NAME: _ZN6CCargo11SetCapacityEi
// DEMANGLED: CCargo::SetCapacity(int)
CCargo* CCargo::SetCapacity(int capacity) {
    capacity_ = capacity;
    return this;
}
