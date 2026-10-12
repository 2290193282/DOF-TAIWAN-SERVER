#ifndef OLDSRV_OLD_DATA_CHANNEL_ARAD_SERVER_STATE_MESSAGE_H_
#define OLDSRV_OLD_DATA_CHANNEL_ARAD_SERVER_STATE_MESSAGE_H_

#include <stdint.h>

// IDA unit: 03_channel_select/AradServerStateMessage.  The end timestamp is
// deliberately unaligned at old offset +0x02.
#pragma pack(push, 1)
struct AradServerStateMessage {
    AradServerStateMessage();

    uint16_t active;
    uint32_t end_sec;
    uint8_t raw_06[6];
};
#pragma pack(pop)

typedef char AradServerStateMessage_Size_Must_Be_0x0C[
    (sizeof(AradServerStateMessage) == 0x0C) ? 1 : -1];

#endif  // OLDSRV_OLD_DATA_CHANNEL_ARAD_SERVER_STATE_MESSAGE_H_
