#ifndef OLDSRV_DB_DB_UPDATE_BINGO_EVENT_H_
#define OLDSRV_DB_DB_UPDATE_BINGO_EVENT_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 11_event_runtime/DB_UpdateBingoEvent.
class DB_UpdateBingoEvent : public DBDispatcher {
public:
    DB_UpdateBingoEvent();
    virtual ~DB_UpdateBingoEvent();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx,
                            unsigned int account_id,
                            int bingo_count,
                            bool selected_count);
};

#endif  // OLDSRV_DB_DB_UPDATE_BINGO_EVENT_H_
