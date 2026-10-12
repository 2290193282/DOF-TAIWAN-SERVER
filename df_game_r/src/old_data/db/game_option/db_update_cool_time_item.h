#ifndef OLDSRV_OLD_DATA_DB_GAME_OPTION_DB_UPDATE_COOL_TIME_ITEM_H_
#define OLDSRV_OLD_DATA_DB_GAME_OPTION_DB_UPDATE_COOL_TIME_ITEM_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x8430570/0x844EC5E/0x845DB88/0x845DBB8.
// IDA unit: 13_database_dispatch/DB_UpdateCoolTimeItem.
class DB_UpdateCoolTimeItem : public DBDispatcher {
public:
    DB_UpdateCoolTimeItem();
    virtual ~DB_UpdateCoolTimeItem();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_GAME_OPTION_DB_UPDATE_COOL_TIME_ITEM_H_
