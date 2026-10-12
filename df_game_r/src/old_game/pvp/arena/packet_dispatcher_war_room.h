#ifndef OLDSRV_OLD_GAME_PVP_ARENA_PACKET_DISPATCHER_WAR_ROOM_H_
#define OLDSRV_OLD_GAME_PVP_ARENA_PACKET_DISPATCHER_WAR_ROOM_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// Native 140828 WarRoom request owners.  The old registered classes were
// LineFunc-only shells; these dispatchers bind the exact client bodies to the
// already-restored WarRoom state machine.
class Dispatcher_EnterWarRoom
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_EnterWarRoom();
    virtual ~Dispatcher_EnterWarRoom();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_SetWarRoomSeatState
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_SetWarRoomSeatState();
    virtual ~Dispatcher_SetWarRoomSeatState();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_DieWarRoomCharacter
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_DieWarRoomCharacter();
    virtual ~Dispatcher_DieWarRoomCharacter();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_EnterWarRoom) == 0x04u,
              "CMD90 dispatcher must remain vptr-only on i386");
static_assert(sizeof(Dispatcher_SetWarRoomSeatState) == 0x04u,
              "CMD91 dispatcher must remain vptr-only on i386");
static_assert(sizeof(Dispatcher_DieWarRoomCharacter) == 0x04u,
              "CMD92 dispatcher must remain vptr-only on i386");
#endif

#endif
