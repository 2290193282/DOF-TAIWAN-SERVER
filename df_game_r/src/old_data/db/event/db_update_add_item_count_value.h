#ifndef OLDSRV_DB_DB_UPDATE_ADD_ITEM_COUNT_VALUE_H_
#define OLDSRV_DB_DB_UPDATE_ADD_ITEM_COUNT_VALUE_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_UpdateAddItemCountValue.
// DBTask raw slot 641 is signal 640 because DBProcess indexes dispatchers_
// directly by the first stream DWORD.
class DB_UpdateAddItemCountValue : public DBDispatcher {
public:
    DB_UpdateAddItemCountValue();
    virtual ~DB_UpdateAddItemCountValue();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id, unsigned int count);
};

#endif  // OLDSRV_DB_DB_UPDATE_ADD_ITEM_COUNT_VALUE_H_
