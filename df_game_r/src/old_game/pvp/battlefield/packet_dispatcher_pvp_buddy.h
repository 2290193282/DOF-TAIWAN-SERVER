#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_PVP_BUDDY_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_PVP_BUDDY_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class Dispatcher_ResponseAddPvPBuddy
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_ResponseAddPvPBuddy();
    virtual ~Dispatcher_ResponseAddPvPBuddy();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_RemovePvPBuddy
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_RemovePvPBuddy();
    virtual ~Dispatcher_RemovePvPBuddy();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_PvPBuddyConnList
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_PvPBuddyConnList();
    virtual ~Dispatcher_PvPBuddyConnList();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ResponseAddPvPBuddy) == 0x04u,
              "CMD290 must remain vptr-only");
static_assert(sizeof(Dispatcher_RemovePvPBuddy) == 0x04u,
              "CMD291 must remain vptr-only");
static_assert(sizeof(Dispatcher_PvPBuddyConnList) == 0x04u,
              "CMD292 must remain vptr-only");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_PVP_BUDDY_H_
