#include "old_game/player/core/cuser.h"
#include "common/config.h"

#include "common/manager/data_manager.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_game/appearance/avatar/arad_avatar_runtime.h"
#include "old_game/pvp/match/online_preliminary_match_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/equipment/expand_equipslot_runtime.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/economy/mail/mail_trade_value_types.h"
#include "old_game/world/region/game_world.h"
#include "old_data/PVF/common/rdar_script.h"

namespace {

enum ItemSpace {
    kInven = 0,
    kAvatar = 1,
    kCargo = 2,
    kEquipment = 3,
    kTrade = 4,
    kCreature = 7,
    kAccountCargo = 12,
    kExpandEquipment = 18,
};

bool CopyItemAt(CUser* user, int item_space, int slot, Inven_Item& item) {
    CInventory* const inventory = user->getCurCharacInvenR();
    switch (item_space) {
        case kInven:
            if (inventory->CheckValidSlot(1, slot) != 1) return false;
            item.setCopy(inventory->GetInvenSlot(1, slot));
            return true;
        case kAvatar:
            if (inventory->CheckValidSlot(2, slot) != 1) return false;
            item.setCopy(inventory->GetInvenSlot(2, slot));
            return true;
        case kCargo: {
            CCargo* const cargo = user->getCurCharacCargoR();
            if (!cargo || !cargo->IsOperatorable(slot)) return false;
            item.setCopy(cargo->get_cargo_slot(slot));
            return true;
        }
        case kEquipment:
            if (inventory->CheckValidSlot(0, slot) != 1) return false;
            item.setCopy(inventory->GetInvenSlot(0, slot));
            return true;
        case kExpandEquipment: {
            const CExpandEquipslot* const expand =
                static_cast<const CExpandEquipslot*>(user->GetCharacExpandDataR(
                    static_cast<ENUM_CHARAC_EXPAND_TYPE>(9)));
            const Inven_Item* const expanded_item =
                expand ? expand->GetInvenRefR(static_cast<INVEN_TYPE>(5), slot) : 0;
            if (!expanded_item) return false;
            item.setCopy(*expanded_item);
            return true;
        }
        case kCreature:
            if (inventory->CheckValidSlot(3, slot) != 1) return false;
            item.setCopy(inventory->GetInvenSlot(3, slot));
            return true;
        case kAccountCargo: {
            CAccountCargo* const account_cargo = user->GetAccountCargo();
            if (!account_cargo || !account_cargo->CheckValidSlot(slot)) return false;
            item.setCopy(account_cargo->GetSlot(slot));
            return true;
        }
        default:
            return false;
    }
}

}  // namespace

// IDA_EA: 0x865F1E1
// IDA_NAME: _Z17_checkItemMovablePK5CUser14ENUM_ITEMSPACEimS2_im
// DEMANGLED: _checkItemMovable(CUser const*,ENUM_ITEMSPACE,int,unsigned long,ENUM_ITEMSPACE,int,unsigned long)
// The 0627 client sends an add-info/instance key next to the source item id.
// The original identity gate does not accept or validate that field: it only
// compares the two item ids after mapping item spaces to inventory types.
bool _checkItemMovable(const CUser* user,
                       ENUM_ITEMSPACE source_space,
                       int source_slot,
                       unsigned long source_item_id,
                       ENUM_ITEMSPACE destination_space,
                       int destination_slot,
                       unsigned long destination_item_id) {
    if (source_space == static_cast<ENUM_ITEMSPACE>(2) ||
        destination_space == static_cast<ENUM_ITEMSPACE>(2)) {
        return true;
    }
    if (source_space == static_cast<ENUM_ITEMSPACE>(12) ||
        destination_space == static_cast<ENUM_ITEMSPACE>(12)) {
        return true;
    }

    const int source_type = GetInvenTypeFromItemSpace(source_space);
    const int destination_type = GetInvenTypeFromItemSpace(destination_space);
    unsigned long actual_source_item_id = 0;
    unsigned long actual_destination_item_id = 0;

    if (source_type == 5) {
        const CExpandEquipslot* const expanded =
            static_cast<const CExpandEquipslot*>(user->GetCharacExpandDataR(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(9)));
        const Inven_Item item = expanded->GetInvenSlot(
            static_cast<INVEN_TYPE>(source_type), source_slot);
        actual_source_item_id = static_cast<unsigned long>(
            static_cast<uint32_t>(item.item_id_));
    } else if (source_type == 6) {
        source_item_id = 0;
    } else {
        const Inven_Item item = user->getCurCharacInvenR()->GetInvenSlot(
            static_cast<INVEN_TYPE>(source_type), source_slot);
        actual_source_item_id = static_cast<unsigned long>(
            static_cast<uint32_t>(item.item_id_));
    }

    if (destination_type == 5) {
        const CExpandEquipslot* const expanded =
            static_cast<const CExpandEquipslot*>(user->GetCharacExpandDataR(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(9)));
        const Inven_Item item = expanded->GetInvenSlot(
            static_cast<INVEN_TYPE>(destination_type), destination_slot);
        actual_destination_item_id = static_cast<unsigned long>(
            static_cast<uint32_t>(item.item_id_));
    } else if (destination_type == 6) {
        destination_item_id = 0;
    } else {
        const Inven_Item item = user->getCurCharacInvenR()->GetInvenSlot(
            static_cast<INVEN_TYPE>(destination_type), destination_slot);
        actual_destination_item_id = static_cast<unsigned long>(
            static_cast<uint32_t>(item.item_id_));
    }

    return actual_source_item_id == source_item_id &&
           actual_destination_item_id == destination_item_id;
}

namespace {

int ResolveTrade(CUser* user, CTradeSpace*& trade) {
    trade = G_CGameManager()->GetTradeSpace(user->GetTradeSpace());
    if (!trade) return 1;
    if (trade->is_empty()) {
        user->SetTradeSpace(-1);
        return 21;
    }
    if (trade->check_trade_possibility() != 1) {
        trade->cancel_trade();
        G_CGameManager()->PutTradeSpace(trade);
        return 21;
    }
    return trade->IsLocked() ? 17 : 0;
}

int RegisterTradeItem(CUser* user,
                      int source_space,
                      int source_slot,
                      int count,
                      int* actual_slot) {
    CTradeSpace* trade = 0;
    const int state = ResolveTrade(user, trade);
    if (state) return state;
    const int result = trade->regist_item(user, source_space, source_slot, count);
    if (result < 0) return result == -4 ? 94 : 17;
    *actual_slot = result;
    return 0;
}

int MoveInventoryToCargo(CUser* user,
                         int source_slot,
                         int count,
                         int requested_slot,
                         int* actual_slot) {
    if (user->CheckInTrade()) return 19;
    CInventory* const inventory = user->getCurCharacInvenW();
    CCargo* const cargo = user->getCurCharacCargoW();
    Inven_Item moving = inventory->GetInvenSlot(1, source_slot);
    CItem* const prototype = G_CDataManager()->find_item(moving.item_id_);
    if (!prototype || prototype->vtable_->is_impossible_charac_cargo(prototype) ||
        (moving.item_id_ > 0x1963 && moving.item_id_ <= 0x1b57)) {
        return 23;
    }
    if (!moving.isEquipableItemType()) {
        if (moving.get_add_info() < count) return 17;
        moving.set_add_info(count);
        const int existing = cargo->check_item_exist(moving.item_id_);
        if (existing >= 0) requested_slot = existing;
    }
    if ((moving.isEquipableItemType() && !cargo->check_slot_empty(requested_slot)) ||
        (!moving.isEquipableItemType() && requested_slot < 0)) {
        requested_slot = cargo->get_empty_slot();
    }
    if (requested_slot < 0) return 4;
    const int planned = cargo->tryInsertItemSpecialSlot(moving, requested_slot);
    if (planned < 0) return 4;
    if (inventory->delete_item(static_cast<INVEN_TYPE>(1),
                               source_slot,
                               count,
                               static_cast<eItemDelReason>(7),
                               true) != 1) {
        return 17;
    }
    *actual_slot = cargo->insert_item_special_slot(moving, planned);
    return *actual_slot < 0 ? 17 : 0;
}

int MoveCargoToInventory(CUser* user,
                         int source_slot,
                         int count,
                         int* actual_slot) {
    if (user->CheckInTrade()) return 19;
    CCargo* const cargo = user->getCurCharacCargoW();
    CInventory* const inventory = user->getCurCharacInvenW();
    Inven_Item moving = cargo->get_cargo_slot(source_slot);
    if (!moving.isEquipableItemType()) moving.set_add_info(count);
    if (inventory->tryInsertItemIntoInventory(moving) < 0) return 17;
    if (cargo->delete_item(source_slot,
                           count,
                           static_cast<eItemDelReason>(7)) != 1) {
        return 17;
    }
    *actual_slot = inventory->insertItemIntoInventory(
        moving, static_cast<eItemAddReason>(7), true, false);
    return *actual_slot < 0 ? 17 : 0;
}

int MoveToAccountCargo(CUser* user,
                       int source_space,
                       int source_slot,
                       int count,
                       int requested_slot,
                       int* actual_slot) {
    if (user->CheckInTrade()) return 19;
    if (ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
            user, RestrictCategory::AuthMobile, 27)) {
        return 209;
    }
    const int protection = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(1));
    if (protection) return protection;
    if (!user->IsExistAccountCargo()) return 19;

    CAccountCargo* const account = user->GetAccountCargo();
    Inven_Item moving;
    if (!CopyItemAt(user, source_space, source_slot, moving) ||
        !account->CheckInsertCondition(moving)) return 17;
    if (!moving.isEquipableItemType()) {
        if (moving.get_add_info() < count) return 17;
        moving.set_add_info(count);
        const int existing = account->GetSpecificItemSlot(moving.item_id_);
        if (existing >= 0) {
            if (!account->CheckStackLimit(existing, moving.item_id_, count)) return 4;
            requested_slot = existing;
        }
    }
    if (requested_slot < 0 ||
        (moving.isEquipableItemType() && !account->CheckSlotEmpty(requested_slot))) {
        requested_slot = account->GetEmptySlot();
    }
    if (requested_slot < 0) return 4;

    int delete_ok = 0;
    if (source_space == kInven) {
        delete_ok = user->getCurCharacInvenW()->delete_item(
            static_cast<INVEN_TYPE>(1), source_slot, count,
            static_cast<eItemDelReason>(37), true);
    } else if (source_space == kCargo) {
        delete_ok = user->getCurCharacCargoW()->delete_item(
            source_slot, count, static_cast<eItemDelReason>(37));
    }
    if (delete_ok != 1) return 17;
    *actual_slot = account->InsertItem(moving, requested_slot);
    return *actual_slot < 0 ? 17 : 0;
}

int MoveAccountCargoOut(CUser* user,
                        int source_slot,
                        int count,
                        int destination_space,
                        int destination_slot,
                        int* actual_slot) {
    if (user->CheckInTrade()) return 19;
    if (!user->IsExistAccountCargo()) return 19;
    CAccountCargo* const account = user->GetAccountCargo();
    Inven_Item moving = account->GetSlot(source_slot);
    if (moving.isEmpty()) return 17;
    if (!moving.isEquipableItemType()) moving.set_add_info(count);

    if (destination_space == kInven) {
        CInventory* const inventory = user->getCurCharacInvenW();
        if (inventory->tryInsertItemIntoInventory(moving) < 0) return 17;
        if (account->DeleteItem(source_slot, count) != 1) return 17;
        *actual_slot = inventory->insertItemIntoInventory(
            moving, static_cast<eItemAddReason>(7), true, false);
        return *actual_slot < 0 ? 17 : 0;
    }
    if (destination_space == kCargo) {
        CCargo* const cargo = user->getCurCharacCargoW();
        if (!moving.isEquipableItemType()) {
            const int existing = cargo->check_item_exist(moving.item_id_);
            if (existing >= 0) destination_slot = existing;
        }
        if ((moving.isEquipableItemType() && !cargo->check_slot_empty(destination_slot)) ||
            destination_slot < 0) {
            destination_slot = cargo->get_empty_slot();
        }
        if (destination_slot < 0 ||
            cargo->tryInsertItemSpecialSlot(moving, destination_slot) < 0) {
            return 4;
        }
        if (account->DeleteItem(source_slot, count) != 1) return 17;
        *actual_slot = cargo->insert_item_special_slot(moving, destination_slot);
        return *actual_slot < 0 ? 17 : 0;
    }
    return 1;
}

}  // namespace

// IDA_EA: 0x865EED2
// IDA_NAME: _ZN5CUser12change_equipEisi
// DEMANGLED: CUser::change_equip(int,short,int)
int CUser::change_equip(int source_space,
                        int16_t source_slot,
                        int equipment_slot) {
    CInventory* const inventory = getCurCharacInvenW();
    if (source_space == kAvatar) {
        if (G_GameWorld()->IsCheckUnusableEquipmentChannel() &&
            online_preliminary::COnlinePreliminary::CheckUnusableEquipment(
                getCurCharacInvenR()->GetInvenRef(
                    static_cast<INVEN_TYPE>(2), source_slot)) != 1) {
            SendNotiPacketMessage(
                g_scriptStringManager_.findString(
                    4, "game_server_msg_146", 0),
                static_cast<ENUM_MESSAGE_TYPE>(0));
            return 1;
        }
        if (isCompetitionMercenary()) {
            return 179;
        }
        const int result = inventory->ChangeEquip(
            static_cast<INVEN_TYPE>(2), source_slot, equipment_slot);
        if (!result) {
            setAvatarTypeBonus(
                static_cast<char>(CalcMercenaryAvatarBonus()));
        }
        if (result > 0) {
            return result;
        }
        ARAD::Singleton<AvatarRechargeServer>::Get()
            ->CalcDurabilityChangeEquip(this, equipment_slot);
    } else if (source_space == kInven) {
        if (G_GameWorld()->IsCheckUnusableEquipmentChannel() &&
            online_preliminary::COnlinePreliminary::CheckUnusableEquipment(
                getCurCharacInvenR()->GetInvenRef(
                    static_cast<INVEN_TYPE>(1), source_slot)) != 1) {
            SendNotiPacketMessage(
                g_scriptStringManager_.findString(
                    4, "game_server_msg_146", 0),
                static_cast<ENUM_MESSAGE_TYPE>(0));
            return 1;
        }
        if (getCurChannelEquipslotSwitch() == 1 &&
            equipment_slot != oldsrv::config::kEquipmentTypeTitle) {
            const Inven_Item* const source = getCurCharacInvenR()->GetInvenRef(
                static_cast<INVEN_TYPE>(1), source_slot);
            CItem* const item = G_CDataManager()->find_item(source->item_id_);
            if (item &&
                item->IsEnableWorld(static_cast<ENUM_WORLD_TYPE>(1)) != 1) {
                return 1;
            }
        }
        const int result = inventory->ChangeEquip(
            static_cast<INVEN_TYPE>(1), source_slot, equipment_slot);
        if (result > 0) {
            return result;
        }
    } else if (source_space == kCargo) {
        if (CheckInTrade()) {
            return 19;
        }
    } else {
        return 1;
    }

    if (equipment_slot <= oldsrv::config::kEquipmentTypeTitle) {
        character_view().enableSaveCharacView();
    }
    send_equip(equipment_slot);
    return 0;
}

// IDA_EA: 0x865F382
// IDA_NAME: _ZN5CUser19change_equip_expandEiiiiPi
// DEMANGLED: CUser::change_equip_expand(int,int,int,int,int *)
int CUser::change_equip_expand(int source_space,
                               int source_slot,
                               int destination_space,
                               int destination_slot,
                               int* actual_destination_slot) {
    if (!actual_destination_slot) return 1;
    CExpandEquipslot* const expand = static_cast<CExpandEquipslot*>(
        GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(9)));
    if (!expand) return 1;

    if (source_space == kEquipment && destination_space == kInven &&
        source_slot >= oldsrv::config::kOrdinaryEquipmentTypeFirst &&
        source_slot <= oldsrv::config::kOrdinaryEquipmentTypeLast) {
        if (CheckInTrade()) return 19;
        const int result = change_equip(kInven, destination_slot, source_slot);
        if (!result) *actual_destination_slot = source_slot;
        return result;
    }
    if (source_space == kExpandEquipment && destination_space == kInven) {
        if (CheckInTrade()) return 19;
        const int result = expand->Change_Equip(this,
                                                kInven,
                                                destination_slot,
                                                kExpandEquipment,
                                                source_slot);
        if (!result) *actual_destination_slot = destination_slot;
        return result;
    }
    if (source_space == kInven && destination_space == kEquipment &&
        source_slot >= oldsrv::config::kMainInventoryQuickSlotFirst &&
        source_slot <= oldsrv::config::kMainInventoryEquipmentSlotLast &&
        destination_slot >= oldsrv::config::kOrdinaryEquipmentTypeFirst &&
        destination_slot <= oldsrv::config::kOrdinaryEquipmentTypeLast) {
        if (CheckInTrade()) return 19;
        const int result = change_equip(kInven, source_slot, destination_slot);
        if (!result) {
            *actual_destination_slot = destination_slot;
            if (GetParty()) GetParty()->Reset_party_overlapped_drop_ratio();
        }
        return result;
    }
    if (source_space == kInven && destination_space == kExpandEquipment &&
        source_slot >= oldsrv::config::kMainInventoryQuickSlotFirst &&
        source_slot <= oldsrv::config::kMainInventoryEquipmentSlotLast) {
        if (CheckInTrade()) return 19;
        const int result = expand->Change_Equip(this,
                                                kInven,
                                                source_slot,
                                                kExpandEquipment,
                                                destination_slot);
        if (!result) *actual_destination_slot = destination_slot;
        return result;
    }
    return 1;
}

// IDA_EA: 0x865F614
// IDA_NAME: _ZN5CUser26exchange_interspace_expandEiimiiimPicc
// DEMANGLED: CUser::exchange_interspace_expand(int,int,unsigned long,int,int,int,unsigned long,int *,char,char)
int CUser::exchange_interspace_expand(int source_space,
                                      int source_slot,
                                      unsigned long source_item_id,
                                      int source_count,
                                      int destination_space,
                                      int destination_slot,
                                      unsigned long destination_item_id,
                                      int* actual_destination_slot,
                                      char channel_equip_switch,
                                      char expand_equip_switch) {
    int mapped_destination = destination_space;
    if (getCurChannelEquipslotSwitch() == 1) {
        if (destination_space == kEquipment) mapped_destination = kExpandEquipment;
        else if (destination_space == kExpandEquipment) mapped_destination = kEquipment;
    }
    if (!_checkItemMovable(this,
                           static_cast<ENUM_ITEMSPACE>(source_space),
                           source_slot,
                           source_item_id,
                           static_cast<ENUM_ITEMSPACE>(mapped_destination),
                           destination_slot,
                           destination_item_id)) {
        if (actual_destination_slot) *actual_destination_slot = -1;
        return 21;
    }
    if (getCurChannelEquipslotSwitch() != channel_equip_switch ||
        getCurExpandEquipslotSwitch() != expand_equip_switch) {
        return 1;
    }
    if (!getCurChannelEquipslotSwitch() && !getCurExpandEquipslotSwitch()) {
        return change_equip_expand(source_space, source_slot, destination_space,
                                   destination_slot, actual_destination_slot);
    }
    if (getCurChannelEquipslotSwitch() == 1 &&
        !getCurExpandEquipslotSwitch()) {
        return change_equip_expand(source_space, source_slot, mapped_destination,
                                   destination_slot, actual_destination_slot);
    }
    return 1;
}

// IDA_EA: 0x865F7B0
// IDA_NAME: _ZN5CUser19exchange_interspaceEiimiiimPi
// DEMANGLED: CUser::exchange_interspace(int,int,unsigned long,int,int,int,unsigned long,int *)
int CUser::exchange_interspace(int source_space,
                               int source_slot,
                               unsigned long source_item_id,
                               int source_count,
                               int destination_space,
                               int destination_slot,
                               unsigned long destination_item_id,
                               int* actual_destination_slot) {
    if (!actual_destination_slot) return 1;
    *actual_destination_slot = -1;
    if (!_checkItemMovable(this,
                           static_cast<ENUM_ITEMSPACE>(source_space),
                           source_slot,
                           source_item_id,
                           static_cast<ENUM_ITEMSPACE>(destination_space),
                           destination_slot,
                           destination_item_id)) {
        return 21;
    }

    CInventory* const inventory = getCurCharacInvenW();
    if (source_space == kInven) {
        if (destination_space == kInven) {
            const int result = inventory->move_item(static_cast<INVEN_TYPE>(1), source_slot,
                                                    static_cast<INVEN_TYPE>(1), destination_slot);
            if (!result) *actual_destination_slot = destination_slot;
            return result;
        }
        if (destination_space == kCargo) return MoveInventoryToCargo(
            this, source_slot, source_count, destination_slot, actual_destination_slot);
        if (destination_space == kEquipment) {
            if (CheckInTrade()) return 19;
            const int result = change_equip(kInven, source_slot, destination_slot);
            if (!result && GetParty()) GetParty()->Reset_party_overlapped_drop_ratio();
            if (!result) *actual_destination_slot = destination_slot;
            return result;
        }
        if (destination_space == kTrade) return RegisterTradeItem(
            this, kInven, source_slot, source_count, actual_destination_slot);
        if (destination_space == kAccountCargo) return MoveToAccountCargo(
            this, kInven, source_slot, source_count, destination_slot, actual_destination_slot);
        return 1;
    }
    if (source_space == kAvatar) {
        if (destination_space == kAvatar) {
            const int result = inventory->move_item(static_cast<INVEN_TYPE>(2), source_slot,
                                                    static_cast<INVEN_TYPE>(2), destination_slot);
            if (!result) *actual_destination_slot = destination_slot;
            return result;
        }
        if (destination_space == kEquipment) {
            const int result = change_equip(kAvatar, source_slot, destination_slot);
            if (!result) *actual_destination_slot = destination_slot;
            return result;
        }
        if (destination_space == kTrade) return RegisterTradeItem(
            this, kAvatar, source_slot, source_count, actual_destination_slot);
        return 1;
    }
    if (source_space == kCargo) {
        if (destination_space == kCargo) {
            const int result = getCurCharacCargoW()->move_item(source_slot, destination_slot);
            if (result != 1) return 17;
            *actual_destination_slot = destination_slot;
            return 0;
        }
        if (destination_space == kInven) return MoveCargoToInventory(
            this, source_slot, source_count, actual_destination_slot);
        if (destination_space == kAccountCargo) return MoveToAccountCargo(
            this, kCargo, source_slot, source_count, destination_slot, actual_destination_slot);
        return 1;
    }
    if (source_space == kEquipment) {
        if (CheckInTrade() && destination_space != kTrade) return 19;
        if (destination_space == kInven) {
            const int result = change_equip(kInven, destination_slot, source_slot);
            if (!result) *actual_destination_slot = source_slot;
            return result;
        }
        if (destination_space == kAvatar) {
            const int result = change_equip(kAvatar, destination_slot, source_slot);
            if (!result) *actual_destination_slot = destination_slot;
            return result;
        }
        if (destination_space == kTrade) return RegisterTradeItem(
            this, kEquipment, source_slot, source_count, actual_destination_slot);
        if (destination_space == kCreature) {
            const int result = inventory->move_item(static_cast<INVEN_TYPE>(0), source_slot,
                                                    static_cast<INVEN_TYPE>(3), destination_slot);
            if (!result) { *actual_destination_slot = destination_slot; send_equip(destination_slot); }
            return result;
        }
        return 1;
    }
    if (source_space == kCreature) {
        if (destination_space == kCreature || destination_space == kEquipment) {
            const int target_type = destination_space == kCreature ? 3 : 0;
            const int result = inventory->move_item(static_cast<INVEN_TYPE>(3), source_slot,
                                                    static_cast<INVEN_TYPE>(target_type), destination_slot);
            if (!result) { *actual_destination_slot = destination_slot; if (destination_space == kEquipment) send_equip(destination_slot); }
            return result;
        }
        if (destination_space == kTrade) return RegisterTradeItem(
            this, kCreature, source_slot, source_count, actual_destination_slot);
        return 1;
    }
    if (source_space == kTrade) {
        CTradeSpace* trade = 0;
        const int state = ResolveTrade(this, trade);
        if (state) return state;
        if (destination_space != kInven && destination_space != kAvatar &&
            destination_space != kEquipment && destination_space != kCreature) return 1;
        const int result = trade->remove_item(this, source_slot, destination_space,
                                              destination_slot, source_count);
        if (result < 0) return 17;
        *actual_destination_slot = result;
        if (destination_space == kEquipment) send_equip(destination_slot);
        return 0;
    }
    if (source_space == kAccountCargo) {
        if (destination_space == kAccountCargo) {
            const int result = GetAccountCargo()->MoveItem(source_slot, destination_slot);
            if (result == 1) *actual_destination_slot = destination_slot;
            return result == 1 ? 0 : 17;
        }
        return MoveAccountCargoOut(this, source_slot, source_count,
                                   destination_space, destination_slot,
                                   actual_destination_slot);
    }
    return 1;
}
