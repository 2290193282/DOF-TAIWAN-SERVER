#ifndef OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_GOBLIN_PAD_STATUS_H_
#define OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_GOBLIN_PAD_STATUS_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA unit: 13_database_dispatch/Dispatcher_GoblinPadStatus.
// The class carries no state beyond the state-0 IPacketDispatcher base.
class Dispatcher_GoblinPadStatus
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596850
    // IDA_NAME: _ZN26Dispatcher_GoblinPadStatusC2Ev
    // DEMANGLED: Dispatcher_GoblinPadStatus::Dispatcher_GoblinPadStatus(void)
    Dispatcher_GoblinPadStatus();

    // IDA_EA: 0x8241998
    // IDA_NAME: _ZN26Dispatcher_GoblinPadStatusD2Ev
    // DEMANGLED: Dispatcher_GoblinPadStatus::~Dispatcher_GoblinPadStatus()
    // IDA_EA: 0x82419C8
    // IDA_NAME: _ZN26Dispatcher_GoblinPadStatusD0Ev
    // DEMANGLED: Dispatcher_GoblinPadStatus::~Dispatcher_GoblinPadStatus()
    virtual ~Dispatcher_GoblinPadStatus();

    // IDA_EA: 0x82177AE
    // IDA_NAME: _ZN26Dispatcher_GoblinPadStatus12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_GoblinPadStatus::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233CD6
    // IDA_NAME: _ZN26Dispatcher_GoblinPadStatus4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_GoblinPadStatus::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233CE0
    // IDA_NAME: _ZN26Dispatcher_GoblinPadStatus7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_GoblinPadStatus::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_GoblinPadStatus) == 0x04u,
              "CMD196 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_GOBLIN_PAD_STATUS_H_
