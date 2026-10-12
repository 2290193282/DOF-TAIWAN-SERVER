#ifndef OLDSRV_OLD_DATA_DB_AUTH_SECURITY_DB_SECURITY_CARD_DISUSE_H_
#define OLDSRV_OLD_DATA_DB_AUTH_SECURITY_DB_SECURITY_CARD_DISUSE_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: dispatch @0x842B6E6; makeRequest @0x842B790; ctor/dtor
// @0x844E8DE/@0x845E410/@0x845E440. DBTask signal 254.
class DB_SecurityCardDisuse : public DBDispatcher {
public:
    DB_SecurityCardDisuse();
    virtual ~DB_SecurityCardDisuse();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id);
};


#endif  // OLDSRV_OLD_DATA_DB_AUTH_SECURITY_DB_SECURITY_CARD_DISUSE_H_
