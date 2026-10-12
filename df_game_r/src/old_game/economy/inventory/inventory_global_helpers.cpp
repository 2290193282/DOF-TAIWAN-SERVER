#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/inventory.h"

// IDA_EA: 0x8501A79
// IDA_NAME: _Z19checkStackableLimitmi
// DEMANGLED: checkStackableLimit(unsigned long,int)
int checkStackableLimit(unsigned long item_id, int stackable_limit) {
    CItem* const item =
        G_CDataManager()->find_item(static_cast<int>(item_id));
    if (item && item->is_stackable() &&
        (static_cast<const CStackableItem*>(item)->getStackableLimit() <
             stackable_limit ||
         stackable_limit < 0)) {
        return 0;
    }
    return 1;
}

// IDA_EA: 0x850D32A
// IDA_NAME: _Z29isGainedGoldFromDungeonReason15eMoneyAddReason
// DEMANGLED: isGainedGoldFromDungeonReason(eMoneyAddReason)
int isGainedGoldFromDungeonReason(eMoneyAddReason reason) {
    const int reason_value = static_cast<int>(reason);
    return reason_value == 4 || reason_value == 15;
}
