#ifndef OLDSRV_DB_DB_INSERT_ACCOUNT_FIRST_LOGIN_DAILY_FLAG_H_
#define OLDSRV_DB_DB_INSERT_ACCOUNT_FIRST_LOGIN_DAILY_FLAG_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 02_login_account/DB_InsertAccountFirstLoginDailyFlag.
// IDA_EA: 0x843E8BE dispatch, 0x843EA64 makeRequest,
//         0x844FF9E ctor, 0x845C01C dtor, 0x845C04C deleting dtor.
class DB_InsertAccountFirstLoginDailyFlag : public DBDispatcher {
public:
    DB_InsertAccountFirstLoginDailyFlag();
    virtual ~DB_InsertAccountFirstLoginDailyFlag();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx,
                            unsigned int account_id,
                            unsigned int charac_no,
                            int event_type);
};

#endif  // OLDSRV_DB_DB_INSERT_ACCOUNT_FIRST_LOGIN_DAILY_FLAG_H_
