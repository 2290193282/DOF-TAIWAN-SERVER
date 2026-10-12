#include "old_game/economy/mall/auction_runtime.h"

#include <algorithm>
#include <utility>

#include "common/manager/find_price_abi.h"
#include "common/manager/global_data.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"

// IDA_EA: 0x8A8CC42
// IDA_NAME: _ZN25stAuctionLimitItemParam_tC2Ev
// DEMANGLED: stAuctionLimitItemParam_t::stAuctionLimitItemParam_t(void)
stAuctionLimitItemParam_t::stAuctionLimitItemParam_t()
    : item_id(0),
      begin_time(0),
      end_time(0) {}

// IDA_EA: 0x8A934EE
// IDA_NAME: _ZN25stAuctionLimitItemParam_tD2Ev
// DEMANGLED: stAuctionLimitItemParam_t::~stAuctionLimitItemParam_t()
stAuctionLimitItemParam_t::~stAuctionLimitItemParam_t() {}

// IDA_EA: 0x82F5A94
// IDA_NAME: _ZN20CAuctionAveragePriceC2Ev
// DEMANGLED: CAuctionAveragePrice::CAuctionAveragePrice(void)
CAuctionAveragePrice::CAuctionAveragePrice()
    : average_prices_() {}

// IDA_EA: 0x82AA812
// IDA_NAME: _ZN20CAuctionAveragePriceD2Ev
// DEMANGLED: CAuctionAveragePrice::~CAuctionAveragePrice()
CAuctionAveragePrice::~CAuctionAveragePrice() {}

// IDA_EA: 0x82F5AA8
// IDA_NAME: _ZN20CAuctionAveragePrice16LoadAveragePriceEv
// DEMANGLED: CAuctionAveragePrice::LoadAveragePrice(void)
int CAuctionAveragePrice::LoadAveragePrice() {
    MySQL* const database = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(12),
        static_cast<ENUM_SERVER_GROUP>(0));
    database->set_query(
        "seLect item_id,upgrade,average_price from auction_average_price");
    if (!database->exec(true)) {
        return 0;
    }
    if (!database->get_n_rows()) {
        return 1;
    }

    int item_id = 0;
    int average_price = 0;
    unsigned char upgrade = 0;
    for (unsigned int row_index = 0;; ++row_index) {
        const unsigned int row_count =
            static_cast<unsigned int>(database->get_n_rows());
        if (row_count <= row_index) {
            break;
        }
        if (!database->fetch() || !database->get_int(0, item_id) ||
            !database->get_ubyte(1, upgrade) ||
            !database->get_int(2, average_price)) {
            return 0;
        }
        SetAveragePrice(item_id, upgrade, average_price);
    }
    return 1;
}

// IDA_EA: 0x82F5C24
// IDA_NAME: _ZN20CAuctionAveragePrice15SetAveragePriceEihi
// DEMANGLED: CAuctionAveragePrice::SetAveragePrice(int,unsigned char,int)
void CAuctionAveragePrice::SetAveragePrice(int item_id,
                                           unsigned char upgrade,
                                           int average_price) {
    std::map<int, std::vector<std::pair<unsigned char, int> > >::iterator it =
        average_prices_.find(item_id);
    if (it != average_prices_.end()) {
        it->second.push_back(std::make_pair(upgrade, average_price));
        return;
    }

    std::vector<std::pair<unsigned char, int> > prices;
    prices.push_back(std::make_pair(upgrade, average_price));
    average_prices_.insert(std::make_pair(item_id, prices));
}

// IDA_EA: 0x82F5DA8
// IDA_NAME: _ZNK20CAuctionAveragePrice15GetAveragePriceEih
// DEMANGLED: CAuctionAveragePrice::GetAveragePrice(int,unsigned char)const
int CAuctionAveragePrice::GetAveragePrice(int item_id,
                                          unsigned char upgrade) const {
    std::map<int, std::vector<std::pair<unsigned char, int> > >::const_iterator
        found = average_prices_.find(item_id);
    if (found == average_prices_.end()) {
        return 0;
    }

    const std::vector<std::pair<unsigned char, int> >& prices = found->second;
    std::vector<std::pair<unsigned char, int> >::const_iterator price =
        std::find_if(prices.begin(), prices.end(), FindPrice(upgrade));
    if (price != prices.end()) {
        return price->second;
    }
    return 0;
}
