#ifndef OLDSRV_DB_DB_LOAD_MILEAGE_H_
#define OLDSRV_DB_DB_LOAD_MILEAGE_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x844F476/0x845B794/0x845B7C4/0x8443D2C.
// IDA unit: 13_database_dispatch/DB_LoadMileage.
class DB_LoadMileage : public DBDispatcher {
public:
    DB_LoadMileage();
    virtual ~DB_LoadMileage();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_LOAD_MILEAGE_H_
