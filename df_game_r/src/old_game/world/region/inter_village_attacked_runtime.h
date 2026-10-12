#ifndef OLDSRV_OLD_GAME_WORLD_INTER_VILLAGE_ATTACKED_RUNTIME_H_
#define OLDSRV_OLD_GAME_WORLD_INTER_VILLAGE_ATTACKED_RUNTIME_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// DB InterDispatcher providers for the village-attacked event.  These
// classes are deliberately vptr-only: InterDispatcher constructs each with
// `new 4` in the original i386 binary.
class Inter_TakeScreenShot : public ISigDispatcher {
public:
    Inter_TakeScreenShot();
    virtual ~Inter_TakeScreenShot();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_VillageAttackedStart : public ISigDispatcher {
public:
    Inter_VillageAttackedStart();
    virtual ~Inter_VillageAttackedStart();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_VillageAttackedEnd : public ISigDispatcher {
public:
    Inter_VillageAttackedEnd();
    virtual ~Inter_VillageAttackedEnd();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_VillageAttackedCountdown : public ISigDispatcher {
public:
    Inter_VillageAttackedCountdown();
    virtual ~Inter_VillageAttackedCountdown();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_VillageAttackedUpdate : public ISigDispatcher {
public:
    Inter_VillageAttackedUpdate();
    virtual ~Inter_VillageAttackedUpdate();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_VillageAttackedScore : public ISigDispatcher {
public:
    Inter_VillageAttackedScore();
    virtual ~Inter_VillageAttackedScore();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_VillageAttackedReward : public ISigDispatcher {
public:
    Inter_VillageAttackedReward();
    virtual ~Inter_VillageAttackedReward();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

    int _send_reward_item(CUser* user,
                          unsigned long item_index,
                          int item_count,
                          int reward_value);
};

class Inter_VillageAttackedRewardServer : public ISigDispatcher {
public:
    Inter_VillageAttackedRewardServer();
    virtual ~Inter_VillageAttackedRewardServer();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_FullLevelBroadCast : public ISigDispatcher {
public:
    Inter_FullLevelBroadCast();
    virtual ~Inter_FullLevelBroadCast();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_TakeScreenShot) == 0x04u,
              "Inter_TakeScreenShot must stay vptr-only on i386");
static_assert(sizeof(Inter_VillageAttackedStart) == 0x04u,
              "Inter_VillageAttackedStart must stay vptr-only on i386");
static_assert(sizeof(Inter_VillageAttackedEnd) == 0x04u,
              "Inter_VillageAttackedEnd must stay vptr-only on i386");
static_assert(sizeof(Inter_VillageAttackedCountdown) == 0x04u,
              "Inter_VillageAttackedCountdown must stay vptr-only on i386");
static_assert(sizeof(Inter_VillageAttackedUpdate) == 0x04u,
              "Inter_VillageAttackedUpdate must stay vptr-only on i386");
static_assert(sizeof(Inter_VillageAttackedScore) == 0x04u,
              "Inter_VillageAttackedScore must stay vptr-only on i386");
static_assert(sizeof(Inter_VillageAttackedReward) == 0x04u,
              "Inter_VillageAttackedReward must stay vptr-only on i386");
static_assert(sizeof(Inter_VillageAttackedRewardServer) == 0x04u,
              "Inter_VillageAttackedRewardServer must stay vptr-only on i386");
static_assert(sizeof(Inter_FullLevelBroadCast) == 0x04u,
              "Inter_FullLevelBroadCast must stay vptr-only on i386");
#endif

#endif  // OLDSRV_OLD_GAME_WORLD_INTER_VILLAGE_ATTACKED_RUNTIME_H_
