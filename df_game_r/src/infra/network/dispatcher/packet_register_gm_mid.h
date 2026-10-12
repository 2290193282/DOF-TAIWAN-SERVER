#ifndef OLDSRV_PROTOCOL_PACKET_REGISTER_GM_MID_H_
#define OLDSRV_PROTOCOL_PACKET_REGISTER_GM_MID_H_

#include <stdint.h>

#pragma pack(push, 1)

// Monitor protocol packet sent when a character enters GM mode.  IDA sends
// exactly 0x0e bytes, so this public packet ABI must not be redefined in CUser
// translation units with host-default padding.
struct Packet_Register_GM_MID {
    // IDA_EA: 0x86945D4
    // IDA_NAME: _ZN22Packet_Register_GM_MIDC2Ev
    // DEMANGLED: Packet_Register_GM_MID::Packet_Register_GM_MID(void)
    Packet_Register_GM_MID();

    uint16_t type;
    uint16_t size;
    uint32_t checksum;
    uint16_t reserved;
    int32_t charac_no;
};

#pragma pack(pop)

static_assert(sizeof(Packet_Register_GM_MID) == 0x0eu,
              "Packet_Register_GM_MID protocol ABI must stay 0x0e");

#endif  // OLDSRV_PROTOCOL_PACKET_REGISTER_GM_MID_H_
