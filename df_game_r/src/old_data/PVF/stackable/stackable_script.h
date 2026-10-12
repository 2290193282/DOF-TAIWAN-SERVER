#ifndef OLDSRV_OLD_DATA_PVF_STACKABLE_STACKABLE_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_STACKABLE_STACKABLE_SCRIPT_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "old_data/PVF/stackable/arad_st_stackable_script.h"
#include "old_data/PVF/equipment/datamanager/equipment_script.h"
#include "old_data/PVF/data/datamanager/script_file_list.h"
#include "old_data/PVF/stackable/stackable_script_types.h"
#include "common/types/stackable_value_types.h"

namespace StackableAction {
enum UsablePlace : int {
    USABLE_PLACE_NONE = 0,
};

enum UsableServerType : int {
    USABLE_SERVER_TYPE_NONE = 0,
};
}  // namespace StackableAction

// IDA unit: 12_pvf_datamanager_scripts/STStackableScript.
struct STStackableScript {
    STStackableScript();
    STStackableScript(STStackableScript& other);
    ~STStackableScript();
    STStackableScript& operator=(const STStackableScript& other);
    STStackableScript* Clear();

    // IDA data 0x94FAC20 / _ZN17STStackableScript11priceTable_E.
    // This is static storage, so it is not part of the 0x524-byte object.
    static int priceTable_[3000];

    STItemScript item;
    Arad_STStackableScript arad;
    int32_t value_18c;
    std::vector<int> values_190;
    std::vector<CRabbitBox> rabbit_boxes_19c;
    int32_t values_1a8[4];
    uint8_t value_1b8;
    CountinuousItemOption continuous_option_1b9;
    CountinuousItemOption continuous_option_1bc;
    uint8_t value_1bf;
    uint8_t exp_bonus_enabled_1c0;
    uint8_t reserved_1c1[3];
    float exp_bonus_rate_1c4;
    uint8_t value_1c8;
    uint8_t reserved_1c9[3];
    float pc_exp_bonus_rate_1cc;
    uint8_t value_1d0;
    uint8_t reserved_1d1[3];
    int32_t value_1d4;
    std::string string_1d8;
    int32_t value_1dc;
    int32_t value_1e0;
    int32_t value_1e4;
    // [action type]; the target client maps [change creature] and
    // [change creature list] to 61.
    int32_t value_1e8;
    // Action payload values.  For action 61, values are ordered
    // (source_creature_item_id, destination_creature_item_id) pairs.
    std::vector<int> values_1ec;
    std::vector<StackableAction::UsablePlace> usable_places_1f8;
    // `[use action packet]` / `[daily delete item]` parser bytes.  Their
    // exact offsets are copied by CStackableItem::set_item.
    uint8_t use_action_packet_;  // +0x204
    uint8_t daily_delete_item_;  // +0x205
    uint8_t reserved_206[2];
    std::vector<STPassiveObjectInStackable> passive_objects_208;
    std::vector<StackableAction::UsableServerType> usable_server_types_214;
    int32_t value_220;
    std::vector<int> values_224;
    std::string strings_230[3];
    int32_t values_23c[2];
    std::vector<std::vector<int> > values_244;
    std::vector<int> values_250;
    std::vector<int> values_25c;
    std::vector<STStackablePackageData> package_data_268;
    std::vector<std::vector<STStackablePackageData> > package_data_274;
    stStackableBooster_t booster_280;
    std::vector<STStackablePackageData> package_data_2a4;
    STRecovery recovery_2b0;
    STRecovery recovery_2c4;
    STStatusRecovery status_recovery_2d8;
    STStatusGenerate status_generate_2ec;
    STBuff buff_304;
    STSummonMonster summon_monster_314;
    STSummonApc summon_apc_330;
    int32_t value_348;
    STStatChangeVector stat_changes_34c;
    int32_t value_364;
    std::string string_368;
    int32_t value_36c;
    int32_t value_370;
    std::vector<std::pair<int, int> > pairs_374;
    uint16_t value_380;
    uint16_t reserved_382;
    int32_t value_384;
    uint8_t value_388;
    uint8_t reserved_389[3];
    int32_t value_38c;
    int32_t value_390;
    uint16_t value_394;
    uint8_t value_396;
    uint8_t reserved_397;
    uint16_t value_398;
    uint8_t value_39a;
    uint8_t value_39b;
    int32_t value_39c;
    uint8_t values_3a0[2];
    uint8_t reserved_3a2[2];
    std::map<int, int> map_3a4;
    int32_t value_3bc;
    std::vector<std::pair<int, int> > pairs_3c0;
    int32_t value_3cc;
    uint8_t values_3d0[2];
    uint8_t reserved_3d2[2];
    stLimitCubeInfo limit_cube_3d4;
    std::map<STEnchantSystemMapKey, STEnchantSystemMapData> enchant_map_3f4;
    uint8_t value_40c;
    uint8_t value_40d;
    uint8_t value_40e;
    uint8_t reserved_40f;
    std::string string_410;
    uint8_t value_414;
    uint8_t reserved_415[3];
    // [evolution creature].  CStackableItem::set_item copies this into its
    // +0x360 evolution_creature_id_ member; CMD 324 verifies that it matches
    // the currently equipped creature before ItemEvolute is permitted.
    int32_t evolution_creature_id;
    BoosterSelectInfo booster_select_41c;
    uint8_t value_438;
    uint8_t reserved_439[3];
    std::pair<int, int> pair_43c;
    int32_t value_444;
    int32_t values_448[12];
    int32_t value_478;
    AvatarOptionChangeType avatar_option_change_47c;
    uint8_t reserved_47f;
    std::set<int> set_480;
    std::set<int> set_498;
    uint8_t value_4b0;
    uint8_t reserved_4b1[3];
    std::string string_4b4;
    int32_t value_4b8;
    std::map<int, EquipmentParameterInfo*> module_parameters_4bc;
    std::set<int> set_4d4;
    int32_t values_4ec[4];
    std::vector<int> values_4fc;
    int32_t value_508;
    std::set<int> set_50c;
    // CLIENT_EA: ImportStackableScript_ParseLimitDungeonIds @ 0x0156D215.
    std::set<int> limit_dungeon_ids;
    // CLIENT_EA: ImportStackableScript_ParseAddSocketLimitedAvatarIds
    // @ 0x0156D47E.
    std::set<int> add_socket_limited_avatar_ids;
    // CLIENT_EA: ImportStackableScript_ReadResentDayValueClampedNonnegative
    // @ 0x0156EDAE.
    int32_t resent_day_value;
    // CLIENT_EA: ImportStackableScript_ReadCeraCouponFlag @ 0x0156EDDC.
    uint8_t cera_coupon;
    // CLIENT_EA: ImportStackableScript_ParseExclusiveDungeonTooltipAndIds
    // @ 0x0156EE2B.
    uint8_t exclusive_dungeon_tooltip_enabled;
    uint8_t reserved_55a[2];
    std::set<int> exclusive_dungeon_ids;
    // CLIENT_EA: ImportStackableScript_ParseAvatarBoosterPreviewItemIds
    // @ 0x0156EF4A.
    std::vector<int> avatar_booster_preview_item_ids;
    // 0828 tail fields recovered from ImportStackableScript @0x01565A70.
    std::vector<std::string> stackable_appendage_scripts;
    uint8_t verify_stack_number;
    uint8_t reserved_58d[3];
    int32_t all_skill_plus;
    int32_t levelup_mode;
    std::vector<StackableDisguiseAppendageModifier> disguise_appendages;
    std::vector<StackableEmancipateGrowGroup> emancipate_grow_groups;
    std::vector<std::pair<int, int> > emancipate_material_revisions;
    std::set<int> stackable_action_limit_indexes;
    std::vector<StackableRandomImageEntry> random_image_entries;
};

static_assert(offsetof(STStackableScript, limit_dungeon_ids) == 0x524u,
              "STStackableScript must preserve its 0x524-byte old prefix");
static_assert(
    offsetof(STStackableScript, add_socket_limited_avatar_ids) == 0x53cu &&
        offsetof(STStackableScript, resent_day_value) == 0x554u &&
        offsetof(STStackableScript, cera_coupon) == 0x558u &&
        offsetof(STStackableScript, exclusive_dungeon_tooltip_enabled) ==
            0x559u &&
        offsetof(STStackableScript, exclusive_dungeon_ids) == 0x55cu &&
        offsetof(STStackableScript, avatar_booster_preview_item_ids) ==
            0x574u &&
        offsetof(STStackableScript, stackable_appendage_scripts) == 0x580u &&
        offsetof(STStackableScript, verify_stack_number) == 0x58cu &&
        offsetof(STStackableScript, all_skill_plus) == 0x590u &&
        offsetof(STStackableScript, levelup_mode) == 0x594u &&
        offsetof(STStackableScript, disguise_appendages) == 0x598u &&
        offsetof(STStackableScript, emancipate_grow_groups) == 0x5a4u &&
        offsetof(STStackableScript, emancipate_material_revisions) == 0x5b0u &&
        offsetof(STStackableScript, stackable_action_limit_indexes) == 0x5bcu &&
        offsetof(STStackableScript, random_image_entries) == 0x5d4u &&
        sizeof(STStackableScript) == 0x5e0u,
    "STStackableScript target stackable extension layout drifted");
static_assert(offsetof(STStackableScript, use_action_packet_) == 0x204u &&
                  offsetof(STStackableScript, daily_delete_item_) == 0x205u,
              "STStackableScript daily-delete parser bytes must retain IDA offsets");
static_assert(offsetof(STStackableScript, summon_apc_330) == 0x330u &&
                  offsetof(STStackableScript, value_348) == 0x348u &&
                  offsetof(STStackableScript, value_40c) == 0x40cu &&
                  offsetof(STStackableScript, value_40d) == 0x40du &&
                  offsetof(STStackableScript, value_40e) == 0x40eu &&
                  offsetof(STStackableScript, string_410) == 0x410u &&
                  offsetof(STStackableScript, value_414) == 0x414u &&
                  offsetof(STStackableScript, evolution_creature_id) == 0x418u &&
                  offsetof(STStackableScript, booster_select_41c) == 0x41cu &&
                  offsetof(STStackableScript, pair_43c) == 0x43cu &&
                  offsetof(STStackableScript, value_478) == 0x478u &&
                  offsetof(STStackableScript, avatar_option_change_47c) ==
                      0x47cu,
              "STStackableScript corrected IDA member offsets must stay exact");

// IDA data 0x94FDB00 / 0x94FDC00; populated by InitStackableScript @0x89AE72E.
// The list storage is shared with the app runtime and has one definition.
extern char g_szStackableScriptBaseDirectory[256];
extern STScriptFileList g_stStackableScriptFileList;

// IDA_EA: 0x89AE72E.
// IDA_NAME: _Z19InitStackableScriptPKcS0_S0_
// DEMANGLED: InitStackableScript(char const*,char const*,char const*)
int InitStackableScript(const char* src,
                        const char* list_file,
                        const char* price_table);

// IDA_EA: 0x89AE906
// IDA_NAME: _Z33StackableScriptSpecialMonsterDropP17STStackableScriptSsPKc
// DEMANGLED: StackableScriptSpecialMonsterDrop(STStackableScript *,std::string,char const*)
int StackableScriptSpecialMonsterDrop(STStackableScript* script,
                                      std::string tag,
                                      const char* filename);

// IDA_EA: 0x89AEB9B, 0x89AED67, 0x89AEE0A, 0x89AEE71.
int getStackableActionType(std::string& tag);
int getStackableActionUsablePlace(std::string& tag);
int getStackableActionUsableServerType(std::string& tag);
bool getStackableIncreaseStatusType(std::string& tag);

// IDA_EA: 0x89AEEC8
// IDA_NAME: _Z21ImportStackableScriptP17STStackableScriptPKcbi
// DEMANGLED: ImportStackableScript(STStackableScript *,char const*,bool,int)
int ImportStackableScript(STStackableScript* script,
                          const char* filename,
                          bool unknown,
                          int item_id);

#endif  // OLDSRV_OLD_DATA_PVF_STACKABLE_STACKABLE_SCRIPT_H_
