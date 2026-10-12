#include "old_game/economy/inventory/inter_load_inven.h"

#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/instance_rental_system_abi.h"
#include "old_game/economy/inventory/item.h"
#include "old_data/db/inventory/load_inventory_payload.h"
#include "old_game/economy/inventory/inventory.h"

#include "common/config.h"
#include "old_game/economy/inventory/inventory_load_verification.h"
#include "old_game/economy/inventory/item_lock_runtime.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "infra/log/format/log_manager.h"
#include "old_game/player/core/character_dispatchers.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/premium_runtime.h"
#include "old_game/world/region/game_world.h"

#include <algorithm>
#include <cstring>
#include <vector>

namespace {

void ReturnToInventoryOrMail(CUser* user,
                             CInventory* inventory,
                             const Inven_Item& item) {
    if (inventory->insertItemIntoInventory(item,
                                           static_cast<eItemAddReason>(14),
                                           false,
                                           false) < 0) {
        WongWork::CMailBoxHelper::ReqDBSendNewMail(
            user,
            item,
            0u,
            static_cast<unsigned int>(user->getCurCharacNo()),
            true,
            0,
            0);
    }
}

InstanceRentalSystem::RentInput::RentInElem MakePcRoomRentElement(
    const SIG_SAVE_PC_ROOM_RENT_INFO& record) {
    InstanceRentalSystem::RentInput::RentInElem element;
    element.section_index = record.slot;
    element.package_index = record.rent_type;
    element.equip_flag = record.state != 0;
    element.expire_time = record.expire_time;
    return element;
}

InstanceRentalSystem::RentInput::RentInElem MakePcRoomRentElement(
    const InstanceRentalSystem::RentalInfo& record) {
    InstanceRentalSystem::RentInput::RentInElem element;
    element.section_index = record.section_index;
    element.package_index = record.package_index;
    element.equip_flag = record.equip_flag != 0;
    element.expire_time = record.expire_time;
    return element;
}

InstanceRentalSystem::RentInput::RentInElem MakeDimensionRentElement(
    const SIG_SAVE_DIMENSION_RENT_INFO& record) {
    InstanceRentalSystem::RentInput::RentInElem element;
    element.section_index = record.rental_type;
    element.package_index = record.inventory_slot;
    element.equip_flag = record.equipped != 0;
    element.rent_type = record.rental_state;
    return element;
}

InstanceRentalSystem::RentInput::RentInElem MakeDimensionRentElement(
    const InstanceRentalSystem::RentalInfo& record) {
    InstanceRentalSystem::RentInput::RentInElem element;
    element.section_index = record.section_index;
    element.package_index = record.package_index;
    element.equip_flag = record.equip_flag != 0;
    element.rent_type = record.rent_type;
    return element;
}

}  // namespace

// IDA_EA: 0x84C3F92
// IDA_NAME: _ZN15Inter_LoadInven12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadInven::dispatch_sig(CUser *,char *,int)
int Inter_LoadInven::dispatch_sig(CUser* user, char* raw_payload,
                                  int payload_size) {
    if (!user || !raw_payload ||
        payload_size != static_cast<int>(sizeof(SIG_LOAD_INVENTORY))) {
        return 0;
    }
    SIG_LOAD_INVENTORY* const payload =
        reinterpret_cast<SIG_LOAD_INVENTORY*>(raw_payload);
    LogManager::logFormat(
        1,
        "Inter_LoadInven.cpp",
        "Inter_LoadInven::dispatch_sig",
        6190,
        "[CHARVIEW][INVEN_ENTER] state=%d lock=%d section1=%d section11=%d payload_size=%d result=%u",
        user->get_state(),
        user->isLocked4DataLoad(),
        user->IsCharacInfoLoaded(static_cast<CACHE_SECTION>(1)) ? 1 : 0,
        user->IsCharacInfoLoaded(static_cast<CACHE_SECTION>(11)) ? 1 : 0,
        payload_size,
        static_cast<unsigned int>(payload->result));
    if (!user->get_state()) return 6197;
    if (payload->result == 1u) return 6204;
    if (!user->getCurCharacR()) return 6209;

    const int creature_detail_capacity = static_cast<int>(
        sizeof(payload->creature_detail) / sizeof(payload->creature_detail[0]));
    const int creature_period_capacity = static_cast<int>(
        sizeof(payload->creature_period) / sizeof(payload->creature_period[0]));
    if (payload->avatar_count < 0 ||
        payload->avatar_count > oldsrv::config::kAvatarDetailItemCount ||
        payload->creature_count < 0 ||
        payload->creature_count > creature_detail_capacity ||
        payload->creature_period_count < 0 ||
        payload->creature_period_count > creature_period_capacity) {
        return 0;
    }
    for (int index = 0; index < payload->creature_count; ++index) {
        const int slot = payload->creature_detail[index].slot;
        if (slot < 0 ||
            slot >= oldsrv::config::kCreatureInventoryItemCount) {
            return 0;
        }
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    inventory->set_inventory_capacity(static_cast<int>(payload->inventory_capacity));

    item_lock::CItemLock* const item_lock =
        static_cast<item_lock::CItemLock*>(user->GetCharacExpandData(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(2)));
    const int verify_result =
        _verifyInventory(payload, item_lock->GetItemLockRefVec(), user);
    LogManager::logFormat(
        1,
        "Inter_LoadInven.cpp",
        "Inter_LoadInven::dispatch_sig",
        6220,
        "[CHARVIEW][INVEN_VERIFY] result=%d",
        verify_result);
    if (verify_result != 1) {
        user->DisConnSig(static_cast<DISCONN_SIG>(36), true, 0);
        // Never install, cache, or continue world entry with a rejected
        // inventory.  The prior fall-through exposed the character-list
        // projection after scheduling the disconnect.
        return 0;
    }

    user_creature::CCreatureMgr* const creature_manager =
        inventory->GetCreatureMgrW();
    creature_manager->Reset();
    creature_manager->SetUser(user);
    for (int index = 0; index < payload->creature_count; ++index) {
        user_creature::CCreatureMgr* const current_manager =
            user->getCurCharacInvenW()->GetCreatureMgrW();
        current_manager->LoadCreatureItem(
            &payload->creature_inventory[payload->creature_detail[index].slot],
            &payload->creature_detail[index]);
    }
    for (int index = 0; index < payload->creature_period_count; ++index) {
        user->getCurCharacInvenW()->GetCreatureMgrW()->LoadCreatureTempPeriod(
            &payload->creature_period[index]);
    }

    if (user->SetInventory(payload) != 1) return 6258;
    if (payload->tag_charac_no != 0u &&
        user->get_charac_idx(payload->tag_charac_no) != -1 &&
        user->setTagCharacInven(&payload->tag_inventory) != 1) {
        return 6290;
    }

    unsigned long clear_avatar[
        oldsrv::config::kPersistedClearAvatarCount] = {};
    std::memcpy(clear_avatar,
                payload->clear_avatar,
                sizeof(payload->clear_avatar));
    inventory->SetClearAvatar(clear_avatar);
    for (int index = 0; index < payload->avatar_count; ++index) {
        const AvatarDetailInfo& detail = payload->avatar_detail[index];
        if (detail.clear_avatar_id) {
            const int clear_avatar_item_id = GetItemIdByAddInfo(
                payload->avatar_equipment,
                105,
                static_cast<int>(detail.clear_avatar_id));
            for (int slot = 0;
                 slot < oldsrv::config::kPersistedClearAvatarCount;
                 ++slot) {
                if (payload->avatar_inventory[slot].add_info_ ==
                    static_cast<int>(detail.ui_id)) {
                    inventory->SetClearAvatar(slot, clear_avatar_item_id);
                }
            }
        }
        inventory->GetAvatarItemMgrW()->RegistItem(detail, false);
    }
    if (payload->max_equip_level) {
        user->SetCurCharacMaxEquipLevel(payload->max_equip_level);
    }

    _CheckOverEquipItem(user);
    _CheckUsableExpertJobEquipment(user);
    user->EnableCharacInfo(static_cast<CACHE_SECTION>(1));
    LogManager::logFormat(
        1,
        "Inter_LoadInven.cpp",
        "Inter_LoadInven::dispatch_sig",
        6295,
        "[CHARVIEW][INVEN_SECTION1] lock=%d section1=%d section11=%d",
        user->isLocked4DataLoad(),
        user->IsCharacInfoLoaded(static_cast<CACHE_SECTION>(1)) ? 1 : 0,
        user->IsCharacInfoLoaded(static_cast<CACHE_SECTION>(11)) ? 1 : 0);

    if (G_GameWorld()->GetChannelType() == 7) {
        for (int slot = oldsrv::config::kOrdinaryEquipmentTypeFirst;
             slot <= oldsrv::config::kOrdinaryEquipmentTypeLast;
             ++slot) {
            inventory->GetInvenRef(0, slot)->SetUpgrade(0);
        }
        for (int slot = oldsrv::config::kMainInventoryQuickSlotFirst;
             slot <= oldsrv::config::kMainInventoryEquipmentSlotLast;
             ++slot) {
            Inven_Item* const item = inventory->GetInvenRef(1, slot);
            if (item->item_category_ == 1u &&
                G_CDataManager()->find_item(item->item_id_)) {
                item->SetUpgrade(0);
            }
        }
    }

    user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(2))
        ->loadData(user, reinterpret_cast<char*>(payload->item_locks));
    user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(9))
        ->loadData(user, reinterpret_cast<char*>(&payload->expanded_equipment));
    user->disableSaveInven();
    user->disableSaveCargo();
    _UpdateRentalItems(user, payload);
    _UpdateDimensionRentalItems(user, payload);
    // Signal 8 is now authoritative in CUser.  Only this point may release the
    // dependent signal-32 request; Inter_LoadEtc owns world insertion and the
    // final data-load unlock.
    const int queue_result =
        character_select_load_order::RequestEtcAfterInventory(
            user, payload->tag_charac_no);
    LogManager::logFormat(
        1,
        "Inter_LoadInven.cpp",
        "Inter_LoadInven::dispatch_sig",
        6300,
        "[CHARVIEW][INVEN_QUEUE32] result=%d lock=%d section1=%d section11=%d",
        queue_result,
        user->isLocked4DataLoad(),
        user->IsCharacInfoLoaded(static_cast<CACHE_SECTION>(1)) ? 1 : 0,
        user->IsCharacInfoLoaded(static_cast<CACHE_SECTION>(11)) ? 1 : 0);
    return queue_result == 1 ? 0 : 6301;
}

// IDA_EA: 0x84C455C
// IDA_NAME: _ZN15Inter_LoadInven18_UpdateRentalItemsEP5CUserP18SIG_LOAD_INVENTORY
// DEMANGLED: Inter_LoadInven::_UpdateRentalItems(CUser *,SIG_LOAD_INVENTORY *)
void Inter_LoadInven::_UpdateRentalItems(CUser* user,
                                         SIG_LOAD_INVENTORY* payload) {
    if (!user || !payload) return;
    const int channel_type = G_GameWorld()->GetChannelType();
    if (channel_type == 15 || channel_type == 16) {
        user->DeleteRentalItemInfo(1);
        return;
    }

    InstanceRentalSystem::RentInput input;
    input.rental_type = 1;
    input.force_pcroom = 1;
    if (user->CheckLoadRentalInfoFromExchange()) {
        const SIG_SAVE_PC_ROOM_RENT_INFO& record = payload->pc_room_rent.items[0];
        if (record.item_id) input.entries.push_back(MakePcRoomRentElement(record));
    } else {
        CUserRentalInfoList rental_infos;
        if (user->GetCurrentCharacRentalInfo(rental_infos, 1)) {
            for (CUserRentalInfoList::const_iterator it = rental_infos.begin();
                 it != rental_infos.end();
                 ++it) {
                input.entries.push_back(MakePcRoomRentElement(*it));
            }
        }
    }

    if (input.entries.empty()) {
        user->DeleteCurrentCharacRentalItemInfo(1);
        return;
    }
    InstanceRentalSystem::RentOutput output;
    if (!GlobalData::s_rentalSystem->Rent(*user, input, output)) {
        user->UpdateCurrentCharacRentalInfo(output, 1, true);
    }
}

// IDA_EA: 0x84C4858
// IDA_NAME: _ZN15Inter_LoadInven27_UpdateDimensionRentalItemsEP5CUserP18SIG_LOAD_INVENTORY
// DEMANGLED: Inter_LoadInven::_UpdateDimensionRentalItems(CUser *,SIG_LOAD_INVENTORY *)
void Inter_LoadInven::_UpdateDimensionRentalItems(CUser* user,
                                                  SIG_LOAD_INVENTORY* payload) {
    if (!user || !payload) return;
    const int channel_type = G_GameWorld()->GetChannelType();
    if (channel_type == 15 || channel_type == 16) {
        user->DeleteRentalItemInfo(2);
        return;
    }

    InstanceRentalSystem::RentInput input;
    input.rental_type = 2;
    if (user->CheckLoadRentalInfoFromExchange()) {
        for (int index = 0; index <= 10; ++index) {
            const SIG_SAVE_DIMENSION_RENT_INFO& record =
                payload->dimension_rent.items[index];
            if (record.item_id) {
                input.entries.push_back(MakeDimensionRentElement(record));
            }
        }
    } else {
        CUserRentalInfoList rental_infos;
        if (user->GetCurrentCharacRentalInfo(rental_infos, 2)) {
            for (CUserRentalInfoList::const_iterator it = rental_infos.begin();
                 it != rental_infos.end();
                 ++it) {
                input.entries.push_back(MakeDimensionRentElement(*it));
            }
        }
    }

    if (input.entries.empty()) {
        user->DeleteCurrentCharacRentalItemInfo(2);
        return;
    }
    InstanceRentalSystem::RentOutput output;
    if (!GlobalData::s_rentalSystem->Rent(*user, input, output)) {
        user->UpdateCurrentCharacRentalInfo(output, 2, true);
    }
}

// IDA_EA: 0x84C4B38
// IDA_NAME: _ZN15Inter_LoadInven30_CheckUsableExpertJobEquipmentEP5CUser
// DEMANGLED: Inter_LoadInven::_CheckUsableExpertJobEquipment(CUser *)
bool Inter_LoadInven::_CheckUsableExpertJobEquipment(CUser* user) {
    if (!user->getCurCharacR()) return true;

    CInventory* const inventory = user->getCurCharacInvenW();
    for (int slot = oldsrv::config::kOrdinaryEquipmentTypeFirst;
         slot <= oldsrv::config::kOrdinaryEquipmentTypeLast;
         ++slot) {
        const Inven_Item item = inventory->GetInvenSlot(0, slot);
        if (!item.item_id_) continue;
        CItem* const prototype = G_CDataManager()->find_item(item.item_id_);
        if (!prototype || prototype->is_stackable()) continue;

        const int expert_job_level = user->GetCurExpertJobLevel(
            user->GetCurCharacExpertJobExp());
        if (!prototype->isExpertJobUsable(
                static_cast<ENUM_EXPERT_JOB_TYPE>(
                    user->GetCurCharacExpertJobType()),
                expert_job_level)) {
            inventory->ResetSlot(0, slot);
            ReturnToInventoryOrMail(user, inventory, item);
        }
    }
    return false;
}

// IDA_EA: 0x84C4D38
// IDA_NAME: _ZN15Inter_LoadInven19_CheckOverEquipItemEP5CUser
// DEMANGLED: Inter_LoadInven::_CheckOverEquipItem(CUser *)
char Inter_LoadInven::_CheckOverEquipItem(CUser* user) {
    if (!user->needCurCharacCheckOverEquip() || !user->getCurCharacR()) {
        return 1;
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    for (int slot = oldsrv::config::kAvatarEquipmentTypeFirst;
         slot < oldsrv::config::kEquippedAvatarAndOrdinaryItemCount;
         ++slot) {
        const Inven_Item item = inventory->GetInvenSlot(0, slot);
        if (!item.item_id_) continue;
        CItem* const prototype = G_CDataManager()->find_item(item.item_id_);
        if (!prototype || prototype->is_stackable()) continue;

        const int premium_over_level =
            user->GetPremiumInfo()->GetOverEquipableLevel(
                static_cast<ENUM_EQUIPMENTTYPE>(slot));
        const int level_gap = user->GetCurCharacMaxEquipLevel() -
                              user->get_charac_level();
        const int over_equipable_level = std::max(premium_over_level, level_gap);
        const bool disallowed_level =
            !prototype->check_low_level(over_equipable_level +
                                        user->get_charac_level());
        const bool disallowed_max_level =
            prototype->GetAttachType() == 5 &&
            static_cast<CEquipItem*>(prototype)->GetUsableMaxLevel() <
                user->get_charac_level();
        if (!disallowed_level && !disallowed_max_level) continue;

        if (prototype->vtable_->is_avatar_item(prototype)) {
            const int empty_slot = inventory->get_empty_slot(
                static_cast<INVEN_TYPE>(2),
                static_cast<Inven_Item::ITEM_TYPE>(8));
            if (empty_slot != -1) {
                inventory->move_item(static_cast<INVEN_TYPE>(0),
                                     slot,
                                     static_cast<INVEN_TYPE>(2),
                                     empty_slot);
            }
        } else {
            inventory->ResetSlot(0, slot);
            ReturnToInventoryOrMail(user, inventory, item);
        }
    }
    return static_cast<char>(user->resetCurCharacOverEquipFlag());
}

// IDA_EA: 0x84E9D8A / 0x84E9DA6 / 0x84E9DD6
// IDA_NAME: _ZN15Inter_LoadInvenC2Ev / _ZN15Inter_LoadInvenD2Ev /
//           _ZN15Inter_LoadInvenD0Ev
// DEMANGLED: Inter_LoadInven::Inter_LoadInven(void) /
//            Inter_LoadInven::~Inter_LoadInven()
Inter_LoadInven::Inter_LoadInven() : ISigDispatcher() {}
Inter_LoadInven::~Inter_LoadInven() {}
