#ifndef OLDSRV_OLD_DATA_DB_ANTI_CHEAT_DB_INSERT_AUTO_PUNISH_FIRST_USER_H_
#define OLDSRV_OLD_DATA_DB_ANTI_CHEAT_DB_INSERT_AUTO_PUNISH_FIRST_USER_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: dispatch @0x842D6CE; makeRequest @0x842DD7C; ctor/dtor
// @0x844EA4A/@0x845E068/@0x845E098. DBTask signal 266.
class DB_InsertAutoPunishFirstUser : public DBDispatcher {
public:
    DB_InsertAutoPunishFirstUser();
    virtual ~DB_InsertAutoPunishFirstUser();

    // IDA_NAME: _ZN28DB_InsertAutoPunishFirstUser8dispatchEiiP6Stream
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id,
                            const char* ip,
                            unsigned int hack_type,
                            unsigned int count,
                            unsigned int hack_sub_type,
                            unsigned int hack_sub_count,
                            unsigned short punish_flag,
                            unsigned int punish_value);
};


#endif  // OLDSRV_OLD_DATA_DB_ANTI_CHEAT_DB_INSERT_AUTO_PUNISH_FIRST_USER_H_
