#include "old_game/player/core/cuser.h"

#include <utility>
#include <vector>

#include "old_game/economy/inventory/item.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"

// IDA_EA: 0x8689576
// IDA_NAME: _ZN5CUser23EventDungeonClearRewardERKSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: CUser::EventDungeonClearReward(std::vector<std::pair<int,int> > const&)
char CUser::EventDungeonClearReward(
    const std::vector<std::pair<int, int> >& rewards) {
    for (std::vector<std::pair<int, int> >::const_iterator it =
             rewards.begin();
         it != rewards.end(); ++it) {
        const int item_id = it->first;
        const int add_count = it->second;
        CItem* const prototype = G_CDataManager()->find_item(item_id);
        if (!prototype) {
            return 0;
        }

        Inven_Item item;
        item.reset();
        int slot = getCurCharacInvenR()->GetInvenData(item_id, item);
        if (slot >= 0) {
            const int previous_count = item.get_add_info();
            history_log().ItemAdd(static_cast<INVEN_TYPE>(1),
                                  add_count,
                                  previous_count,
                                  item,
                                  static_cast<eItemAddReason>(13));
            item.set_add_info(previous_count + add_count);
            getCurCharacInvenW()->update_item(
                static_cast<INVEN_TYPE>(1), slot, item);
        } else {
            prototype->vtable_->make_item(prototype, item);
            item.set_add_info(add_count);
            item.item_id_ = item_id;
            slot = getCurCharacInvenW()->insertItemIntoInventory(
                item, static_cast<eItemAddReason>(13), true, false);
        }

        if (slot >= 0) {
            SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                               static_cast<ENUM_ITEMSPACE>(0),
                               slot);
        }
    }
    return 1;
}
