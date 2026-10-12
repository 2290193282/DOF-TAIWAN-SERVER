#ifndef OLDSRV_DB_DB_SAVE_PU_USER_CHECK_H_
#define OLDSRV_DB_DB_SAVE_PU_USER_CHECK_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_SavePuUserCheck.
// IDA_EA: dispatch @0x844BFB8; makeRequest @0x844C098;
// ctor/dtor @0x844FC1E/@0x845A9FA/@0x845AA2A.
class DB_SavePuUserCheck : public DBDispatcher {
public:
    DB_SavePuUserCheck();
    virtual ~DB_SavePuUserCheck();

    static void makeRequest(unsigned int m_id);
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_SAVE_PU_USER_CHECK_H_
