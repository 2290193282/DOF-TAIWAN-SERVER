#ifndef OLDSRV_DB_DB_UPDATE_RETURN_USER_FIRST_LOGIN_H_
#define OLDSRV_DB_DB_UPDATE_RETURN_USER_FIRST_LOGIN_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x844F5C6/0x845C106/0x845C136/0x843DFFE/0x843E146.
// IDA unit: 02_login_account/DB_UpdateReturnUserFirstLogin.
class DB_UpdateReturnUserFirstLogin : public DBDispatcher {
public:
    DB_UpdateReturnUserFirstLogin();
    virtual ~DB_UpdateReturnUserFirstLogin();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int m_id, int first_login,
                            int expire_time);
};

#endif  // OLDSRV_DB_DB_UPDATE_RETURN_USER_FIRST_LOGIN_H_
