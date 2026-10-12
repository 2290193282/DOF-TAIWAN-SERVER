#ifndef OLDSRV_DATAMANAGER_QUEST_SHOP_SCRIPT_H_
#define OLDSRV_DATAMANAGER_QUEST_SHOP_SCRIPT_H_

#include "old_game/economy/shop/npc_dialog_value_types.h"
#include "old_data/PVF/itemshop/quest_shop_status.h"

#include <map>
#include <stdint.h>

struct stQuestShop {
    stQuestShop();
    ~stQuestShop();

    std::map<int, stQuestShopStatus> statuses;
    stQuestShoptInitCost init_cost;
    uint8_t appendage_status_map[77];
    uint8_t max_status_id_alignment[3];
    int32_t max_status_id;
    uint32_t money_limit_per_level[201];
};

#if defined(__i386__)
static_assert(offsetof(stQuestShop, statuses) == 0x00u,
              "stQuestShop status map offset drift");
static_assert(offsetof(stQuestShop, init_cost) == 0x18u,
              "stQuestShop initialization-cost offset drift");
static_assert(offsetof(stQuestShop, appendage_status_map) == 0x24u,
              "stQuestShop appendage-status map offset drift");
static_assert(offsetof(stQuestShop, max_status_id) == 0x74u,
              "stQuestShop maximum-status id offset drift");
static_assert(offsetof(stQuestShop, money_limit_per_level) == 0x78u,
              "stQuestShop money-limit table offset drift");
static_assert(sizeof(stQuestShop) == 0x39cu,
              "stQuestShop i386 ABI size must stay 0x39c");
#endif

int importQuestShopScript(const char* filename, stQuestShop& quest_shop);

#endif  // OLDSRV_DATAMANAGER_QUEST_SHOP_SCRIPT_H_
