#ifndef OLDSRV_SECURITY_CHECK_SECURITY_PROTECTION_DISPATCHER_H_
#define OLDSRV_SECURITY_CHECK_SECURITY_PROTECTION_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CheckSecurityProtection : public ParamBase {
public:
    // IDA_EA: 0x8265424
    // IDA_NAME: _ZN23CheckSecurityProtectionC2Ev
    // DEMANGLED: CheckSecurityProtection::CheckSecurityProtection(void)
    CheckSecurityProtection();

    // IDA_EA: 0x82658E8
    // IDA_NAME: _ZN23CheckSecurityProtectionD2Ev
    // DEMANGLED: CheckSecurityProtection::~CheckSecurityProtection()
    // IDA_EA: 0x8265918
    // IDA_NAME: _ZN23CheckSecurityProtectionD0Ev
    // DEMANGLED: CheckSecurityProtection::~CheckSecurityProtection()
    virtual ~CheckSecurityProtection();

    uint8_t result;
};

class DisPatcher_CheckSecurityProtection
    : public IPacketDispatcher<MSG_BASE,
                               CheckSecurityProtection,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597656
    // IDA_NAME: _ZN34DisPatcher_CheckSecurityProtectionC2Ev
    // DEMANGLED: DisPatcher_CheckSecurityProtection::DisPatcher_CheckSecurityProtection(void)
    DisPatcher_CheckSecurityProtection();

    // IDA_EA: 0x826562A
    // IDA_NAME: _ZN34DisPatcher_CheckSecurityProtectionD2Ev
    // DEMANGLED: DisPatcher_CheckSecurityProtection::~DisPatcher_CheckSecurityProtection()
    // IDA_EA: 0x826565A
    // IDA_NAME: _ZN34DisPatcher_CheckSecurityProtectionD0Ev
    // DEMANGLED: DisPatcher_CheckSecurityProtection::~DisPatcher_CheckSecurityProtection()
    virtual ~DisPatcher_CheckSecurityProtection();

    // IDA_EA: 0x8264810
    // IDA_NAME: _ZN34DisPatcher_CheckSecurityProtection4readER9PacketBufR8MSG_BASE
    // DEMANGLED: DisPatcher_CheckSecurityProtection::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x826481A
    // IDA_NAME: _ZN34DisPatcher_CheckSecurityProtection7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: DisPatcher_CheckSecurityProtection::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x8264852
    // IDA_NAME: _ZN34DisPatcher_CheckSecurityProtection4sendEP5CUserR9ParamBase
    // DEMANGLED: DisPatcher_CheckSecurityProtection::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x8264912
    // IDA_NAME: _ZN34DisPatcher_CheckSecurityProtection11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: DisPatcher_CheckSecurityProtection::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(CheckSecurityProtection) == 0x08u,
              "CMD374 parameter must retain its eight-byte i386 ABI");
static_assert(sizeof(DisPatcher_CheckSecurityProtection) == 0x04u,
              "CMD374 dispatcher must retain its four-byte i386 ABI");
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(CheckSecurityProtection, result) == 0x04u,
              "CMD374 parameter result offset changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#endif

#endif  // OLDSRV_SECURITY_CHECK_SECURITY_PROTECTION_DISPATCHER_H_
