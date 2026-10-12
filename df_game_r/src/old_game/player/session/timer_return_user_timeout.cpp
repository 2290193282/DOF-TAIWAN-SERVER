#include "old_game/player/session/timer_return_user_timeout.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/timer/timer_queue.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x863CF8A
// IDA_NAME: _ZN23Timer_ReturnUserTimeoutC2Ev
// DEMANGLED: Timer_ReturnUserTimeout::Timer_ReturnUserTimeout(void)
Timer_ReturnUserTimeout::Timer_ReturnUserTimeout() : TimerSig() {}

// IDA_EA: 0x863E96A
// IDA_NAME: _ZN23Timer_ReturnUserTimeoutD2Ev
// DEMANGLED: Timer_ReturnUserTimeout::~Timer_ReturnUserTimeout()
Timer_ReturnUserTimeout::~Timer_ReturnUserTimeout() {}

// IDA_EA: 0x863A4C8
// IDA_NAME: _ZN23Timer_ReturnUserTimeout12dispatch_sigEiij
// DEMANGLED: Timer_ReturnUserTimeout::dispatch_sig(int,int,unsigned int)
int Timer_ReturnUserTimeout::dispatch_sig(int user_uid,
                                          int unique_id,
                                          unsigned int /* unused_param2 */) {
    // Old dispatch uses the manager's signal-bias value 2 for this timer.
    CUser* const user = G_CGameManager()->getUser(user_uid, 2);
    if (!user) {
        return 0;
    }
    if (static_cast<uint16_t>(user->get_unique_id()) != unique_id) {
        return 0;
    }

    user->ResetReturnUser();

    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 301);
    packet.put_int(0);
    packet.finalize(true);
    user->Send(packet);

    packet.clear();
    packet.put_header(0, 2);
    packet.put_byte(0);
    packet.put_short(1);
    user->make_basic_info(packet, 0);
    packet.finalize(true);

    CParty* const party = user->GetParty();
    if (user->get_state() == 5) {
        if (party) {
            party->send_to_party(packet);
        } else {
            user->Send(packet);
        }
    } else {
        G_GameWorld()->send_all(packet, user);
    }

    if (party &&
        (party->IsReturnUserParty() != 1 ||
         party->IsEventCharacParty() != 1)) {
        G_GameWorld()->send_party_info_to_all(party, 1);
    }
    return 1;
}

// IDA_EA: 0x863A706
// IDA_NAME: _ZN23Timer_ReturnUserTimeout15registNextTimerEiij
// DEMANGLED: Timer_ReturnUserTimeout::registNextTimer(int,int,unsigned int)
int Timer_ReturnUserTimeout::registNextTimer(int user_uid,
                                             int unique_id,
                                             unsigned int expire_time) {
    const int delay_seconds =
        static_cast<int>(expire_time) - GlobalData::s_systemTime_.getCurSec();
    return G_TimerQueue()->InsertTimer(
        2, user_uid, 149, delay_seconds, static_cast<unsigned int>(unique_id),
        0);
}
