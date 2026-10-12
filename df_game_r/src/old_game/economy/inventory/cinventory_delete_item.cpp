#include "old_game/appearance/pet/creature.h"
#include "old_game/activity/schedule/freely_give_item.h"
#include "common/config.h"
#include "common/manager/user_history_log_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"

#include <stdint.h>


// IDA_EA: 0x850400C
// IDA_NAME: _ZN10CInventory11delete_itemE10INVEN_TYPEii14eItemDelReasonb
// DEMANGLED: CInventory::delete_item(INVEN_TYPE,int,int,eItemDelReason,bool)
// Active reconstructed-tree ABI:
// _ZN10CInventory11delete_itemE10INVEN_TYPEii14eItemDelReasonb.
int CInventory::delete_item(INVEN_TYPE inventory_type,
                            int slot,
                            int count,
                            eItemDelReason reason,
                            bool log_history) {
    if (static_cast<unsigned char>(
            CheckValidSlot(inventory_type, slot)) != 1 ||
        count <= 0) {
        return 0;
    }

    CUser* const parent = parent_;
    uint32_t removed_item_id = 0u;
    uint8_t removed_upgrade = 0u;
    if (inventory_type == 1 && slot != 0) {
        const Inven_Item& before = stackable_items_[slot];
        removed_item_id = static_cast<uint32_t>(before.item_id_);
        removed_upgrade = static_cast<uint8_t>(before.GetUpgrade());
    } else if (inventory_type == 0) {
        const Inven_Item& before = equipment_items_[slot];
        removed_item_id = static_cast<uint32_t>(before.item_id_);
        removed_upgrade = static_cast<uint8_t>(before.GetUpgrade());
    }
    if (inventory_type == 1) {
        if (slot == 0) {
            return use_money(
                count, static_cast<eMoneySubReason>(9), log_history);
        }

        Inven_Item* const item = &stackable_items_[slot];
        if (!item->item_id_) {
            return 0;
        }
        if (item->item_category_ == 1) {
            if (log_history) {
                if (parent) {
                    CUser_history_log(parent)->ItemDel(
                        inventory_type,
                        0,
                        1,
                        *item,
                        reason);
                } else {
                    cMyTrace trace(
                        "bool CInventory::delete_item(INVEN_TYPE, int, int, eItemDelReason, bool)",
                        6193,
                        5);
                    trace("History ERROR, m_pParent NULL, ItemDel ");
                }
            }
            item->reset();
        } else if (item->add_info_ == count) {
            if (log_history) {
                if (parent) {
                    CUser_history_log(parent)->ItemDel(
                        inventory_type,
                        0,
                        item->add_info_,
                        *item,
                        reason);
                } else {
                    cMyTrace trace(
                        "bool CInventory::delete_item(INVEN_TYPE, int, int, eItemDelReason, bool)",
                        6221,
                        5);
                    trace("History ERROR, m_pParent NULL, ItemDel ");
                }
            }
            item->reset();
        } else {
            const int before_count = item->add_info_;
            if (before_count - count <= 0) {
                LogManager::logFormat(
                    1,
                    "inventory.cpp",
                    "bool CInventory::delete_item(INVEN_TYPE, int, int, eItemDelReason, bool)",
                    0x186F,
                    "%s - m_InvenSlot[slot_no].add_info(%d) < count(%d)",
                    parent->getCurCharacName(),
                    before_count,
                    count);
                return 0;
            }
            item->add_info_ = before_count - count;
            if (log_history) {
                if (parent) {
                    CUser_history_log(parent)->ItemDel(
                        inventory_type,
                        item->add_info_,
                        count,
                        *item,
                        reason);
                } else {
                    cMyTrace trace(
                        "bool CInventory::delete_item(INVEN_TYPE, int, int, eItemDelReason, bool)",
                        6248,
                        5);
                    trace("History ERROR, m_pParent NULL, ItemDel ");
                }
            }
        }
    } else if (inventory_type > 1) {
        if (inventory_type == 2) {
            Inven_Item* const item = &avatar_items_[slot];
            if (!item->item_id_) {
                return 0;
            }
            if (log_history) {
                if (parent) {
                    const char* const ipg_agency_no =
                        avatar_item_manager_.GetIPGAgencyNo(item->add_info_);
                    CUser_history_log(parent)->AvatarItemDel(
                        item->item_id_,
                        item->add_info_,
                        ipg_agency_no,
                        static_cast<eAvatarItemDelReason>(reason));
                } else {
                    cMyTrace trace(
                        "bool CInventory::delete_item(INVEN_TYPE, int, int, eItemDelReason, bool)",
                        6270,
                        5);
                    trace("History ERROR, m_pParent NULL, ItemDel ");
                }
            }
            item->reset();
        } else if (inventory_type == 3) {
            Inven_Item* const item = &creature_items_[slot];
            if (!item->item_id_) {
                return 0;
            }
            if (item->isEquipableItemType()) {
                if (log_history) {
                    if (parent) {
                        CUser_history_log(parent)->CreatureItemDel(
                            inventory_type,
                            item->item_id_,
                            item->add_info_,
                            1,
                            item->item_category_,
                            reason);
                    } else {
                        cMyTrace trace(
                            "bool CInventory::delete_item(INVEN_TYPE, int, int, eItemDelReason, bool)",
                            6307,
                            5);
                        trace("History ERROR, m_pParent NULL, ItemDel ");
                    }
                }
                item->reset();
            } else if (item->add_info_ == count) {
                if (log_history) {
                    if (parent) {
                        CUser_history_log(parent)->CreatureItemDel(
                            inventory_type,
                            item->item_id_,
                            0,
                            item->add_info_,
                            item->item_category_,
                            reason);
                    } else {
                        cMyTrace trace(
                            "bool CInventory::delete_item(INVEN_TYPE, int, int, eItemDelReason, bool)",
                            6331,
                            5);
                        trace("History ERROR, m_pParent NULL, ItemDel ");
                    }
                }
                item->reset();
            } else {
                const int before_count = item->add_info_;
                if (before_count - count <= 0) {
                    LogManager::logFormat(
                        1,
                        "inventory.cpp",
                        "bool CInventory::delete_item(INVEN_TYPE, int, int, eItemDelReason, bool)",
                        0x18DA,
                        "%s - m_CreatureSlot[slot_no].add_info(%d) < count(%d)",
                        parent->getCurCharacName(),
                        before_count,
                        count);
                    return 0;
                }
                item->add_info_ = before_count - count;
                if (log_history) {
                    if (parent) {
                        CUser_history_log(parent)->CreatureItemDel(
                            inventory_type,
                            item->item_id_,
                            item->add_info_,
                            count,
                            item->item_category_,
                            reason);
                    } else {
                        cMyTrace trace(
                            "bool CInventory::delete_item(INVEN_TYPE, int, int, eItemDelReason, bool)",
                            6354,
                            5);
                        trace("History ERROR, m_pParent NULL, ItemDel ");
                    }
                }
            }
        }
    } else if (inventory_type == 0) {
        Inven_Item* const item = &equipment_items_[slot];
        if (!item->item_id_) {
            return 0;
        }
        if (log_history) {
            if (parent) {
                CUser_history_log(parent)->ItemDel(
                    inventory_type,
                    0,
                    1,
                    *item,
                    reason);
            } else {
                cMyTrace trace(
                    "bool CInventory::delete_item(INVEN_TYPE, int, int, eItemDelReason, bool)",
                    6161,
                    5);
                trace("History ERROR, m_pParent NULL, ItemDel ");
            }
        }
        item->reset();
        if (slot <= oldsrv::config::kAvatarEquipmentTypeLast) {
            SendAvatarEvent(51, slot, 0, 0, 0);
        }
    }
    bool removed_completely = false;
    if (removed_item_id != 0u) {
        if (inventory_type == 1 && slot != 0) {
            removed_completely =
                stackable_items_[slot].item_id_ !=
                static_cast<int32_t>(removed_item_id);
        } else if (inventory_type == 0) {
            removed_completely =
                equipment_items_[slot].item_id_ !=
                static_cast<int32_t>(removed_item_id);
        }
    }
    // Reasons 7 and 37 are character/account cargo transfers.  The item still
    // belongs to the account and must not be mistaken for a destroyed event
    // weapon.  Feature-owned conversions are suppressed by its local fence.
    if (parent && removed_completely && reason != static_cast<eItemDelReason>(7) &&
        reason != static_cast<eItemDelReason>(37)) {
        if (reason == static_cast<eItemDelReason>(13)) {
            freely_give_item::OnUpgradeDisjoint(
                parent, removed_item_id, removed_upgrade, true);
        }
        freely_give_item::OnEventWeaponRemoved(parent, removed_item_id);
    }
    return 1;
}
