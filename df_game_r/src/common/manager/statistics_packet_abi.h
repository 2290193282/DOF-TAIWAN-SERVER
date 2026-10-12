#ifndef OLDSRV_GLOBAL_STATISTICS_PACKET_ABI_H_
#define OLDSRV_GLOBAL_STATISTICS_PACKET_ABI_H_

#include <stdint.h>

#include "infra/network/packet/PacketHeader.h"

// IDA unit: 99_misc_global/StatisticsPacket.  The wire object is deliberately
// packed: the final unsigned field starts at +0x85, immediately after the
// 57-byte formatted-value string.
#pragma pack(push, 1)
struct StatisticsPacket {
    PacketHeader header_;             // +0x00
    char category_[33];               // +0x0a
    char statistic_[33];              // +0x2b
    char formatted_value_[57];        // +0x4c
    uint32_t value_;                  // +0x85

    // IDA_EA: 0x8610132
    // IDA_NAME: _ZN16StatisticsPacketC2Ev
    // DEMANGLED: StatisticsPacket::StatisticsPacket(void)
    StatisticsPacket();
};
#pragma pack(pop)

static_assert(sizeof(StatisticsPacket) == 0x89u,
              "StatisticsPacket IDA ABI size must stay 0x89");

#endif  // OLDSRV_GLOBAL_STATISTICS_PACKET_ABI_H_
