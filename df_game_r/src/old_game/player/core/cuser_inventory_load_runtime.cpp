#include "old_game/player/core/cuser.h"

#include "common/config.h"
#include "old_data/db/inventory/load_inventory_payload.h"

#include <cstring>

namespace {

// CUser::SetInventory @0x864D160 lays the 11 avatar, 12 ordinary equipment,
// and four creature-artifact records contiguously into CInventory's 27-slot
// equipment table.  The three source arrays are distinct named fields in the
// DB signal; this local value is the original table, not a byte overlay.
struct EquipmentLoadTable {
    Inven_Item avatar[oldsrv::config::kAvatarEquipmentTypeCount];
    Inven_Item equipment[oldsrv::config::kOrdinaryEquipmentTypeCount];
    Inven_Item creature_artifact[
        oldsrv::config::kCreatureArtifactEquipmentTypeCount];
};

static_assert(sizeof(EquipmentLoadTable) ==
                  oldsrv::config::kEquipmentTransferItemCount *
                      sizeof(Inven_Item),
              "CUser::SetInventory equipment transfer must stay 27 records");

}  // namespace

// IDA_EA: 0x864D160
// IDA_NAME: _ZN5CUser12SetInventoryEP18SIG_LOAD_INVENTORY
// DEMANGLED: CUser::SetInventory(SIG_LOAD_INVENTORY *)
int CUser::SetInventory(SIG_LOAD_INVENTORY* payload) {
    if (!payload || !getCurCharacR()) {
        return 0;
    }

    EquipmentLoadTable equipment_load;
    std::memcpy(equipment_load.avatar,
                payload->avatar_inventory,
                sizeof(equipment_load.avatar));
    std::memcpy(equipment_load.equipment,
                payload->equipment,
                sizeof(equipment_load.equipment));
    std::memcpy(equipment_load.creature_artifact,
                &payload->creature_inventory[
                    oldsrv::config::kCreatureInventoryEquipmentFirst],
                sizeof(equipment_load.creature_artifact));

    CInventory* const inventory = getCurCharacInvenW();
    const Inven_Item* const incoming_equipment = equipment_load.avatar;
    for (int slot = oldsrv::config::kAvatarEquipmentTypeFirst;
         slot <= oldsrv::config::kAvatarEquipmentTypeLast;
         ++slot) {
        const Inven_Item* const current = inventory->GetInvenRef(0, slot);
        if (current && current->item_id_ != incoming_equipment[slot].item_id_) {
            character_view_.enableSaveCharacView();
            break;
        }
    }

    inventory->avatar_coin_.Set(payload->avatar_coin);
    inventory->set_money(payload->money);
    inventory->SetCoin(payload->coin);
    inventory->SetPayCoin(payload->pay_coin);
    inventory->SetEventCoin(payload->event_coin);

    inventory->SetInvenData(1, payload->inventory,
                            static_cast<int>(sizeof(payload->inventory)));
    inventory->SetInvenData(2, payload->avatar_equipment,
                            static_cast<int>(sizeof(payload->avatar_equipment)));
    inventory->SetInvenData(0, &equipment_load,
                            static_cast<int>(sizeof(equipment_load)));

    for (int slot = oldsrv::config::kCreatureArtifactEquipmentTypeFirst;
         slot <= oldsrv::config::kCreatureArtifactEquipmentTypeLast;
         ++slot) {
        inventory->GetCreatureMgrW()->LoadArtifact(
            slot, inventory->GetInvenRef(0, slot));
    }

    inventory->SetInvenData(3, payload->creature_inventory,
                            static_cast<int>(sizeof(payload->creature_inventory)));

    CCargo* const cargo = getCurCharacCargoW();
    if (payload->cargo_capacity) {
        cargo->SetCapacity(payload->cargo_capacity);
        cargo->set_cargo(payload->cargo);
    } else {
        cargo->SetCapacity(8);
    }
    return 1;
}
