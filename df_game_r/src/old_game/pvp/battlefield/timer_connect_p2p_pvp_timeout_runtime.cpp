#include "old_game/pvp/battlefield/timer_connect_p2p_pvp_timeout_runtime.h"

#include "common/manager/global_lifecycle.h"
#include "old_game/pvp/arena/pvp_room.h"

// IDA_EA: 0x863CEE2
// IDA_NAME: _ZN26Timer_ConnectP2PPvPTimeoutC2Ev
// DEMANGLED: Timer_ConnectP2PPvPTimeout::Timer_ConnectP2PPvPTimeout(void)
Timer_ConnectP2PPvPTimeout::Timer_ConnectP2PPvPTimeout() : TimerSig() {}

// IDA_EA: 0x863EB3E (D2), 0x863EB6E (D0)
// IDA_NAME: _ZN26Timer_ConnectP2PPvPTimeoutD2Ev
// IDA_NAME: _ZN26Timer_ConnectP2PPvPTimeoutD0Ev
// DEMANGLED: Timer_ConnectP2PPvPTimeout::~Timer_ConnectP2PPvPTimeout()
Timer_ConnectP2PPvPTimeout::~Timer_ConnectP2PPvPTimeout() {}

// IDA_EA: 0x8639834
// IDA_NAME: _ZN26Timer_ConnectP2PPvPTimeout12dispatch_sigEiij
// DEMANGLED: Timer_ConnectP2PPvPTimeout::dispatch_sig(int,int,unsigned int)
int Timer_ConnectP2PPvPTimeout::dispatch_sig(int pvp_index,
                                             int timer_key,
                                             unsigned int) {
    PvP_Room* const room = G_CGameManager()->GetPvp(pvp_index, 0, 0);
    if (!room ||
        !room->CheckTimerKey(static_cast<TIMER_MESSAGE>(59), timer_key)) {
        return 0;
    }

    room->OnConnectP2PPvPTimeout();
    return 1;
}
