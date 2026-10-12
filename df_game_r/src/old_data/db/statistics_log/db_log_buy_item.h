#ifndef OLDSRV_DB_DB_LOG_BUY_ITEM_H_
#define OLDSRV_DB_DB_LOG_BUY_ITEM_H_

#include "old_data/db/core/db_dispatcher.h"

enum ENUM_DBLOG_ITEM_TYPE : int;


// IDA unit: 13_database_dispatch/DB_LogBuyItem.
class DB_LogBuyItem : public DBDispatcher {
public:
    DB_LogBuyItem();
    virtual ~DB_LogBuyItem();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    static void makeRequest(unsigned int account_id,
                            unsigned int charac_no,
                            unsigned int item_id,
                            unsigned int count,
                            ENUM_DBLOG_ITEM_TYPE item_type);
};

#endif  // OLDSRV_DB_DB_LOG_BUY_ITEM_H_
