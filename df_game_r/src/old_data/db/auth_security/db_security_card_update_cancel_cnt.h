#ifndef OLDSRV_OLD_DATA_DB_AUTH_SECURITY_DB_SECURITY_CARD_UPDATE_CANCEL_CNT_H_
#define OLDSRV_OLD_DATA_DB_AUTH_SECURITY_DB_SECURITY_CARD_UPDATE_CANCEL_CNT_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: dispatch @0x842C08E; makeRequest @0x842C112; ctor/dtor
// @0x844E94E/@0x845E2D8/@0x845E308. DBTask signal 258.
class DB_SecurityCardUpdateCancelCnt : public DBDispatcher {
public:
    DB_SecurityCardUpdateCancelCnt();
    virtual ~DB_SecurityCardUpdateCancelCnt();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id,
                            unsigned int cancel_count);
};


#endif  // OLDSRV_OLD_DATA_DB_AUTH_SECURITY_DB_SECURITY_CARD_UPDATE_CANCEL_CNT_H_
