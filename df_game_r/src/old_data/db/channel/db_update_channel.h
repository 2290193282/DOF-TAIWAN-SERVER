#ifndef OLDSRV_OLD_DATA_DB_CHANNEL_DB_UPDATE_CHANNEL_H_
#define OLDSRV_OLD_DATA_DB_CHANNEL_DB_UPDATE_CHANNEL_H_

#include "old_data/channel/channel_db_payloads.h"
#include "old_data/db/core/db_dispatcher.h"

// IDA unit: 03_channel_select/DB_UpdateChannel.
class DB_UpdateChannel : public DBDispatcher {
public:
    DB_UpdateChannel();
    virtual ~DB_UpdateChannel();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool UpdateChannel(SIG_UPDATE_CHANNEL* channel);
};

#endif  // OLDSRV_OLD_DATA_DB_CHANNEL_DB_UPDATE_CHANNEL_H_
