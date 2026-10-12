#ifndef OLDSRV_GLOBAL_FIND_PRICE_ABI_H_
#define OLDSRV_GLOBAL_FIND_PRICE_ABI_H_

#include <utility>

// IDA unit: 99_misc_global/Global equipment-price helpers.
extern int itemPriceRates_[3];
extern float itemRepairCost_;

// IDA_EA: 0x898C86E
// IDA_NAME: _Z16getItemSellPriceiiib
// DEMANGLED: getItemSellPrice(int,int,int,bool)
int getItemSellPrice(int base_price,
                     int current_endurance,
                     int max_endurance,
                     bool is_special_price_type);

// IDA unit: 99_misc_global/FindPrice.
// IDA_EA: ctor @0x82F5F20, predicate @0x82F5F38.
// Predicate used by the old auction average-price lookup path.
struct FindPrice {
    explicit FindPrice(unsigned char upgrade);
    bool operator()(const std::pair<unsigned char, int>& price) const;

    unsigned char upgrade;
};


using ::FindPrice;
using ::getItemSellPrice;
using ::itemPriceRates_;
using ::itemRepairCost_;


static_assert(sizeof(FindPrice) == 0x01u,
              "FindPrice IDA ABI size must stay 0x01");

#endif  // OLDSRV_GLOBAL_FIND_PRICE_ABI_H_
