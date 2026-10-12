#ifndef OLDSRV_DB_DB_INSERT_ACCOUNT_FIRST_LOGIN_FLAG_H_
#define OLDSRV_DB_DB_INSERT_ACCOUNT_FIRST_LOGIN_FLAG_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x844F5E2/0x845C06A/0x845C09A/0x843E636/0x843E79A.
// IDA unit: 02_login_account/DB_InsertAccountFirstLoginFlag.
class DB_InsertAccountFirstLoginFlag : public DBDispatcher {
public:
    DB_InsertAccountFirstLoginFlag();
    virtual ~DB_InsertAccountFirstLoginFlag();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx,
                            unsigned int account_id,
                            int server_id,
                            unsigned int charac_no,
                            int event_type);
};

#endif  // OLDSRV_DB_DB_INSERT_ACCOUNT_FIRST_LOGIN_FLAG_H_
