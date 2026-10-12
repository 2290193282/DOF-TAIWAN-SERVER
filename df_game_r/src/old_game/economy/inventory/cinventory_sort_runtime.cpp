#include "old_game/economy/inventory/inventory.h"

#include <cstdlib>
#include <stdint.h>

// IDA_EA: 0x8504D0D
// IDA_NAME: _Z12_CompareSlotPKvS0_
// DEMANGLED: _CompareSlot(void const*,void const*)
//
// The raw comparator deliberately only groups occupied entries by their byte
// at +0x01 and gives all empty entries category 11.  It does not order equal
// categories by id; preserving that limited comparison is required because
// qsort's tie ordering is part of the old inventory presentation behavior.
int _CompareSlot(const void* left, const void* right) {
    const Inven_Item& left_item = *static_cast<const Inven_Item*>(left);
    const Inven_Item& right_item = *static_cast<const Inven_Item*>(right);
    const int left_category = left_item.item_id_ ? left_item.item_category_ : 11;
    const int right_category =
        right_item.item_id_ ? right_item.item_category_ : 11;

    if (left_category < right_category) return -1;
    return left_category > right_category;
}

// IDA_EA: 0x8504D80
// IDA_NAME: _ZN10CInventory4sortE10INVEN_TYPE
// DEMANGLED: CInventory::sort(INVEN_TYPE)
int CInventory::sort(INVEN_TYPE inventory_type) {
    if (inventory_type == static_cast<INVEN_TYPE>(1)) {
        for (int item_type = 1; item_type <= 4; ++item_type) {
            int start = 0;
            int end = 0;
            StartEndPos(static_cast<Inven_Item::ITEM_TYPE>(item_type), start,
                        end);
            std::qsort(stackable_items_ + start,
                       static_cast<size_t>(end - start + 1),
                       sizeof(Inven_Item), _CompareSlot);
        }

        int start = 0;
        int end = 0;
        StartEndPos(static_cast<Inven_Item::ITEM_TYPE>(10), start, end);
        std::qsort(stackable_items_ + start,
                   static_cast<size_t>(end - start + 1), sizeof(Inven_Item),
                   _CompareSlot);
        return 0;
    }

    if (inventory_type == static_cast<INVEN_TYPE>(3)) {
        for (int item_type = 6; item_type <= 7; ++item_type) {
            int start = 0;
            int end = 0;
            StartEndPos(static_cast<Inven_Item::ITEM_TYPE>(item_type), start,
                        end);
            std::qsort(creature_items_ + start,
                       static_cast<size_t>(end - start + 1),
                       sizeof(Inven_Item), _CompareSlot);
        }
        return 0;
    }

    return 1;
}

// IDA_EA: 0x850BD0C
// IDA_NAME: _ZN6CCargo4sortEv
// DEMANGLED: CCargo::sort(void)
void* CCargo::sort() {
    void* result = reinterpret_cast<void*>(
        static_cast<uintptr_t>(static_cast<unsigned char>(cargo_sort_flag_) ^
                               1u));
    if (cargo_sort_flag_ != true) {
        std::qsort(cargo_, 0x98u, sizeof(Inven_Item), CCargo::_CompareSlot);
        cargo_sort_flag_ = true;
        result = this;
    }
    return result;
}
