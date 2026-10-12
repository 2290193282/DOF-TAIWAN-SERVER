#include "old_game/player/core/inter_pad_reward_dispatcher.h"

#include "old_game/player/core/cuser.h"

// IDA_EA: 0x84E4354
// IDA_NAME: _ZN29Inter_GoblinPadChagePwdReward12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_GoblinPadChagePwdReward::dispatch_sig(CUser *,char *,int)
int Inter_GoblinPadChagePwdReward::dispatch_sig(CUser* user, char*, int) {
    if (user) {
        user->getPad()->SendChagePwdReward(user);
    }
    return 0;
}

// IDA_EA: 0x84EEAE8 / 0x84F64AA / 0x84F64DA
Inter_GoblinPadChagePwdReward::Inter_GoblinPadChagePwdReward()
    : ISigDispatcher() {}
Inter_GoblinPadChagePwdReward::~Inter_GoblinPadChagePwdReward() {}
