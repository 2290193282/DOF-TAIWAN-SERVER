#include "common/manager/find_price_abi.h"

int itemPriceRates_[3];
float itemRepairCost_;

// IDA_EA: 0x898C86E
// IDA_NAME: _Z16getItemSellPriceiiib
// DEMANGLED: getItemSellPrice(int,int,int,bool)
int getItemSellPrice(int base_price,
                     int current_endurance,
                     int max_endurance,
                     bool is_special_price_type) {
    unsigned char rate_index = 0;
    if (is_special_price_type) {
        rate_index = 2;
    }
    if (current_endurance > max_endurance) {
        current_endurance = max_endurance;
    }
    if (max_endurance != 0) {
        return current_endurance *
               (base_price * itemPriceRates_[rate_index] / 1000) /
               max_endurance;
    }
    return base_price * itemPriceRates_[rate_index] / 1000;
}

// IDA_EA: 0x82F5F20
// IDA_NAME: _ZN9FindPriceC2Eh
// DEMANGLED: FindPrice::FindPrice(unsigned char)
FindPrice::FindPrice(unsigned char upgrade_value) : upgrade(upgrade_value) {}

// IDA_EA: 0x82F5F38
// IDA_NAME: _ZNK9FindPriceclERKSt4pairIhiE
// DEMANGLED: FindPrice::operator()(std::pair<unsigned char,int> const&)const
bool FindPrice::operator()(
    const std::pair<unsigned char, int>& price) const {
    return price.first == upgrade;
}
