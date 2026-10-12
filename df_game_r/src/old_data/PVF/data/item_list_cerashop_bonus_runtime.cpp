#include "old_game/economy/inventory/item_list.h"

#include "common/manager/server_utility.h"

// IDA_EA: 0x8512930
// IDA_NAME: _ZNK9CItemList20getCeraShopBonusItemEiRmRj
// DEMANGLED: CItemList::getCeraShopBonusItem(
//     int,unsigned long &,unsigned int &)const
unsigned int* CItemList::getCeraShopBonusItem(
    int used_cera,
    unsigned long& item_id,
    unsigned int& count) const {
    item_id = static_cast<unsigned long>(-1);
    count = 0;

    for (std::vector<stCeraShopBonusInfo_t>::const_iterator info =
             cera_shop_bonus_items_.begin();
         info != cera_shop_bonus_items_.end();
         ++info) {
        if (info->raw_00 > used_cera || info->raw_04 < used_cera) {
            continue;
        }

        const int random_value = static_cast<int>(get_rand_int(1000));
        int accumulated_rate = 0;
        for (std::vector<stCeraShopBonusItemInfo_t>::const_iterator bonus =
                 info->bonus_items.begin();
             bonus != info->bonus_items.end();
             ++bonus) {
            accumulated_rate += bonus->rate;
            if (accumulated_rate > random_value) {
                item_id = static_cast<unsigned long>(bonus->item_index);
                count = bonus->count;
                return &count;
            }
        }
    }
    return &count;
}

// IDA_EA: 0x8512E06
// IDA_NAME: _ZN9CItemList18GetProperBonusItemEj
// DEMANGLED: CItemList::GetProperBonusItem(unsigned int)
BonusItem* CItemList::GetProperBonusItem(unsigned int value) {
    for (std::vector<BonusItem>::size_type index = 0;
         index < vending_bonus_items_.size(); ++index) {
        BonusItem& bonus = vending_bonus_items_[index];
        if (static_cast<unsigned int>(bonus.minimum_value) <= value &&
            static_cast<unsigned int>(bonus.maximum_value) > value) {
            return &bonus;
        }
    }
    return 0;
}

// IDA_EA: 0x854C596
// IDA_NAME: _ZNK9CItemList19GetVendingBonusRateEv
// DEMANGLED: CItemList::GetVendingBonusRate(void) const
int CItemList::GetVendingBonusRate() const {
    return vending_bonus_rate_;
}
