#ifndef OLDSRV_DB_DB_UPDATE_NAT_TYPE_USER_H_
#define OLDSRV_DB_DB_UPDATE_NAT_TYPE_USER_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x841C7F6/0x844E2BE/0x845F5BC/0x845F5EC.
// IDA unit: 13_database_dispatch/DB_UpdateNatTypeUser.
class DB_UpdateNatTypeUser : public DBDispatcher {
public:
    DB_UpdateNatTypeUser();
    virtual ~DB_UpdateNatTypeUser();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_UPDATE_NAT_TYPE_USER_H_
