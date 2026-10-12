#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_CERA_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_CERA_DISPATCHER_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD66 / CERA.  The old handler is a base-only state-0 dispatcher: it first
// queries the existing IPG helper (virtual slot +0x2c) and then requests the
// CUser mileage load through the established DB queue path.
class DisPatcher_Cera
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595B30
    // IDA_NAME: _ZN15DisPatcher_CeraC2Ev
    // DEMANGLED: DisPatcher_Cera::DisPatcher_Cera(void)
    DisPatcher_Cera();

    // IDA_EA: 0x82436D8 / 0x8243708
    // IDA_NAME: _ZN15DisPatcher_CeraD2Ev / _ZN15DisPatcher_CeraD0Ev
    // DEMANGLED: DisPatcher_Cera::~DisPatcher_Cera()
    virtual ~DisPatcher_Cera();

    // IDA_EA: 0x81FD360
    // IDA_NAME: _ZN15DisPatcher_Cera12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: DisPatcher_Cera::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x82334A4 / 0x82334AE
    // IDA_NAME: _ZN15DisPatcher_Cera4readER9PacketBufR8MSG_BASE /
    //           _ZN15DisPatcher_Cera7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: DisPatcher_Cera::read(PacketBuf &,MSG_BASE &) /
    //            DisPatcher_Cera::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_Cera) == 0x04u,
              "CMD66 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_CERA_DISPATCHER_H_
