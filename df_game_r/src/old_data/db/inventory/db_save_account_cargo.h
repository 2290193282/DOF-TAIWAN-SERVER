#ifndef OLDSRV_DB_DB_SAVE_ACCOUNT_CARGO_H_
#define OLDSRV_DB_DB_SAVE_ACCOUNT_CARGO_H_

#include "old_data/db/core/db_dispatcher.h"


class CAccountCargo;

// IDA_EA: 0x843B7C2/0x843B946/0x844F43E/0x845C6D0/0x845C700.
// IDA unit: 02_login_account/DB_SaveAccountCargo.
class DB_SaveAccountCargo : public DBDispatcher {
public:
    DB_SaveAccountCargo();
    virtual ~DB_SaveAccountCargo();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static bool makeRequest(int user_idx,
                            unsigned int account_id,
                            CAccountCargo* account_cargo,
                            int server_group,
                            unsigned int charac_no);
};

#endif  // OLDSRV_DB_DB_SAVE_ACCOUNT_CARGO_H_
