#ifndef OLDSRV_PROTOCOL_P2P_STATISTICS_DISPATCHER_H_
#define OLDSRV_PROTOCOL_P2P_STATISTICS_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "infra/network/packet/PacketHeader.h"

// CMD 479's input is an ordinary-aligned old MSG object.  The explicitly
// named gaps are compiler alignment bytes: the raw constructor does not write
// them and the reader never consumes them from PacketBuf.
struct MSG_P2P_STATISTICS : MSG_BASE {
    char raw_0d;
    char raw_0e;
    char raw_0f;
    short raw_10;
    short raw_12;
    short raw_14;
    char raw_16[2];
    int raw_18;
    int raw_1c;
    int raw_20;
    int raw_24;
    short raw_28;
    short raw_2a;
    short raw_2c;
    char raw_2e[2];
    int raw_30;
    int raw_34;
    int raw_38;
    int raw_3c;

    // IDA_EA: 0x822E1D2
    // IDA_NAME: _ZN18MSG_P2P_STATISTICSC2Ev
    // DEMANGLED: MSG_P2P_STATISTICS::MSG_P2P_STATISTICS(void)
    MSG_P2P_STATISTICS();
};

static_assert(sizeof(int) == 4u,
              "CMD479 P2P statistics uses four-byte old int fields");
static_assert(sizeof(MSG_P2P_STATISTICS) == 0x40u,
              "CMD479 P2P input must retain its 0x40-byte old ABI");
// MSG_BASE supplies data members, so C++11 calls this inherited layout
// conditionally-standard.  GCC 4.9 is the target ABI and evaluates these
// offsets correctly; retain the compile-time checks without noisy warnings.
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(MSG_P2P_STATISTICS, raw_0d) == 0x0du &&
                  offsetof(MSG_P2P_STATISTICS, raw_0e) == 0x0eu &&
                  offsetof(MSG_P2P_STATISTICS, raw_0f) == 0x0fu &&
                  offsetof(MSG_P2P_STATISTICS, raw_10) == 0x10u &&
                  offsetof(MSG_P2P_STATISTICS, raw_12) == 0x12u &&
                  offsetof(MSG_P2P_STATISTICS, raw_14) == 0x14u &&
                  offsetof(MSG_P2P_STATISTICS, raw_16) == 0x16u &&
                  offsetof(MSG_P2P_STATISTICS, raw_18) == 0x18u &&
                  offsetof(MSG_P2P_STATISTICS, raw_1c) == 0x1cu &&
                  offsetof(MSG_P2P_STATISTICS, raw_20) == 0x20u &&
                  offsetof(MSG_P2P_STATISTICS, raw_24) == 0x24u &&
                  offsetof(MSG_P2P_STATISTICS, raw_28) == 0x28u &&
                  offsetof(MSG_P2P_STATISTICS, raw_2a) == 0x2au &&
                  offsetof(MSG_P2P_STATISTICS, raw_2c) == 0x2cu &&
                  offsetof(MSG_P2P_STATISTICS, raw_2e) == 0x2eu &&
                  offsetof(MSG_P2P_STATISTICS, raw_30) == 0x30u &&
                  offsetof(MSG_P2P_STATISTICS, raw_34) == 0x34u &&
                  offsetof(MSG_P2P_STATISTICS, raw_38) == 0x38u &&
                  offsetof(MSG_P2P_STATISTICS, raw_3c) == 0x3cu,
              "CMD479 P2P input offsets changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// The old statistic proxy receives this packed record verbatim.  The group
// byte at +0x12 belongs to the record; it does not select the proxy (that
// raw call always selects group zero).
#pragma pack(push, 1)
struct Packet_P2P_Statistics {
    PacketHeader header;
    int raw_0a;
    int raw_0e;
    char raw_12;
    short raw_13;
    short raw_15;
    short raw_17;
    int raw_19;
    int raw_1d;
    int raw_21;
    int raw_25;
    short raw_29;
    short raw_2b;
    short raw_2d;
    int raw_2f;
    int raw_33;
    int raw_37;
    int raw_3b;

    // IDA_EA: 0x822C696
    // IDA_NAME: _ZN21Packet_P2P_StatisticsC2Ev
    // DEMANGLED: Packet_P2P_Statistics::Packet_P2P_Statistics(void)
    Packet_P2P_Statistics();
};
#pragma pack(pop)

static_assert(sizeof(PacketHeader) == 0x0au,
              "CMD479 P2P statistic requires a 10-byte PacketHeader");
static_assert(sizeof(Packet_P2P_Statistics) == 0x3fu,
              "CMD479 P2P statistic packet must stay 0x3f bytes");
static_assert(offsetof(Packet_P2P_Statistics, header) == 0x00u &&
                  offsetof(Packet_P2P_Statistics, raw_0a) == 0x0au &&
                  offsetof(Packet_P2P_Statistics, raw_0e) == 0x0eu &&
                  offsetof(Packet_P2P_Statistics, raw_12) == 0x12u &&
                  offsetof(Packet_P2P_Statistics, raw_13) == 0x13u &&
                  offsetof(Packet_P2P_Statistics, raw_15) == 0x15u &&
                  offsetof(Packet_P2P_Statistics, raw_17) == 0x17u &&
                  offsetof(Packet_P2P_Statistics, raw_19) == 0x19u &&
                  offsetof(Packet_P2P_Statistics, raw_1d) == 0x1du &&
                  offsetof(Packet_P2P_Statistics, raw_21) == 0x21u &&
                  offsetof(Packet_P2P_Statistics, raw_25) == 0x25u &&
                  offsetof(Packet_P2P_Statistics, raw_29) == 0x29u &&
                  offsetof(Packet_P2P_Statistics, raw_2b) == 0x2bu &&
                  offsetof(Packet_P2P_Statistics, raw_2d) == 0x2du &&
                  offsetof(Packet_P2P_Statistics, raw_2f) == 0x2fu &&
                  offsetof(Packet_P2P_Statistics, raw_33) == 0x33u &&
                  offsetof(Packet_P2P_Statistics, raw_37) == 0x37u &&
                  offsetof(Packet_P2P_Statistics, raw_3b) == 0x3bu,
              "CMD479 P2P statistic packet offsets changed");

// CMD 479 / P2P_STATISTICS.  This class remains deliberately unregistered
// until the original full 606-slot PacketDispatcher transaction is restored.
class Dispatcher_P2P_Statistics
    : public IPacketDispatcher<MSG_P2P_STATISTICS,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597CCA
    // IDA_NAME: _ZN25Dispatcher_P2P_StatisticsC2Ev
    // DEMANGLED: Dispatcher_P2P_Statistics::Dispatcher_P2P_Statistics(void)
    Dispatcher_P2P_Statistics();

    // IDA_EA: 0x82440E6
    // IDA_NAME: _ZN25Dispatcher_P2P_StatisticsD2Ev
    // DEMANGLED: Dispatcher_P2P_Statistics::~Dispatcher_P2P_Statistics()
    // IDA_EA: 0x8244116
    // IDA_NAME: _ZN25Dispatcher_P2P_StatisticsD0Ev
    // DEMANGLED: Dispatcher_P2P_Statistics::~Dispatcher_P2P_Statistics()
    virtual ~Dispatcher_P2P_Statistics();

    // IDA_EA: 0x81E2780
    // IDA_NAME: _ZN25Dispatcher_P2P_Statistics4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_P2P_Statistics::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81E2BDE
    // IDA_NAME: _ZN25Dispatcher_P2P_Statistics7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_P2P_Statistics::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81E2EF6
    // IDA_NAME: _ZN25Dispatcher_P2P_Statistics11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: Dispatcher_P2P_Statistics::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_P2P_Statistics) == 0x04u,
              "CMD479 dispatcher must retain the four-byte i386 base object");
#endif

#endif  // OLDSRV_PROTOCOL_P2P_STATISTICS_DISPATCHER_H_
