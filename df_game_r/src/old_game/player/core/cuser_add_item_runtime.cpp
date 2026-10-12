#include "old_game/player/core/cuser.h"

#include "old_game/economy/inventory/item.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"

// IDA_EA: 0x867B6D4
// IDA_NAME: _ZN5CUser7AddItemEii14eItemAddReasonR14ENUM_ITEMSPACEi
// DEMANGLED: CUser::AddItem(int,int,eItemAddReason,ENUM_ITEMSPACE &,int)
int CUser::AddItem(int item_id,
                   int count,
                   eItemAddReason reason,
                   ENUM_ITEMSPACE& item_space,
                   int upgrade) {
    if (!getCurCharacR() || CheckInTrade() || count < 0) {
        return -1;
    }

    CItem* const prototype = G_CDataManager()->find_item(item_id);
    if (!prototype) {
        return -1;
    }

    Inven_Item item;
    item.item_id_ = item_id;
    prototype->vtable_->make_item(prototype, item);
    if (prototype->is_stackable()) {
        item.set_add_info(count);
    }

    item_space = static_cast<ENUM_ITEMSPACE>(0);
    if (item.IsCreatureItemType()) {
        item_space = static_cast<ENUM_ITEMSPACE>(7);
    }
    if (upgrade > 0 && upgrade <= 31 && item.item_category_ == 1) {
        item.SetUpgrade(static_cast<uint8_t>(upgrade));
    }

    CInventory* const inventory = getCurCharacInvenW();
    const int slot = inventory->insertItemIntoInventory(item, reason, true, true);
    if (slot < 0) {
        return -1;
    }

    if (item.IsCreatureItemType()) {
        inventory->GetCreatureMgrW()->InsertCreatureItem(
            &item, slot, static_cast<int>(reason), prototype->getUsablePeriod(),
            prototype->getExpirationDate());
    }
    return slot;
}
