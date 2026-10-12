#ifndef OLDSRV_SECURITY_ANTIBOT_DPROTO_DISPATCHER_H_
#define OLDSRV_SECURITY_ANTIBOT_DPROTO_DISPATCHER_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// 0828 CMD_DPROTO is registered directly in PacketDispatcher. This class
// preserves the restored concrete IPacketDispatcher ABI and nested-packet
// dispatch path.
class Dispatcher_Antibot_DP
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85965CC
    // IDA_NAME: _ZN21Dispatcher_Antibot_DPC2Ev
    // DEMANGLED: Dispatcher_Antibot_DP::Dispatcher_Antibot_DP(void)
    Dispatcher_Antibot_DP();

    // IDA_EA: 0x8241FB0
    // IDA_NAME: _ZN21Dispatcher_Antibot_DPD2Ev
    // DEMANGLED: Dispatcher_Antibot_DP::~Dispatcher_Antibot_DP()
    // IDA_EA: 0x8241FE0
    // IDA_NAME: _ZN21Dispatcher_Antibot_DPD0Ev
    // DEMANGLED: Dispatcher_Antibot_DP::~Dispatcher_Antibot_DP()
    virtual ~Dispatcher_Antibot_DP();

    // IDA_EA: 0x820B322
    // IDA_NAME: OldSrv_Dispatcher_Antibot_DP_dispatch_sig_CMD_DPROTO
    // DEMANGLED: OldSrv_Dispatcher_Antibot_DP_dispatch_sig_CMD_DPROTO
    // IDA_SYNTHETIC_MEMBER_LABEL: the old export has no native Itanium name;
    // compiler output is the actual member symbol verified separately.
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233B5A
    // IDA_NAME: _ZN21Dispatcher_Antibot_DP4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_Antibot_DP::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233B64
    // IDA_NAME: _ZN21Dispatcher_Antibot_DP7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_Antibot_DP::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#endif  // OLDSRV_SECURITY_ANTIBOT_DPROTO_DISPATCHER_H_
