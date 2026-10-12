#ifndef OLDSRV_OLD_GAME_ECONOMY_INVENTORY_PACKET_DISPATCHER_USE_CHESTNUT_STONE_H_
#define OLDSRV_OLD_GAME_ECONOMY_INVENTORY_PACKET_DISPATCHER_USE_CHESTNUT_STONE_H_

#include <stddef.h>
#include <stdint.h>

#include <vector>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// 0828 CMD431.  Both official client senders write only the ordinary
// inventory slot that contains the selected chestnut-stone material.
#pragma pack(push, 1)
struct MSG_USE_CHESTNUT_STONE : public MSG_BASE {
    MSG_USE_CHESTNUT_STONE();

    uint16_t inventory_slot;  // +0x0d
};
#pragma pack(pop)

static_assert(sizeof(MSG_USE_CHESTNUT_STONE) == 0x0fu,
              "CMD431 request must retain its 15-byte wire-backed ABI");
static_assert(offsetof(MSG_USE_CHESTNUT_STONE, inventory_slot) == 0x0du,
              "CMD431 inventory slot must stay at +0x0d");

struct ChestnutStoneRewardResult {
    ChestnutStoneRewardResult();

    uint16_t inventory_slot;
    uint32_t item_id;
    uint32_t final_stack_count;
    uint16_t equipment_extra;
};

class stUseChestnutStone : public ParamBase {
public:
    stUseChestnutStone();
    virtual ~stUseChestnutStone();

    uint8_t error_code;
    uint32_t consumed_item_id;
    std::vector<ChestnutStoneRewardResult> rewards;
};

class Dispatcher_UseChestnutStone
    : public IPacketDispatcher<MSG_USE_CHESTNUT_STONE,
                               stUseChestnutStone,
                               static_cast<ch_state>(3)> {
public:
    Dispatcher_UseChestnutStone();
    virtual ~Dispatcher_UseChestnutStone();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_UseChestnutStone) == 0x04u,
              "CMD431 dispatcher must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_ECONOMY_INVENTORY_PACKET_DISPATCHER_USE_CHESTNUT_STONE_H_
