#ifndef OLDSRV_DB_DB_UPDATE_LIMIT_ITEM_USAGE_H_
#define OLDSRV_DB_DB_UPDATE_LIMIT_ITEM_USAGE_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_UpdateLimitItemUsage.
// IDA_EA: dispatch @0x843F022; makeRequest @0x843F030; ctor/dtor
// @0x844F652/@0x845BF32/@0x845BF62. DBTask slot 577.
class DB_UpdateLimitItemUsage : public DBDispatcher {
public:
    DB_UpdateLimitItemUsage();
    virtual ~DB_UpdateLimitItemUsage();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int a1, int a2, int a3, int a4, long a5);
};

#endif  // OLDSRV_DB_DB_UPDATE_LIMIT_ITEM_USAGE_H_
