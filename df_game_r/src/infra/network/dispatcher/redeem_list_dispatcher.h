#ifndef OLDSRV_PROTOCOL_REDEEM_LIST_DISPATCHER_H_
#define OLDSRV_PROTOCOL_REDEEM_LIST_DISPATCHER_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD 310 / REDEEM_LIST. The original registry remains inactive until the
// complete PacketDispatcher constructor transaction is restored.
class Dispatcher_RedeemList
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x859729E
    // IDA_NAME: _ZN21Dispatcher_RedeemListC2Ev
    // DEMANGLED: Dispatcher_RedeemList::Dispatcher_RedeemList(void)
    Dispatcher_RedeemList();

    // IDA_EA: 0x824052E
    // IDA_NAME: _ZN21Dispatcher_RedeemListD2Ev
    // DEMANGLED: Dispatcher_RedeemList::~Dispatcher_RedeemList()
    // IDA_EA: 0x824055E
    // IDA_NAME: _ZN21Dispatcher_RedeemListD0Ev
    // DEMANGLED: Dispatcher_RedeemList::~Dispatcher_RedeemList()
    virtual ~Dispatcher_RedeemList();

    // IDA_EA: 0x822621C
    // IDA_NAME: _ZN21Dispatcher_RedeemList12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_RedeemList::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x82341FE
    // IDA_NAME: _ZN21Dispatcher_RedeemList4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_RedeemList::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8234208
    // IDA_NAME: _ZN21Dispatcher_RedeemList7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_RedeemList::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_RedeemList) == 0x04u,
              "CMD310 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_PROTOCOL_REDEEM_LIST_DISPATCHER_H_
