#ifndef OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_MOVE_ITEM_H_
#define OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_MOVE_ITEM_H_

#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 13_database_dispatch/DisPatcher_MoveItem (CMD19).
// The request carries both source and destination identities.  The four-byte
// field at +0x1f is read by the legacy server but not consumed by its move
// transaction; retain it so the wire object keeps its exact 37-byte extent.
struct __attribute__((packed)) MSG_MOVE_ITEMSPACE : MSG_BASE {
    char source_space;                 // +0x0d
    int16_t source_slot;               // +0x0e
    uint32_t source_item_id;           // +0x10
    int32_t source_add_info;           // +0x14
    char destination_space;            // +0x18
    int16_t destination_slot;          // +0x19
    uint32_t destination_item_id;      // +0x1b
    int32_t target_add_info;           // +0x1f
    char channel_equip_switch;         // +0x23
    char expand_equip_switch;          // +0x24
};

static_assert(sizeof(MSG_MOVE_ITEMSPACE) == 0x25u,
              "CMD19 request must retain the old 37-byte ABI");

// IDA_NAME: stMoveItemParam.  The primary dispatcher template constructs this
// result for every accepted state-3 request.  Its C2 initializes every named
// scalar to zero; send still retains the original INT_MAX suppression branch.
struct stMoveItemParam : ParamBase {
    stMoveItemParam();

    int error;                         // ParamBase +0x04
    char source_space;                 // ParamBase +0x08
    char padding_09;
    int16_t source_slot;               // ParamBase +0x0a
    int32_t source_add_info;           // ParamBase +0x0c
    char destination_space;            // ParamBase +0x10
    char padding_11[3];
    int actual_destination_slot;       // ParamBase +0x14
};

#if defined(__i386__)
static_assert(sizeof(stMoveItemParam) == 0x18u,
              "stMoveItemParam must retain the old 24-byte ABI");
#endif

class DisPatcher_MoveItem
    : public IPacketDispatcher<MSG_MOVE_ITEMSPACE,
                               stMoveItemParam,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x85951DC
    // IDA_NAME: _ZN19DisPatcher_MoveItemC2Ev
    DisPatcher_MoveItem();

    // IDA_EA: 0x8232410 / 0x8232440
    // IDA_NAME: _ZN19DisPatcher_MoveItemD2Ev / D0Ev
    virtual ~DisPatcher_MoveItem();

    // IDA_EA: 0x81C57A0
    int check_error(CUser* user, MSG_BASE& message);

    // IDA_EA: 0x81C54E0 / 0x81C5904 / 0x81C5B76
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_MoveItem) == 0x04u,
              "CMD19 dispatcher must retain the base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_MOVE_ITEM_H_
