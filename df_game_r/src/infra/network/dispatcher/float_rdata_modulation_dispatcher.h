#ifndef OLDSRV_PROTOCOL_FLOAT_RDATA_MODULATION_DISPATCHER_H_
#define OLDSRV_PROTOCOL_FLOAT_RDATA_MODULATION_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD 453 is an ordinary-aligned old message object.  MSG_BASE ends at
// +0x0d; the next two dwords start at +0x10/+0x14 after the compiler's three
// alignment bytes.  The raw provider has no
// MSG_FLOAT_RDATA_MODULATION::C2 identity: do not add a constructor or
// initialize these bytes.  The shared typed template constructs only the
// no-write MSG_BASE base where the ABI requires a C++ object.
struct MSG_FLOAT_RDATA_MODULATION : MSG_BASE {
    uint8_t raw_0d_to_0f[3];
    uint32_t raw_10;
    uint32_t raw_14;
};

static_assert(sizeof(MSG_FLOAT_RDATA_MODULATION) == 0x18u,
              "CMD453 FLOAT_RDATA message must retain its 0x18-byte ABI");
// MSG_BASE supplies data members, so C++11 calls this inherited layout
// conditionally-standard.  GCC 4.9 is the target ABI and evaluates these
// offsets correctly; retain the compile-time checks without noisy warnings.
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(MSG_FLOAT_RDATA_MODULATION, raw_0d_to_0f) == 0x0du &&
                  offsetof(MSG_FLOAT_RDATA_MODULATION, raw_10) == 0x10u &&
                  offsetof(MSG_FLOAT_RDATA_MODULATION, raw_14) == 0x14u,
              "CMD453 FLOAT_RDATA message offsets changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// CMD 453 / FLOAT_RDATA_MODULATION.  The original constructor table allocates
// exactly four i386 bytes for this state-0 dispatcher.  It remains deliberately
// unregistered until the complete original 606-slot PacketDispatcher
// transaction, ingress route, and return chains are restored.
class Dispatcher_Float_RDATA_Modulation
    : public IPacketDispatcher<MSG_FLOAT_RDATA_MODULATION,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597AB6
    // IDA_NAME: _ZN33Dispatcher_Float_RDATA_ModulationC2Ev
    // DEMANGLED: Dispatcher_Float_RDATA_Modulation::Dispatcher_Float_RDATA_Modulation(void)
    Dispatcher_Float_RDATA_Modulation();

    // IDA_EA: 0x826558E
    // IDA_NAME: _ZN33Dispatcher_Float_RDATA_ModulationD2Ev
    // DEMANGLED: Dispatcher_Float_RDATA_Modulation::~Dispatcher_Float_RDATA_Modulation()
    // IDA_EA: 0x82655BE
    // IDA_NAME: _ZN33Dispatcher_Float_RDATA_ModulationD0Ev
    // DEMANGLED: Dispatcher_Float_RDATA_Modulation::~Dispatcher_Float_RDATA_Modulation()
    virtual ~Dispatcher_Float_RDATA_Modulation();

    // IDA_EA: 0x8264A3A
    // IDA_NAME: _ZN33Dispatcher_Float_RDATA_Modulation4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_Float_RDATA_Modulation::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8264ACC
    // IDA_NAME: _ZN33Dispatcher_Float_RDATA_Modulation7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_Float_RDATA_Modulation::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x8264B5C
    // IDA_NAME: _ZN33Dispatcher_Float_RDATA_Modulation11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: Dispatcher_Float_RDATA_Modulation::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Float_RDATA_Modulation) == 0x04u,
              "CMD453 dispatcher must retain the four-byte i386 base object");
#endif

#endif  // OLDSRV_PROTOCOL_FLOAT_RDATA_MODULATION_DISPATCHER_H_
