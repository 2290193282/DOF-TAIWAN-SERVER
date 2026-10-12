#ifndef OLDSRV_DB_DB_INSERT_RANDOM_OPTION_ITEM_INFORM_H_
#define OLDSRV_DB_DB_INSERT_RANDOM_OPTION_ITEM_INFORM_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_InsertRandomOptionItemInform.
// IDA_EA: dispatch @0x8442C6A; makeRequest @0x8442D70; ctor/dtor
// @0x844F8F2/@0x845B91A/@0x845B94A. DBTask/makeRequest signal 665.
class DB_InsertRandomOptionItemInform : public DBDispatcher {
public:
    DB_InsertRandomOptionItemInform();
    virtual ~DB_InsertRandomOptionItemInform();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx,
                            unsigned int account_id,
                            unsigned char check_count);
};

#endif  // OLDSRV_DB_DB_INSERT_RANDOM_OPTION_ITEM_INFORM_H_
