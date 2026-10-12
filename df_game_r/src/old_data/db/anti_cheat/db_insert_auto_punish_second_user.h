#ifndef OLDSRV_OLD_DATA_DB_ANTI_CHEAT_DB_INSERT_AUTO_PUNISH_SECOND_USER_H_
#define OLDSRV_OLD_DATA_DB_ANTI_CHEAT_DB_INSERT_AUTO_PUNISH_SECOND_USER_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: dispatch @0x842DEB4; makeRequest @0x842E34E; ctor/dtor
// @0x844EA66/@0x845E01A/@0x845E04A. DBTask signal 267.
class DB_InsertAutoPunishSecondUser : public DBDispatcher {
public:
    DB_InsertAutoPunishSecondUser();
    virtual ~DB_InsertAutoPunishSecondUser();

    // IDA_NAME: _ZN29DB_InsertAutoPunishSecondUser8dispatchEiiP6Stream
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id,
                            unsigned int hack_account_id,
                            unsigned long long trade_gold,
                            bool punish);
};


#endif  // OLDSRV_OLD_DATA_DB_ANTI_CHEAT_DB_INSERT_AUTO_PUNISH_SECOND_USER_H_
