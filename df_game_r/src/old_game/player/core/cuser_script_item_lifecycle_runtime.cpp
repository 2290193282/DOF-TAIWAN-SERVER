#include "old_game/player/core/cuser.h"

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/limit_item_usage_info_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "common/manager/system_time.h"

#include <utility>
#include <vector>


// IDA_EA: 0x867E734
// IDA_NAME: _ZN5CUser24deleteDailyStackableItemERSt6vectorISt4pairIiiESaIS2_EEii
// DEMANGLED: CUser::deleteDailyStackableItem(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,int,int)
bool CUser::deleteDailyStackableItem(
    std::vector<std::pair<int, int> >& changed_slots,
    int begin_slot,
    int end_slot) {
    bool result = getCurCharacR() == 0;
    if (!result) {
        CInventory* const inventory = getCurCharacInvenR();
        for (int slot = begin_slot;; ++slot) {
            result = slot <= end_slot;
            if (!result) {
                break;
            }

            const Inven_Item inventory_item = inventory->GetInvenSlot(1, slot);
            if (inventory_item.item_id_) {
                CItem* const item =
                    G_CDataManager()->find_item(inventory_item.item_id_);
                if (item && item->is_stackable() == 1) {
                    const CStackableItem* const stackable_item =
                        static_cast<const CStackableItem*>(item);
                    if (stackable_item->is_daily_delete_item()) {
                        enableSaveInven();
                        inventory->delete_item(
                            static_cast<INVEN_TYPE>(1),
                            slot,
                            inventory_item.add_info_,
                            static_cast<eItemDelReason>(19),
                            true);
                        changed_slots.push_back(std::make_pair(0, slot));
                    }
                }
            }
        }
    }
    return result;
}

// IDA_EA: 0x8689010
// IDA_NAME: _ZN5CUser17ResetItemByScriptERSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: CUser::ResetItemByScript(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &)
bool CUser::ResetItemByScript(
    std::vector<std::pair<int, int> >& reset_items) {
    const stResetItemInfo* const reset_item_info =
        G_CDataManager()->GetResetItemInfo();
    const int current_sec = GlobalData::s_systemTime_.getCurSec();

    for (std::vector<stResetItemInfo::ResetItem>::const_iterator item =
             reset_item_info->values.begin();
         item != reset_item_info->values.end();
         ++item) {
        if (item->reset_time <= current_sec) {
            reset_items.push_back(std::make_pair(item->item_id, 0));
        }
    }

    // The original loop leaves its `index < size()` condition in EAX, which
    // is false once all rows have been examined.
    return false;
}

// IDA_EA: 0x86890BE
// IDA_NAME: _ZN5CUser18RefillItemByScriptERSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: CUser::RefillItemByScript(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &)
bool CUser::RefillItemByScript(
    std::vector<std::pair<int, int> >& changed_slots) {
    const stRefillItemInfo* const refill_info =
        G_CDataManager()->GetRefillItemInfo();
    const int current_time = GlobalData::s_systemTime_.getCurSec();

    for (unsigned int index = 0;; ++index) {
        const bool result = refill_info->values.size() > index;
        if (!result) {
            return false;
        }

        const stRefillItemInfo::RefillItem& refill =
            refill_info->values[index];
        if (refill.refill_time <= current_time) {
            continue;
        }

        if (!refill.refill_count) {
            changed_slots.push_back(std::make_pair(refill.item_id, 0));
            continue;
        }

        CItem* const prototype = G_CDataManager()->find_item(refill.item_id);
        if (!prototype) {
            continue;
        }

        Inven_Item item;
        const int slot = getCurCharacInvenR()->GetInvenData(
            refill.item_id, item);
        int updated_slot = slot;
        if (updated_slot >= 0) {
            history_log().ItemAdd(static_cast<INVEN_TYPE>(1),
                                  item.get_add_info(),
                                  item.item_id_,
                                  item,
                                  static_cast<eItemAddReason>(13));
            item.set_add_info(refill.refill_count);
            getCurCharacInvenW()->update_item(
                static_cast<INVEN_TYPE>(1), updated_slot, item);
        } else {
            prototype->vtable_->make_item(prototype, item);
            item.set_add_info(refill.refill_count);
            item.item_id_ = refill.item_id;
            updated_slot = getCurCharacInvenW()->insertItemIntoInventory(
                item, static_cast<eItemAddReason>(13), true, false);
        }

        if (updated_slot >= 0) {
            SendUpdateItemList(static_cast<eSendTarget>(1),
                               static_cast<ENUM_ITEMSPACE>(0),
                               updated_slot);
        }
    }
}
