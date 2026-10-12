#include "old_game/economy/inventory/integrated_pvp_item_attr.h"

#include "common/config.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/world/region/game_world.h"

#include <cstring>
#include <stdint.h>

namespace {

typedef int (*ItemGetTypeFn)(const CItem* item);

struct IntegratedPvPItemVTable {
    uintptr_t destructor;
    uintptr_t deleting_destructor;
    uintptr_t make_item;
    ItemGetTypeFn get_item_type;
};

const IntegratedPvPItemVTable* GetItemVTable(const CItem* item) {
    const IntegratedPvPItemVTable* table = 0;
    std::memcpy(&table, item, sizeof(table));
    return table;
}

uint32_t ItemId(const Inven_Item& item) {
    return static_cast<uint32_t>(item.item_id_);
}

}  // namespace

// IDA_EA: 0x84FC541
// IDA_NAME: _Z25_getIntegratedPvPItemAttrRK10Inven_Item
// DEMANGLED: _getIntegratedPvPItemAttr(Inven_Item const&)
int _getIntegratedPvPItemAttr(const Inven_Item& item) {
    const CItem* const item_data =
        G_CDataManager()->find_item(static_cast<int>(ItemId(item)));
    if (!item_data) {
        return 0;
    }

    const int item_type = GetItemVTable(item_data)->get_item_type(item_data);
    if (item_type != oldsrv::config::kEquipmentTypeWeapon &&
        (item_type <= oldsrv::config::kEquipmentTypeTitle ||
         item_type > oldsrv::config::kOrdinaryEquipmentTypeLast)) {
        return item.GetItemAttr();
    }
    return 32 * static_cast<unsigned char>(item.GetReSealCount());
}

// IDA_EA: 0x84FC5FF
// IDA_NAME: _Z24GetIntegratedPvPItemAttrRK10Inven_Item
// DEMANGLED: GetIntegratedPvPItemAttr(Inven_Item const&)
int GetIntegratedPvPItemAttr(const Inven_Item& item) {
    if (G_GameWorld()->IsIntegratedPvPBaseChannel() &&
        !G_CDataManager()->GetUpgradeRevisionPvPChannel()) {
        return _getIntegratedPvPItemAttr(item);
    }
    return item.GetItemAttr();
}
