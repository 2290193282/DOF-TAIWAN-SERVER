#ifndef OLDSRV_DATAMANAGER_EQUIPMENT_SCRIPT_H_
#define OLDSRV_DATAMANAGER_EQUIPMENT_SCRIPT_H_

#include <stddef.h>
#include <stdint.h>

#include <set>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "common/config.h"
#include "old_game/economy/inventory/avatar_type_select_runtime.h"
#include "old_game/progression/quest/quest_skill_value_types.h"
#include "common/manager/change_status_data.h"
#include "old_game/economy/equipment/enchant_system_data.h"
#include "old_game/economy/equipment/equipment_ani_script.h"
#include "common/manager/striker_public_abi.h"

enum ENUM_CHARACTERJOB : int;

namespace ItemUpgradeType {
enum T : int;
}

// CLIENT_EA: GetGrowAvatarTypeByName @0x14DFB60.
// These values identify class-specific appearance parts hidden by an equipped
// avatar.  Value 37 is the client sentinel for an unrecognized PVF name.
enum ENUM_GROW_AVATAR_TYPE {
    GROW_AVATAR_WEAPON_MASTER_GLOVES = 0,
    GROW_AVATAR_SOUL_BRINGER_AURA = 1,
    GROW_AVATAR_BERSERKER_RED_EYE = 2,
    GROW_AVATAR_ASURA_RED_EYE_PATCH = 3,
    GROW_AVATAR_DEMONIC_SWORDMAN_BLACK_HAND = 4,
    GROW_AVATAR_NEN_MASTER_AURA = 5,
    GROW_AVATAR_STRIKER_WRIST_WRAP = 6,
    GROW_AVATAR_STREET_FIGHTER_BAND = 7,
    GROW_AVATAR_GRAPPLER_HAIR_BAND = 8,
    GROW_AVATAR_RANGER_CIGARETTE = 9,
    GROW_AVATAR_LAUNCHER_HEAVY_WEAPON = 10,
    GROW_AVATAR_MECHANIC_ROBOT = 11,
    GROW_AVATAR_SPITFIRE_MAGAZINE = 12,
    GROW_AVATAR_RANGER_NECKLACE = 13,
    GROW_AVATAR_CRUSADER_AURA = 14,
    GROW_AVATAR_INFIGHTER_AURA = 15,
    GROW_AVATAR_EXORCIST_GHOST = 16,
    GROW_AVATAR_AVENGER_AURA = 17,
    GROW_AVATAR_ELEMENTAL_MASTER_AURA = 18,
    GROW_AVATAR_SUMMONER_BOOK = 19,
    GROW_AVATAR_SUMMONER_FLOOR_MARK = 20,
    GROW_AVATAR_BATTLE_MAGE_GLOVES = 21,
    GROW_AVATAR_WITCH_AID_KIT = 22,
    GROW_AVATAR_CREATOR_COORDINATION = 23,
    GROW_AVATAR_ROGUE_WHITE_EYE = 24,
    GROW_AVATAR_NECROMANCER_AURA = 25,
    GROW_AVATAR_ELEMENTAL_BOMBER_ELEMENTAL = 26,
    GROW_AVATAR_GLACIAL_MASTER_BREATH = 27,
    GROW_AVATAR_SWORD_MASTER_LIGHT = 28,
    GROW_AVATAR_DEMON_SLAYER_AURA = 29,
    GROW_AVATAR_JIANSHEN_BLUE_EYE = 30,
    GROW_AVATAR_JIANSHEN_SWORD = 31,
    GROW_AVATAR_DARKLORD_AURA = 32,
    GROW_AVATAR_BLOOD_EVIL_INJURY = 33,
    GROW_AVATAR_BLOOD_EVIL_BLOOD = 34,
    GROW_AVATAR_INDARATIAN_TATTOO = 35,
    GROW_AVATAR_INDARATIAN_POWER = 36,
    GROW_AVATAR_TYPE_INVALID = 37
};

struct ItemAuraInfo {
    int32_t active_status_type;
    uint8_t use_percent;
    uint8_t reserved_05;
    uint8_t reserved_06;
    uint8_t reserved_07;
    float value;
    int32_t duration;
};

struct STChangeStatusRate {
    ~STChangeStatusRate();

    int32_t active_status_type;
    float values_04[4];
};

struct Position {
    Position();

    int32_t x;
    int32_t y;
    int32_t z;
};

struct WpBonusPointData {
    WpBonusPointData();
    WpBonusPointData* clear();

    int32_t type;
    int32_t value;
};

struct ExpBonusPointData {
    ExpBonusPointData();
    ExpBonusPointData* clear();

    int32_t type;
    int32_t value;
};

struct stSetItemBackImageInfo {
    ~stSetItemBackImageInfo();

    int32_t values_00[3];
};

namespace effect {
struct STEffect {
    uint8_t attach_target;
    uint8_t type;
    uint8_t attach_position;
    uint8_t reserved_03;
    int32_t z_revision;
    int32_t index;
    uint8_t options;
    uint8_t reserved_0d;
    uint8_t reserved_0e;
    uint8_t reserved_0f;
    int32_t module_type;
    std::string effect_name;

    STEffect();
    STEffect(const STEffect& other);
    ~STEffect();

    STEffect& operator=(const STEffect& other);
    STEffect* clear();
};
}  // namespace effect

struct STKeyCommandUnit {
    std::vector<int> keys;
    int32_t command;
    int32_t interval;

    STKeyCommandUnit();
    STKeyCommandUnit(const STKeyCommandUnit& rhs);
    ~STKeyCommandUnit();
    STKeyCommandUnit& operator=(const STKeyCommandUnit& rhs);
    STKeyCommandUnit* clear();
};

struct STKeyCommand {
    std::vector<STKeyCommandUnit> key_commands;
    std::vector<int> command_states;
    int32_t type;
    int32_t value_1c;
    uint8_t enabled;
    uint8_t reserved_21;
    uint8_t reserved_22;
    uint8_t reserved_23;

    STKeyCommand();
    ~STKeyCommand();
    STKeyCommand* clear();
};

union EquipmentParameterValue {
    int32_t integer;
    float real;
    char bytes[4];
};

struct STAvatarAbilityVariation {
    // IDA_EA: 0x89C2D82
    // IDA_NAME: _ZN24STAvatarAbilityVariationC2Ev
    // DEMANGLED: STAvatarAbilityVariation::STAvatarAbilityVariation(void)
    STAvatarAbilityVariation();

    // IDA_EA: 0x89D2AF0
    // IDA_NAME: _ZN24STAvatarAbilityVariationD2Ev
    // DEMANGLED: STAvatarAbilityVariation::~STAvatarAbilityVariation()
    ~STAvatarAbilityVariation();

    // IDA_EA: 0x89C2DA4
    // IDA_NAME: _ZN24STAvatarAbilityVariation5clearEv
    // DEMANGLED: STAvatarAbilityVariation::clear(void)
    LevelUpSkill* clear();

    int32_t ability_index;
    int32_t sign_type;
    float value;
    LevelUpSkill level_up_skill;
};

static_assert(sizeof(STAvatarAbilityVariation) == 0x18u,
              "STAvatarAbilityVariation IDA ABI size must stay 0x18");

struct EquipmentParameterInfo {
    enum ENUM_SKILL_UP_VECTOR_INDEX {
        SKILL_UP_VECTOR_INDEX_0 = 0,
        SKILL_UP_VECTOR_INDEX_1 = 1
    };

    struct EquipmentAuraAppendageData {
        ~EquipmentAuraAppendageData();

        int32_t type;
        int32_t value_04;
        int32_t value_08;
        int32_t value_0c;
        int32_t value_10;
    };

    struct SASPROPERTY {
        int32_t item_growtype;
        int32_t item_id;
        int32_t skill_group;
        int32_t lower_bound_level;
        int32_t upper_bound_level;
        int32_t value;
        int32_t extra_condition;
        int32_t skill_phase;
        std::vector<float> values;
        std::string text;

        SASPROPERTY();
        SASPROPERTY(const SASPROPERTY& rhs);
        ~SASPROPERTY();
        SASPROPERTY& operator=(const SASPROPERTY& rhs);
    };

    struct RebirthAppendageinfo {
        RebirthAppendageinfo();
        RebirthAppendageinfo* clear();

        int32_t probability;
        int32_t consume_item;
        int32_t consume_item_count;
        int32_t hp_recovery;
        int32_t mp_recovery;
        int32_t cooltime;
    };

    EquipmentParameterInfo();
    EquipmentParameterInfo(const EquipmentParameterInfo& rhs);
    ~EquipmentParameterInfo();
    EquipmentParameterInfo& operator=(const EquipmentParameterInfo& rhs);
    EquipmentParameterInfo* clear();
    char* getEquipmentPhysicalAttack(int index);
    char* getEquipmentPhysicalDefense(int index);
    char* getEquipmentMagicalAttack(int index);
    char* getEquipmentMagicalDefense(int index);
    int addString(std::string value);

    EquipmentParameterValue values_000[80];
    EquipmentParameterValue values_140[4];
    uint8_t value_150;
    uint8_t reserved_151;
    uint8_t reserved_152;
    uint8_t reserved_153;
    EquipmentParameterValue value_154;
    std::vector<int> values_158;
    EquipmentParameterValue value_164;
    EquipmentParameterValue value_168;
    EquipmentParameterValue value_16c;
    EquipmentParameterValue value_170;
    uint8_t value_174;
    uint8_t reserved_175;
    uint8_t reserved_176;
    uint8_t reserved_177;
    EquipmentParameterValue value_178;
    std::vector<int> values_17c;
    std::vector<ItemAuraInfo> aura_infos_188;
    std::vector<int> values_194;
    EquipmentParameterValue values_1a0[6];
    std::vector<EquipmentSkillDataUp> skill_data_up_1b8;
    std::map<int, std::string> strings_1c4;
    std::vector<EquipmentSkillDataUp> skill_data_up_1dc;
    std::vector<std::vector<EquipmentSkillDataUp> > skill_data_up_1e8;
    std::vector<std::string> strings_1f4;
    std::vector<EquipmentAuraAppendageData> aura_appendages_200;
    EquipmentParameterValue value_20c;
    EquipmentParameterValue value_210;
    std::vector<int> hardcoding_parameters_214;
    EquipmentParameterValue values_220[3];
    uint8_t value_22c;
    uint8_t reserved_22d;
    uint8_t reserved_22e;
    uint8_t reserved_22f;
    EquipmentParameterValue value_230;
    EquipmentParameterValue value_234;
    EquipmentParameterValue value_238;
    uint8_t value_23c;
    uint8_t value_23d;
    uint16_t reserved_23e;
    std::vector<std::vector<float> > recovery_statuses_240;
    std::vector<LevelUpSkill> skill_levelups_24c;
    std::vector<LevelUpSkill> skill_levelups_258;
    std::vector<STAvatarAbilityVariation> avatar_select_abilities_264;
    EquipmentParameterValue value_270;
    EquipmentParameterValue value_274;
    std::string value_278;
    std::string value_27c;
    EquipmentParameterValue values_280[102];
    std::vector<SASPROPERTY> all_skill_items_418;
    uint8_t item_overpower_enabled_424;
    uint8_t item_overpower_ready_425;
    uint16_t reserved_426;
    std::vector<std::vector<float> > item_overpower_stats_428;
    EquipmentParameterValue values_434[3];
    uint8_t value_440;
    uint8_t reserved_441;
    uint8_t reserved_442;
    uint8_t reserved_443;
    EquipmentParameterValue values_444[3];
    RebirthAppendageinfo rebirth_450;
    std::vector<std::pair<int, int> > conditional_values_468;
    std::vector<VariableStat> variable_stats_474;
};

// 0828 extends EquipmentParameterInfo beyond the fixed 0x480-byte server
// prefix.  Keep the restored ABI stable and own the new fields in a typed
// sidecar keyed by the parameter object.  Copy/clear/destruction hooks in the
// implementation keep the sidecar lifetime identical to the owning object.
namespace equipment_parameter_0828 {

struct SkillCooltimeSetItemRate {
    SkillCooltimeSetItemRate();

    int32_t lower_bound_level;
    int32_t upper_bound_level;
    float cooltime_rate;
};

struct Extension {
    Extension();

    bool has_skill_cooltime_set_item_rate;
    bool not_accumulate;
    SkillCooltimeSetItemRate skill_cooltime_set_item_rate;
};

bool Find(const EquipmentParameterInfo* parameter, Extension* extension);

}  // namespace equipment_parameter_0828

// IDA unit: 12_pvf_datamanager_scripts/STItemScript.  This is the shared
// i386 layout used as the base of STEquipmentScript and STStackableScript.
// Every non-trivial member offset is confirmed by STItemScript C2/D2/aS.
struct STSpecialMonsterDrop {
    STSpecialMonsterDrop();
    void* clear();

    uint8_t enabled;
    uint8_t extra_drop_easy;
    uint8_t extra_drop_medium;
    uint8_t extra_drop_hard;
    uint8_t extra_drop_ultimate;
    uint8_t extra_drop_hero;
    uint8_t dungeon_normal;
    uint8_t dungeon_ancient;
    uint8_t dungeon_dimension;
};

struct DyeInfo {
    DyeInfo();
    ~DyeInfo();
    DyeInfo& operator=(const DyeInfo& rhs);
    int clear();
    int getDyeColorIndex();

    int32_t type;
    std::vector<std::pair<int, int> > colors;
};

struct STItemScript {
    enum ENUM_ITEM_CATEGORY {
        ITEM_CATEGORY_NONE = 0,
        ITEM_CATEGORY_MONSTER_CARD = 1,
        ITEM_CATEGORY_CANNOT_BIND = 2,
        ITEM_CATEGORY_BOSS_DROP = 3,
        ITEM_CATEGORY_EXPERT_JOB = 4,
        ITEM_CATEGORY_RECIPE = 5,
        ITEM_CATEGORY_LEGACY = 7
    };

    struct SEXPERTJOB {
        struct stExpertCompoundResultVariation {
            stExpertCompoundResultVariation();

            float value_00;
            float value_04;
        };

        struct stExpertJobAdditionalExp {
            stExpertJobAdditionalExp();

            float value_00;
            int32_t value_04;
        };

        struct SUSABLE {
            ~SUSABLE();

            int32_t type;
            int32_t value;
        };

        SEXPERTJOB();
        ~SEXPERTJOB();
        SEXPERTJOB& operator=(const SEXPERTJOB& rhs);

        stExpertCompoundResultVariation result_variation;
        stExpertJobAdditionalExp additional_exp;
        float disjoint_big_win_rate;
        float disjoint_result_variation;
        float material_variation;
        float compound_rate;
        std::vector<SUSABLE> usable;
    };

    STItemScript();
    ~STItemScript();
    STItemScript& operator=(const STItemScript& rhs);
    STItemScript* Clear();
    bool hasCategory(ENUM_ITEM_CATEGORY category) const;

    int32_t index;
    int32_t grade;
    int32_t values_08[7];
    int32_t value_24;
    int32_t value_28;
    int32_t value_2c;
    uint8_t value_30;
    uint8_t value_31;
    uint16_t value_32;
    int32_t value_34;
    uint8_t usable_job_flags[oldsrv::config::kCharacterJobCount];
    std::set<int> usable_jobs;
    int32_t values_5c[4];
    std::string icon;
    int32_t icon_index;
    std::string field_image;
    int32_t field_image_index;
    std::string tarotcard_view;
    int32_t tarotcard_view_index;
    std::string name;
    std::vector<int> int_data;
    std::vector<std::string> string_data;
    uint8_t value_a0;
    uint8_t value_a1;
    uint8_t value_a2;
    uint8_t value_a3;
    int32_t values_a4[2];
    std::string strings_ac[6];
    int32_t values_c4[3];
    uint8_t value_d0;
    uint8_t value_d1;
    uint8_t value_d2;
    uint8_t value_d3;
    int32_t value_d4;
    uint8_t value_d8;
    uint8_t value_d9;
    uint8_t value_da;
    uint8_t value_db;
    std::set<int> usable_worlds;
    int32_t values_f4[2];
    uint8_t value_fc;
    uint8_t value_fd;
    uint8_t value_fe;
    uint8_t value_ff;
    int32_t value_100;
    uint8_t value_104;
    STSpecialMonsterDrop special_monster_drop;
    uint8_t value_10e;
    uint8_t value_10f;
    std::vector<std::pair<int, int> > special_drop_items;
    std::set<int> item_categories;
    uint16_t usable_upgrade_character_level;
    uint16_t value_136;
    SEXPERTJOB expert_job;
    DyeInfo dye_info;
    std::string value_174;
};

// The 0828 client stores the valueless [event reward item] marker in the
// enlarged common item prefix shared by equipment and stackable scripts.  A
// common sidecar preserves the fixed 0627 STItemScript layout and gives both
// importers one authoritative owner.
namespace item_script_0828 {

void SetEventRewardItem(const STItemScript* item, bool enabled);
bool IsEventRewardItem(const STItemScript* item);
// Promote the script marker to the immutable item-prototype registry.  The
// registry is keyed by the PVF item index because the fixed 0627 CItem base
// cannot grow without shifting both derived prototype ABIs.
void RegisterEventRewardPrototype(int32_t item_index,
                                  const STItemScript* item);
bool IsEventRewardPrototype(int32_t item_index);

}  // namespace item_script_0828

static_assert(sizeof(STSpecialMonsterDrop) == 0x09u,
               "STSpecialMonsterDrop IDA ABI size must stay 0x09");
static_assert(sizeof(ItemAuraInfo) == 0x10u,
              "ItemAuraInfo IDA ABI size must stay 0x10");
static_assert(offsetof(ItemAuraInfo, value) == 0x08u,
              "ItemAuraInfo value offset must stay 0x08");
static_assert(sizeof(STChangeStatusRate) == 0x14u,
              "STChangeStatusRate IDA ABI size must stay 0x14");
static_assert(sizeof(Position) == 0x0Cu,
              "Position IDA ABI size must stay 0x0C");
static_assert(sizeof(WpBonusPointData) == 0x08u,
              "WpBonusPointData IDA ABI size must stay 0x08");
static_assert(sizeof(ExpBonusPointData) == 0x08u,
              "ExpBonusPointData IDA ABI size must stay 0x08");
static_assert(sizeof(stSetItemBackImageInfo) == 0x0Cu,
              "stSetItemBackImageInfo IDA ABI size must stay 0x0C");
static_assert(sizeof(effect::STEffect) == 0x18u,
              "effect::STEffect IDA ABI size must stay 0x18");
static_assert(sizeof(STKeyCommand) == 0x24u,
              "STKeyCommand IDA ABI size must stay 0x24");
static_assert(sizeof(EquipmentParameterValue) == 0x04u,
              "EquipmentParameterValue must stay a 32-bit old ABI word");
static_assert(sizeof(EquipmentParameterInfo::EquipmentAuraAppendageData) ==
                  0x14u,
              "EquipmentAuraAppendageData IDA ABI size must stay 0x14");
static_assert(sizeof(EquipmentParameterInfo::SASPROPERTY) == 0x30u,
              "SASPROPERTY IDA ABI size must stay 0x30");
static_assert(sizeof(EquipmentParameterInfo::RebirthAppendageinfo) == 0x18u,
              "RebirthAppendageinfo IDA ABI size must stay 0x18");
static_assert(sizeof(EquipmentParameterInfo) == 0x480u,
              "EquipmentParameterInfo IDA ABI size must stay 0x480");
static_assert(offsetof(EquipmentParameterInfo, values_158) == 0x158u,
              "EquipmentParameterInfo values_158 offset must stay 0x158");
static_assert(offsetof(EquipmentParameterInfo, skill_data_up_1b8) == 0x1B8u,
              "EquipmentParameterInfo skill_data_up_1b8 offset must stay 0x1B8");
static_assert(offsetof(EquipmentParameterInfo, recovery_statuses_240) ==
                  0x240u,
              "EquipmentParameterInfo recovery_statuses_240 offset must stay 0x240");
static_assert(offsetof(EquipmentParameterInfo, all_skill_items_418) == 0x418u,
              "EquipmentParameterInfo all_skill_items_418 offset must stay 0x418");
static_assert(offsetof(EquipmentParameterInfo, rebirth_450) == 0x450u,
              "EquipmentParameterInfo rebirth_450 offset must stay 0x450");
static_assert(offsetof(EquipmentParameterInfo, values_444) == 0x444u,
              "EquipmentParameterInfo values_444 offset must stay 0x444");
static_assert(offsetof(EquipmentParameterInfo, variable_stats_474) == 0x474u,
              "EquipmentParameterInfo variable_stats_474 offset must stay 0x474");
static_assert(sizeof(DyeInfo) == 0x10u,
              "DyeInfo IDA ABI size must stay 0x10");
static_assert(sizeof(STItemScript::SEXPERTJOB) == 0x2Cu,
              "STItemScript::SEXPERTJOB IDA ABI size must stay 0x2C");
static_assert(sizeof(STItemScript) == 0x178u,
               "STItemScript IDA ABI size must stay 0x178");
static_assert(offsetof(STItemScript, usable_jobs) == 0x44u,
              "STItemScript usable_jobs offset must stay 0x44");
static_assert(offsetof(STItemScript, icon) == 0x6Cu,
              "STItemScript icon offset must stay 0x6C");
static_assert(offsetof(STItemScript, usable_worlds) == 0xDCu,
              "STItemScript usable_worlds offset must stay 0xDC");
static_assert(offsetof(STItemScript, special_drop_items) == 0x110u,
              "STItemScript special_drop_items offset must stay 0x110");
static_assert(offsetof(STItemScript, item_categories) == 0x11Cu,
              "STItemScript item_categories offset must stay 0x11C");
static_assert(offsetof(STItemScript, expert_job) == 0x138u,
              "STItemScript expert_job offset must stay 0x138");
static_assert(offsetof(STItemScript, dye_info) == 0x164u,
              "STItemScript dye_info offset must stay 0x164");

struct STEquipmentScript {
    static int priceTable_[3000];

    struct artifact_info {
        artifact_info();
        int isUsable() const;

        int32_t usable_type;
        int32_t grade;
        int32_t rarity;
        float physical_attack;
        float magical_attack;
        float physical_defense;
        float magical_defense;
        float strength;
        float vitality;
        float spirit;
        int32_t item_index;
    };

    STEquipmentScript();
    ~STEquipmentScript();
    int Clear();
    EquipmentAniInfoScript* addAniInfo(ENUM_CHARACTERJOB job);
    EquipmentAniInfoScript* getAniInfo(ENUM_CHARACTERJOB job);
    bool isUpgradableLevel(ItemUpgradeType::T upgrade_type, int level);
    bool isExistUpgradableLevel();

    STItemScript item;
    int32_t values_178[5];
    std::vector<std::string> strings_18c;
    std::vector<int> values_198;
    std::string string_1a4;
    std::vector<std::string> strings_1a8;
    int32_t values_1b4[3];
    EquipmentParameterInfo parameter_1c0;
    std::map<ENUM_CHARACTERJOB, EquipmentAniInfoScript> animation_infos_640;
    std::vector<std::string> strings_658;
    std::string directory_664;
    std::string set_name_668;
    std::vector<int> values_66c;
    std::map<int, EquipmentParameterInfo> set_parameters_678;
    std::vector<EquipmentParameterInfo> level_parameters_690;
    uint8_t value_69c;
    uint8_t reserved_69d;
    uint8_t reserved_69e;
    uint8_t reserved_69f;
    int32_t value_6a0;
    std::string strings_6a4[6];
    int32_t value_6bc;
    std::vector<stAvatarTypeSelect_t> avatar_type_select_6c0;
    std::vector<unsigned short> avatar_emblems_6cc;
    int32_t value_6d8;
    int32_t value_6dc;
    int32_t value_6e0;
    int32_t value_6e4;
    std::vector<STChangeStatusRate> change_status_rates_6e8;
    uint8_t impossible_upgrade_6f4;
    uint8_t impossible_amplify_6f5;
    uint8_t impossible_disjoint_6f6;
    uint8_t reserved_6f7;
    int32_t value_6f8;
    std::map<ItemUpgradeType::T, std::pair<int, int> > upgradable_levels_6fc;
    std::set<int> usable_modules_714;
    uint8_t value_72c;
    uint8_t value_72d;
    uint16_t reserved_72e;
    int32_t value_730;
    int32_t value_734;
    artifact_info artifact_738;
    int32_t value_764;
    std::vector<int> values_768;
    std::vector<int> values_774;
    std::vector<int> values_780;
    uint8_t value_78c;
    uint8_t reserved_78d;
    uint8_t reserved_78e;
    uint8_t reserved_78f;
    int32_t value_790;
    int32_t value_794;
    int32_t value_798;
    int32_t value_79c;
    std::vector<int> values_7a0;
    uint8_t clear_avatar_7ac;
    uint8_t reserved_7ad;
    uint8_t reserved_7ae;
    uint8_t reserved_7af;
    std::vector<std::string> strings_7b0;
    uint8_t value_7bc;
    uint8_t reserved_7bd;
    uint8_t reserved_7be;
    uint8_t reserved_7bf;
    int32_t value_7c0;
    int32_t value_7c4;
    uint8_t value_7c8;
    uint8_t reserved_7c9;
    uint8_t reserved_7ca;
    uint8_t reserved_7cb;
    std::vector<STEnchantSystemData> enchant_data_7cc;
    std::vector<std::pair<int, int> > pairs_7d8;
    std::vector<std::pair<int, int> > pairs_7e4;
    std::string string_7f0;
    std::string string_7f4;
    Position position_7f8;
    STKeyCommand* key_command_804;
    int32_t value_808;
    uint16_t character_name_x_80c;
    uint16_t character_name_y_80e;
    int32_t value_810;
    uint8_t value_814;
    uint8_t reserved_815;
    uint8_t reserved_816;
    uint8_t reserved_817;
    int32_t value_818;
    WpBonusPointData wp_bonus_81c;
    ExpBonusPointData exp_bonus_824;
    uint8_t value_82c;
    uint8_t reserved_82d;
    uint8_t reserved_82e;
    uint8_t reserved_82f;
    std::vector<int> values_830;
    std::string string_83c;
    std::vector<stSetItemBackImageInfo> back_images_840;
    std::vector<effect::STEffect> effects_84c;
    std::vector<std::string> strings_858;
    std::vector<bool> values_864;
    std::string string_878;
    int32_t value_87c;
    uint8_t value_880;
    uint8_t reserved_881;
    uint8_t reserved_882;
    uint8_t reserved_883;
    int32_t value_884;
    uint8_t value_888;
    uint8_t value_889;
    uint8_t value_88a;
    uint8_t value_88b;
    int32_t value_88c;
    int32_t value_890;
    // Native PVF classification flag for the valueless [full avatar] tag.
    // USERINFO derives the emitted Equipment.lst item id from this flag.
    uint8_t full_avatar_flag;
    // Client importer evidence: ImportEquipmentScript @0x1572840 parses
    // [rental service] as an integer category in the closed range 1..3.
    // All 559 target-PVF uses are equipment scripts, so the upgraded server
    // keeps the field in the appended equipment extension without shifting
    // any restored 0627 prefix member.
    uint8_t rental_service_type;
    // CLIENT_EA: ImportEquipmentScript handler @0x157510C. The valueless
    // [skin emblem socket] tag enables emblem sockets on a skin avatar.
    uint8_t skin_emblem_socket_enabled;
    uint8_t reserved_897;
    // CLIENT_EA: ImportEquipmentScript parser @0x157B032, client storage
    // STEquipmentScript+0x1210. This is separate from numeric
    // [hide growtype avatar] data at values_780.
    std::vector<ENUM_GROW_AVATAR_TYPE> hidden_grow_avatar_types;
    // CLIENT_EA: ImportEquipmentScript_ReadDictionaryText @0x1574290.
    std::string dictionary_text;
    // CLIENT_EA: ImportEquipmentScript_ReadUpgradeLimitTableName @0x157464C.
    std::string upgrade_limit_table_name;
    // CLIENT_EA: ImportEquipmentScript_SetEquipmentGradeTextId @0x157B0F1.
    int32_t equipment_grade_text_id;
    // CLIENT_EA: ImportEquipmentScript_ReadSeedCorrectionIndex @0x1579C86.
    int32_t seed_correction_index;
    // CLIENT_EA: ImportEquipmentScript_ReadEquipmentGrowthTypeAndStage
    // @0x1579B89. Both values are truncated to one byte by the client.
    uint8_t equipment_growth_type;
    uint8_t equipment_growth_stage;
    uint16_t reserved_8b6;
    // CLIENT_EA: ImportEquipmentScript_ReadSpectrumAvatarParameters
    // @0x1574404. The first four values form little-endian RGBA. Client
    // runtime consumers copy the remaining values into the character model,
    // but their narrower visual meaning is not yet proven.
    uint32_t spectrum_avatar_rgba;
    int32_t spectrum_avatar_parameter_5;
    int32_t spectrum_avatar_parameter_6;
    // CLIENT_EA: EquipmentParameterInfo_ImportWeaponOverspecPerItem
    // @0x1554470 stores these on the enlarged client parameter object. All
    // 424 target-PVF blocks are in the main equipment parameter scope, so the
    // restored server keeps them in its appended equipment extension without
    // shifting the embedded 0627 EquipmentParameterInfo prefix.
    std::vector<std::vector<float> > weapon_overspec_stats;
    std::set<int> weapon_overspec_item_categories;

    // CLIENT_EA: ImportEquipmentScript @0x1576B27.  [aura pos datas] is
    // presentation-only, but it is a structured record rather than an
    // ignorable scalar tail.  Preserve its exact parsed shape so later tags
    // cannot be hidden by ScanType mismatch recovery.
    struct AuraPositionData {
        AuraPositionData();

        int32_t character_job;
        int32_t x;
        int32_t y;
        int32_t variant;
        uint8_t value_count;
        uint8_t reserved_11;
        uint16_t reserved_12;
    };

    int32_t aura_position_mode;
    std::vector<AuraPositionData> aura_position_data;
};

static_assert(sizeof(STEquipmentScript::artifact_info) == 0x2Cu,
              "STEquipmentScript::artifact_info IDA ABI size must stay 0x2C");

static_assert(offsetof(STEquipmentScript, full_avatar_flag) == 0x894u,
              "STEquipmentScript full-avatar flag must stay at +0x894");
static_assert(offsetof(STEquipmentScript, rental_service_type) == 0x895u,
              "STEquipmentScript rental-service type must stay at +0x895");
static_assert(offsetof(STEquipmentScript, skin_emblem_socket_enabled) ==
                  0x896u,
              "STEquipmentScript skin-emblem flag must stay at +0x896");
static_assert(offsetof(STEquipmentScript, hidden_grow_avatar_types) == 0x898u,
              "STEquipmentScript hidden grow-avatar list must stay at +0x898");
static_assert(offsetof(STEquipmentScript, dictionary_text) == 0x8A4u,
              "STEquipmentScript dictionary text must stay at +0x8A4");
static_assert(offsetof(STEquipmentScript, upgrade_limit_table_name) == 0x8A8u,
              "STEquipmentScript upgrade-limit table must stay at +0x8A8");
static_assert(offsetof(STEquipmentScript, equipment_grade_text_id) == 0x8ACu,
              "STEquipmentScript grade text id must stay at +0x8AC");
static_assert(offsetof(STEquipmentScript, seed_correction_index) == 0x8B0u,
              "STEquipmentScript seed-correction index must stay at +0x8B0");
static_assert(offsetof(STEquipmentScript, equipment_growth_type) == 0x8B4u,
              "STEquipmentScript growth type must stay at +0x8B4");
static_assert(offsetof(STEquipmentScript, equipment_growth_stage) == 0x8B5u,
              "STEquipmentScript growth stage must stay at +0x8B5");
static_assert(offsetof(STEquipmentScript, spectrum_avatar_rgba) == 0x8B8u,
              "STEquipmentScript spectrum RGBA must stay at +0x8B8");
static_assert(offsetof(STEquipmentScript, spectrum_avatar_parameter_5) ==
                  0x8BCu,
              "STEquipmentScript spectrum parameter 5 must stay at +0x8BC");
static_assert(offsetof(STEquipmentScript, spectrum_avatar_parameter_6) ==
                  0x8C0u,
              "STEquipmentScript spectrum parameter 6 must stay at +0x8C0");
static_assert(offsetof(STEquipmentScript, weapon_overspec_stats) == 0x8C4u,
              "STEquipmentScript weapon overspec stats must stay at +0x8C4");
static_assert(offsetof(STEquipmentScript, weapon_overspec_item_categories) ==
                  0x8D0u,
              "STEquipmentScript weapon overspec categories must stay at +0x8D0");
static_assert(sizeof(STEquipmentScript::AuraPositionData) == 0x14u,
              "aura-position record must stay five 32-bit words");
static_assert(offsetof(STEquipmentScript, aura_position_mode) == 0x8E8u,
              "STEquipmentScript aura-position mode must stay at +0x8E8");
static_assert(offsetof(STEquipmentScript, aura_position_data) == 0x8ECu,
              "STEquipmentScript aura-position data must stay at +0x8EC");
static_assert(sizeof(STEquipmentScript) == 0x8F8u,
              "STEquipmentScript semantic extension must stay 0x8F8 bytes");
static_assert(offsetof(STEquipmentScript, parameter_1c0) == 0x1C0u,
              "STEquipmentScript parameter offset must stay 0x1C0");
static_assert(offsetof(STEquipmentScript, animation_infos_640) == 0x640u,
              "STEquipmentScript animation-info offset must stay 0x640");
static_assert(offsetof(STEquipmentScript, set_parameters_678) == 0x678u,
              "STEquipmentScript set-parameter offset must stay 0x678");
static_assert(offsetof(STEquipmentScript, level_parameters_690) == 0x690u,
              "STEquipmentScript level-parameter offset must stay 0x690");
static_assert(offsetof(STEquipmentScript, key_command_804) == 0x804u,
              "STEquipmentScript key-command offset must stay 0x804");
static_assert(offsetof(STEquipmentScript, value_808) == 0x808u,
              "STEquipmentScript module-result offset must stay 0x808");
static_assert(offsetof(STEquipmentScript, effects_84c) == 0x84Cu,
              "STEquipmentScript effect offset must stay 0x84C");

int importCommand(STKeyCommand* command);
int isEndModuleTag(std::string* tag, int module_index);

// IDA_EA: 0x8990F66
// IDA_NAME: _Z19getAvatarEmblemTypePKc
// DEMANGLED: getAvatarEmblemType(char const*)
int getAvatarEmblemType(const char* text);

// IDA_EA: 0x8990F1F
// IDA_NAME: _Z16GetEquipmentTypePKc
// DEMANGLED: GetEquipmentType(char const*)
int GetEquipmentType(const char* text);

// CLIENT_EA: 0x14DFB60
// CLIENT_NAME: GetGrowAvatarTypeByName
ENUM_GROW_AVATAR_TYPE GetGrowAvatarTypeByName(const char* text);

// CLIENT_EA: 0x154DFF0
// CLIENT_NAME: GetEquipmentGradeTextIdByName
int GetEquipmentGradeTextIdByName(const char* text);

// CLIENT_EA: 0x154CBE0
// CLIENT_NAME: ReadItemCategoryList
int ReadItemCategoryList(std::set<int>* categories);

// IDA_EA: 0x89A75EA
// IDA_NAME: _Z28importEquipmentParameterInfoP22EquipmentParameterInfoRSsPKcRiS4_
// DEMANGLED: importEquipmentParameterInfo(EquipmentParameterInfo *,std::string &,char const*,int &,int &)
int importEquipmentParameterInfo(EquipmentParameterInfo* info,
                                 std::string& tag,
                                 const char* filename,
                                 int& first,
                                 int& second);

// IDA_EA: 0x8991229
// IDA_NAME: _Z21GetApplyStatusInRangePSt6vectorIfSaIfEE
int GetApplyStatusInRange(std::vector<float>* values);

// IDA_EA: 0x8991384
// IDA_NAME: _Z18GetStatByConditionPSt6vectorIfSaIfEE
int GetStatByCondition(std::vector<float>* values);

// IDA_EA: 0x89915D4
// IDA_NAME: _Z7GetStatPSt6vectorIfSaIfEE
int GetStat(std::vector<float>* values);

// IDA_EA: 0x8991086
// IDA_NAME: _Z15GetSkillLevelUpPSt6vectorIfSaIfEE
bool GetSkillLevelUp(std::vector<float>* values);

// IDA_EA: 0x899CD6C
// IDA_NAME: _Z6scanIfP22EquipmentParameterInfoi
void scanIf(EquipmentParameterInfo* info, int unused);

// IDA_EA: 0x899CE3B
// IDA_NAME: _Z12readCondtionP22EquipmentParameterInfoRSsRSt6vectorIiSaIiEE
void readCondtion(EquipmentParameterInfo* info,
                  std::string& tag,
                  std::vector<int>& values);

// IDA_EA: 0x89A123C
// IDA_NAME: _Z8scanThenSsP22EquipmentParameterInfoi
int scanThen(std::string filename, EquipmentParameterInfo* info, int unused);

// IDA_EA: 0x89A13E5
// IDA_NAME: _Z10readActionRKSsP22EquipmentParameterInfoRSsRSt6vectorIfSaIfEEi
int readAction(const std::string& filename,
               EquipmentParameterInfo* info,
               std::string& tag,
               std::vector<float>& values,
               int unused);

// IDA_EA: 0x89BF67F
// IDA_NAME: _Z21scanRandomOptionValueP22EquipmentParameterInfoRSs
int scanRandomOptionValue(EquipmentParameterInfo* info, std::string& tag);

// IDA_EA: 0x89BD079
// IDA_NAME: _Z18importArtifactInfoRN17STEquipmentScript13artifact_infoESs
// DEMANGLED: importArtifactInfo(STEquipmentScript::artifact_info &,std::string)
int importArtifactInfo(STEquipmentScript::artifact_info& info,
                       std::string tag);

// IDA_EA: 0x8991AEB
// IDA_NAME: _Z21ImportEquipmentScriptP17STEquipmentScriptPKcbii
// DEMANGLED: ImportEquipmentScript(STEquipmentScript *,char const*,bool,int,int)
int ImportEquipmentScript(STEquipmentScript* script,
                          const char* filename,
                          bool unknown,
                          int module_type,
                          int set_id);

int EquipmentScriptSpecialMonsterDrop(STEquipmentScript* script,
                                      std::string tag,
                                      const char* filename);

// IDA_EA: 0x8A97637
// IDA_NAME: _Z19processModuleTagSetRiS_PSsib
// DEMANGLED: processModuleTagSet(int &,int &,std::string *,int,bool)
int processModuleTagSet(int& tag_index,
                        int& parsed_module_type,
                        std::string* tag,
                        int module_type,
                        bool parse_equipment);

#endif  // OLDSRV_DATAMANAGER_EQUIPMENT_SCRIPT_H_
