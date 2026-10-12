#ifndef OLDSRV_DB_DB_SECURITY_CARD_UPDATE_VALIDITY_TIME_H_
#define OLDSRV_DB_DB_SECURITY_CARD_UPDATE_VALIDITY_TIME_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x842BC64/0x842BCE8/0x844E916/0x845E374/0x845E3A4.
// IDA unit: 02_login_account/DB_SecurityCardUpdateValidityTime.
class DB_SecurityCardUpdateValidityTime : public DBDispatcher {
public:
    DB_SecurityCardUpdateValidityTime();
    virtual ~DB_SecurityCardUpdateValidityTime();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id, unsigned int seconds);
};

#endif  // OLDSRV_DB_DB_SECURITY_CARD_UPDATE_VALIDITY_TIME_H_
