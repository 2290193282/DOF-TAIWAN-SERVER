#ifndef OLDSRV_DB_DB_UPDATE_GM_DATA_H_
#define OLDSRV_DB_DB_UPDATE_GM_DATA_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_UpdateGMData.
// IDA_EA: makeRequest @0x843982C; dispatch @0x843990A; ctor/dtor
// @0x844EDE6/@0x845CD36/@0x845CD66.
class DB_UpdateGMData : public DBDispatcher {
public:
    DB_UpdateGMData();
    virtual ~DB_UpdateGMData();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(bool enabled);
};

#endif  // OLDSRV_DB_DB_UPDATE_GM_DATA_H_
