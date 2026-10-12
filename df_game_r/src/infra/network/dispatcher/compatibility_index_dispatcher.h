#ifndef OLDSRV_PROTOCOL_COMPATIBILITY_INDEX_DISPATCHER_H_
#define OLDSRV_PROTOCOL_COMPATIBILITY_INDEX_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "infra/network/packet/PacketHeader.h"

// CMD 477 stores this input object on the old i386 dispatch_template stack.
// MSG_BASE ends at +0x0d; the compiler-owned +0x0f alignment byte is required
// before raw_10, and the object retains its two trailing alignment bytes.
struct MSG_COMPATIBILITY_INDEX : MSG_BASE {
    uint8_t raw_0d;
    uint8_t raw_0e;
    uint32_t raw_10;
    uint16_t raw_14;
    uint16_t raw_16;
    uint16_t raw_18;
    uint16_t raw_1a;
    uint8_t raw_1c;
    uint8_t raw_1d;

    // IDA_EA: 0x822E174
    // IDA_NAME: _ZN23MSG_COMPATIBILITY_INDEXC2Ev
    // DEMANGLED: MSG_COMPATIBILITY_INDEX::MSG_COMPATIBILITY_INDEX(void)
    MSG_COMPATIBILITY_INDEX();
};

static_assert(sizeof(MSG_COMPATIBILITY_INDEX) == 0x20u,
              "CMD477 compatibility input must retain its 0x20-byte ABI");
// MSG_BASE supplies data members, so C++11 calls this inherited layout
// conditionally-standard.  GCC 4.9 is the target ABI and evaluates these
// offsets correctly; retain the compile-time checks without noisy warnings.
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(MSG_COMPATIBILITY_INDEX, raw_0d) == 0x0du &&
                  offsetof(MSG_COMPATIBILITY_INDEX, raw_0e) == 0x0eu &&
                  offsetof(MSG_COMPATIBILITY_INDEX, raw_10) == 0x10u &&
                  offsetof(MSG_COMPATIBILITY_INDEX, raw_14) == 0x14u &&
                  offsetof(MSG_COMPATIBILITY_INDEX, raw_16) == 0x16u &&
                  offsetof(MSG_COMPATIBILITY_INDEX, raw_18) == 0x18u &&
                  offsetof(MSG_COMPATIBILITY_INDEX, raw_1a) == 0x1au &&
                  offsetof(MSG_COMPATIBILITY_INDEX, raw_1c) == 0x1cu &&
                  offsetof(MSG_COMPATIBILITY_INDEX, raw_1d) == 0x1du,
              "CMD477 compatibility input offsets changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// The statistic-server record is sent verbatim, including the unaligned
// dwords immediately after the old 10-byte PacketHeader.
#pragma pack(push, 1)
struct Packet_Stat_Compatibility_Index {
    PacketHeader header;
    int32_t raw_0a;
    uint8_t raw_0e;
    uint8_t raw_0f;
    int32_t raw_10;
    uint16_t raw_14;
    uint16_t raw_16;
    uint16_t raw_18;
    uint16_t raw_1a;
    uint8_t raw_1c;
    uint8_t raw_1d;

    // IDA_EA: 0x822C61E
    // IDA_NAME: _ZN31Packet_Stat_Compatibility_IndexC2Ev
    // DEMANGLED: Packet_Stat_Compatibility_Index::Packet_Stat_Compatibility_Index(void)
    Packet_Stat_Compatibility_Index();
};
#pragma pack(pop)

static_assert(sizeof(PacketHeader) == 0x0au,
              "CMD477 compatibility statistic requires a 10-byte header");
static_assert(sizeof(Packet_Stat_Compatibility_Index) == 0x1eu,
              "CMD477 compatibility statistic must stay 0x1e bytes");
static_assert(offsetof(Packet_Stat_Compatibility_Index, header) == 0x00u &&
                  offsetof(Packet_Stat_Compatibility_Index, raw_0a) == 0x0au &&
                  offsetof(Packet_Stat_Compatibility_Index, raw_0e) == 0x0eu &&
                  offsetof(Packet_Stat_Compatibility_Index, raw_0f) == 0x0fu &&
                  offsetof(Packet_Stat_Compatibility_Index, raw_10) == 0x10u &&
                  offsetof(Packet_Stat_Compatibility_Index, raw_14) == 0x14u &&
                  offsetof(Packet_Stat_Compatibility_Index, raw_16) == 0x16u &&
                  offsetof(Packet_Stat_Compatibility_Index, raw_18) == 0x18u &&
                  offsetof(Packet_Stat_Compatibility_Index, raw_1a) == 0x1au &&
                  offsetof(Packet_Stat_Compatibility_Index, raw_1c) == 0x1cu &&
                  offsetof(Packet_Stat_Compatibility_Index, raw_1d) == 0x1du,
              "CMD477 compatibility statistic offsets changed");

// CMD 477 / COMPATIBILITY_INDEX.  This concrete state-0 handler is kept
// unregistered until the full original 606-slot PacketDispatcher transaction
// is restored.
class Dispatcher_CompatibilityIndex
    : public IPacketDispatcher<MSG_COMPATIBILITY_INDEX,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597CAE
    // IDA_NAME: _ZN29Dispatcher_CompatibilityIndexC2Ev
    // DEMANGLED: Dispatcher_CompatibilityIndex::Dispatcher_CompatibilityIndex(void)
    Dispatcher_CompatibilityIndex();

    // IDA_EA: 0x8244134
    // IDA_NAME: _ZN29Dispatcher_CompatibilityIndexD2Ev
    // DEMANGLED: Dispatcher_CompatibilityIndex::~Dispatcher_CompatibilityIndex()
    // IDA_EA: 0x8244164
    // IDA_NAME: _ZN29Dispatcher_CompatibilityIndexD0Ev
    // DEMANGLED: Dispatcher_CompatibilityIndex::~Dispatcher_CompatibilityIndex()
    virtual ~Dispatcher_CompatibilityIndex();

    // IDA_EA: 0x81E23E0
    // IDA_NAME: _ZN29Dispatcher_CompatibilityIndex11check_errorEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_CompatibilityIndex::check_error(CUser *,MSG_BASE &,ParamBase &)
    int check_error(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81E23EA
    // IDA_NAME: _ZN29Dispatcher_CompatibilityIndex4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_CompatibilityIndex::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81E2664
    // IDA_NAME: _ZN29Dispatcher_CompatibilityIndex7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_CompatibilityIndex::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_CompatibilityIndex) == 0x04u,
              "CMD477 dispatcher must retain the four-byte i386 base object");
#endif

#endif  // OLDSRV_PROTOCOL_COMPATIBILITY_INDEX_DISPATCHER_H_
