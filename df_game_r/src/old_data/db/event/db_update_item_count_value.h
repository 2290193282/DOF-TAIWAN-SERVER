#ifndef OLDSRV_DB_DB_UPDATE_ITEM_COUNT_VALUE_H_
#define OLDSRV_DB_DB_UPDATE_ITEM_COUNT_VALUE_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_UpdateItemCountValue.
// IDA_EA: dispatch @0x843EB6E; makeRequest @0x843ECA4; ctor/dtor
// @0x844F636/@0x845BFCE/@0x845BFFE. makeRequest emits signal 0x240.
// DBTask raw restore stores the object at this+577; active DBProcess indexes
// by emitted signal, matching the existing restored source convention.
const char* get_item_count_table_name();

class DB_UpdateItemCountValue : public DBDispatcher {
public:
    DB_UpdateItemCountValue();
    virtual ~DB_UpdateItemCountValue();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id, unsigned int count);
};

#endif  // OLDSRV_DB_DB_UPDATE_ITEM_COUNT_VALUE_H_
