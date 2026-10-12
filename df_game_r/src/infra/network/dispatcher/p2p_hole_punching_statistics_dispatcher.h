#ifndef OLDSRV_PROTOCOL_P2P_HOLE_PUNCHING_STATISTICS_DISPATCHER_H_
#define OLDSRV_PROTOCOL_P2P_HOLE_PUNCHING_STATISTICS_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


// IDA unit: 99_misc_global/MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE. The original
// message starts with the 13-byte C2S base and keeps its two following dwords
// unaligned. Its C2 intentionally leaves raw_12 unwritten.
#pragma pack(push, 1)
struct MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE : MSG_BASE {
    uint8_t raw_0d;
    uint32_t raw_0e;
    uint32_t raw_12;

    // IDA_EA: 0x8260470
    // IDA_NAME: _ZN34MSG_P2P_HOLE_PUNCHING_SUCCESS_RATEC2Ev
    // DEMANGLED: MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE::MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE(void)
    MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE();
};
#pragma pack(pop)

static_assert(sizeof(MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE) == 0x16u,
              "CMD599 P2P-hole message must retain its 22-byte ABI");
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE, raw_0d) == 0x0du &&
                  offsetof(MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE, raw_0e) == 0x0eu &&
                  offsetof(MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE, raw_12) == 0x12u,
              "CMD599 P2P-hole message offsets changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif


// CMD 599 / P2P_HOLE_PUNCHING_SUCCESS_RATE. The class carries only its
// exact typed state-0 packet-dispatch base.
class Dispatcher_P2PHolePunchingSuccessRateStatistics
    : public IPacketDispatcher<MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597F32
    // IDA_NAME: _ZN47Dispatcher_P2PHolePunchingSuccessRateStatisticsC2Ev
    // DEMANGLED: Dispatcher_P2PHolePunchingSuccessRateStatistics::Dispatcher_P2PHolePunchingSuccessRateStatistics(void)
    Dispatcher_P2PHolePunchingSuccessRateStatistics();

    // IDA_EA: 0x8260908
    // IDA_NAME: _ZN47Dispatcher_P2PHolePunchingSuccessRateStatisticsD2Ev
    // DEMANGLED: Dispatcher_P2PHolePunchingSuccessRateStatistics::~Dispatcher_P2PHolePunchingSuccessRateStatistics()
    // IDA_EA: 0x8260938
    // IDA_NAME: _ZN47Dispatcher_P2PHolePunchingSuccessRateStatisticsD0Ev
    // DEMANGLED: Dispatcher_P2PHolePunchingSuccessRateStatistics::~Dispatcher_P2PHolePunchingSuccessRateStatistics()
    virtual ~Dispatcher_P2PHolePunchingSuccessRateStatistics();

    // IDA_EA: 0x825FEDA
    // IDA_NAME: _ZN47Dispatcher_P2PHolePunchingSuccessRateStatistics4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_P2PHolePunchingSuccessRateStatistics::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x825FFB6
    // IDA_NAME: _ZN47Dispatcher_P2PHolePunchingSuccessRateStatistics7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_P2PHolePunchingSuccessRateStatistics::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x82605FE
    // IDA_NAME: _ZN47Dispatcher_P2PHolePunchingSuccessRateStatistics12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_P2PHolePunchingSuccessRateStatistics::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8260608
    // IDA_NAME: _ZN47Dispatcher_P2PHolePunchingSuccessRateStatistics4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_P2PHolePunchingSuccessRateStatistics::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);
};


#if defined(__i386__)
static_assert(sizeof(Dispatcher_P2PHolePunchingSuccessRateStatistics) == 0x04u,
              "CMD599 dispatcher must retain its i386 base-only ABI");
#endif


#endif  // OLDSRV_PROTOCOL_P2P_HOLE_PUNCHING_STATISTICS_DISPATCHER_H_
