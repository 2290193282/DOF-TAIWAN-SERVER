#ifndef OLDSRV_DB_DB_PASSPAD_UPDATE_CANCEL_CNT_H_
#define OLDSRV_DB_DB_PASSPAD_UPDATE_CANCEL_CNT_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x8423FB6/0x842403A/0x844E4EE/0x845EF56/0x845EF86.
// IDA unit: 13_database_dispatch/DB_PassPadUpdateCancelCnt.
class DB_PassPadUpdateCancelCnt : public DBDispatcher {
public:
    DB_PassPadUpdateCancelCnt();
    virtual ~DB_PassPadUpdateCancelCnt();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id,
                            unsigned int cancel_count,
                            char* ip);
};

#endif  // OLDSRV_DB_DB_PASSPAD_UPDATE_CANCEL_CNT_H_
