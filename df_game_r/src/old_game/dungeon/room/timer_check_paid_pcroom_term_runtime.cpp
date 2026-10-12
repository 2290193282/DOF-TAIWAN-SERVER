#include "old_game/dungeon/room/timer_check_paid_pcroom_term_runtime.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/premium_runtime.h"

// IDA_EA: 0x863C362
// IDA_NAME: _ZN24TimerCheckPaidPCRoomTermC2Ev
// DEMANGLED: TimerCheckPaidPCRoomTerm::TimerCheckPaidPCRoomTerm(void)
TimerCheckPaidPCRoomTerm::TimerCheckPaidPCRoomTerm() : TimerSig() {}

// IDA_EA: 0x8640926
// IDA_NAME: _ZN24TimerCheckPaidPCRoomTermD2Ev
// IDA_EA: 0x8640956
// IDA_NAME: _ZN24TimerCheckPaidPCRoomTermD0Ev
// DEMANGLED: TimerCheckPaidPCRoomTerm::~TimerCheckPaidPCRoomTerm()
TimerCheckPaidPCRoomTerm::~TimerCheckPaidPCRoomTerm() {}

// IDA_EA: 0x8634B60
// IDA_NAME: _ZN24TimerCheckPaidPCRoomTerm12dispatch_sigEiij
// DEMANGLED: TimerCheckPaidPCRoomTerm::dispatch_sig(int,int,unsigned int)
int TimerCheckPaidPCRoomTerm::dispatch_sig(int user_uid,
                                           int premium_type,
                                           unsigned int account_id) {
    CUser* const user = G_CGameManager()->getUser(user_uid, 6);
    if (!user || static_cast<unsigned int>(user->get_acc_id()) != account_id) {
        return 0;
    }

    if (!user->isAffectedPremium(
            static_cast<ENUM_PREMIUM_TYPE>(premium_type))) {
        WongWork::CHandlePremium::handleNotifyPremiumInfo(user,
                                                          premium_type,
                                                          0);
        return 0;
    }

    user->SendLogoutToPCRoomServer();
    user->SendLoginToPCRoomServer();
    return 1;
}
