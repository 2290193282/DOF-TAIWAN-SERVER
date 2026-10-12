#ifndef OLDSRV_OLD_GAME_ECONOMY_SHOP_NPC_RELATION_EVENT_MANAGER_H_
#define OLDSRV_OLD_GAME_ECONOMY_SHOP_NPC_RELATION_EVENT_MANAGER_H_

#include <map>
#include <stdint.h>
#include <vector>

#include "old_game/economy/shop/npc_dialog_value_types.h"

struct stTempRewardInfo;

// IDA's favor-kind argument is a four-byte enum.  The three persistent
// relation slots and their PVF tags establish the old values 0..2.
enum ENUM_NPC_RELATION_EVENT_FAVOR_KIND {
    NPC_RELATION_EVENT_FAVOR_GOOD = 0,
    NPC_RELATION_EVENT_FAVOR_SOSO = 1,
    NPC_RELATION_EVENT_FAVOR_BAD = 2,
};

// IDA unit: 07_npc_shop_dialog/stNPCFavorAndRewardInfo.
struct stNPCFavorAndRewardInfo {
    stNPCFavorAndRewardInfo();
    ~stNPCFavorAndRewardInfo();

    std::vector<stNPCEventItemInfo> favor_items;
    int reward_item_id;
    int reward_item_count;
};

// IDA unit: 07_npc_shop_dialog/stNPCRelationEventInfo.
struct stNPCRelationEventInfo {
    stNPCRelationEventInfo();
    ~stNPCRelationEventInfo();

    stNPCFavorAndRewardInfo favor_infos[3];
    std::vector<int> dislike_items;
};

// Canonical C-owned NPC relation state machine.  The original object is the
// one i386 std::map at +0x00; CDataManager's following +0x4D40 pointer is a
// distinct CItemLimitEditionMgr child and must not be absorbed as fake tail.
class CNPCRelationEventManager {
public:
    typedef std::map<uint16_t, stNPCRelationEventInfo*> NPCInfoMap;

    // IDA_EA: 0x890CE20
    CNPCRelationEventManager();
    // IDA_EA: 0x890CE34
    ~CNPCRelationEventManager();

    int AddNPCInfo(unsigned short npc_index, stNPCRelationEventInfo* info);
    int AddFavorItemInfo(unsigned short npc_index,
                         ENUM_NPC_RELATION_EVENT_FAVOR_KIND favor_kind,
                         unsigned int item_id,
                         unsigned int required_count);
    int AddRewardItemInfo(unsigned short npc_index,
                          ENUM_NPC_RELATION_EVENT_FAVOR_KIND favor_kind,
                          unsigned int item_id,
                          unsigned int item_count);
    int AddDislikeItemInfo(unsigned short npc_index, unsigned int item_id);
    int IsDislikeItem(unsigned short npc_index, unsigned int item_id);
    stTempRewardInfo GetItemKind(unsigned short npc_index,
                                 unsigned int item_id,
                                 unsigned int item_count);
    stNPCEventItemInfo GetRewardItem(
        unsigned short npc_index,
        ENUM_NPC_RELATION_EVENT_FAVOR_KIND favor_kind);
    int GetFavorIndex(const char* tag);

    // 140828 Event/NPCRelationEvent.evt adds two independent audit
    // selectors before the legacy NPC records.  They are kept in an
    // ABI-safe sidecar because the original manager is exactly one i386
    // std::map.  Import publishes the policy and NPC records together only
    // after the complete file has passed the strict grammar checks.
    int ReplaceImportedState(
        CNPCRelationEventManager& parsed,
        const std::vector<unsigned short>& gift_npc_log_ids,
        const std::vector<unsigned int>& gift_item_log_ids);
    int IsGiftNpcLogTarget(unsigned short npc_index) const;
    int IsGiftItemLogTarget(unsigned int item_id) const;

    NPCInfoMap npc_infos;
};

static_assert(sizeof(stNPCFavorAndRewardInfo) == 0x14u,
              "stNPCFavorAndRewardInfo IDA ABI size must stay 0x14");
static_assert(sizeof(stNPCRelationEventInfo) == 0x48u,
              "stNPCRelationEventInfo IDA ABI size must stay 0x48");
static_assert(sizeof(CNPCRelationEventManager) == 0x18u,
              "CNPCRelationEventManager is exactly one i386 std::map");

#endif  // OLDSRV_OLD_GAME_ECONOMY_SHOP_NPC_RELATION_EVENT_MANAGER_H_
