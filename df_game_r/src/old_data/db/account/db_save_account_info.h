#ifndef OLDSRV_DB_DB_SAVE_ACCOUNT_INFO_H_
#define OLDSRV_DB_DB_SAVE_ACCOUNT_INFO_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 02_login_account/DB_SaveAccountInfo.
class DB_SaveAccountInfo : public DBDispatcher {
public:
    DB_SaveAccountInfo();
    virtual ~DB_SaveAccountInfo();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_SAVE_ACCOUNT_INFO_H_
