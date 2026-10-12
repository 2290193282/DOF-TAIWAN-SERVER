#ifndef OLDSRV_DATAMANAGER_AI_CHARACTER_SCRIPT_H_
#define OLDSRV_DATAMANAGER_AI_CHARACTER_SCRIPT_H_

#include <stdint.h>

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "old_data/PVF/common/script_initializers.h"

#include "old_data/PVF/aicharacter/datamanager/monster_script.h"
#include "old_data/PVF/aicharacter/st_ai_info_script.h"
#include "common/manager/striker_public_abi.h"
#include "old_game/player/core/character_status_data.h"

class STScriptFileList;

// IDA_EA: 0x88CC190, 0x88CC3E2, 0x88CC3F6.
struct MinimumInfo {
    MinimumInfo();
    ~MinimumInfo();

    MinimumInfo* clear();

    std::string name;
    int32_t level;
};

// IDA_EA: 0x88CC328, 0x88CC390, 0x88CC3B2, 0x88CC40A,
// 0x88CC450, 0x88D06C6.
struct APCSpeech {
    APCSpeech();
    APCSpeech(const APCSpeech& other);
    ~APCSpeech();

    APCSpeech& operator=(const APCSpeech& other);
    APCSpeech* clear();
    int add(std::string& text, int value);

    std::vector<std::string> texts;
    std::vector<int32_t> values;
};

// IDA_EA: 0x8367EEE, 0x83680F4.
struct ActiveActionFileSet {
    ActiveActionFileSet();
    ~ActiveActionFileSet();

    std::string waiting;
    std::string move;
    std::string dash;
    std::string sit;
    std::string damage_1;
    std::string damage_2;
    std::string down;
    std::string overturn;
    std::string jump;
    std::string jump_attack;
    std::vector<std::string> attack_actions;
    std::vector<std::string> etc_actions;
};

// IDA_EA: 0x88CEAF2.
struct STAICharacterEquipmentData {
    ~STAICharacterEquipmentData();

    int32_t item_index;
    int32_t option_1;
    int32_t option_2;
};

// IDA_EA: 0x88CED32.
struct STAICharacterSkillData {
    ~STAICharacterSkillData();

    int32_t skill_index;
    int32_t skill_level;
};

// IDA_EA: 0x88CEF72.
struct STAICharacterItemData {
    ~STAICharacterItemData();

    int32_t item_index;
    int32_t item_count;
};

// IDA_EA: 0x88CC30E, 0x88CF1B4.
struct STAICharacterSkillCorrection {
    STAICharacterSkillCorrection();
    ~STAICharacterSkillCorrection();

    int32_t skill_index;
    float correction;
};

// IDA_EA: 0x88CF8CA.
struct STAICharacterCustomSkillLevel {
    ~STAICharacterCustomSkillLevel();

    int32_t skill_index;
    int32_t level_1;
    int32_t level_2;
    int32_t level_3;
};

struct STAICharacterReviveOption {
    std::vector<std::string> map_conditions;
    std::string active_object_state;
    int32_t revive_value;
};

struct STAICharacterRecoveryOption {
    std::vector<std::string> map_conditions;
    std::string active_object_state;
    int32_t hp_recovery_value;
};

// IDA_EA: 0x83D31D2, 0x83DC9A0, 0x83E0980, 0x89E85FE, 0x89E8676.
// Parsed by the Map/PVF loader and consumed by the map runtime; this is data
// rather than a C04 character-session record.
struct STAICharacterMapArrangeData {
    STAICharacterMapArrangeData();
    STAICharacterMapArrangeData(const STAICharacterMapArrangeData& other);
    ~STAICharacterMapArrangeData();

    STAICharacterMapArrangeData& operator=(
        const STAICharacterMapArrangeData& other);
    void Clear();

    int32_t raw_00;
    int32_t raw_04;
    int32_t raw_08;
    int32_t raw_0c;
    int32_t raw_10;
    int32_t raw_14;
    uint8_t raw_18;
    uint8_t raw_19;
    uint8_t pad_1a[2];
    std::vector<int> int_values;
    int32_t raw_28;
    std::vector<uint8_t> byte_values;
};

// IDA_EA: 0x88C8A78, 0x88C903C, 0x88C9574, 0x88C9592,
// 0x88C98BC. Old i386 size: 0x1014.
struct STAICharacterCommonScript {
    STAICharacterCommonScript();
    virtual ~STAICharacterCommonScript();

    STAICharacterCommonScript* clear();
    bool destroy();

    MinimumInfo minimum_info;
    STRDCharacterStatus character_status;
    std::vector<STAICharacterEquipmentData> equipment;
    std::vector<STAICharacterSkillData> quick_skills;
    std::vector<STAICharacterSkillData> skills;
    std::vector<STAICharacterSkillCorrection> skill_corrections;
    std::vector<STAICharacterItemData> quick_items;
    std::vector<STAICharacterItemData> items;
    STAIInfoScript ai_info;
    int32_t ai_runtime_values[8];
    std::vector<std::string> key_files;
    std::vector<std::string> key_file_names;
    std::map<std::string, int32_t> key_cooltimes;
    APCSpeech speech[111];
    std::vector<APCSpeech> attack_speech;
    std::vector<APCSpeech> key_speech;
    int32_t rarity;
    int32_t appearance_point;
    std::vector<std::pair<int32_t, int32_t> > death_tower_items;
    std::vector<std::pair<int32_t, int32_t> > party_members;
    int32_t far_attack_reaction_rate;
    int32_t pvp_win_point_min;
    int32_t pvp_win_point_max;
    STRDCharacterStatusRate character_status_rate;
    std::vector<STAICharacterCustomSkillLevel> custom_skill_levels;
    float attack_damage_rate;
    int32_t add_equipment_status_from_level;
    int32_t armor_subtype;
    int32_t default_equipment;
    std::vector<int32_t> doll_special_dungeons;
    STRDCharacterStatusRate character_status_rate_special;
    float attack_damage_rate_special;
    std::vector<int32_t> common_champion_elements;
    std::vector<stMonsterIndependentDrop_t> independent_drops;
    std::vector<int32_t> limited_passive_objects;
    int32_t overhead_gauge_type;
    int32_t overhead_gauge_x;
    int32_t overhead_gauge_y;
    stEnamyDropAvatarInfo_t drop_avatar_info;
    int32_t getup_invincible;
    std::vector<EquipmentSkillDataUp> skill_data_up;
    std::vector<float> summon_monster_status_rates;
    uint8_t leave_body;
    uint8_t padding_fa9[3];
    ActiveActionFileSet action_files;
    std::string last_action;
    std::string proc_action;
    uint8_t add_gravity_in_aerial;
    uint8_t padding_ff5[3];
    float aerial_gravity_1;
    float aerial_gravity_2;
    int32_t no_cooltime;
    uint8_t check_account_item;
    uint8_t padding_1005[3];
    int32_t npc_index;
    uint8_t hell_monster;
    uint8_t padding_100d[3];
    int32_t animation_job;
    STAICharacterReviveOption revive_option;
    STAICharacterRecoveryOption recovery_option;
};

static_assert(sizeof(MinimumInfo) == 0x08u,
              "MinimumInfo IDA ABI size must stay 0x08");
static_assert(sizeof(APCSpeech) == 0x18u,
              "APCSpeech IDA ABI size must stay 0x18");
static_assert(sizeof(ActiveActionFileSet) == 0x40u,
              "ActiveActionFileSet IDA ABI size must stay 0x40");
static_assert(sizeof(STRDCharacterStatus) == 0x57u,
              "STRDCharacterStatus IDA ABI size must stay 0x57");
static_assert(sizeof(STRDCharacterStatusRate) == 0xA4u,
              "STRDCharacterStatusRate IDA ABI size must stay 0xA4");
static_assert(sizeof(STAICharacterEquipmentData) == 0x0Cu,
              "STAICharacterEquipmentData IDA ABI size must stay 0x0C");
static_assert(sizeof(STAICharacterSkillData) == 0x08u,
              "STAICharacterSkillData IDA ABI size must stay 0x08");
static_assert(sizeof(STAICharacterItemData) == 0x08u,
              "STAICharacterItemData IDA ABI size must stay 0x08");
static_assert(sizeof(STAICharacterSkillCorrection) == 0x08u,
              "STAICharacterSkillCorrection IDA ABI size must stay 0x08");
static_assert(sizeof(STAICharacterCustomSkillLevel) == 0x10u,
              "STAICharacterCustomSkillLevel IDA ABI size must stay 0x10");
static_assert(sizeof(STAICharacterReviveOption) == 0x14u,
              "STAICharacterReviveOption i386 size must stay 0x14");
static_assert(sizeof(STAICharacterRecoveryOption) == 0x14u,
              "STAICharacterRecoveryOption i386 size must stay 0x14");
static_assert(sizeof(STAICharacterMapArrangeData) == 0x38u,
              "STAICharacterMapArrangeData IDA ABI size must stay 0x38");
static_assert(sizeof(STAICharacterCommonScript) == 0x103Cu,
              "STAICharacterCommonScript i386 size must stay 0x103C");

extern char g_aiCharacterScriptBaseDirectory[0x100];
extern STScriptFileList g_aiCharacterScriptFileList;

// IDA_EA: 0x88C9CCC.
int importAICharacterCommonScript(STAICharacterCommonScript* script,
                                  const char* filename,
                                  bool module_type)
    __asm__("_Z29importAICharacterCommonScriptP25STAICharacterCommonScriptPKcb");

// IDA_EA: 0x8A38AF4, 0x8A38B5D, 0x8A390DB.
int stringToInt(const char* text, const char** table, int count)
    __asm__("_Z11stringToIntPKcPS0_i");
extern const char* ACTIVEOBJECT_STATE_STRING[];
extern const char* ACTIVESTATUS_ATTACK_TAG[];
void makeAIFullPath(std::string& base_path, std::string& path)
    __asm__("_Z14makeAIFullPathRSsS_");
void makeKeyFullPath(std::string& base_path, std::string& path)
    __asm__("_Z15makeKeyFullPathRSsS_");
int importAIInfoScript(STAIInfoScript* script,
                       std::string* tag,
                       bool module_type)
    __asm__("_Z18importAIInfoScriptP14STAIInfoScriptPSsb");

#endif  // OLDSRV_DATAMANAGER_AI_CHARACTER_SCRIPT_H_
