#include "common/config.h"
#include "old_game/economy/inventory/inventory.h"

#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "infra/log/format/trace.h"

#include <stdint.h>


namespace {

bool IsFinishPointItem(const Inven_Item& item) {
    const uint32_t item_id = static_cast<uint32_t>(item.item_id_);
    // 0x850A0B5 first rejects zero, then performs unsigned >0x1963 and
    // <=0x1b57 checks on the physical +0x02 item id.
    return item_id != 0u && item_id > 0x1963u && item_id <= 0x1B57u;
}

void RemoveFinishPointItem(Inven_Item& item,
                           int index,
                           const char* missing_item_message,
                           int trace_line) {
    const int item_id = item.item_id_;
    CItem* const template_item = G_CDataManager()->find_item(item_id);
    if (template_item) {
        if (template_item->GetFinishPointPrice() != 0) {
            item.reset();
        }
        return;
    }
    LogManager::logFormat(1,
                          "inventory.cpp",
                          "static void CInventory::RemoveKCItem(Inven_Item*, Inven_Item*)",
                          trace_line,
                          missing_item_message,
                          item_id,
                          index);
}

}  // namespace

// IDA_EA: 0x850A096
// IDA_NAME: _ZN10CInventory12RemoveKCItemEP10Inven_ItemS1_
// DEMANGLED: CInventory::RemoveKCItem(Inven_Item *,Inven_Item *)
bool CInventory::RemoveKCItem(Inven_Item* equipment_items,
                              Inven_Item* stackable_items) {
    for (int index = 0;
         index < oldsrv::config::kEquipmentTypeCount;
         ++index) {
        Inven_Item& item = equipment_items[index];
        if (IsFinishPointItem(item)) {
            RemoveFinishPointItem(item,
                                  index,
                                  "CInventory::RemoveKCItem No EquipItem(%d) "
                                  "Found loopcount(%d)",
                                  0x23FE);
        }
    }

    bool result = false;
    for (int index = 3;; ++index) {
        result = index <= oldsrv::config::kMainInventoryLastSlot;
        if (index > oldsrv::config::kMainInventoryLastSlot) {
            break;
        }
        Inven_Item& item = stackable_items[index];
        if (IsFinishPointItem(item)) {
            RemoveFinishPointItem(item,
                                  index,
                                  "CInventory::RemoveKCItem No InvenItem(%d) "
                                  "Found loopcount(%d)",
                                  0x2416);
        }
    }
    return result;
}

// IDA_EA: 0x850A298
// IDA_NAME: _ZN10CInventory12RemoveKCItemEv
// DEMANGLED: CInventory::RemoveKCItem(void)
bool CInventory::RemoveKCItem() {
    // 0x850A298 passes the physical +0x1c equipment array and the +0x650
    // stackable table pointer directly to the two-table helper.
    return RemoveKCItem(equipment_items_, stackable_items_);
}
