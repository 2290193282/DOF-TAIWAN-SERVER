#include "old_game/economy/mail/mail_trade_runtime.h"

#include "old_game/economy/inventory/item.h"
#include "old_game/economy/equipment/item_option.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/progression/quest/quest_skill_expert_job.h"
#include "old_game/player/core/cuser.h"

namespace private_store {

// IDA_EA: 0x85C532C
// IDA_NAME: _ZN13private_store13CPrivateStore21CheckPrivateStoreDollEt
// DEMANGLED: private_store::CPrivateStore::CheckPrivateStoreDoll(unsigned short)
int CPrivateStore::CheckPrivateStoreDoll(uint16_t inventory_slot) {
    if (!seller) return 58;

    CInventory* const inventory = seller->getCurCharacInvenR();
    const Inven_Item doll = inventory->GetInvenSlot(1, inventory_slot);
    CItem* const item = G_CDataManager()->find_item(doll.getKey());
    if (!item || item->is_stackable() != 1) return 22;

    const CStackableItem* const stackable =
        static_cast<const CStackableItem*>(item);
    if (stackable->private_store_info_.shop_doll_item_index == -1) return 22;

    return doll.get_add_info() > 0 ? 0 : 22;
}

// IDA_EA: 0x85C6D48
// IDA_NAME: _ZN13private_store13CPrivateStore7AddItemERNS_16PrivateStoreItemEi
// DEMANGLED: private_store::CPrivateStore::AddItem(private_store::PrivateStoreItem &,int)
int CPrivateStore::AddItem(PrivateStoreItem& source, int request_index) {
    if (isDollCreated()) {
        CItem* const doll_item = G_CDataManager()->find_item(
            static_cast<int>(GetDollItemID()));
        if (!doll_item) {
            LogManager::logFormat(
                1, "PrivateStore.cpp",
                "virtual bool private_store::CPrivateStore::AddItem(private_store::PrivateStoreItem&, int)",
                0x550,
                "CPrivateStore::AddItem ERROR : shop_doll_item_id(%d)",
                GetDollItemID());
            return 0;
        }

        const CStackableItem* const stackable_doll =
            static_cast<const CStackableItem*>(doll_item);
        const int max_slot = G_CDataManager()->GetExpertJobScript(4)->GetShopDollSlot(
            stackable_doll->private_store_info_.shop_doll_item_index);
        if (max_slot <= 0) {
            LogManager::logFormat(
                1, "PrivateStore.cpp",
                "virtual bool private_store::CPrivateStore::AddItem(private_store::PrivateStoreItem&, int)",
                0x559,
                "CPrivateStore::AddItem ERROR : shop_doll_item_index(%d)",
                stackable_doll->private_store_info_.shop_doll_item_index);
            return 0;
        }
        if (source.store_slot >= max_slot) {
            LogManager::logFormat(
                1, "PrivateStore.cpp",
                "virtual bool private_store::CPrivateStore::AddItem(private_store::PrivateStoreItem&, int)",
                0x55f,
                "CPrivateStore::AddItem ERROR : slot_no(%d), shop_doll_item_index(%d), maxSlot(%d)",
                source.store_slot,
                stackable_doll->private_store_info_.shop_doll_item_index,
                max_slot);
            return 0;
        }
    }

    if (source.store_slot > 6) {
        LogManager::logFormat(
            1, "PrivateStore.cpp",
            "virtual bool private_store::CPrivateStore::AddItem(private_store::PrivateStoreItem&, int)",
            0x566, "CPrivateStore::AddItem slot_no(%d) ERROR", source.store_slot);
        return 0;
    }
    if (!seller) {
        LogManager::logFormat(
            1, "PrivateStore.cpp",
            "virtual bool private_store::CPrivateStore::AddItem(private_store::PrivateStoreItem&, int)",
            0x56c, "CPrivateStore::AddItem seller_ == NULL ERROR");
        return 0;
    }
    if (!seller->getCurCharacR()) {
        LogManager::logFormat(
            1, "PrivateStore.cpp",
            "virtual bool private_store::CPrivateStore::AddItem(private_store::PrivateStoreItem&, int)",
            0x572, "CPrivateStore::AddItem getCurCharacR() == NULL ERROR");
        return 0;
    }

    CInventory* const inventory = seller->getCurCharacInvenR();
    if (!inventory) {
        LogManager::logFormat(
            1, "PrivateStore.cpp",
            "virtual bool private_store::CPrivateStore::AddItem(private_store::PrivateStoreItem&, int)",
            0x579, "CPrivateStore::AddItem inventory == NULL ERROR");
        return 0;
    }

    const int inventory_type = GetInvenTypeFromItemSpace(
        static_cast<ENUM_ITEMSPACE>(source.inventory_space));
    const Inven_Item inventory_item = inventory->GetInvenSlot(
        inventory_type, source.inventory_slot);
    CItem* const item = G_CDataManager()->find_item(inventory_item.getKey());
    if (!item) {
        LogManager::logFormat(
            1, "PrivateStore.cpp",
            "virtual bool private_store::CPrivateStore::AddItem(private_store::PrivateStoreItem&, int)",
            0x582, "CPrivateStore::AddItem find_item() ERROR item_id(%d)",
            inventory_item.getKey());
        return 0;
    }
    if (item->GetCashPrice() > 0) {
        LogManager::logFormat(
            1, "PrivateStore.cpp",
            "virtual bool private_store::CPrivateStore::AddItem(private_store::PrivateStoreItem&, int)",
            0x5b8,
            "CPrivateStore::AddItem GetCashPrice() ERROR item_id(%d), cash_price(%d)",
            inventory_item.getKey(), item->GetCashPrice());
        return 0;
    }
    if (item->vtable_->is_avatar_item(item)) {
        LogManager::logFormat(
            1, "PrivateStore.cpp",
            "virtual bool private_store::CPrivateStore::AddItem(private_store::PrivateStoreItem&, int)",
            0x5be,
            "CPrivateStore::AddItem IsAvatarItem() ERROR item_id(%d), cash_price(%d)",
            inventory_item.getKey(), item->GetCashPrice());
        return 0;
    }
    if (inventory_item.upgrade_separate_.IsTradeRestriction()) return 0;
    if (source.store_slot >= 14 || request_index >= 14) return 0;
    if (request_index == 0) ResetItems();
    if (items[source.store_slot].state) return 0;
    if (source.inventory_space == 3) return 0;

    source.state = 1;
    items[source.store_slot].Reset();
    items[source.store_slot] = source;
    return 1;
}

}  // namespace private_store
