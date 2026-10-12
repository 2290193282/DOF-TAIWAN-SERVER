#ifndef OLDSRV_OLD_GAME_ECONOMY_INVENTORY_PACKET_DISPATCHER_EXCHANGE_ITEM_FROM_NPC_H_
#define OLDSRV_OLD_GAME_ECONOMY_INVENTORY_PACKET_DISPATCHER_EXCHANGE_ITEM_FROM_NPC_H_

#include <stddef.h>
#include <stdint.h>

#include <vector>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// 0828 CMD718.  CLIENT_IDA_EVIDENCE: sub_5B6B90 writes the selected
// exchange-item NPC id followed by the constant PVF list id one.
#pragma pack(push, 1)
struct MSG_EXCHANGE_ITEM_FROM_NPC : public MSG_BASE {
    MSG_EXCHANGE_ITEM_FROM_NPC();

    uint16_t npc_id;            // +0x0d
    uint16_t exchange_list_id;  // +0x0f; the only client sender writes 1
};
#pragma pack(pop)

static_assert(sizeof(MSG_EXCHANGE_ITEM_FROM_NPC) == 0x11u,
              "CMD718 request must retain its 17-byte wire-backed ABI");
static_assert(offsetof(MSG_EXCHANGE_ITEM_FROM_NPC, npc_id) == 0x0du,
              "CMD718 NPC id must stay at +0x0d");
static_assert(offsetof(MSG_EXCHANGE_ITEM_FROM_NPC, exchange_list_id) == 0x0fu,
              "CMD718 exchange list id must stay at +0x0f");

struct NpcExchangeRewardResult {
    NpcExchangeRewardResult();

    uint32_t item_id;
    uint16_t count;
};

class stExchangeItemFromNpc : public ParamBase {
public:
    stExchangeItemFromNpc();
    virtual ~stExchangeItemFromNpc();

    uint8_t error_code;
    std::vector<NpcExchangeRewardResult> rewards;
};

class Dispatcher_ExchangeItemFromNpc
    : public IPacketDispatcher<MSG_EXCHANGE_ITEM_FROM_NPC,
                               stExchangeItemFromNpc,
                               static_cast<ch_state>(3)> {
public:
    Dispatcher_ExchangeItemFromNpc();
    virtual ~Dispatcher_ExchangeItemFromNpc();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ExchangeItemFromNpc) == 0x04u,
              "CMD718 dispatcher must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_ECONOMY_INVENTORY_PACKET_DISPATCHER_EXCHANGE_ITEM_FROM_NPC_H_
