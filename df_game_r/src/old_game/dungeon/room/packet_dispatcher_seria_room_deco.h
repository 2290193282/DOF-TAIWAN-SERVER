#ifndef OLDSRV_09_DUNGEON_PVP_PARTY_PACKET_DISPATCHER_SERIA_ROOM_DECO_H_
#define OLDSRV_09_DUNGEON_PVP_PARTY_PACKET_DISPATCHER_SERIA_ROOM_DECO_H_

#include <stddef.h>
#include <stdint.h>

#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 09_dungeon_pvp_party/MSG_SERIAROOM_DECO_EVENT.
// CMD495 (0x81E5C88) and CMD513 (0x81E8814) both write through
// `reinterpret_cast<uint16_t*>(message) + 7`, i.e. +0x0e.  The preceding
// byte is part of the raw request extent, not the first inventory-slot byte.
#pragma pack(push, 1)
struct MSG_SERIAROOM_DECO_EVENT : public MSG_BASE {
    uint8_t reserved_0d;      // +0x0d
    uint16_t inventory_slot;  // +0x0e
};
#pragma pack(pop)

static_assert(sizeof(MSG_SERIAROOM_DECO_EVENT) == 0x10u,
              "Seria-room decoration request must retain its 16-byte extent");
static_assert(offsetof(MSG_SERIAROOM_DECO_EVENT, inventory_slot) == 0x0eu,
              "Seria-room decoration inventory slot must remain at +0x0e");

// A owns this generic template family.  Declare every original specialization
// before emitting the C-owned derived vtable below.
template <>
IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                  stSeriaRoomDecoEvent,
                  static_cast<ch_state>(0)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                  stSeriaRoomDecoEvent,
                  static_cast<ch_state>(0)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                      stSeriaRoomDecoEvent,
                      static_cast<ch_state>(0)>::dispatch_template(CUser*,
                                                                      PacketBuf&);
template <>
int IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                      stSeriaRoomDecoEvent,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                 PacketBuf&);
template <>
int IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                      stSeriaRoomDecoEvent,
                      static_cast<ch_state>(0)>::check_state(CUser*,
                                                                PacketBuf&);
template <>
void IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                       stSeriaRoomDecoEvent,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&);

// CMD513 has a distinct state-3 template family. It must not bind to the
// state-0 CMD495 methods merely because the two dispatchers share payload
// shapes.
template <>
IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                  stSeriaRoomDecoEvent,
                  static_cast<ch_state>(3)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                  stSeriaRoomDecoEvent,
                  static_cast<ch_state>(3)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                      stSeriaRoomDecoEvent,
                      static_cast<ch_state>(3)>::dispatch_template(CUser*,
                                                                      PacketBuf&);
template <>
int IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                      stSeriaRoomDecoEvent,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                 PacketBuf&);
template <>
int IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                      stSeriaRoomDecoEvent,
                      static_cast<ch_state>(3)>::check_state(CUser*,
                                                                PacketBuf&);
template <>
void IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                       stSeriaRoomDecoEvent,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&);

// IDA unit: 09_dungeon_pvp_party/Dispatcher_SeriaRoom_Deco_Event.
// IDA_EA: read/process/check/send @0x81E5C88/0x81E5CDE/0x81E5E50/0x81E5E86;
//         D2/D0/C2 @0x8243CF0/0x8243D20/0x8597DAA.
class Dispatcher_SeriaRoom_Deco_Event
    : public IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                               stSeriaRoomDecoEvent,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_SeriaRoom_Deco_Event();
    virtual ~Dispatcher_SeriaRoom_Deco_Event();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

    // IDA name retains MSG_BASE& even though 0x81E5E50 never reads it.
    // Preserve that ABI argument so the concrete provider has the raw symbol.
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SeriaRoom_Deco_Event) == 0x04u,
              "CMD495 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/Dispatcher_SeriaRoom_Ani_Deco_Event.
// CMD513 shares CMD495's request/result records, but its state-3 template
// drives the animation-decoration event closure.
class Dispatcher_SeriaRoom_Ani_Deco_Event
    : public IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                               stSeriaRoomDecoEvent,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x8597EDE
    Dispatcher_SeriaRoom_Ani_Deco_Event();
    // IDA_EA: 0x8243A32 / 0x8243A62
    virtual ~Dispatcher_SeriaRoom_Ani_Deco_Event();

    // IDA_EA: 0x81E8814 / 0x81E886A / 0x81E8B64
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81E8B20
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SeriaRoom_Ani_Deco_Event) == 0x04u,
              "CMD513 dispatcher must retain the old vptr-only ABI");
#endif

#endif  // OLDSRV_09_DUNGEON_PVP_PARTY_PACKET_DISPATCHER_SERIA_ROOM_DECO_H_
