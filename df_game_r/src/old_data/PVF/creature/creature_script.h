#ifndef DATAMANAGER_CREATURE_SCRIPT_H_
#define DATAMANAGER_CREATURE_SCRIPT_H_

// Canonical B-owned PVF creature-script declarations.

#include <stdint.h>

#include <map>
#include <set>
#include <string>
#include <vector>

struct CreatureAdditionalAnimation {
    CreatureAdditionalAnimation(int index, int category, const char* name);
    CreatureAdditionalAnimation(const CreatureAdditionalAnimation& rhs);
    ~CreatureAdditionalAnimation();
    CreatureAdditionalAnimation& operator=(
        const CreatureAdditionalAnimation& rhs);

    int index;
    int category;
    std::string name;
};

struct st_crature_id_item_id {
    ~st_crature_id_item_id();

    int creature_id;
    int item_id;
};

struct CreatureScriptSkillInfo {
    CreatureScriptSkillInfo();
    CreatureScriptSkillInfo(const CreatureScriptSkillInfo& rhs);
    ~CreatureScriptSkillInfo();
    void clear();

    std::string skill_name;
    std::string skill_explain;
    int skill_recovery_time;
    int learn_skill_level;
    int skill_mp;
    std::string skill_motion;
    std::string skill_under_effect;
    std::string skill_over_effect;
    std::string skill_info;
    std::string attack_info;

    // CLIENT_EA: ImportCreatureScript @0x158ACE0, handler selected by
    // dword_1C5E024 ([preload Passive Index]).  The counted vector belongs to
    // the skill-node value that is copied into the normal/over skill map; it
    // is not a CreatureScript-level side table.
    std::vector<int> preload_passive_indices;
};

struct selectiveGrowCreatureInfo {
    selectiveGrowCreatureInfo();
    selectiveGrowCreatureInfo* clear();

    bool using_exp_item;
    int grow_level;
    int grow_a_type_creature_id;
    int grow_b_type_creature_id;
    int grow_a_type_item_index;
    int grow_b_type_item_index;
    int item_count_for_levelup;
};

struct CreatureSkillBalloonPosition {
    int32_t enabled;
    int32_t x;
    int32_t y;
};

enum CreatureSpecialFeatureType {
    CREATURE_SPECIAL_FEATURE_NONE = 0,
    CREATURE_SPECIAL_FEATURE_TOWN_UPGRADE_FEEDBACK = 1,
    CREATURE_SPECIAL_FEATURE_CHANGE_MOTION = 2,
    CREATURE_SPECIAL_FEATURE_SPIN_AROUND = 3
};

struct CreatureTownUpgradeFeedbackMotion {
    std::string interaction_type;
    std::string base_animation;
    std::string working_animation;
    std::string great_success_animation;
    std::string success_animation;
    std::string fail_animation;
    std::string big_fail_animation;
};

struct CreatureMotionChangeFeature {
    std::string sub_directory;
    int32_t height;
};

struct CreatureSpinAroundFeature {
    int32_t duration_ms;
    int32_t spin_type;
    std::string apply_target;
};

struct CreatureSpecialFeature {
    CreatureSpecialFeatureType type;
    CreatureTownUpgradeFeedbackMotion town_upgrade_feedback;
    CreatureMotionChangeFeature motion_change;
    CreatureSpinAroundFeature spin_around;
};

// IDA_EA: 0x83415E2, 0x889037C, 0x88904B8, 0x8890760,
// 0x889079A, 0x88907FE, 0x88908B8, 0x88908C6.
struct CatchObjectInfo {
    int32_t object_type;
    int32_t object_count;
    float probability;
};

enum ENUM_ELEMENT : int {
    ENUM_ELEMENT_NONE = 4
};

struct AttackInfo {
    enum ENUM_ATTACKTYPE {
        ATTACK_TYPE_PHYSIC = 0,
        ATTACK_TYPE_MAGIC = 1,
        ATTACK_TYPE_IGNORE_DEFENSE = 2
    };

    AttackInfo();
    ~AttackInfo();
    AttackInfo* clear();
    bool isIntendDamage();
    void setElement(ENUM_ELEMENT element);
    bool isValidElement(ENUM_ELEMENT element);
    AttackInfo* setAttackType(ENUM_ATTACKTYPE attack_type);
    static int getAttackType(std::string& name);

    float damage;
    float damage_increase_rate;
    int32_t absolute_damage;
    int32_t damage_bonus;
    int32_t attack_type;
    int32_t weapon_damage_apply;
    float human_active_status_rate;
    int32_t damage_reaction;
    int32_t push_aside;
    int32_t lift_up;
    float critical_hit;
    float stuck;
    int32_t stuck_probability_increase;
    int32_t knock_back;
    float knock_back_rate;
    float weakness_rate;
    float weakness_time;
    int32_t bounce;
    int32_t ignore_defense;
    std::string hit_wav;
    int32_t attack_direction;
    int32_t hit_type;
    uint8_t show_blood;
    int32_t hit_stun_time;
    float hit_stun_rate;
    uint8_t use_weapon_hit_delay_rate;
    int32_t hit_direction;
    int32_t ignore_super_armor;
    int32_t ignore_weight;
    int32_t force_hit_stun_time;
    int32_t hp_drain;
    int32_t mp_drain;
    int32_t unused_80;
    float unused_84;
    uint8_t is_hit_number_increasing;
    int32_t module_type;
    uint8_t ignore_diehard;
    uint8_t hit_check_by_screen_pos;
    int32_t max_target_hit_number;
    int32_t attack_myself;
    int32_t attack_friend;
    int32_t attack_enemy;
    std::set<int> elements;
    int32_t active_status_type[3];
    float active_status_rate[3];
    int32_t active_status_time[3];
    int32_t unused_e0[3];
    int32_t active_status_value[3];
    std::vector<int32_t> active_status_data[3];
    float damage_rate[3];
    std::vector<CatchObjectInfo> catch_objects;
};

int getActiveStatusType(const std::string& name);
int ImportAttackInfoScript(AttackInfo* info, const char* filename, int module_type);

struct CreatureScript {
    CreatureScript();
    ~CreatureScript();
    int clear();

    static int32_t ExpTable_[99];

    int index;
    std::string name;
    int width;
    int height;
    int floating_height;
    int draw_layer;
    int character_span;
    int skill_recovery_time;
    int over_skill_recovery_time;
    bool direction_correct;
    int move_speed[5];
    int gravity;
    int start_level;
    int permission_level;
    int max_level;
    int parent_max_level;
    int artifact_slot[3];
    int learn_overskill_level;
    int skill_mp;
    int over_skill_mp;
    int skill_type;
    int over_skill_type;
    std::string basic_motion;
    std::string walk_motion;
    std::string run_motion;
    std::string fly_motion;
    std::string get_exp_motion;
    std::string get_exp_over_motion;
    std::string get_exp_under_motion;
    std::string skill_motion;
    std::string over_skill_motion;
    std::string response_motion;
    std::string special_motion;
    std::string equip_motion;
    std::string equip_over_motion;
    std::string equip_under_motion;
    std::string basic_over_effect;
    std::string walk_over_effect;
    std::string run_over_effect;
    std::string unused_motion_over_effect;
    std::string skill_over_effect;
    std::string over_skill_over_effect;
    std::string response_over_effect;
    std::string special_over_effect;
    std::string basic_under_effect;
    std::string walk_under_effect;
    std::string run_under_effect;
    std::string unused_motion_under_effect;
    std::string skill_under_effect;
    std::string over_skill_under_effect;
    std::string response_under_effect;
    std::string special_under_effect;
    std::vector<CreatureAdditionalAnimation> additional_animations;
    std::string default_particle;
    std::string response_sound_tag;
    std::string wing_sound_tag;
    std::string walk_sound_tag;
    std::string run_sound_tag;
    int revision_x;
    int revision_y;
    std::vector<std::string> etc_motions;
    std::vector<std::string> attack_infos;
    std::vector<std::string> skill_strings;
    std::vector<int> skill_level_values;
    std::vector<int> over_skill_level_values;
    std::vector<int> piercing;
    std::vector<int> int_data;
    std::vector<std::string> string_data;
    std::string script_path;
    int evolution_quest;
    int evolution_creature_id;
    int evolution_level;
    std::vector<int> evolution_creature_ids;
    selectiveGrowCreatureInfo selective_grow_info;
    std::vector<std::string> random_motions;
    int default_random_motion_rate;
    int increase_random_motion_rate;
    int evolution_item_id;
    std::vector<st_crature_id_item_id> item_evolution_creature_ids;
    std::string skill_info;
    std::string over_skill_info;
    std::map<int, CreatureScriptSkillInfo> skill_infos;
    std::map<int, CreatureScriptSkillInfo> over_skill_infos;
    bool using_random_skill;
    std::vector<int> event_evolute_creatures;
    // Native PVF extension. Keep the restored 0x214-byte prefix unchanged so
    // old server consumers retain their proven field offsets.
    CreatureSkillBalloonPosition skill_balloon_position;
    CreatureSkillBalloonPosition over_skill_balloon_position;
    int32_t upgrade_probability_increase;
    CreatureSpecialFeature special_feature;

};

int importCreatureScript(CreatureScript* script, const char* filename);

#include "old_game/appearance/pet/creature_cmd372_abi.h"

namespace user_creature {

class CCreatureScriptMgr {
public:
    CCreatureScriptMgr();
    ~CCreatureScriptMgr();
    int LoadCreatureScriptFile();
    int Destroy();
    CCreatureScript* FindCreatureScript(int item_id);
    int RegisterCreatureIdAndItemId(int creature_id, int item_id);

private:
    std::map<int, int> creature_id_to_item_id;
    std::map<int, CCreatureScript*> item_id_to_script;
};

CCreatureScriptMgr* GetInstanceCreatureScriptMgr();

}  // namespace user_creature

static_assert(sizeof(CreatureAdditionalAnimation) == 0x0Cu,
              "CreatureAdditionalAnimation IDA ABI size must stay 0x0C");
static_assert(sizeof(st_crature_id_item_id) == 0x08u,
              "st_crature_id_item_id IDA ABI size must stay 0x08");
static_assert(sizeof(CreatureScriptSkillInfo) == 0x34u,
              "0828 CreatureScriptSkillInfo must include preload vector");
static_assert(sizeof(selectiveGrowCreatureInfo) == 0x1Cu,
              "selectiveGrowCreatureInfo IDA ABI size must stay 0x1C");
static_assert(sizeof(CreatureSkillBalloonPosition) == 0x0Cu,
              "CreatureSkillBalloonPosition must keep three 32-bit values");
static_assert(sizeof(CatchObjectInfo) == 0x0Cu,
              "CatchObjectInfo IDA ABI size must stay 0x0C");
static_assert(sizeof(AttackInfo) == 0x134u,
              "AttackInfo IDA ABI size must stay 0x134");
static_assert(sizeof(CreatureScript) == 0x264u,
              "CreatureScript 0828 native PVF ABI size must stay 0x264");
static_assert(sizeof(user_creature::CCreatureScriptMgr) == 0x30u,
              "CCreatureScriptMgr IDA ABI size must stay 0x30");

#endif  // DATAMANAGER_CREATURE_SCRIPT_H_
