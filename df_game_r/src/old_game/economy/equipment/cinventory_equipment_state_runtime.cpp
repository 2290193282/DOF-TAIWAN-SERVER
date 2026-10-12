#include "old_game/economy/inventory/inventory.h"

#include "common/config.h"
#include "common/manager/data_manager.h"

namespace {

// IDA data at 0x8AE5B60: five equipment-subtype mastery values for each
// valid character job.  The original body only checks upper bounds before
// indexing this flat table, so the provider deliberately preserves that gate.
const int kCharacJobMastery[45] = {
    3, 2, 0, 3, 4,
    2, 0, 2, 3, 2,
    1, 1, 3, 0, 1,
    0, 0, 0, 1, 1,
    3, 4, 2, 0, 3,
    1, 1, 3, 0, 1,
    1, 1, 2, 1, 1,
    2, 0, 2, 3, 2,
    0, 0, 1, 0, 0,
};

}  // namespace

// IDA_EA: 0x850CE40
// IDA_NAME: _ZN10CInventory19checkEquipmentStateEiiRiS0_S0_S0_S0_S0_
// DEMANGLED: CInventory::checkEquipmentState(int,int,int &,int &,int &,int &,int &,int &)
void CInventory::checkEquipmentState(int charac_job,
                                     int first_grow_type,
                                     int& empty_equipment_count,
                                     int& mastery_mismatch_count,
                                     int& mastery_mismatch_state,
                                     int& unselected_avatar_count,
                                     int& creature_artifact_count,
                                     int& mastery_equipment_count) {
    if (charac_job >= oldsrv::config::kCharacterJobSwordman &&
        charac_job <= oldsrv::config::kCharacterJobAtMage &&
        first_grow_type <= 4) {
        const Inven_Item& weapon =
            equipment_items_[oldsrv::config::kEquipmentTypeWeapon];
        if (weapon.item_id_ != 0) {
            if (weapon.avatar_select_ == 0) {
                ++unselected_avatar_count;
            }
        } else {
            ++empty_equipment_count;
        }

        for (int slot = oldsrv::config::kEquipmentTypeCoat;
             slot <= oldsrv::config::kEquipmentTypeMagicStone;
             ++slot) {
            const Inven_Item& inventory_item = equipment_items_[slot];
            if (slot <= oldsrv::config::kEquipmentTypeRing) {
                if (inventory_item.item_id_ == 0) {
                    ++empty_equipment_count;
                }
                if (slot <= oldsrv::config::kEquipmentTypeWaist) {
                    CEquipItem* const item = static_cast<CEquipItem*>(
                        G_CDataManager()->find_item(inventory_item.item_id_));
                    if (item) {
                        if (inventory_item.avatar_select_ == 0) {
                            ++unselected_avatar_count;
                        }

                        const int subtype = item->get_sub_type();
                        if (charac_job ==
                                oldsrv::config::kCharacterJobPriest &&
                            first_grow_type == 3) {
                            if (subtype == 0 || subtype == 3 || subtype == 4) {
                                ++mastery_equipment_count;
                            } else {
                                mastery_mismatch_count += 2;
                                mastery_mismatch_state = 2;
                            }
                        } else if (kCharacJobMastery[
                                       5 * charac_job + first_grow_type] ==
                                   subtype) {
                            ++mastery_equipment_count;
                        } else if (kCharacJobMastery[5 * charac_job] ==
                                   subtype) {
                            ++mastery_mismatch_count;
                            mastery_mismatch_state =
                                mastery_mismatch_state == 2 ? 2 : 1;
                        } else {
                            mastery_mismatch_count += 2;
                            mastery_mismatch_state = 2;
                        }
                    }
                }
            } else if (inventory_item.item_id_ != 0) {
                ++creature_artifact_count;
            }
        }
    }
}
