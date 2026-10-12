#ifndef OLDSRV_DB_DB_UPDATE_BLOOD_BEST_RECORD_H_
#define OLDSRV_DB_DB_UPDATE_BLOOD_BEST_RECORD_H_

#include "old_data/db/dungeon/blood_best_record_payload.h"
#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_UpdateBloodBestRecord.
class DB_UpdateBloodBestRecord : public DBDispatcher {
public:
    // IDA_EA: 0x844ED06.
    DB_UpdateBloodBestRecord();
    // IDA_EA: 0x845D9B4 / 0x845D9E4.
    virtual ~DB_UpdateBloodBestRecord();

    // IDA_EA: 0x8431C74.
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_UPDATE_BLOOD_BEST_RECORD_H_
