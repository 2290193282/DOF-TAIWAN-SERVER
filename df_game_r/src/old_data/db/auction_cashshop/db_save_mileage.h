#ifndef OLDSRV_DB_DB_SAVE_MILEAGE_H_
#define OLDSRV_DB_DB_SAVE_MILEAGE_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x844F45A/0x845B7E2/0x845B812/0x8443C62.
// IDA unit: 13_database_dispatch/DB_SaveMileage.
class DB_SaveMileage : public DBDispatcher {
public:
    DB_SaveMileage();
    virtual ~DB_SaveMileage();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_SAVE_MILEAGE_H_
