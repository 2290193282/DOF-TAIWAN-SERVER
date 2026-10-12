#include "infra/concurrent/timer/timer_pcroom_punish_runtime.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/player/core/cuser.h"
#include "infra/concurrent/timer/timer_queue.h"

// IDA_EA: 0x863CF1A
// IDA_NAME: _ZN21Timer_PcRoomAuthRetryC2Ev
// DEMANGLED: Timer_PcRoomAuthRetry::Timer_PcRoomAuthRetry(void)
Timer_PcRoomAuthRetry::Timer_PcRoomAuthRetry() : TimerSig() {}

// IDA_EA: 0x863EAA2
// IDA_NAME: _ZN21Timer_PcRoomAuthRetryD2Ev
// IDA_EA: 0x863EAD2
// IDA_NAME: _ZN21Timer_PcRoomAuthRetryD0Ev
// DEMANGLED: Timer_PcRoomAuthRetry::~Timer_PcRoomAuthRetry()
Timer_PcRoomAuthRetry::~Timer_PcRoomAuthRetry() {}

// IDA_EA: 0x8639924
// IDA_NAME: _ZN21Timer_PcRoomAuthRetry12dispatch_sigEiij
// DEMANGLED: Timer_PcRoomAuthRetry::dispatch_sig(int,int,unsigned int)
int Timer_PcRoomAuthRetry::dispatch_sig(int user_uid, int unique_id,
                                        unsigned int charac_no) {
    CUser* const user = G_CGameManager()->getUser(user_uid, 2);
    if (!user) {
        return 0;
    }
    if (static_cast<unsigned short>(user->get_unique_id()) != unique_id) {
        return 0;
    }
    if (!user->getCurCharacR()) {
        return 0;
    }
    if (static_cast<unsigned int>(user->getCurCharacNo()) != charac_no) {
        return 0;
    }
    if (static_cast<unsigned char>(user->getPcRoomAuth()) != 1u) {
        user->SendLoginToPCRoomServer();
    }
    return 1;
}

// IDA_EA: 0x86399D0
// IDA_NAME: _ZN21Timer_PcRoomAuthRetry15registNextTimerEliij
// DEMANGLED: Timer_PcRoomAuthRetry::registNextTimer(long,int,int,unsigned int)
int Timer_PcRoomAuthRetry::registNextTimer(long delay_seconds,
                                            int user_uid,
                                            int unique_id,
                                            unsigned int charac_no) {
    return G_TimerQueue()->InsertTimer(
        2, user_uid, 146, static_cast<int>(delay_seconds), unique_id,
        static_cast<int>(charac_no));
}

// IDA_EA: 0x863CCEA
// IDA_NAME: _ZN18TimerSetPunishUserC2Ev
// DEMANGLED: TimerSetPunishUser::TimerSetPunishUser(void)
TimerSetPunishUser::TimerSetPunishUser() : TimerSig() {}

// IDA_EA: 0x863F108
// IDA_NAME: _ZN18TimerSetPunishUserD2Ev
// IDA_EA: 0x863F138
// IDA_NAME: _ZN18TimerSetPunishUserD0Ev
// DEMANGLED: TimerSetPunishUser::~TimerSetPunishUser()
TimerSetPunishUser::~TimerSetPunishUser() {}

// IDA_EA: 0x8638BA0
// IDA_NAME: _ZN18TimerSetPunishUser12dispatch_sigEiij
// DEMANGLED: TimerSetPunishUser::dispatch_sig(int,int,unsigned int)
int TimerSetPunishUser::dispatch_sig(int user_uid, int ignored_param,
                                     unsigned int unique_id) {
    (void)ignored_param;

    CUser* const user = G_CGameManager()->getUser(user_uid, 2);
    if (!user) {
        return 0;
    }
    if (static_cast<unsigned short>(user->get_unique_id()) != unique_id) {
        return 0;
    }

    user->setHackUserFlag();
    user->SetTradePunishType(64);
    if (static_cast<unsigned char>(user->isARSUserKick()) != 0u) {
        user->DisConnSig(static_cast<DISCONN_SIG>(46), true, 0);
    }
    return 1;
}

// IDA_EA: 0x863CD06
// IDA_NAME: _ZN23TimerSetTradePunishUserC2Ev
// DEMANGLED: TimerSetTradePunishUser::TimerSetTradePunishUser(void)
TimerSetTradePunishUser::TimerSetTradePunishUser() : TimerSig() {}

// IDA_EA: 0x863F0BA
// IDA_NAME: _ZN23TimerSetTradePunishUserD2Ev
// IDA_EA: 0x863F0EA
// IDA_NAME: _ZN23TimerSetTradePunishUserD0Ev
// DEMANGLED: TimerSetTradePunishUser::~TimerSetTradePunishUser()
TimerSetTradePunishUser::~TimerSetTradePunishUser() {}

// IDA_EA: 0x8638C48
// IDA_NAME: _ZN23TimerSetTradePunishUser12dispatch_sigEiij
// DEMANGLED: TimerSetTradePunishUser::dispatch_sig(int,int,unsigned int)
int TimerSetTradePunishUser::dispatch_sig(int user_uid, int ignored_param,
                                          unsigned int unique_id) {
    (void)ignored_param;

    CUser* const user = G_CGameManager()->getUser(user_uid, 2);
    if (!user) {
        return 0;
    }
    if (static_cast<unsigned short>(user->get_unique_id()) != unique_id) {
        return 0;
    }

    user->SetTradePunishType(4);
    return 1;
}
