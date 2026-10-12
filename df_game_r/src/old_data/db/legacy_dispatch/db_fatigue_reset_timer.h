#ifndef OLDSRV_DB_DB_FATIGUE_RESET_TIMER_H_
#define OLDSRV_DB_DB_FATIGUE_RESET_TIMER_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_FatigueResetTimer.
// IDA_EA: dispatch @0x841ABBC; ctor/dtor @0x844DBAC/@0x844DBC8/@0x844DBF8.
class DB_FatigueResetTimer : public DBDispatcher {
public:
    DB_FatigueResetTimer();
    virtual ~DB_FatigueResetTimer();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_FATIGUE_RESET_TIMER_H_
