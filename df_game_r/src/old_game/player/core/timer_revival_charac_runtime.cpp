#include "old_game/player/core/timer_revival_charac_runtime.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x863CD22
// IDA_NAME: _ZN18TimerRevivalCharacC2Ev
// DEMANGLED: TimerRevivalCharac::TimerRevivalCharac(void)
TimerRevivalCharac::TimerRevivalCharac() : TimerSig() {}

// IDA_EA: 0x863EFD0 / 0x863F000
// IDA_NAME: _ZN18TimerRevivalCharacD2Ev / _ZN18TimerRevivalCharacD0Ev
// DEMANGLED: TimerRevivalCharac::~TimerRevivalCharac()
TimerRevivalCharac::~TimerRevivalCharac() {}

// IDA_EA: 0x8638E78
// IDA_NAME: _ZN18TimerRevivalCharac12dispatch_sigEiij
// DEMANGLED: TimerRevivalCharac::dispatch_sig(int,int,unsigned int)
int TimerRevivalCharac::dispatch_sig(int user_uid,
                                     int revival_time,
                                     unsigned int) {
    CUser* const user = G_CGameManager()->getUser(user_uid, 2);
    if (!user || !user->getCurCharacR() ||
        user->GetRevivalTime() != revival_time) {
        return 0;
    }

    PacketGuard packet;
    packet.put_header(0, 228);
    packet.finalize(true);
    user->Send(packet);
    return 1;
}
