#ifndef OLDSRV_DB_DB_CREATE_ACCOUNT_CARGO_H_
#define OLDSRV_DB_DB_CREATE_ACCOUNT_CARGO_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x843AC52/0x843AE86/0x844F3CE/0x845C808/0x845C838.
// IDA unit: 02_login_account/DB_CreateAccountCargo.
class DB_CreateAccountCargo : public DBDispatcher {
public:
    DB_CreateAccountCargo();
    virtual ~DB_CreateAccountCargo();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx,
                            unsigned int account_id,
                            unsigned int capacity);
};

#endif  // OLDSRV_DB_DB_CREATE_ACCOUNT_CARGO_H_
