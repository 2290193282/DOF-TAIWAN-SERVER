#include "old_data/Login/inter_returns/inter_load_break_away_prevent_system.h"

#include "common/manager/global_data.h"
#include "old_data/Login/inter_returns/break_away_prevent_runtime.h"
#include "old_game/activity/schedule/event_messages.h"

// IDA_EA: 0x84EDF18
// IDA_NAME: _ZN32Inter_LoadBreakAwayPreventSystemC2Ev
Inter_LoadBreakAwayPreventSystem::Inter_LoadBreakAwayPreventSystem()
    : ISigDispatcher() {
}

// IDA_EA: 0x84F8286 / 0x84F82B6
// IDA_NAME: _ZN32Inter_LoadBreakAwayPreventSystemD2Ev / D0Ev
Inter_LoadBreakAwayPreventSystem::~Inter_LoadBreakAwayPreventSystem() {
}

// IDA_EA: 0x84D928C
// IDA_NAME: _ZN32Inter_LoadBreakAwayPreventSystem12dispatch_sigEP5CUserPci
int Inter_LoadBreakAwayPreventSystem::dispatch_sig(CUser*,
                                                   char* payload,
                                                   int) {
    GlobalData::s_BreakAwaySys->loadDbData(
        reinterpret_cast<SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER*>(payload));
    return 0;
}
