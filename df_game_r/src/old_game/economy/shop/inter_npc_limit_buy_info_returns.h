#ifndef OLDSRV_OLD_GAME_NPC_INTER_NPC_LIMIT_BUY_INFO_RETURNS_H_
#define OLDSRV_OLD_GAME_NPC_INTER_NPC_LIMIT_BUY_INFO_RETURNS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

class Inter_NpcLimitBuyItemInfoAll : public ISigDispatcher {
public:
    Inter_NpcLimitBuyItemInfoAll();
    virtual ~Inter_NpcLimitBuyItemInfoAll();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_NpcLimitBuyItemChangeInfo : public ISigDispatcher {
public:
    Inter_NpcLimitBuyItemChangeInfo();
    virtual ~Inter_NpcLimitBuyItemChangeInfo();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_NpcLimitBuyItemInfoRenew : public ISigDispatcher {
public:
    Inter_NpcLimitBuyItemInfoRenew();
    virtual ~Inter_NpcLimitBuyItemInfoRenew();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_NpcLimitBuyItemInfoAll) == 0x04u,
              "Inter_NpcLimitBuyItemInfoAll must remain base-only");
static_assert(sizeof(Inter_NpcLimitBuyItemChangeInfo) == 0x04u,
              "Inter_NpcLimitBuyItemChangeInfo must remain base-only");
static_assert(sizeof(Inter_NpcLimitBuyItemInfoRenew) == 0x04u,
              "Inter_NpcLimitBuyItemInfoRenew must remain base-only");
#endif

#endif  // OLDSRV_OLD_GAME_NPC_INTER_NPC_LIMIT_BUY_INFO_RETURNS_H_
