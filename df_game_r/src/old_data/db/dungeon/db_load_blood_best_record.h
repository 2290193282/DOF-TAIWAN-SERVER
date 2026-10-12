#ifndef OLDSRV_DB_DB_LOAD_BLOOD_BEST_RECORD_H_
#define OLDSRV_DB_DB_LOAD_BLOOD_BEST_RECORD_H_

#include "old_data/db/dungeon/blood_best_record_payload.h"
#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_LoadBloodBestRecord.
class DB_LoadBloodBestRecord : public DBDispatcher {
public:
    // IDA_EA: 0x844ECEA.
    DB_LoadBloodBestRecord();
    // IDA_EA: 0x845DA02 / 0x845DA32.
    virtual ~DB_LoadBloodBestRecord();

    // IDA_EA: 0x843199A.
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    // IDA_EA: 0x84318C4.
    static void makeRequest(int user_idx, unsigned int charac_no);
};

#endif  // OLDSRV_DB_DB_LOAD_BLOOD_BEST_RECORD_H_
