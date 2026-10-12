#ifndef OLDSRV_DB_DB_DELETE_ACCOUNT_CARGO_H_
#define OLDSRV_DB_DB_DELETE_ACCOUNT_CARGO_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x843B24E/0x843B2E0/0x844F406/0x845C76C/0x845C79C.
// IDA unit: 02_login_account/DB_DeleteAccountCargo.
class DB_DeleteAccountCargo : public DBDispatcher {
public:
    DB_DeleteAccountCargo();
    virtual ~DB_DeleteAccountCargo();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx, unsigned int account_id);
};

#endif  // OLDSRV_DB_DB_DELETE_ACCOUNT_CARGO_H_
