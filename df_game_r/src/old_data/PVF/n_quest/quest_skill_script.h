#ifndef OLDSRV_OLD_DATA_PVF_N_QUEST_QUEST_SKILL_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_N_QUEST_QUEST_SKILL_SCRIPT_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "common/config.h"
#include "old_game/player/core/character_misc_data.h"
#include "old_game/world/region/world_area_types.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_data/PVF/npc/interactive_dialog_script.h"
#include "infra/concurrent/timer/daily_schedule_manager.h"
#include "common/manager/global_tools.h"

#include "old_data/PVF/skill/skill_script.h"

// IDA unit: 99_misc_global/MonsterKillItem.
// The eight signed dwords and their clear defaults are recovered from
// 0x869383A; this is a real quest parser value, not opaque storage.
struct MonsterKillItem {
    MonsterKillItem();
    ~MonsterKillItem();

    MonsterKillItem* clear();

    int target_type;
    int monster_index;
    int required_count;
    int current_count;
    int map_index;
    int difficulty;
    int value_18;
    int value_1c;
};

// IDA unit: 08_quest_skill_growth/QuestScript exception-quest list.
// The original reader currently emits the urgent exception value (0); the
// enum remains typed so vector element width and parser ownership are explicit.
enum ENUM_QUEST_EXCEPTION_TYPE {
    QUEST_EXCEPTION_URGENT = 0
};

// IDA unit: 08_quest_skill_growth/QuestScript.
// Layout is reconstructed from the constructor/destructor/copy body at
// 0x836DEBA/0x836E4E2/0x836EAF0/0x837ADC8 and checked on the old i386 ABI.
class QuestScript {
public:
    QuestScript();
    QuestScript(const QuestScript& other);
    ~QuestScript();

    QuestScript* clear();
    std::string* getName(ENUM_CHARACTERJOB job);
    std::string* getRequestMessage(ENUM_CHARACTERJOB job);
    std::string* getConditionMessage(ENUM_CHARACTERJOB job);
    std::string* getSolveMessage(ENUM_CHARACTERJOB job);
    bool destroy();

    int quest_index;
    std::string name;
    std::string request_message;
    std::string condition_message;
    std::string solve_message;
    int job_message_enabled;
    std::string* job_names[oldsrv::config::kCharacterJobCount];
    std::string* job_request_messages[oldsrv::config::kCharacterJobCount];
    std::string* job_condition_messages[oldsrv::config::kCharacterJobCount];
    std::string* job_solve_messages[oldsrv::config::kCharacterJobCount];
    std::vector<std::string> substitutive_names;
    int quest_grade;
    int quest_type;
    int value_0dc;
    int value_0e0;
    int value_0e4;
    int value_0e8;
    int value_0ec;
    int value_0f0;
    std::vector<stQuestTargetCharacter_t> target_characters;
    int value_100;
    int value_104;
    bool value_108;
    std::vector<std::vector<int> > conditions;
    std::vector<int> condition_values;
    int value_124;
    int value_128;
    int value_12c;
    bool value_130;
    int value_134;
    std::vector<DungeonClearItem> dungeon_clear_items;
    std::vector<MonsterKillItem> monster_kill_items;
    AppearMap appear_map;
    std::vector<std::pair<int, int> > depend_give_items;
    std::vector<AdditionalDungeonInfo> additional_dungeon_info;
    int value_178;
    int value_17c;
    std::vector<int> limit_dungeon_indices;
    std::vector<std::string> value_18c;
    std::map<int, std::pair<std::string, std::string> > value_198;
    std::vector<int> value_1b0;
    bool value_1bc;
    int value_1c0;
    int value_1c4;
    std::vector<int> value_1c8;
    int value_1d4;
    std::vector<RewardItemInfo> reward_items;
    int value_1e4;
    std::vector<RewardItemInfo> light_reward_items;
    std::vector<InteractiveDialogScript> interactive_dialogs;
    std::vector<dungeonEventHanaseStruct> dungeon_event_dialogs;
    std::vector<int> clear_reward_items;
    int value_218;
    std::vector<int> relation_quest_indices;
    int value_228;
    bool value_22c;
    bool exposed_by_npc;
    bool event;
    int auto_display[3];
    bool has_substitutive_names;
    uint8_t difficulty;
    int gold_multiple;
    std::string mail_sender;
    int mail_value_248;
    int mail_value_24c;
    int mail_value_250;
    int mail_value_254;
    // IDA importQuestScript @0x8A61813 stores the raw ScanInt byte at
    // +0x258; the clear body writes -1 there.  This is a signed sentinel,
    // not a C++ bool (which would collapse -1 to true).
    int8_t mail_time;
    std::string mail_title;
    std::string mail_contents;
    std::vector<int> mail_enclose;
    bool cant_giveup;
    bool searching_information;
    // IDA QuestScript::clear @0x836DEBA writes 0xFF at +0x272. Values 0/1
    // select a PowerWar side rule; 0xFF means this quest has no side rule.
    uint8_t belong_to_winning_power;
    int grow_number;
    bool value_278;
    int online;
    uint16_t quest_point;
    uint16_t quest_piece;
    int go_guide[4];
    std::string go_guide_message;
    PostalReward postal_reward;
    int expert_job_level;
    int expert_job_level_max;
    std::map<int, std::pair<int, int> > change_dungeon_info;
    bool has_change_dungeon_info_message;
    std::string change_dungeon_info_message;
    std::vector<ENUM_QUEST_EXCEPTION_TYPE> exception_quest_types;
    std::vector<_DailySchedule> schedules;
    std::string value_2e8;
    bool value_2ec;
    // 0828 client ImportQuestScript @0x162911c..0x1629266 stores these
    // five tail policies in this exact order: two integer vectors followed
    // by three booleans.  The old reconstruction incorrectly treated
    // [show npc on clear] as a second NPC-index vector.
    std::vector<int> is_not_clear_levelup_system;
    std::vector<int> hide_npc_on_clear;
    bool show_npc_on_clear;
    bool show_click_on_complete_npc;
    bool no_dungeon_name;
};

// IDA_EA: 0x8A6143F
// IDA_NAME: _Z10ScanJobStrPSsPS_
// DEMANGLED: ScanJobStr(std::string *,std::string **)
int ScanJobStr(std::string* output, std::string** job_strings);

// IDA_EA: 0x8A61813
// IDA_NAME: _Z17importQuestScriptP11QuestScriptPKc
// DEMANGLED: importQuestScript(QuestScript *,char const*)
int importQuestScript(QuestScript* script, const char* filename);

// IDA_EA: 0x8A65DA5
// IDA_NAME: _Z15ScanRewardItemsRSt6vectorI14RewardItemInfoSaIS0_EESs
// DEMANGLED: ScanRewardItems(std::vector<RewardItemInfo,...> &,std::string)
bool ScanRewardItems(std::vector<RewardItemInfo>& output,
                     std::string closing_tag);

static_assert(sizeof(MonsterKillItem) == 0x20u,
              "MonsterKillItem IDA ABI size must stay 0x20");
static_assert(offsetof(QuestScript, belong_to_winning_power) == 0x282u,
              "QuestScript 0828 PowerWar byte must stay at offset 0x282");
static_assert(sizeof(QuestScript) == 0x31cu,
              "QuestScript 0828 ABI size must stay 0x31c");

#endif  // OLDSRV_OLD_DATA_PVF_N_QUEST_QUEST_SKILL_SCRIPT_H_
