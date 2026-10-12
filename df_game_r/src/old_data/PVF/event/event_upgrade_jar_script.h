#ifndef OLDSRV_DATAMANAGER_EVENT_UPGRADE_JAR_SCRIPT_H_
#define OLDSRV_DATAMANAGER_EVENT_UPGRADE_JAR_SCRIPT_H_

#include <stdint.h>
#include <stddef.h>

#include <map>
#include <utility>
#include <vector>

#include "old_data/PVF/equipment/upgrade_table.h"
#include "old_game/economy/inventory/inventory.h"

// IDA_EA: 0x8968EC2, 0x8969E90.
struct ST_Event_Upgrade_Jar_EventNPCs_Script {
    ST_Event_Upgrade_Jar_EventNPCs_Script();
    ~ST_Event_Upgrade_Jar_EventNPCs_Script();

    int fail_probability;
    int material_count;
};

// IDA_EA: 0x80F50A4, 0x80F51D2.  Loaded from
// Etc/EventUpgradeJar.etc during CDataManager::init.
struct EventUpgradeJarInfo {
    EventUpgradeJarInfo();
    ~EventUpgradeJarInfo();

    uint16_t group_count;
    std::map<unsigned long, short> group_to_booster;
    std::map<std::pair<int, int>, unsigned long> booster_pair_to_index;
    std::vector<unsigned long> fortune_indexes;
    std::vector<unsigned int> fortune_fail_rates[4];
    float new_npc_discount_rate;
    std::vector<ST_Event_Upgrade_Jar_EventNPCs_Script> event_npc_scripts;
    int upgrade_max;
    int event_mode;
    uint8_t event_upgrade_index;
};

// IDA_EA: 0x80F49B8 through 0x854B590.
struct Event_Upgrade_Jar : public EventUpgradeJarInfo {
    Event_Upgrade_Jar();
    ~Event_Upgrade_Jar();

    bool LoadScript(const char* dir, const char* filename);
    int getBoosterIndex_byUpgradeCardIndex(unsigned long& first_card_index,
                                           unsigned long& second_card_index);
    bool check_requiredRecipe(const Inven_Item& item) const;
    int _getFailProb_Fortune(int item_index, unsigned int upgrade_index) const;
    int _getFailProb_NewNPC(unsigned int script_index) const;
    int _getMeterialCount_NewNPC(unsigned int script_index);
    long double _getDiscountRate_NewNPC() const;
    char CheckMaterialIndex(unsigned long item_index);
    Event_Upgrade_Jar* SettingRecipeNUpConunt(int item_index,
                                              upgrade_info_t& upgrade_info,
                                              uint8_t upgrade_index);
    int GetProbability(int item_index, unsigned int& probability) const;
    bool CheckMaxLuckyLevel(Inven_Item& item, unsigned long item_index);
    int GetIndexLuckyStrengtheningDeed();
    bool isFortune() const;
    bool isNewNPC() const;
};

int importEventUpgradeJarScript(EventUpgradeJarInfo* info,
                                const char* filename);
int updateUpgradeTableFromEventUpgradeJarScript(
    upgrade_table_t* table,
    EventUpgradeJarInfo* info);

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(ST_Event_Upgrade_Jar_EventNPCs_Script) == 0x08u,
              "event-upgrade NPC row IDA ABI size must stay 0x08");
static_assert(sizeof(EventUpgradeJarInfo) == 0x8Cu,
              "EventUpgradeJarInfo IDA ABI size must stay 0x8C");
static_assert(offsetof(EventUpgradeJarInfo, fortune_indexes) == 0x34u,
              "EventUpgradeJarInfo::fortune_indexes must remain at +0x34");
static_assert(offsetof(EventUpgradeJarInfo, fortune_fail_rates) == 0x40u,
              "EventUpgradeJarInfo::fortune_fail_rates must remain at +0x40");
static_assert(offsetof(EventUpgradeJarInfo, new_npc_discount_rate) == 0x70u,
              "EventUpgradeJarInfo::new_npc_discount_rate must remain at +0x70");
static_assert(offsetof(EventUpgradeJarInfo, event_npc_scripts) == 0x74u,
              "EventUpgradeJarInfo::event_npc_scripts must remain at +0x74");
static_assert(sizeof(Event_Upgrade_Jar) == 0x8Cu,
              "Event_Upgrade_Jar IDA ABI size must stay 0x8C");
#endif

#endif  // OLDSRV_DATAMANAGER_EVENT_UPGRADE_JAR_SCRIPT_H_
