#ifndef OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_PVP_PLAYER_RATE_H_
#define OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_PVP_PLAYER_RATE_H_

#include "old_data/db/core/db_dispatcher.h"

// IDA unit: 09_dungeon_pvp_party/DB_InsertPVPPlayerRate.
class DB_InsertPVPPlayerRate : public DBDispatcher {
public:
    DB_InsertPVPPlayerRate();
    virtual ~DB_InsertPVPPlayerRate();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_PVP_PLAYER_RATE_H_
