#include "old_data/PVF/character/datamanager/character_script_runtime.h"

#include "common/config.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "common/manager/character_script_public_abi.h"
#include "common/manager/global_tools.h"

#include <stdint.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <new>


extern char g_szCharacterScriptBaseDirectory[256];

namespace {

typedef std::vector<CharacterCreateEquipmentEntry> CreateEquipmentList;
typedef std::map<int, CreateEquipmentList> CreateEquipmentTable;

CreateEquipmentTable& PublishedCreateEquipment() {
    static CreateEquipmentTable* table = new CreateEquipmentTable;
    return *table;
}

CreateEquipmentTable& StagedCreateEquipment() {
    static CreateEquipmentTable* table = new CreateEquipmentTable;
    return *table;
}

bool& CreateEquipmentLoadActive() {
    static bool active = false;
    return active;
}

typedef char STCharacterScriptActiveActionFileSet_Size_Must_Be_0x40[
    (sizeof(STCharacterScriptActiveActionFileSet) == 0x40) ? 1 : -1];
typedef char STCharacterScriptWeaponInfo_Size_Must_Be_0x34[
    (sizeof(STCharacterScriptWeaponInfo) == 0x34) ? 1 : -1];
typedef char STCharacterScriptComboInfo_Size_Must_Be_0x3c[
    (sizeof(STCharacterScriptComboInfo) == 0x3c) ? 1 : -1];
typedef char STCharacterScript_Size_Must_Be_0xc1c[
    (sizeof(STCharacterScript) == 0xc1c) ? 1 : -1];

const char* const kActiveStatusTags[17] = {
    "[slow resistance]",
    "[freeze resistance]",
    "[poison resistance]",
    "[stun resistance]",
    "[curse resistance]",
    "[blind resistance]",
    "[lightning resistance]",
    "[stone resistance]",
    "[sleep resistance]",
    "[burn resistance]",
    "[weapon break resistance]",
    "[bleeding resistance]",
    "[haste resistance]",
    "[bless resistance]",
    "[element resistance]",
    "[confuse resistance]",
    "[hold resistance]",
};

int ComboInfo_clear(STCharacterScriptComboInfo* self) {
    self->first_.clear();
    self->second_.clear();
    self->values_.clear();
    return 0;
}

bool Eq(const std::string& lhs, const char* rhs) {
    return lhs == rhs;
}

bool ScanRequiredStr(std::string* output) {
    return ScanStr(output) == 1;
}

bool ScanRequiredInt(int* output) {
    return ScanInt(output) == 1;
}

bool ScanRequiredFloat(float* output) {
    return ScanFloat(output) == 1;
}

bool SkipCharacterClientBlock(const char* end_tag) {
    while (1) {
        const DNFLEX_TOKEN token = g_dnfLex.getToken();
        if (token == DNF_TOK_EOF || token == DNF_TOK_ERROR) {
            return false;
        }
        if (token == DNF_TOK_TYPE &&
            std::strcmp(g_dnfLex.getType(), end_tag) == 0) {
            return true;
        }
    }
}

bool ParseCreateEquipmentList(CreateEquipmentList* equipment) {
    if (!equipment) return false;
    CreateEquipmentList parsed;
    bool seen_weapon = false;
    bool seen_coat = false;
    bool seen_pants = false;

    while (true) {
        bool is_type = false;
        std::string token;
        if (ScanTypeOrStr(is_type, token, true) != 1) return false;
        if (is_type) {
            if (token != "[/create equipment list]") return false;
            if (!seen_weapon || !seen_coat || !seen_pants ||
                parsed.size() != 3u) {
                return false;
            }
            equipment->swap(parsed);
            return true;
        }

        int equipment_type = oldsrv::config::kEquipmentTypeInvalid;
        bool* seen = 0;
        if (token == "[weapon]") {
            equipment_type = oldsrv::config::kEquipmentTypeWeapon;
            seen = &seen_weapon;
        } else if (token == "[coat]") {
            equipment_type = oldsrv::config::kEquipmentTypeCoat;
            seen = &seen_coat;
        } else if (token == "[pants]") {
            equipment_type = oldsrv::config::kEquipmentTypePants;
            seen = &seen_pants;
        } else {
            return false;
        }
        if (*seen) return false;

        int item_id = 0;
        if (!ScanRequiredInt(&item_id) || item_id <= 0) return false;
        *seen = true;
        parsed.push_back(
            CharacterCreateEquipmentEntry(equipment_type, item_id));
    }
}

bool PublishImportedCreateEquipment(int character_job,
                                    const CreateEquipmentList& equipment) {
    if (character_job < 0 ||
        character_job >= oldsrv::config::kCharacterJobCount ||
        equipment.size() != 3u) {
        return false;
    }
    CreateEquipmentTable& table = CreateEquipmentLoadActive()
                                      ? StagedCreateEquipment()
                                      : PublishedCreateEquipment();
    if (CreateEquipmentLoadActive()) {
        return table.insert(std::make_pair(character_job, equipment)).second;
    }
    table[character_job] = equipment;
    return true;
}

int RoundTenths(float value) {
    return GetRoundInt(value * 10.0f);
}

int FloatBits(float value) {
    int bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}

bool ScanStatusFloat(int16_t& destination) {
    float value = 0.0f;
    if (!ScanRequiredFloat(&value)) {
        return false;
    }
    destination = static_cast<int16_t>(RoundTenths(value));
    return true;
}

// The original parser wrote these slots through an int16_t pointer.  Preserve
// that narrowing and its stored two-byte representation even though the
// physical status declaration exposes these particular members as uint16_t.
bool ScanStatusFloat(uint16_t& destination) {
    float value = 0.0f;
    if (!ScanRequiredFloat(&value)) {
        return false;
    }
    destination = static_cast<uint16_t>(
        static_cast<int16_t>(RoundTenths(value)));
    return true;
}

bool ScanStatusFloat32(uint32_t& destination) {
    float value = 0.0f;
    if (!ScanRequiredFloat(&value)) {
        return false;
    }
    destination = static_cast<uint32_t>(RoundTenths(value));
    return true;
}

bool ParsePairIntVector(std::vector<std::pair<int, int> >* values,
                        const char* end_tag) {
    std::string token;
    bool is_type = false;
    bool success = false;
    values->clear();
    while (1) {
        const int first = ScanTypeOrInt(is_type, token, success, true);
        if (!success) {
            return false;
        }
        if (is_type) {
            return token == end_tag;
        }
        int second = 0;
        if (!ScanRequiredInt(&second)) {
            return false;
        }
        values->push_back(std::make_pair(first, second));
    }
}

bool ParseStringVector(std::vector<std::string>* values,
                       const char* end_tag) {
    std::string token;
    bool is_type = false;
    values->clear();
    while (1) {
        if (ScanTypeOrStr(is_type, token, true) != 1) {
            return false;
        }
        if (is_type) {
            return token == end_tag;
        }
        values->push_back(token);
    }
}

bool ParseCategoryVector(std::vector<int>* values, const char* end_tag) {
    std::string token;
    bool is_type = false;
    values->clear();
    while (1) {
        if (ScanTypeOrStr(is_type, token, true) != 1) {
            return false;
        }
        if (is_type) {
            return token == end_tag;
        }
        const int category = getObjectCategory(token);
        if (category != 44) {
            values->push_back(category);
        }
    }
}

bool ParseWeaponWav(std::vector<STCharacterScriptWeaponInfo>* values,
                    const char* end_tag) {
    std::string token;
    bool is_type = false;
    STCharacterScriptWeaponInfo info;
    while (1) {
        if (ScanTypeOrStr(is_type, token, true) != 1) {
            return false;
        }
        if (is_type) {
            if (token != end_tag) {
                return false;
            }
            values->push_back(info);
            return true;
        }
        info.attack_files_.push_back(token);
    }
}

bool EnsureWeaponInfo(std::vector<STCharacterScriptWeaponInfo>* values,
                      unsigned int index) {
    if (values->size() <= index) {
        values->resize(index + 1);
    }
    return true;
}

void ExportStringVector(FILE* stream,
                        const char* open_tag,
                        const char* close_tag,
                        const std::vector<std::string>* values) {
    if (values->empty()) {
        return;
    }
    std::fprintf(stream, "%s\n", open_tag);
    for (std::vector<std::string>::const_iterator it = values->begin();
         it != values->end();
         ++it) {
        std::fprintf(stream, "`%s`\n", it->c_str());
    }
    std::fprintf(stream, "%s\n", close_tag);
}

void ExportNonEmptyString(FILE* stream,
                          const char* tag,
                          const std::string* value) {
    if (!value->empty()) {
        std::fprintf(stream, "%s `%s`\n", tag, value->c_str());
    }
}

}  // namespace

CharacterCreateEquipmentEntry::CharacterCreateEquipmentEntry()
    : equipment_type(oldsrv::config::kEquipmentTypeInvalid), item_id(0) {}

CharacterCreateEquipmentEntry::CharacterCreateEquipmentEntry(
    int32_t equipment_type_value,
    int32_t item_id_value)
    : equipment_type(equipment_type_value), item_id(item_id_value) {}

void BeginCharacterCreateEquipmentLoad() {
    StagedCreateEquipment().clear();
    CreateEquipmentLoadActive() = true;
}

void CommitCharacterCreateEquipmentLoad() {
    if (!CreateEquipmentLoadActive()) return;
    PublishedCreateEquipment().swap(StagedCreateEquipment());
    StagedCreateEquipment().clear();
    CreateEquipmentLoadActive() = false;
}

void AbortCharacterCreateEquipmentLoad() {
    StagedCreateEquipment().clear();
    CreateEquipmentLoadActive() = false;
}

bool FindCharacterCreateEquipment(int character_job,
                                  int equipment_type,
                                  int* item_id) {
    if (item_id) *item_id = 0;
    const CreateEquipmentTable::const_iterator found =
        PublishedCreateEquipment().find(character_job);
    if (found == PublishedCreateEquipment().end()) return false;
    for (CreateEquipmentList::const_iterator it = found->second.begin();
         it != found->second.end(); ++it) {
        if (it->equipment_type == equipment_type) {
            if (item_id) *item_id = it->item_id;
            return it->item_id > 0;
        }
    }
    return false;
}

// IDA_EA: 0x8367A98
// IDA_NAME: _ZN17STCharacterScript5ClearEv
// DEMANGLED: STCharacterScript::Clear(void)
int STCharacterScript::Clear() {
    character_job_ = oldsrv::config::kCharacterJobInvalid;
    width_ = 0;
    height_ = 0;

    for (int n5 = 0; n5 <= 5; ++n5) {
        growtype_names_[n5].assign("");
        for (int i = 0; i <= 1; ++i) {
            awakening_names_[n5][i].assign("");
        }
    }

    for (int n6 = 0; n6 <= 6; ++n6) {
        growtype_status_[n6].Clear();
        give_skills_[n6].clear();
        etc_stats_[n6].clear();
        second_grow_type_data_[n6].clear();
    }

    waiting_motion_.assign("");
    move_motion_.assign("");
    sit_motion_.assign("");
    damage_motion_1_.assign("");
    damage_motion_2_.assign("");
    down_motion_.assign("");
    overturn_motion_.assign("");
    jump_motion_.assign("");
    jumpattack_motion_.assign("");
    rest_motion_.assign("");
    throw_motion_1_1_.assign("");
    throw_motion_2_1_.assign("");
    throw_motion_3_1_.assign("");
    throw_motion_4_1_.assign("");
    throw_motion_1_2_.assign("");
    throw_motion_2_2_.assign("");
    throw_motion_3_2_.assign("");
    throw_motion_4_2_.assign("");
    dash_motion_.assign("");
    dashattack_motion_.assign("");
    getitem_motion_.assign("");
    buff_motion_.assign("");

    // The original Clear() deliberately leaves simple/ghost/back motions,
    // active actions, last/proc actions and source_filename_ untouched.
    attack_motion_.clear();
    etc_motion_.clear();
    jumpattack_info_.assign("");
    dashattack_info_.assign("");
    attack_info_.clear();
    etc_attack_info_.clear();
    weapon_infos_.clear();
    source_full_path_.assign("");

    jump_speed_ = 100;
    start_town_ = 0;
    field_bd8_ = 1.0f;
    return ComboInfo_clear(&combo_info_);
}

// IDA_EA: 0x8368348
// IDA_NAME: _ZN17STCharacterScriptC2Ev
// DEMANGLED: STCharacterScript::STCharacterScript(void)
STCharacterScript::STCharacterScript() { Clear(); }

// IDA_EA: 0x8376362
// IDA_NAME: _ZN17STCharacterScriptD2Ev
// DEMANGLED: STCharacterScript::~STCharacterScript()
STCharacterScript::~STCharacterScript() {}

int ImportCharacterScript(STCharacterScript* script, const char* filename);

// IDA_EA: 0x8A0CE06
// IDA_NAME: _Z21ImportCharacterScriptP17STCharacterScriptPKc
// DEMANGLED: ImportCharacterScript(STCharacterScript *,char const*)
int ImportCharacterScript(STCharacterScript* script, const char* filename) {
    if (loadRDARScriptFile(g_szCharacterScriptBaseDirectory, filename) != 1) {
        return 0;
    }

    script->Clear();
    script->source_filename_.assign(filename);
    getDirectoryPath(script->source_full_path_,
                     "%s%s",
                     g_szCharacterScriptBaseDirectory,
                     filename);

    std::string type;
    std::string text;
    int scanned_int = 0;
    float scanned_float = 0.0f;
    CreateEquipmentList create_equipment;
    bool has_create_equipment = false;
    bool success = false;

    STRDCharacterStatus* status = &script->growtype_status_[0];
    std::vector<std::pair<int, int> >* skills =
        &script->give_skills_[0];
    CharacterEtcStats* etc_stats = &script->etc_stats_[0];
    STCharacterScript::STSecondGrowTypeData* grow = 0;
    std::string* awakening_names = 0;

    while (ScanType(type, true) == 1) {
        if (Eq(type, "[job]")) {
            if (!ScanRequiredStr(&text)) {
                return 0;
            }
            script->character_job_ = ::getCharacterJob(text.c_str());
        } else if (Eq(type, "[growtype name]")) {
            for (int index = 0; index <= 5; ++index) {
                if (!ScanRequiredStr(&script->growtype_names_[index])) {
                    return 0;
                }
            }
        } else if (Eq(type, "[width]")) {
            if (!ScanRequiredInt(&scanned_int)) {
                return 0;
            }
            script->width_ = scanned_int;
            if (!ScanRequiredInt(&scanned_int)) {
                return 0;
            }
            script->height_ = scanned_int;
        } else if (Eq(type, "[jump speed]")) {
            if (!ScanRequiredInt(&scanned_int)) {
                return 0;
            }
            script->jump_speed_ = scanned_int;
        } else if (Eq(type, "[start town]")) {
            if (!ScanRequiredInt(&scanned_int) ||
                scanned_int <= 0 || scanned_int >= 256) {
                return 0;
            }
            script->start_town_ = scanned_int;
        } else if (Eq(type, "[initial value]")) {
            status = &script->growtype_status_[0];
            skills = &script->give_skills_[0];
            etc_stats = &script->etc_stats_[0];
            grow = &script->second_grow_type_data_[0];
            awakening_names = 0;
        } else if (Eq(type, "[growtype 1]") || Eq(type, "[growtype 2]") ||
                   Eq(type, "[growtype 3]") || Eq(type, "[growtype 4]") ||
                   Eq(type, "[growtype 5]") || Eq(type, "[growtype 6]")) {
            const int grow_index = type[10] - '1';
            status = &script->growtype_status_[grow_index + 1];
            skills = &script->give_skills_[grow_index + 1];
            etc_stats = &script->etc_stats_[grow_index + 1];
            grow = &script->second_grow_type_data_[grow_index + 1];
            awakening_names = script->awakening_names_[grow_index];
        } else if (Eq(type, "[awakening 1]")) {
            if (!grow) {
                return 0;
            }
            status = &grow->status[0];
            skills = &grow->skill_values[0];
            etc_stats = &grow->etc_stats[0];
        } else if (Eq(type, "[awakening 2]")) {
            if (!grow) {
                return 0;
            }
            status = &grow->status[1];
            skills = &grow->skill_values[1];
            etc_stats = &grow->etc_stats[1];
        } else if (Eq(type, "[awakening name]")) {
            if (!awakening_names || !ScanRequiredStr(&awakening_names[0]) ||
                !ScanRequiredStr(&awakening_names[1])) {
                return 0;
            }
        } else if (Eq(type, "[awakening skill]")) {
            if (!ParsePairIntVector(skills, "[/awakening skill]")) {
                return 0;
            }
        } else if (Eq(type, "[HP MAX]")) {
            if (!ScanRequiredFloat(&scanned_float)) {
                return 0;
            }
            status->raw_00 = static_cast<uint32_t>(RoundTenths(scanned_float));
        } else if (Eq(type, "[MP MAX]")) {
            if (!ScanRequiredFloat(&scanned_float)) {
                return 0;
            }
            status->raw_04 = static_cast<uint32_t>(RoundTenths(scanned_float));
        } else if (Eq(type, "[physical attack]")) {
            if (!ScanStatusFloat(status->raw_08)) {
                return 0;
            }
        } else if (Eq(type, "[physical defense]")) {
            if (!ScanStatusFloat(status->raw_0a)) {
                return 0;
            }
        } else if (Eq(type, "[magical attack]")) {
            if (!ScanStatusFloat(status->raw_0c)) {
                return 0;
            }
        } else if (Eq(type, "[magical defense]")) {
            if (!ScanStatusFloat(status->raw_0e)) {
                return 0;
            }
        } else if (Eq(type, "[fire resistance]")) {
            if (!ScanStatusFloat(status->four_values[0])) {
                return 0;
            }
        } else if (Eq(type, "[water resistance]")) {
            if (!ScanStatusFloat(status->four_values[1])) {
                return 0;
            }
        } else if (Eq(type, "[dark resistance]")) {
            if (!ScanStatusFloat(status->four_values[2])) {
                return 0;
            }
        } else if (Eq(type, "[light resistance]")) {
            if (!ScanStatusFloat(status->four_values[3])) {
                return 0;
            }
        } else if (Eq(type, "[inventory limit]")) {
            if (!ScanStatusFloat32(status->raw_3a)) {
                return 0;
            }
        } else if (Eq(type, "[HP regen speed]")) {
            if (!ScanStatusFloat(status->raw_3e)) {
                return 0;
            }
        } else if (Eq(type, "[MP regen speed]")) {
            if (!ScanStatusFloat(status->raw_40)) {
                return 0;
            }
        } else if (Eq(type, "[move speed]")) {
            if (!ScanStatusFloat32(status->raw_42)) {
                return 0;
            }
        } else if (Eq(type, "[attack speed]")) {
            if (!ScanStatusFloat(status->raw_46)) {
                return 0;
            }
        } else if (Eq(type, "[cast speed]")) {
            if (!ScanStatusFloat(status->raw_48)) {
                return 0;
            }
        } else if (Eq(type, "[hit recovery]")) {
            if (!ScanStatusFloat(status->raw_4a)) {
                return 0;
            }
        } else if (Eq(type, "[jump power]")) {
            if (!ScanStatusFloat(status->raw_4c)) {
                return 0;
            }
        } else if (Eq(type, "[weight]")) {
            if (!ScanStatusFloat32(status->raw_4e)) {
                return 0;
            }
        } else if (Eq(type, "[skill]")) {
            if (!ParsePairIntVector(skills, "[/skill]")) {
                return 0;
            }
        } else if (Eq(type, "[category]")) {
            if (!ParseCategoryVector(&etc_stats->values, "[/category]")) {
                return 0;
            }
        } else if (Eq(type, "[module damage rate]")) {
            for (int index = 0; index <= 3; ++index) {
                if (!ScanRequiredFloat(&scanned_float)) {
                    return 0;
                }
                etc_stats->raw_0c[index] = FloatBits(scanned_float);
            }
        } else if (Eq(type, "[waiting motion]")) {
            if (!ScanRequiredStr(&script->waiting_motion_)) return 0;
        } else if (Eq(type, "[move motion]")) {
            if (!ScanRequiredStr(&script->move_motion_)) return 0;
        } else if (Eq(type, "[sit motion]")) {
            if (!ScanRequiredStr(&script->sit_motion_)) return 0;
        } else if (Eq(type, "[damage motion 1]")) {
            if (!ScanRequiredStr(&script->damage_motion_1_)) return 0;
        } else if (Eq(type, "[damage motion 2]")) {
            if (!ScanRequiredStr(&script->damage_motion_2_)) return 0;
        } else if (Eq(type, "[down motion]")) {
            if (!ScanRequiredStr(&script->down_motion_)) return 0;
        } else if (Eq(type, "[overturn motion]")) {
            if (!ScanRequiredStr(&script->overturn_motion_)) return 0;
        } else if (Eq(type, "[jump motion]")) {
            if (!ScanRequiredStr(&script->jump_motion_)) return 0;
        } else if (Eq(type, "[jumpattack motion]")) {
            if (!ScanRequiredStr(&script->jumpattack_motion_)) return 0;
        } else if (Eq(type, "[rest motion]")) {
            if (!ScanRequiredStr(&script->rest_motion_)) return 0;
        } else if (Eq(type, "[simple rest motion]")) {
            if (!ScanRequiredStr(&script->simple_rest_motion_)) return 0;
        } else if (Eq(type, "[simple move motion]")) {
            if (!ScanRequiredStr(&script->simple_move_motion_)) return 0;
        } else if (Eq(type, "[throw motion 1-1]")) {
            if (!ScanRequiredStr(&script->throw_motion_1_1_)) return 0;
        } else if (Eq(type, "[throw motion 2-1]")) {
            if (!ScanRequiredStr(&script->throw_motion_2_1_)) return 0;
        } else if (Eq(type, "[throw motion 3-1]")) {
            if (!ScanRequiredStr(&script->throw_motion_3_1_)) return 0;
        } else if (Eq(type, "[throw motion 4-1]")) {
            if (!ScanRequiredStr(&script->throw_motion_4_1_)) return 0;
        } else if (Eq(type, "[throw motion 1-2]")) {
            if (!ScanRequiredStr(&script->throw_motion_1_2_)) return 0;
        } else if (Eq(type, "[throw motion 2-2]")) {
            if (!ScanRequiredStr(&script->throw_motion_2_2_)) return 0;
        } else if (Eq(type, "[throw motion 3-2]")) {
            if (!ScanRequiredStr(&script->throw_motion_3_2_)) return 0;
        } else if (Eq(type, "[throw motion 4-2]")) {
            if (!ScanRequiredStr(&script->throw_motion_4_2_)) return 0;
        } else if (Eq(type, "[dash motion]")) {
            if (!ScanRequiredStr(&script->dash_motion_)) return 0;
        } else if (Eq(type, "[dashattack motion]")) {
            if (!ScanRequiredStr(&script->dashattack_motion_)) return 0;
        } else if (Eq(type, "[getitem motion]")) {
            if (!ScanRequiredStr(&script->getitem_motion_)) return 0;
        } else if (Eq(type, "[buff motion]")) {
            if (!ScanRequiredStr(&script->buff_motion_)) return 0;
        } else if (Eq(type, "[back motion]")) {
            if (!ScanRequiredStr(&script->back_motion_)) return 0;
        } else if (Eq(type, "[attack motion]")) {
            if (!ParseStringVector(&script->attack_motion_,
                                   "[/attack motion]")) {
                return 0;
            }
        } else if (Eq(type, "[ghost motion]")) {
            if (!ParseStringVector(&script->ghost_motion_,
                                   "[/ghost motion]")) {
                return 0;
            }
        } else if (Eq(type, "[etc motion]")) {
            if (!ParseStringVector(&script->etc_motion_,
                                   "[/etc motion]")) {
                return 0;
            }
        } else if (Eq(type, "[waiting action]")) {
            if (!ScanRequiredStr(&script->active_actions_.waiting_action_)) return 0;
        } else if (Eq(type, "[move action]")) {
            if (!ScanRequiredStr(&script->active_actions_.move_action_)) return 0;
        } else if (Eq(type, "[dash action]")) {
            if (!ScanRequiredStr(&script->active_actions_.dash_action_)) return 0;
        } else if (Eq(type, "[sit action]")) {
            if (!ScanRequiredStr(&script->active_actions_.sit_action_)) return 0;
        } else if (Eq(type, "[damage action 1]")) {
            if (!ScanRequiredStr(&script->active_actions_.damage_action_1_)) return 0;
        } else if (Eq(type, "[damage action 2]")) {
            if (!ScanRequiredStr(&script->active_actions_.damage_action_2_)) return 0;
        } else if (Eq(type, "[down action]")) {
            if (!ScanRequiredStr(&script->active_actions_.down_action_)) return 0;
        } else if (Eq(type, "[overturn action]")) {
            if (!ScanRequiredStr(&script->active_actions_.overturn_action_)) return 0;
        } else if (Eq(type, "[jump action]")) {
            if (!ScanRequiredStr(&script->active_actions_.jump_action_)) return 0;
        } else if (Eq(type, "[jumpattack action]")) {
            if (!ScanRequiredStr(&script->active_actions_.jumpattack_action_)) return 0;
        } else if (Eq(type, "[attack action]")) {
            if (!ParseStringVector(&script->active_actions_.attack_actions_,
                                   "[/attack action]")) {
                return 0;
            }
        } else if (Eq(type, "[action list]")) {
            if (!ParseStringVector(&script->active_actions_.attack_actions_,
                                   "[/action list]")) {
                return 0;
            }
        } else if (Eq(type, "[etc action]")) {
            if (!ParseStringVector(&script->active_actions_.etc_actions_,
                                   "[/etc action]")) {
                return 0;
            }
        } else if (Eq(type, "[last action]")) {
            if (!ScanRequiredStr(&script->last_action_)) return 0;
        } else if (Eq(type, "[proc action]")) {
            if (!ScanRequiredStr(&script->proc_action_)) return 0;
        } else if (Eq(type, "[jumpattack info]")) {
            if (!ScanRequiredStr(&script->jumpattack_info_)) return 0;
        } else if (Eq(type, "[dashattack info]")) {
            if (!ScanRequiredStr(&script->dashattack_info_)) return 0;
        } else if (Eq(type, "[attack info]")) {
            if (!ParseStringVector(&script->attack_info_,
                                   "[/attack info]")) {
                return 0;
            }
        } else if (Eq(type, "[etc attack info]")) {
            if (!ParseStringVector(&script->etc_attack_info_,
                                   "[/etc attack info]")) {
                return 0;
            }
        } else if (Eq(type, "[weapon wav]")) {
            if (!ParseWeaponWav(&script->weapon_infos_,
                                "[/weapon wav]")) {
                return 0;
            }
        } else if (Eq(type, "[weapon hit info]")) {
            std::string token;
            bool is_type = false;
            std::vector<STCharacterScriptWeaponInfo>* weapons =
                &script->weapon_infos_;
            for (unsigned int index = 0;; ++index) {
                if (ScanTypeOrStr(is_type, token, true) != 1) {
                    return 0;
                }
                if (is_type) {
                    if (token != "[/weapon hit info]") {
                        return 0;
                    }
                    break;
                }
                EnsureWeaponInfo(weapons, index);
                STCharacterScriptWeaponInfo& weapon = (*weapons)[index];
                if (token == "[blow]") {
                    weapon.hit_type_ = 0;
                } else if (token == "[cut]") {
                    weapon.hit_type_ = 1;
                } else if (token == "[burn]") {
                    weapon.hit_type_ = 2;
                } else if (token == "[etc]") {
                    weapon.hit_type_ = 3;
                } else {
                    return 0;
                }
                if (!ScanRequiredStr(&text)) {
                    return 0;
                }
                if (text == "[no blood]") {
                    weapon.blood_effect_ = 0;
                } else if (text == "[blood]") {
                    weapon.blood_effect_ = 1;
                } else {
                    return 0;
                }
                if (!ScanRequiredInt(&weapon.hit_count_) ||
                    !ScanRequiredFloat(&weapon.hit_value_18_) ||
                    !ScanRequiredFloat(&weapon.hit_value_1c_) ||
                    !ScanRequiredFloat(&weapon.hit_value_20_)) {
                    return 0;
                }
            }
        } else if (Eq(type, "[weapon skill info]")) {
            std::string token;
            bool is_type = false;
            std::vector<STCharacterScriptWeaponInfo>* weapons =
                &script->weapon_infos_;
            for (unsigned int index = 0;; ++index) {
                const long double first =
                    ScanTypeOrFloat(is_type, token, success, true);
                if (!success) {
                    return 0;
                }
                if (is_type) {
                    if (token != "[/weapon skill info]") {
                        return 0;
                    }
                    break;
                }
                EnsureWeaponInfo(weapons, index);
                STCharacterScriptWeaponInfo& weapon = (*weapons)[index];
                weapon.skill_values_[0] = static_cast<float>(first);
                if (!ScanRequiredFloat(&weapon.skill_values_[1]) ||
                    !ScanRequiredFloat(&weapon.skill_values_[2]) ||
                    !ScanRequiredFloat(&weapon.skill_values_[3])) {
                    return 0;
                }
            }
        } else if (Eq(type, "[body image path]")) {
            if (!ScanRequiredStr(&text)) {
                return 0;
            }
            ::s_bodyImagePath[
                static_cast<ENUM_CHARACTERJOB>(script->character_job_)] =
                text;
        } else if (Eq(type, "[upgrade weapon attack power rate]")) {
            std::string token;
            bool is_type = false;
            for (unsigned int index = 0;; ++index) {
                const long double first =
                    ScanTypeOrFloat(is_type, token, success, true);
                if (!success) {
                    return 0;
                }
                if (is_type) {
                    if (token != "[/upgrade weapon attack power rate]") {
                        return 0;
                    }
                    break;
                }
                float second = 0.0f;
                if (!ScanRequiredFloat(&second)) {
                    return 0;
                }
                if (index <= 5) {
                    const int job = script->character_job_;
                    ::upgradeWeaponAttackPowerRate_[12 * job + 2 * index] =
                        static_cast<float>(first);
                    ::upgradeWeaponAttackPowerRate_[12 * job + 2 * index + 1] =
                        second;
                }
            }
        } else if (Eq(type, "[weapon durability decrease rate]")) {
            for (int index = 0; index <= 5; ++index) {
                if (!ScanRequiredFloat(&scanned_float)) {
                    return 0;
                }
                const int job = script->character_job_;
                ::g_weaponDurabilityDecreaseRate_[6 * job + index] =
                    scanned_float;
            }
        } else if (Eq(type, "[down up frame]") ||
                   Eq(type, "[down down frame]") ||
                   Eq(type, "[down bounce up frame]") ||
                   Eq(type, "[down bounce down frame]") ||
                   Eq(type, "[down lie frame]") ||
                   Eq(type, "[jump up start frame]") ||
                   Eq(type, "[jump down start frame]") ||
                   Eq(type, "[jump land start frame]") ||
                   Eq(type, "[attack x velocity]") ||
                   Eq(type, "[attack x accel]") ||
                   Eq(type, "[attack x velocity fast]") ||
                   Eq(type, "[attack x accel fast]") ||
                   Eq(type, "[dash attack slide stop frame]")) {
            if (!ScanRequiredInt(&scanned_int)) {
                return 0;
            }
        } else if (Eq(type, "[jump attack sound]") ||
                   Eq(type, "[attack sound]") ||
                   Eq(type, "[damage sound]") ||
                   Eq(type, "[die sound]") ||
                   Eq(type, "[jump start sound]") ||
                   Eq(type, "[jump land sound]") ||
                   Eq(type, "[back step sound]") ||
                   Eq(type, "[dash attack sound]") ||
                   Eq(type, "[lack mp sound]") ||
                   Eq(type, "[cooltime sound]") ||
                   Eq(type, "[creature command sound]") ||
                   Eq(type, "[throw item sound]")) {
            if (!ScanRequiredStr(&text)) {
                return 0;
            }
        } else if (Eq(type, "[attack cancel start frame]")) {
            if (!SkipCharacterClientBlock("[/attack cancel start frame]")) {
                return 0;
            }
        } else if (Eq(type, "[change equipment ani]")) {
            if (!SkipCharacterClientBlock("[/change equipment ani]")) {
                return 0;
            }
        } else if (Eq(type, "[create equipment list]")) {
            if (has_create_equipment ||
                !ParseCreateEquipmentList(&create_equipment)) {
                return 0;
            }
            has_create_equipment = true;
        } else {
            for (int index = 0; index <= 16; ++index) {
                if (Eq(type, kActiveStatusTags[index])) {
                    if (!ScanStatusFloat(status->seventeen_values[index])) {
                        return 0;
                    }
                    break;
                }
            }
        }
    }
    if (!has_create_equipment ||
        !PublishImportedCreateEquipment(script->character_job_,
                                        create_equipment)) {
        return 0;
    }
    return 1;
}

int exportCharacterStatusScript(FILE* stream, STRDCharacterStatus* status);
size_t exportCharacterGiveSkillScript(
    FILE* stream,
    std::vector<std::pair<int, int> >* skills);
void exportCharacterEtcStatsScript(FILE* stream, CharacterEtcStats* stats);
int exportCharacterScript(STCharacterScript* script, const char* src);

// IDA_EA: 0x8A0FB38
// IDA_NAME: _Z27exportCharacterStatusScriptP8_IO_FILEP19STRDCharacterStatus
// DEMANGLED: exportCharacterStatusScript(_IO_FILE *,STRDCharacterStatus *)
int exportCharacterStatusScript(FILE* stream, STRDCharacterStatus* status) {
    int result = 0;
    if (status->raw_00) {
        result = std::fprintf(stream, "[HP MAX] %.1f\n",
                              static_cast<double>(status->raw_00) / 10.0);
    }
    if (status->raw_04) {
        result = std::fprintf(stream, "[MP MAX] %.1f\n",
                              static_cast<double>(status->raw_04) / 10.0);
    }
    if (status->raw_08) {
        result = std::fprintf(stream, "[physical attack] %.1f\n",
                              static_cast<double>(status->raw_08) / 10.0);
    }
    if (status->raw_0a) {
        result = std::fprintf(stream, "[physical defense] %.1f\n",
                              static_cast<double>(status->raw_0a) / 10.0);
    }
    if (status->raw_0c) {
        result = std::fprintf(stream, "[magical attack] %.1f\n",
                              static_cast<double>(status->raw_0c) / 10.0);
    }
    if (status->raw_0e) {
        result = std::fprintf(stream, "[magical defense] %.1f\n",
                              static_cast<double>(status->raw_0e) / 10.0);
    }

    const char* const elemental_tags[4] = {
        "[fire resistance]",
        "[water resistance]",
        "[dark resistance]",
        "[light resistance]",
    };
    for (int index = 0; index <= 3; ++index) {
        if (status->four_values[index]) {
            result = std::fprintf(
                stream,
                "%s %.1f\n",
                elemental_tags[index],
                static_cast<double>(status->four_values[index]) / 10.0);
        }
    }
    for (int index = 0; index <= 16; ++index) {
        if (status->seventeen_values[index]) {
            result = std::fprintf(
                stream,
                "%s %.1f\n",
                kActiveStatusTags[index],
                static_cast<double>(status->seventeen_values[index]) / 10.0);
        }
    }
    if (status->raw_3a) {
        result = std::fprintf(stream, "[inventory limit] %.1f\n",
                              static_cast<double>(
                                  static_cast<int32_t>(status->raw_3a)) /
                                  10.0);
    }
    if (status->raw_3e) {
        result = std::fprintf(stream, "[HP regen speed] %.1f\n",
                              static_cast<double>(status->raw_3e) / 10.0);
    }
    if (status->raw_40) {
        result = std::fprintf(stream, "[MP regen speed] %.1f\n",
                              static_cast<double>(status->raw_40) / 10.0);
    }
    if (status->raw_42) {
        result = std::fprintf(stream, "[move speed] %.1f\n",
                              static_cast<double>(status->raw_42) / 10.0);
    }
    if (status->raw_46) {
        result = std::fprintf(stream, "[attack speed] %.1f\n",
                              static_cast<double>(status->raw_46) / 10.0);
    }
    if (status->raw_48) {
        result = std::fprintf(stream, "[cast speed] %.1f\n",
                              static_cast<double>(status->raw_48) / 10.0);
    }
    if (status->raw_4a) {
        result = std::fprintf(stream, "[hit recovery] %.1f\n",
                              static_cast<double>(status->raw_4a) / 10.0);
    }
    if (status->raw_4c) {
        result = std::fprintf(stream, "[jump power] %.1f\n",
                              static_cast<double>(status->raw_4c) / 10.0);
    }
    if (status->raw_4e) {
        result = std::fprintf(stream, "[weight] %.1f\n",
                              static_cast<double>(
                                  static_cast<int32_t>(status->raw_4e)) /
                                  10.0);
    }
    return result;
}

// IDA_EA: 0x8A0FFF6
// IDA_NAME: _Z30exportCharacterGiveSkillScriptP8_IO_FILEPSt6vectorISt4pairIiiESaIS3_EE
// DEMANGLED: exportCharacterGiveSkillScript(_IO_FILE *,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> *)
size_t exportCharacterGiveSkillScript(
    FILE* stream,
    std::vector<std::pair<int, int> >* skills) {
    if (skills->empty()) {
        return 0;
    }
    std::fwrite("[skill]\n", 1, 8, stream);
    for (std::vector<std::pair<int, int> >::const_iterator it =
             skills->begin();
         it != skills->end();
         ++it) {
        std::fprintf(stream, "%d %d\n", it->first, it->second);
    }
    return std::fwrite("[/skill]\n", 1, 9, stream);
}

// IDA_EA: 0x8A100F3
// IDA_NAME: _Z29exportCharacterEtcStatsScriptP8_IO_FILEP17CharacterEtcStats
// DEMANGLED: exportCharacterEtcStatsScript(_IO_FILE *,CharacterEtcStats *)
void exportCharacterEtcStatsScript(FILE* stream, CharacterEtcStats* stats) {
    if (!stats->values.empty()) {
        std::fwrite("[category]\n", 1, 0x0b, stream);
        for (std::vector<int>::const_iterator it = stats->values.begin();
             it != stats->values.end();
             ++it) {
            const std::string category = getObjectCategoryString(*it);
            std::fprintf(stream, "`%s`\n", category.c_str());
        }
        std::fwrite("[/category]\n", 1, 0x0c, stream);
    }

    int index = 0;
    for (; index <= 3; ++index) {
        float value = 0.0f;
        std::memcpy(&value, &stats->raw_0c[index], sizeof(value));
        if (value != 0.0f) {
            break;
        }
    }
    if (index != 4) {
        std::fwrite("[module damage rate]", 1, 0x14, stream);
        for (int n3 = 0; n3 <= 3; ++n3) {
            float value = 0.0f;
            std::memcpy(&value, &stats->raw_0c[n3], sizeof(value));
            std::fprintf(stream, " %.3f", value);
        }
        std::fputc('\n', stream);
    }
}

// IDA_EA: 0x8A102A4
// IDA_NAME: _Z21exportCharacterScriptP17STCharacterScriptPKc
// DEMANGLED: exportCharacterScript(STCharacterScript *,char const*)
int exportCharacterScript(STCharacterScript* script, const char* src) {
    static char saveFileName[0x100];
    getScriptFileFullPath(saveFileName,
                          sizeof(saveFileName),
                          g_szCharacterScriptBaseDirectory,
                          src);
    FILE* stream = std::fopen(saveFileName, "wb");
    if (!stream) {
        return 0;
    }
    std::fprintf(stream,
                 "[job] `%s`\n",
                 getCharacterJobName(
                     static_cast<ENUM_CHARACTERJOB>(script->character_job_)));
    std::fwrite("[growtype name]", 1, 0x0f, stream);
    for (int index = 0; index <= 5; ++index) {
        std::fprintf(stream,
                     " `%s`",
                     script->growtype_names_[index].c_str());
    }
    std::fputc('\n', stream);
    std::fprintf(stream,
                 "[width] %d %d\n",
                 script->width_,
                 script->height_);
    std::fputc('\n', stream);
    std::fprintf(stream, "[jump speed] %d\n", script->jump_speed_);
    std::fputc('\n', stream);
    if (script->start_town_ > 0) {
        std::fprintf(stream, "[start town] %d\n\n", script->start_town_);
    }
    std::fwrite("[initial value]\n", 1, 0x10, stream);
    exportCharacterStatusScript(stream, &script->growtype_status_[0]);
    exportCharacterGiveSkillScript(
        stream,
        &script->give_skills_[0]);
    exportCharacterEtcStatsScript(stream, &script->etc_stats_[0]);

    for (int grow = 1; grow <= 6; ++grow) {
        std::fprintf(stream, "[growtype %d]\n", grow);
        exportCharacterStatusScript(stream, &script->growtype_status_[grow]);
        exportCharacterGiveSkillScript(
            stream,
            &script->give_skills_[grow]);
        exportCharacterEtcStatsScript(stream, &script->etc_stats_[grow]);
    }
    std::fputc('\n', stream);

    const struct {
        const std::string* value;
        const char* tag;
    } string_fields[] = {
        {&script->waiting_motion_, "[waiting motion]"},
        {&script->move_motion_, "[move motion]"},
        {&script->sit_motion_, "[sit motion]"},
        {&script->damage_motion_1_, "[damage motion 1]"},
        {&script->damage_motion_2_, "[damage motion 2]"},
        {&script->down_motion_, "[down motion]"},
        {&script->overturn_motion_, "[overturn motion]"},
        {&script->jump_motion_, "[jump motion]"},
        {&script->jumpattack_motion_, "[jumpattack motion]"},
        {&script->rest_motion_, "[rest motion]"},
        {&script->throw_motion_1_1_, "[throw motion 1-1]"},
        {&script->throw_motion_1_2_, "[throw motion 1-2]"},
        {&script->throw_motion_2_1_, "[throw motion 2-1]"},
        {&script->throw_motion_2_2_, "[throw motion 2-2]"},
        {&script->throw_motion_3_1_, "[throw motion 3-1]"},
        {&script->throw_motion_3_2_, "[throw motion 3-2]"},
        {&script->throw_motion_4_1_, "[throw motion 4-1]"},
        {&script->throw_motion_4_2_, "[throw motion 4-2]"},
        {&script->dash_motion_, "[dash motion]"},
        {&script->dashattack_motion_, "[dashattack motion]"},
        {&script->getitem_motion_, "[getitem motion]"},
        {&script->buff_motion_, "[buff motion]"},
    };
    for (unsigned int index = 0;
         index < sizeof(string_fields) / sizeof(string_fields[0]);
         ++index) {
        ExportNonEmptyString(stream,
                             string_fields[index].tag,
                             string_fields[index].value);
    }
    ExportStringVector(stream,
                       "[attack motion]",
                       "[/attack motion]",
                       &script->attack_motion_);
    ExportStringVector(stream,
                       "[etc motion]",
                       "[/etc motion]",
                       &script->etc_motion_);

    std::fputc('\n', stream);
    ExportNonEmptyString(stream,
                         "[jumpattack info]",
                         &script->jumpattack_info_);
    ExportNonEmptyString(stream,
                         "[dashattack info]",
                         &script->dashattack_info_);
    ExportStringVector(stream,
                       "[attack info]",
                       "[/attack info]",
                       &script->attack_info_);
    ExportStringVector(stream,
                       "[etc attack info]",
                       "[/etc attack info]",
                       &script->etc_attack_info_);

    std::fputc('\n', stream);
    std::vector<STCharacterScriptWeaponInfo>* weapons =
        &script->weapon_infos_;
    for (std::vector<STCharacterScriptWeaponInfo>::const_iterator it = weapons->begin();
         it != weapons->end();
         ++it) {
        std::fwrite("[weapon wav]\n", 1, 0x0d, stream);
        for (std::vector<std::string>::const_iterator wav =
                 it->attack_files_.begin();
             wav != it->attack_files_.end();
             ++wav) {
            std::fprintf(stream, "`%s`\n", wav->c_str());
        }
        std::fwrite("[/weapon wav]\n", 1, 0x0e, stream);
    }

    std::fputc('\n', stream);
    std::fwrite("[weapon hit info]\n", 1, 0x12, stream);
    for (std::vector<STCharacterScriptWeaponInfo>::const_iterator it =
             weapons->begin();
         it != weapons->end();
         ++it) {
        switch (it->hit_type_) {
            case 1:
                std::fwrite("`[cut]` ", 1, 8, stream);
                break;
            case 2:
                std::fwrite("`[burn]` ", 1, 9, stream);
                break;
            case 3:
                std::fwrite("`[etc]` ", 1, 8, stream);
                break;
            default:
                std::fwrite("`[blow]` ", 1, 9, stream);
                break;
        }
        if (it->blood_effect_) {
            std::fwrite("`[blood]` ", 1, 0x0a, stream);
        } else {
            std::fwrite("`[no blood]` ", 1, 0x0d, stream);
        }
        std::fprintf(stream,
                     "%d %.2f\n",
                     it->hit_count_,
                     it->hit_value_18_);
    }
    std::fwrite("[/weapon hit info]\n", 1, 0x13, stream);

    std::fwrite("[weapon skill info]\n", 1, 0x14, stream);
    for (std::vector<STCharacterScriptWeaponInfo>::const_iterator it =
             weapons->begin();
         it != weapons->end();
         ++it) {
        std::fprintf(stream,
                     "%.2f %.2f %.2f %.2f\n",
                     it->skill_values_[0],
                     it->skill_values_[1],
                     it->skill_values_[2],
                     it->skill_values_[3]);
    }
    std::fwrite("[/weapon skill info]\n", 1, 0x15, stream);
    std::fclose(stream);
    convertBinaryToText(saveFileName);
    return 1;
}
