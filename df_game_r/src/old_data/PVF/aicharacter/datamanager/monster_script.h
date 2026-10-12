#ifndef OLDSRV_DATAMANAGER_MONSTER_SCRIPT_H_
#define OLDSRV_DATAMANAGER_MONSTER_SCRIPT_H_

#include <stddef.h>
#include <stdint.h>

#include <deque>
#include <string>
#include <utility>
#include <vector>

#include "old_game/activity/schedule/event_value_types.h"
#include "old_data/PVF/aicharacter/st_ai_info_script.h"
#include "old_data/PVF/etc/independent_drop_types.h"
#include "common/manager/monster_script_public_abi.h"

// IDA unit: 99_misc_global/CatchItemInfo. Old i386 size: 0x0C.
struct CatchItemInfo {
    int32_t item_index;
    int32_t count;
    float probability;

    ~CatchItemInfo();
};

struct STMonsterParameterCategory {
    STMonsterParameterCategory();
    ~STMonsterParameterCategory();

    int getValueInt() const;
    long double getValueFloat() const;

    int32_t parameter_type;
    int32_t parameter_index;
    int32_t value_x100;
};

struct STAttackMethod {
    STAttackMethod();
    ~STAttackMethod();

    STAttackMethod* clear();

    int32_t values[13];
};

struct STDifficulty {
    STDifficulty();
    ~STDifficulty();

    STDifficulty* clear();

    int32_t level;
    int32_t value;
};

struct STAtkSuper {
    STAtkSuper();
    ~STAtkSuper();

    STAtkSuper* clear();

    int32_t values[6];
};

struct STDash {
    STDash();
    ~STDash();

    STDash* clear();

    int32_t values[10];
};

struct STCounterAttack {
    STCounterAttack();
    ~STCounterAttack();

    STCounterAttack* clear();

    int32_t probability;
    int32_t attack_index;
    int32_t damage_type;
    uint8_t use_need_hp;
    uint8_t enable;
    int32_t need_hp;
};

struct STHold {
    STHold();
    ~STHold();

    STHold* clear();

    int32_t values[9];
};

struct STJumpAttack {
    STJumpAttack();
    ~STJumpAttack();

    STJumpAttack* clear();

    int32_t attack_index;
    int32_t jump_height;
    int32_t jump_speed;
    int32_t collision_animation_index;
    float enemy_distance_rate;
    int32_t jump_type;
    int32_t jump_direction;
    int32_t start_frame_index;
    int32_t end_frame_index;
    int32_t limited_x_coordinate;
    int32_t limited_y_coordinate;
};

struct STSummonInfo {
    STSummonInfo();
    ~STSummonInfo();

    STSummonInfo* clear();

    int32_t values[4];
};

struct STSummon {
    STSummon();
    STSummon(const STSummon& other);
    ~STSummon();

    STSummon& operator=(const STSummon& other);
    STSummon* clear();

    int32_t values_00[4];
    std::vector<STSummonInfo> summons;
    int32_t values_1c[5];
};

struct STWarp {
    STWarp();
    ~STWarp();

    STWarp* clear();

    int32_t function_index;
    int32_t attack_index;
    int32_t warp_style;
    int32_t after_attack;
    float after_appear_time;
    int32_t cool_time;
    int32_t target_offset;
    int32_t passive_object_index;
    int32_t target_category;
};

struct STCoolTime {
    STCoolTime();
    STCoolTime(const STCoolTime& other);
    ~STCoolTime();

    STCoolTime& operator=(const STCoolTime& other);
    void clear();

    int32_t value_00;
    std::vector<int> values_04;
};

struct STInitialTarget {
    STInitialTarget();
    STInitialTarget(const STInitialTarget& other);
    ~STInitialTarget();

    STInitialTarget& operator=(const STInitialTarget& other);

    int32_t target_index;
    std::string target_name;
    int32_t value_08;
    uint8_t value_0c;
};

struct STMosterAttackReady {
    STMosterAttackReady();
    STMosterAttackReady(const STMosterAttackReady& other);
    ~STMosterAttackReady();

    STMosterAttackReady& operator=(const STMosterAttackReady& other);
    void clear();

    std::vector<int> layer_indices;
    int32_t motion_index;
    std::vector<std::string> effect_animation_files;
};

struct STThrowAttack {
    STThrowAttack();
    STThrowAttack(const STThrowAttack& other);
    ~STThrowAttack();

    STThrowAttack& operator=(const STThrowAttack& other);
    STThrowAttack* clear();

    std::vector<std::string> passive_object_paths;
    int32_t value_0c;
    int32_t value_10;
    int32_t value_14;
    std::vector<int> start_x_coordinates;
    std::vector<int> start_y_coordinates;
    std::vector<int> start_z_coordinates;
    int32_t attack_number;
    std::string throw_sound;
    int32_t values_44[5];
    std::vector<int> values_58;
};

struct STRidableSkill {
    STRidableSkill();
    STRidableSkill(const STRidableSkill& other);
    ~STRidableSkill();

    STRidableSkill& operator=(const STRidableSkill& other);

    int32_t pattern;
    std::string image_path;
    int32_t keys[8];
    std::vector<int> parameters[8];
    int32_t modes[8];
    int32_t values_a[8];
    int32_t values_b[8];
    int32_t values_c[8];
};

struct STRidableSkillExplain {
    STRidableSkillExplain();
    STRidableSkillExplain(const STRidableSkillExplain& other);
    ~STRidableSkillExplain();

    STRidableSkillExplain& operator=(const STRidableSkillExplain& other);

    std::string title;
    std::string subtitle;
    std::string category;
    std::string detail;
    int32_t pattern;
};

// STGuard has no non-trivial old lifecycle symbol. The element allocation is
// 36 bytes in _ZN9__gnu_cxx13new_allocatorI7STGuardE8allocateEjPKv.
struct STGuard {
    int32_t values[9];
};

// IDA unit: 06_userinfo_stat_inventory/stEnamyDropAvatarElement_t.
struct stEnamyDropAvatarElement_t {
    int32_t item_index;
    int32_t accumulated_rate;
    int32_t avatar_part;

    ~stEnamyDropAvatarElement_t();
};

struct stEnamyDropAvatarInfo_t {
    stEnamyDropAvatarInfo_t();
    ~stEnamyDropAvatarInfo_t();

    stEnamyDropAvatarInfo_t& operator=(
        const stEnamyDropAvatarInfo_t& other);
    void clear();

    int32_t base_rate;
    int32_t drop_count;
    float part_rates[4];
    float grade_rates[5];
    int32_t total_rate;
    std::vector<stEnamyDropAvatarElement_t> elements;
};

class STMonsterEquipment : public std::vector<std::string> {
public:
    STMonsterEquipment();
    ~STMonsterEquipment();

    STMonsterEquipment& operator=(const STMonsterEquipment& other);
    void clear();
};

// IDA unit: 12_pvf_datamanager_scripts/STMonsterScript. Old i386 size: 0x1CC4.
struct STMonsterScript {
    STMonsterScript();
    ~STMonsterScript();

    STMonsterScript& operator=(const STMonsterScript& other);
    STMonsterScript* Clear();

    int32_t index;
    std::string name;
    std::string name_08;
    int32_t values_0c[4];
    std::vector<int> values_1c;
    std::vector<std::pair<int, int> > special_item_lists[4];
    std::vector<std::pair<int, int> > death_tower_specify_items;
    int32_t values_64[3];
    std::vector<CatchItemInfo> catch_items;
    std::vector<STMonsterParameterCategory> parameter_categories;
    int32_t value_88;
    std::vector<std::pair<int, int> > special_drop_items;
    std::string burning_death_path;
    std::vector<STThrowAttack> throw_attacks;
    std::vector<STCounterAttack> counter_attacks;
    std::vector<STJumpAttack> jump_attacks;
    std::vector<STSummon> summons;
    std::vector<STDash> dash_attacks;
    std::vector<STHold> hold_attacks;
    std::vector<STWarp> warp_attacks;
    std::vector<STDifficulty> difficulties;
    std::vector<STEventSuper> event_supers;
    std::vector<STAtkSuper> attack_supers;
    std::vector<STCoolTime> cooltimes;
    std::vector<int> value_120;
    std::vector<MonsterBaseParameter> base_parameters;
    STAIInfoScript ai_info;
    uint8_t value_33c;
    std::deque<STInitialTarget> initial_targets;
    uint8_t value_368;
    int32_t values_36c[2];
    uint8_t value_374;
    int32_t values_378[3];
    std::vector<STGuard> guards;
    STMonsterEquipment equipment;
    std::vector<STMosterAttackReady> ready_actions;
    uint8_t value_3a8;
    std::vector<stMonsterIndependentDrop_t> independent_drops;
    uint8_t value_3b8;
    int32_t value_3bc;
    int32_t value_3c0;
    int32_t values_3c4[10];
    // IDA load_monsters @0x834FCB1/+0x834FCE1 reads these two words as the
    // inclusive level-index range used to populate CMonsterManager's
    // level-to-monster multimap.
    int32_t level_range[2];
    int32_t values_3f4[8];
    int32_t values_414[34];
    int32_t values_49c[22];
    int32_t values_4f4[5];
    int32_t values_508[8];
    int32_t values_528[2];
    std::string strings_530[2];
    int32_t values_538[4];
    uint8_t value_548;
    float value_54c;
    float value_550;
    std::vector<int> values_554;
    std::vector<int> values_560;
    uint8_t value_56c;
    std::string value_570;
    int32_t values_574[3];
    uint8_t values_580[2];
    int32_t values_584[3];
    uint8_t values_590[3];
    int32_t values_594[4];
    uint8_t values_5a4[2];
    int32_t value_5a8;
    std::string value_5ac;
    int32_t value_5b0;
    std::string strings_5b4[6];
    std::string strings_5cc[2];
    std::string strings_5d4[4];
    std::vector<std::string> values_5e4;
    std::vector<std::string> values_5f0;
    std::string strings_5fc[6];
    std::string strings_614[2];
    std::string strings_61c[4];
    std::vector<std::string> values_62c;
    std::vector<std::string> values_638;
    std::string strings_644[5];
    int32_t values_658[7];
    uint8_t value_674;
    int32_t values_678[2];
    uint8_t value_680;
    int32_t value_684;
    std::vector<std::vector<int> > patterns;
    std::string strings_694[9];
    std::vector<std::string> values_6b8;
    std::vector<std::string> values_6c4;
    std::string value_6d0;
    std::vector<int> values_6d4;
    int32_t value_6e0;
    uint8_t value_6e4;
    std::vector<STAttackMethod> attack_methods;
    int32_t values_6f4[5];
    std::string value_708;
    std::vector<std::string> motion_data[111];
    std::vector<int> values_c40;
    int32_t value_c4c;
    uint8_t value_c50;
    int32_t values_c54[7];
    uint8_t values_c70[2];
    int32_t values_c74[3];
    std::vector<int> values_c80;
    std::vector<int> values_c8c;
    stEnamyDropAvatarInfo_t enemy_drop_avatar;
    uint8_t value_cd4;
    int32_t value_cd8;
    std::vector<STRidableSkill> ridable_skills;
    std::vector<STRidableSkillExplain> ridable_skill_explains;
    float physical_attack_by_difficulty[200][5];
    int32_t values_1c94[2];
    int32_t value_1c9c;
    int32_t value_1ca0;
    float value_1ca4;
    uint8_t value_1ca8;
    int32_t values_1cac[2];
    std::vector<int> can_beat_indices;
    int32_t value_1cc0;

    // 140828 monster-state extension.  CLIENT_EA sub_15A0960 stores these
    // immediately after the newer client's legacy body: get-up uses a
    // literal-one test, deathless owns a boolean property, and the final two
    // switches use nonzero semantics.  The official 0828 PVF has exactly two
    // champion multiplier keys, [fastmove] and [fastattack].
    uint8_t getup_invincible;
    uint8_t deathless;
    uint8_t impossible_hp_recovery;
    uint8_t disable_appendage;
    uint8_t has_champion_fastmove_factor;
    uint8_t has_champion_fastattack_factor;
    uint8_t reserved_1cca[2];
    float champion_fastmove_factor;
    float champion_fastattack_factor;
};

// IDA_EA: 0x8A3C67D
// IDA_NAME: _Z16isValidExtensionRKSsS0_
// DEMANGLED: isValidExtension(std::string const&,std::string const&)
int isValidExtension(const std::string& filename,
                     const std::string& extension)
    __asm__("_Z16isValidExtensionRKSsS0_");

// IDA_EA: 0x8A3C736
// IDA_NAME: _Z22ImportMonsterBaseTableP15STMonsterScriptPKc
// DEMANGLED: ImportMonsterBaseTable(STMonsterScript *,char const*)
int ImportMonsterBaseTable(STMonsterScript* script, const char* filename);

// IDA_EA: 0x8A12827
// IDA_NAME: _Z19ImportMonsterScriptP15STMonsterScriptiPKcb
// DEMANGLED: ImportMonsterScript(STMonsterScript *,int,char const*,bool)
int ImportMonsterScript(STMonsterScript* script,
                        int module_type,
                        const char* filename,
                        bool server_only);

static_assert(sizeof(CatchItemInfo) == 0x0Cu,
              "CatchItemInfo IDA ABI size must stay 0x0C");
static_assert(offsetof(CatchItemInfo, item_index) == 0x00u,
              "CatchItemInfo item index must retain IDA +0x00");
static_assert(offsetof(CatchItemInfo, count) == 0x04u,
              "CatchItemInfo count must retain IDA +0x04");
static_assert(offsetof(CatchItemInfo, probability) == 0x08u,
              "CatchItemInfo probability must retain IDA +0x08");
static_assert(sizeof(STMonsterParameterCategory) == 0x0Cu,
              "STMonsterParameterCategory IDA ABI size must stay 0x0C");
static_assert(sizeof(STAttackMethod) == 0x34u,
              "STAttackMethod IDA ABI size must stay 0x34");
static_assert(sizeof(STDifficulty) == 0x08u,
              "STDifficulty IDA ABI size must stay 0x08");
static_assert(sizeof(STAtkSuper) == 0x18u,
              "STAtkSuper IDA ABI size must stay 0x18");
static_assert(sizeof(STDash) == 0x28u,
              "STDash IDA ABI size must stay 0x28");
static_assert(sizeof(STCounterAttack) == 0x14u,
              "STCounterAttack IDA ABI size must stay 0x14");
static_assert(sizeof(STHold) == 0x24u,
              "STHold IDA ABI size must stay 0x24");
static_assert(sizeof(STJumpAttack) == 0x2Cu,
              "STJumpAttack IDA ABI size must stay 0x2C");
static_assert(sizeof(STSummonInfo) == 0x10u,
              "STSummonInfo IDA ABI size must stay 0x10");
static_assert(sizeof(STSummon) == 0x30u,
              "STSummon IDA ABI size must stay 0x30");
static_assert(sizeof(STWarp) == 0x24u,
              "STWarp IDA ABI size must stay 0x24");
static_assert(sizeof(STCoolTime) == 0x10u,
              "STCoolTime IDA ABI size must stay 0x10");
static_assert(sizeof(STInitialTarget) == 0x10u,
              "STInitialTarget IDA ABI size must stay 0x10");
static_assert(sizeof(STMosterAttackReady) == 0x1Cu,
              "STMosterAttackReady IDA ABI size must stay 0x1C");
static_assert(sizeof(STThrowAttack) == 0x64u,
              "STThrowAttack IDA ABI size must stay 0x64");
static_assert(sizeof(STRidableSkill) == 0x108u,
              "STRidableSkill IDA ABI size must stay 0x108");
static_assert(sizeof(STRidableSkillExplain) == 0x14u,
              "STRidableSkillExplain IDA ABI size must stay 0x14");
static_assert(sizeof(STGuard) == 0x24u,
              "STGuard IDA ABI size must stay 0x24");
static_assert(sizeof(stEnamyDropAvatarElement_t) == 0x0Cu,
              "stEnamyDropAvatarElement_t IDA ABI size must stay 0x0C");
static_assert(sizeof(stEnamyDropAvatarInfo_t) == 0x3Cu,
              "stEnamyDropAvatarInfo_t IDA ABI size must stay 0x3C");
static_assert(offsetof(stEnamyDropAvatarElement_t, item_index) == 0x00u,
              "avatar element item index must retain IDA +0x00");
static_assert(offsetof(stEnamyDropAvatarElement_t, accumulated_rate) == 0x04u,
              "avatar element rate must retain IDA +0x04");
static_assert(offsetof(stEnamyDropAvatarElement_t, avatar_part) == 0x08u,
              "avatar element part must retain IDA +0x08");
static_assert(offsetof(stEnamyDropAvatarInfo_t, base_rate) == 0x00u,
              "avatar base rate must retain IDA +0x00");
static_assert(offsetof(stEnamyDropAvatarInfo_t, drop_count) == 0x04u,
              "avatar drop count must retain IDA +0x04");
static_assert(offsetof(stEnamyDropAvatarInfo_t, part_rates) == 0x08u,
              "avatar part rates must retain IDA +0x08");
static_assert(offsetof(stEnamyDropAvatarInfo_t, grade_rates) == 0x18u,
              "avatar grade rates must retain IDA +0x18");
static_assert(offsetof(stEnamyDropAvatarInfo_t, total_rate) == 0x2Cu,
              "avatar total rate must retain IDA +0x2C");
static_assert(offsetof(stEnamyDropAvatarInfo_t, elements) == 0x30u,
              "avatar element vector must retain IDA +0x30");
static_assert(sizeof(STMonsterEquipment) == 0x0Cu,
              "STMonsterEquipment IDA ABI size must stay 0x0C");
static_assert(offsetof(STMonsterScript, getup_invincible) == 0x1CC4u,
              "0828 monster state flags must follow the legacy body");
static_assert(offsetof(STMonsterScript, champion_fastmove_factor) == 0x1CCCu,
              "0828 champion multipliers must retain the appended layout");
static_assert(sizeof(STMonsterScript) == 0x1CD4u,
              "STMonsterScript 0828 size must stay 0x1CD4");
static_assert(offsetof(STMonsterScript, catch_items) == 0x070u,
              "STMonsterScript catch-items vector must retain IDA +0x70");
static_assert(offsetof(STMonsterScript, enemy_drop_avatar) == 0xC98u,
              "STMonsterScript avatar data must retain IDA +0xC98");

#endif  // OLDSRV_DATAMANAGER_MONSTER_SCRIPT_H_
