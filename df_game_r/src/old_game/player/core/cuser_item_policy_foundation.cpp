#include "old_game/player/core/cuser.h"

#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_data/PVF/data/data_manager.h"

#include <stdint.h>

// IDA_EA: 0x8672042
// IDA_NAME: _ZN5CUser23_CalcItemVectorMultiplyERSt6vectorISt4pairIiiESaIS2_EEi
// DEMANGLED: CUser::_CalcItemVectorMultiply(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,int)
bool CUser::_CalcItemVectorMultiply(std::vector<std::pair<int, int> >& items,
                                    int multiplier) {
    for (std::vector<std::pair<int, int> >::iterator it = items.begin();
         it != items.end();
         ++it) {
        // The old i386 `imul` retains the low 32 bits of the signed product.
        const uint32_t product = static_cast<uint32_t>(multiplier) *
                                 static_cast<uint32_t>(it->second);
        it->second = static_cast<int32_t>(product);
    }

    // The old function returns the terminating iterator comparison, which is
    // false after both the empty and non-empty loops.
    return false;
}

// IDA_EA: 0x86720D0
// IDA_NAME: _ZN5CUser27_IsCompoundResult2MoreEquipERKSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: CUser::_IsCompoundResult2MoreEquip(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&)
int CUser::_IsCompoundResult2MoreEquip(
    const std::vector<std::pair<int, int> >& items) {
    const CDataManager* const data_manager = G_CDataManager();
    for (std::vector<std::pair<int, int> >::const_iterator it = items.begin();
         it != items.end();
         ++it) {
        CItem* const item = data_manager->find_item(it->first);
        if (item == 0) {
            return 1;
        }
        if (item->is_stackable() == 0 && it->second > 1) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8673342
// IDA_NAME: _ZN5CUser16IsOldLotteryItemEj
// DEMANGLED: CUser::IsOldLotteryItem(unsigned int)
int CUser::IsOldLotteryItem(unsigned int item_id) {
    if (item_id > 0x1D56u) {
        if (item_id > 0x1E5Cu) {
            return item_id - 0x1EBAu <= 1u;
        }
        if (item_id >= 0x1E44u || item_id - 0x1DCAu <= 5u) {
            return 1;
        }
        return 0;
    }

    if (item_id >= 0x1D4Cu ||
        (item_id >= 0x1BC6u &&
         (item_id <= 0x1BF5u || item_id - 0x1C15u <= 4u))) {
        return 1;
    }
    return 0;
}
