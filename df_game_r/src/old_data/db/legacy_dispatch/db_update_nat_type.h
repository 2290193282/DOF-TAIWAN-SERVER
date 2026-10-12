#ifndef OLDSRV_DB_DB_UPDATE_NAT_TYPE_H_
#define OLDSRV_DB_DB_UPDATE_NAT_TYPE_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_UpdateNatType.
// IDA_EA: dispatch @0x841C74E; ctor/dtor
// @0x844E2A2/@0x845F60A/@0x845F63A.
class DB_UpdateNatType : public DBDispatcher {
public:
    DB_UpdateNatType();
    virtual ~DB_UpdateNatType();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_UPDATE_NAT_TYPE_H_
