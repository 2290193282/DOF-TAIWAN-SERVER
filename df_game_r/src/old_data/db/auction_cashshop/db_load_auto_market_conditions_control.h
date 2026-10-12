#ifndef OLDSRV_DB_DB_LOAD_AUTO_MARKET_CONDITIONS_CONTROL_H_
#define OLDSRV_DB_DB_LOAD_AUTO_MARKET_CONDITIONS_CONTROL_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 10_mail_trade_auction_cash/DB_LoadAutoMarketConditionsControl.
// IDA_EA: dispatch @0x84335DC; makeRequest @0x8433ADA; ctor/dtor
// @0x844EE56/@0x845D744/@0x845D774.
class DB_LoadAutoMarketConditionsControl : public DBDispatcher {
public:
    DB_LoadAutoMarketConditionsControl();
    virtual ~DB_LoadAutoMarketConditionsControl();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(bool load_from_event);
};

#endif  // OLDSRV_DB_DB_LOAD_AUTO_MARKET_CONDITIONS_CONTROL_H_
