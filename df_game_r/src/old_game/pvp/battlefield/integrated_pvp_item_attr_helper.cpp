#include "common/manager/data_manager.h"
#include "common/config.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/integrated_pvp_item_attr.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/world/region/game_world.h"

// IDA_EA: 0x84FC651
// IDA_NAME: _Z24GetIntegratedPvPItemAttrRK10Inven_Item10INVEN_TYPEi
// DEMANGLED: GetIntegratedPvPItemAttr(Inven_Item const&,INVEN_TYPE,int)
int GetIntegratedPvPItemAttr(const Inven_Item& item,
                             INVEN_TYPE inventory_type,
                             int slot) {
    const bool use_integrated_attr =
        G_GameWorld()->IsIntegratedPvPBaseChannel() &&
        !G_CDataManager()->GetUpgradeRevisionPvPChannel();
    if (use_integrated_attr && inventory_type == static_cast<INVEN_TYPE>(1) &&
        slot >= oldsrv::config::kMainInventoryQuickSlotFirst &&
        slot <= oldsrv::config::kMainInventoryEquipmentSlotLast) {
        return _getIntegratedPvPItemAttr(item);
    }
    return item.GetItemAttr();
}
