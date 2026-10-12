#include "old_game/economy/mall/npc_shop_timer_runtime.h"

// IDA_EA: 0x863CC06
// IDA_NAME: _ZN18TimerNPCMoodChangeC2Ev
// DEMANGLED: TimerNPCMoodChange::TimerNPCMoodChange(void)
TimerNPCMoodChange::TimerNPCMoodChange() : TimerSig() {}

// IDA_EA: 0x863F336 / 0x863F366
// IDA_NAME: _ZN18TimerNPCMoodChangeD2Ev / D0Ev
// DEMANGLED: TimerNPCMoodChange::~TimerNPCMoodChange()
TimerNPCMoodChange::~TimerNPCMoodChange() {}

// IDA_EA: 0x86381E2
// IDA_NAME: _ZN18TimerNPCMoodChange12dispatch_sigEiij
// DEMANGLED: TimerNPCMoodChange::dispatch_sig(int,int,unsigned int)
int TimerNPCMoodChange::dispatch_sig(int /*object_id*/,
                                     int /*param1*/,
                                     unsigned int /*param2*/) {
    return 1;
}

// IDA_EA: 0x86381EC
// IDA_NAME: _ZN18TimerNPCMoodChange15registNextTimerEl
// DEMANGLED: TimerNPCMoodChange::registNextTimer(long)
void TimerNPCMoodChange::registNextTimer(long /*delay_seconds*/) {
    // The complete original body is empty.
}
