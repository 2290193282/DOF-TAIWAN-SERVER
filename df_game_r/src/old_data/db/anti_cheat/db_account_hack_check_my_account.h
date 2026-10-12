#ifndef OLDSRV_OLD_DATA_DB_ANTI_CHEAT_DB_ACCOUNT_HACK_CHECK_MY_ACCOUNT_H_
#define OLDSRV_OLD_DATA_DB_ANTI_CHEAT_DB_ACCOUNT_HACK_CHECK_MY_ACCOUNT_H_

#include "old_data/db/core/db_dispatcher.h"

// IDA_EA: dispatch @0x8439F06; makeRequest @0x843A13E; ctor/dtor
// @0x844F1BA/@0x845CB62/@0x845CB92. Request signal 395, reply 396.
class DB_AccountHack_CheckMyAccount : public DBDispatcher {
public:
    DB_AccountHack_CheckMyAccount();
    virtual ~DB_AccountHack_CheckMyAccount();

    // IDA_NAME: _ZN29DB_AccountHack_CheckMyAccount8dispatchEiiP6Stream
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int user_idx,
                            unsigned int account_id,
                            unsigned int reply_uid,
                            unsigned int package_money,
                            char* charac_name,
                            int name_length);
};


#endif  // OLDSRV_OLD_DATA_DB_ANTI_CHEAT_DB_ACCOUNT_HACK_CHECK_MY_ACCOUNT_H_
