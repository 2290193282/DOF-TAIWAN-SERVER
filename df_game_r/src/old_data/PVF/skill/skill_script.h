#ifndef OLDSRV_OLD_DATA_PVF_SKILL_SKILL_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_SKILL_SKILL_SCRIPT_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "old_game/progression/quest/quest_skill_value_types.h"
#include "old_data/PVF/equipment/datamanager/equipment_script.h"

// IDA unit: 08_quest_skill_growth/SkillLevelFeature.
struct SkillLevelFeature {
    SkillLevelFeature();
    SkillLevelFeature(const SkillLevelFeature& other);
    ~SkillLevelFeature();

    SkillLevelFeature* clear();
    SkillLevelFeature& operator=(const SkillLevelFeature& other);

    int min_level;
    int max_level;
    std::string property_name;
    std::vector<std::pair<int, int> > property_ranges;
    std::vector<float> property_values;
};

// IDA unit: 99_misc_global/STPassiveFeatureData.
struct STPassiveFeatureData {
    STPassiveFeatureData();
    ~STPassiveFeatureData();

    STPassiveFeatureData* clear();

    bool applies_to_all;
    int skill_index;
    int operation;
    int value;
};

// 0828-only [fixed level skill] metadata is kept in an external registry so
// the recovered STSkillScript/CSkill i386 layouts remain byte-exact.
struct FixedLevelSkillPolicy {
    int interval_level;
    int add_level_per_interval;
    bool apply_contract_skillmaster;
};

bool GetFixedLevelSkillPolicy(int character_job,
                              int skill_index,
                              FixedLevelSkillPolicy* policy);

// CLIENT_EA: DNF.exe ImportSkillScript @0x15FCD70.  Both
// [add static data] and [add level info] are streams of the same 5-field
// record.  The fourth field is stored by the client as a bool; its higher
// level selection semantics are not named in the recovered binary, so keep
// the neutral source_variant name instead of inventing a gameplay meaning.
enum SkillAddDataOperation0828 {
    SKILL_ADD_DATA_PLUS_0828 = 0,
    SKILL_ADD_DATA_PLUS_PERCENT_0828 = 1
};

struct SkillAddDataRule0828 {
    int target_data_index;
    SkillAddDataOperation0828 operation;
    int source_skill_index;
    bool source_variant;
    int source_data_index;
};

struct SkillAddDataExtension0828 {
    std::vector<SkillAddDataRule0828> static_data_rules;
    std::vector<SkillAddDataRule0828> level_info_rules;
};

void ResetSkillAddDataRegistry0828();
bool GetSkillAddDataExtension0828(int character_job,
                                  int skill_index,
                                  int module_type,
                                  SkillAddDataExtension0828* extension);
bool EvaluateSkillAddDataRule0828(
    const SkillAddDataRule0828& rule,
    const std::vector<std::vector<float> >& source_level_info,
    int source_skill_level,
    float base_value,
    float* result);

// IDA CSkillList::init @0x8350C26 reads the 0828 second-grow block as six
// 0x10-byte rows: fitness at row +0/+4 and maximum level at row +8/+12.
// Keeping the row typed prevents the two pairs from being flattened into the
// wrong all-fitness-then-all-maximum order.
struct STSecondGrowTypeMatrixRow {
    int fitness[2];
    int maximum_level[2];
};

// IDA unit: 08_quest_skill_growth/STSkillScript.
// The concrete field sequence below is proven by ctor/dtor at
// 0x8375642/0x837A0FA and ImportSkillScript's typed RDAR writes.
class STSkillScript {
public:
    STSkillScript();
    ~STSkillScript();

    STSkillScript* Clear();

    int skill_index;
    std::string name;
    std::string name2;
    std::string explain;
    std::string basic_explain;
    int maximum_level;
    std::vector<int> purchase_cost;
    std::vector<int> special_purchase_cost;
    int required_level;
    int required_level_range;
    int required_rank;
    int required_guild_level;
    int required_guild_level_range;
    std::vector<int> pre_required_skill_indices;
    std::vector<int> pre_required_skill_levels;
    int type;
    int skill_class;
    int growtype_fitness[6];
    int growtype_maximum_level[6];
    STSecondGrowTypeMatrixRow second_growtype[6];
    std::vector<int> skill_fitness_second_growtype;
    float consume_mp[2];
    float maintain_mp[2];
    int casting_time[2];
    int durability_decrease_rate;
    std::vector<SkillUseItem> consume_items;
    std::vector<int> limit_count;
    int auto_limit_count_reduce;
    int cool_time[2];
    std::vector<int> skill_under_cooltime_effect;
    std::vector<int> skill_under_cooltime_effect_each;
    int auto_cooltime_apply;
    int weapon_effect_type;
    std::string icon_name;
    std::string icon_sprite;
    int icon_x;
    int icon_y;
    STKeyCommand command;
    std::string command_key_explain;
    std::vector<int> static_data;
    int static_data_type;
    std::vector<std::vector<float> > cooltime_level_info;
    // The old parser writes [level property] at +0x1b4 and [basic property]
    // at +0x1c0.  Their distinct vectors must stay in this physical order.
    std::vector<SkillLevelFeature> level_properties;
    std::vector<SkillLevelFeature> basic_properties;
    int value_1cc;
    int value_1d0;
    bool value_1d4;
    bool value_1d5;
    int physical_magical_change;
    std::vector<int> skill_fitness_growtype;
    bool steel_learning_enabled;
    int steel_learning_skill;
    int steel_learning_value;
    int steel_learning_value_2;
    std::vector<std::string> preloading_images;
    std::vector<int> purchase_gsp;
    std::vector<int> purchase_gold;
    int start_cool_time;
    int start_cool_time_value;
    int module_type;
    bool module_tag_enabled[17];
    int league_ban;
    int skill_command_advantage;
    int skill_command_advantage_value;
    int is_linked_blast;
    int linked_blast_add_point;
    int linked_blast_dec_point;
    int value_254;
    int feature_skill_index;
    int child_skill_index;
    bool feature_skill_type;
    int online;
    std::vector<STPassiveFeatureData> special_level_up;
    std::string explain_ex;
    std::string basic_explain_ex;
    std::string nut_skill_script;
    bool command_customizing;
    std::vector<int> executable_states;
    bool combo_set_enabled;
    std::vector<int> combo_set;
    std::vector<int> value_2a0;
    // CLIENT_EA: ImportSkillScript @0x15FCD70.
    // [usable passiveobject] is stored as a repeated integer list at the
    // target client's STSkillScript+0x5A8; it is not a consume-only token.
    std::vector<int> usable_passiveobject_ids;
    // The valueless [ignore active status] marker and its optional state list
    // are independent 0828 fields.  Keeping both prevents the Mage skill
    // header from terminating before its learning requirements are imported.
    bool ignore_active_status;
    std::vector<int> ignored_active_status_states;
};

// IDA_EA: 0x8A9792F
// IDA_NAME: _Z17ImportSkillScriptP13STSkillScriptPKc17ENUM_CHARACTERJOBii
// DEMANGLED: ImportSkillScript(STSkillScript *,char const*,ENUM_CHARACTERJOB,int,int)
int ImportSkillScript(STSkillScript* script,
                      const char* filename,
                      ENUM_CHARACTERJOB character_job,
                      int module_type,
                      int script_index);

static_assert(sizeof(SkillLevelFeature) == 0x24u,
              "SkillLevelFeature IDA ABI size must stay 0x24");
static_assert(sizeof(STPassiveFeatureData) == 0x10u,
              "STPassiveFeatureData IDA ABI size must stay 0x10");
static_assert(sizeof(STSecondGrowTypeMatrixRow) == 0x10u,
              "0828 second-grow matrix row must stay 0x10");
static_assert(offsetof(STSecondGrowTypeMatrixRow, fitness) == 0x00u,
              "0828 second-grow fitness pair must stay at row +0");
static_assert(offsetof(STSecondGrowTypeMatrixRow, maximum_level) == 0x08u,
              "0828 second-grow maximum pair must stay at row +8");
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(offsetof(STSkillScript, growtype_fitness) == 0x64u,
              "0828 first-grow fitness must stay at +0x64");
static_assert(offsetof(STSkillScript, growtype_maximum_level) == 0x7cu,
              "0828 first-grow maximum must stay at +0x7c");
static_assert(offsetof(STSkillScript, second_growtype) == 0x94u,
              "0828 second-grow row matrix must stay at +0x94");
static_assert(offsetof(STSkillScript, skill_fitness_second_growtype) == 0xf4u,
              "0828 global second-grow fitness vector must stay at +0xf4");
#endif
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(STSkillScript) == 0x2c8u,
              "STSkillScript 0828 extension size must stay 0x2c8");
#endif

#endif  // OLDSRV_OLD_DATA_PVF_SKILL_SKILL_SCRIPT_H_
