#ifndef OLDSRV_OLD_GAME_WORLD_PACKET_DISPATCHER_SET_POS_RUNTIME_H_
#define OLDSRV_OLD_GAME_WORLD_PACKET_DISPATCHER_SET_POS_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


class CUser;
class PacketBuf;

// CMD 37 / SET_USER_POSITION.  The old reader starts immediately after the
// 13-byte C2S MSG_BASE and reads x, y, direction, and position context in
// this exact order.  Unlike later packets, there is no compiler alignment
// gap: the raw object ends at +0x14.
#pragma pack(push, 1)
struct MSG_USER_SET_POS : MSG_BASE {
    uint16_t position_x;       // +0x0d
    uint16_t position_y;       // +0x0f
    char direction;            // +0x11 (raw reader uses PacketBuf::get_byte)
    uint16_t position_context; // +0x12
};
#pragma pack(pop)

static_assert(sizeof(MSG_USER_SET_POS) == 0x14u,
              "CMD37 SET_USER_POSITION must retain its 20-byte old ABI");
// MSG_BASE supplies data members, so C++11 calls this inherited layout
// conditionally-standard.  GCC 4.9/i386 is the target ABI.
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(MSG_USER_SET_POS, position_x) == 0x0du &&
                  offsetof(MSG_USER_SET_POS, position_y) == 0x0fu &&
                  offsetof(MSG_USER_SET_POS, direction) == 0x11u &&
                  offsetof(MSG_USER_SET_POS, position_context) == 0x12u,
              "CMD37 SET_USER_POSITION field offsets changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// The original CMD37 handler is a four-byte i386 IPacketDispatcher object.
// This semantic world provider deliberately remains unregistered until A's
// complete PacketDispatcher root, ingress validation, and return chains are
// restored atomically.
class DisPatcher_SetPos
    : public IPacketDispatcher<MSG_USER_SET_POS,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    DisPatcher_SetPos();
    virtual ~DisPatcher_SetPos();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_SetPos) == 0x04u,
              "CMD37 DisPatcher_SetPos must retain its i386 base footprint");
#endif

#endif  // OLDSRV_OLD_GAME_WORLD_PACKET_DISPATCHER_SET_POS_RUNTIME_H_
