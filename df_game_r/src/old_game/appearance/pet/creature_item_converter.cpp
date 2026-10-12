#include "old_game/appearance/pet/creature.h"

#include "common/manager/global_lifecycle.h"
#include "common/manager/server_utility.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/random_item_table.h"

namespace user_creature {

// IDA_EA: 0x8336EAE
// IDA_NAME: _ZN13user_creature22CCreatureItemConverter10SetEggItemEP10Inven_Item
// DEMANGLED: user_creature::CCreatureItemConverter::SetEggItem(Inven_Item *)
bool CCreatureItemConverter::SetEggItem(Inven_Item* item) {
    int item_id = item->item_id_;
    CItem* const prototype = G_CDataManager()->find_item(item_id);
    if (!prototype || prototype->is_stackable()) {
        return false;
    }

    const CEquipItem* const equipment =
        static_cast<const CEquipItem*>(prototype);
    const int sub_type = equipment->get_sub_type();
    if (sub_type == 1) {
        item->seal_or_state_ = 1u;
    } else if (sub_type == 2) {
        const RandomItemTable& random_items = equipment->random_item_table_;
        item_id = random_items.GetRandomItemIndex(
            static_cast<int>(get_rand_int(random_items.GetTotalWeight())));
        item->item_id_ = item_id;
        item->seal_or_state_ = 1u;
    }
    return true;
}

}  // namespace user_creature
