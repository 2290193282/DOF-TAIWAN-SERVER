#include "old_game/economy/inventory/item_list.h"

#include "old_game/economy/inventory/inventory.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/equipment/item_amplifier_abi.h"
#include "old_game/economy/equipment/item_option.h"
#include "infra/log/format/trace.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern "C++" unsigned int get_rand_int_global(int limit)
    __asm__("_Z12get_rand_inti");

namespace {

typedef void (*MakeItemFn)(CItem*, Inven_Item*);

uint16_t InvenItemEndurance(const Inven_Item& item) {
    return item.avatar_select_;
}

void SetInvenItemEndurance(Inven_Item& item, uint16_t endurance) {
    item.avatar_select_ = endurance;
}

const char kCreateItemPrettyFunction[] =
    "void CItemList::create_item(ENUM_ITEM_CREATE_TYPE, Inven_Item&, int)";

}  // namespace

// IDA_EA: 0x817A10C
// IDA_NAME: _ZN9CItemList15GetRestrictCodeEi
// DEMANGLED: CItemList::GetRestrictCode(int)
int CItemList::GetRestrictCode(int item_id) {
    CItemRestrictCodeList::iterator it =
        restrict_codes_by_item_.find(static_cast<unsigned int>(item_id));
    return it == restrict_codes_by_item_.end() ? 0 : it->second;
}

// IDA_EA: 0x8511F1E
// IDA_NAME: _ZN9CItemList9find_itemEPKc
// DEMANGLED: CItemList::find_item(char const*)
CItem* CItemList::find_item(const char* item_name) {
    for (CItemIndexList::iterator it = items_by_index_.begin();
         it != items_by_index_.end(); ++it) {
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
    return it == goods_by_index_.end() ? 0 : it->second;
}

// IDA_EA: 0x8511FB8
// IDA_NAME: _ZN9CItemList11create_itemE21ENUM_ITEM_CREATE_TYPER10Inven_Itemi
// DEMANGLED: CItemList::create_item(ENUM_ITEM_CREATE_TYPE, Inven_Item &,int)
void CItemList::create_item(ENUM_ITEM_CREATE_TYPE create_type,
                            Inven_Item& item,
                            int add_info) {
    item.set_add_info(add_info);
    CItem* proto = find_item(item.item_id_);
    if (!proto) {
        LogManager::logFormat(1, "item.cpp", kCreateItemPrettyFunction, 0x98F,
                               "find_item(%d)", item.item_id_);
        item.item_id_ = -1;
        item.set_add_info(0);
        SetInvenItemEndurance(item, 0);
        item.item_category_ = 0;
        item.ResetItemAttr();
        return;
    }
    void** vtable = *reinterpret_cast<void***>(proto);
    reinterpret_cast<MakeItemFn>(vtable[2])(proto, &item);
    if (proto->is_stackable()) {
        if (item.get_add_info() < 0) item.set_add_info(0);
        if (item.get_add_info() > 999) {
            cMyTrace trace(kCreateItemPrettyFunction, 2465, 5);
            trace("CItemList::create_item, add_info >= STACK_LIMIT_ERROR , %d ITEM #%d",
                  item.get_add_info(), item.item_id_);
        }
    }
    item.amplify_option_.reset();
    if (create_type == static_cast<ENUM_ITEM_CREATE_TYPE>(1)) {
        if (::GlobalData::s_itemAmplifier_->checkInvestableItem(proto, item, 0)) {
            ::GlobalData::s_itemAmplifier_->generateAmplifyItem(proto, item);
        }
        if (item.item_category_ == 1) {
            const int max_endurance =
                reinterpret_cast<const CEquipItem*>(proto)->get_endurance();
            const int random_value = get_rand_int_global(100);
            const int repair_rate =
                G_CDataManager()->get_server_parameter_script()
                    ->get_damaged_equipment_ratio();
            const int endurance_range = static_cast<int>(
                static_cast<long double>(max_endurance) *
                (static_cast<long double>(100 - repair_rate) / 100.0L));
            int new_endurance = static_cast<int>(
                static_cast<long double>(max_endurance) +
                static_cast<long double>(endurance_range) *
                    (static_cast<long double>(random_value) / -100.0L));
            if (new_endurance < 0) new_endurance = 1;
            else if (new_endurance > max_endurance) new_endurance = max_endurance;
            SetInvenItemEndurance(item,
                                  static_cast<uint16_t>(new_endurance));
        }
    }
}

// IDA_EA: 0x8512230
// IDA_NAME: _ZN9CItemList14get_lotto_itemEiPc
// DEMANGLED: CItemList::get_lotto_item(int,char *)
Inven_Item* CItemList::get_lotto_item(int item_id, char* output) {
    Inven_Item* const result = reinterpret_cast<Inven_Item*>(output);
    CItem* const item = find_item(item_id);
    if (!item) {
        result->item_id_ = 0;
        result->set_add_info(0);
        result->avatar_select_ = 0;
        result->item_category_ = 0;
        return result->ResetItemAttr();
    }

    if (item->is_stackable()) {
        CStackableItem* const stackable = static_cast<CStackableItem*>(item);
        // 0627 CItemList::get_lotto_item reads CStackableItem +0x2b0, the
        // prototype item type.  [upgradable legacy] jars are type 13 while
        // their independent [sub type] field at +0x2bc is normally zero.
        const int item_type = stackable->GetItemType();
        if (item_type == 6 || item_type == 7 || item_type == 13) {
            return stackable->get_lotto_item(output);
        }

        result->item_id_ = 0;
        result->set_add_info(0);
        result->avatar_select_ = 0;
        result->item_category_ = 2;
        return result->ResetItemAttr();
    }

    result->item_id_ = 0;
    result->set_add_info(0);
    result->avatar_select_ = 0;
    result->item_category_ = 1;
    return result->ResetItemAttr();
}
