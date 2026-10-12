#ifndef OLDSRV_DATAMANAGER_PASSIVE_OBJECT_SCRIPT_TYPES_H_
#define OLDSRV_DATAMANAGER_PASSIVE_OBJECT_SCRIPT_TYPES_H_

#include <stddef.h>
#include <stdint.h>

#include <string>
#include <utility>
#include <vector>

// PVF PassiveObject value layouts.  These are the concrete original member
// types used by STPassiveObjectScript; no consumer may substitute a raw byte
// buffer for this data surface.
struct SpeedCtrlPassiveObjInfo {
    SpeedCtrlPassiveObjInfo();
    ~SpeedCtrlPassiveObjInfo();

    int32_t passive_object_index;
    uint8_t enabled;
};

struct WarningDistance {
    WarningDistance();
    WarningDistance(int first_value, int second_value, int third_value);

    int32_t first;
    int32_t second;
    int32_t third;
};

struct RandomItemFairy {
    RandomItemFairy();
    ~RandomItemFairy();
    bool clear();

    int32_t item_index;
    int32_t item_count;
    int32_t probability;
    int32_t condition;
    // 0828 racing reward weights for the three dungeon grades.
    std::vector<int> racing_dungeon_probabilities;
};

struct STPassiveObjectAddEffect {
    STPassiveObjectAddEffect();
    ~STPassiveObjectAddEffect();
    void clear();

    std::vector<std::string> effect_files;
    std::vector<int> effect_values;
};

struct STPassiveObjectAdditionalParticleInfo {
    STPassiveObjectAdditionalParticleInfo();
    std::string* clear();

    std::string file_name;
    std::string create_sound;
    std::string reserved_name;
    int32_t create_delay;
    int32_t follow_parent[3];
    uint8_t loop;
    uint8_t padding_1d;
    uint8_t padding_1e;
    uint8_t padding_1f;
};

struct STPassiveObjectDestroy {
    STPassiveObjectDestroy();
    ~STPassiveObjectDestroy();
    void clear();

    int32_t destroy_type;
    int32_t destroy_delay;
    std::vector<int> item_indices;
    std::vector<std::string> effect_files;
    std::vector<std::string> sound_files;
    std::vector<int> value_list;
    int32_t field_38;
    int32_t field_3c;
    int32_t field_40;
    std::vector<int> additional_values;
};

struct PassiveObjGeneratorPattern {
    PassiveObjGeneratorPattern();
    PassiveObjGeneratorPattern(const PassiveObjGeneratorPattern& other);
    ~PassiveObjGeneratorPattern();
    PassiveObjGeneratorPattern& operator=(
        const PassiveObjGeneratorPattern& other);

    uint8_t enabled;
    uint8_t padding_01;
    uint8_t padding_02;
    uint8_t padding_03;
    std::string pattern_name;
    int32_t first_value;
    int32_t second_value;
    std::vector<uint8_t> values;
};

struct PassiveObjGeneratorSequence {
    PassiveObjGeneratorSequence();
    PassiveObjGeneratorSequence(const PassiveObjGeneratorSequence& other);
    ~PassiveObjGeneratorSequence();
    PassiveObjGeneratorSequence& operator=(
        const PassiveObjGeneratorSequence& other);

    std::string sequence_name;
    int32_t value;
};

struct PassiveObjGeneratorObject {
    PassiveObjGeneratorObject();
    PassiveObjGeneratorObject(const PassiveObjGeneratorObject& other);
    ~PassiveObjGeneratorObject();
    PassiveObjGeneratorObject& operator=(
        const PassiveObjGeneratorObject& other);

    int32_t object_index;
    float probability;
    std::string warning_name;
};

struct PassiveObjGeneratorInfo {
    PassiveObjGeneratorInfo();
    ~PassiveObjGeneratorInfo();

    std::vector<PassiveObjGeneratorPattern> patterns;
    std::vector<PassiveObjGeneratorSequence> sequences;
    std::vector<PassiveObjGeneratorObject> objects;
};

struct PassiveIObjSniperInfo {
    PassiveIObjSniperInfo();
    ~PassiveIObjSniperInfo();
    PassiveIObjSniperInfo* clear();
    PassiveIObjSniperInfo& operator=(const PassiveIObjSniperInfo& other);

    std::vector<int> first_values;
    std::vector<int> second_values;
    std::vector<int> third_values;
    std::vector<int> fourth_values;
    int32_t field_30;
    int32_t field_34;
    uint8_t field_38;
    uint8_t padding_39;
    uint8_t padding_3a;
    uint8_t padding_3b;
    int32_t field_3c;
    uint8_t field_40;
    uint8_t padding_41;
    uint8_t padding_42;
    uint8_t padding_43;
    int32_t fields_44_58[6];
    int32_t field_5c;
    int32_t field_60;
    int32_t field_64;
    int32_t field_68;
    int32_t field_6c;
    int32_t field_70;
    int32_t fields_74_7c[3];
    int32_t field_80;
    int32_t sniper_type;
};

// IDA unit: 12_pvf_datamanager_scripts/STPassiveObjectScript.
struct STPassiveObjectScript {
    STPassiveObjectScript();
    ~STPassiveObjectScript();
    STPassiveObjectScript* clear();

    int32_t object_index;
    std::string object_name;
    std::string resource_name;
    int32_t fields_0c_24[7];
    std::vector<int> spawn_values;
    std::string display_name;
    std::vector<std::string> first_string_values;
    std::string first_effect;
    std::vector<std::string> second_string_values;
    std::string second_effect;
    uint8_t state_58;
    uint8_t state_59;
    uint8_t state_5a;
    uint8_t padding_5b;
    int32_t field_5c;
    uint8_t state_60;
    uint8_t state_61;
    uint8_t state_62;
    uint8_t state_63;
    std::string third_effect;
    std::string fourth_effect;
    std::vector<std::string> third_string_values;
    std::vector<int> additional_spawn_values;
    std::vector<std::string> fourth_string_values;
    std::string script_filename;
    int32_t field_94;
    int32_t field_98;
    int32_t field_9c;
    int32_t field_a0;
    int32_t field_a4;
    float field_a8;
    float field_ac;
    int32_t field_b0;
    uint8_t state_b4;
    uint8_t padding_b5;
    uint8_t padding_b6;
    uint8_t padding_b7;
    float field_b8;
    float field_bc;
    float field_c0;
    int32_t field_c4;
    int32_t field_c8;
    int32_t field_cc;
    int32_t field_d0;
    uint8_t state_d4;
    uint8_t state_d5;
    uint8_t padding_d6;
    uint8_t padding_d7;
    STPassiveObjectDestroy destroy;
    STPassiveObjectAddEffect add_effect;
    std::vector<STPassiveObjectAdditionalParticleInfo*> additional_particles;
    uint8_t for_tournament;
    uint8_t for_school;
    uint8_t die_when_parent_dies;
    uint8_t transparent_on_meet_player;
    int16_t pvp_observer_transparency;
    uint16_t reserved_152;
    std::string particle_name;
    uint8_t state_158;
    uint8_t state_159;
    uint8_t padding_15a;
    uint8_t padding_15b;
    int32_t field_15c;
    std::vector<int> particle_values;
    uint8_t state_16c;
    uint8_t padding_16d;
    uint8_t padding_16e;
    uint8_t padding_16f;
    float field_170;
    int32_t field_174;
    int32_t field_178;
    std::string speed_control_name;
    std::vector<SpeedCtrlPassiveObjInfo> speed_control_objects;
    std::vector<PassiveObjGeneratorPattern> generator_patterns;
    std::vector<PassiveObjGeneratorSequence> generator_sequences;
    std::vector<PassiveObjGeneratorObject> generator_objects;
    PassiveObjGeneratorInfo generator_info;
    WarningDistance warning_distance_near;
    WarningDistance warning_distance_far;
    PassiveIObjSniperInfo sniper_info;
    std::vector<RandomItemFairy> random_item_fairies;
    uint8_t state_280;
    uint8_t padding_281;
    uint8_t padding_282;
    uint8_t padding_283;
    std::string final_name;
    // The original C2/D2 only construct/destruct final_name.  These two
    // trailing plain integers are written by [passive object icon] and are
    // intentionally left untouched by clear(), exactly like the old ABI.
    int32_t passive_object_icon_x;
    int32_t passive_object_icon_y;
    // 0828 extension fields.  These remain immutable PVF prototype data;
    // client-rendered fields are still preserved so importing one tag never
    // hides the following server-relevant object definition.
    int32_t blocking_area;
    uint8_t apply_parent_ability_category;
    uint8_t reserved_295[3];
    std::vector<std::pair<int32_t, int32_t> > party_hp_max_rates;
    float damage_absorb;
    int32_t minimap_marker_mode;
    std::string minimap_marker_image;
    int32_t absolute_zpos;
};

// IDA_EA: 0x8A2C51A
// IDA_NAME: _Z25ImportPassiveObjectScriptP21STPassiveObjectScriptPKcb
int ImportPassiveObjectScript(STPassiveObjectScript* script,
                              const char* filename,
                              bool resolve_file_index);

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(SpeedCtrlPassiveObjInfo) == 0x08u,
              "SpeedCtrlPassiveObjInfo IDA ABI");
static_assert(sizeof(WarningDistance) == 0x0cu, "WarningDistance IDA ABI");
static_assert(sizeof(RandomItemFairy) == 0x1cu,
              "RandomItemFairy 0828 extension ABI");
static_assert(sizeof(STPassiveObjectAddEffect) == 0x18u,
              "STPassiveObjectAddEffect IDA ABI");
static_assert(sizeof(STPassiveObjectAdditionalParticleInfo) == 0x20u,
              "STPassiveObjectAdditionalParticleInfo IDA ABI");
static_assert(sizeof(STPassiveObjectDestroy) == 0x50u,
              "STPassiveObjectDestroy IDA ABI");
static_assert(sizeof(PassiveObjGeneratorPattern) == 0x1cu,
              "PassiveObjGeneratorPattern IDA ABI");
static_assert(sizeof(PassiveObjGeneratorSequence) == 0x08u,
              "PassiveObjGeneratorSequence IDA ABI");
static_assert(sizeof(PassiveObjGeneratorObject) == 0x0cu,
              "PassiveObjGeneratorObject IDA ABI");
static_assert(sizeof(PassiveObjGeneratorInfo) == 0x24u,
              "PassiveObjGeneratorInfo IDA ABI");
static_assert(sizeof(PassiveIObjSniperInfo) == 0x88u,
              "PassiveIObjSniperInfo IDA ABI");
static_assert(sizeof(STPassiveObjectScript) == 0x2b4u,
              "STPassiveObjectScript 0828 extension ABI");
static_assert(offsetof(STPassiveObjectScript, destroy) == 0xd8u &&
                  offsetof(STPassiveObjectScript, add_effect) == 0x128u &&
                  offsetof(STPassiveObjectScript, for_tournament) == 0x14cu &&
                  offsetof(STPassiveObjectScript, pvp_observer_transparency) ==
                      0x150u &&
                  offsetof(STPassiveObjectScript, sniper_info) == 0x1ecu &&
                  offsetof(STPassiveObjectScript, final_name) == 0x284u,
              "STPassiveObjectScript IDA member offsets");
#endif

#endif  // OLDSRV_DATAMANAGER_PASSIVE_OBJECT_SCRIPT_TYPES_H_
