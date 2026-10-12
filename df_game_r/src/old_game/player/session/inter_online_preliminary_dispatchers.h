#ifndef OLDSRV_OLD_GAME_USERINFO_INTER_ONLINE_PRELIMINARY_DISPATCHERS_H_
#define OLDSRV_OLD_GAME_USERINFO_INTER_ONLINE_PRELIMINARY_DISPATCHERS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// InterDispatcher consumers for the two persisted CData slots of the online
// preliminary system.  The payload schemas live with their concrete CData
// implementations; these leaves only preserve the original state gate and
// virtual loadData hand-off.
class Inter_LoadOnlinePreliminaryMember : public ISigDispatcher {
public:
    Inter_LoadOnlinePreliminaryMember();
    virtual ~Inter_LoadOnlinePreliminaryMember();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_LoadOnlinePreliminaryMemberMatchList : public ISigDispatcher {
public:
    Inter_LoadOnlinePreliminaryMemberMatchList();
    virtual ~Inter_LoadOnlinePreliminaryMemberMatchList();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_LoadOnlinePreliminaryTeam : public ISigDispatcher {
public:
    Inter_LoadOnlinePreliminaryTeam();
    virtual ~Inter_LoadOnlinePreliminaryTeam();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_LoadOnlinePreliminaryTeamMatchList : public ISigDispatcher {
public:
    Inter_LoadOnlinePreliminaryTeamMatchList();
    virtual ~Inter_LoadOnlinePreliminaryTeamMatchList();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_LoadOnlinePreliminaryMember) == 0x04u,
              "Inter_LoadOnlinePreliminaryMember must remain base-only");
static_assert(sizeof(Inter_LoadOnlinePreliminaryMemberMatchList) == 0x04u,
              "Inter_LoadOnlinePreliminaryMemberMatchList must remain base-only");
static_assert(sizeof(Inter_LoadOnlinePreliminaryTeam) == 0x04u,
              "Inter_LoadOnlinePreliminaryTeam must remain base-only");
static_assert(sizeof(Inter_LoadOnlinePreliminaryTeamMatchList) == 0x04u,
              "Inter_LoadOnlinePreliminaryTeamMatchList must remain base-only");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_INTER_ONLINE_PRELIMINARY_DISPATCHERS_H_
