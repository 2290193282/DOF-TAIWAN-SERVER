#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_DROP_ITEM_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_DROP_ITEM_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA unit: 13_database_dispatch/DisPatcher_DropItem.  The packed payload
// starts immediately after MSG_BASE's 13-byte protocol header.
#pragma pack(push, 1)
struct MSG_DROP_ITEM : MSG_BASE {
    int16_t pos_x;               // +0x0d
    int16_t pos_y;               // +0x0f
    char item_space;             // +0x11
    int16_t slot;                // +0x12
    int32_t count;               // +0x14
    char security_bypass;        // +0x18
};
#pragma pack(pop)

static_assert(sizeof(MSG_DROP_ITEM) == 0x19u,
              "CMD050 drop-item request must retain its 25-byte ABI");

// IDA unit: 99_misc_global/stDropItemParam. The security path separately
// marks the response at +0x0c so send() selects the original CmdError format.
struct stDropItemParam : ParamBase {
    stDropItemParam() : result_code(0), item_space(0), restriction_error(0) {}
    virtual ~stDropItemParam() {}

    int result_code;          // +0x04
    char item_space;          // +0x08
    char pad_09_0b[3];
    int restriction_error;    // +0x0c
};

#if defined(__i386__)
static_assert(sizeof(stDropItemParam) == 0x10u,
              "CMD050 parameter must retain the original 16-byte ABI");
static_assert(offsetof(stDropItemParam, result_code) == 0x04u,
              "CMD050 result-code offset mismatch");
static_assert(offsetof(stDropItemParam, item_space) == 0x08u,
              "CMD050 item-space offset mismatch");
static_assert(offsetof(stDropItemParam, restriction_error) == 0x0cu,
              "CMD050 restriction-result offset mismatch");
#endif

class DisPatcher_DropItem
    : public IPacketDispatcher<MSG_DROP_ITEM,
                               stDropItemParam,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x859516C
    DisPatcher_DropItem();
    // IDA_EA: 0x8232290 / 0x82322C0
    virtual ~DisPatcher_DropItem();

    // IDA_EA: 0x81C2BEE / 0x81C2F94 / 0x81C3254
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

private:
    // IDA_EA: 0x81C2D9A
    int check_error(CUser* user,
                    const MSG_DROP_ITEM& message,
                    stDropItemParam& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_DropItem) == 0x04u,
              "CMD050 dispatcher must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_DROP_ITEM_H_
