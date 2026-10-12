#ifndef OLDSRV_DB_CLOCALCHINA_DB_BREAK_AWAY_RESET_H_
#define OLDSRV_DB_CLOCALCHINA_DB_BREAK_AWAY_RESET_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/CLocalChina_DB_BreakAwayReset.
// IDA_EA: dispatch @0x8127E84; makeRequest @0x8127F42; ctor/dtor
// @0x844FB92/@0x8128BD0/@0x8128C00. DBTask old slot 733.
class CLocalChina_DB_BreakAwayReset : public DBDispatcher {
public:
    CLocalChina_DB_BreakAwayReset();
    virtual ~CLocalChina_DB_BreakAwayReset();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id);
};

#endif  // OLDSRV_DB_CLOCALCHINA_DB_BREAK_AWAY_RESET_H_
