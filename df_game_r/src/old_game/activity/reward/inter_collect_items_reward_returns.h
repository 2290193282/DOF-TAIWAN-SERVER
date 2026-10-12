#ifndef OLDSRV_OLD_GAME_EVENT_INTER_COLLECT_ITEMS_REWARD_RETURNS_H_
#define OLDSRV_OLD_GAME_EVENT_INTER_COLLECT_ITEMS_REWARD_RETURNS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

class Inter_CollectItemsReward : public ISigDispatcher {
public:
    Inter_CollectItemsReward();
    virtual ~Inter_CollectItemsReward();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_CollectItemsReward) == 0x04u,
              "Inter_CollectItemsReward must remain base-only");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_INTER_COLLECT_ITEMS_REWARD_RETURNS_H_
