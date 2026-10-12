#include "common/config.h"
#include "old_game/economy/inventory/inventory.h"

#include "old_game/economy/equipment/expand_equipslot_runtime.h"
#include "old_game/player/core/cuser.h"

#include <stdint.h>


// IDA_EA: 0x84FB0A0
// IDA_NAME: _ZNK10CInventory13GetInvenStartEi
// DEMANGLED: CInventory::GetInvenStart(int)const
Inven_Item* CInventory::GetInvenStart(int inven_type) const {
    switch (inven_type) {
    case 0:
        return const_cast<Inven_Item*>(&equipment_items_[
            oldsrv::config::kOrdinaryEquipmentTypeFirst]);
    case 1:
        return stackable_items_;
    case 2:
        return avatar_items_;
    case 3:
        return creature_items_;
    case 5: {
        CExpandEquipslot* const expand = reinterpret_cast<CExpandEquipslot*>(
            parent_->GetCharacExpandData(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(9)));
        return expand->getExpandEquipslotW(
            static_cast<ENUM_EQUIPSLOT>(1));
    }
    default:
        return 0;
    }
}

// IDA_EA: 0x84B4262
// IDA_NAME: _ZNK6CCargo15get_cargo_startEv
// DEMANGLED: CCargo::get_cargo_start(void)const
Inven_Item* CCargo::get_cargo_start() const {
    return cargo_;
}
#include "common/config.h"
