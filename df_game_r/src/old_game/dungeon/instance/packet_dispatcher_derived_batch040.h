#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_DERIVED_BATCH040_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_DERIVED_BATCH040_H_

// Concrete members of the inactive 606-entry PacketDispatcher root.  Their
// presence here does not authorize slot registration; each class stays
// unreachable until that root transaction is complete.

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "old_game/dungeon/instance/dungeon_pvp_dispatchers.h"
#include "old_game/player/session/packet_dispatcher_return_select_character.h"

class CUser;
class PacketBuf;

class Dispatcher_RequestMatch
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_RequestMatch();
    virtual ~Dispatcher_RequestMatch();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_SelectUltimateDifficulty
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_SelectUltimateDifficulty();
    virtual ~Dispatcher_SelectUltimateDifficulty();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_AskRematch
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_AskRematch();
    virtual ~Dispatcher_AskRematch();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_TutorialLevelUp
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_TutorialLevelUp();
    virtual ~Dispatcher_TutorialLevelUp();
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_RequestMatch) == 0x04u,
              "CMD199 dispatcher must remain vptr-only");
static_assert(sizeof(Dispatcher_SelectUltimateDifficulty) == 0x04u,
              "CMD318 dispatcher must remain vptr-only");
static_assert(sizeof(Dispatcher_AskRematch) == 0x04u,
              "CMD368 dispatcher must remain vptr-only");
static_assert(sizeof(Dispatcher_TutorialLevelUp) == 0x04u,
              "CMD484 dispatcher must remain vptr-only");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_DERIVED_BATCH040_H_
