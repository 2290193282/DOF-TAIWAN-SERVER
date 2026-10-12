// Canonical C-owned secret-shop value runtime.
#include "old_game/economy/mall/secret_shop_runtime.h"

#include <algorithm>

namespace secretshop {

// IDA_EA: 0x8AB1606
// IDA_NAME: _ZN10secretshop8BUY_INFOC2Ev
// DEMANGLED: secretshop::BUY_INFO::BUY_INFO(void)
BUY_INFO::BUY_INFO() : item_id(0), count(0) {}

// IDA_EA: 0x8AB161E
// IDA_NAME: _ZN10secretshop8BUY_INFOeqEi
// DEMANGLED: secretshop::BUY_INFO::operator==(int)
bool BUY_INFO::operator==(int compare_item_id) {
    return item_id == compare_item_id;
}

// IDA_EA: 0x8AB1EFC
// IDA_NAME: _ZN10secretshop8BUY_INFOD2Ev
// DEMANGLED: secretshop::BUY_INFO::~BUY_INFO()
BUY_INFO::~BUY_INFO() {}

// IDA_EA: 0x82A5346
// IDA_NAME: _ZN10secretshop8RETAILERC2Ev
// DEMANGLED: secretshop::RETAILER::RETAILER(void)
RETAILER::RETAILER() : std::vector<SALE_INFO>() {}

// IDA_EA: 0x82A535A
// IDA_NAME: _ZN10secretshop8RETAILERD2Ev
// DEMANGLED: secretshop::RETAILER::~RETAILER()
RETAILER::~RETAILER() {}

// IDA_EA: 0x8AB12A0
// IDA_NAME: _ZN10secretshop8RETAILER11GetSaleInfoEi
// DEMANGLED: secretshop::RETAILER::GetSaleInfo(int)
SALE_INFO* RETAILER::GetSaleInfo(int requested_item_id) {
    std::vector<SALE_INFO>::iterator it =
        std::find(begin(), end(), requested_item_id);
    if (it == end()) {
        return 0;
    }
    return &*it;
}

// IDA_EA: 0x8AB15F2
// IDA_NAME: _ZN10secretshop8RETAILER5clearEv
// DEMANGLED: secretshop::RETAILER::clear(void)
void RETAILER::clear() {
    std::vector<SALE_INFO>::clear();
}

// IDA_EA: 0x82A536E
// IDA_NAME: _ZN10secretshop7SHOPPERC2Ev
// DEMANGLED: secretshop::SHOPPER::SHOPPER(void)
SHOPPER::SHOPPER() : std::vector<BUY_INFO>() {}

// IDA_EA: 0x82A5382
// IDA_NAME: _ZN10secretshop7SHOPPERD2Ev
// DEMANGLED: secretshop::SHOPPER::~SHOPPER()
SHOPPER::~SHOPPER() {}

// IDA_EA: 0x8AB1344
// IDA_NAME: _ZN10secretshop7SHOPPER10GetBuyInfoEi
// DEMANGLED: secretshop::SHOPPER::GetBuyInfo(int)
BUY_INFO* SHOPPER::GetBuyInfo(int requested_item_id) {
    std::vector<BUY_INFO>::iterator it =
        std::find(begin(), end(), requested_item_id);
    if (it == end()) {
        return 0;
    }
    return &*it;
}

// IDA_EA: 0x8AB13E8
// IDA_NAME: _ZN10secretshop7SHOPPER7BuyItemEii
// DEMANGLED: secretshop::SHOPPER::BuyItem(int,int)
void SHOPPER::BuyItem(int purchased_item_id, int purchased_count) {
    BUY_INFO* info = GetBuyInfo(purchased_item_id);
    if (info) {
        info->count += purchased_count;
        return;
    }

    BUY_INFO new_info;
    new_info.item_id = purchased_item_id;
    new_info.count = purchased_count;
    push_back(new_info);
}

// IDA_EA: 0x8AB162E
// IDA_NAME: _ZN10secretshop7SHOPPER5clearEv
// DEMANGLED: secretshop::SHOPPER::clear(void)
void SHOPPER::clear() {
    std::vector<BUY_INFO>::clear();
}

// IDA_EA: 0x82A5396
// IDA_NAME: _ZN10secretshop16SECRET_SHOP_INFOC2Ev
// DEMANGLED: secretshop::SECRET_SHOP_INFO::SECRET_SHOP_INFO(void)
SECRET_SHOP_INFO::SECRET_SHOP_INFO() : retailer(), shopper() {}

// IDA_EA: 0x82A53DC
// IDA_NAME: _ZN10secretshop16SECRET_SHOP_INFOD2Ev
// DEMANGLED: secretshop::SECRET_SHOP_INFO::~SECRET_SHOP_INFO()
SECRET_SHOP_INFO::~SECRET_SHOP_INFO() {}

// IDA_EA: 0x85FBE8A
// IDA_NAME: _ZN10secretshop16SECRET_SHOP_INFO11GetSaleInfoEi
// DEMANGLED: secretshop::SECRET_SHOP_INFO::GetSaleInfo(int)
SALE_INFO* SECRET_SHOP_INFO::GetSaleInfo(int requested_item_id) {
    return retailer.GetSaleInfo(requested_item_id);
}

// IDA_EA: 0x8AB1642
// IDA_NAME: _ZN10secretshop16SECRET_SHOP_INFO5clearEv
// DEMANGLED: secretshop::SECRET_SHOP_INFO::clear(void)
void SECRET_SHOP_INFO::clear() {
    purchase_recorded = 0;
    retailer.clear();
    shopper.clear();
}

}  // namespace secretshop
