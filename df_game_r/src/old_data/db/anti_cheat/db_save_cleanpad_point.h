#ifndef OLDSRV_DB_DB_SAVE_CLEANPAD_POINT_H_
#define OLDSRV_DB_DB_SAVE_CLEANPAD_POINT_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x8432F3E/0x8432FE2/0x844EA9E/0x845D7E0/0x845D810.
// IDA unit: 13_database_dispatch/DB_SaveCleanpadPoint.
class DB_SaveCleanpadPoint : public DBDispatcher {
public:
    DB_SaveCleanpadPoint();
    virtual ~DB_SaveCleanpadPoint();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int cleanpad_point, int account_id);
};

#endif  // OLDSRV_DB_DB_SAVE_CLEANPAD_POINT_H_
