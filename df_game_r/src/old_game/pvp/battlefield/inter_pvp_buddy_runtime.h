#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_INTER_PVP_BUDDY_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_INTER_PVP_BUDDY_RUNTIME_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

// IDA units: 09_dungeon_pvp_party/Inter_{LoadPvPBuddy,PvPBuddyConnList,
// ReqAddPvPBuddy,AddPvPBuddyResult,RemovePvPBuddyResult,NoticePvPBuddyInOut}.
// InterDispatcher allocates each as a vptr-only ISigDispatcher on i386.
class Inter_LoadPvPBuddy : public ISigDispatcher {
public:
    Inter_LoadPvPBuddy();
    virtual ~Inter_LoadPvPBuddy();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_PvPBuddyConnList : public ISigDispatcher {
public:
    Inter_PvPBuddyConnList();
    virtual ~Inter_PvPBuddyConnList();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_ReqAddPvPBuddy : public ISigDispatcher {
public:
    Inter_ReqAddPvPBuddy();
    virtual ~Inter_ReqAddPvPBuddy();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_AddPvPBuddyResult : public ISigDispatcher {
public:
    Inter_AddPvPBuddyResult();
    virtual ~Inter_AddPvPBuddyResult();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_RemovePvPBuddyResult : public ISigDispatcher {
public:
    Inter_RemovePvPBuddyResult();
    virtual ~Inter_RemovePvPBuddyResult();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_NoticePvPBuddyInOut : public ISigDispatcher {
public:
    Inter_NoticePvPBuddyInOut();
    virtual ~Inter_NoticePvPBuddyInOut();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_LoadPvPBuddy) == 0x04u,
              "Inter_LoadPvPBuddy must remain vptr-only");
static_assert(sizeof(Inter_PvPBuddyConnList) == 0x04u,
              "Inter_PvPBuddyConnList must remain vptr-only");
static_assert(sizeof(Inter_ReqAddPvPBuddy) == 0x04u,
              "Inter_ReqAddPvPBuddy must remain vptr-only");
static_assert(sizeof(Inter_AddPvPBuddyResult) == 0x04u,
              "Inter_AddPvPBuddyResult must remain vptr-only");
static_assert(sizeof(Inter_RemovePvPBuddyResult) == 0x04u,
              "Inter_RemovePvPBuddyResult must remain vptr-only");
static_assert(sizeof(Inter_NoticePvPBuddyInOut) == 0x04u,
              "Inter_NoticePvPBuddyInOut must remain vptr-only");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_INTER_PVP_BUDDY_RUNTIME_H_
