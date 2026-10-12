#ifndef OLDSRV_OLD_DATA_DB_CHANNEL_DB_UPDATE_DETAIL_CHANNEL_H_
#define OLDSRV_OLD_DATA_DB_CHANNEL_DB_UPDATE_DETAIL_CHANNEL_H_

#include "old_data/channel/channel_db_payloads.h"
#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 03_channel_select/DB_UpdateDetailChannel.
class DB_UpdateDetailChannel : public DBDispatcher {
public:
    DB_UpdateDetailChannel();
    virtual ~DB_UpdateDetailChannel();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_CHANNEL_DB_UPDATE_DETAIL_CHANNEL_H_
