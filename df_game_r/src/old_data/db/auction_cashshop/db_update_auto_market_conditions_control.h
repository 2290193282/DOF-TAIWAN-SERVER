#ifndef OLDSRV_DB_DB_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL_H_
#define OLDSRV_DB_DB_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>


// IDA unit: 10_mail_trade_auction_cash/DB_UpdateAutoMarketConditionsControl.
// IDA_EA: dispatch @0x8433BB8; makeRequest @0x8433D70; ctor/dtor
// @0x844EE72/@0x845D6F6/@0x845D726.
class DB_UpdateAutoMarketConditionsControl : public DBDispatcher {
public:
    DB_UpdateAutoMarketConditionsControl();
    virtual ~DB_UpdateAutoMarketConditionsControl();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(uint64_t total_gold,
                            uint64_t auction_gold,
                            uint64_t over_gold,
                            uint64_t optimum_gold_supply,
                            int gold_phase,
                            int item_phase,
                            int durability_phase);
};

#endif  // OLDSRV_DB_DB_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL_H_
