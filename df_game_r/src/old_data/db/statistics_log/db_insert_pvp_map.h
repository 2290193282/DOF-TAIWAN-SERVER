#ifndef OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_PVP_MAP_H_
#define OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_PVP_MAP_H_

#include "old_data/db/core/db_dispatcher.h"

// IDA unit: 05_world_enter_village/DB_InsertPVPMap.
class DB_InsertPVPMap : public DBDispatcher {
public:
    DB_InsertPVPMap();
    virtual ~DB_InsertPVPMap();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_PVP_MAP_H_
