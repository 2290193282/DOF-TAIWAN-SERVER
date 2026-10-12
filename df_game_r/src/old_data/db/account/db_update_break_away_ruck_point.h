#ifndef OLDSRV_DB_DB_UPDATE_BREAK_AWAY_RUCK_POINT_H_
#define OLDSRV_DB_DB_UPDATE_BREAK_AWAY_RUCK_POINT_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x8438300/0x844ED92/0x845CF58/0x845CF88.
// IDA unit: 13_database_dispatch/DB_UpdateBreakAwayRuckPoint.
class DB_UpdateBreakAwayRuckPoint : public DBDispatcher {
public:
    DB_UpdateBreakAwayRuckPoint();
    virtual ~DB_UpdateBreakAwayRuckPoint();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

// IDA_EA: 0x843856A/0x843865C/0x844ED5A/0x845CE6E/0x845CE9E.
// IDA unit: 10_mail_trade_auction_cash/DB_UpdateBreakAwayAccruedCera.
// This B-owned header is the sole public declaration for the class.
class DB_UpdateBreakAwayAccruedCera : public DBDispatcher {
public:
    DB_UpdateBreakAwayAccruedCera();
    virtual ~DB_UpdateBreakAwayAccruedCera();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id, unsigned int used_cera);
};

#endif  // OLDSRV_DB_DB_UPDATE_BREAK_AWAY_RUCK_POINT_H_
