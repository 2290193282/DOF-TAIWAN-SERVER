#ifndef OLDSRV_DB_DB_SAVE_GOLD_ZERO_TRADE_LOG_H_
#define OLDSRV_DB_DB_SAVE_GOLD_ZERO_TRADE_LOG_H_

#include "old_data/db/core/db_dispatcher.h"
#include "old_data/db/auction_cashshop/zero_trade_payload.h"


// IDA unit: 10_mail_trade_auction_cash/DB_SaveGoldZeroTradeLog.
// DBTask::RegistDispatcher @0x83FBFEE stores this at old object offset +162,
// which is active dispatcher signal 161.
class DB_SaveGoldZeroTradeLog : public DBDispatcher {
public:
    // IDA_EA: 0x844E49A.
    DB_SaveGoldZeroTradeLog();
    // IDA_EA: 0x845F040 / 0x845F070.
    virtual ~DB_SaveGoldZeroTradeLog();

    // IDA_EA: 0x84227F6.
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_SAVE_GOLD_ZERO_TRADE_LOG_H_
