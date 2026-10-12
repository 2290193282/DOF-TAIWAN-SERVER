#ifndef OLDSRV_OLD_DATA_DB_CHANNEL_DB_SAVE_CHANNEL_USER_LEVEL_H_
#define OLDSRV_OLD_DATA_DB_CHANNEL_DB_SAVE_CHANNEL_USER_LEVEL_H_

#include "old_data/channel/channel_db_payloads.h"
#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 03_channel_select/DB_SaveChannelUserLevel.
class DB_SaveChannelUserLevel : public DBDispatcher {
public:
    DB_SaveChannelUserLevel();
    virtual ~DB_SaveChannelUserLevel();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_CHANNEL_DB_SAVE_CHANNEL_USER_LEVEL_H_
