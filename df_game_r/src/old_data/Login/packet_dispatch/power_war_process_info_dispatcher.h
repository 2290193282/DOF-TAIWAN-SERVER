#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_POWER_WAR_PROCESS_INFO_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_POWER_WAR_PROCESS_INFO_DISPATCHER_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD236.  This is a base-only request: it forwards the selected character
// number to the real guild-proxy manager and confirms the client request.
class Dispatcher_PowerWarProcessInfo
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596B28
    // IDA_NAME: _ZN30Dispatcher_PowerWarProcessInfoC2Ev
    // DEMANGLED: Dispatcher_PowerWarProcessInfo::Dispatcher_PowerWarProcessInfo(void)
    Dispatcher_PowerWarProcessInfo();

    // IDA_EA: 0x82411FA / 0x824122A
    // IDA_NAME: _ZN30Dispatcher_PowerWarProcessInfoD2Ev /
    //           _ZN30Dispatcher_PowerWarProcessInfoD0Ev
    // DEMANGLED: Dispatcher_PowerWarProcessInfo::~Dispatcher_PowerWarProcessInfo()
    virtual ~Dispatcher_PowerWarProcessInfo();

    // IDA_EA: 0x821EA32
    // IDA_NAME: _ZN30Dispatcher_PowerWarProcessInfo12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_PowerWarProcessInfo::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233EB6 / 0x8233EC0
    // IDA_NAME: _ZN30Dispatcher_PowerWarProcessInfo4readER9PacketBufR8MSG_BASE /
    //           _ZN30Dispatcher_PowerWarProcessInfo7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_PowerWarProcessInfo::read(PacketBuf &,MSG_BASE &) /
    //            Dispatcher_PowerWarProcessInfo::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_PowerWarProcessInfo) == 0x04u,
              "CMD236 dispatcher must retain the old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_POWER_WAR_PROCESS_INFO_DISPATCHER_H_
