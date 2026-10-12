#ifndef OLDSRV_PROTOCOL_FRAME_LAG_STATISTICS_DISPATCHER_H_
#define OLDSRV_PROTOCOL_FRAME_LAG_STATISTICS_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "infra/network/packet/PacketHeader.h"

// CMD 197 sends this fixed packet directly to the statistic-server proxy.
// The raw body only proves byte offsets, widths, and parser order, so the
// unnamed payload fields deliberately retain raw-offset names.
#pragma pack(push, 1)
struct FrameLagStatisticRawCell {
    int raw_00;
    char raw_04[4];
};

struct FrameLagStatisticRawRow {
    short raw_00;
    short raw_02;
    short raw_04;
    short raw_06;
    FrameLagStatisticRawCell raw_08[6];
};

struct Packet_Frame_Lag_Statistic_Add {
    PacketHeader header;
    int raw_0a;
    char raw_0e;
    char raw_0f;

    // IDA 0x8217046 never writes these two bytes before the raw 0x186-byte
    // send. They are explicit raw storage rather than a guessed field and
    // must remain uninitialized after this constructor returns.
    char raw_10[2];

    unsigned short raw_12;
    unsigned short raw_14;
    short raw_16;
    short raw_18;
    char raw_1a;
    int raw_1b;
    char raw_1f;
    short raw_20;
    short raw_22;
    FrameLagStatisticRawRow raw_24[6];
    char raw_174[6];
    short raw_17a[6];

    // IDA_EA: 0x822BCF4
    // IDA_NAME: _ZN30Packet_Frame_Lag_Statistic_AddC2Ev
    // DEMANGLED: Packet_Frame_Lag_Statistic_Add::Packet_Frame_Lag_Statistic_Add(void)
    Packet_Frame_Lag_Statistic_Add();
};
#pragma pack(pop)

static_assert(sizeof(int) == 4u,
              "CMD197 frame-lag wire fields require 32-bit int");
static_assert(sizeof(PacketHeader) == 0x0Au,
              "CMD197 frame-lag packet requires a 10-byte PacketHeader");
static_assert(sizeof(FrameLagStatisticRawCell) == 0x08u,
              "CMD197 frame-lag cell width changed");
static_assert(sizeof(FrameLagStatisticRawRow) == 0x38u &&
                  offsetof(FrameLagStatisticRawRow, raw_08) == 0x08u,
              "CMD197 frame-lag row layout changed");
static_assert(sizeof(Packet_Frame_Lag_Statistic_Add) == 0x186u,
              "CMD197 frame-lag statistic packet must stay 0x186 bytes");
static_assert(offsetof(Packet_Frame_Lag_Statistic_Add, header) == 0x00u &&
                  offsetof(Packet_Frame_Lag_Statistic_Add, raw_0a) == 0x0Au &&
                  offsetof(Packet_Frame_Lag_Statistic_Add, raw_0e) == 0x0Eu &&
                  offsetof(Packet_Frame_Lag_Statistic_Add, raw_0f) == 0x0Fu &&
                  offsetof(Packet_Frame_Lag_Statistic_Add, raw_10) == 0x10u &&
                  offsetof(Packet_Frame_Lag_Statistic_Add, raw_12) == 0x12u &&
                  offsetof(Packet_Frame_Lag_Statistic_Add, raw_14) == 0x14u &&
                  offsetof(Packet_Frame_Lag_Statistic_Add, raw_16) == 0x16u &&
                  offsetof(Packet_Frame_Lag_Statistic_Add, raw_18) == 0x18u &&
                  offsetof(Packet_Frame_Lag_Statistic_Add, raw_1a) == 0x1Au &&
                  offsetof(Packet_Frame_Lag_Statistic_Add, raw_1b) == 0x1Bu &&
                  offsetof(Packet_Frame_Lag_Statistic_Add, raw_1f) == 0x1Fu &&
                  offsetof(Packet_Frame_Lag_Statistic_Add, raw_20) == 0x20u &&
                  offsetof(Packet_Frame_Lag_Statistic_Add, raw_22) == 0x22u &&
                  offsetof(Packet_Frame_Lag_Statistic_Add, raw_24) == 0x24u &&
                  offsetof(Packet_Frame_Lag_Statistic_Add, raw_174) == 0x174u &&
                  offsetof(Packet_Frame_Lag_Statistic_Add, raw_17a) == 0x17Au,
              "CMD197 frame-lag statistic packet offsets changed");

// CMD 197 / FRAME_LAG_STATISTICS. This concrete state-0 dispatcher remains
// unregistered until the original full 606-slot PacketDispatcher transaction
// is available.
class Dispatcher_FrameLagStatistics
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596834
    // IDA_NAME: _ZN29Dispatcher_FrameLagStatisticsC2Ev
    // DEMANGLED: Dispatcher_FrameLagStatistics::Dispatcher_FrameLagStatistics(void)
    Dispatcher_FrameLagStatistics();

    // IDA_EA: 0x82419E6
    // IDA_NAME: _ZN29Dispatcher_FrameLagStatisticsD2Ev
    // DEMANGLED: Dispatcher_FrameLagStatistics::~Dispatcher_FrameLagStatistics()
    // IDA_EA: 0x8241A16
    // IDA_NAME: _ZN29Dispatcher_FrameLagStatisticsD0Ev
    // DEMANGLED: Dispatcher_FrameLagStatistics::~Dispatcher_FrameLagStatistics()
    virtual ~Dispatcher_FrameLagStatistics();

    // IDA_EA: 0x8217046
    // IDA_NAME: _ZN29Dispatcher_FrameLagStatistics12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_FrameLagStatistics::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233CC2
    // IDA_NAME: _ZN29Dispatcher_FrameLagStatistics4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_FrameLagStatistics::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233CCC
    // IDA_NAME: _ZN29Dispatcher_FrameLagStatistics7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_FrameLagStatistics::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#endif  // OLDSRV_PROTOCOL_FRAME_LAG_STATISTICS_DISPATCHER_H_
