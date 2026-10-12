#ifndef OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_PARTY_TYPE_H_
#define OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_PARTY_TYPE_H_

#include "old_data/db/core/db_dispatcher.h"

// IDA unit: 09_dungeon_pvp_party/DB_InsertPartyType.
class DB_InsertPartyType : public DBDispatcher {
public:
    DB_InsertPartyType();
    virtual ~DB_InsertPartyType();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_STATISTICS_LOG_DB_INSERT_PARTY_TYPE_H_
