#include "old_game/appearance/pet/creature.h"

#include "old_data/db/inventory/load_inventory_payload.h"

#include <cstring>

namespace user_creature {

// IDA_EA: 0x833B80C
// IDA_NAME: _ZNK13user_creature12CCreatureMgr15GetCreatureItemEP18SIG_LOAD_INVENTORY
// DEMANGLED: user_creature::CCreatureMgr::GetCreatureItem(SIG_LOAD_INVENTORY *)const
int CCreatureMgr::GetCreatureItem(SIG_LOAD_INVENTORY* inventory) const {
    std::memset(inventory->creature_detail,
                0,
                sizeof(inventory->creature_detail));

    // The original constructs this item once before walking the ordered map.
    // Each polymorphic saver receives the same local object and its matching
    // typed detail row.
    Inven_Item saved_item;
    int count = 0;
    for (CCreatureItemMap::const_iterator it = creature_items_.begin();
         it != creature_items_.end() && count < 141;
         ++it, ++count) {
        CCreatureItem* const item = it->second;
        item->SaveCreatureItem(&saved_item,
                               &inventory->creature_detail[count],
                               owner_);
        inventory->creature_detail[count].expiration_value =
            item->getExpireTime();
    }
    inventory->creature_count = count;

    count = 0;
    for (CCreatureTempPeriodMap::const_iterator it =
             temporary_periods_.begin();
         it != temporary_periods_.end() && count < 20;
         ++it, ++count) {
        inventory->creature_period[count].creature_id = it->first;
        inventory->creature_period[count].expiration_time = it->second;
    }
    inventory->creature_period_count = count;
    return 1;
}

}  // namespace user_creature
