#ifndef OLDSRV_DB_DB_SECURITY_CARD_UPDATE_FAIL_CNT_H_
#define OLDSRV_DB_DB_SECURITY_CARD_UPDATE_FAIL_CNT_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x842BDDA/0x842BFAA/0x844E932/0x845E326/0x845E356.
// IDA unit: 02_login_account/DB_SecurityCardUpdateFailCnt.
class DB_SecurityCardUpdateFailCnt : public DBDispatcher {
public:
    DB_SecurityCardUpdateFailCnt();
    virtual ~DB_SecurityCardUpdateFailCnt();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id, unsigned int fail_count);
};

#endif  // OLDSRV_DB_DB_SECURITY_CARD_UPDATE_FAIL_CNT_H_
