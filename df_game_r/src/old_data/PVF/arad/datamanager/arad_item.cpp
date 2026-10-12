#include "old_data/PVF/arad/datamanager/arad_item.h"

#include "old_data/PVF/data/datamanager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/inventory.h"

#include <stdint.h>

namespace ARAD {

namespace {

typedef void (*CItemMakeInvenItemFn)(CItem*, Inven_Item*);

}  // namespace

// IDA_EA: 0x8197375
// IDA_NAME: _ZN4ARAD9ARAD_ITEM14arad_make_itemEmiR10Inven_Item
// DEMANGLED: ARAD::ARAD_ITEM::arad_make_item(unsigned long,int,Inven_Item &)
int ARAD_ITEM::arad_make_item(unsigned long item_index,
                               int item_count,
                               Inven_Item& item) {
    if (item_index == 0 || item_index == static_cast<unsigned long>(-1) ||
        item_count == 0) {
        return 0;
    }

    CItem* item_prototype = G_CDataManager()->find_item(
        static_cast<int>(item_index));
    if (!item_prototype) {
        return 0;
    }

    item.reset();
    void** vtable = *reinterpret_cast<void***>(item_prototype);
    reinterpret_cast<CItemMakeInvenItemFn>(vtable[2])(item_prototype, &item);
    item.item_id_ = static_cast<int32_t>(item_index);
    item.set_add_info(item_count);
    return 1;
}

}  // namespace ARAD
