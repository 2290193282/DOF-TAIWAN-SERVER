#include "old_game/economy/inventory/inventory_load_verification.h"
#include "common/config.h"

#include "common/manager/data_manager.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_list.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/arad_data_manager.h"
#include "old_game/player/core/cuser.h"

namespace {

void AppendLock(std::vector<item_lock::stItemLockRef>& item_locks,
                const Inven_Item& item,
                uint8_t item_space,
                uint16_t slot) {
    const uint8_t lock = static_cast<uint8_t>(item.amplify_option_.GetLock());
    if (lock) {
        item_lock::stItemLockRef ref = {lock, item_space, slot};
        item_locks.push_back(ref);
    }
}

int VerifyAndLog(int slot,
                 const Inven_Item& item,
                 INVENTORY_TYPE inventory_type,
                 bool reject_sealed_item,
                 bool reject_stackable_item,
                 CUser* user,
                 uint32_t account_id,
                 const char* inventory_name) {
    const int result = _verifyItem(slot,
                                   &item,
                                   inventory_type,
                                   reject_sealed_item,
                                   reject_stackable_item,
                                   user);
    if (result) {
        _writeIncorrectItemInventoryForRecovery(account_id,
                                                item,
                                                inventory_name,
                                                result,
                                                slot);
    }
    return result;
}

}  // namespace

// IDA_EA: 0x84EFCDD
// IDA_NAME: _Z11_verifyItemiPK10Inven_Item14INVENTORY_TYPEbbP5CUser
// DEMANGLED: _verifyItem(int,Inven_Item const*,INVENTORY_TYPE,bool,bool,CUser*)
int _verifyItem(int slot,
                const Inven_Item* item,
                INVENTORY_TYPE inventory_type,
                bool reject_sealed_item,
                bool reject_stackable_item,
                CUser* user) {
    if (!item->item_id_) {
        if (item->seal_or_state_ || item->item_category_ || item->GetItemAttr() ||
            item->add_info_ || item->avatar_select_ || item->extend_info_ ||
            item->amplify_option_.GetLock()) {
            return 5474;
        }
        return 0;
    }

    CItem* const prototype = G_CDataManager()->find_item(item->item_id_);
    if (!prototype) return 5494;
    if (item->item_id_ != prototype->get_index()) return 5498;
    if (item->seal_or_state_ && reject_sealed_item) return 5501;

    if (prototype->is_stackable()) {
        if (reject_stackable_item) return 5506;
        const uint8_t category = item->item_category_;
        if (category > 10u || ((1u << category) & 0x69cu) == 0u) return 5519;
        const int stack_count = item->add_info_;
        const int stack_limit =
            static_cast<const CStackableItem*>(prototype)->getStackableLimit();
        if (stack_count > stack_limit || stack_count < 0) return 5534;
    } else {
        const uint8_t category = item->item_category_;
        if (category > 6u) {
            if (category != 8u) return 5559;
            if (inventory_type != static_cast<INVENTORY_TYPE>(0)) return 5556;
        } else if (category >= 5u) {
            if (inventory_type != static_cast<INVENTORY_TYPE>(0) &&
                inventory_type != static_cast<INVENTORY_TYPE>(3)) {
                return 5552;
            }
        } else if (category != 1u) {
            return 5559;
        }

        if (prototype->get_rarity() <= 1u && item->GetReSealCount()) return 5570;
        const int attach_type = prototype->GetAttachType();
        if (attach_type >= 0 && attach_type <= 5 &&
            ((1u << attach_type) & 0x27u) != 0u) {
            if (item->GetReSealCount()) return 5593;
            if (item->seal_or_state_) return 5601;
        }
    }

    if (inventory_type == static_cast<INVENTORY_TYPE>(1)) {
        if (!user->getCurCharacR()) return 5616;
        const int slot_item_type = user->getCurCharacInvenR()->GetSlotItemType(slot);
        if (slot_item_type && item->item_category_ != slot_item_type) return 5624;
    }
    return 0;
}

// IDA_EA: 0x84F0075
// IDA_NAME: _Z39_writeIncorrectItemInventoryForRecoveryjRK10Inven_ItemPKcii
// DEMANGLED: _writeIncorrectItemInventoryForRecovery(unsigned int,Inven_Item const&,char const*,int,int)
int _writeIncorrectItemInventoryForRecovery(uint32_t account_id,
                                            const Inven_Item& item,
                                            const char* inventory_name,
                                            int error_line,
                                            int slot) {
    LogManager::logFormat(
        1,
        "InterDispatcher.cpp",
        "void _writeIncorrectItemInventoryForRecovery(unsigned int, const Inven_Item&, const char*, int, int)",
        0x1645,
        "ERROR_ITEM_LOG %u,%d,%d,%u,%d,%d,%d,%u,%d - %s slot(%d), error_line(%d) \\n",
        account_id,
        static_cast<int>(item.seal_or_state_),
        static_cast<int>(item.item_category_),
        static_cast<uint32_t>(item.item_id_),
        item.GetItemAttr(),
        item.add_info_,
        static_cast<int>(item.avatar_select_),
        static_cast<uint32_t>(item.extend_info_),
        item.amplify_option_.GetLock(),
        inventory_name,
        slot,
        error_line);
    return 0;
}

// IDA_EA: 0x84C3DA5
// IDA_NAME: _Z20meterial_item_changeiP18SIG_LOAD_INVENTORYP10Inven_Item
// DEMANGLED: meterial_item_change(int,SIG_LOAD_INVENTORY*,Inven_Item*)
int meterial_item_change(int /*slot*/,
                         SIG_LOAD_INVENTORY* payload,
                         Inven_Item* item) {
    if (!payload || !item) {
        LogManager::logFormat(
            1,
            "InterDispatcher.cpp",
            "void meterial_item_change(int, SIG_LOAD_INVENTORY*, Inven_Item*)",
            0x1604,
            "selected_item | pInfo is null");
        return 0;
    }

    ExceptionItem* const exceptions =
        ARAD::Singleton<ARAD::Arad_DataManager>::Get()->getExceptionItem();
    if (!exceptions->find_index(item->item_id_) || item->item_category_ != 3u) {
        return 0;
    }

    item->item_category_ = 10u;
    for (int destination_slot = oldsrv::config::kMainInventoryEtcSlotFirst;
         destination_slot <= oldsrv::config::kMainInventoryEtcSlotLast;
         ++destination_slot) {
        Inven_Item& destination = payload->inventory[destination_slot];
        if (destination.isEmpty()) {
            destination.setCopy(*item);
            return static_cast<int>(reinterpret_cast<uintptr_t>(item->reset()));
        }
        if (destination.item_id_ == item->item_id_) {
            destination.add_info_ += item->add_info_;
            return static_cast<int>(reinterpret_cast<uintptr_t>(item->reset()));
        }
    }
    LogManager::logFormat(
        1,
        "InterDispatcher.cpp",
        "void meterial_item_change(int, SIG_LOAD_INVENTORY*, Inven_Item*)",
        0x161d,
        "new_meterial_slot_index overflow");
    return 0;
}

// IDA_EA: 0x84C3F2C
// IDA_NAME: _Z25cago_meterial_item_changeiP18SIG_LOAD_INVENTORYP10Inven_Item
// DEMANGLED: cago_meterial_item_change(int,SIG_LOAD_INVENTORY*,Inven_Item*)
void cago_meterial_item_change(int /*slot*/,
                               SIG_LOAD_INVENTORY* payload,
                               Inven_Item* item) {
    if (!payload || !item) return;
    ExceptionItem* const exceptions =
        ARAD::Singleton<ARAD::Arad_DataManager>::Get()->getExceptionItem();
    if (exceptions->find_index(item->item_id_) && item->item_category_ == 3u) {
        item->item_category_ = 10u;
    }
}

// IDA_EA: 0x84F0145
// IDA_NAME: _Z16_verifyInventoryP18SIG_LOAD_INVENTORYRSt6vectorIN9item_lock13stItemLockRefESaIS3_EEP5CUser
// DEMANGLED: _verifyInventory(SIG_LOAD_INVENTORY*,std::vector<item_lock::stItemLockRef>&,CUser*)
int _verifyInventory(SIG_LOAD_INVENTORY* payload,
                     std::vector<item_lock::stItemLockRef>& item_locks,
                     CUser* user) {
    const uint32_t account_id = payload->charac_no;
    for (int slot = 0; slot <= 11; ++slot) {
        const Inven_Item& item = payload->expanded_equipment.expand_equipslot_items[slot];
        AppendLock(
            item_locks,
            item,
            18u,
            static_cast<uint16_t>(
                slot + oldsrv::config::kOrdinaryEquipmentTypeFirst));
        if (VerifyAndLog(slot, item, static_cast<INVENTORY_TYPE>(0), true, true,
                         user, account_id, "fairpvp_equip")) return 0;
    }
    for (int slot = oldsrv::config::kOrdinaryEquipmentTypeFirst;
         slot <= oldsrv::config::kOrdinaryEquipmentTypeLast;
         ++slot) {
        const Inven_Item& item = payload->equipment[
            slot - oldsrv::config::kOrdinaryEquipmentTypeFirst];
        AppendLock(item_locks, item, 3u, static_cast<uint16_t>(slot));
        if (VerifyAndLog(slot, item, static_cast<INVENTORY_TYPE>(0), true, true,
                         user, account_id, "equip")) return 0;
    }
    for (int slot = 0; slot <= oldsrv::config::kMainInventoryLastSlot; ++slot) {
        Inven_Item& item = payload->inventory[slot];
        AppendLock(item_locks, item, 0u, static_cast<uint16_t>(slot));
        if (item.item_id_) meterial_item_change(slot, payload, &item);
        if (VerifyAndLog(slot, item, static_cast<INVENTORY_TYPE>(1), false, false,
                         user, account_id, "inven")) return 0;
    }
    for (int slot = 0; slot <= oldsrv::config::kCargoInventoryLastSlot; ++slot) {
        Inven_Item& item = payload->cargo[slot];
        AppendLock(item_locks, item, 2u, static_cast<uint16_t>(slot));
        if (item.item_id_) cago_meterial_item_change(slot, payload, &item);
        if (VerifyAndLog(slot, item, static_cast<INVENTORY_TYPE>(2), false, false,
                         user, account_id, "cargo")) return 0;
    }
    for (int slot = 0;
         slot <= oldsrv::config::kCreatureInventoryLastSlot;
         ++slot) {
        const Inven_Item& item = payload->creature_inventory[slot];
        AppendLock(item_locks, item, 7u, static_cast<uint16_t>(slot));
        if (VerifyAndLog(slot, item, static_cast<INVENTORY_TYPE>(3), false, false,
                         user, account_id, "creature")) return 0;
    }
    for (int slot = oldsrv::config::kAvatarEquipmentTypeFirst;
         slot <= oldsrv::config::kAvatarEquipmentTypeLast;
         ++slot) {
        AppendLock(item_locks, payload->avatar_inventory[slot], 3u,
                   static_cast<uint16_t>(slot));
    }
    for (int slot = 0;
         slot <= oldsrv::config::kAvatarInventoryLastSlot;
         ++slot) {
        AppendLock(item_locks, payload->avatar_equipment[slot], 1u,
                   static_cast<uint16_t>(slot));
    }
    return 1;
}
