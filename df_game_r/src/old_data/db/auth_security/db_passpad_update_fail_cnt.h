#ifndef OLDSRV_DB_DB_PASSPAD_UPDATE_FAIL_CNT_H_
#define OLDSRV_DB_DB_PASSPAD_UPDATE_FAIL_CNT_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x8423BC4/0x8423EB4/0x844E4D2/0x845EFA4/0x845EFD4.
// IDA unit: 13_database_dispatch/DB_PassPadUpdateFailCnt.
class DB_PassPadUpdateFailCnt : public DBDispatcher {
public:
    DB_PassPadUpdateFailCnt();
    virtual ~DB_PassPadUpdateFailCnt();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id,
                            unsigned int fail_count,
                            char* ip);
};

#endif  // OLDSRV_DB_DB_PASSPAD_UPDATE_FAIL_CNT_H_
