#ifndef OLDSRV_DB_DB_UPDATE_GIFT_COUNT_H_
#define OLDSRV_DB_DB_UPDATE_GIFT_COUNT_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x8427112/0x84271EC/0x844E756/0x845EAC4/0x845EAF4.
// IDA unit: 13_database_dispatch/DB_UpdateGiftCount.
class DB_UpdateGiftCount : public DBDispatcher {
public:
    DB_UpdateGiftCount();
    virtual ~DB_UpdateGiftCount();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id, int gift_count);
};

#endif  // OLDSRV_DB_DB_UPDATE_GIFT_COUNT_H_
