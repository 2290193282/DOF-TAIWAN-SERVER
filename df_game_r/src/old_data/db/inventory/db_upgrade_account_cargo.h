#ifndef OLDSRV_DB_DB_UPGRADE_ACCOUNT_CARGO_H_
#define OLDSRV_DB_DB_UPGRADE_ACCOUNT_CARGO_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x843AF76/0x843B15E/0x844F3EA/0x845C7BA/0x845C7EA.
// IDA unit: 02_login_account/DB_UpgradeAccountCargo.
class DB_UpgradeAccountCargo : public DBDispatcher {
public:
    DB_UpgradeAccountCargo();
    virtual ~DB_UpgradeAccountCargo();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx,
                            unsigned int account_id,
                            unsigned int capacity);
};

#endif  // OLDSRV_DB_DB_UPGRADE_ACCOUNT_CARGO_H_
