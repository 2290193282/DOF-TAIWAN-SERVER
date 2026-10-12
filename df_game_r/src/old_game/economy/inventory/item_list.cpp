#include "old_game/economy/inventory/item_list.h"

#include "old_game/economy/inventory/inventory.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/instance_rental_system_abi.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/equipment/item_amplifier_abi.h"
#include "old_game/economy/equipment/item_option.h"
#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"

#include <ext/hash_map>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


// IDA_EA: 0x817A10C
// IDA_NAME: _ZN9CItemList15GetRestrictCodeEi
// DEMANGLED: CItemList::GetRestrictCode(int)
int CItemList::GetRestrictCode(int item_id) {
    unsigned int key = static_cast<unsigned int>(item_id);
    CItemRestrictCodeList::iterator it = restrict_codes_by_item_.find(key);
    if (it == restrict_codes_by_item_.end()) {
        return 0;
    }
    return it->second;
}

// IDA_EA: 0x8511F1E
// IDA_NAME: _ZN9CItemList9find_itemEPKc
// DEMANGLED: CItemList::find_item(char const*)
CItem* CItemList::find_item(const char* item_name) {
    for (CItemIndexList::iterator it = items_by_index_.begin();
         it != items_by_index_.end();
         ++it) {
        CItem* item = it->second;
        if (::strcmp(item->GetItemName(), item_name) == 0) {
            return item;
        }
    }
    return 0;
}

// IDA_EA: 0x8512358
// IDA_NAME: _ZN9CItemList9FindGoodsEi
// DEMANGLED: CItemList::FindGoods(int)
CCeraShopGoods* CItemList::FindGoods(int goods_no) {
    CItemGoodsList::iterator it = goods_by_index_.find(goods_no);
    if (it == goods_by_index_.end()) {
        return 0;
    }
    return it->second;
}
