#ifndef OLDSRV_OLD_DATA_DB_ANTI_CHEAT_DB_AUTO_PUNISHED_SECOND_USER_DATA_H_
#define OLDSRV_OLD_DATA_DB_ANTI_CHEAT_DB_AUTO_PUNISHED_SECOND_USER_DATA_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: dispatch @0x843A462; makeRequest @0x843A790; ctor/dtor
// @0x844F1F2/@0x845CAC6/@0x845CAF6. Request/reply signal 458.
class DB_AutoPunishedSecondUserData : public DBDispatcher {
public:
    DB_AutoPunishedSecondUserData();
    virtual ~DB_AutoPunishedSecondUserData();

    // IDA_NAME: _ZN29DB_AutoPunishedSecondUserData8dispatchEiiP6Stream
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id,
                            unsigned long long trade_gold,
                            unsigned int user_idx,
                            unsigned int related_account_id);
};


#endif  // OLDSRV_OLD_DATA_DB_ANTI_CHEAT_DB_AUTO_PUNISHED_SECOND_USER_DATA_H_
