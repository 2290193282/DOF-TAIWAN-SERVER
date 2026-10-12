#ifndef OLDSRV_OLD_DATA_DB_MERCENARY_DB_SAVE_MERCENARY_DATA_H_
#define OLDSRV_OLD_DATA_DB_MERCENARY_DB_SAVE_MERCENARY_DATA_H_

#include "old_data/db/core/db_dispatcher.h"

// IDA unit: 13_database_dispatch/DB_SaveMercenaryData.
class DB_SaveMercenaryData : public DBDispatcher {
public:
    DB_SaveMercenaryData();
    virtual ~DB_SaveMercenaryData();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_MERCENARY_DB_SAVE_MERCENARY_DATA_H_
