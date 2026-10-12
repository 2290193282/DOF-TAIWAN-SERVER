#include "old_game/pvp/match/fair_pvp_match_runtime.h"

#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"

namespace fair_pvp {

// IDA_EA: 0x8562BDC
// IDA_NAME: _ZN8fair_pvp10CFairMatch22CheckUnusableEquipmentEPK10Inven_Item
// DEMANGLED: fair_pvp::CFairMatch::CheckUnusableEquipment(Inven_Item const*)
bool CFairMatch::CheckUnusableEquipment(const Inven_Item* item) {
    if (!item) {
        return false;
    }

    const int item_id = item->getKey();
    if (!item_id) {
        return true;
    }

    CItem* const definition = G_CDataManager()->find_item(item_id);
    if (!definition || definition->is_stackable()) {
        return false;
    }

    const CEquipItem* const equipment =
        static_cast<const CEquipItem*>(definition);
    const int usable_equipment_type = equipment->GetUsableEquipmentType();
    if (!usable_equipment_type) {
        return true;
    }

    ServerParameterScript* const parameters =
        G_CDataManager()->get_server_parameter_script();
    if (parameters->CheckUnusableEquipmentInfo(
            usable_equipment_type, definition->getUsableLevel()) != 1) {
        return false;
    }

    // The original calls CItem's named +0x0c item-type virtual slot before
    // checking the exact PVF item index in ServerParameterScript.
    const int item_type = definition->vtable_->get_item_type(definition);
    return parameters->CheckUnusableItem(
               item_type,
               static_cast<unsigned long>(definition->get_index())) == 1;
}

}  // namespace fair_pvp
