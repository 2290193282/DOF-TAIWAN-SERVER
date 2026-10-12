#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_GATHERING_PARTY_STATUS_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_GATHERING_PARTY_STATUS_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 09_dungeon_pvp_party/Dispatcher_GatheringPartyStatus.
// Its common state-0 MSG_BASE/ParamBase generic is owned only by
// src/protocol/packet_dispatcher_msg_base.cpp.
class Dispatcher_GatheringPartyStatus
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595F74
    // IDA_NAME: _ZN31Dispatcher_GatheringPartyStatusC2Ev
    // DEMANGLED: Dispatcher_GatheringPartyStatus::Dispatcher_GatheringPartyStatus(void)
    Dispatcher_GatheringPartyStatus();

    // IDA_EA: 0x8242D66 / 0x8242D96
    // IDA_NAME: _ZN31Dispatcher_GatheringPartyStatusD2Ev / D0Ev
    // DEMANGLED: Dispatcher_GatheringPartyStatus::~Dispatcher_GatheringPartyStatus()
    virtual ~Dispatcher_GatheringPartyStatus();

    // IDA_EA: 0x8203878
    // IDA_NAME: _ZN31Dispatcher_GatheringPartyStatus12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_GatheringPartyStatus::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233724
    // IDA_NAME: _ZN31Dispatcher_GatheringPartyStatus4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_GatheringPartyStatus::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x823372E
    // IDA_NAME: _ZN31Dispatcher_GatheringPartyStatus7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_GatheringPartyStatus::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_GatheringPartyStatus) == 0x04u,
              "CMD108 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_GATHERING_PARTY_STATUS_H_
