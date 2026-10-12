#ifndef OLDSRV_PROTOCOL_LAG_STATISTICS_DISPATCHER_H_
#define OLDSRV_PROTOCOL_LAG_STATISTICS_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "infra/network/packet/PacketHeader.h"

// CMD 282 sends this fixed statistic-server packet directly.  The old
// constructor's write sequence is represented by raw-offset fields rather
// than inferred game-domain names.
#pragma pack(push, 1)
struct LagStatisticsModuleRaw {
    int raw_00;
    int raw_04;
    uint16_t raw_08;
};

struct LagStatisticsDungeonRaw {
    uint16_t raw_00;
    uint8_t raw_02;
    int raw_03;
    int raw_07;
    uint16_t raw_0b;
    uint8_t raw_0d;
    int raw_0e;
    int raw_12;
    uint16_t raw_16;
};

struct Packet_Stat_Lag_Statistics {
    PacketHeader header;
    uint8_t module_count;
    LagStatisticsModuleRaw modules[8];
    uint8_t dungeon_count;
    LagStatisticsDungeonRaw dungeons[10];

    // IDA_EA: 0x822C144
    // IDA_NAME: _ZN26Packet_Stat_Lag_StatisticsC2Ev
    // DEMANGLED: Packet_Stat_Lag_Statistics::Packet_Stat_Lag_Statistics(void)
    Packet_Stat_Lag_Statistics();

    // IDA_EA: 0x82354EC
    // IDA_NAME: _ZN26Packet_Stat_Lag_StatisticsD2Ev
    // DEMANGLED: Packet_Stat_Lag_Statistics::~Packet_Stat_Lag_Statistics()
    ~Packet_Stat_Lag_Statistics();
};
#pragma pack(pop)

static_assert(sizeof(int) == 4u,
              "CMD282 statistic wire fields require 32-bit int");
static_assert(sizeof(PacketHeader) == 0x0Au,
              "CMD282 statistic packet requires a 10-byte PacketHeader");
static_assert(sizeof(LagStatisticsModuleRaw) == 0x0Au,
              "CMD282 module statistic entry width changed");
static_assert(offsetof(LagStatisticsModuleRaw, raw_00) == 0x00u &&
                  offsetof(LagStatisticsModuleRaw, raw_04) == 0x04u &&
                  offsetof(LagStatisticsModuleRaw, raw_08) == 0x08u,
              "CMD282 module statistic entry offsets changed");
static_assert(sizeof(LagStatisticsDungeonRaw) == 0x18u,
              "CMD282 dungeon statistic entry width changed");
static_assert(offsetof(LagStatisticsDungeonRaw, raw_00) == 0x00u &&
                  offsetof(LagStatisticsDungeonRaw, raw_02) == 0x02u &&
                  offsetof(LagStatisticsDungeonRaw, raw_03) == 0x03u &&
                  offsetof(LagStatisticsDungeonRaw, raw_07) == 0x07u &&
                  offsetof(LagStatisticsDungeonRaw, raw_0b) == 0x0Bu &&
                  offsetof(LagStatisticsDungeonRaw, raw_0d) == 0x0Du &&
                  offsetof(LagStatisticsDungeonRaw, raw_0e) == 0x0Eu &&
                  offsetof(LagStatisticsDungeonRaw, raw_12) == 0x12u &&
                  offsetof(LagStatisticsDungeonRaw, raw_16) == 0x16u,
              "CMD282 dungeon statistic entry offsets changed");
static_assert(sizeof(Packet_Stat_Lag_Statistics) == 0x14Cu,
              "CMD282 statistic packet must stay 0x14C bytes");
static_assert(offsetof(Packet_Stat_Lag_Statistics, header) == 0x00u &&
                  offsetof(Packet_Stat_Lag_Statistics, module_count) == 0x0Au &&
                  offsetof(Packet_Stat_Lag_Statistics, modules) == 0x0Bu &&
                  offsetof(Packet_Stat_Lag_Statistics, dungeon_count) == 0x5Bu &&
                  offsetof(Packet_Stat_Lag_Statistics, dungeons) == 0x5Cu,
              "CMD282 statistic packet offsets changed");

// CMD 282 / LAG_STATISTICS.  This remains an unregistered state-0 provider
// until the original full PacketDispatcher transaction is restored.
class Dispatcher_Lag_Statistics
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596F6C
    // IDA_NAME: _ZN25Dispatcher_Lag_StatisticsC2Ev
    // DEMANGLED: Dispatcher_Lag_Statistics::Dispatcher_Lag_Statistics(void)
    Dispatcher_Lag_Statistics();

    // IDA_EA: 0x8240AAA
    // IDA_NAME: _ZN25Dispatcher_Lag_StatisticsD2Ev
    // DEMANGLED: Dispatcher_Lag_Statistics::~Dispatcher_Lag_Statistics()
    // IDA_EA: 0x8240ADA
    // IDA_NAME: _ZN25Dispatcher_Lag_StatisticsD0Ev
    // DEMANGLED: Dispatcher_Lag_Statistics::~Dispatcher_Lag_Statistics()
    virtual ~Dispatcher_Lag_Statistics();

    // IDA_EA: 0x8223DE0
    // IDA_NAME: _ZN25Dispatcher_Lag_Statistics12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_Lag_Statistics::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8234096
    // IDA_NAME: _ZN25Dispatcher_Lag_Statistics4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_Lag_Statistics::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x82340A0
    // IDA_NAME: _ZN25Dispatcher_Lag_Statistics7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_Lag_Statistics::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#endif  // OLDSRV_PROTOCOL_LAG_STATISTICS_DISPATCHER_H_
