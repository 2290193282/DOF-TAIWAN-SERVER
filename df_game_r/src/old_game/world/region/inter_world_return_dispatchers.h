#ifndef OLDSRV_OLD_GAME_WORLD_INTER_WORLD_RETURN_DISPATCHERS_H_
#define OLDSRV_OLD_GAME_WORLD_INTER_WORLD_RETURN_DISPATCHERS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

class Inter_NoCache : public ISigDispatcher {
public:
    Inter_NoCache();
    virtual ~Inter_NoCache();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_BroadcastMsg : public ISigDispatcher {
public:
    Inter_BroadcastMsg();
    virtual ~Inter_BroadcastMsg();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_CollectItemsResult : public ISigDispatcher {
public:
    Inter_CollectItemsResult();
    virtual ~Inter_CollectItemsResult();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_CollectItemsRewardBroadcast : public ISigDispatcher {
public:
    Inter_CollectItemsRewardBroadcast();
    virtual ~Inter_CollectItemsRewardBroadcast();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_NoCache) == 0x04u,
              "Inter_NoCache must remain base-only");
static_assert(sizeof(Inter_BroadcastMsg) == 0x04u,
              "Inter_BroadcastMsg must remain base-only");
static_assert(sizeof(Inter_CollectItemsResult) == 0x04u,
              "Inter_CollectItemsResult must remain base-only");
static_assert(sizeof(Inter_CollectItemsRewardBroadcast) == 0x04u,
              "Inter_CollectItemsRewardBroadcast must remain base-only");
#endif

#endif  // OLDSRV_OLD_GAME_WORLD_INTER_WORLD_RETURN_DISPATCHERS_H_
