#ifndef OLDSRV_GLOBAL_PREMIUM_PUBLIC_ABI_H_
#define OLDSRV_GLOBAL_PREMIUM_PUBLIC_ABI_H_

#include "common/config.h"
#include <stdint.h>

#include <map>
#include <string>
#include <vector>

#include "old_data/PVF/equipment/datamanager/premium_item_data.h"
#include "common/manager/play_info_public_abi.h"

struct SPremiumInfo {
    SPremiumInfo();
    ~SPremiumInfo();

    // IDA: g_SPremiumInfo has a 0x120-byte i386 record stride.  The first
    // two dwords are consumed by CUserPremium's lifecycle: the first selects
    // the advantage bucket and the second enables the start/end-time window.
    int advantage_bucket;          // +0x000
    int uses_time_window;          // +0x004
    int expire_time;               // +0x008
    struct ItemReward {
        uint32_t item_id;
        uint32_t item_count;
    } item_rewards[5];             // +0x00c .. +0x033
    uint16_t fatigue_rate;         // +0x034
    uint16_t reserved_036;         // +0x036
    int coin_bonus;                // +0x038
    int exp_bonus;                 // +0x03c
    bool unlimited_fatigue;        // +0x040
    uint8_t reserved_041;          // +0x041
    uint16_t reserved_042;         // +0x042
    // CUserPremium::GetAddExpBonus @0x85BF940 indexes this exact dword.
    int add_exp_bonus;             // +0x044
    // CUserPremium::GetAddQuestItemDropRate @0x8694A28 indexes this dword.
    int add_quest_item_drop_rate;  // +0x048
    std::vector<unsigned int> independent_drop_items;  // +0x04c
    uint16_t over_skill_level;     // +0x058
    uint16_t cera_shop_discount_rate;  // +0x05a
    int apply_target_server;       // +0x05c
    bool gold_bonus;               // +0x060
    uint8_t reserved_061[3];       // +0x061
    int over_equipable_count;      // +0x064
    struct IndexedRate {
        int list_index;
        int rate;
    } over_equipable[5];           // +0x068 .. +0x08f
    int return_item_count;         // +0x090
    IndexedRate return_items[3];   // +0x094 .. +0x0ab
    // The original CUserPremium::_CheckApply reads the seven weekday entries
    // at +0xAC through +0xC4, with -1 in the first entry meaning unrestricted.
    int apply_weekdays[7];         // +0x0ac .. +0x0c4
    _Additioal_info additional_info;
    std::string name;
};

struct SPremiumOverEquipableInfo {
    ~SPremiumOverEquipableInfo();

    uint32_t equipment_type_count;  // +0x00
    uint32_t equipment_types[oldsrv::config::kEquipmentTypeCount];
};

struct SPremiumReturnItemInfo {
    ~SPremiumReturnItemInfo();

    uint32_t item_count;       // +0x00
    uint32_t item_ids[10];     // +0x04..+0x2b
};

struct STPremiumItemData {
    STPremiumItemData();

    uint32_t premium_type;
    uint8_t term;
    uint8_t restrict_flag;
    uint8_t reserved_06[2];
    uint32_t target_item_id;
};

extern SPremiumInfo
    g_SPremiumInfo[oldsrv::config::kPremiumDefinitionTypeCount];
extern std::vector<SPremiumOverEquipableInfo> g_vecPremiumOverEquipableInfo;
extern std::vector<SPremiumReturnItemInfo> g_vecPremiumReturnItemInfo;
extern std::map<unsigned long, STPremiumItemData> g_map_premium_item_data;

static_assert(sizeof(SPremiumInfo::ItemReward) == 0x08u,
              "SPremiumInfo::ItemReward must retain its i386 ABI");
static_assert(sizeof(SPremiumInfo::IndexedRate) == 0x08u,
              "SPremiumInfo::IndexedRate must retain its i386 ABI");
static_assert(offsetof(SPremiumInfo, independent_drop_items) == 0x04cu,
              "SPremiumInfo independent-drop vector must stay at +0x4c");
static_assert(offsetof(SPremiumInfo, over_skill_level) == 0x058u,
              "SPremiumInfo over-skill level must stay at +0x58");
static_assert(offsetof(SPremiumInfo, apply_weekdays) == 0x0acu,
              "SPremiumInfo weekday table must stay at +0xac");
static_assert(offsetof(SPremiumInfo, additional_info) == 0x0c8u,
              "SPremiumInfo additional info must stay at +0xc8");
static_assert(sizeof(SPremiumOverEquipableInfo) == 0x70u &&
                  offsetof(SPremiumOverEquipableInfo, equipment_types) ==
                      0x04u,
              "premium over-equipable list 0828 ABI drift");
static_assert(sizeof(SPremiumReturnItemInfo) == 0x2cu &&
                  offsetof(SPremiumReturnItemInfo, item_ids) == 0x04u,
              "premium return-item list ABI drift");

namespace WongWork {

// C owns the complete gameplay object declaration in
// old_game/userinfo/premium_runtime.h.  This shared B/A-facing header owns only
// the premium-table POD/extern ABI and may forward-declare the runtime class.
class CUserPremium;

}  // namespace WongWork

#endif  // OLDSRV_GLOBAL_PREMIUM_PUBLIC_ABI_H_
