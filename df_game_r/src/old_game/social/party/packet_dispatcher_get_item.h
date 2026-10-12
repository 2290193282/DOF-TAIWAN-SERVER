#ifndef OLDSRV_OLD_GAME_GROUP_PACKET_DISPATCHER_GET_ITEM_H_
#define OLDSRV_OLD_GAME_GROUP_PACKET_DISPATCHER_GET_ITEM_H_

#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 13_database_dispatch/DisPatcher_GetItem (0828 CMD43).
// Three trailing u16 fields are consumed in wire order +26, +28, +24.  The
// read-status byte is a local result at +23, not a client-supplied field.
struct __attribute__((packed)) MSG_GET_ITEM : MSG_BASE {
    uint16_t unique_id;                 // +0x0d
    uint8_t client_routing_flag;        // +0x0f
    uint8_t client_rarity;              // +0x10
    uint16_t validation_x;              // +0x11
    uint16_t validation_y;              // +0x13
    uint16_t encoded_check;             // +0x15
    uint8_t read_status;                // +0x17, local read result
    uint16_t packet_extra;              // +0x18, read last
    uint16_t history_x;                 // +0x1a, read first
    uint16_t history_y;                 // +0x1c, read second
};

static_assert(sizeof(MSG_GET_ITEM) == 0x1eu,
              "CMD46 request must retain the old 30-byte ABI");

struct stGetItemParam : ParamBase {
    stGetItemParam();
    int error;
};

class DisPatcher_GetItem
    : public IPacketDispatcher<MSG_GET_ITEM,
                               stGetItemParam,
                               static_cast<ch_state>(5)> {
public:
    // IDA_EA: 0x8595188
    DisPatcher_GetItem();
    virtual ~DisPatcher_GetItem();

    // IDA_EA: 0x81C35AC / 0x81C335C / 0x81C35C0 / 0x81C3CE8
    int check_error(CUser* user, MSG_GET_ITEM& message);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_GetItem) == 0x04u,
              "GET_ITEM dispatcher must retain the base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_GROUP_PACKET_DISPATCHER_GET_ITEM_H_
