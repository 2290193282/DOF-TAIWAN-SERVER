#ifndef OLDSRV_OLD_DATA_PVF_ETC_DATAMANAGER_ETC_DRIVERS_0828_H_
#define OLDSRV_OLD_DATA_PVF_ETC_DATAMANAGER_ETC_DRIVERS_0828_H_

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "old_data/PVF/etc/blood_war_script.h"
#include "old_data/PVF/etc/party_matching_info.h"
#include "old_data/PVF/etc/quest_movie_info.h"

// IDA_EA: 0x891BB5E / 0x891BC4A (0627 server).
// 0828 client owner: sub_5D6FB0.
//
// Keep the old value layout: the selectable-equipment bool is runtime state;
// the PVF supplies only the equipment key and the paired integer value.
struct PremiumServiceData {
    PremiumServiceData();
    PremiumServiceData* clear();

    int type;
    std::pair<int, int> main_premium_image_index;
    int additional_premium_image_index;
    int icon_index;
    std::string icon_explain;
    std::vector<int> add_equipment_list;
    std::map<int, std::pair<bool, int> > selectable_equipment_list;
};

// Exact enum values recovered independently from the 0627 server importer and
// the 0828 client's sub_5CD100 condition dispatch.
enum ENUM_QUEST_MOVIE_EVENT {
    QUEST_MOVIE_EVENT_ON_ACCEPT = 0,
    QUEST_MOVIE_EVENT_CLEAR = 1,
    QUEST_MOVIE_EVENT_ON_COMPLETE = 2,
    QUEST_MOVIE_EVENT_INVALID = 3
};

typedef std::pair<int, ENUM_QUEST_MOVIE_EVENT> QuestMovieKey;
typedef std::map<QuestMovieKey, QuestMovieInfo> QuestMovieMap;

// Etc/Labyrinth.etc is absent from the 0627 server importer, while the 0828
// resource contains a complete minigame grammar.  Keep neutral field names
// wherever the gameplay meaning is not proven; the parser must preserve the
// hierarchy without turning labels into guessed character/equipment logic.
struct LabyrinthIntPair0828 {
    LabyrinthIntPair0828();

    int first;
    int second;
};

struct LabyrinthIntTriple0828 {
    LabyrinthIntTriple0828();

    int first;
    int second;
    int third;
};

enum LabyrinthReinforceConditionKind0828 {
    LABYRINTH_CONDITION_ATTACK_SUCCESS = 0,
    LABYRINTH_CONDITION_ATTACK_CONDITION = 1
};

struct LabyrinthReinforceCondition0828 {
    LabyrinthReinforceCondition0828();

    int kind;
    int integer_value;
    std::string string_value;
};

enum LabyrinthReinforceEffectKind0828 {
    LABYRINTH_EFFECT_TARGET = 0,
    LABYRINTH_EFFECT_ADD_ABSOLUTE_DAMAGE,
    LABYRINTH_EFFECT_INCREASE_CRITICAL_DAMAGE,
    LABYRINTH_EFFECT_DURATION,
    LABYRINTH_EFFECT_PROBABILITY,
    LABYRINTH_EFFECT_ACTIVE_STATUS,
    LABYRINTH_EFFECT_FIRE_ATTACK,
    LABYRINTH_EFFECT_WATER_ATTACK,
    LABYRINTH_EFFECT_DARK_ATTACK,
    LABYRINTH_EFFECT_LIGHT_ATTACK,
    LABYRINTH_EFFECT_PHYSICAL_CRITICAL_HIT,
    LABYRINTH_EFFECT_EQUIPMENT_PHYSICAL_DEFENSE,
    LABYRINTH_EFFECT_HP_MAX,
    LABYRINTH_EFFECT_HP_REGEN_SPEED,
    LABYRINTH_EFFECT_HP_DRAIN,
    LABYRINTH_EFFECT_MAGICAL_CRITICAL_HIT,
    LABYRINTH_EFFECT_EQUIPMENT_MAGICAL_DEFENSE,
    LABYRINTH_EFFECT_MP_MAX,
    LABYRINTH_EFFECT_MP_REGEN_SPEED,
    LABYRINTH_EFFECT_MP_DRAIN,
    LABYRINTH_EFFECT_STUCK_RESISTANCE,
    LABYRINTH_EFFECT_STUCK,
    LABYRINTH_EFFECT_ALL_ACTIVE_STATUS_RESISTANCE,
    LABYRINTH_EFFECT_ALL_ELEMENTAL_RESISTANCE,
    LABYRINTH_EFFECT_MOVE_SPEED,
    LABYRINTH_EFFECT_ATTACK_SPEED,
    LABYRINTH_EFFECT_CAST_SPEED
};

struct LabyrinthReinforceEffect0828 {
    LabyrinthReinforceEffect0828();

    int kind;
    std::vector<std::string> string_values;
    std::vector<int> integer_values;
};

struct LabyrinthReinforceType0828 {
    LabyrinthReinforceType0828();
    void clear();

    int pattern;
    std::string name;
    std::vector<LabyrinthReinforceCondition0828> conditions;
    std::vector<LabyrinthReinforceEffect0828> then_effects;
    std::vector<LabyrinthReinforceEffect0828> passive_effects;
};

struct LabyrinthEquipmentDefinition0828 {
    LabyrinthEquipmentDefinition0828();
    void clear();

    std::string category;
    int fp_cost;
    int reinforce_max;
    std::vector<int> use_patterns;
};

struct LabyrinthConfig0828 {
    LabyrinthConfig0828();
    void clear();
    void swap(LabyrinthConfig0828& other);

    std::vector<LabyrinthIntPair0828> character_exp_table;
    std::vector<LabyrinthIntTriple0828> monster_fp_exp_points;
    int level_up_skill_point;
    LabyrinthIntPair0828 barrack_count;
    LabyrinthIntPair0828 tower_count;
    std::vector<LabyrinthIntTriple0828> barrack_fp_exp_points;
    std::vector<LabyrinthIntTriple0828> tower_fp_exp_points;
    LabyrinthIntPair0828 barrack_destroy_point;
    std::vector<LabyrinthReinforceType0828> reinforce_types;
    std::vector<LabyrinthEquipmentDefinition0828> equipment_definitions;
};

enum ChaosFPShopSection0828 {
    CHAOS_FP_SHOP_CONSUME = 0,
    CHAOS_FP_SHOP_FAIRY = 1,
    CHAOS_FP_SHOP_IMPROVE = 2,
    CHAOS_FP_SHOP_SPECIAL = 3
};

// The three integers are intentionally neutral.  No server shop packet or
// currency/catalog transaction proves which column is item/count/price.
struct ChaosFPShopEntry0828 {
    ChaosFPShopEntry0828();

    int value0;
    int value1;
    int value2;
};

struct ChaosFPShopCatalog0828 {
    std::vector<ChaosFPShopEntry0828> consume_items;
    std::vector<ChaosFPShopEntry0828> fairy_items;
    std::vector<ChaosFPShopEntry0828> improve_items;
    std::vector<ChaosFPShopEntry0828> special_items;

    void clear();
    void swap(ChaosFPShopCatalog0828& other);
};

// Exact original parser surfaces.  The rebuild implementations commit to the
// caller only after the complete file has passed structural validation.
int importPremiumServiceData(std::vector<PremiumServiceData>& output);
int importPartyMatchingInfo(const char* filename,
                            std::vector<stPartyMatchingInfo>* output);
int importQuestMovieList(QuestMovieMap& output);
int ImportWarRoomWPTable(stBloodWarGrowTable* output);
int ImportLabyrinthConfig0828(LabyrinthConfig0828* output);
int ImportChaosFPShopCatalog0828(ChaosFPShopCatalog0828* output);

// 0627 server owner: importEtcParameterScriptForServer @0x80966FA.
//
// The original server ABI publishes the first 26 values as process globals.
// The official 0828 file appends three values.  Keep the complete ordered
// generation in a rebuild-owned sidecar so the old globals are never
// overrun.  Only index 28 currently has a proven server consumer; indices
// 26/27 deliberately retain ordinal names until an official owner is found.
enum DamageFormulaConstantIndex0828 {
    DAMAGE_FORMULA_PHYSICAL_DAMAGE_RATE = 0,
    DAMAGE_FORMULA_PHYSICAL_DAMAGE_ADD = 1,
    DAMAGE_FORMULA_PHYSICAL_REDUCE_RATE = 2,
    DAMAGE_FORMULA_PHYSICAL_REDUCE_ADD = 3,
    DAMAGE_FORMULA_PHYSICAL_RANDOM_BASE = 4,
    DAMAGE_FORMULA_PHYSICAL_RANDOM_RATE = 5,
    DAMAGE_FORMULA_MAGICAL_DAMAGE_RATE = 6,
    DAMAGE_FORMULA_MAGICAL_DAMAGE_ADD = 7,
    DAMAGE_FORMULA_MAGICAL_REDUCE_RATE = 8,
    DAMAGE_FORMULA_MAGICAL_REDUCE_ADD = 9,
    DAMAGE_FORMULA_MAGICAL_RANDOM_BASE = 10,
    DAMAGE_FORMULA_MAGICAL_RANDOM_RATE = 11,
    DAMAGE_FORMULA_COMMON_ELEMENT_BASE = 12,
    DAMAGE_FORMULA_COMMON_ELEMENT_RATE = 13,
    DAMAGE_FORMULA_COMMON_ELEMENT_RATE_PVP = 14,
    DAMAGE_FORMULA_COMMON_ELEMENT_RATE_LADDER_PVP = 15,
    DAMAGE_FORMULA_HP_INCREASE_RATE = 16,
    DAMAGE_FORMULA_HP_REGEN_RATE = 17,
    DAMAGE_FORMULA_MP_INCREASE_RATE = 18,
    DAMAGE_FORMULA_MP_REGEN_RATE = 19,
    DAMAGE_FORMULA_ACTIVE_STATUS_DAMAGE_RATE = 20,
    DAMAGE_FORMULA_ACTIVE_STATUS_DEFENSE_RATE = 21,
    DAMAGE_FORMULA_PHYSICAL_DEFENSE_ADD = 22,
    DAMAGE_FORMULA_MAGICAL_DEFENSE_ADD = 23,
    DAMAGE_FORMULA_HP_REGEN_ADD_RATE = 24,
    DAMAGE_FORMULA_MP_REGEN_ADD_RATE = 25,
    DAMAGE_FORMULA_0828_ORDINAL_27 = 26,
    DAMAGE_FORMULA_0828_ORDINAL_28 = 27,
    DAMAGE_FORMULA_MAX_TOTAL_DEFENSE_RATE = 28,
    DAMAGE_FORMULA_LEGACY_COUNT = 26,
    DAMAGE_FORMULA_0828_COUNT = 29
};

struct DamageFormulaConstants0828 {
    DamageFormulaConstants0828();
    void clear();

    float values[DAMAGE_FORMULA_0828_COUNT];
};

int ImportDamageFormulaConstants0828(DamageFormulaConstants0828* output);
const DamageFormulaConstants0828& GetDamageFormulaConstants0828();
float GetDamageFormulaConstant0828(unsigned int index);
float GetMaxTotalDefenseRate0828();

// Rebuild-owned DataManager sidecar.  The original CDataManager i386 object is
// ABI-locked at 0xB7EC after its SP safety extension, so these newly recovered
// STL owners intentionally live beside that object instead of shifting it.
struct DataManagerEtcStore0828 {
    DamageFormulaConstants0828 damage_formula_constants;
    std::vector<PremiumServiceData> premium_services;
    std::vector<int> common_party_matching_dungeon_indices;
    std::vector<stPartyMatchingInfo> party_matching_infos;
    QuestMovieMap quest_movies;
    stBloodWarGrowTable war_room_wp_table;
    LabyrinthConfig0828 labyrinth;
    ChaosFPShopCatalog0828 chaos_fp_shop;

    void clear();
    void swap(DataManagerEtcStore0828& other);
};

// Loads the proven legacy ETC assets plus the strict 0828 Labyrinth/ChaosFP
// resources atomically into the process-wide CDataManager sidecar.  A failed
// file never publishes a partial generation.  Loading a catalog is not
// authorization to expose it to a purchase transaction.
int ImportDataManagerEtcStore0828();
const DataManagerEtcStore0828& GetDataManagerEtcStore0828();
const LabyrinthConfig0828& GetLabyrinthConfig0828();
const LabyrinthReinforceType0828* FindLabyrinthReinforceType0828(int pattern);
const LabyrinthEquipmentDefinition0828*
FindLabyrinthEquipmentDefinition0828(const std::string& category);
const ChaosFPShopCatalog0828& GetChaosFPShopCatalog0828();
const std::vector<ChaosFPShopEntry0828>* GetChaosFPShopSection0828(
    ChaosFPShopSection0828 section);

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(PremiumServiceData) == 0x3Cu,
              "PremiumServiceData must retain the 0627 i386 value ABI");
#endif

static_assert(DAMAGE_FORMULA_MP_REGEN_ADD_RATE == 25,
              "0627 damage-formula prefix must stay 26 values");
static_assert(DAMAGE_FORMULA_0828_ORDINAL_27 == DAMAGE_FORMULA_LEGACY_COUNT,
              "0828 extension must begin after the old ABI prefix");
static_assert(DAMAGE_FORMULA_MAX_TOTAL_DEFENSE_RATE == 28,
              "0828 defense floor must remain the 29th value");
static_assert(sizeof(DamageFormulaConstants0828) == 0x74u,
              "29 float damage-formula sidecar must stay 0x74 bytes");

#endif  // OLDSRV_OLD_DATA_PVF_ETC_DATAMANAGER_ETC_DRIVERS_0828_H_
