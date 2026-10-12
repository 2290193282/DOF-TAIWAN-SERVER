#include "old_game/economy/inventory/inventory.h"
#include "common/config.h"

#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/user_history_log_abi.h"
#include "old_game/economy/inventory/item.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/system_time.h"

#include <utility>

namespace {

enum InventoryTypeValue {
    kEquipment = 0,
    kInventory = 1,
    kAvatar = 2,
    kCreature = 3,
};

bool IsIdentified(const Inven_Item& item) {
    return item.amplify_option_.isIdentified() == 1;
}

void SwapItemValues(Inven_Item* first, Inven_Item* second) {
    Inven_Item saved;
    saved.setCopy(*first);
    first->setCopy(*second);
    second->setCopy(saved);
}

}  // namespace

// IDA_EA: 0x84FC37E
// IDA_NAME: _ZN10CInventory11ChangeEquipE10INVEN_TYPEii
// DEMANGLED: CInventory::ChangeEquip(INVEN_TYPE,int,int)
int CInventory::ChangeEquip(INVEN_TYPE inventory_type,
                            int source_slot,
                            int equipment_slot) {
    const int source_type = static_cast<int>(inventory_type);
    if (CheckValidSlot(source_type, source_slot) != 1) {
        return 1;
    }

    if (source_type == kInventory) {
        if (equipment_slot <
            oldsrv::config::kOrdinaryEquipmentTypeFirst) {
            return 21;
        }

        const Inven_Item item = GetInvenSlot(kInventory, source_slot);
        if (!item.isEmpty()) {
            const int protection = GlobalData::s_pSecuProtectionField->Check(
                parent_, static_cast<SECURITY_PROTCTION>(22));
            if (protection) {
                return protection;
            }
        }
        // Item lock protects destructive mutations (sale/drop/upgrade/etc.).
        // Moving an owned item into its equipment slot is reversible and the
        // original CMD19 wear path does not treat the lock as equipability.

        return move_item(static_cast<INVEN_TYPE>(kEquipment),
                         equipment_slot,
                         static_cast<INVEN_TYPE>(kInventory),
                         source_slot);
    }

    if (source_type == kAvatar) {
        if (equipment_slot > oldsrv::config::kAvatarEquipmentTypeLast) {
            return 21;
        }
        return move_item(static_cast<INVEN_TYPE>(kEquipment),
                         equipment_slot,
                         static_cast<INVEN_TYPE>(kAvatar),
                         source_slot);
    }
    return 0;
}

// IDA_EA: 0x8500688
// IDA_NAME: _ZN10CInventory9move_itemE10INVEN_TYPEiS0_i
// DEMANGLED: CInventory::move_item(INVEN_TYPE,int,INVEN_TYPE,int)
int CInventory::move_item(INVEN_TYPE source_type_arg,
                          int source_slot,
                          INVEN_TYPE destination_type_arg,
                          int destination_slot) {
    int source_type = static_cast<int>(source_type_arg);
    int destination_type = static_cast<int>(destination_type_arg);
    if (CheckValidSlot(source_type, source_slot) != 1 ||
        CheckValidSlot(destination_type, destination_slot) != 1) {
        return 17;
    }

    // The original normalizes any request that enters creature equipment so
    // the creature manager owns the item transition and persistence update.
    if (source_type <= kAvatar && destination_type == kCreature) {
        std::swap(source_type, destination_type);
        std::swap(source_slot, destination_slot);
    }

    Inven_Item* const source = GetInvenRef(source_type, source_slot);
    Inven_Item* const destination =
        GetInvenRef(destination_type, destination_slot);
    if (!source || !destination) {
        return 17;
    }

    if (source_type == kInventory) {
        if (destination_type == kInventory) {
            const int source_item_type = GetSlotItemType(source_slot);
            const int destination_item_type = GetSlotItemType(destination_slot);
            if (source_item_type == 11 || destination_item_type == 11 ||
                (source_item_type && destination_item_type &&
                 source_item_type != destination_item_type) ||
                (source_item_type && !destination->isEmpty() &&
                 destination->item_category_ != source_item_type) ||
                (destination_item_type && !source->isEmpty() &&
                 source->item_category_ != destination_item_type)) {
                return 17;
            }
            SwapItemValues(source, destination);
            return 0;
        }
        if (destination_type == kEquipment) {
            if (!CheckEquipable(*source, *destination) ||
                !IsIdentified(*source) || !IsIdentified(*destination) ||
                GetSlotItemType(source_slot) == 11) {
                return 17;
            }

            const unsigned int incoming_item_id =
                static_cast<unsigned int>(source->item_id_);
            if (incoming_item_id) {
                CItem* const item = G_CDataManager()->find_item(
                    incoming_item_id);
                if (!item || !parent_) {
                    return 17;
                }
                const CItemVTable* const item_vtable = item->vtable_;
                if (!item_vtable || item_vtable->is_avatar_item(item) ||
                    item_vtable->is_creature_item(item)) {
                    return 17;
                }
                const int equip_result =
                    IsEquipable(parent_, item, destination_slot);
                if (equip_result) {
                    return equip_result;
                }
                if (item_vtable->is_random_option(item) &&
                    source->chronicle_random_option_.random_option_.empty()) {
                    return 17;
                }
                const int unsealed_days =
                    item_vtable->period_after_unsealing(item);
                if (unsealed_days) {
                    const int expire_time =
                        GlobalData::s_systemTime_.getCurSec() +
                        86400 * unsealed_days;
                    if (expire_time) {
                        source->avatar_select_ = static_cast<uint16_t>(
                            static_cast<int16_t>(
                                (expire_time - 1151683200) / 86400));
                    }
                }
            }

            SwapItemValues(source, destination);
            // Equipment-space records are no longer sealed after a successful
            // equip.
            destination->seal_or_state_ = 0;
            return 0;
        }
        return 17;
    }

    if (source_type == kAvatar) {
        if (destination_type == kAvatar) {
            const int first_key = source->get_add_info();
            const int second_key = destination->get_add_info();
            SwapItemValues(source, destination);
            SendAvatarEvent(50,
                            source_slot +
                                oldsrv::config::kAvatarInventorySlotBase,
                            destination_slot +
                                oldsrv::config::kAvatarInventorySlotBase,
                            first_key,
                            second_key);
            return 0;
        }
        if (destination_type == kEquipment) {
            Inven_Item saved_avatar;
            saved_avatar.setCopy(*source);
            if (!CheckEquipable(*source, *destination)) {
                return 17;
            }

            if (destination->item_id_) {
                CItem* const outgoing_item = G_CDataManager()->find_item(
                    static_cast<unsigned int>(destination->item_id_));
                if (parent_ && outgoing_item) {
                    if (outgoing_item->vtable_->is_avatar_item(outgoing_item) !=
                        1) {
                        return 17;
                    }
                    const int equip_result =
                        IsEquipable(parent_, outgoing_item, destination_slot);
                    if (equip_result) {
                        return equip_result;
                    }
                }
            }

            const int avatar_add_info = source->get_add_info();
            const int equipment_add_info = destination->get_add_info();
            MoveClearAvatar(source_slot +
                                oldsrv::config::kAvatarInventorySlotBase,
                            avatar_add_info,
                            source->item_id_,
                            destination_slot,
                            equipment_add_info,
                            destination->item_id_);
            source->setCopy(*destination);
            destination->setCopy(saved_avatar);
            SendAvatarEvent(50,
                            source_slot +
                                oldsrv::config::kAvatarInventorySlotBase,
                            destination_slot,
                            avatar_add_info,
                            equipment_add_info);
            return 0;
        }
        return 17;
    }

    if (source_type == kCreature) {
        if (destination_type != kCreature && destination_type != kEquipment) {
            return 17;
        }
        user_creature::CCreatureMgr* const creatures = GetCreatureMgrW();
        if (!creatures ||
            creatures->SwapCreatureItem(destination,
                                        destination_slot,
                                        destination_type,
                                        source,
                                        source_slot,
                                        source_type) != 1) {
            return 17;
        }
        SwapItemValues(source, destination);
        return 0;
    }

    if (source_type == kEquipment) {
        if (destination_type == kInventory) {
            if (!CheckEquipable(*source, *destination) ||
                isEquipmentPlaceableAtInventory(destination_slot) != 1) {
                return 17;
            }

            CItem* const incoming_item = G_CDataManager()->find_item(
                static_cast<unsigned int>(destination->item_id_));
            if (parent_ && incoming_item) {
                const CItemVTable* const item_vtable = incoming_item->vtable_;
                if (item_vtable->is_avatar_item(incoming_item) ||
                    item_vtable->is_creature_item(incoming_item)) {
                    return 17;
                }
                const int equip_result =
                    IsEquipable(parent_, incoming_item, source_slot);
                if (equip_result) {
                    return equip_result;
                }
                if (item_vtable->is_random_option(incoming_item) &&
                    destination->chronicle_random_option_.random_option_.empty()) {
                    return 17;
                }
            }

            if (GetSlotItemType(destination_slot) == 11 ||
                !IsIdentified(*source) || !IsIdentified(*destination)) {
                return 17;
            }
            if (destination->seal_or_state_ &&
                ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
                    parent_, RestrictCategory::AuthMobile, 14)) {
                return 209;
            }

            Inven_Item saved_equipment;
            saved_equipment.setCopy(*source);
            source->setCopy(*destination);
            if (parent_ && source->seal_or_state_) {
                CUser_history_log(parent_)->seal_Item(
                    source->item_id_,
                    static_cast<short>(source->GetReSealCount()),
                    static_cast<eSealItemReason>(1));
            }
            source->seal_or_state_ = 0;

            if (incoming_item &&
                incoming_item->vtable_->period_after_unsealing(incoming_item)) {
                const int expire_time =
                    GlobalData::s_systemTime_.getCurSec() +
                    86400 * incoming_item->vtable_->period_after_unsealing(
                        incoming_item);
                if (expire_time) {
                    source->avatar_select_ = static_cast<uint16_t>(
                        static_cast<int16_t>(
                            (expire_time - 1151683200) / 86400));
                }
            }
            destination->setCopy(saved_equipment);
            return 0;
        }
        if (destination_type == kAvatar) {
            if (!CheckEquipable(*source, *destination)) {
                return 17;
            }

            if (destination->item_id_) {
                CItem* const incoming_item = G_CDataManager()->find_item(
                    static_cast<unsigned int>(destination->item_id_));
                if (parent_ && incoming_item) {
                    if (incoming_item->vtable_->is_avatar_item(incoming_item) !=
                            1 ||
                        incoming_item->vtable_->is_creature_item(incoming_item)) {
                        return 17;
                    }
                    const int equip_result =
                        IsEquipable(parent_, incoming_item, source_slot);
                    if (equip_result) {
                        return equip_result;
                    }
                }
            }

            const int equipment_add_info = source->get_add_info();
            const int avatar_add_info = destination->get_add_info();
            MoveClearAvatar(destination_slot +
                                oldsrv::config::kAvatarInventorySlotBase,
                            avatar_add_info,
                            destination->item_id_,
                            source_slot,
                            equipment_add_info,
                            source->item_id_);
            SwapItemValues(source, destination);
            SendAvatarEvent(50,
                            source_slot,
                            destination_slot +
                                oldsrv::config::kAvatarInventorySlotBase,
                            equipment_add_info,
                            avatar_add_info);
            return 0;
        }
        return 17;
    }

    return 17;
}
