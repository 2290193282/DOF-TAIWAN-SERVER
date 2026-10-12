#include "old_game/economy/inventory/item_list.h"

#include "old_game/economy/inventory/inventory.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/equipment/item_amplifier_abi.h"
#include "old_game/economy/equipment/item_option.h"
#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"

#include <stdint.h>

extern "C++" unsigned int get_rand_int_global(int limit)
    __asm__("_Z12get_rand_inti");

namespace {

const char kCreateItemPrettyFunction[] =
    "void CItemList::create_item(ENUM_ITEM_CREATE_TYPE, Inven_Item&, int)";

typedef void (*MakeItemFn)(CItem*, Inven_Item*);

uint16_t InvenItemEndurance(const Inven_Item& item) {
    return item.avatar_select_;
}

void SetInvenItemEndurance(Inven_Item& item, uint16_t endurance) {
    item.avatar_select_ = endurance;
}

}  // namespace

// IDA_EA: 0x8511FB8
// IDA_NAME: _ZN9CItemList11create_itemE21ENUM_ITEM_CREATE_TYPER10Inven_Itemi
// DEMANGLED: CItemList::create_item(ENUM_ITEM_CREATE_TYPE, Inven_Item &,int)
void CItemList::create_item(ENUM_ITEM_CREATE_TYPE create_type,
                            Inven_Item& item,
                            int add_info) {
    item.set_add_info(add_info);
    CItem* proto = find_item(item.item_id_);
    if (!proto) {
        LogManager::logFormat(
            1,
            "item.cpp",
            kCreateItemPrettyFunction,
            0x98F,
            "find_item(%d)",
            item.item_id_);
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
        if (item.get_add_info() < 0) {
            item.set_add_info(0);
        }
        if (item.get_add_info() > 999) {
            cMyTrace trace(kCreateItemPrettyFunction, 2465, 5);
            trace(
                "CItemList::create_item, add_info >= STACK_LIMIT_ERROR , %d ITEM #%d",
                item.get_add_info(),
                item.item_id_);
        }
    }

    item.amplify_option_.reset();
    if (create_type == static_cast<ENUM_ITEM_CREATE_TYPE>(1)) {
        if (::GlobalData::s_itemAmplifier_->checkInvestableItem(
                proto,
                item,
                0)) {
            ::GlobalData::s_itemAmplifier_->generateAmplifyItem(proto, item);
        }
        if (item.item_category_ == 1) {
            // CEquipItem::get_endurance is C-owned runtime behavior.  Keep
            // this PVF factory on the original member symbol instead of the
            // removed global ABI implementation.
            const int max_endurance =
                reinterpret_cast<const CEquipItem*>(proto)->get_endurance();
            const int random_value =
                get_rand_int_global(100);
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
            if (new_endurance < 0) {
                new_endurance = 1;
            } else if (new_endurance > max_endurance) {
                new_endurance = max_endurance;
            }
            SetInvenItemEndurance(item,
                                  static_cast<uint16_t>(new_endurance));
        }
    }
}
