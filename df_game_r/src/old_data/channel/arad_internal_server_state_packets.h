#ifndef OLDSRV_OLD_DATA_CHANNEL_ARAD_INTERNAL_SERVER_STATE_PACKETS_H_
#define OLDSRV_OLD_DATA_CHANNEL_ARAD_INTERNAL_SERVER_STATE_PACKETS_H_

#include <stddef.h>
#include <stdint.h>

// CDispatchServerPacket forwards these two complete control packets into the
// ARAD inner dispatcher.  The first fourteen bytes are transport/header data
// owned by that dispatcher and are deliberately not interpreted here.  The
// following fields and their offsets are the direct reads in the original
// handlers at 0x8199278 and 0x819935E.
#pragma pack(push, 1)
struct Packet_Arad_ApplyEffect {
    uint8_t transport_header[0x0e];
    int32_t effect;
    uint32_t end_sec;
};

struct Packet_Arad_DeleteEffect {
    uint8_t transport_header[0x0e];
    int32_t effect;
};
#pragma pack(pop)

static_assert(offsetof(Packet_Arad_ApplyEffect, effect) == 0x0eu &&
                  offsetof(Packet_Arad_ApplyEffect, end_sec) == 0x12u &&
                  sizeof(Packet_Arad_ApplyEffect) == 0x16u,
              "Packet_Arad_ApplyEffect direct-read ABI changed");
static_assert(offsetof(Packet_Arad_DeleteEffect, effect) == 0x0eu &&
                  sizeof(Packet_Arad_DeleteEffect) == 0x12u,
              "Packet_Arad_DeleteEffect direct-read ABI changed");

#endif  // OLDSRV_OLD_DATA_CHANNEL_ARAD_INTERNAL_SERVER_STATE_PACKETS_H_
