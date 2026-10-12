#ifndef OLDSRV_OLD_GAME_USERINFO_INTER_PAD_REWARD_DISPATCHER_H_
#define OLDSRV_OLD_GAME_USERINFO_INTER_PAD_REWARD_DISPATCHER_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// Slot 581.  The result has no payload: it consumes the CPad's pending
// password-change reward state for the current user.
class Inter_GoblinPadChagePwdReward : public ISigDispatcher {
public:
    Inter_GoblinPadChagePwdReward();
    virtual ~Inter_GoblinPadChagePwdReward();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_GoblinPadChagePwdReward) == 0x04u,
              "Inter_GoblinPadChagePwdReward must remain base-only");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_INTER_PAD_REWARD_DISPATCHER_H_
