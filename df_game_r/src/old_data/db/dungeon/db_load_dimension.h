#ifndef OLDSRV_DB_DB_LOAD_DIMENSION_H_
#define OLDSRV_DB_DB_LOAD_DIMENSION_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x8431D8C/0x8431ECE/0x844EE02/0x845D966/0x845D996.
// IDA unit: 13_database_dispatch/DB_LoadDimension.
class DB_LoadDimension : public DBDispatcher {
public:
    DB_LoadDimension();
    virtual ~DB_LoadDimension();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int uid, unsigned int charac_no);
};

#endif  // OLDSRV_DB_DB_LOAD_DIMENSION_H_
