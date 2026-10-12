#ifndef OLDSRV_OLD_DATA_DB_GAME_OPTION_DB_UPDATE_EFFECT_ITEM_H_
#define OLDSRV_OLD_DATA_DB_GAME_OPTION_DB_UPDATE_EFFECT_ITEM_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x8430676/0x844EC7A/0x845DB3A/0x845DB6A.
// IDA unit: 13_database_dispatch/DB_UpdateEffectItem.
class DB_UpdateEffectItem : public DBDispatcher {
public:
    DB_UpdateEffectItem();
    virtual ~DB_UpdateEffectItem();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_GAME_OPTION_DB_UPDATE_EFFECT_ITEM_H_
