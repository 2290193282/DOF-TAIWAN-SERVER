#include "old_game/economy/inventory/item_timeout_runtime.h"

#include "common/config.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/inventory.h"
#include "common/manager/system_time.h"

#include <cstddef>
#include <cstring>
#include <stdint.h>

namespace {

typedef int (*ItemGetTypeFn)(const CItem* item);
typedef unsigned char (*ItemTimeoutExemptFn)(const CItem* item);
typedef int (*ItemRevisionExemptFn)(const CItem* item);

struct ItemTimeoutVTable {
    uintptr_t destructor;                  // +0x00
    uintptr_t deleting_destructor;         // +0x04
    uintptr_t make_item;                   // +0x08
    ItemGetTypeFn get_item_type;           // +0x0c
    ItemTimeoutExemptFn timeout_exempt;    // +0x10
    uintptr_t slots_05_through_17[13];     // +0x14..+0x47
    ItemRevisionExemptFn revision_exempt;  // +0x48
};

#if defined(__i386__)
static_assert(offsetof(ItemTimeoutVTable, get_item_type) == 0x0cu,
              "CItem get-item-type vtable offset must stay 0x0c");
static_assert(offsetof(ItemTimeoutVTable, timeout_exempt) == 0x10u,
              "CItem timeout-exempt vtable offset must stay 0x10");
static_assert(offsetof(ItemTimeoutVTable, revision_exempt) == 0x48u,
              "CItem revision-exempt vtable offset must stay 0x48");
#endif

const ItemTimeoutVTable* GetItemVTable(const CItem* item) {
    const ItemTimeoutVTable* table = 0;
    std::memcpy(&table, item, sizeof(table));
    return table;
}

uint32_t ItemId(const Inven_Item& item) {
    return static_cast<uint32_t>(item.item_id_);
}

uint16_t ItemPeriodDate(const Inven_Item& item) {
    return item.avatar_select_;
}

}  // namespace

// IDA_EA: 0x867CF8D
// IDA_NAME: _Z17_checkTimeoutItemRK10Inven_Item
// DEMANGLED: _checkTimeoutItem(Inven_Item const&)
int _checkTimeoutItem(const Inven_Item& item) {
    const CItem* const item_data =
        G_CDataManager()->find_item(static_cast<int>(ItemId(item)));
    if (!item_data ||
        (!item_data->getUsablePeriod() && !item_data->getExpirationDate())) {
        return 0;
    }

    const ItemTimeoutVTable* const table = GetItemVTable(item_data);
    if (table->timeout_exempt(item_data)) {
        return 0;
    }
    if (table->revision_exempt(item_data) && item.seal_or_state_) {
        return 0;
    }

    const int now = GlobalData::system_time()->getCurSec();
    if (table->get_item_type(item_data) ==
        oldsrv::config::kEquipmentTypeWeapon) {
        return item_data->getExpirationDate() <= now ? 1 : 0;
    }
    return 86400 * static_cast<int>(ItemPeriodDate(item)) + 1151683200 <=
                   now
               ? 1
               : 0;
}
