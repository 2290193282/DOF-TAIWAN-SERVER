#ifndef OLDSRV_OLD_DATA_DB_GAME_OPTION_DB_UPDATE_DIMENSION_H_
#define OLDSRV_OLD_DATA_DB_GAME_OPTION_DB_UPDATE_DIMENSION_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x843230A/0x844EE1E/0x845D918/0x845D948.
// IDA unit: 13_database_dispatch/DB_UpdateDimension.
class DB_UpdateDimension : public DBDispatcher {
public:
    DB_UpdateDimension();
    virtual ~DB_UpdateDimension();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_GAME_OPTION_DB_UPDATE_DIMENSION_H_
