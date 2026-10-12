#ifndef OLDSRV_DB_DB_INSERT_CERA_ITEM_HISTORY_H_
#define OLDSRV_DB_DB_INSERT_CERA_ITEM_HISTORY_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x8445032/0x844511A/0x844FA42/0x845B572/0x845B5A2.
// IDA unit: 10_mail_trade_auction_cash/DB_InsertCeraItemHistory.
class DB_InsertCeraItemHistory : public DBDispatcher {
public:
    // IDA_EA: 0x844FA42.
    DB_InsertCeraItemHistory();
    // IDA_EA: 0x845B572 / 0x845B5A2.
    virtual ~DB_InsertCeraItemHistory();

    // IDA_EA: 0x8445032.
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    // IDA_EA: 0x844511A.
    static void makeRequest(int avatar_ui_id,
                            unsigned int old_charac_no,
                            unsigned int new_charac_no,
                            char history_kind);
};

#endif  // OLDSRV_DB_DB_INSERT_CERA_ITEM_HISTORY_H_
