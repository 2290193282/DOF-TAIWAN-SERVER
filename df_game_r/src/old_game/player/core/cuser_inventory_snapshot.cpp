#include "old_game/player/core/cuser.h"

#include "common/config.h"
#include "old_data/db/character/tag_charac_info_payload.h"
#include "old_data/db/inventory/load_inventory_payload.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/avatar_coin.h"
#include "old_game/player/core/charac_expand_cdata.h"

#include <cstring>

namespace {

// IDA_EA: CUser::GetInventory @0x864D6DF/0x864D70F/0x864D73F.
// The original CInventory ABI exposes these three independently serialized
// storage classes only through the numeric GetInvenData selector.
const int kInventoryItems = 1;
const int kAvatarEquipmentItems = 2;
const int kCreatureItems = 3;
const int kEquipmentItems = 0;

const unsigned int kCreatureEquipmentTailIndex =
    oldsrv::config::kCreatureInventoryEquipmentFirst;
const unsigned int kCreatureEquipmentTailCount =
    oldsrv::config::kCreatureArtifactEquipmentTypeCount;

}  // namespace

// IDA_EA: 0x864D646
// IDA_NAME: _ZN5CUser12GetInventoryEP18SIG_LOAD_INVENTORY
// DEMANGLED: CUser::GetInventory(SIG_LOAD_INVENTORY *)
int CUser::GetInventory(SIG_LOAD_INVENTORY* payload) {
    CInventory* const inventory = getCurCharacInvenR();

    payload->money = static_cast<uint32_t>(inventory->get_money());
    payload->coin = static_cast<uint32_t>(inventory->GetCoin());
    payload->pay_coin = static_cast<uint32_t>(inventory->GetPayCoin());
    payload->event_coin = static_cast<uint32_t>(inventory->GetEventCoin());

    inventory->GetInvenData(kInventoryItems,
                            payload->inventory,
                            sizeof(payload->inventory));
    inventory->GetInvenData(kAvatarEquipmentItems,
                            payload->avatar_equipment,
                            sizeof(payload->avatar_equipment));
    inventory->GetInvenData(kCreatureItems,
                            payload->creature_inventory,
                            sizeof(payload->creature_inventory));

    Inven_Item equipment_items[oldsrv::config::kEquipmentTransferItemCount];
    inventory->GetInvenData(kEquipmentItems,
                            equipment_items,
                            sizeof(equipment_items));

    // The DB signal names the three ranges separately even though CInventory
    // keeps all 27 equipment records in one physical table.
    std::memcpy(payload->avatar_inventory,
                equipment_items,
                sizeof(payload->avatar_inventory));
    std::memcpy(payload->equipment,
                &equipment_items[oldsrv::config::kOrdinaryEquipmentTypeFirst],
                sizeof(payload->equipment));
    std::memcpy(&payload->creature_inventory[kCreatureEquipmentTailIndex],
                &equipment_items[
                    oldsrv::config::kCreatureArtifactEquipmentTypeFirst],
                kCreatureEquipmentTailCount * sizeof(Inven_Item));

    unsigned long clear_avatar[
        oldsrv::config::kPersistedClearAvatarCount] = {};
    inventory->GetClearAvatar(clear_avatar);
    std::memcpy(payload->clear_avatar,
                clear_avatar,
                sizeof(payload->clear_avatar));

    CCargo* const cargo = getCurCharacCargoR();
    payload->cargo_capacity = cargo->GetCapacity();
    cargo->get_cargo(payload->cargo);

    payload->avatar_coin = static_cast<uint32_t>(inventory->avatar_coin_.Get());
    payload->inventory_capacity = static_cast<uint32_t>(
        inventory->get_inventory_capacity());
    return 1;
}

// IDA_EA: 0x869067C
// IDA_NAME: _ZNK5CUser21getTagCharacInventoryEP21TAG_CHARAC_INVEN_DATA
// DEMANGLED: CUser::getTagCharacInventory(TAG_CHARAC_INVEN_DATA *)const
int CUser::getTagCharacInventory(TAG_CHARAC_INVEN_DATA* payload) const {
    if (!getTagCharacR()) {
        return 0;
    }

    CInventory* const inventory = getTagCharacInvenR();
    inventory->GetInvenData(kAvatarEquipmentItems,
                            payload->avatar_equipment,
                            sizeof(payload->avatar_equipment));

    Inven_Item equipment_items[oldsrv::config::kEquipmentTransferItemCount];
    inventory->GetInvenData(kEquipmentItems,
                            equipment_items,
                            sizeof(equipment_items));
    std::memcpy(payload->avatar_inventory,
                equipment_items,
                sizeof(payload->avatar_inventory));
    unsigned long clear_avatar[
        oldsrv::config::kPersistedClearAvatarCount] = {};
    inventory->GetClearAvatar(clear_avatar);
    std::memcpy(payload->clear_avatar,
                clear_avatar,
                sizeof(payload->clear_avatar));
    inventory->GetAvatarItemMgrR()->GetItem(payload->avatar_detail,
                                             &payload->avatar_count);

    payload->cargo_capacity = static_cast<uint32_t>(
        getTagCharacCargoR()->GetCapacity());
    return 1;
}

// IDA_EA: 0x864D4BA
// IDA_NAME: _ZN5CUser16GetInventoryInfoEP18SIG_LOAD_INVENTORY
// DEMANGLED: CUser::GetInventoryInfo(SIG_LOAD_INVENTORY *)
int CUser::GetInventoryInfo(SIG_LOAD_INVENTORY* payload) {
    payload->result = 0;

    GetCharacExpandDataR(static_cast<ENUM_CHARAC_EXPAND_TYPE>(2))
        ->getData(reinterpret_cast<char*>(payload->item_locks));
    GetCharacExpandDataR(static_cast<ENUM_CHARAC_EXPAND_TYPE>(9))
        ->getData(reinterpret_cast<char*>(&payload->expanded_equipment));

    const CInventory* const inventory = getCurCharacInvenR();
    if (GetInventory(payload) != 1 ||
        inventory->GetCreatureMgrR()->GetCreatureItem(payload) != 1 ||
        inventory->GetAvatarItemMgrR()->GetItem(payload->avatar_detail,
                                                 &payload->avatar_count) != 1) {
        payload->result = 1;
        return 0;
    }

    if (getTagCharacR()) {
        getTagCharacInventory(&payload->tag_inventory);
    }
    removeRentalItemInfoFromInvenLoadBuf(payload, 1);
    removeRentalItemInfoFromInvenLoadBuf(payload, 2);
    payload->max_equip_level = GetCurCharacMaxEquipLevel();
    return 1;
}
