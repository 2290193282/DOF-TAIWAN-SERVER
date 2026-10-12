#ifndef OLDSRV_DB_DB_UPDATE_LOGIN_H_
#define OLDSRV_DB_DB_UPDATE_LOGIN_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 02_login_account/DB_UpdateLogin.
class DB_UpdateLogin : public DBDispatcher {
public:
    DB_UpdateLogin();
    virtual ~DB_UpdateLogin();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_UPDATE_LOGIN_H_
