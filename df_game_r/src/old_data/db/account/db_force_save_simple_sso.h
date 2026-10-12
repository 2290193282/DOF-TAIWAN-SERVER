#ifndef OLDSRV_DB_DB_FORCE_SAVE_SIMPLE_SSO_H_
#define OLDSRV_DB_DB_FORCE_SAVE_SIMPLE_SSO_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_ForceSaveSimpleSSO.
// IDA_EA: dispatch @0x8428154; makeRequest @0x84281AE; ctor/dtor
// @0x844E71E/@0x845E8A2/@0x845E8D2. Old DBTask slot 207.
class DB_ForceSaveSimpleSSO : public DBDispatcher {
public:
    DB_ForceSaveSimpleSSO();
    virtual ~DB_ForceSaveSimpleSSO();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id, int human_certify);
};

#endif  // OLDSRV_DB_DB_FORCE_SAVE_SIMPLE_SSO_H_
