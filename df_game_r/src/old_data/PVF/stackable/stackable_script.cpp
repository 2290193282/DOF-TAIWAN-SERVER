#include "old_data/PVF/stackable/stackable_script.h"

#include <algorithm>

#include "common/config.h"
#include "old_data/PVF/common/arad_script_loader.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/common/script_value.h"
#include "common/manager/change_status_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/striker_public_abi.h"

namespace {

const int kChangeCreatureActionType = 61;

bool ReadStringSection(std::vector<std::string>* output,
                       const char* closing_tag) {
    if (!output || !closing_tag) return false;
    output->clear();
    while (true) {
        bool is_type = false;
        std::string value;
        if (ScanTypeOrStr(is_type, value, true) != 1) return false;
        if (is_type) return value == closing_tag;
        output->push_back(value);
    }
}

bool ReadDisguiseAppendageSection(
    std::vector<StackableDisguiseAppendageModifier>* output) {
    if (!output) return false;
    output->clear();
    while (true) {
        bool is_type = false;
        std::string ability;
        if (ScanTypeOrStr(is_type, ability, true) != 1) return false;
        if (is_type) return ability == "[/disguise appendage]";

        StackableDisguiseAppendageModifier modifier;
        modifier.ability_name = ability;
        if (ScanStr(&modifier.unit_name) != 1) return false;
        bool success = false;
        modifier.value = ScanInt(&success);
        if (!success) return false;
        output->push_back(modifier);
    }
}

bool ReadEmancipateGrowApplySection(
    std::vector<StackableEmancipateGrowGroup>* output) {
    if (!output) return false;
    output->clear();
    while (true) {
        std::string section;
        if (ScanType(section, true) != 1) return false;
        if (section == "[/emancipate grow apply]") return true;
        if (section != "[group]") return false;

        bool success = false;
        StackableEmancipateGrowGroup group;
        group.group = ScanInt(&success);
        if (!success) return false;
        group.declared_count = ScanInt(&success);
        if (!success || group.declared_count < -1 ||
            group.declared_count > 256) {
            return false;
        }
        for (int index = 0; index < group.declared_count; ++index) {
            const int grow_type = ScanInt(&success);
            if (!success) return false;
            group.grow_types.push_back(grow_type);
        }
        output->push_back(group);
    }
}

bool ReadIntPairSection(std::vector<std::pair<int, int> >* output,
                        const char* closing_tag) {
    if (!output || !closing_tag) return false;
    output->clear();
    while (true) {
        bool is_type = false;
        bool success = false;
        std::string type;
        const int first = ScanTypeOrInt(is_type, type, success, true);
        if (is_type) return type == closing_tag;
        if (!success) return false;
        const int second = ScanInt(&success);
        if (!success) return false;
        output->push_back(std::make_pair(first, second));
    }
}

bool ReadIntSetSection(std::set<int>* output, const char* closing_tag) {
    if (!output || !closing_tag) return false;
    output->clear();
    while (true) {
        bool is_type = false;
        bool success = false;
        std::string type;
        const int value = ScanTypeOrInt(is_type, type, success, true);
        if (is_type) return type == closing_tag;
        if (!success || !output->insert(value).second) return false;
    }
}

bool ReadRandomImageSection(std::vector<StackableRandomImageEntry>* output) {
    if (!output) return false;
    output->clear();
    while (true) {
        bool is_type = false;
        bool success = false;
        std::string type;
        StackableRandomImageEntry entry;
        entry.image_index = ScanTypeOrInt(is_type, type, success, true);
        if (is_type) return type == "[/chn random image percent]";
        if (!success) return false;
        entry.probability = ScanInt(&success);
        if (!success || entry.probability < 0) return false;
        entry.reward_item_id = ScanInt(&success);
        if (!success || ScanStr(&entry.body_animation) != 1 ||
            ScanStr(&entry.effect_animation) != 1) {
            return false;
        }
        output->push_back(entry);
    }
}

}  // namespace

// IDA_EA: 0x898F218
// IDA_NAME: _ZN17STStackableScriptC2Ev
// DEMANGLED: STStackableScript::STStackableScript(void)
STStackableScript::STStackableScript()
    : item(),
      arad(),
      values_190(),
      rabbit_boxes_19c(),
      continuous_option_1b9(),
      continuous_option_1bc(),
      string_1d8(),
      values_1ec(),
      usable_places_1f8(),
      passive_objects_208(),
      usable_server_types_214(),
      values_224(),
      strings_230(),
      values_244(),
      values_250(),
      values_25c(),
      package_data_268(),
      package_data_274(),
      booster_280(),
      package_data_2a4(),
      recovery_2b0(),
      recovery_2c4(),
      status_recovery_2d8(),
      status_generate_2ec(),
      buff_304(),
      summon_monster_314(),
      summon_apc_330(),
      stat_changes_34c(),
      string_368(),
      pairs_374(),
      map_3a4(),
      pairs_3c0(),
      limit_cube_3d4(),
      enchant_map_3f4(),
      string_410(),
      booster_select_41c(),
      pair_43c(),
      value_478(),
      avatar_option_change_47c(),
      set_480(),
      set_498(),
      string_4b4(),
      module_parameters_4bc(),
      set_4d4(),
      values_4fc(),
      set_50c(),
      limit_dungeon_ids(),
      add_socket_limited_avatar_ids(),
      exclusive_dungeon_ids(),
      avatar_booster_preview_item_ids(),
      stackable_appendage_scripts(),
      disguise_appendages(),
      emancipate_grow_groups(),
      emancipate_material_revisions(),
      stackable_action_limit_indexes(),
      random_image_entries() {
    Clear();
}

// IDA_EA: 0x898F8E6
// IDA_NAME: _ZN17STStackableScriptC2ERS_
// DEMANGLED: STStackableScript::STStackableScript(STStackableScript&)
STStackableScript::STStackableScript(STStackableScript& other)
    : STStackableScript() {
    *this = other;
}

// IDA_EA: 0x899013C
// IDA_NAME: _ZN17STStackableScriptD2Ev
// DEMANGLED: STStackableScript::~STStackableScript()
STStackableScript::~STStackableScript() {
    for (std::map<int, EquipmentParameterInfo*>::iterator it =
             module_parameters_4bc.begin();
         it != module_parameters_4bc.end();
         ++it) {
        delete it->second;
        it->second = 0;
    }
    module_parameters_4bc.clear();
}

// IDA_EA: 0x899087A
// IDA_NAME: _ZN17STStackableScript5ClearEv
// DEMANGLED: STStackableScript::Clear(void)
STStackableScript* STStackableScript::Clear() {
    item.Clear();
    value_18c = 0;
    arad.AradStClear();
    values_190.clear();
    rabbit_boxes_19c.clear();
    values_1a8[0] = 0;
    values_1a8[1] = 0;
    values_1a8[2] = 0;
    values_1a8[3] = -1;
    value_1b8 = 0;
    continuous_option_1b9.clear();
    continuous_option_1bc.clear();
    value_1bf = 0;
    exp_bonus_enabled_1c0 = 0;
    exp_bonus_rate_1c4 = 0.0f;
    value_1c8 = 0;
    pc_exp_bonus_rate_1cc = 0.0f;
    value_1d0 = 0;
    value_1d4 = 0;
    string_1d8.clear();
    value_1dc = 0;
    value_1e0 = 39;
    value_1e4 = 0;
    value_1e8 = 0;
    values_1ec.clear();
    usable_places_1f8.clear();
    use_action_packet_ = 0;
    daily_delete_item_ = 0;
    passive_objects_208.clear();
    usable_server_types_214.clear();
    value_220 = 0;
    values_224.clear();
    strings_230[0].clear();
    strings_230[1].clear();
    strings_230[2].clear();
    values_23c[0] = 1;
    values_23c[1] = -1;
    values_244.clear();
    values_250.clear();
    values_25c.clear();
    package_data_268.clear();
    package_data_274.clear();
    booster_280.clear();
    package_data_2a4.clear();
    recovery_2b0.clear();
    recovery_2c4.clear();
    status_recovery_2d8.clear();
    status_generate_2ec.clear();
    buff_304.clear();
    summon_monster_314.clear();
    summon_apc_330.clear();
    value_348 = -1;
    stat_changes_34c.clear();
    value_364 = 9999999;
    string_368.clear();
    value_36c = -1;
    value_370 = -1;
    pairs_374.clear();
    value_380 = 0;
    value_384 = 0;
    value_388 = 1;
    value_38c = -1;
    value_390 = 1;
    value_394 = 0;
    value_396 = 0;
    value_398 = 0;
    value_39a = 0;
    value_39b = 0;
    value_39c = 0;
    values_3a0[0] = 0;
    values_3a0[1] = 0;
    map_3a4.clear();
    value_3bc = 0;
    pairs_3c0.clear();
    value_3cc = 0;
    values_3d0[0] = 0;
    values_3d0[1] = 1;
    limit_cube_3d4.condition_items.clear();
    limit_cube_3d4.value_0c = 0;
    limit_cube_3d4.value_10 = 0;
    limit_cube_3d4.results.clear();
    enchant_map_3f4.clear();
    value_40c = 0xffu;
    value_40d = 0;
    value_40e = 0;
    string_410.clear();
    value_414 = 0;
    evolution_creature_id = 0;
    booster_select_41c = BoosterSelectInfo();
    value_438 = 0;
    pair_43c.first = 0;
    pair_43c.second = 0;
    value_444 = 11;
    // STStackableScript::Clear @ 0x899087A does not use one sentinel for
    // this mixed-purpose array.  In particular +0x448 is the optional
    // [minimum itemmaking level] and its native "tag absent" value is zero.
    // Initialising it to -1 makes every ordinary recipe truthy at
    // CItem::GetNeedLevel(), then sends the default type 11 past the
    // item-making type table.  Keep every slot at its recovered native
    // default instead of flattening the array to -1.
    values_448[0] = 0;
    for (int index = 1; index <= 8; ++index) {
        values_448[index] = -1;
    }
    values_448[9] = 0;
    values_448[10] = -1;
    values_448[11] = 0;
    value_478 = 0;
    avatar_option_change_47c.clear();
    set_480.clear();
    set_498.clear();
    value_4b0 = 0;
    string_4b4.clear();
    value_4b8 = 0;
    for (std::map<int, EquipmentParameterInfo*>::iterator it =
             module_parameters_4bc.begin();
         it != module_parameters_4bc.end();
         ++it) {
        delete it->second;
        it->second = 0;
    }
    module_parameters_4bc.clear();
    set_4d4.clear();
    values_4ec[0] = 0;
    values_4ec[1] = 0;
    values_4ec[2] = 0;
    values_4ec[3] = 0;
    values_4fc.clear();
    value_508 = 0;
    set_50c.clear();
    limit_dungeon_ids.clear();
    add_socket_limited_avatar_ids.clear();
    resent_day_value = 0;
    cera_coupon = 0;
    // CLIENT_EA: STStackableScript_Clear @ 0x0155F90D resets the client
    // field to enabled; ImportStackableScript @ 0x0156EE62 clears it only
    // for the explicit tooltip-off policy.
    exclusive_dungeon_tooltip_enabled = 1;
    reserved_55a[0] = 0;
    reserved_55a[1] = 0;
    exclusive_dungeon_ids.clear();
    avatar_booster_preview_item_ids.clear();
    stackable_appendage_scripts.clear();
    verify_stack_number = 0;
    reserved_58d[0] = 0;
    reserved_58d[1] = 0;
    reserved_58d[2] = 0;
    all_skill_plus = 0;
    // 0828 CLIENT_EA 0x0155F320 initializes STStackableScript +0x650 to
    // -1.  Zero is not the omitted-tag state and must not leak into the
    // server-side level-up ticket policy.
    levelup_mode = -1;
    disguise_appendages.clear();
    emancipate_grow_groups.clear();
    emancipate_material_revisions.clear();
    stackable_action_limit_indexes.clear();
    random_image_entries.clear();
    return this;
}

// IDA_EA: 0x89C418E
// IDA_NAME: _ZN17STStackableScriptaSERKS_
// DEMANGLED: STStackableScript::operator=(STStackableScript const&)
STStackableScript& STStackableScript::operator=(
    const STStackableScript& other) {
    if (this == &other) {
        return *this;
    }

    item = other.item;
    arad = other.arad;
    value_18c = other.value_18c;
    values_190 = other.values_190;
    rabbit_boxes_19c = other.rabbit_boxes_19c;
    for (int index = 0; index != 4; ++index) values_1a8[index] = other.values_1a8[index];
    value_1b8 = other.value_1b8;
    continuous_option_1b9 = other.continuous_option_1b9;
    continuous_option_1bc = other.continuous_option_1bc;
    value_1bf = other.value_1bf;
    exp_bonus_enabled_1c0 = other.exp_bonus_enabled_1c0;
    exp_bonus_rate_1c4 = other.exp_bonus_rate_1c4;
    value_1c8 = other.value_1c8;
    pc_exp_bonus_rate_1cc = other.pc_exp_bonus_rate_1cc;
    value_1d0 = other.value_1d0;
    value_1d4 = other.value_1d4;
    string_1d8 = other.string_1d8;
    value_1dc = other.value_1dc;
    value_1e0 = other.value_1e0;
    value_1e4 = other.value_1e4;
    value_1e8 = other.value_1e8;
    values_1ec = other.values_1ec;
    usable_places_1f8 = other.usable_places_1f8;
    use_action_packet_ = other.use_action_packet_;
    daily_delete_item_ = other.daily_delete_item_;
    passive_objects_208 = other.passive_objects_208;
    usable_server_types_214 = other.usable_server_types_214;
    value_220 = other.value_220;
    values_224 = other.values_224;
    strings_230[0] = other.strings_230[0];
    strings_230[1] = other.strings_230[1];
    strings_230[2] = other.strings_230[2];
    values_23c[0] = other.values_23c[0];
    values_23c[1] = other.values_23c[1];
    values_244 = other.values_244;
    values_250 = other.values_250;
    values_25c = other.values_25c;
    package_data_268 = other.package_data_268;
    package_data_274 = other.package_data_274;
    booster_280 = other.booster_280;
    package_data_2a4 = other.package_data_2a4;
    recovery_2b0 = other.recovery_2b0;
    recovery_2c4 = other.recovery_2c4;
    status_recovery_2d8 = other.status_recovery_2d8;
    status_generate_2ec = other.status_generate_2ec;
    buff_304 = other.buff_304;
    summon_monster_314 = other.summon_monster_314;
    summon_apc_330 = other.summon_apc_330;
    value_348 = other.value_348;
    stat_changes_34c = other.stat_changes_34c;
    value_364 = other.value_364;
    string_368 = other.string_368;
    value_36c = other.value_36c;
    value_370 = other.value_370;
    pairs_374 = other.pairs_374;
    value_380 = other.value_380;
    value_384 = other.value_384;
    value_388 = other.value_388;
    value_38c = other.value_38c;
    value_390 = other.value_390;
    value_394 = other.value_394;
    value_396 = other.value_396;
    value_398 = other.value_398;
    value_39a = other.value_39a;
    value_39b = other.value_39b;
    value_39c = other.value_39c;
    values_3a0[0] = other.values_3a0[0];
    values_3a0[1] = other.values_3a0[1];
    map_3a4 = other.map_3a4;
    value_3bc = other.value_3bc;
    pairs_3c0 = other.pairs_3c0;
    value_3cc = other.value_3cc;
    values_3d0[0] = other.values_3d0[0];
    values_3d0[1] = other.values_3d0[1];
    limit_cube_3d4 = other.limit_cube_3d4;
    enchant_map_3f4 = other.enchant_map_3f4;
    value_40c = other.value_40c;
    value_40d = other.value_40d;
    value_40e = other.value_40e;
    string_410 = other.string_410;
    value_414 = other.value_414;
    evolution_creature_id = other.evolution_creature_id;
    booster_select_41c = other.booster_select_41c;
    value_438 = other.value_438;
    pair_43c = other.pair_43c;
    value_444 = other.value_444;
    for (int index = 0; index != 12; ++index) values_448[index] = other.values_448[index];
    value_478 = other.value_478;
    avatar_option_change_47c = other.avatar_option_change_47c;
    set_480 = other.set_480;
    set_498 = other.set_498;
    value_4b0 = other.value_4b0;
    string_4b4 = other.string_4b4;
    value_4b8 = other.value_4b8;
    for (std::map<int, EquipmentParameterInfo*>::iterator it =
             module_parameters_4bc.begin();
         it != module_parameters_4bc.end();
         ++it) {
        delete it->second;
        it->second = 0;
    }
    module_parameters_4bc.clear();
    for (std::map<int, EquipmentParameterInfo*>::const_iterator it =
             other.module_parameters_4bc.begin();
         it != other.module_parameters_4bc.end();
         ++it) {
        module_parameters_4bc[it->first] =
            it->second ? new EquipmentParameterInfo(*it->second) : 0;
    }
    set_4d4 = other.set_4d4;
    for (int index = 0; index != 4; ++index) values_4ec[index] = other.values_4ec[index];
    values_4fc = other.values_4fc;
    value_508 = other.value_508;
    set_50c = other.set_50c;
    limit_dungeon_ids = other.limit_dungeon_ids;
    add_socket_limited_avatar_ids = other.add_socket_limited_avatar_ids;
    resent_day_value = other.resent_day_value;
    cera_coupon = other.cera_coupon;
    exclusive_dungeon_tooltip_enabled =
        other.exclusive_dungeon_tooltip_enabled;
    reserved_55a[0] = other.reserved_55a[0];
    reserved_55a[1] = other.reserved_55a[1];
    exclusive_dungeon_ids = other.exclusive_dungeon_ids;
    avatar_booster_preview_item_ids =
        other.avatar_booster_preview_item_ids;
    stackable_appendage_scripts = other.stackable_appendage_scripts;
    verify_stack_number = other.verify_stack_number;
    reserved_58d[0] = other.reserved_58d[0];
    reserved_58d[1] = other.reserved_58d[1];
    reserved_58d[2] = other.reserved_58d[2];
    all_skill_plus = other.all_skill_plus;
    levelup_mode = other.levelup_mode;
    disguise_appendages = other.disguise_appendages;
    emancipate_grow_groups = other.emancipate_grow_groups;
    emancipate_material_revisions =
        other.emancipate_material_revisions;
    stackable_action_limit_indexes =
        other.stackable_action_limit_indexes;
    random_image_entries = other.random_image_entries;
    return *this;
}

// IDA_EA: 0x89AE906
// IDA_NAME: _Z33StackableScriptSpecialMonsterDropP17STStackableScriptSsPKc
// DEMANGLED: StackableScriptSpecialMonsterDrop(STStackableScript *,std::string,char const*)
int StackableScriptSpecialMonsterDrop(STStackableScript* script,
                                      std::string tag,
                                      const char* filename) {
    (void)filename;

    if (tag == "[special monster drop]") {
        bool success = false;
        script->item.special_monster_drop.enabled =
            ScanInt(&success) != 0;
        return success ? 1 : 0;
    }

    std::string value;
    if (tag == "[difficulty for extra drop]") {
        while (ScanStr(&value) == 1) {
            if (value == "[easy]") {
                script->item.special_monster_drop.extra_drop_easy = 1;
            } else if (value == "[medium]") {
                script->item.special_monster_drop.extra_drop_medium = 1;
            } else if (value == "[hard]") {
                script->item.special_monster_drop.extra_drop_hard = 1;
            } else if (value == "[ultimate]") {
                script->item.special_monster_drop.extra_drop_ultimate = 1;
            } else if (value == "[hero]") {
                script->item.special_monster_drop.extra_drop_hero = 1;
            } else if (value == "[all]") {
                script->item.special_monster_drop.extra_drop_easy = 1;
                script->item.special_monster_drop.extra_drop_medium = 1;
                script->item.special_monster_drop.extra_drop_hard = 1;
                script->item.special_monster_drop.extra_drop_ultimate = 1;
                script->item.special_monster_drop.extra_drop_hero = 1;
            }
        }
    } else if (tag == "[dungeon type for extra drop]") {
        while (ScanStr(&value) == 1) {
            if (value == "[normal]") {
                script->item.special_monster_drop.dungeon_normal = 1;
            } else if (value == "[ancient]") {
                script->item.special_monster_drop.dungeon_ancient = 1;
            } else if (value == "[dimension]") {
                script->item.special_monster_drop.dungeon_dimension = 1;
            } else if (value == "[all]") {
                script->item.special_monster_drop.dungeon_normal = 1;
                script->item.special_monster_drop.dungeon_ancient = 1;
                script->item.special_monster_drop.dungeon_dimension = 1;
            }
        }
    }
    return 1;
}

// IDA_EA: 0x89AEB9B
// IDA_NAME: _Z22getStackableActionTypeRSs
// DEMANGLED: getStackableActionType(std::string &)
// CLIENT_EA: 0x01541850
// CLIENT_NAME: GetStackableActionTypeByName
int getStackableActionType(std::string& tag) {
    if (tag == "[megaphone]") return 22;
    if (tag == "[account megaphone]") return 23;
    if (tag == "[upgrade cargo]") return 44;
    if (tag == "[chatemoticon]") return 45;
    if (tag == "[seria room deco]") return 43;
    if (tag == "[open guide book]") return 46;
    if (tag == "[class change]") return 47;
    if (tag == "[character name change]") return 51;
    if (tag == "[bingo]") return 52;
    if (tag == "[portable upgrade]") return 53;
    if (tag == "[add fatigue]") return 55;
    if (tag == "[init dimension dungeon count]") return 56;
    if (tag == "[candle golden wax]") return 7;
    if (tag == "[add avatar socket]") return 33;
    if (tag == "[LotteryCount]") return 54;
    if (tag == "[level up ticket]") return 38;
    if (tag == "[event extexsion day item]") return 60;
    if (tag == "[equipment purify]") return 34;
    if (tag == "[emancipate growth item upgrade]") return 58;
    if (tag == "[tournament ticket]") return 57;
    if (tag == "[cargo transport stone]") return 59;
    if (tag == "[limited cube]") return 4;
    if (tag == "[change creature]" || tag == "[change creature list]") {
        return kChangeCreatureActionType;
    }
    if (tag == "[change avatar]") return 62;
    if (tag == "[3rd chronicle ability enchant]") return 3;
    if (tag == "[up layer despair dungeon]") return 67;
    if (tag == "[portable genuine damage upgrade upgrede]") return 70;
    if (tag == "[portable amplify]") return 71;
    return 0;
}

// IDA_EA: 0x89AED67
// IDA_NAME: _Z29getStackableActionUsablePlaceRSs
// DEMANGLED: getStackableActionUsablePlace(std::string &)
int getStackableActionUsablePlace(std::string& tag) {
    if (tag == "[village]") return 1;
    if (tag == "[dungeon]") return 2;
    if (tag == "[pvp]") return 3;
    if (tag == "[seria room]") return 4;
    if (tag == "[dont use assult]") return 5;
    return 1;
}

// IDA_EA: 0x89AEE0A
// IDA_NAME: _Z34getStackableActionUsableServerTypeRSs
// DEMANGLED: getStackableActionUsableServerType(std::string &)
int getStackableActionUsableServerType(std::string& tag) {
    if (tag == "[general server]") return 1;
    if (tag == "[event server]") return 2;
    if (tag == "[starter server]") return 3;
    return 1;
}

// IDA_EA: 0x89AEE71
// IDA_NAME: _Z30getStackableIncreaseStatusTypeRSs
// DEMANGLED: getStackableIncreaseStatusType(std::string &)
bool getStackableIncreaseStatusType(std::string& tag) {
    return tag == "[expUp]";
}

// IDA_EA: 0x89AEEC8
// IDA_NAME: _Z21ImportStackableScriptP17STStackableScriptPKcbi
// DEMANGLED: ImportStackableScript(STStackableScript *,char const*,bool,int)
int ImportStackableScript(STStackableScript* script,
                          const char* filename,
                          bool unknown,
                          int item_id) {
    if (!script || !filename) {
        return 0;
    }
    if (loadRDARScriptFile(g_szStackableScriptBaseDirectory, filename) != 1) {
        return 0;
    }

    script->Clear();
    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[name]") {
            ScanStr(&script->item.strings_ac[0]);
            if (unknown) {
                break;
            }
        } else if (tag == "[name2]") {
            ScanStr(&script->item.strings_ac[1]);
        } else if (tag == "[explain]" || tag == "[explain2]") {
            ScanStr(&script->item.strings_ac[2]);
        } else if (tag == "[flavor text]") {
            ScanStr(&script->item.strings_ac[3]);
        } else if (tag == "[grade]") {
            script->item.grade = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[attach type]") {
            std::string type;
            if (ScanStr(&type) != 1) {
                return 0;
            }
            if (type == "[free]") {
                script->item.values_08[0] = 0;
            } else if (type == "[trade]") {
                script->item.values_08[0] = 1;
            } else if (type == "[trade delete]") {
                script->item.values_08[0] = 2;
            } else if (type == "[sealing]") {
                script->item.values_08[0] = 3;
            } else if (type == "[account]") {
                script->item.values_08[0] = 5;
            } else if (type == "[trade limit]") {
                script->item.values_08[0] = 7;
            }
        } else if (tag == "[creation rate]") {
            script->item.values_08[1] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[rarity]") {
            script->item.values_08[2] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[weight]") {
            script->item.value_34 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[icon]") {
            if (ScanStr(&script->item.icon) != 1) {
                return 0;
            }
            script->item.icon_index = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[field image]") {
            if (ScanStr(&script->item.field_image) != 1) {
                return 0;
            }
            script->item.field_image_index = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[tarotcard view]") {
            if (ScanStr(&script->item.tarotcard_view) != 1) {
                return 0;
            }
            script->item.tarotcard_view_index = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[usable job]") {
            std::string job_name;
            while (ScanStr(&job_name) == 1) {
                const int job = ::getCharacterJob(job_name.c_str());
                if (job == oldsrv::config::kCharacterJobCommon) {
                    for (int index = 0;
                         index != oldsrv::config::kCharacterJobCount;
                         ++index) {
                        script->item.usable_job_flags[index] = 1;
                    }
                    script->item.value_a0 = 1;
                } else if (job >= 0 &&
                           job < oldsrv::config::kCharacterJobCount) {
                    script->item.usable_job_flags[job] = 1;
                }
            }
        } else if (tag == "[impossible jobs]") {
            // 0828 uses this as a subtraction from [usable job].  The
            // official EXP capsules and avatar bind cubes explicitly name
            // swordman/at swordman here, and CItem::check_job_type consumes
            // this same per-job flag array.  Keep the shared CItem owner
            // instead of inventing a second Stackable-only deny list.
            std::string job_name;
            bool saw_job = false;
            while (ScanStr(&job_name) == 1) {
                // 0828 stackables use the unbracketed enum spelling here
                // (for example "at swordman"), whereas [usable job] uses
                // bracketed strings such as "[all]".  getCharacterJob only
                // recognizes the bracketed table and maps an unknown value to
                // common, which made cube_goldbind.stk fail at startup.  Use
                // the official unbracketed converter first, while retaining
                // compatibility with older bracketed impossible-job rows.
                int job = ::getCharacterJobToEnumValue(job_name);
                if (job < 0) {
                    job = ::getCharacterJob(job_name.c_str());
                }
                if (job < 0 || job >= oldsrv::config::kCharacterJobCount) {
                    return 0;
                }
                script->item.usable_job_flags[job] = 0;
                saw_job = true;
            }
            if (!saw_job) {
                return 0;
            }
        } else if (tag == "[suitable job]") {
            script->item.usable_jobs.clear();
            std::string job_name;
            while (ScanStr(&job_name) == 1) {
                const int job = ::getCharacterJob(job_name.c_str());
                if (job == oldsrv::config::kCharacterJobCommon) {
                    for (int index = 0;
                         index != oldsrv::config::kCharacterJobCount;
                         ++index) {
                        script->item.usable_jobs.insert(index);
                    }
                } else if (job >= 0 &&
                           job < oldsrv::config::kCharacterJobCount) {
                    script->item.usable_jobs.insert(job);
                }
            }
        } else if (tag == "[minimum level]") {
            script->item.values_5c[0] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[max having count]") {
            script->item.values_5c[1] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[minimum rank]") {
            script->item.values_5c[2] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[cool time]") {
            script->item.values_5c[3] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[maximum level]") {
            script->values_1a8[1] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[usable period]") {
            script->item.values_f4[0] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[expiration date]") {
            script->item.values_f4[1] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[price]") {
            script->item.values_08[3] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[cash]") {
            script->item.values_08[4] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[medal]") {
            script->item.values_08[5] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[finish point price]") {
            script->item.values_08[6] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[value]") {
            script->item.value_24 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[need material]") {
            script->item.value_28 = ScanInt(static_cast<bool*>(0));
            script->item.value_2c = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[recieve postal message]") {
            if (ScanStr(&script->item.value_174) != 1) {
                return 0;
            }
        } else if (tag == "[mail from]") {
            if (ScanStr(&script->arad.value_0C) != 1) {
                return 0;
            }
        } else if (tag == "[mail content]") {
            if (ScanStr(&script->arad.value_10) != 1) {
                return 0;
            }
        } else if (tag == "[arad_item_kind]") {
            if (ScanStr(&script->arad.value_08) != 1) {
                return 0;
            }
        } else if (tag == "[arad_use]") {
            bool success = false;
            script->arad.value_00 = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[usable first growtype]") {
            bool success = false;
            script->arad.value_04 =
                static_cast<unsigned char>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[usable date]") {
            ScanInt(static_cast<bool*>(0));
            ScanInt(static_cast<bool*>(0));
        } else if (tag == "[item group name]") {
            std::string group_name;
            if (ScanStr(&group_name) != 1) {
                return 0;
            }
            std::map<std::string, int>* groups = getItemGroupNameTable();
            std::map<std::string, int>::const_iterator found =
                groups->find(group_name);
            if (found == groups->end()) {
                return 0;
            }
            script->item.value_100 = found->second;
        } else if (tag == "[npc gift disallowance]") {
            bool success = false;
            script->item.value_104 =
                static_cast<unsigned char>(ScanInt(&success) != 1);
            if (!success) {
                return 0;
            }
        } else if (tag == "[guild power war point]") {
            script->item.values_c4[1] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[user power war point]") {
            script->item.values_c4[2] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[int data]") {
            script->item.int_data.clear();
            bool success = false;
            int value = ScanInt(&success);
            while (success) {
                script->item.int_data.push_back(value);
                value = ScanInt(&success);
            }
        } else if (tag == "[event reward item]") {
            // Shared 0828 common-item marker.  Equipment and stackable
            // importers intentionally publish through the same owner.
            item_script_0828::SetEventRewardItem(&script->item, true);
        } else if (tag == "[stackable appendage]") {
            if (!ReadStringSection(&script->stackable_appendage_scripts,
                                   "[/stackable appendage]")) {
                return 0;
            }
        } else if (tag == "[verify stack number]") {
            // CLIENT_EA 0x01568E89 writes byte STStackableScript+0x77c.
            script->verify_stack_number = 1;
        } else if (tag == "[all skill plus]") {
            bool success = false;
            script->all_skill_plus = ScanInt(&success);
            if (!success) return 0;
        } else if (tag == "[disguise appendage]") {
            if (!ReadDisguiseAppendageSection(&script->disguise_appendages)) {
                return 0;
            }
        } else if (tag == "[emancipate grow apply]") {
            if (!ReadEmancipateGrowApplySection(
                    &script->emancipate_grow_groups)) {
                return 0;
            }
        } else if (tag == "[emancipate material revision]") {
            if (!ReadIntPairSection(&script->emancipate_material_revisions,
                                    "[/emancipate material revision]")) {
                return 0;
            }
        } else if (tag == "[stackable action limit indexes]") {
            if (!ReadIntSetSection(&script->stackable_action_limit_indexes,
                                   "[/stackable action limit indexes]")) {
                return 0;
            }
        } else if (tag == "[chn random image percent]") {
            if (!ReadRandomImageSection(&script->random_image_entries)) {
                return 0;
            }
        } else if (tag == "[string data]") {
            script->item.string_data.clear();
            std::string value;
            while (ScanStr(&value) == 1) {
                script->item.string_data.push_back(value);
            }
        } else if (tag == "[packagable]") {
            script->item.value_fc =
                ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[icon mark]") {
            if (ScanStr(&script->item.strings_ac[5]) != 1) {
                return 0;
            }
            script->item.values_c4[0] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[auction free]") {
            script->item.value_30 = 1;
        } else if (tag == "[hidden option]") {
            script->item.value_31 = 1;
        } else if (tag == "[impossible contents]") {
            std::string content;
            while (ScanStr(&content) == 1) {
                if (content == "gift") {
                    script->item.value_d0 = 1;
                } else if (content == "trade") {
                    script->item.value_d1 = 1;
                } else if (content == "charac cargo") {
                    script->item.value_da = 1;
                } else if (content == "ban redeemitem") {
                    script->item.value_d8 = 1;
                }
            }
        } else if (tag == "[usable upgrade chracter level]") {
            script->item.usable_upgrade_character_level =
                static_cast<unsigned short>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[stack limit]") {
            script->value_364 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[cooltime group]") {
            script->values_1a8[3] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[party cooltime]") {
            script->value_1b8 =
                ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[cooltime maintenance]") {
            script->continuous_option_1b9.option_type =
                ScanInt(static_cast<bool*>(0)) != 0;
            script->continuous_option_1b9.option_value =
                ScanInt(static_cast<bool*>(0)) != 0;
            script->continuous_option_1b9.enabled =
                ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[effect maintenance]") {
            script->continuous_option_1bc.option_type =
                ScanInt(static_cast<bool*>(0)) != 0;
            script->continuous_option_1bc.option_value =
                ScanInt(static_cast<bool*>(0)) != 0;
            script->continuous_option_1bc.enabled =
                ScanInt(static_cast<bool*>(0)) != 0;
            script->value_1bf = ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[exp bonus rate]") {
            script->exp_bonus_enabled_1c0 = 1;
            script->exp_bonus_rate_1c4 = ScanFloat(static_cast<bool*>(0));
        } else if (tag == "[pc exp bonus rate]") {
            script->value_1c8 = 1;
            script->pc_exp_bonus_rate_1cc =
                ScanFloat(static_cast<bool*>(0));
        } else if (tag == "[shared effect]") {
            bool success = false;
            const int value = ScanInt(&success);
            if (success) {
                script->value_1d0 = static_cast<unsigned char>(value == 1);
            }
        } else if (tag == "[daily delete item]") {
            bool success = false;
            const int value = ScanInt(&success);
            if (success) {
                script->daily_delete_item_ = static_cast<unsigned char>(value);
            }
        } else if (tag == "[use action packet]") {
            bool success = false;
            const int value = ScanInt(&success);
            if (success) {
                script->use_action_packet_ =
                    static_cast<unsigned char>(value != 0);
            }
        } else if (tag == "[collision item]") {
            script->values_190.clear();
            bool success = false;
            int value = ScanInt(&success);
            while (success) {
                script->values_190.push_back(value);
                value = ScanInt(&success);
            }
        } else if (tag == "[increase status type]") {
            std::string type;
            if (ScanStr(&type) != 1) {
                return 0;
            }
            script->value_220 = getStackableIncreaseStatusType(type);
            bool success = false;
            int value = ScanInt(&success);
            while (success) {
                script->values_224.push_back(value);
                value = ScanInt(&success);
            }
        } else if (tag == "[my status]") {
            script->status_generate_2ec.enabled = 1;
            script->status_generate_2ec.values.push_back(6.0f);

            std::string status;
            ScanStr(&status);
            const float status_type =
                static_cast<float>(getActiveStatusToEnumValue(status));
            script->status_generate_2ec.value_10 =
                ScanInt(static_cast<bool*>(0));
            script->status_generate_2ec.value_0c =
                ScanInt(static_cast<bool*>(0));
            const float value =
                static_cast<float>(ScanInt(static_cast<bool*>(0)));
            script->status_generate_2ec.values.push_back(status_type);
            script->status_generate_2ec.values.push_back(value);

            std::string option;
            while (ScanStr(&option) == 1) {
                int value_count = 0;
                if (option == "blind" || option == "slow" ||
                    option == "haste") {
                    value_count = 2;
                } else if (option == "stone" || option == "confuse" ||
                           option == "weakness" ||
                           option == "armor break" ||
                           option == "weapon break" || option == "poison" ||
                           option == "lightning" || option == "bleeding") {
                    value_count = 1;
                } else if (option == "burn" || option == "bless" ||
                           option == "curse") {
                    value_count = 4;
                }
                for (int index = 0; index != value_count; ++index) {
                    script->status_generate_2ec.values.push_back(
                        ScanFloat(static_cast<bool*>(0)));
                }
            }
        } else if (tag == "[passive object in stackable]") {
            bool success = true;
            while (success) {
                STPassiveObjectInStackable passive_object;
                passive_object.values_00[0] = ScanInt(&success);
                if (!success) {
                    break;
                }
                // The old branch uses the same token-success flag but only
                // tests it after the leading field, before pushing the row.
                for (int index = 1; index != 5; ++index) {
                    passive_object.values_00[index] = ScanInt(&success);
                }
                script->passive_objects_208.push_back(passive_object);
            }
        } else if (tag == "[package data]") {
            script->package_data_268.clear();
            bool success = false;
            int item_index = ScanInt(&success);
            while (success) {
                STStackablePackageData package_data;
                package_data.item_index = item_index;
                package_data.item_count = ScanInt(static_cast<bool*>(0));
                script->package_data_268.push_back(package_data);
                item_index = ScanInt(&success);
            }
        } else if (tag == "[secret add item]") {
            script->package_data_2a4.clear();
            bool success = false;
            int item_index = ScanInt(&success);
            while (success) {
                STStackablePackageData package_data;
                package_data.item_index = item_index;
                package_data.item_count = ScanInt(static_cast<bool*>(0));
                script->package_data_2a4.push_back(package_data);
                item_index = ScanInt(&success);
            }
        } else if (tag == "[package data selection]") {
            std::vector<STStackablePackageData> package_selection;
            bool success = false;
            int item_index = ScanInt(&success);
            while (success) {
                STStackablePackageData package_data;
                package_data.item_index = item_index;
                package_data.item_count = ScanInt(static_cast<bool*>(0));
                package_selection.push_back(package_data);
                item_index = ScanInt(&success);
            }
            script->package_data_274.push_back(package_selection);
        } else if (tag == "[hp recovery]" || tag == "[mp recovery]") {
            STRecovery& recovery = tag == "[hp recovery]"
                ? script->recovery_2b0
                : script->recovery_2c4;
            std::string sign;
            ScanStr(&sign);
            recovery.sign = static_cast<unsigned char>(sign == "+" ? 0 : 1);
            recovery.value = ScanInt(static_cast<bool*>(0));
            recovery.duration = ScanInt(static_cast<bool*>(0));
            std::string target;
            ScanStr(&target);
            recovery.target = target == "myself" ? 0 : 1;
            recovery.enabled = 1;
        } else if (tag == "[active status recovery]") {
            // IDA writes +0x04/+0x08/+0x0c of STStatusRecovery for the
            // active-status form; +0x00 remains the exception-status slot.
            std::string status;
            ScanStr(&status);
            script->status_recovery_2d8.value =
                getActiveStatusToEnumValue(status);
            script->status_recovery_2d8.target =
                ScanInt(static_cast<bool*>(0));
            std::string target;
            ScanStr(&target);
            script->status_recovery_2d8.value_0c =
                target == "myself" ? 0 : 1;
            script->status_recovery_2d8.enabled = 1;
        } else if (tag == "[except status recovery]") {
            // IDA writes +0x00/+0x08/+0x0c for the exception-status form;
            // +0x04 remains the active-status slot.
            std::string status;
            ScanStr(&status);
            script->status_recovery_2d8.status_type =
                getExcepttionStatusToEnumValue(status);
            script->status_recovery_2d8.target =
                ScanInt(static_cast<bool*>(0));
            std::string target;
            ScanStr(&target);
            script->status_recovery_2d8.value_0c =
                target == "myself" ? 0 : 1;
            script->status_recovery_2d8.enabled = 1;
        } else if (tag == "[buff]") {
            std::string buff;
            ScanStr(&buff);
            script->buff_304.buff_type = getBuffToEnumValue(buff);
            script->buff_304.value = ScanInt(static_cast<bool*>(0));
            std::string target;
            ScanStr(&target);
            script->buff_304.target = target == "myself" ? 0 : 1;
            script->buff_304.enabled = 1;
        } else if (tag == "[monster summons]" ||
                   tag == "[event monster summons]") {
            for (int index = 0; index != 6; ++index) {
                script->summon_monster_314.values_00[index] =
                    ScanInt(static_cast<bool*>(0));
            }
            script->summon_monster_314.enabled = 1;
        } else if (tag == "[summon apc]") {
            for (int index = 0; index != 5; ++index) {
                script->summon_apc_330.values_00[index] =
                    ScanInt(static_cast<bool*>(0));
            }
            script->summon_apc_330.enabled = 1;
        } else if (tag == "[summon npc]") {
            script->value_348 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[advance altar need gage]") {
            script->values_4ec[0] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[guild exp amount]") {
            script->value_384 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[do not delete]") {
            if (script->value_1e0 == 16) {
                script->values_3d0[1] = 0;
            }
        } else if (tag == "[use 2nd upgrade table]") {
            script->values_3d0[0] = 1;
        } else if (tag == "[limit dungeon]") {
            script->limit_dungeon_ids.clear();
            bool success = false;
            int dungeon_id = ScanInt(&success);
            if (!success) {
                return 0;
            }
            while (success) {
                script->limit_dungeon_ids.insert(dungeon_id);
                dungeon_id = ScanInt(&success);
            }
        } else if (tag == "[bead limited usable item]") {
            script->set_50c.clear();
            bool success = false;
            int item_id_value = ScanInt(&success);
            if (!success) {
                return 0;
            }
            while (success) {
                script->set_50c.insert(item_id_value);
                item_id_value = ScanInt(&success);
            }
        } else if (tag == "[add socket limited avatar]") {
            script->add_socket_limited_avatar_ids.clear();
            bool success = false;
            int avatar_item_id = ScanInt(&success);
            if (!success) {
                return 0;
            }
            while (success) {
                script->add_socket_limited_avatar_ids.insert(avatar_item_id);
                avatar_item_id = ScanInt(&success);
            }
        } else if (tag == "[resent day]") {
            bool success = false;
            const int value = ScanInt(&success);
            if (!success) {
                return 0;
            }
            script->resent_day_value = value < 0 ? 0 : value;
        } else if (tag == "[cera coupon]") {
            bool success = false;
            const int value = ScanInt(&success);
            if (!success) {
                return 0;
            }
            script->cera_coupon = static_cast<uint8_t>(value > 0);
        } else if (tag == "[exclusive dungeon]") {
            std::string tooltip_policy;
            if (ScanStr(&tooltip_policy) != 1) {
                return 0;
            }
            script->exclusive_dungeon_tooltip_enabled =
                static_cast<uint8_t>(tooltip_policy != "tooltip off");
            script->exclusive_dungeon_ids.clear();
            bool success = false;
            int dungeon_id = ScanInt(&success);
            if (!success) {
                return 0;
            }
            while (success) {
                script->exclusive_dungeon_ids.insert(dungeon_id);
                dungeon_id = ScanInt(&success);
            }
        } else if (tag == "[avatar booster preview]") {
            script->avatar_booster_preview_item_ids.clear();
            bool success = false;
            int avatar_item_id = ScanInt(&success);
            if (!success) {
                return 0;
            }
            while (success) {
                script->avatar_booster_preview_item_ids.push_back(
                    avatar_item_id);
                avatar_item_id = ScanInt(&success);
            }
        } else if (tag == "[disable]") {
            script->value_388 = 0;
        } else if (tag == "[use item emoticon index]") {
            script->value_38c = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[use effect explain]") {
            script->value_390 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[expert type]") {
            std::string expert_type;
            ScanStr(&expert_type);
            if (expert_type == "[alchemist]") {
                script->value_3bc = 2;
            } else if (expert_type == "[enchanter]") {
                script->value_3bc = 1;
            } else if (expert_type == "[doll_controller]") {
                script->value_3bc = 4;
            } else {
                script->value_3bc = 0;
            }
        } else if (tag == "[bead item]") {
            bool success = false;
            int first = ScanInt(&success);
            while (success) {
                const int second = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->pairs_3c0.push_back(std::make_pair(first, second));
                first = ScanInt(&success);
            }
        } else if (tag == "[monster card id]") {
            script->value_3cc = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[shop doll]") {
            bool success = false;
            script->values_448[2] = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[unseal scroll level limit]") {
            bool success = false;
            script->values_448[3] = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[monstercard bind]") {
            bool success = false;
            script->values_448[4] = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[alchemist extraction]") {
            bool success = false;
            script->values_448[5] = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[enchanter extraction]") {
            bool success = false;
            script->values_448[6] = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[doll_controller extraction]") {
            bool success = false;
            script->values_448[7] = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[unlimited waste]") {
            bool success = false;
            script->values_448[10] = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[minimum itemmaking level]") {
            std::string equipment_type;
            if (ScanStr(&equipment_type) != 1) {
                return 0;
            }
            int type = -1;
            if (equipment_type == "weapon") type = 0;
            else if (equipment_type == "cloth") type = 1;
            else if (equipment_type == "leather") type = 2;
            else if (equipment_type == "larmor") type = 3;
            else if (equipment_type == "harmor") type = 4;
            else if (equipment_type == "plate") type = 5;
            else if (equipment_type == "amulet") type = 6;
            else if (equipment_type == "wrist") type = 7;
            else if (equipment_type == "ring") type = 8;
            else if (equipment_type == "support") type = 9;
            else if (equipment_type == "magic stone") type = 10;
            if (type == -1) {
                return 0;
            }
            script->value_444 = type;
            if (ScanInt(&script->values_448[0]) != 1) {
                return 0;
            }
        } else if (tag == "[type]") {
            bool success = false;
            const int value = ScanInt(&success);
            if (!success) {
                return 0;
            }
            script->value_40c = static_cast<unsigned char>(value);
        } else if (tag == "[evolution creature]") {
            bool success = false;
            script->evolution_creature_id = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[onemore check]") {
            bool success = false;
            ScanInt(&success);
            if (success) {
                script->value_414 = 1;
                ScanStr(&script->string_410);
            }
        } else if (tag == "[usable game server type]") {
            bool success = false;
            script->values_448[8] = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[levelup range]") {
            bool success = false;
            script->values_448[9] = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[levelup mode]") {
            // CLIENT_EA 0x015688DD stores this scalar at the 0828 script
            // tail; it is independent from the existing levelup range.
            bool success = false;
            script->levelup_mode = ScanInt(&success);
            if (!success) return 0;
        } else if (tag == "[lottery use cost]") {
            bool success = false;
            script->values_448[11] = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[chat notice gold]") {
            bool success = false;
            ScanInt(&success);
            if (!success) {
                return 0;
            }
            script->value_478 = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[random option unseal]") {
            script->value_4b8 = 0;
            script->value_4b8 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[stat desc]") {
            if (ScanStr(&script->string_4b4) != 1) {
                return 0;
            }
        } else if (tag == "[success rate]") {
            script->values_4ec[1] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[broadcast type]") {
            script->values_4ec[2] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[maintain ability]") {
            script->values_4ec[3] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[fail output]") {
            script->values_4fc.clear();
            bool success = false;
            int value = ScanInt(&success);
            while (success) {
                script->values_4fc.push_back(value);
                value = ScanInt(&success);
            }
        } else if (tag == "[trade limit max]") {
            bool success = false;
            const int value = ScanInt(&success);
            if (success) {
                script->value_508 = value;
            }
        } else if (tag == "[target item id]") {
            bool success = false;
            int item_index = ScanInt(&success);
            while (success) {
                script->set_4d4.insert(item_index);
                item_index = ScanInt(&success);
            }
        } else if (tag == "[3choro enchant]") {
            std::string section;
            if (ScanType(section, true) != 1 || section != "[probability]") {
                return 0;
            }
            bool success = false;
            script->value_40d =
                static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                return 0;
            }
            script->value_40e =
                static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                return 0;
            }

            while (true) {
                if (ScanType(section, true) != 1) {
                    return 0;
                }
                if (section == "[/3choro enchant]") {
                    break;
                }
                if (section != "[check]") {
                    return 0;
                }

                STEnchantSystemMapKey key;
                STEnchantSystemMapData data;
                key.value_00 = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                key.value_04 = static_cast<uint8_t>(ScanInt(&success));
                if (!success) {
                    return 0;
                }
                std::string equipment_type;
                if (ScanStr(&equipment_type) != 1) {
                    return 0;
                }
                key.value_08 = getEquipmentTypeToEnumValue(equipment_type);

                // CLIENT_EA: ImportStackableScript @ 0x156D9CC. The client
                // consumes these IDs into a check-local container only.
                std::vector<int> hidden_skill_indices;
                while (true) {
                    if (ScanType(section, true) != 1) {
                        return 0;
                    }
                    if (section == "[hide skill]") {
                        hidden_skill_indices.clear();
                        bool hide_success = false;
                        int hidden_skill = ScanInt(&hide_success);
                        while (hide_success) {
                            hidden_skill_indices.push_back(hidden_skill);
                            hidden_skill = ScanInt(&hide_success);
                        }
                        continue;
                    }
                    if (section == "[/hide skill]") {
                        continue;
                    }
                    if (section != "[skill]") {
                        break;
                    }
                    key.job = static_cast<uint8_t>(ScanInt(&success));
                    if (!success) {
                        return 0;
                    }
                    data.skill_data.clear();
                    importSkillDataUp(data.skill_data);
                    if (ScanType(section, true) != 1) {
                        return 0;
                    }
                    if (section == "[skill explain]") {
                        ScanStr(&data.skill_explain);
                    }
                    script->enchant_map_3f4.insert(
                        std::make_pair(key, data));
                }
                if (section != "[/check]") {
                    return 0;
                }
            }
        } else if (tag == "[enchant]") {
            // The old parser opens module 19 first, and switches to module
            // 18 only after the nested [pvp] delimiter.  Both records use
            // the same EquipmentParameterInfo grammar as equipment scripts.
            EquipmentParameterInfo* active_parameter = 0;
            std::map<int, EquipmentParameterInfo*>::iterator parameter_it =
                script->module_parameters_4bc.find(19);
            if (parameter_it == script->module_parameters_4bc.end()) {
                active_parameter = new EquipmentParameterInfo();
                script->module_parameters_4bc.insert(
                    std::make_pair(19, active_parameter));
            } else {
                active_parameter = parameter_it->second;
            }
            active_parameter->values_280[101].integer = 2;
            int parameter_first = 0;
            int parameter_second = 0;

            std::string section;
            while (true) {
                // In the original ImportStackableScript [enchant] loop, the
                // return value of ScanType is intentionally ignored.
                // ScanType advances past scalar values and leaves the next
                // type tag in section.  In particular, [cool time] 10000
                // must be followed by processing [if], not by terminating
                // this block when ScanType reports the intervening scalar.
                ScanType(section, true);
                if (section == "[/enchant]") {
                    break;
                }
                if (section == "[pvp]") {
                    parameter_it = script->module_parameters_4bc.find(18);
                    if (parameter_it == script->module_parameters_4bc.end()) {
                        active_parameter = new EquipmentParameterInfo();
                        script->module_parameters_4bc.insert(
                            std::make_pair(18, active_parameter));
                    } else {
                        active_parameter = parameter_it->second;
                    }
                    active_parameter->values_280[101].integer = 2;
                    parameter_first = 0;
                    parameter_second = 0;
                    continue;
                }
                // IDA deliberately ignores this return value: unhandled
                // parameter tags remain owned by the enclosing enchant block.
                importEquipmentParameterInfo(active_parameter,
                                             section,
                                             filename,
                                             parameter_first,
                                             parameter_second);
            }
        } else if (tag == "[portable disjoint]") {
            bool success = false;
            script->values_448[1] = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[equipment upgrade]") {
            bool success = false;
            const int enabled = ScanInt(&success);
            if (success) {
                script->booster_select_41c.enabled =
                    static_cast<uint8_t>(enabled);
            }
        } else if (tag == "[booster category name]") {
            std::string filename;
            while (ScanStr(&filename) == 1) {
                script->booster_select_41c.filenames.push_back(filename);
            }
        } else if (tag == "[booster selection num]") {
            bool success = false;
            const int count = ScanInt(&success);
            if (success) {
                script->booster_select_41c.value_08 = count;
            }
        } else if (tag == "[booster category num]") {
            bool success = false;
            const int count = ScanInt(&success);
            if (success && count <= 2) {
                if (count <= 0) {
                    script->booster_select_41c.value_00 = 0;
                    script->booster_select_41c.value_04 = 0;
                } else {
                    const int first = ScanInt(&success);
                    if (success) {
                        if (count == 1) {
                            script->booster_select_41c.value_00 = first;
                            script->booster_select_41c.value_04 = 0;
                        } else {
                            const int second = ScanInt(&success);
                            if (success) {
                                script->booster_select_41c.value_00 = first;
                                script->booster_select_41c.value_04 = second;
                            }
                        }
                    }
                }
            }
        } else if (tag == "[avatar package preview info]") {
            ScanStr(&script->string_368);
            script->value_36c = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[creature preview info]") {
            script->value_370 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[consume item]") {
            bool success = false;
            int item_index = ScanInt(&success);
            while (success) {
                const int count = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->pairs_374.push_back(
                    std::make_pair(item_index, count));
                item_index = ScanInt(&success);
            }
        } else if (tag == "[avatar emblem target type]") {
            std::string emblem;
            while (ScanStr(&emblem) == 1) {
                const int type = getAvatarEmblemType(emblem.c_str());
                if (type == 0) {
                    return 0;
                }
                script->value_380 |= static_cast<uint16_t>(type);
            }
        } else if (tag == "[stat change]") {
            std::string sign;
            while (ScanStr(&sign) == 1) {
                STStatChange change;
                change.enabled = sign == "+" ? 0 : 1;
                bool success = false;
                change.value = ScanInt(&success);
                std::string status;
                ScanStr(&status);
                change.stat_type = getChangeStatusType(status);
                if (change.stat_type == 77) {
                    change.stat_type = getActivePropertyTypeToEnumValue(status);
                    change.duration = 1;
                }
                script->stat_changes_34c.values.push_back(change);
            }
        } else if (tag == "[stat change duration]") {
            script->stat_changes_34c.value_0c =
                ScanInt(static_cast<bool*>(0));
            std::string target;
            ScanStr(&target);
            script->stat_changes_34c.value_10 = target == "myself" ? 0 : 1;
            script->stat_changes_34c.enabled = 1;
        } else if (tag == "[avatar select ability change]") {
            std::string job_name;
            if (ScanStr(&job_name) != 1) {
                return 0;
            }
            const int job = ::getCharacterJob(job_name.c_str());
            if (job == oldsrv::config::kCharacterJobCommon &&
                job_name != "[all]") {
                return 0;
            }
            std::string equipment_name;
            if (ScanStr(&equipment_name) != 1) {
                return 0;
            }
            const int equipment_type = GetEquipmentType(equipment_name.c_str());
            if (equipment_type == oldsrv::config::kEquipmentTypeInvalid &&
                equipment_name != "[all]") {
                return 0;
            }
            bool success = false;
            const int option = ScanInt(&success);
            if (!success || option >= 4) {
                return 0;
            }
            script->avatar_option_change_47c.option_0 =
                static_cast<int8_t>(job);
            script->avatar_option_change_47c.option_1 =
                static_cast<int8_t>(equipment_type);
            script->avatar_option_change_47c.option_2 =
                static_cast<int8_t>(option);
            script->value_4b0 = 1;
        } else if (tag == "[usable avatar index]") {
            bool success = false;
            int avatar_index = ScanInt(&success);
            while (success) {
                if (avatar_index <= 0 ||
                    !script->set_480.insert(avatar_index).second) {
                    return 0;
                }
                avatar_index = ScanInt(&success);
            }
            script->value_4b0 = 1;
        } else if (tag == "[avatar select ability except]") {
            bool success = false;
            int avatar_index = ScanInt(&success);
            while (success) {
                if (avatar_index < 0 ||
                    !script->set_498.insert(avatar_index).second) {
                    return 0;
                }
                avatar_index = ScanInt(&success);
            }
        } else if (tag == "[action type]") {
            std::string type;
            ScanStr(&type);
            script->value_1e8 = getStackableActionType(type);
            bool success = false;
            int value = ScanInt(&success);
            while (success) {
                script->values_1ec.push_back(value);
                value = ScanInt(&success);
            }
        } else if (tag == "[creature item list]") {
            // CLIENT_EA: ImportStackableScript_ParseCreatureItemListPairs
            // @ 0x0154E4C0. Values are source/destination creature item IDs.
            if (script->value_1e8 != kChangeCreatureActionType) {
                continue;
            }
            bool success = false;
            int source_item_id = ScanInt(&success);
            while (success) {
                const int destination_item_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->values_1ec.push_back(source_item_id);
                script->values_1ec.push_back(destination_item_id);
                source_item_id = ScanInt(&success);
            }
        } else if (tag == "[action usable place]") {
            std::string place;
            while (ScanStr(&place) == 1) {
                script->usable_places_1f8.push_back(
                    static_cast<StackableAction::UsablePlace>(
                        getStackableActionUsablePlace(place)));
            }
        } else if (tag == "[action usable server type]") {
            std::string server_type;
            while (ScanStr(&server_type) == 1) {
                script->usable_server_types_214.push_back(
                static_cast<StackableAction::UsableServerType>(
                        getStackableActionUsableServerType(server_type)));
            }
        } else if (tag == "[premium]") {
            script->value_18c = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[appendage group]") {
            script->values_1a8[2] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[linking quest index]") {
            script->values_1a8[0] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[sub type]") {
            script->value_1e4 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[move wav]") {
            ScanStr(&script->item.name);
        } else if (tag == "[disable icon]") {
            ScanStr(&script->string_1d8);
            script->value_1dc = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[applying range]") {
            script->value_1d4 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[additional item]") {
            script->rabbit_boxes_19c.clear();
            CRabbitBox item;
            item.setItemIndex(ScanInt(static_cast<bool*>(0)));
            item.SetItemCount(ScanInt(static_cast<bool*>(0)));
            item.SetDropRate(ScanInt(static_cast<bool*>(0)));
            script->rabbit_boxes_19c.push_back(item);
        } else if (tag == "[booster info]") {
            std::string section;
            while (true) {
                if (ScanType(section, true) != 1) {
                    return 0;
                }
                if (section == "[/booster info]") {
                    break;
                }

                stStackableBoosterElement_t element;
                element.category = script->booster_280.tag2Type(section.c_str());
                if (element.category == 9) {
                    return 0;
                }

                bool success = false;
                element.item_count = ScanInt(&success);
                if (!success) {
                    return 0;
                }

                int accumulated_rate = 0;
                std::string closing_tag;
                while (true) {
                    bool is_type = false;
                    bool scan_success = false;
                    const int item_index = ScanTypeOrInt(
                        is_type, closing_tag, scan_success, true);
                    if (!scan_success) {
                        return 0;
                    }
                    if (is_type) {
                        break;
                    }

                    stStackableBoosterItemInfo_t item;
                    item.item_index = item_index;
                    const int rate = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                    accumulated_rate += rate;
                    item.accumulated_rate = accumulated_rate;
                    item.item_count =
                        static_cast<unsigned short>(ScanInt(&success));
                    if (!success) {
                        return 0;
                    }
                    if (element.category <= 1) {
                        item.avatar_option_0d =
                            static_cast<unsigned char>(ScanInt(&success));
                        if (!success) {
                            return 0;
                        }
                        item.avatar_option_0e =
                            static_cast<unsigned char>(ScanInt(&success));
                        if (!success) {
                            return 0;
                        }
                    }
                    element.items.push_back(item);
                }

                if (element.category !=
                    script->booster_280.tag2EndType(closing_tag.c_str())) {
                    return 0;
                }
                element.total_rate = accumulated_rate;
                script->booster_280.elements.push_back(element);
            }
        } else if (tag == "[booster select category]") {
            int first_category = 0;
            int second_category = 0;
            ScanInt(&first_category);
            ScanInt(&second_category);

            std::vector<stStackableBoosterItemInfo_t> items;
            int equipment_upgrade = 0;
            std::vector<int> default_items;
            std::vector<int> recommended_items;
            std::vector<std::pair<int, int> > reinforcement_level_weights;
            std::vector<std::pair<int, int> > forced_avatar_options;
            std::string section;
            while (ScanType(section, true) == 1) {
                if (section == "[booster equipment upgrade]") {
                    if (ScanInt(&equipment_upgrade) != 1) {
                        return 0;
                    }
                } else if (section == "[random probability]") {
                    if (equipment_upgrade != 0xff) {
                        return 0;
                    }
                    reinforcement_level_weights.clear();
                    bool success = false;
                    int reinforcement_level = ScanInt(&success);
                    while (success) {
                        const int selection_weight = ScanInt(&success);
                        if (!success) {
                            break;
                        }
                        reinforcement_level_weights.push_back(
                            std::make_pair(reinforcement_level,
                                           selection_weight));
                        reinforcement_level = ScanInt(&success);
                    }
                } else if (section == "[force insert avatar option]") {
                    forced_avatar_options.clear();
                    bool success = false;
                    int first_value = ScanInt(&success);
                    while (success) {
                        const int second_value = ScanInt(&success);
                        if (!success) {
                            break;
                        }
                        forced_avatar_options.push_back(
                            std::make_pair(first_value, second_value));
                        first_value = ScanInt(&success);
                    }
                } else if (section == "[/random probability]" ||
                           section == "[/force insert avatar option]") {
                    continue;
                } else if (section == "[default select]") {
                    int count = 0;
                    if (ScanInt(&count) != 1) {
                        return 0;
                    }
                    for (int index = 0; index < count; ++index) {
                        default_items.push_back(
                            ScanInt(static_cast<bool*>(0)));
                    }
                } else if (section == "[recommend]") {
                    int count = 0;
                    if (ScanInt(&count) != 1) {
                        return 0;
                    }
                    for (int index = 0; index < count; ++index) {
                        recommended_items.push_back(
                            ScanInt(static_cast<bool*>(0)));
                    }
                } else {
                    if (section == "[/booster select category]") {
                        break;
                    }

                    stStackableBoosterElement_t element;
                    element.category =
                        script->booster_280.tag2Type(section.c_str());
                    int total_rate = 0;
                    while (true) {
                        bool success = false;
                        stStackableBoosterItemInfo_t item;
                        item.item_index = ScanInt(&success);
                        if (!success) {
                            break;
                        }
                        item.item_count = static_cast<unsigned short>(
                            ScanInt(&success));
                        if (element.category <= 1) {
                            item.avatar_option_0d =
                                static_cast<unsigned char>(ScanInt(&success));
                            item.avatar_option_0e =
                                static_cast<unsigned char>(ScanInt(&success));
                        }
                        item.equipment_upgrade =
                            static_cast<unsigned char>(
                                element.category == 4 ? equipment_upgrade : 0);
                        item.default_selected = static_cast<unsigned char>(
                            std::find(default_items.begin(),
                                      default_items.end(),
                                      item.item_index) != default_items.end());
                        item.recommended = static_cast<unsigned char>(
                            std::find(recommended_items.begin(),
                                      recommended_items.end(),
                                      item.item_index) !=
                            recommended_items.end());
                        item.reinforcement_level_weights =
                            reinforcement_level_weights;
                        item.forced_avatar_options = forced_avatar_options;
                        items.push_back(item);
                    }
                    element.total_rate = total_rate;
                }
            }

            script->booster_280.category_items[
                std::make_pair(static_cast<char>(first_category),
                               static_cast<char>(second_category))] = items;
        } else if (tag == "[random]") {
            script->values_244.clear();
            std::string section;
            while (ScanType(section, true) == 1 && section != "[/random]") {
                if (section == "[random list]") {
                    std::vector<int> values;
                    bool success = false;
                    int value = ScanInt(&success);
                    while (success) {
                        values.push_back(value);
                        value = ScanInt(&success);
                    }
                    script->values_244.push_back(values);
                } else if (section == "[sealing removal item]") {
                    script->values_250.clear();
                    bool success = false;
                    int value = ScanInt(&success);
                    while (success) {
                        script->values_250.push_back(value);
                        value = ScanInt(&success);
                    }
                } else if (section == "[default list]") {
                    bool success = false;
                    int value = ScanInt(&success);
                    while (success) {
                        script->values_25c.push_back(value);
                        value = ScanInt(&success);
                    }
                }
            }
        } else if (tag == "[stackable type]") {
            std::string type;
            if (ScanStr(&type) != 1) {
                return 0;
            }
            script->item.strings_ac[4] = type;
            if (type == "[waste]") script->value_1e0 = 0;
            else if (type == "[material]") script->value_1e0 = 1;
            else if (type == "[material expert job]") script->value_1e0 = 2;
            else if (type == "[recipe]") script->value_1e0 = 3;
            else if (type == "[throw]") script->value_1e0 = 4;
            else if (type == "[set]") script->value_1e0 = 5;
            else if (type == "[legacy]") script->value_1e0 = 6;
            else if (type == "[stackable legacy]") script->value_1e0 = 7;
            else if (type == "[quest]") script->value_1e0 = 8;
            else if (type == "[creature]") script->value_1e0 = 9;
            else if (type == "[feed]") script->value_1e0 = 10;
            else if (type == "[etc]") script->value_1e0 = 11;
            else if (type == "[only effect]") script->value_1e0 = 12;
            else if (type == "[upgradable legacy]") script->value_1e0 = 13;
            else if (type == "[cera package]") script->value_1e0 = 14;
            else if (type == "[cera booster]") script->value_1e0 = 15;
            else if (type == "[booster]") script->value_1e0 = 16;
            else if (type == "[booster random]") script->value_1e0 = 17;
            else if (type == "[unlimited waste]") script->value_1e0 = 18;
            else if (type == "[quest receive]") script->value_1e0 = 19;
            else if (type == "[avatar emblem]") script->value_1e0 = 20;
            else if (type == "[unlimited etc]") script->value_1e0 = 21;
            else if (type == "[disguise]") script->value_1e0 = 22;
            else if (type == "[global effect]") script->value_1e0 = 23;
            else if (type == "[expert town potion]") script->value_1e0 = 24;
            else if (type == "[contract]") script->value_1e0 = 25;
            else if (type == "[multi upgradable legacy]" ||
                     type == "[random upgradable legacy]") script->value_1e0 = 26;
            else if (type == "[multi upgradable legacy bonus cera]") script->value_1e0 = 27;
            else if (type == "[enchant waste]") script->value_1e0 = 28;
            else if (type == "[upgrade limit cube]") script->value_1e0 = 29;
            else if (type == "[tarot card lottery]") script->value_1e0 = 30;
            else if (type == "[tarot card]") script->value_1e0 = 31;
            else if (type == "[town and dungeon]") script->value_1e0 = 32;
            else if (type == "[unlimited town and dungeon]") script->value_1e0 = 33;
            else if (type == "[booster selection]") script->value_1e0 = 34;
            else if (type == "[creature expitem]") script->value_1e0 = 35;
            else if (type == "[dye]") script->value_1e0 = 36;
            else if (type == "[teleport potion]") script->value_1e0 = 37;
            else if (type == "[usable cera package]") script->value_1e0 = 38;
            else script->value_1e0 = getStackableActionType(type);
            script->item.values_a4[0] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[creature minimum level]") {
            script->values_23c[0] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[set item master]") {
            script->values_23c[1] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[use wav]") {
            if (ScanStr(&script->strings_230[2]) != 1) {
                return 0;
            }
        } else if (tag == "[item category]") {
            if (ReadItemCategoryList(&script->item.item_categories) != 1) {
                return 0;
            }
        } else if (tag == "[routing priority]") {
            script->item.value_10f = 1;
            std::string job_name;
            while (ScanStr(&job_name) == 1) {
                if (job_name == "[/routing priority]") {
                    break;
                }
                const int job = getCharacterJobToEnumValue(job_name);
                if (job == -1) {
                    break;
                }
                bool success = false;
                const int priority = ScanInt(&success);
                if (!success) {
                    break;
                }
                if (priority <= 0 || priority > 4) {
                    return 0;
                }
                script->item.special_drop_items.push_back(
                    std::make_pair(job, priority));
            }
        } else if (tag == "[special monster drop]" ||
                   tag == "[difficulty for extra drop]" ||
                   tag == "[dungeon type for extra drop]") {
            if (StackableScriptSpecialMonsterDrop(script, tag, filename) != 1) {
                return 0;
            }
        } else if (tag == "[usable world]") {
            script->item.usable_worlds.clear();
            for (int world = scanWorldType(); world != 2;
                 world = scanWorldType()) {
                script->item.usable_worlds.insert(world);
            }
        } else if (tag == "[expertjob only]") {
            std::vector<STItemScript::SEXPERTJOB::SUSABLE>& usable =
                script->item.expert_job.usable;
            usable.clear();
            std::string expert_job_name;
            while (ScanStr(&expert_job_name) == 1) {
                int expert_job_type = 0;
                if (expert_job_name == "enchanter") {
                    expert_job_type = 1;
                } else if (expert_job_name == "alchemist") {
                    expert_job_type = 2;
                } else if (expert_job_name == "disjointer") {
                    expert_job_type = 3;
                } else if (expert_job_name == "doll_controller") {
                    expert_job_type = 4;
                } else if (expert_job_name == "all") {
                    expert_job_type = 5;
                } else {
                    return 0;
                }
                bool success = false;
                const int expert_job_value = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                STItemScript::SEXPERTJOB::SUSABLE entry;
                entry.type = expert_job_type;
                entry.value = expert_job_value;
                usable.push_back(entry);
            }
            if (usable.empty()) {
                return 0;
            }
        } else if (tag == "[prof material variation]") {
            bool success = true;
            script->item.expert_job.material_variation =
                ScanFloat(&success);
            if (!success ||
                script->item.expert_job.material_variation <= -100.0f) {
                return 0;
            }
        } else if (tag == "[prof compound rate]") {
            bool success = true;
            script->item.expert_job.compound_rate = ScanFloat(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[dye type]") {
            bool success = false;
            const int dye_type = ScanInt(&success);
            if (success) {
                script->item.dye_info.type = dye_type;
            }
        } else if (tag == "[dye info]") {
            bool success = false;
            int first = ScanInt(&success);
            while (success) {
                const int second = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->item.dye_info.colors.push_back(
                    std::make_pair(first, second));
                first = ScanInt(&success);
            }
        } else if (tag == "[emancipate ticket]") {
            script->item.value_10e = 1;
            script->value_394 =
                static_cast<unsigned short>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[equipment reinforcement ticket]") {
            bool success = false;
            script->value_396 = 1;
            script->value_398 =
                static_cast<unsigned short>(ScanInt(&success));
            if (!success) {
                return 0;
            }
            script->value_39a = static_cast<unsigned char>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[equipment amplify reinforcement ticket]") {
            bool success = false;
            script->value_39b = 1;
            script->value_39c = ScanInt(&success);
            if (!success) {
                return 0;
            }
            script->values_3a0[0] = static_cast<unsigned char>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[check usable itemlevel]") {
            bool success = false;
            script->pair_43c.first = ScanInt(&success);
            if (!success) {
                return 0;
            }
            script->pair_43c.second = ScanInt(&success);
            if (!success) {
                return 0;
            }
            script->value_438 = 1;
        } else if (tag == "[amplification random value]") {
            // IDA preserves earlier entries and std::map::insert keeps the
            // first value for a duplicate source key.
            script->values_3a0[1] = 1;
            bool success = false;
            int source = ScanInt(&success);
            while (success) {
                const int target = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->map_3a4.insert(std::make_pair(source, target));
                source = ScanInt(&success);
            }
        } else if (tag == "[upgrade limit cube info]") {
            std::string section;
            while (ScanType(section, true) == 1 &&
                   section != "[/upgrade limit cube info]") {
                if (section == "[A condition item]") {
                    bool success = false;
                    int index = ScanInt(&success);
                    while (success) {
                        const int count = ScanInt(&success);
                        if (!success) {
                            return 0;
                        }
                        script->limit_cube_3d4.condition_items.push_back(
                            std::make_pair(index, count));
                        index = ScanInt(&success);
                    }
                } else if (section == "[B condition item]") {
                    bool success = false;
                    script->limit_cube_3d4.value_0c = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                    script->limit_cube_3d4.value_10 = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                } else if (section == "[result item]") {
                    bool success = false;
                    int index = ScanInt(&success);
                    while (success) {
                        const int count = ScanInt(&success);
                        if (!success) {
                            return 0;
                        }
                        const int value = ScanInt(&success);
                        if (!success) {
                            return 0;
                        }
                        stLimitCubeResult result;
                        result.item_index = index;
                        result.item_count = count;
                        result.probability = value;
                        script->limit_cube_3d4.results.push_back(result);
                        index = ScanInt(&success);
                    }
                }
            }
        }
    }

    if (script->item.value_24 == -1) {
        script->item.value_24 = script->item.values_08[3];
    }
    script->item.index = item_id == -1
        ? g_stStackableScriptFileList.FindIndexByFullScan(filename)
        : item_id;
    return 1;
}
