#ifndef OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_REQUEST_PC_ROOM_PLAYER_LIST_H_
#define OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_REQUEST_PC_ROOM_PLAYER_LIST_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA unit: 08_quest_skill_growth/Dispatcher_RequestPCRoomPlayerList.
// The class carries no state beyond the state-0 IPacketDispatcher base.
class Dispatcher_RequestPCRoomPlayerList
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596A10
    // IDA_NAME: _ZN34Dispatcher_RequestPCRoomPlayerListC2Ev
    // DEMANGLED: Dispatcher_RequestPCRoomPlayerList::Dispatcher_RequestPCRoomPlayerList(void)
    Dispatcher_RequestPCRoomPlayerList();

    // IDA_EA: 0x8241506
    // IDA_NAME: _ZN34Dispatcher_RequestPCRoomPlayerListD2Ev
    // DEMANGLED: Dispatcher_RequestPCRoomPlayerList::~Dispatcher_RequestPCRoomPlayerList()
    // IDA_EA: 0x8241536
    // IDA_NAME: _ZN34Dispatcher_RequestPCRoomPlayerListD0Ev
    // DEMANGLED: Dispatcher_RequestPCRoomPlayerList::~Dispatcher_RequestPCRoomPlayerList()
    virtual ~Dispatcher_RequestPCRoomPlayerList();

    // IDA_EA: 0x821D7FA
    // IDA_NAME: _ZN34Dispatcher_RequestPCRoomPlayerList12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_RequestPCRoomPlayerList::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233DEE
    // IDA_NAME: _ZN34Dispatcher_RequestPCRoomPlayerList4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_RequestPCRoomPlayerList::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233DF8
    // IDA_NAME: _ZN34Dispatcher_RequestPCRoomPlayerList7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_RequestPCRoomPlayerList::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_RequestPCRoomPlayerList) == 0x04u,
              "CMD216 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_REQUEST_PC_ROOM_PLAYER_LIST_H_
