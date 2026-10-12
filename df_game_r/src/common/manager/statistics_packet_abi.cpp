#include "common/manager/statistics_packet_abi.h"

#include <cstring>

// IDA_EA: 0x8610132
// IDA_NAME: _ZN16StatisticsPacketC2Ev
// DEMANGLED: StatisticsPacket::StatisticsPacket(void)
StatisticsPacket::StatisticsPacket() : header_(0x2720u, 0x89u), value_(0) {
    std::memset(category_, 0, sizeof(category_));
    std::memset(statistic_, 0, sizeof(statistic_));
    std::memset(formatted_value_, 0, sizeof(formatted_value_));
}
