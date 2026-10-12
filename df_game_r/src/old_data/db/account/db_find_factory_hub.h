#ifndef OLDSRV_DB_DB_FIND_FACTORY_HUB_H_
#define OLDSRV_DB_DB_FIND_FACTORY_HUB_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_FindFactoryHub.
// IDA_EA: dispatch @0x8436BA0; ctor/dtor @0x844F016/@0x845D264/@0x845D294.
class DB_FindFactoryHub : public DBDispatcher {
public:
    DB_FindFactoryHub();
    virtual ~DB_FindFactoryHub();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_FIND_FACTORY_HUB_H_
