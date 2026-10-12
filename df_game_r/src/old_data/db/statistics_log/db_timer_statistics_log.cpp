#include "old_data/db/statistics_log/db_timer_statistics_log.h"

#include "common/manager/crashdown_abi.h"
#include "old_data/db/legacy_dispatch/db_crashdown_advertisement_dispatch.h"
#include "old_data/db/legacy_dispatch/db_legacy_stat_dispatchers.h"
#include "old_data/db/pvp_party/db_pvp_tower_remaining_dispatchers.h"

// IDA_EA: 0x844E6E6
// IDA_NAME: _ZN21DB_TimerStatisticsLogC2Ev
// DEMANGLED: DB_TimerStatisticsLog::DB_TimerStatisticsLog(void)
DB_TimerStatisticsLog::DB_TimerStatisticsLog() {}

// IDA_EA: 0x845E93E
// IDA_NAME: _ZN21DB_TimerStatisticsLogD2Ev
// DEMANGLED: DB_TimerStatisticsLog::~DB_TimerStatisticsLog()
// IDA_EA: 0x845E96E
// IDA_NAME: _ZN21DB_TimerStatisticsLogD0Ev
// DEMANGLED: DB_TimerStatisticsLog::~DB_TimerStatisticsLog()
// The C++ destructor supplies the D2 body; the compiler emits the D0 deleting
// wrapper for the same Itanium ABI destructor identity.
DB_TimerStatisticsLog::~DB_TimerStatisticsLog() {}

// IDA_EA: 0x8427C06
// IDA_NAME: _ZN21DB_TimerStatisticsLog8dispatchEiiP6Stream
// DEMANGLED: DB_TimerStatisticsLog::dispatch(int,int,Stream *)
// The original uses function-local instances so their dispatch paths share no
// mutable request state.  Preserve that periodic sequence: crash observations,
// dungeon failures, PvP room totals, Shusia use totals, then carried client
// crash records.  IDA 0x8427C62..0x8427DF0 forwards the exact incoming
// (signal, user_idx, stream) tuple to every sub-dispatcher, ignores all their
// return values, calls CarryClientCrashDown without a null branch, and returns
// one unconditionally.
bool DB_TimerStatisticsLog::dispatch(int signal, int user_idx, Stream* stream) {
    static DB_CrashDownReport crash_down_report;
    static DB_StatDungeonFail stat_dungeon_fail;
    static DB_StatPVPPlay2 stat_pvp_play2;
    static DB_StatUseShusiaService stat_use_shusia_service;

    crash_down_report.dispatch(signal, user_idx, stream);
    stat_dungeon_fail.dispatch(signal, user_idx, stream);
    stat_pvp_play2.dispatch(signal, user_idx, stream);
    stat_use_shusia_service.dispatch(signal, user_idx, stream);
    GetInstanceClientCrashDownLogStatistic()->CarryClientCrashDown();
    return true;
}
