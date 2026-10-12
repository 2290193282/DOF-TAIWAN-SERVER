#ifndef OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_REQUEST_PC_ROOM_PLAYER_COUNT_H_
#define OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_REQUEST_PC_ROOM_PLAYER_COUNT_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA unit: 08_quest_skill_growth/Dispatcher_RequestPCRoomPlayerCount.
// The class carries no state beyond the state-0 IPacketDispatcher base.
class Dispatcher_RequestPCRoomPlayerCount
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596A2C
    // IDA_NAME: _ZN35Dispatcher_RequestPCRoomPlayerCountC2Ev
    // DEMANGLED: Dispatcher_RequestPCRoomPlayerCount::Dispatcher_RequestPCRoomPlayerCount(void)
    Dispatcher_RequestPCRoomPlayerCount();

    // IDA_EA: 0x82414B8
    // IDA_NAME: _ZN35Dispatcher_RequestPCRoomPlayerCountD2Ev
    // DEMANGLED: Dispatcher_RequestPCRoomPlayerCount::~Dispatcher_RequestPCRoomPlayerCount()
    // IDA_EA: 0x82414E8
    // IDA_NAME: _ZN35Dispatcher_RequestPCRoomPlayerCountD0Ev
    // DEMANGLED: Dispatcher_RequestPCRoomPlayerCount::~Dispatcher_RequestPCRoomPlayerCount()
    virtual ~Dispatcher_RequestPCRoomPlayerCount();

    // IDA_EA: 0x821D8D0
    // IDA_NAME: _ZN35Dispatcher_RequestPCRoomPlayerCount12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_RequestPCRoomPlayerCount::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233E02
    // IDA_NAME: _ZN35Dispatcher_RequestPCRoomPlayerCount4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_RequestPCRoomPlayerCount::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233E0C
    // IDA_NAME: _ZN35Dispatcher_RequestPCRoomPlayerCount7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_RequestPCRoomPlayerCount::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_RequestPCRoomPlayerCount) == 0x04u,
              "CMD217 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_REQUEST_PC_ROOM_PLAYER_COUNT_H_
