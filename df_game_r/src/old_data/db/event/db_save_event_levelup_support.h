#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_SAVE_EVENT_LEVELUP_SUPPORT_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_SAVE_EVENT_LEVELUP_SUPPORT_H_

#include "old_data/db/core/db_dispatcher.h"

// IDA unit: 11_event_runtime/DB_SaveEventLevelupSupport.
// IDA_EA: dispatch @0x844CAFE; makeRequest @0x844CC2E; ctor/dtor
// @0x844FFBA/@0x845A7A6/@0x845A7D6. DBTask slot 847.
class DB_SaveEventLevelupSupport : public DBDispatcher {
public:
    DB_SaveEventLevelupSupport();
    virtual ~DB_SaveEventLevelupSupport();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx,
                            int charac_no,
                            int level,
                            int state);
};

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_SAVE_EVENT_LEVELUP_SUPPORT_H_
