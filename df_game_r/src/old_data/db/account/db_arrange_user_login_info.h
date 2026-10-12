#ifndef OLDSRV_DB_DB_ARRANGE_USER_LOGIN_INFO_H_
#define OLDSRV_DB_DB_ARRANGE_USER_LOGIN_INFO_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x842ACF4/0x842AD30/0x844E8A6/0x845E4AC/0x845E4DC.
// IDA unit: 02_login_account/DB_ArrangeUserLoginInfo.
class DB_ArrangeUserLoginInfo : public DBDispatcher {
public:
    DB_ArrangeUserLoginInfo();
    virtual ~DB_ArrangeUserLoginInfo();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id,
                            bool expire_login_time,
                            unsigned int user_idx);
};

#endif  // OLDSRV_DB_DB_ARRANGE_USER_LOGIN_INFO_H_
