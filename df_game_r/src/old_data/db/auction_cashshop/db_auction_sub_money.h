#ifndef OLDSRV_DB_DB_AUCTION_SUB_MONEY_H_
#define OLDSRV_DB_DB_AUCTION_SUB_MONEY_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 10_mail_trade_auction_cash/DB_AuctionSubMoney.
class DB_AuctionSubMoney : public DBDispatcher {
public:
    DB_AuctionSubMoney();
    virtual ~DB_AuctionSubMoney();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_AUCTION_SUB_MONEY_H_
