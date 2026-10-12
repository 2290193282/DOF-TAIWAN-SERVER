#ifndef OLDSRV_DB_CLOCALCHINA_DB_LAST_DAY_BREAK_AWAY_H_
#define OLDSRV_DB_CLOCALCHINA_DB_LAST_DAY_BREAK_AWAY_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/CLocalChina_DB_LastDay_BreakAway.
// IDA_EA: dispatch @0x812801A; makeRequest @0x8128152; ctor/dtor
// @0x844FBAE/@0x8128B82/@0x8128BB2. DBTask old slot 734.
class CLocalChina_DB_LastDay_BreakAway : public DBDispatcher {
public:
    CLocalChina_DB_LastDay_BreakAway();
    virtual ~CLocalChina_DB_LastDay_BreakAway();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id,
                            int day_count,
                            int reward_kind);
};

#endif  // OLDSRV_DB_CLOCALCHINA_DB_LAST_DAY_BREAK_AWAY_H_
