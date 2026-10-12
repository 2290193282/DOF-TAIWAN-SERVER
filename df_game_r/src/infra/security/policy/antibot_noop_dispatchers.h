#ifndef OLDSRV_SECURITY_ANTIBOT_NOOP_DISPATCHERS_H_
#define OLDSRV_SECURITY_ANTIBOT_NOOP_DISPATCHERS_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD603 and CMD605 retain no-op MSG_BASE/ParamBase read/process callbacks,
// but both override dispatch_sig in the original vtables.  Their direct
// handlers are outside the generated class units (0x820B50C/0x820B17A), so
// keep the real packet boundary here rather than letting the state-0 template
// silently consume the anti-bot handshake.
class Dispatcher_Antibot
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85965E8
    // IDA_NAME: _ZN18Dispatcher_AntibotC2Ev
    Dispatcher_Antibot();

    // IDA_EA: 0x8241F62
    // IDA_NAME: _ZN18Dispatcher_AntibotD2Ev
    // IDA_EA: 0x8241F92
    // IDA_NAME: _ZN18Dispatcher_AntibotD0Ev
    virtual ~Dispatcher_Antibot();

    // IDA_EA: 0x820B50C
    // IDA_NAME: OldSrv_Dispatcher_Antibot_dispatch_sig_CMD_ANTIBOT
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233B6E
    // IDA_NAME: _ZN18Dispatcher_Antibot4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233B78
    // IDA_NAME: _ZN18Dispatcher_Antibot7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_Antibot_DPCallBack
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85965B0
    // IDA_NAME: _ZN29Dispatcher_Antibot_DPCallBackC2Ev
    Dispatcher_Antibot_DPCallBack();

    // IDA_EA: 0x8241FFE
    // IDA_NAME: _ZN29Dispatcher_Antibot_DPCallBackD2Ev
    // IDA_EA: 0x824202E
    // IDA_NAME: _ZN29Dispatcher_Antibot_DPCallBackD0Ev
    virtual ~Dispatcher_Antibot_DPCallBack();

    // IDA_EA: 0x820B17A
    // IDA_NAME: OldSrv_Dispatcher_Antibot_DPCallBack_dispatch_sig_CMD_DPROTO_CALLBACK
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233B46
    // IDA_NAME: _ZN29Dispatcher_Antibot_DPCallBack4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233B50
    // IDA_NAME: _ZN29Dispatcher_Antibot_DPCallBack7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Antibot) == 0x04u,
              "CMD603 dispatcher must retain its four-byte i386 ABI");
static_assert(sizeof(Dispatcher_Antibot_DPCallBack) == 0x04u,
              "CMD605 dispatcher must retain its four-byte i386 ABI");
#endif

#endif  // OLDSRV_SECURITY_ANTIBOT_NOOP_DISPATCHERS_H_
