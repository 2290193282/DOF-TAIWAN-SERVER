#include "old_game/appearance/pet/creature.h"

#include <cstring>

#include "old_data/db/inventory/load_inventory_payload.h"
#include "old_game/economy/inventory/inventory.h"

namespace user_creature {

// IDA_EA: 0x83387F6
// IDA_NAME: _ZN13user_creature9CCreature16SaveCreatureItemEP10Inven_ItemP29SIG_CREATURE_ITEM_DETAIL_INFOP5CUser
// DEMANGLED: user_creature::CCreature::SaveCreatureItem(Inven_Item *,SIG_CREATURE_ITEM_DETAIL_INFO *,CUser *)
int CCreature::SaveCreatureItem(Inven_Item* item,
                                SIG_CREATURE_ITEM_DETAIL_INFO* detail,
                                CUser* user) {
    (void)user;
    detail->stomach = static_cast<int>(
        stomach_.GetStomachValue(state_, item_grow_state_ != 0));
    detail->experience = experience_;
    detail->ui_id = uid_;
    state_dirty_ =
        static_cast<unsigned char>(stomach_.IsSaveStomach(state_));
    item->item_id_ = item_id_;
    detail->slot = slot_no_;
    detail->creature_type = static_cast<uint8_t>(creature_item_type_);
    std::strncpy(detail->name, name_, 12u);
    detail->no_charge = no_charge_;
    detail->state = stat_;
    return 1;
}

// IDA_EA: 0x833F04C
// IDA_NAME: _ZN13user_creature9CCreature6IsSaveEv
// DEMANGLED: user_creature::CCreature::IsSave(void)
int CCreature::IsSave() {
    return state_dirty_;
}

// IDA_EA: 0x83390C8
// IDA_NAME: _ZN13user_creature4CEgg16SaveCreatureItemEP10Inven_ItemP29SIG_CREATURE_ITEM_DETAIL_INFOP5CUser
// DEMANGLED: user_creature::CEgg::SaveCreatureItem(Inven_Item *,SIG_CREATURE_ITEM_DETAIL_INFO *,CUser *)
int CEgg::SaveCreatureItem(Inven_Item* item,
                           SIG_CREATURE_ITEM_DETAIL_INFO* detail,
                           CUser* user) {
    (void)user;
    item->item_id_ = item_id_;
    detail->ui_id = uid_;
    detail->slot = slot_no_;
    detail->creature_type = static_cast<uint8_t>(creature_item_type_);
    detail->no_charge = no_charge_;
    detail->state = stat_;
    return 1;
}

// IDA_EA: 0x833F102
// IDA_NAME: _ZN13user_creature4CEgg6IsSaveEv
// DEMANGLED: user_creature::CEgg::IsSave(void)
int CEgg::IsSave() {
    return 0;
}

}  // namespace user_creature
