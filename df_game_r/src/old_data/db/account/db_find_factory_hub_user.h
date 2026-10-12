#ifndef OLDSRV_DB_DB_FIND_FACTORY_HUB_USER_H_
#define OLDSRV_DB_DB_FIND_FACTORY_HUB_USER_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_Find_Factory_Hub_User.
// IDA_EA: dispatch @0x8437A32; makeRequest @0x8437C50;
// ctor/dtor @0x844F0BE/@0x845D042/@0x845D072.
class DB_Find_Factory_Hub_User : public DBDispatcher {
public:
    DB_Find_Factory_Hub_User();
    virtual ~DB_Find_Factory_Hub_User();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int user_idx,
                            char* charac_name,
                            unsigned short value);
};

#endif  // OLDSRV_DB_DB_FIND_FACTORY_HUB_USER_H_
