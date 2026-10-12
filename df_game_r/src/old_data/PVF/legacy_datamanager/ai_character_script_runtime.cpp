#include "old_data/PVF/aicharacter/datamanager/ai_character_script.h"

#include "common/config.h"
#include "old_data/PVF/aicharacter/datamanager/ai_character_runtime.h"
#include "old_data/PVF/etc/independent_drop_parameter_script.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/datamanager/script_file_list.h"
#include "old_data/PVF/data/datamanager/sync_script.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"

// IDA data: ACTIVEOBJECT_STATE_STRING @0x093F4D80, 0x40-byte pointer table.
// Extracted directly from the old df_game_r ELF: the final pointer is null.
const char* ACTIVEOBJECT_STATE_STRING[] = {
    "[stand]",
    "[sit]",
    "[sit forever]",
    "[damage]",
    "[down]",
    "[die]",
    "[jump]",
    "[jump attack]",
    "[attack]",
    "[hold]",
    "[throw]",
    "[dash]",
    "[dash attack]",
    "[get item]",
    "[buff]",
    0
};

#include <cstdio>
#include <cstring>
#include <new>
#include <string>
#include <utility>
#include <vector>

char g_aiCharacterScriptBaseDirectory[0x100];
STScriptFileList g_aiCharacterScriptFileList;

// IDA_EA: 0x8A38A8B
// IDA_NAME: _Z11stringToIntPKcPS0_i
// DEMANGLED: stringToInt(char const*,char const**,int)
int stringToInt(const char* text, const char** table, int count) {
    if (text == 0 || table == 0) {
        return -1;
    }
    for (int index = 0;
         (count < 0 || index < count) && table[index] != 0;
         ++index) {
        if (std::strcmp(text, table[index]) == 0) {
            return index;
        }
    }
    return -1;
}

// IDA_EA: 0x88CC190
// IDA_NAME: _ZN11MinimumInfo5clearEv
// DEMANGLED: MinimumInfo::clear(void)
MinimumInfo* MinimumInfo::clear() {
    level = 0;
    return this;
}

// IDA_EA: 0x88CC3E2
// IDA_NAME: _ZN11MinimumInfoC2Ev
// DEMANGLED: MinimumInfo::MinimumInfo(void)
MinimumInfo::MinimumInfo() : name(), level(0) {}

// IDA_EA: 0x88CC3F6
// IDA_NAME: _ZN11MinimumInfoD2Ev
// DEMANGLED: MinimumInfo::~MinimumInfo()
MinimumInfo::~MinimumInfo() {}

// IDA_EA: 0x88CC328
// IDA_NAME: _ZN9APCSpeechC2Ev
// DEMANGLED: APCSpeech::APCSpeech(void)
APCSpeech::APCSpeech() : texts(), values() {
    clear();
}

// IDA_EA: 0x88CC450
// IDA_NAME: _ZN9APCSpeechC2ERKS_
// DEMANGLED: APCSpeech::APCSpeech(APCSpeech const&)
APCSpeech::APCSpeech(const APCSpeech& other)
    : texts(other.texts), values(other.values) {}

// IDA_EA: 0x88CC390
// IDA_NAME: _ZN9APCSpeech5clearEv
// DEMANGLED: APCSpeech::clear(void)
APCSpeech* APCSpeech::clear() {
    texts.clear();
    values.clear();
    return this;
}

// IDA_EA: 0x88CC3B2
// IDA_NAME: _ZN9APCSpeech3addERSsi
// DEMANGLED: APCSpeech::add(std::string &,int)
int APCSpeech::add(std::string& text, int value) {
    texts.push_back(text);
    values.push_back(value);
    return 1;
}

// IDA_EA: 0x88CC40A
// IDA_NAME: _ZN9APCSpeechD2Ev
// DEMANGLED: APCSpeech::~APCSpeech()
APCSpeech::~APCSpeech() {}

// IDA_EA: 0x88D06C6
// IDA_NAME: _ZN9APCSpeechaSERKS_
// DEMANGLED: APCSpeech::operator=(APCSpeech const&)
APCSpeech& APCSpeech::operator=(const APCSpeech& other) {
    texts = other.texts;
    values = other.values;
    return *this;
}

// IDA_EA: 0x8367EEE
// IDA_NAME: _ZN19ActiveActionFileSetC2Ev
// DEMANGLED: ActiveActionFileSet::ActiveActionFileSet(void)
ActiveActionFileSet::ActiveActionFileSet()
    : waiting(),
      move(),
      dash(),
      sit(),
      damage_1(),
      damage_2(),
      down(),
      overturn(),
      jump(),
      jump_attack(),
      attack_actions(),
      etc_actions() {}

// IDA_EA: 0x83680F4
// IDA_NAME: _ZN19ActiveActionFileSetD2Ev
// DEMANGLED: ActiveActionFileSet::~ActiveActionFileSet()
ActiveActionFileSet::~ActiveActionFileSet() {}

// IDA_EA: 0x88CC30E
// IDA_NAME: _ZN28STAICharacterSkillCorrectionC2Ev
// DEMANGLED: STAICharacterSkillCorrection::STAICharacterSkillCorrection(void)
STAICharacterSkillCorrection::STAICharacterSkillCorrection()
    : skill_index(0), correction(1.0f) {}

// IDA_EA: 0x88CF1B4
// IDA_NAME: _ZN28STAICharacterSkillCorrectionD2Ev
// DEMANGLED: STAICharacterSkillCorrection::~STAICharacterSkillCorrection()
STAICharacterSkillCorrection::~STAICharacterSkillCorrection() {}

// IDA_EA: 0x88CEAF2
// IDA_NAME: _ZN26STAICharacterEquipmentDataD2Ev
// DEMANGLED: STAICharacterEquipmentData::~STAICharacterEquipmentData()
STAICharacterEquipmentData::~STAICharacterEquipmentData() {}

// IDA_EA: 0x88CED32
// IDA_NAME: _ZN22STAICharacterSkillDataD2Ev
// DEMANGLED: STAICharacterSkillData::~STAICharacterSkillData()
STAICharacterSkillData::~STAICharacterSkillData() {}

// IDA_EA: 0x88CEF72
// IDA_NAME: _ZN21STAICharacterItemDataD2Ev
// DEMANGLED: STAICharacterItemData::~STAICharacterItemData()
STAICharacterItemData::~STAICharacterItemData() {}

// IDA_EA: 0x88CF8CA
// IDA_NAME: _ZN29STAICharacterCustomSkillLevelD2Ev
// DEMANGLED: STAICharacterCustomSkillLevel::~STAICharacterCustomSkillLevel()
STAICharacterCustomSkillLevel::~STAICharacterCustomSkillLevel() {}

// IDA_EA: 0x83D31D2
// IDA_NAME: _ZN27STAICharacterMapArrangeDataC2ERKS_
// DEMANGLED: STAICharacterMapArrangeData::STAICharacterMapArrangeData(STAICharacterMapArrangeData const&)
STAICharacterMapArrangeData::STAICharacterMapArrangeData(
    const STAICharacterMapArrangeData& other)
    : raw_00(other.raw_00),
      raw_04(other.raw_04),
      raw_08(other.raw_08),
      raw_0c(other.raw_0c),
      raw_10(other.raw_10),
      raw_14(other.raw_14),
      raw_18(other.raw_18),
      raw_19(other.raw_19),
      pad_1a(),
      int_values(other.int_values),
      raw_28(other.raw_28),
      byte_values(other.byte_values) {
    pad_1a[0] = other.pad_1a[0];
    pad_1a[1] = other.pad_1a[1];
}

// IDA_EA: 0x83DC9A0
// IDA_NAME: _ZN27STAICharacterMapArrangeDataD2Ev
// DEMANGLED: STAICharacterMapArrangeData::~STAICharacterMapArrangeData()
STAICharacterMapArrangeData::~STAICharacterMapArrangeData() {}

// IDA_EA: 0x83E0980
// IDA_NAME: _ZN27STAICharacterMapArrangeDataaSERKS_
// DEMANGLED: STAICharacterMapArrangeData::operator=(STAICharacterMapArrangeData const&)
STAICharacterMapArrangeData& STAICharacterMapArrangeData::operator=(
    const STAICharacterMapArrangeData& other) {
    raw_00 = other.raw_00;
    raw_04 = other.raw_04;
    raw_08 = other.raw_08;
    raw_0c = other.raw_0c;
    raw_10 = other.raw_10;
    raw_14 = other.raw_14;
    raw_18 = other.raw_18;
    raw_19 = other.raw_19;
    int_values = other.int_values;
    raw_28 = other.raw_28;
    byte_values = other.byte_values;
    return *this;
}

// IDA_EA: 0x89E85FE
// IDA_NAME: _ZN27STAICharacterMapArrangeData5ClearEv
// DEMANGLED: STAICharacterMapArrangeData::Clear(void)
void STAICharacterMapArrangeData::Clear() {
    raw_00 = 0;
    raw_04 = 0;
    raw_08 = 0;
    raw_0c = 0;
    raw_10 = 0;
    raw_14 = 0;
    raw_18 = 0;
    raw_19 = 0;
    int_values.clear();
    raw_28 = -1;
    byte_values.clear();
}

// IDA_EA: 0x89E8676
// IDA_NAME: _ZN27STAICharacterMapArrangeDataC2Ev
// DEMANGLED: STAICharacterMapArrangeData::STAICharacterMapArrangeData(void)
STAICharacterMapArrangeData::STAICharacterMapArrangeData()
    : int_values(), byte_values() {
    Clear();
}

// IDA_EA: 0x88C8A78
// IDA_NAME: _ZN25STAICharacterCommonScriptC2Ev
// DEMANGLED: STAICharacterCommonScript::STAICharacterCommonScript(void)
STAICharacterCommonScript::STAICharacterCommonScript()
    : minimum_info(),
      character_status(),
      equipment(),
      quick_skills(),
      skills(),
      skill_corrections(),
      quick_items(),
      items(),
      ai_info(),
      ai_runtime_values(),
      key_files(),
      key_file_names(),
      key_cooltimes(),
      speech(),
      attack_speech(),
      key_speech(),
      rarity(0),
      appearance_point(0),
      death_tower_items(),
      party_members(),
      far_attack_reaction_rate(0),
      pvp_win_point_min(0),
      pvp_win_point_max(0),
      character_status_rate(),
      custom_skill_levels(),
      attack_damage_rate(1.0f),
      add_equipment_status_from_level(0),
      armor_subtype(2),
      default_equipment(1),
      doll_special_dungeons(),
      character_status_rate_special(),
      attack_damage_rate_special(1.0f),
      common_champion_elements(),
      independent_drops(),
      limited_passive_objects(),
      overhead_gauge_type(0),
      overhead_gauge_x(0),
      overhead_gauge_y(0),
      drop_avatar_info(),
      getup_invincible(1),
      skill_data_up(),
      summon_monster_status_rates(),
      leave_body(1),
      padding_fa9(),
      action_files(),
      last_action(),
      proc_action(),
      add_gravity_in_aerial(0),
      padding_ff5(),
      aerial_gravity_1(0.0f),
      aerial_gravity_2(0.0f),
      no_cooltime(0),
      check_account_item(0),
      padding_1005(),
      npc_index(-1),
      hell_monster(0),
      padding_100d(),
      animation_job(oldsrv::config::kCharacterJobInvalid),
      revive_option(),
      recovery_option() {
    clear();
}

// IDA_EA: 0x88C903C
// IDA_NAME: _ZN25STAICharacterCommonScriptD2Ev
// DEMANGLED: STAICharacterCommonScript::~STAICharacterCommonScript()
STAICharacterCommonScript::~STAICharacterCommonScript() {
    destroy();
}

// IDA_EA: 0x88C9592
// IDA_NAME: _ZN25STAICharacterCommonScript5clearEv
// DEMANGLED: STAICharacterCommonScript::clear(void)
STAICharacterCommonScript* STAICharacterCommonScript::clear() {
    minimum_info.clear();
    character_status.Clear();
    equipment.clear();
    quick_skills.clear();
    skills.clear();
    skill_corrections.clear();
    quick_items.clear();
    items.clear();
    ai_info.clear();
    for (int index = 0; index <= 7; ++index) {
        ai_runtime_values[index] = 0;
    }
    key_files.clear();
    key_file_names.clear();
    key_cooltimes.clear();
    for (int index = 0; index <= 110; ++index) {
        speech[index].clear();
    }
    attack_speech.clear();
    key_speech.clear();
    rarity = 0;
    appearance_point = 0;
    death_tower_items.clear();
    party_members.clear();
    far_attack_reaction_rate = 0;
    pvp_win_point_min = 0;
    pvp_win_point_max = 0;
    character_status_rate.Clear();
    custom_skill_levels.clear();
    attack_damage_rate = 1.0f;
    add_equipment_status_from_level = 0;
    armor_subtype = 2;
    default_equipment = 1;
    doll_special_dungeons.clear();
    character_status_rate_special.Clear();
    attack_damage_rate_special = 1.0f;
    common_champion_elements.clear();
    independent_drops.clear();
    limited_passive_objects.clear();
    overhead_gauge_type = 0;
    overhead_gauge_x = 0;
    overhead_gauge_y = 0;
    drop_avatar_info.clear();
    getup_invincible = 1;
    skill_data_up.clear();
    summon_monster_status_rates.clear();
    leave_body = 1;
    add_gravity_in_aerial = 0;
    aerial_gravity_1 = 0.0f;
    aerial_gravity_2 = 0.0f;
    no_cooltime = 0;
    check_account_item = 0;
    npc_index = -1;
    hell_monster = 0;
    animation_job = oldsrv::config::kCharacterJobInvalid;
    revive_option.map_conditions.clear();
    revive_option.active_object_state.clear();
    revive_option.revive_value = 0;
    recovery_option.map_conditions.clear();
    recovery_option.active_object_state.clear();
    recovery_option.hp_recovery_value = 0;
    return this;
}

// IDA_EA: 0x88C98BC
// IDA_NAME: _ZN25STAICharacterCommonScript7destroyEv
// DEMANGLED: STAICharacterCommonScript::destroy(void)
bool STAICharacterCommonScript::destroy() {
    return ai_info.destroy();
}

// IDA_EA: 0x8A38AF4
// IDA_NAME: _Z14makeAIFullPathRSsS_
// DEMANGLED: makeAIFullPath(std::string &,std::string &)
void makeAIFullPath(std::string& base_path, std::string& path) {
    makeFullPath(base_path + "ai/", path);
}

// IDA_EA: 0x8A38B5D
// IDA_NAME: _Z15makeKeyFullPathRSsS_
// DEMANGLED: makeKeyFullPath(std::string &,std::string &)
void makeKeyFullPath(std::string& base_path, std::string& path) {
    makeFullPath(base_path + "key/", path);
}

// IDA_EA: 0x8A390DB
// IDA_NAME: _Z18importAIInfoScriptP14STAIInfoScriptPSsb
// DEMANGLED: importAIInfoScript(STAIInfoScript *,std::string *,bool)
int importAIInfoScript(STAIInfoScript* script,
                       std::string* tag,
                       bool module_type) {
    if (*tag == "[think term]") {
        script->value_04 = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (*tag == "[destination change term]") {
        script->value_08 = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (*tag == "[ai pattern]") {
        int difficulty = 0;
        bool is_type = false;
        std::string value;
        while (ScanTypeOrStr(is_type, value, true) == 1) {
            if (is_type) {
                if (value == "[/ai pattern]") {
                    break;
                }
                if (value == "[easy]") {
                    difficulty = 0;
                } else if (value == "[medium]") {
                    difficulty = 1;
                } else if (value == "[hard]") {
                    difficulty = 2;
                } else if (value == "[ultimate]") {
                    difficulty = 3;
                } else if (value == "[hero]") {
                    difficulty = 4;
                }
            } else {
                if (module_type) {
                    value = script->value_0C + value;
                } else {
                    makeAIFullPath(script->value_0C, value);
                }
                script->string_lists[difficulty].push_back(
                    new std::string(value));
            }
        }
        return 1;
    }
    if (*tag == "[no change target]") {
        script->value_200 = ScanInt(static_cast<bool*>(0)) != 0;
        return 1;
    }
    if (*tag == "[/targeting bonus]") {
        // The original target-bonus reader consumes its old flat payload
        // directly.  Current PVFs additionally emit this explicit, empty
        // terminator; it has no member or value to publish in the old ABI.
        return 1;
    }
    if (*tag == "[targeting bonus]") {
        for (int index = 0; index <= 21; ++index) {
            script->values_4C[index] = ScanInt(static_cast<bool*>(0));
            if (index == 20 || index == 21) {
                std::string value;
                while (ScanStr(&value) == 1) {
                    const int converted =
                        index == 20
                            ? stringToInt(value.c_str(),
                                          ACTIVEOBJECT_STATE_STRING,
                                          -1)
                            : stringToInt(value.c_str(),
                                          ACTIVESTATUS_ATTACK_TAG,
                                          19);
                    script->int_lists[index].push_back(converted);
                }
            }
        }
        return 1;
    }
    if (*tag == "[keep distance with target]") {
        script->value_1AC = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (*tag == "[keep range distance with target]" ||
        *tag == "[keep range distance with target 1]" ||
        *tag == "[keep range distance with target 2]" ||
        *tag == "[keep range distance with target 3]" ||
        *tag == "[keep range distance with target 4]" ||
        *tag == "[keep range distance with target 5]") {
        script->values_1B0.push_back(ScanInt(static_cast<bool*>(0)));
        script->values_1BC.push_back(ScanInt(static_cast<bool*>(0)));
        script->values_1C8.push_back(ScanInt(static_cast<bool*>(0)));
        script->values_1D4.push_back(ScanInt(static_cast<bool*>(0)));
        return 1;
    }
    if (*tag == "[warlike]") {
        script->value_1EC = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (*tag == "[intelligence]") {
        script->value_1EC = ScanInt(static_cast<bool*>(0));
        script->value_1F0 = ScanInt(static_cast<bool*>(0));
        script->value_1F4 = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (*tag == "[vision]") {
        script->value_1F8 = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (*tag == "[back vision]") {
        script->value_1FC = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    return 0;
}

// IDA_EA: 0x88C9CCC
// IDA_NAME: _Z29importAICharacterCommonScriptP25STAICharacterCommonScriptPKcb
// DEMANGLED: importAICharacterCommonScript(STAICharacterCommonScript *,char const*,bool)
int importAICharacterCommonScript(STAICharacterCommonScript* script,
                                  const char* filename,
                                  bool module_type) {
    if (loadRDARScriptFile(g_aiCharacterScriptBaseDirectory,
                                   filename) != 1) {
        return 0;
    }

    script->clear();
    getDirectoryPath(script->ai_info.value_0C,
                             "%s%s",
                             g_aiCharacterScriptBaseDirectory,
                             filename);

    std::string tag;
    std::string value;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[minimum info]") {
            if (ScanStr(&script->minimum_info.name) != 1) {
                return 0;
            }
            ScanInt(static_cast<bool*>(0));
            ScanInt(static_cast<bool*>(0));
            ScanInt(static_cast<bool*>(0));
            ScanInt(static_cast<bool*>(0));
            script->minimum_info.level = ScanInt(static_cast<bool*>(0));
            ScanInt(static_cast<bool*>(0));
            ScanInt(static_cast<bool*>(0));
            ScanInt(static_cast<bool*>(0));
            ScanInt(static_cast<bool*>(0));
            ScanInt(static_cast<bool*>(0));
            ScanInt(static_cast<bool*>(0));
            ScanStr(&value);
            ScanInt(static_cast<bool*>(0));
            ScanInt(static_cast<bool*>(0));
            ScanInt(static_cast<bool*>(0));
            if (module_type) {
                break;
            }
        } else if (tag == "[NPC]") {
            script->npc_index = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[additional character status]") {
            script->character_status.raw_00 = ScanInt(static_cast<bool*>(0));
            script->character_status.raw_04 = ScanInt(static_cast<bool*>(0));
            script->character_status.raw_08 =
                static_cast<uint16_t>(ScanInt(static_cast<bool*>(0)));
            script->character_status.raw_0a =
                static_cast<uint16_t>(ScanInt(static_cast<bool*>(0)));
            script->character_status.raw_0c =
                static_cast<uint16_t>(ScanInt(static_cast<bool*>(0)));
            script->character_status.raw_0e =
                static_cast<uint16_t>(ScanInt(static_cast<bool*>(0)));
            script->character_status.raw_3a = ScanInt(static_cast<bool*>(0));
            script->character_status.raw_3e =
                static_cast<int16_t>(ScanInt(static_cast<bool*>(0)));
            script->character_status.raw_40 =
                static_cast<int16_t>(ScanInt(static_cast<bool*>(0)));
            script->character_status.raw_42 = ScanInt(static_cast<bool*>(0));
            script->character_status.raw_46 =
                static_cast<int16_t>(ScanInt(static_cast<bool*>(0)));
            script->character_status.raw_48 =
                static_cast<int16_t>(ScanInt(static_cast<bool*>(0)));
            script->character_status.raw_4a =
                static_cast<int16_t>(ScanInt(static_cast<bool*>(0)));
            script->character_status.raw_4c =
                static_cast<int16_t>(ScanInt(static_cast<bool*>(0)));
            script->character_status.raw_4e = ScanInt(static_cast<bool*>(0));
            script->character_status.raw_52 =
                static_cast<uint8_t>(ScanInt(static_cast<bool*>(0)));
            for (int index = 0; index <= 3; ++index) {
                script->character_status.four_values[index] =
                    static_cast<int16_t>(ScanInt(static_cast<bool*>(0)));
            }
            for (int index = 0;; ++index) {
                const int parsed = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->character_status.seventeen_values[index] =
                    static_cast<int16_t>(parsed);
            }
        } else if (tag == "[equipment]") {
            while (true) {
                STAICharacterEquipmentData data;
                data.item_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                data.option_1 = ScanInt(static_cast<bool*>(0));
                data.option_2 = ScanInt(static_cast<bool*>(0));
                script->equipment.push_back(data);
            }
        } else if (tag == "[skill]" || tag == "[quick skill]") {
            while (true) {
                STAICharacterSkillData data;
                data.skill_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                data.skill_level = ScanInt(static_cast<bool*>(0));
                if (tag == "[quick skill]") {
                    script->quick_skills.push_back(data);
                } else {
                    script->skills.push_back(data);
                }
            }
        } else if (tag == "[item]" || tag == "[quick item]") {
            while (true) {
                STAICharacterItemData data;
                data.item_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                data.item_count = ScanInt(static_cast<bool*>(0));
                if (tag == "[quick item]") {
                    script->quick_items.push_back(data);
                } else {
                    script->items.push_back(data);
                }
            }
        } else if (tag == "[skill correction]") {
            while (true) {
                STAICharacterSkillCorrection correction;
                correction.skill_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                correction.correction = ScanFloat(0, 0);
                script->skill_corrections.push_back(correction);
            }
        } else if (tag == "[key stream]") {
            while (true) {
                std::string key_name;
                if (ScanStr(&key_name) != 1) {
                    break;
                }
                std::string key_file;
                bool has_key_file = true;
                if (isKeyFile(key_name.c_str()) != 0) {
                    key_file = key_name;
                    key_name.clear();
                } else {
                    has_key_file = ScanStr(&key_file) == 1;
                }
                if (key_name.empty()) {
                    key_name = getFileName(key_file);
                }
                script->key_file_names.push_back(key_name);
                if (!has_key_file) {
                    break;
                }
                makeKeyFullPath(script->ai_info.value_0C, key_file);
                script->key_files.push_back(key_file);
            }
        } else if (tag == "[key cooltime]") {
            while (ScanStr(&value) == 1) {
                const int cooltime = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->key_cooltimes[value] = cooltime;
            }
        } else if (tag == "[rarity]") {
            if (ScanStr(&value) != 1) {
                return 0;
            }
            script->rarity = getItemRarity(value.c_str());
            if (script->rarity == 6) {
                return 0;
            }
        } else if (tag == "[appearance point]") {
            script->appearance_point = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[death tower item]") {
            script->death_tower_items.clear();
            while (true) {
                const int item_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->death_tower_items.push_back(
                    std::make_pair(item_index, ScanInt(static_cast<bool*>(0))));
            }
        } else if (tag == "[leave body]") {
            script->leave_body = ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[waiting action]") {
            ScanStr(&script->action_files.waiting);
        } else if (tag == "[move action]") {
            ScanStr(&script->action_files.move);
        } else if (tag == "[dash action]") {
            ScanStr(&script->action_files.dash);
        } else if (tag == "[sit action]") {
            ScanStr(&script->action_files.sit);
        } else if (tag == "[damage action 1]") {
            ScanStr(&script->action_files.damage_1);
        } else if (tag == "[damage action 2]") {
            ScanStr(&script->action_files.damage_2);
        } else if (tag == "[down action]") {
            ScanStr(&script->action_files.down);
        } else if (tag == "[overturn action]") {
            ScanStr(&script->action_files.overturn);
        } else if (tag == "[jump action]") {
            ScanStr(&script->action_files.jump);
        } else if (tag == "[jumpattack action]") {
            ScanStr(&script->action_files.jump_attack);
        } else if (tag == "[attack action]" || tag == "[action list]") {
            script->action_files.attack_actions.clear();
            while (ScanStr(&value) == 1) {
                script->action_files.attack_actions.push_back(value);
            }
        } else if (tag == "[etc action]" || tag == "[custom action]") {
            script->action_files.etc_actions.clear();
            while (ScanStr(&value) == 1) {
                script->action_files.etc_actions.push_back(value);
            }
        } else if (tag == "[last action]") {
            ScanStr(&script->last_action);
        } else if (tag == "[proc action]") {
            ScanStr(&script->proc_action);
        } else if (tag == "[add gravity in aerial]") {
            script->add_gravity_in_aerial = ScanInt(static_cast<bool*>(0)) != 0;
            script->aerial_gravity_1 = ScanFloat(0, 0);
            script->aerial_gravity_2 = ScanFloat(0, 0);
        } else if (tag == "[speech on situation]") {
            for (int index = 0; index <= 110; ++index) {
                script->speech[index].clear();
            }
            script->attack_speech.clear();
            script->key_speech.clear();
            bool is_type = false;
            while (ScanTypeOrStr(is_type, value, true) == 1 &&
                   value != "[/speech on situation]") {
                std::string text;
                if (ScanStr(&text) != 1) {
                    return 0;
                }
                const int speech_value =
                    ScanInt(static_cast<bool*>(0));
                int speech_index = -1;
                if (value == "[on attack]" || value == "[on key]") {
                    const bool is_attack_speech = value == "[on attack]";
                    float rate = 0.0f;
                    const int result = ScanFloatStr(rate, value);
                    if (result == 0) {
                        speech_index = static_cast<int>(rate);
                    } else if (result == 1) {
                        for (std::size_t index = 0;
                             index < script->key_file_names.size();
                             ++index) {
                            if (script->key_file_names[index] == value) {
                                speech_index = static_cast<int>(index);
                                break;
                            }
                        }
                    }
                    if (speech_index < 0) {
                        return 0;
                    }
                    if (is_attack_speech) {
                        if (script->attack_speech.size() <=
                            static_cast<std::size_t>(speech_index)) {
                            script->attack_speech.resize(
                                static_cast<std::size_t>(speech_index) + 1);
                        }
                        script->attack_speech[speech_index].add(
                            text, speech_value);
                    } else {
                        if (script->key_speech.size() <=
                            static_cast<std::size_t>(speech_index)) {
                            script->key_speech.resize(
                                static_cast<std::size_t>(speech_index) + 1);
                        }
                        script->key_speech[speech_index].add(text,
                                                             speech_value);
                    }
                } else {
                    if (value == "[on damage]") {
                        speech_index = 101;
                    } else if (value == "[on die]") {
                        speech_index = 102;
                    } else if (value == "[on revenge]") {
                        speech_index = 103;
                    } else if (value == "[on etc]") {
                        speech_index = 104;
                    } else if (value == "[on appear]") {
                        speech_index = 105;
                    } else if (value == "[on disappear]") {
                        speech_index = 106;
                    } else if (value == "[on go to help]") {
                        speech_index = 107;
                    } else if (value == "[on stand]") {
                        speech_index = 108;
                    } else {
                        return 0;
                    }
                    script->speech[speech_index].add(text, speech_value);
                }
            }
        } else if (tag == "[common champion elemental property]") {
            script->common_champion_elements.clear();
            while (ScanStr(&value) == 1) {
                for (int element = 0; element <= 4; ++element) {
                    const ENUM_ELEMENT typed_element =
                        static_cast<ENUM_ELEMENT>(element);
                    if (value == getElementName(typed_element)) {
                        script->common_champion_elements.push_back(
                            typed_element);
                        break;
                    }
                }
            }
        } else if (tag == "[party member]") {
            script->party_members.clear();
            while (true) {
                const int member_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int member_value = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->party_members.push_back(
                    std::make_pair(member_index, member_value));
            }
        } else if (tag == "[far attack reaction rate]") {
            script->far_attack_reaction_rate = static_cast<int>(
                ScanFloat(&success) * 100.0f);
        } else if (tag == "[pvp win point]") {
            script->pvp_win_point_min = ScanInt(&success);
            if (!success) {
                return 0;
            }
            script->pvp_win_point_max = ScanInt(&success);
            if (!success ||
                script->pvp_win_point_max < script->pvp_win_point_min) {
                return 0;
            }
        } else if (tag == "[character status rate]") {
            for (int index = 0; index <= 9; ++index) {
                script->character_status_rate.rates[index] =
                    ScanFloat(0, 0);
            }
            for (int index = 31; index <= 40; ++index) {
                script->character_status_rate.rates[index] =
                    ScanFloat(0, 0);
            }
            for (int index = 10; index <= 13; ++index) {
                script->character_status_rate.rates[index] =
                    ScanFloat(0, 0);
            }
            for (int index = 14;; ++index) {
                const float rate = ScanFloat(&success);
                if (!success) {
                    break;
                }
                script->character_status_rate.rates[index] = rate;
            }
        } else if (tag == "[custom skill level]") {
            while (true) {
                STAICharacterCustomSkillLevel level;
                level.skill_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                level.level_1 = ScanInt(static_cast<bool*>(0));
                level.level_2 = ScanInt(static_cast<bool*>(0));
                level.level_3 = ScanInt(static_cast<bool*>(0));
                script->custom_skill_levels.push_back(level);
            }
        } else if (tag == "[attack damage rate]") {
            script->attack_damage_rate = ScanFloat(&success);
        } else if (tag == "[add equipment status from level]") {
            script->add_equipment_status_from_level = ScanInt(&success);
        } else if (tag == "[armor subtype]") {
            script->armor_subtype = ScanInt(&success);
        } else if (tag == "[default equipment]") {
            script->default_equipment = ScanInt(&success);
        } else if (tag == "[doll special dungeon]") {
            while (true) {
                const int dungeon = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->doll_special_dungeons.push_back(dungeon);
            }
        } else if (tag == "[character status rate special]") {
            for (int index = 0; index <= 9; ++index) {
                script->character_status_rate_special.rates[index] =
                    ScanFloat(0, 0);
            }
            for (int index = 31; index <= 40; ++index) {
                script->character_status_rate_special.rates[index] =
                    ScanFloat(0, 0);
            }
            for (int index = 10; index <= 13; ++index) {
                script->character_status_rate_special.rates[index] =
                    ScanFloat(0, 0);
            }
            for (int index = 14;; ++index) {
                const float rate = ScanFloat(&success);
                if (!success) {
                    break;
                }
                script->character_status_rate_special.rates[index] = rate;
            }
        } else if (tag == "[attack damage rate special]") {
            script->attack_damage_rate_special = ScanFloat(&success);
        } else if (tag == "[attack stuck rate]") {
            const float attack_stuck_rate = ScanFloat(&success);
            std::memcpy(&script->character_status.raw_53,
                        &attack_stuck_rate,
                        sizeof(attack_stuck_rate));
        } else if (tag == "[limit passiveobject]") {
            while (true) {
                const int object_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->limited_passive_objects.push_back(object_index);
            }
        } else if (tag == "[overhead gauge type]") {
            script->overhead_gauge_type = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[overhead gauge pos]") {
            script->overhead_gauge_x = ScanInt(static_cast<bool*>(0));
            script->overhead_gauge_y = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[getup invincible]") {
            script->getup_invincible = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[drop avatar item]") {
            for (int index = 0; index <= 3; ++index) {
                if (ScanFloat(&script->drop_avatar_info.part_rates[index]) !=
                    1) {
                    return 0;
                }
            }
            for (int index = 0; index <= 4; ++index) {
                if (ScanFloat(&script->drop_avatar_info.grade_rates[index]) !=
                    1) {
                    return 0;
                }
            }
            script->drop_avatar_info.base_rate = ScanInt(&success);
            if (!success) {
                return 0;
            }
            script->drop_avatar_info.drop_count = ScanInt(&success);
            if (!success) {
                return 0;
            }
            while (true) {
                stEnamyDropAvatarElement_t element;
                element.item_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int rate = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                element.avatar_part = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                script->drop_avatar_info.total_rate += rate;
                element.accumulated_rate = script->drop_avatar_info.total_rate;
                script->drop_avatar_info.elements.push_back(element);
            }
        } else if (tag == "[skill data up]") {
            importSkillDataUp(script->skill_data_up);
        } else if (tag == "[summon monster status rate]") {
            do {
                const float rate = ScanFloat(&success);
                script->summon_monster_status_rates.push_back(rate);
            } while (success);
        } else if (tag == "[no cooltime]") {
            script->no_cooltime = 1;
        } else if (tag == "[check accountitem]") {
            script->check_account_item = ScanInt(static_cast<bool*>(0)) == 1;
        } else if (tag == "[hell monster]") {
            script->hell_monster = ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[animation job]") {
            script->animation_job = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[revive option]") {
            std::string option_tag;
            while (ScanType(option_tag, true) == 1) {
                if (option_tag == "[/revive option]") {
                    break;
                }
                if (option_tag == "[map condition]") {
                    std::string condition;
                    if (ScanStr(&condition) != 1) {
                        return 0;
                    }
                    script->revive_option.map_conditions.push_back(condition);
                } else if (option_tag == "[condition]") {
                    if (ScanStr(&script->revive_option.active_object_state) !=
                        1) {
                        return 0;
                    }
                } else if (option_tag == "[revive]") {
                    script->revive_option.revive_value = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                } else {
                    return 0;
                }
            }
        } else if (tag == "[recovery option]") {
            std::string option_tag;
            while (ScanType(option_tag, true) == 1) {
                if (option_tag == "[/recovery option]") {
                    break;
                }
                if (option_tag == "[map condition]") {
                    std::string condition;
                    if (ScanStr(&condition) != 1) {
                        return 0;
                    }
                    script->recovery_option.map_conditions.push_back(
                        condition);
                } else if (option_tag == "[condition]") {
                    if (ScanStr(
                            &script->recovery_option.active_object_state) !=
                        1) {
                        return 0;
                    }
                } else if (option_tag == "[hp recovery]") {
                    script->recovery_option.hp_recovery_value =
                        ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                } else {
                    return 0;
                }
            }
        } else if (importAIInfoScript(&script->ai_info, &tag, false) != 1) {
            return 0;
        }
    }
    return 1;
}

namespace {

std::vector<stIndependentDropInfo>* AICharacterIndependentDropInfos() {
    CDataManager* data_manager = G_CDataManager();
    if (data_manager == 0) {
        return 0;
    }
    return &data_manager->get_independent_drop_parameter_script()
                ->independent_drop_info_;
}

}  // namespace

// IDA_EA: 0x834A066
// IDA_NAME: _ZN12CAICharacter3setEiRK25STAICharacterCommonScript
// DEMANGLED: CAICharacter::set(int,STAICharacterCommonScript const&)
CAICharacter* CAICharacter::set(
    int index,
    const STAICharacterCommonScript& script) {
    idx = index;
    level = script.minimum_info.level;
    rarity = script.rarity;
    appearance_point = script.appearance_point;
    death_tower_items = script.death_tower_items;
    pvp_win_point_min = script.pvp_win_point_min;
    pvp_win_point_max = script.pvp_win_point_max;
    independent_drop_items = script.independent_drops;
    drop_avatar_info = script.drop_avatar_info;
    name = script.minimum_info.name;
    hell_monster = script.hell_monster;
    return this;
}

// IDA_EA: 0x8150850
// IDA_NAME: _ZNK12CAICharacter6getIdxEv
// DEMANGLED: CAICharacter::getIdx(void) const
int CAICharacter::getIdx() const {
    return idx;
}

// IDA_EA: 0x815085A
// IDA_NAME: _ZNK12CAICharacter8getLevelEv
// DEMANGLED: CAICharacter::getLevel(void) const
int CAICharacter::getLevel() const {
    return level;
}

// IDA_EA: 0x8150866
// IDA_NAME: _ZNK12CAICharacter18getAppearancePointEv
// DEMANGLED: CAICharacter::getAppearancePoint(void) const
int CAICharacter::getAppearancePoint() const {
    return appearance_point;
}

// IDA_EA: 0x834A19C
// IDA_NAME: _ZNK12CAICharacter25deathTowerSpecifyItemDropERSt6vectorImSaImEE
// DEMANGLED: CAICharacter::deathTowerSpecifyItemDrop(std::vector<unsigned long>&) const
bool CAICharacter::deathTowerSpecifyItemDrop(
    std::vector<unsigned long>& item_ids) const {
    for (std::vector<std::pair<int32_t, int32_t> >::const_iterator it =
             death_tower_items.begin();
         it != death_tower_items.end();
         ++it) {
        if (it->second > static_cast<int>(get_rand_int(10000))) {
            item_ids.push_back(static_cast<unsigned long>(it->first));
        }
    }

    // The original body returns the final iterator comparison after its loop,
    // which is false once all candidate pairs have been considered.
    return false;
}

// IDA_EA: 0x834A240
// IDA_NAME: _ZNK12CAICharacter14getPvPWinPointEv
// DEMANGLED: CAICharacter::getPvPWinPoint(void) const
int CAICharacter::getPvPWinPoint() const {
    if (pvp_win_point_min == pvp_win_point_max) {
        return pvp_win_point_min;
    }
    return pvp_win_point_min + static_cast<int>(
        get_rand_int(pvp_win_point_max - pvp_win_point_min + 1));
}

// IDA_EA: 0x8373A8E
// IDA_NAME: _ZNK12CAICharacter9getRarityEv
// DEMANGLED: CAICharacter::getRarity(void) const
int CAICharacter::getRarity() const {
    return rarity;
}

// IDA_EA: 0x8373A9A
// IDA_NAME: _ZN12CAICharacterC2Ev
// DEMANGLED: CAICharacter::CAICharacter(void)
CAICharacter::CAICharacter()
    : death_tower_items(),
      independent_drop_items(),
      drop_avatar_info(),
      name() {}

// IDA_EA: 0x8373B32
// IDA_NAME: _ZN12CAICharacterD2Ev
// DEMANGLED: CAICharacter::~CAICharacter()
CAICharacter::~CAICharacter() {}

// IDA_EA: 0x84B410E
// IDA_NAME: _ZNK12CAICharacter19getIndepentDropItemEv
// DEMANGLED: CAICharacter::getIndepentDropItem(void) const
std::vector<stMonsterIndependentDrop_t>
CAICharacter::getIndepentDropItem() const {
    return independent_drop_items;
}

// IDA_EA: 0x84B4138
// IDA_NAME: _ZNK12CAICharacter7GetNameEv
// DEMANGLED: CAICharacter::GetName(void) const
const char* CAICharacter::GetName() const {
    return name.c_str();
}

// IDA_EA: 0x8539DE8
// IDA_NAME: _ZNK12CAICharacter13IsHellMonsterEv
// DEMANGLED: CAICharacter::IsHellMonster(void) const
int CAICharacter::IsHellMonster() const {
    return hell_monster;
}

// IDA_EA: 0x8150872
// IDA_NAME: _ZN16CAICharacterList8getMinAPEv
// DEMANGLED: CAICharacterList::getMinAP(void)
int CAICharacterList::getMinAP() {
    return min_ap;
}

// IDA_EA: 0x8150880
// IDA_NAME: _ZNK16CAICharacterList10getAPCountEv
// DEMANGLED: CAICharacterList::getAPCount(void) const
const std::map<int, int>& CAICharacterList::getAPCount() const {
    return ap_count;
}

// IDA_EA: 0x834A59A
// IDA_NAME: _ZN16CAICharacterList7destroyEv
// DEMANGLED: CAICharacterList::destroy(void)
int CAICharacterList::destroy() {
    for (std::map<int, CAICharacter*>::iterator it = characters.begin();
         it != characters.end();
         ++it) {
        CAICharacter* character = it->second;
        if (character != 0) {
            character->~CAICharacter();
            operator delete(character);
        }
    }
    // IDA leaves the index map populated; its destructor releases nodes after
    // this explicit object destruction pass.
    return 0;
}

// IDA_EA: 0x834A61C
// IDA_NAME: _ZN16CAICharacterList4_addEP12CAICharacter
// DEMANGLED: CAICharacterList::_add(CAICharacter *)
int CAICharacterList::_add(CAICharacter* character) {
    const std::pair<std::map<int, CAICharacter*>::iterator, bool> inserted =
        characters.insert(std::make_pair(character->getIdx(), character));
    if (!inserted.second) {
        return 0;
    }

    const uint32_t character_level =
        static_cast<uint32_t>(character->getLevel());
    for (uint32_t level_index = character_level - 5u;
         ;
         ++level_index) {
        if (character_level + 5u <= level_index) {
            break;
        }
        if (level_index > 98u) {
            cMyTrace trace("bool CAICharacterList::_add(CAICharacter*)",
                           1335,
                           0);
            trace("aiCharacter Script file load failed");
        }
        by_level[level_index].push_back(character);
    }

    const int appearance = character->getAppearancePoint();
    if (appearance != 0) {
        std::map<int, std::vector<CAICharacter*> >::iterator by_ap =
            by_appearance_point.find(appearance);
        if (by_ap != by_appearance_point.end()) {
            by_ap->second.push_back(character);
        } else {
            std::vector<CAICharacter*> characters_by_ap;
            characters_by_ap.push_back(character);
            by_appearance_point.insert(std::make_pair(appearance,
                                                       characters_by_ap));
        }

        if (min_ap >= appearance) {
            min_ap = appearance;
        }

        const std::pair<std::map<int, int>::iterator, bool> count =
            ap_count.insert(std::make_pair(appearance, 1));
        if (!count.second) {
            ++count.first->second;
        }
    }

    by_rarity[character->getRarity()].push_back(character);
    return 1;
}

// IDA_EA: 0x834A9D2
// IDA_NAME: _ZN16CAICharacterList3getEj
// DEMANGLED: CAICharacterList::get(unsigned int)
CAICharacter* CAICharacterList::get(uint32_t idx) {
    const std::map<int, CAICharacter*>::const_iterator found =
        characters.find(static_cast<int>(idx));
    return found == characters.end() ? 0 : found->second;
}

// IDA_EA: 0x834AA60
// IDA_NAME: _ZN16CAICharacterList10getByLevelEj
// DEMANGLED: CAICharacterList::getByLevel(unsigned int)
CAICharacter* CAICharacterList::getByLevel(uint32_t standard_level) {
    const uint32_t randomized_level =
        standard_level + static_cast<uint32_t>(get_rand_int(10));
    const uint32_t level_index = randomized_level - 5u;
    if (randomized_level == 5u || level_index > 98u) {
        return 0;
    }

    std::vector<CAICharacter*>& candidates = by_level[level_index];
    if (candidates.empty()) {
        return 0;
    }
    return candidates.at(get_rand_int(static_cast<int>(candidates.size())));
}

// IDA_EA: 0x834AAF4
// IDA_NAME: _ZN16CAICharacterList7getByAPEj
// DEMANGLED: CAICharacterList::getByAP(unsigned int)
CAICharacter* CAICharacterList::getByAP(uint32_t appearance_point) {
    if (by_appearance_point.empty()) {
        return 0;
    }

    std::map<int, std::vector<CAICharacter*> >::iterator selected =
        by_appearance_point.lower_bound(static_cast<int>(appearance_point));
    if (selected == by_appearance_point.end()) {
        --selected;
    }
    if (selected->first > static_cast<int>(appearance_point) &&
        selected != by_appearance_point.begin()) {
        --selected;
    }

    std::vector<CAICharacter*>& candidates = selected->second;
    return candidates.at(get_rand_int(static_cast<int>(candidates.size())));
}

// IDA_EA: 0x8373BF6
// IDA_NAME: _ZN16CAICharacterListC2Ev
// DEMANGLED: CAICharacterList::CAICharacterList(void)
CAICharacterList::CAICharacterList()
    : characters(),
      by_appearance_point(),
      ap_count() {}

// IDA_EA: 0x8373D9E
// IDA_NAME: _ZN16CAICharacterListD2Ev
// DEMANGLED: CAICharacterList::~CAICharacterList()
CAICharacterList::~CAICharacterList() {
    destroy();
}

// IDA_EA: 0x834A28C
// IDA_NAME: _ZN16CAICharacterList4initEv
// DEMANGLED: CAICharacterList::init(void)
int CAICharacterList::init() {
    sync_script::CSyncScript* sync =
        sync_script::GetInstanceSyncScript();
    if (sync->truncate_aicharacter_info_table() != true) {
        std::puts("Fail truncate_aicharacter_info_table");
        return 0;
    }

    min_ap = 0x7FFFFFFF;
    STAICharacterCommonScript script;
    for (STScriptFileList::iterator it = g_aiCharacterScriptFileList.begin();
         it != g_aiCharacterScriptFileList.end();
         ++it) {
        const char* filename = it->second.c_str();
        if (filename == 0) {
            break;
        }

        script.clear();
        CAICharacter* character = new CAICharacter();
        if (importAICharacterCommonScript(&script, filename, false) != 1) {
            std::printf("AI charac Script Error: %s\n", filename);
            delete character;
            return 0;
        }

        const int index =
            g_aiCharacterScriptFileList.FindIndexByFullScan(filename);
        if (sync->insert_aicharacter_info_to_db(index, script) != 1) {
            std::printf("insert_aicharacter_category Error: %s\n", filename);
            delete character;
            return 0;
        }

        std::vector<stIndependentDropInfo>* independent_drops =
            AICharacterIndependentDropInfos();
        if (independent_drops != 0) {
            for (std::vector<stIndependentDropInfo>::iterator drop =
                     independent_drops->begin();
                 drop != independent_drops->end();
                 ++drop) {
                if (drop->dungeon_index == 1 && drop->start_level == index) {
                    script.independent_drops.push_back(drop->drop);
                }
            }
        }

        character->set(index, script);
        _add(character);
    }
    return 1;
}
