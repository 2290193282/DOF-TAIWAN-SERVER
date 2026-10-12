#ifndef OLDSRV_OLD_GAME_USERINFO_INTER_GOLD_CONTROL_DISPATCHER_H_
#define OLDSRV_OLD_GAME_USERINFO_INTER_GOLD_CONTROL_DISPATCHER_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

class Inter_GoldControlMoneyActionCancel : public ISigDispatcher {
public:
    Inter_GoldControlMoneyActionCancel();
    virtual ~Inter_GoldControlMoneyActionCancel();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_GoldControlMoneyActionCancel) == 0x04u,
              "Inter_GoldControlMoneyActionCancel must remain base-only");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_INTER_GOLD_CONTROL_DISPATCHER_H_
