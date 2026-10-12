#ifndef OLDSRV_DB_DB_SECURITY_CARD_ACTIVATE_H_
#define OLDSRV_DB_DB_SECURITY_CARD_ACTIVATE_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x842B86C/0x842BB5C/0x844E8FA/0x845E3C2/0x845E3F2.
// IDA unit: 02_login_account/DB_SecurityCardActivate.
class DB_SecurityCardActivate : public DBDispatcher {
public:
    DB_SecurityCardActivate();
    virtual ~DB_SecurityCardActivate();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx,
                            unsigned int account_id,
                            const char* cert_key,
                            bool reissue_history);
};

#endif  // OLDSRV_DB_DB_SECURITY_CARD_ACTIVATE_H_
