#ifndef OLDSRV_PROTOCOL_SECURITY_LOG_DISPATCHER_H_
#define OLDSRV_PROTOCOL_SECURITY_LOG_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD 538 (ENUM_CMDPACKET_ANTIBOT_DELAY_LOG) reads six unaligned contiguous
// 32-bit words immediately after the 13-byte old C2S MSG_BASE.  The raw
// MSG_SECURITY_LOG_PACKET C2 first calls MSG_BASE::C2, then clears the entire
// packed 0x25-byte storage block; do not add a naturally aligned gap here.
#pragma pack(push, 1)
struct MSG_SECURITY_LOG_PACKET : MSG_BASE {
    uint32_t raw_0d;
    uint32_t raw_11;
    uint32_t raw_15;
    uint32_t raw_19;
    uint32_t raw_1d;
    uint32_t raw_21;

    // IDA_EA: 0x82602EC
    // IDA_NAME: _ZN23MSG_SECURITY_LOG_PACKETC2Ev
    // DEMANGLED: MSG_SECURITY_LOG_PACKET::MSG_SECURITY_LOG_PACKET(void)
    MSG_SECURITY_LOG_PACKET();
};
#pragma pack(pop)

static_assert(sizeof(MSG_SECURITY_LOG_PACKET) == 0x25u,
              "CMD538 security-log input must retain its packed 0x25-byte ABI");
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(MSG_SECURITY_LOG_PACKET, raw_0d) == 0x0du &&
                  offsetof(MSG_SECURITY_LOG_PACKET, raw_11) == 0x11u &&
                  offsetof(MSG_SECURITY_LOG_PACKET, raw_15) == 0x15u &&
                  offsetof(MSG_SECURITY_LOG_PACKET, raw_19) == 0x19u &&
                  offsetof(MSG_SECURITY_LOG_PACKET, raw_1d) == 0x1du &&
                  offsetof(MSG_SECURITY_LOG_PACKET, raw_21) == 0x21u,
              "CMD538 security-log input offsets changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// The raw state-0 dispatch_template constructs this polymorphic parameter
// even though Dispatcher_SecurityLog does not populate payload fields.
struct stSecurityLogParam : ParamBase {
    // IDA_EA: 0x826057A
    // IDA_NAME: _ZN18stSecurityLogParamC2Ev
    // DEMANGLED: stSecurityLogParam::stSecurityLogParam(void)
    stSecurityLogParam();

    // IDA_EA: 0x8260C14
    // IDA_NAME: _ZN18stSecurityLogParamD2Ev
    // DEMANGLED: stSecurityLogParam::~stSecurityLogParam()
    // IDA_EA: 0x8260C44
    // IDA_NAME: _ZN18stSecurityLogParamD0Ev
    // DEMANGLED: stSecurityLogParam::~stSecurityLogParam()
    virtual ~stSecurityLogParam();
};

#if defined(__i386__)
static_assert(sizeof(stSecurityLogParam) == 0x04u,
              "CMD538 security-log parameter must retain its i386 vptr ABI");
#endif

// This raw concrete handler is an isolated provider only.  It deliberately
// remains absent from the live PacketDispatcher 606-slot construction table.
class Dispatcher_SecurityLog
    : public IPacketDispatcher<MSG_SECURITY_LOG_PACKET,
                               stSecurityLogParam,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85956B4
    // IDA_NAME: _ZN22Dispatcher_SecurityLogC2Ev
    // DEMANGLED: Dispatcher_SecurityLog::Dispatcher_SecurityLog(void)
    Dispatcher_SecurityLog();

    // IDA_EA: 0x8260596
    // IDA_NAME: _ZN22Dispatcher_SecurityLogD2Ev
    // DEMANGLED: Dispatcher_SecurityLog::~Dispatcher_SecurityLog()
    // IDA_EA: 0x82605C6
    // IDA_NAME: _ZN22Dispatcher_SecurityLogD0Ev
    // DEMANGLED: Dispatcher_SecurityLog::~Dispatcher_SecurityLog()
    virtual ~Dispatcher_SecurityLog();

    // IDA_EA: 0x825FABC
    // IDA_NAME: _ZN22Dispatcher_SecurityLog4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_SecurityLog::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x825FB38
    // IDA_NAME: _ZN22Dispatcher_SecurityLog7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_SecurityLog::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x825FC34
    // IDA_NAME: _ZN22Dispatcher_SecurityLog4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_SecurityLog::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SecurityLog) == 0x04u,
              "CMD538 security-log dispatcher must retain the i386 base ABI");
#endif

#endif  // OLDSRV_PROTOCOL_SECURITY_LOG_DISPATCHER_H_
