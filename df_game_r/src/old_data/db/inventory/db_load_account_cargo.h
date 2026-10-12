#ifndef OLDSRV_DB_DB_LOAD_ACCOUNT_CARGO_H_
#define OLDSRV_DB_DB_LOAD_ACCOUNT_CARGO_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x843B3B6/0x843B6EC/0x844F422/0x845C71E/0x845C74E.
// IDA unit: 02_login_account/DB_LoadAccountCargo.
class DB_LoadAccountCargo : public DBDispatcher {
public:
    DB_LoadAccountCargo();
    virtual ~DB_LoadAccountCargo();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx,
                            unsigned int account_id,
                            int server_group);
};

#endif  // OLDSRV_DB_DB_LOAD_ACCOUNT_CARGO_H_
