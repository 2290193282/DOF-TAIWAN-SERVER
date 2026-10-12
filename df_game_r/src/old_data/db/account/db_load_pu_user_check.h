#ifndef OLDSRV_DB_DB_LOAD_PU_USER_CHECK_H_
#define OLDSRV_DB_DB_LOAD_PU_USER_CHECK_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_LoadPuUserCheck.
// IDA_EA: dispatch @0x844BD5C; makeRequest @0x844BEE2;
// ctor/dtor @0x844FC02/@0x845AA48/@0x845AA78.
class DB_LoadPuUserCheck : public DBDispatcher {
public:
    DB_LoadPuUserCheck();
    virtual ~DB_LoadPuUserCheck();

    static void makeRequest(unsigned int m_id, int user_idx);
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_LOAD_PU_USER_CHECK_H_
