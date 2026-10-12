#ifndef OLDSRV_DB_DB_UPDATE_REVENGE_DATA_H_
#define OLDSRV_DB_DB_UPDATE_REVENGE_DATA_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_UpdateRevengeData.
// IDA_EA: makeRequest @0x843F232; dispatch @0x843F34C; ctor/dtor
// @0x844F68A/@0x845BEE4/@0x845BF14. DBTask/makeRequest signal 580.
class DB_UpdateRevengeData : public DBDispatcher {
public:
    DB_UpdateRevengeData();
    virtual ~DB_UpdateRevengeData();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int charac_no,
                            unsigned char attack_count,
                            unsigned char revenge_count);
};

#endif  // OLDSRV_DB_DB_UPDATE_REVENGE_DATA_H_
