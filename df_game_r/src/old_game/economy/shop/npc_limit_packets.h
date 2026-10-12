#ifndef OLDSRV_07_NPC_SHOP_DIALOG_NPC_LIMIT_PACKETS_H_
#define OLDSRV_07_NPC_SHOP_DIALOG_NPC_LIMIT_PACKETS_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x84B41FE in the companion source.


#include <stdint.h>

#include "old_game/economy/shop/npc_dialog_value_types.h"
#include "infra/network/packet/PacketHeader.h"


#pragma pack(push, 1)

// IDA unit: 07_npc_shop_dialog/LimitNpcBuyItemRequestInfo.
struct LimitNpcBuyItemRequestInfo : public PacketHeader {
    LimitNpcBuyItemRequestInfo();
};

// IDA unit: 07_npc_shop_dialog/LimitNpcBuyItemInfo.
struct LimitNpcBuyItemInfo : public PacketHeader {
    LimitNpcBuyItemInfo();

    uint32_t user_id;       // +0x0a
    uint32_t account_id;    // +0x0e
    uint32_t charac_no;     // +0x12
    uint32_t item_id;       // +0x16
    uint32_t item_count;    // +0x1a
    uint32_t response_tag;  // +0x1e
    uint32_t error_code;    // +0x22
};

// IDA unit: 07_npc_shop_dialog/LimitNpcBuyItemInfoAll.
struct LimitNpcBuyItemInfoAll : public PacketHeader {
    LimitNpcBuyItemInfoAll();

    uint32_t user_id;       // +0x0a, CUser::GetUID()
    uint32_t account_id;    // +0x0e
    uint32_t charac_no;     // +0x12
    uint32_t item_count;    // +0x16
    NpcBuyLimitItem items[30];
};

// IDA unit: 07_npc_shop_dialog/LimitNpcBuyItemUpdate.
struct LimitNpcBuyItemUpdate : public PacketHeader {
    LimitNpcBuyItemUpdate();

    uint32_t item_id;       // +0x0a
    uint32_t charac_no;     // +0x0e
    uint32_t response_tag;  // +0x12
    uint32_t result;        // +0x16
};

#pragma pack(pop)

static_assert(sizeof(LimitNpcBuyItemInfoAll) == 0x182u,
              "all NPC-limit rows packet size changed");
static_assert(offsetof(LimitNpcBuyItemInfoAll, user_id) == 0x0au &&
                  offsetof(LimitNpcBuyItemInfoAll, account_id) == 0x0eu &&
                  offsetof(LimitNpcBuyItemInfoAll, charac_no) == 0x12u &&
                  offsetof(LimitNpcBuyItemInfoAll, item_count) == 0x16u &&
                  offsetof(LimitNpcBuyItemInfoAll, items) == 0x1au,
              "all NPC-limit rows fields moved");
static_assert(sizeof(LimitNpcBuyItemInfo) == 0x26u &&
                  offsetof(LimitNpcBuyItemInfo, user_id) == 0x0au &&
                  offsetof(LimitNpcBuyItemInfo, item_id) == 0x16u &&
                  offsetof(LimitNpcBuyItemInfo, response_tag) == 0x1eu &&
                  offsetof(LimitNpcBuyItemInfo, error_code) == 0x22u,
              "NPC-limit item response fields moved");
static_assert(sizeof(LimitNpcBuyItemUpdate) == 0x1au &&
                  offsetof(LimitNpcBuyItemUpdate, item_id) == 0x0au &&
                  offsetof(LimitNpcBuyItemUpdate, result) == 0x16u,
              "NPC-limit item update fields moved");


#endif  // OLDSRV_07_NPC_SHOP_DIALOG_NPC_LIMIT_PACKETS_H_
