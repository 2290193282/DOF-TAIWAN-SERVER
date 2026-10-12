#ifndef OLDSRV_DB_DB_UPDATE_BOOSTER_GAGE_DATA_H_
#define OLDSRV_DB_DB_UPDATE_BOOSTER_GAGE_DATA_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_UpdateBoosterGageData.
// IDA_EA: dispatch @0x8446EEC; ctor/dtor
// @0x844FB76/@0x845B218/@0x845B248.
class DB_UpdateBoosterGageData : public DBDispatcher {
public:
    DB_UpdateBoosterGageData();
    virtual ~DB_UpdateBoosterGageData();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_UPDATE_BOOSTER_GAGE_DATA_H_
