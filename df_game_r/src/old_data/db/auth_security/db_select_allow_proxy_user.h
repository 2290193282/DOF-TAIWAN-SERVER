#ifndef OLDSRV_DB_DB_SELECT_ALLOW_PROXY_USER_H_
#define OLDSRV_DB_DB_SELECT_ALLOW_PROXY_USER_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_SelectAllowProxyUser.
// IDA_EA: dispatch @0x844D17C; makeRequest @0x844D2D4; ctor/dtor
// @0x844FFF2/@0x845A70A/@0x845A73A. makeRequest emits signal 850.
class DB_SelectAllowProxyUser : public DBDispatcher {
public:
    DB_SelectAllowProxyUser();
    virtual ~DB_SelectAllowProxyUser();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx, unsigned int account_id);
};

#endif  // OLDSRV_DB_DB_SELECT_ALLOW_PROXY_USER_H_
