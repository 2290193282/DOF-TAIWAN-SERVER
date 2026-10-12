#ifndef OLDSRV_OLD_DATA_DB_CHANNEL_DB_DELETE_CHANNEL_H_
#define OLDSRV_OLD_DATA_DB_CHANNEL_DB_DELETE_CHANNEL_H_

#include "old_data/channel/channel_db_payloads.h"
#include "old_data/db/core/db_dispatcher.h"

// IDA unit: 03_channel_select/DB_DeleteChannel.
class DB_DeleteChannel : public DBDispatcher {
public:
    DB_DeleteChannel();
    virtual ~DB_DeleteChannel();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool DeleteChannel(SIG_DELETE_CHANNEL* channel);
    bool SendResult(int src, int src_1, SIG_DELETE_CHANNEL* channel);
};

#endif  // OLDSRV_OLD_DATA_DB_CHANNEL_DB_DELETE_CHANNEL_H_
