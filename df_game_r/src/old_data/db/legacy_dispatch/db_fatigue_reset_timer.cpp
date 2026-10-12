#include "old_data/db/legacy_dispatch/db_fatigue_reset_timer.h"


// IDA_EA: 0x844DBAC
// IDA_NAME: _ZN20DB_FatigueResetTimerC2Ev
// DEMANGLED: DB_FatigueResetTimer::DB_FatigueResetTimer(void)
DB_FatigueResetTimer::DB_FatigueResetTimer() {}

// IDA_EA: 0x844DBC8
// IDA_NAME: _ZN20DB_FatigueResetTimerD2Ev
// DEMANGLED: DB_FatigueResetTimer::~DB_FatigueResetTimer()
// IDA_EA: 0x844DBF8
// IDA_NAME: _ZN20DB_FatigueResetTimerD0Ev
// DEMANGLED: DB_FatigueResetTimer::~DB_FatigueResetTimer() deleting destructor
DB_FatigueResetTimer::~DB_FatigueResetTimer() {}

// IDA_EA: 0x841ABBC
// IDA_NAME: _ZN20DB_FatigueResetTimer8dispatchEiiP6Stream
// DEMANGLED: DB_FatigueResetTimer::dispatch(int,int,Stream *)
bool DB_FatigueResetTimer::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;
    (void)stream;
    return true;
}
