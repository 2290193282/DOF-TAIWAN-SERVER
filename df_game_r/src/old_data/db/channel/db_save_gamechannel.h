#ifndef OLDSRV_OLD_DATA_DB_CHANNEL_DB_SAVE_GAMECHANNEL_H_
#define OLDSRV_OLD_DATA_DB_CHANNEL_DB_SAVE_GAMECHANNEL_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 03_channel_select/DB_SaveGamechannel.
class DB_SaveGamechannel : public DBDispatcher {
public:
    DB_SaveGamechannel();
    virtual ~DB_SaveGamechannel();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_CHANNEL_DB_SAVE_GAMECHANNEL_H_
