#include "old_game/economy/mail/timer_check_mail_timeout.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"


// IDA_EA: 0x863C6E2
// IDA_NAME: _ZN21TimerCheckMailTimeOutC2Ev
// DEMANGLED: TimerCheckMailTimeOut::TimerCheckMailTimeOut(void)
TimerCheckMailTimeOut::TimerCheckMailTimeOut()
    : TimerSig() {}

// IDA_EA: 0x8640050
// IDA_NAME: _ZN21TimerCheckMailTimeOutD2Ev
// DEMANGLED: TimerCheckMailTimeOut::~TimerCheckMailTimeOut()
// IDA_EA: 0x8640080
// IDA_NAME: _ZN21TimerCheckMailTimeOutD0Ev
// DEMANGLED: TimerCheckMailTimeOut::~TimerCheckMailTimeOut()
TimerCheckMailTimeOut::~TimerCheckMailTimeOut() {}

// IDA_EA: 0x8635D20
// IDA_NAME: _ZN21TimerCheckMailTimeOut12dispatch_sigEiij
// DEMANGLED: TimerCheckMailTimeOut::dispatch_sig(int,int,unsigned int)
int TimerCheckMailTimeOut::dispatch_sig(int user_uid,
                                        int login_tick,
                                        unsigned int charac_no) {
    CUser* const user = G_CGameManager()->getUser(user_uid, 0);
    if (!user) {
        return 0;
    }
    if (user->get_state() <= 1) {
        return 0;
    }
    if (user->GetLoginTick() != login_tick) {
        return 0;
    }
    if (static_cast<unsigned int>(user->getCurCharacNo()) != charac_no) {
        return 0;
    }

    PacketGuard packet;
    packet.put_header(0, 98);
    int result_index = packet.get_index();
    packet.put_int(0);
    const int mail_count =
        WongWork::CMailBoxHelper::CheckMailTimeout(user, &packet);
    if (mail_count) {
        packet.put_int(result_index, mail_count);
        packet.finalize(true);
        user->Send(packet);
    }
    WongWork::CMailBoxHelper::SetPeriodCheckTimer(user);
    return 1;
}
