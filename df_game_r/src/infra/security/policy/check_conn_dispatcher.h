#ifndef OLDSRV_SECURITY_CHECK_CONN_DISPATCHER_H_
#define OLDSRV_SECURITY_CHECK_CONN_DISPATCHER_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD 0 / CHECK_CONNECTION.  This remains an unregistered handler until the
// complete 606-slot PacketDispatcher transaction and TimerCheckConn reply path
// are both real; the class itself is nevertheless the exact old typed
// IDispatcher provider for slot 0.
class DisPatcher_CheckConn
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85959C4
    // IDA_NAME: _ZN20DisPatcher_CheckConnC2Ev
    // DEMANGLED: DisPatcher_CheckConn::DisPatcher_CheckConn(void)
    DisPatcher_CheckConn();

    // IDA_EA: 0x82438FA
    // IDA_NAME: _ZN20DisPatcher_CheckConnD2Ev
    // DEMANGLED: DisPatcher_CheckConn::~DisPatcher_CheckConn()
    // IDA_EA: 0x824392A
    // IDA_NAME: _ZN20DisPatcher_CheckConnD0Ev
    // DEMANGLED: DisPatcher_CheckConn::~DisPatcher_CheckConn()
    virtual ~DisPatcher_CheckConn();

    // IDA_EA: 0x81EA900
    // IDA_NAME: _ZN20DisPatcher_CheckConn12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: DisPatcher_CheckConn::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8232A7E
    // IDA_NAME: _ZN20DisPatcher_CheckConn4readER9PacketBufR8MSG_BASE
    // DEMANGLED: DisPatcher_CheckConn::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8232A88
    // IDA_NAME: _ZN20DisPatcher_CheckConn7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: DisPatcher_CheckConn::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#endif  // OLDSRV_SECURITY_CHECK_CONN_DISPATCHER_H_
