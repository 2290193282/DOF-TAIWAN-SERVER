#ifndef OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_DISGUISE_CANCEL_H_
#define OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_DISGUISE_CANCEL_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA unit: 13_database_dispatch/Dispatcher_DisguiseCancel.
// The class carries no state beyond the state-0 IPacketDispatcher base.
class Dispatcher_DisguiseCancel
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85969BC
    // IDA_NAME: _ZN25Dispatcher_DisguiseCancelC2Ev
    // DEMANGLED: Dispatcher_DisguiseCancel::Dispatcher_DisguiseCancel(void)
    Dispatcher_DisguiseCancel();

    // IDA_EA: 0x824163E
    // IDA_NAME: _ZN25Dispatcher_DisguiseCancelD2Ev
    // DEMANGLED: Dispatcher_DisguiseCancel::~Dispatcher_DisguiseCancel()
    // IDA_EA: 0x824166E
    // IDA_NAME: _ZN25Dispatcher_DisguiseCancelD0Ev
    // DEMANGLED: Dispatcher_DisguiseCancel::~Dispatcher_DisguiseCancel()
    virtual ~Dispatcher_DisguiseCancel();

    // IDA_EA: 0x821B496
    // IDA_NAME: _ZN25Dispatcher_DisguiseCancel12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_DisguiseCancel::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233D9E
    // IDA_NAME: _ZN25Dispatcher_DisguiseCancel4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_DisguiseCancel::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233DA8
    // IDA_NAME: _ZN25Dispatcher_DisguiseCancel7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_DisguiseCancel::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_DisguiseCancel) == 0x04u,
              "CMD215 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_DISGUISE_CANCEL_H_
