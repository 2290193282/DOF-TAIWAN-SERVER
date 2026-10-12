#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_UPDATE_INGAME_EVENT_HISTORY_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_UPDATE_INGAME_EVENT_HISTORY_H_

#include "old_data/db/core/db_dispatcher.h"

// IDA unit: 11_event_runtime/DB_UpdateInGameEventHistory.
// IDA_EA: dispatch @0x8444972; makeRequest @0x8444AC4; ctor/dtor
// @0x844F9EE/@0x845B65C/@0x845B68C. DBTask slot 685.
class DB_UpdateInGameEventHistory : public DBDispatcher {
public:
    DB_UpdateInGameEventHistory();
    virtual ~DB_UpdateInGameEventHistory();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id,
                            int event_type,
                            int history_1,
                            int history_2);
};

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_UPDATE_INGAME_EVENT_HISTORY_H_
