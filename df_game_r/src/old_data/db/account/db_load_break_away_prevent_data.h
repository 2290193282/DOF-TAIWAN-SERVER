#ifndef OLDSRV_DB_DB_LOAD_BREAK_AWAY_PREVENT_DATA_H_
#define OLDSRV_DB_DB_LOAD_BREAK_AWAY_PREVENT_DATA_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 11_event_runtime/DB_LoadBreakAwayPreventData.
// IDA_EA: makeRequest @0x8438920; dispatch @0x84389FE; ctor/dtor
// @0x844ED22/@0x845CDD2/@0x845CE02.
// makeRequest emits DBTask signal 321.
class DB_LoadBreakAwayPreventData : public DBDispatcher {
public:
    DB_LoadBreakAwayPreventData();
    virtual ~DB_LoadBreakAwayPreventData();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(bool enabled);
};

#endif  // OLDSRV_DB_DB_LOAD_BREAK_AWAY_PREVENT_DATA_H_
