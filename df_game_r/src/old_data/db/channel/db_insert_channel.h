#ifndef OLDSRV_OLD_DATA_DB_CHANNEL_DB_INSERT_CHANNEL_H_
#define OLDSRV_OLD_DATA_DB_CHANNEL_DB_INSERT_CHANNEL_H_

#include "old_data/channel/channel_db_payloads.h"
#include "old_data/db/core/db_dispatcher.h"

// IDA unit: 03_channel_select/DB_InsertChannel.
class DB_InsertChannel : public DBDispatcher {
public:
    DB_InsertChannel();
    virtual ~DB_InsertChannel();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool InsertChannel(SIG_INSERT_CHANNEL* channel);
};

#endif  // OLDSRV_OLD_DATA_DB_CHANNEL_DB_INSERT_CHANNEL_H_
