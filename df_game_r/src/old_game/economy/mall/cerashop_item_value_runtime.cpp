#include "old_game/economy/shop/npc_dialog_value_types.h"

#include <stdint.h>

#include "old_data/PVF/data/data_manager.h"

// All offsets in this unit are the old G++4.9/i386 ABI.  In particular the
// string is the four-byte COW libstdc++ representation and the enclosing
// record is padded to the vector's original 0x88-byte stride.
typedef char STCeraShopItem_Size_Must_Be_0x88[
    (sizeof(STCeraShopItem) == 0x88u) ? 1 : -1];

// IDA_EA: 0x88FD974
// IDA_NAME: _ZN14STCeraShopItemC2Ev
// DEMANGLED: STCeraShopItem::STCeraShopItem(void)
STCeraShopItem::STCeraShopItem()
    : raw_40() {
    clear();
    raw_84 = 0;
}

// IDA_EA: 0x85263C4
// IDA_NAME: _ZN14STCeraShopItemD2Ev
// DEMANGLED: STCeraShopItem::~STCeraShopItem()
STCeraShopItem::~STCeraShopItem() {}

// IDA_EA: 0x88FD9C4
// IDA_NAME: _ZN14STCeraShopItem5clearEv
// DEMANGLED: STCeraShopItem::clear(void)
STCeraShopItem* STCeraShopItem::clear() {
    item_type = 0xffu;
    raw_04 = 0;
    compare_key = -2;
    group_idx = -1;
    featured_idx = 0;
    item_idx = -1;
    raw_18 = 0xffu;
    stack_count = 1;
    sticker = 0;
    expiry_date = 0;
    raw_24 = 0;
    raw_28 = 0;
    gold_price = 0;
    win_point = 0;
    mileage_price = 0;
    cera_price = 0;
    raw_3c = 0xffu;
    raw_40 = "";
    bonus_weight = 0.0f;
    raw_48 = 0;
    raw_4c = 0;
    gift_disallowance = 0;
    cera_point_disallowance = 0;
    raw_52 = 0;
    stack_buyable = 1;
    limit_sale_marker = -2;
    raw_58 = -1;
    raw_5c = 0;
    raw_60 = 12;
    raw_62 = 0;
    raw_64 = 0;
    raw_68 = 0;
    raw_6c = 0;
    raw_70 = 0;
    raw_74 = 0;
    raw_78 = 0;
    raw_7c = -1;
    raw_80 = 0;
    raw_84 = 0xffu;
    return this;
}

// IDA_EA: 0x88FDB36
// IDA_NAME: _ZN14STCeraShopItem15isLimitSaleItemEv
// DEMANGLED: STCeraShopItem::isLimitSaleItem(void)
bool STCeraShopItem::isLimitSaleItem() {
    return limit_sale_marker != -2;
}

// IDA_EA: 0x88FDB52
// IDA_NAME: _ZN14STCeraShopItem12getCeraPriceEv
// DEMANGLED: STCeraShopItem::getCeraPrice(void)
int STCeraShopItem::getCeraPrice() {
    return cera_price;
}

// IDA_EA: 0x88FDB5E
// IDA_NAME: _ZN14STCeraShopItem12getGoldPriceEv
// DEMANGLED: STCeraShopItem::getGoldPrice(void)
int STCeraShopItem::getGoldPrice() {
    return gold_price;
}

// IDA_EA: 0x891A89E
// IDA_NAME: _ZN14STCeraShopItemeqERKS_
// DEMANGLED: STCeraShopItem::operator==(STCeraShopItem const&)
bool STCeraShopItem::operator==(const STCeraShopItem& rhs) {
    return compare_key == rhs.compare_key;
}

// IDA_EA: 0x8923E28
// IDA_NAME: _ZN14STCeraShopItemC2ERKS_
// DEMANGLED: STCeraShopItem::STCeraShopItem(STCeraShopItem const&)
STCeraShopItem::STCeraShopItem(const STCeraShopItem& rhs)
    : raw_40(rhs.raw_40) {
    item_type = rhs.item_type;
    raw_04 = rhs.raw_04;
    compare_key = rhs.compare_key;
    group_idx = rhs.group_idx;
    featured_idx = rhs.featured_idx;
    item_idx = rhs.item_idx;
    raw_18 = rhs.raw_18;
    stack_count = rhs.stack_count;
    sticker = rhs.sticker;
    expiry_date = rhs.expiry_date;
    raw_24 = rhs.raw_24;
    raw_28 = rhs.raw_28;
    gold_price = rhs.gold_price;
    win_point = rhs.win_point;
    mileage_price = rhs.mileage_price;
    cera_price = rhs.cera_price;
    raw_3c = rhs.raw_3c;
    bonus_weight = rhs.bonus_weight;
    raw_48 = rhs.raw_48;
    raw_4c = rhs.raw_4c;
    gift_disallowance = rhs.gift_disallowance;
    cera_point_disallowance = rhs.cera_point_disallowance;
    raw_52 = rhs.raw_52;
    stack_buyable = rhs.stack_buyable;
    limit_sale_marker = rhs.limit_sale_marker;
    raw_58 = rhs.raw_58;
    raw_5c = rhs.raw_5c;
    raw_60 = rhs.raw_60;
    raw_62 = rhs.raw_62;
    raw_64 = rhs.raw_64;
    raw_68 = rhs.raw_68;
    raw_6c = rhs.raw_6c;
    raw_70 = rhs.raw_70;
    raw_74 = rhs.raw_74;
    raw_78 = rhs.raw_78;
    raw_7c = rhs.raw_7c;
    raw_80 = rhs.raw_80;
    raw_84 = rhs.raw_84;
}

// IDA_EA: 0x892408A
// IDA_NAME: _ZN14STCeraShopItemaSERKS_
// DEMANGLED: STCeraShopItem::operator=(STCeraShopItem const&)
STCeraShopItem& STCeraShopItem::operator=(const STCeraShopItem& rhs) {
    item_type = rhs.item_type;
    raw_04 = rhs.raw_04;
    compare_key = rhs.compare_key;
    group_idx = rhs.group_idx;
    featured_idx = rhs.featured_idx;
    item_idx = rhs.item_idx;
    raw_18 = rhs.raw_18;
    stack_count = rhs.stack_count;
    sticker = rhs.sticker;
    expiry_date = rhs.expiry_date;
    raw_24 = rhs.raw_24;
    raw_28 = rhs.raw_28;
    gold_price = rhs.gold_price;
    win_point = rhs.win_point;
    mileage_price = rhs.mileage_price;
    cera_price = rhs.cera_price;
    raw_3c = rhs.raw_3c;
    raw_40 = rhs.raw_40;
    bonus_weight = rhs.bonus_weight;
    raw_48 = rhs.raw_48;
    raw_4c = rhs.raw_4c;
    gift_disallowance = rhs.gift_disallowance;
    cera_point_disallowance = rhs.cera_point_disallowance;
    raw_52 = rhs.raw_52;
    stack_buyable = rhs.stack_buyable;
    limit_sale_marker = rhs.limit_sale_marker;
    raw_58 = rhs.raw_58;
    raw_5c = rhs.raw_5c;
    raw_60 = rhs.raw_60;
    raw_62 = rhs.raw_62;
    raw_64 = rhs.raw_64;
    raw_68 = rhs.raw_68;
    raw_6c = rhs.raw_6c;
    raw_70 = rhs.raw_70;
    raw_74 = rhs.raw_74;
    raw_78 = rhs.raw_78;
    raw_7c = rhs.raw_7c;
    raw_80 = rhs.raw_80;
    raw_84 = rhs.raw_84;
    return *this;
}

// IDA_EA: 0x8510C1A
// IDA_NAME: _ZN14CCeraShopGoods7SetItemERK14STCeraShopItem
// DEMANGLED: CCeraShopGoods::SetItem(STCeraShopItem const&)
CCeraShopGoods* CCeraShopGoods::SetItem(const STCeraShopItem& item) {
    item_idx = item.item_idx;
    stack_count = item.stack_count;
    gold_price = item.gold_price;
    win_point = item.win_point;
    cera_price = item.cera_price;
    mileage_price = item.mileage_price;
    sub_group_index = static_cast<int>(static_cast<signed char>(item.item_type));
    group_idx = item.group_idx;
    featured_idx = item.featured_idx;
    sticker = static_cast<int>(static_cast<signed char>(item.sticker));
    expiry_date = static_cast<int>(item.expiry_date);
    bonus_weight = item.bonus_weight;
    raw_30 = item.raw_4c;
    gift_disallowance = item.gift_disallowance;
    cera_point_disallowance = item.cera_point_disallowance;
    stack_buyable = item.stack_buyable;
    related_item = static_cast<int>(reinterpret_cast<uintptr_t>(
        G_CDataManager()->find_item(item_idx)));
    return this;
}

// IDA_EA: 0x8514EA0
// IDA_NAME: _ZN14CCeraShopGoodsC2ERK14STCeraShopItem
// DEMANGLED: CCeraShopGoods::CCeraShopGoods(STCeraShopItem const&)
CCeraShopGoods::CCeraShopGoods(const STCeraShopItem& item) {
    SetItem(item);
}
