#include "old_game/economy/equipment/equipment_part_set_public_abi.h"

#include "common/config.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/progression/quest/quest_skill_value_types.h"
#include "old_data/PVF/equipment/datamanager/equipment_script.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/datamanager/module_tag_table.h"
#include "old_data/PVF/itemshop/quest_shop_status.h"
#include "old_data/PVF/data/datamanager/script_file_list.h"
#include "old_data/PVF/common/script_value.h"
#include "common/manager/change_status_data.h"
#include "old_game/economy/equipment/enchant_system_data.h"
#include "old_game/economy/equipment/equipment_ani_script.h"
#include "common/manager/global_data.h"
#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "common/manager/global_tools.h"
#include "common/manager/striker_public_abi.h"
#include "infra/log/format/trace.h"

#include <map>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <new>
#include <set>
#include <string>
#include <utility>
#include <vector>

int g_EquipmentParameterInfoCount __asm__("g_EquipmentParameterInfoCount");
int g_EquipmentParameterInfoSize __asm__("g_EquipmentParameterInfoSize");

int EquipmentScriptSpecialMonsterDrop(STEquipmentScript* script,
                                      std::string tag,
                                      const char* filename);
int ImportEquipmentScript(STEquipmentScript* script,
                          const char* filename,
                          bool unknown,
                          int module_type,
                          int set_id);
int GetEquipmentType(const char* text);
int getAvatarEmblemType(const char* text);
int importEquipmentParameterInfo(EquipmentParameterInfo* info,
                                 std::string& tag,
                                 const char* filename,
                                 int& first,
                                 int& second);
void scanIf(EquipmentParameterInfo* info, int unused);
void readCondtion(EquipmentParameterInfo* info,
                  std::string& tag,
                  std::vector<int>& values);
int scanThen(std::string filename, EquipmentParameterInfo* info, int unused);
int readAction(const std::string& filename,
               EquipmentParameterInfo* info,
               std::string& tag,
               std::vector<float>& values,
               int unused);
int scanRandomOptionValue(EquipmentParameterInfo* info, std::string& tag);
static __attribute__((noinline, used)) int GetExpertJobTypeList(
    STItemScript* script);
static __attribute__((noinline, used)) bool
GetExpertJobValueMaterialVariation(STItemScript* script);
static __attribute__((noinline, used)) bool GetExpertJobValueCompoundRate(
    STItemScript* script);
static __attribute__((noinline, used)) bool GetExpertJobResultVariation(
    STItemScript* script);
static __attribute__((noinline, used)) bool GetExpertJobDisjointBigWinRate(
    STItemScript* script);
static __attribute__((noinline, used)) bool
GetExpertJobDisjointResultVariation(STItemScript* script);
static __attribute__((noinline, used)) bool GetExpertJobAdditionalGainExp(
    STItemScript* script);
static __attribute__((noinline, used)) int GetItemCategory(
    STItemScript* script);
static __attribute__((noinline, used)) bool GetAuraAvatarOption(
    STEquipmentScript* script);

namespace {

typedef std::map<const EquipmentParameterInfo*,
                 equipment_parameter_0828::Extension>
    EquipmentParameterExtensionTable;

EquipmentParameterExtensionTable& EquipmentParameterExtensions() {
    // Equipment prototypes can outlive ordinary function-static teardown.
    // Keep this pointer-backed table alive until process exit so destructor
    // cleanup never observes a destroyed sidecar owner.
    static EquipmentParameterExtensionTable* table =
        new EquipmentParameterExtensionTable;
    return *table;
}

typedef std::set<const STItemScript*> EventRewardItemTable;

EventRewardItemTable& EventRewardItems() {
    static EventRewardItemTable* table = new EventRewardItemTable;
    return *table;
}

typedef std::set<int32_t> EventRewardPrototypeTable;

EventRewardPrototypeTable& EventRewardPrototypes() {
    // Item prototypes are published during the single-threaded PVF load and
    // remain immutable for the lifetime of the process.
    static EventRewardPrototypeTable* table =
        new EventRewardPrototypeTable;
    return *table;
}

}  // namespace

namespace equipment_parameter_0828 {

SkillCooltimeSetItemRate::SkillCooltimeSetItemRate()
    : lower_bound_level(0), upper_bound_level(0), cooltime_rate(0.0f) {}

Extension::Extension()
    : has_skill_cooltime_set_item_rate(false),
      not_accumulate(false),
      skill_cooltime_set_item_rate() {}

bool Find(const EquipmentParameterInfo* parameter, Extension* extension) {
    if (extension) *extension = Extension();
    if (!parameter) return false;
    const EquipmentParameterExtensionTable::const_iterator found =
        EquipmentParameterExtensions().find(parameter);
    if (found == EquipmentParameterExtensions().end()) return false;
    if (extension) *extension = found->second;
    return true;
}

void Clear(const EquipmentParameterInfo* parameter) {
    if (parameter) EquipmentParameterExtensions().erase(parameter);
}

void Copy(const EquipmentParameterInfo* destination,
          const EquipmentParameterInfo* source) {
    if (!destination) return;
    if (destination == source) return;
    EquipmentParameterExtensions().erase(destination);
    if (!source) return;
    const EquipmentParameterExtensionTable::const_iterator found =
        EquipmentParameterExtensions().find(source);
    if (found != EquipmentParameterExtensions().end()) {
        EquipmentParameterExtensions().insert(
            std::make_pair(destination, found->second));
    }
}

void SetSkillCooltimeSetItemRate(
    const EquipmentParameterInfo* parameter,
    const SkillCooltimeSetItemRate& rule) {
    if (!parameter) return;
    Extension& extension = EquipmentParameterExtensions()[parameter];
    extension.has_skill_cooltime_set_item_rate = true;
    extension.skill_cooltime_set_item_rate = rule;
}

void SetNotAccumulate(const EquipmentParameterInfo* parameter, bool enabled) {
    if (!parameter) return;
    Extension& extension = EquipmentParameterExtensions()[parameter];
    extension.not_accumulate = enabled;
}

}  // namespace equipment_parameter_0828

namespace item_script_0828 {

void SetEventRewardItem(const STItemScript* item, bool enabled) {
    if (!item) return;
    if (enabled) {
        EventRewardItems().insert(item);
    } else {
        EventRewardItems().erase(item);
    }
}

bool IsEventRewardItem(const STItemScript* item) {
    return item && EventRewardItems().find(item) != EventRewardItems().end();
}

void RegisterEventRewardPrototype(int32_t item_index,
                                  const STItemScript* item) {
    if (item_index < 0) return;
    if (IsEventRewardItem(item)) {
        EventRewardPrototypes().insert(item_index);
    } else {
        EventRewardPrototypes().erase(item_index);
    }
}

bool IsEventRewardPrototype(int32_t item_index) {
    return item_index >= 0 &&
           EventRewardPrototypes().find(item_index) !=
               EventRewardPrototypes().end();
}

void Copy(const STItemScript* destination, const STItemScript* source) {
    if (destination == source) return;
    SetEventRewardItem(destination, IsEventRewardItem(source));
}

}  // namespace item_script_0828

namespace {

struct ArtifactInfoTagLess {
    bool operator()(const char* lhs, const char* rhs) const {
        return std::strcmp(lhs, rhs) < 0;
    }
};

int getEquipmentActiveStatusType(std::string& tag) {
    return getChangeStatusType(tag);
}

void scanEquipmentActiveStatusTail(std::string& tag,
                                   std::vector<int>& values) {
    if (tag == "[blind]" || tag == "[slow]" || tag == "[haste]") {
        values.push_back(ScanInt(static_cast<bool*>(0)));
        values.push_back(ScanInt(static_cast<bool*>(0)));
    } else if (tag == "[stone]" || tag == "[confuse]" ||
               tag == "[armor break]" || tag == "[weapon break]" ||
               tag == "[poison]" || tag == "[lightning]" ||
               tag == "[bleeding]") {
        values.push_back(ScanInt(static_cast<bool*>(0)));
    } else if (tag == "[burn]" || tag == "[bless]" || tag == "[curse]") {
        for (int i = 0; i <= 3; ++i) {
            values.push_back(ScanInt(static_cast<bool*>(0)));
        }
    }
}

int getAvatarAbilityVariationIndex(const std::string& tag) {
    static const char* const kTags[] = {
        "[none]", "[HP MAX]", "[MP MAX]", "[PHYSICAL_ATTACK]",
        "[PHYSICAL_DEFENSE]", "[MAGICAL_ATTACK]", "[MAGICAL_DEFENSE]",
        "[EQUIPMENT_PHYSICAL_ATTACK]", "[EQUIPMENT_PHYSICAL_DEFENSE]",
        "[EQUIPMENT_MAGICAL_ATTACK]", "[EQUIPMENT_MAGICAL_DEFENSE]",
        "[ELEMENT_TOLERANCE_FIRE]", "[ELEMENT_TOLERANCE_WATER]",
        "[ELEMENT_TOLERANCE_DARK]", "[ELEMENT_TOLERANCE_LIGHT]",
        "[ELEMENT_TOLERANCE_ALL]", "[ACTIVESTATUS_TOLERANCE_SLOW]",
        "[ACTIVESTATUS_TOLERANCE_FREEZE]",
        "[ACTIVESTATUS_TOLERANCE_POISON]",
        "[ACTIVESTATUS_TOLERANCE_STUN]",
        "[ACTIVESTATUS_TOLERANCE_CURSE]",
        "[ACTIVESTATUS_TOLERANCE_BLIND]",
        "[ACTIVESTATUS_TOLERANCE_LIGHTNING]",
        "[ACTIVESTATUS_TOLERANCE_STONE]",
        "[ACTIVESTATUS_TOLERANCE_SLEEP]",
        "[ACTIVESTATUS_TOLERANCE_BURN]",
        "[ACTIVESTATUS_TOLERANCE_WEAPON_BREAK]",
        "[ACTIVESTATUS_TOLERANCE_BLEEDING]",
        "[ACTIVESTATUS_TOLERANCE_PIERCE]",
        "[ACTIVESTATUS_TOLERANCE_STUCK]",
        "[ACTIVESTATUS_TOLERANCE_CONFUSE]",
        "[ACTIVESTATUS_TOLERANCE_HOLD]",
        "[ACTIVESTATUS_TOLERANCE_ALL]", "[INVENTORY_MAX_WEIGHT]",
        "[HP_REGENRATE]", "[MP_REGENRATE]", "[MP_REGENRATE_PERCENT_UP]",
        "[MOVE_SPEED]", "[ATTACK_SPEED]", "[CAST_SPEED]", "[HIT_RECOVERY]",
        "[JUMP_POWER]", "[CRITICAL]", "[SKILL_LEVEL]",
        "[PHYSICAL ABSOLUTE DEFENSE]", "[MAGICAL ABSOLUTE DEFENSE]",
        "[STUCK ON ATTACK]"};
    for (int i = 0; i <= 46; ++i) {
        if (tag == kTags[i]) {
            return i;
        }
    }
    return -1;
}

}  // namespace

// IDA_EA: 0x89BD079
// IDA_NAME: _Z18importArtifactInfoRN17STEquipmentScript13artifact_infoESs
// DEMANGLED: importArtifactInfo(STEquipmentScript::artifact_info &,std::string)
int importArtifactInfo(STEquipmentScript::artifact_info& info,
                       std::string tag) {
    enum ArtifactInfoTag {
        kCreatureMinimumLevel = 12009,
        kCreaturePhysicalAttack = 12012,
        kCreatureMagicalAttack = 12015,
        kCreatureSkillConsumeMpRate = 12018,
        kCreatureSkillChargeTimeRate = 12021,
        kCreatureSkillOverChargeTimeRate = 12024,
        kCreatureExperienceAmountRate = 12027,
        kCreaturePhysicalCriticalHit = 12030,
        kCreatureMagicalCriticalHit = 12033,
        kCreatureStuck = 12036,
        kCreaturePiercing = 12039,
    };

    static std::map<const char*, int, ArtifactInfoTagLess> lookup;
    static std::mutex init_lock;
    static bool inited = false;

    if (!inited) {
        std::lock_guard<std::mutex> lock(init_lock);
        if (!inited) {
            lookup.insert(std::make_pair("[creature minimum level]",
                                         kCreatureMinimumLevel));
            lookup.insert(std::make_pair("[creature physical attack]",
                                         kCreaturePhysicalAttack));
            lookup.insert(std::make_pair("[creature magical attack]",
                                         kCreatureMagicalAttack));
            lookup.insert(std::make_pair("[creature skill consume mp rate]",
                                         kCreatureSkillConsumeMpRate));
            lookup.insert(std::make_pair("[creature skill charge time rate]",
                                         kCreatureSkillChargeTimeRate));
            lookup.insert(
                std::make_pair("[creature skill over charge time rate]",
                               kCreatureSkillOverChargeTimeRate));
            lookup.insert(std::make_pair("[creature experience amount rate]",
                                         kCreatureExperienceAmountRate));
            lookup.insert(std::make_pair("[creature physical critical hit]",
                                         kCreaturePhysicalCriticalHit));
            lookup.insert(std::make_pair("[creature magical critical hit]",
                                         kCreatureMagicalCriticalHit));
            lookup.insert(std::make_pair("[creature stuck]", kCreatureStuck));
            lookup.insert(
                std::make_pair("[creature piercing]", kCreaturePiercing));
            inited = true;
        }
    }

    const std::map<const char*, int, ArtifactInfoTagLess>::const_iterator it =
        lookup.find(tag.c_str());
    if (it == lookup.end()) {
        return 0;
    }

    switch (it->second) {
        case kCreatureMinimumLevel:
            info.usable_type = ScanInt(static_cast<bool*>(0));
            return 1;
        case kCreaturePhysicalAttack:
            info.grade = ScanInt(static_cast<bool*>(0));
            return 1;
        case kCreatureMagicalAttack:
            info.rarity = ScanInt(static_cast<bool*>(0));
            return 1;
        case kCreatureSkillConsumeMpRate:
            info.physical_attack = ScanFloat(2, 0);
            return 1;
        case kCreatureSkillChargeTimeRate:
            info.magical_attack = ScanFloat(2, 0);
            return 1;
        case kCreatureSkillOverChargeTimeRate:
            info.physical_defense = ScanFloat(2, 0);
            return 1;
        case kCreatureExperienceAmountRate:
            info.magical_defense = ScanFloat(2, 0);
            return 1;
        case kCreaturePhysicalCriticalHit:
            info.strength = ScanFloat(1, 0);
            return 1;
        case kCreatureMagicalCriticalHit:
            info.vitality = ScanFloat(1, 0);
            return 1;
        case kCreatureStuck:
            info.spirit = ScanFloat(1, 0);
            return 1;
        case kCreaturePiercing:
            info.item_index = ScanInt(static_cast<bool*>(0));
            return 1;
    }
    return 0;
}

extern char g_szEquipmentScriptBaseDirectory[256];

bool loadRDARScriptFile(const char* dir, const char* filename);

// IDA_EA: 0x89C2D82
// IDA_NAME: _ZN24STAvatarAbilityVariationC2Ev
// DEMANGLED: STAvatarAbilityVariation::STAvatarAbilityVariation(void)
STAvatarAbilityVariation::STAvatarAbilityVariation() {
    // The member C2 runs before this body, exactly as old C2 constructs
    // LevelUpSkill at +0x0c before entering clear().
    clear();
}

// IDA_EA: 0x89C2DA4
// IDA_NAME: _ZN24STAvatarAbilityVariation5clearEv
// DEMANGLED: STAvatarAbilityVariation::clear(void)
LevelUpSkill* STAvatarAbilityVariation::clear() {
    ability_index = 0;
    sign_type = 0;
    value = 0.0f;
    return level_up_skill.clear();
}

// IDA_EA: 0x89D2AF0
// IDA_NAME: _ZN24STAvatarAbilityVariationD2Ev
// DEMANGLED: STAvatarAbilityVariation::~STAvatarAbilityVariation()
STAvatarAbilityVariation::~STAvatarAbilityVariation() {
    // The compiler emits the member LevelUpSkill D2 after this empty D2 body.
}

// IDA_EA: 0x8513A10
// IDA_NAME: _ZN20STSpecialMonsterDropC2Ev
// DEMANGLED: STSpecialMonsterDrop::STSpecialMonsterDrop(void)
STSpecialMonsterDrop::STSpecialMonsterDrop() {
    clear();
}

// IDA_EA: 0x8513A24
// IDA_NAME: _ZN20STSpecialMonsterDrop5clearEv
// DEMANGLED: STSpecialMonsterDrop::clear(void)
void* STSpecialMonsterDrop::clear() {
    enabled = 0;
    extra_drop_easy = 0;
    extra_drop_medium = 0;
    extra_drop_hard = 0;
    extra_drop_ultimate = 0;
    extra_drop_hero = 0;
    dungeon_normal = 0;
    dungeon_ancient = 0;
    dungeon_dimension = 0;
    return this;
}

// IDA_EA: 0x8513AC6
// IDA_NAME: _ZN7DyeInfoC2Ev
// DEMANGLED: DyeInfo::DyeInfo(void)
DyeInfo::DyeInfo() {
    clear();
}

// IDA_EA: 0x8513B0E
// IDA_NAME: _ZN7DyeInfo5clearEv
// DEMANGLED: DyeInfo::clear(void)
int DyeInfo::clear() {
    type = 1;
    colors.clear();
    return 0;
}

// IDA_EA: 0x851517A
// IDA_NAME: _ZN7DyeInfoaSERKS_
// DEMANGLED: DyeInfo::operator=(DyeInfo const&)
DyeInfo& DyeInfo::operator=(const DyeInfo& rhs) {
    type = rhs.type;
    colors = rhs.colors;
    return *this;
}

// IDA_EA: 0x898D37C
// IDA_NAME: _ZN7DyeInfo16getDyeColorIndexEv
// DEMANGLED: DyeInfo::getDyeColorIndex(void)
int DyeInfo::getDyeColorIndex() {
    if (type != 1) {
        return -1;
    }
    if (colors.empty()) {
        return -1;
    }
    return colors[0].first;
}

// IDA_EA: 0x849F34C
// IDA_NAME: _ZN12STItemScript10SEXPERTJOB24stExpertJobAdditionalExpC2Ev
// DEMANGLED: STItemScript::SEXPERTJOB::stExpertJobAdditionalExp::stExpertJobAdditionalExp(void)
STItemScript::SEXPERTJOB::stExpertJobAdditionalExp::stExpertJobAdditionalExp() {
    value_00 = 0.0f;
    value_04 = 0;
}

// IDA_EA: 0x850D0E8
// IDA_NAME: _ZN12STItemScript10SEXPERTJOB31stExpertCompoundResultVariationC2Ev
// DEMANGLED: STItemScript::SEXPERTJOB::stExpertCompoundResultVariation::stExpertCompoundResultVariation(void)
STItemScript::SEXPERTJOB::stExpertCompoundResultVariation::stExpertCompoundResultVariation() {
    value_00 = 0.0f;
    value_04 = 0.0f;
}

// IDA_EA: 0x89C36AA
// IDA_NAME: _ZN12STItemScript10SEXPERTJOBC2Ev
// DEMANGLED: STItemScript::SEXPERTJOB::SEXPERTJOB(void)
STItemScript::SEXPERTJOB::SEXPERTJOB() {
    disjoint_big_win_rate = 0.0f;
    disjoint_result_variation = 0.0f;
    material_variation = 0.0f;
    compound_rate = 0.0f;
}

// IDA_EA: 0x898D3C6
// IDA_NAME: _ZN12STItemScriptC2Ev
// DEMANGLED: STItemScript::STItemScript(void)
STItemScript::STItemScript() {
    Clear();
}

// IDA_EA: 0x898D718
// IDA_NAME: _ZN12STItemScript5ClearEv
// DEMANGLED: STItemScript::Clear(void)
STItemScript* STItemScript::Clear() {
    item_script_0828::SetEventRewardItem(this, false);
    index = 0;
    grade = 1;
    for (int index = 0; index < 7; ++index) {
        values_08[index] = 0;
    }
    value_24 = -1;
    value_28 = -1;
    value_2c = 0;
    value_30 = 0;
    value_31 = 0;
    value_32 = 0;
    value_34 = 0;
    for (int index = 0; index < oldsrv::config::kCharacterJobCount; ++index) {
        usable_job_flags[index] = 0;
    }
    usable_jobs.clear();
    for (int value = 0; value < oldsrv::config::kCharacterJobCount; ++value) {
        usable_jobs.insert(value);
    }
    for (int index = 0; index < 4; ++index) {
        values_5c[index] = 0;
    }
    icon.clear();
    icon_index = 0;
    field_image.clear();
    field_image_index = 0;
    tarotcard_view.clear();
    tarotcard_view_index = 0;
    name.clear();
    int_data.clear();
    string_data.clear();
    value_a0 = 0;
    values_a4[0] = 0;
    values_a4[1] = -1;
    for (int index = 0; index < 6; ++index) {
        strings_ac[index].clear();
    }
    values_c4[0] = -1;
    values_c4[1] = 0;
    values_c4[2] = 0;
    value_d0 = 0;
    value_d1 = 0;
    value_d4 = 0;
    value_d8 = 0;
    value_d9 = 0;
    value_da = 0;
    usable_worlds.clear();
    usable_worlds.insert(0);
    values_f4[0] = 0;
    values_f4[1] = 0;
    value_fc = 1;
    value_100 = 0;
    value_104 = 1;
    value_10e = 0;
    value_10f = 0;
    special_drop_items.clear();
    item_categories.clear();
    usable_upgrade_character_level = 0;
    expert_job.result_variation.value_00 = 0.0f;
    expert_job.result_variation.value_04 = 0.0f;
    expert_job.additional_exp.value_00 = 0.0f;
    expert_job.additional_exp.value_04 = 0;
    expert_job.disjoint_big_win_rate = 0.0f;
    expert_job.disjoint_result_variation = 0.0f;
    expert_job.material_variation = 0.0f;
    expert_job.compound_rate = 0.0f;
    expert_job.usable.clear();
    dye_info.clear();
    value_174.clear();
    return this;
}

// IDA_EA: 0x898DB2A
// IDA_NAME: _ZNK12STItemScript11hasCategoryENS_18ENUM_ITEM_CATEGORYE
// DEMANGLED: STItemScript::hasCategory(STItemScript::ENUM_ITEM_CATEGORY)const
bool STItemScript::hasCategory(ENUM_ITEM_CATEGORY category) const {
    return item_categories.find(category) != item_categories.end();
}

// IDA_EA: 0x89C5046
// IDA_NAME: _ZN22EquipmentParameterInfo11SASPROPERTYC2Ev
// DEMANGLED: EquipmentParameterInfo::SASPROPERTY::SASPROPERTY(void)
EquipmentParameterInfo::SASPROPERTY::SASPROPERTY() {
}

// IDA_EA: 0x83CB038
// IDA_NAME: _ZN22EquipmentParameterInfo11SASPROPERTYC2ERKS0_
// DEMANGLED: EquipmentParameterInfo::SASPROPERTY::SASPROPERTY(EquipmentParameterInfo::SASPROPERTY const&)
EquipmentParameterInfo::SASPROPERTY::SASPROPERTY(const SASPROPERTY& rhs)
    : item_growtype(rhs.item_growtype),
      item_id(rhs.item_id),
      skill_group(rhs.skill_group),
      lower_bound_level(rhs.lower_bound_level),
      upper_bound_level(rhs.upper_bound_level),
      value(rhs.value),
      extra_condition(rhs.extra_condition),
      skill_phase(rhs.skill_phase),
      values(rhs.values),
      text(rhs.text) {}

// IDA_EA: 0x83DBDFC
// IDA_NAME: _ZN22EquipmentParameterInfo11SASPROPERTYaSERKS0_
// DEMANGLED: EquipmentParameterInfo::SASPROPERTY::operator=(EquipmentParameterInfo::SASPROPERTY const&)
EquipmentParameterInfo::SASPROPERTY&
EquipmentParameterInfo::SASPROPERTY::operator=(const SASPROPERTY& rhs) {
    item_growtype = rhs.item_growtype;
    item_id = rhs.item_id;
    skill_group = rhs.skill_group;
    lower_bound_level = rhs.lower_bound_level;
    upper_bound_level = rhs.upper_bound_level;
    value = rhs.value;
    extra_condition = rhs.extra_condition;
    skill_phase = rhs.skill_phase;
    values = rhs.values;
    text = rhs.text;
    return *this;
}

// IDA_EA: 0x891935E
// IDA_NAME: _ZN22EquipmentParameterInfo20RebirthAppendageinfoC2Ev
// DEMANGLED: EquipmentParameterInfo::RebirthAppendageinfo::RebirthAppendageinfo(void)
EquipmentParameterInfo::RebirthAppendageinfo::RebirthAppendageinfo() {
    clear();
}

// IDA_EA: 0x8919380
// IDA_NAME: _ZN22EquipmentParameterInfo20RebirthAppendageinfo5clearEv
// DEMANGLED: EquipmentParameterInfo::RebirthAppendageinfo::clear(void)
EquipmentParameterInfo::RebirthAppendageinfo*
EquipmentParameterInfo::RebirthAppendageinfo::clear() {
    probability = 0;
    consume_item = -1;
    consume_item_count = 0;
    hp_recovery = 0;
    mp_recovery = 0;
    cooltime = 0;
    return this;
}

// IDA_EA: 0x89193C0
// IDA_NAME: _ZN22EquipmentParameterInfo5clearEv
// DEMANGLED: EquipmentParameterInfo::clear(void)
EquipmentParameterInfo* EquipmentParameterInfo::clear() {
    EquipmentParameterInfo* self = this;
    equipment_parameter_0828::Clear(self);
    self->values_000[0].integer = 2;
    for (int index = 1; index <= 11; ++index) {
        self->values_000[index].integer = 0;
    }
    for (int index = 12; index <= 19; ++index) {
        self->values_000[index].integer = 0;
    }
    for (int index = 20; index <= 28; ++index) {
        self->values_000[index].integer = 0;
    }
    for (int index = 29; index <= 34; ++index) {
        self->values_000[index].integer = 0;
    }
    for (int index = 37; index <= 43; ++index) {
        self->values_000[index].integer = 0;
    }
    for (int index = 44; index <= 48; ++index) {
        self->values_000[index].integer = 0;
    }
    for (int index = 49; index <= 65; ++index) {
        self->values_000[index].integer = 0;
    }
    for (int index = 66; index <= 78; ++index) {
        self->values_000[index].integer = 0;
    }
    self->values_140[0].integer = 4;
    self->values_140[1].integer = 18;
    self->values_140[2].integer = 0;
    self->values_140[3].integer = 0;
    self->value_150 = 0;
    self->value_154.integer = 0;
    self->values_158.clear();
    self->value_164.integer = 0;
    self->value_168.integer = 18;
    self->value_16c.integer = 0;
    self->value_170.integer = 0;
    self->value_174 = 0;
    self->value_178.integer = 0;
    self->values_17c.clear();
    self->aura_infos_188.clear();
    self->values_194.clear();
    self->values_1a0[0].integer = 4;
    self->values_1a0[1].integer = 0;
    self->values_1a0[2].integer = 0;
    self->values_1a0[3].integer = 4;
    self->values_1a0[4].integer = 0;
    self->values_1a0[5].integer = 0;
    self->skill_data_up_1b8.clear();
    self->strings_1c4.clear();
    self->skill_data_up_1dc.clear();
    self->skill_data_up_1e8.clear();
    self->strings_1f4.clear();
    self->aura_appendages_200.clear();
    self->value_20c.integer = 0;
    self->value_210.integer = 0;
    self->hardcoding_parameters_214.clear();
    for (int index = 0; index < 3; ++index) {
        self->values_220[index].integer = 0;
    }
    self->value_22c = 0;
    self->value_230.integer = 0;
    self->value_234.integer = 0;
    self->value_238.integer = 0;
    self->value_23c = 0;
    self->value_23d = 0;
    self->recovery_statuses_240.clear();
    self->skill_levelups_24c.clear();
    self->skill_levelups_258.clear();
    self->avatar_select_abilities_264.clear();
    self->value_270.integer = -1;
    self->value_274.integer = 0;
    self->value_278.clear();
    self->value_27c.clear();
    for (int index = 0; index <= 5; ++index) {
        self->values_280[index].integer = 0;
    }
    self->values_280[6].integer = 0;
    for (int index = 7; index <= 61; ++index) {
        self->values_280[index].integer = 0;
    }
    for (int index = 61; index <= 65; ++index) {
        self->values_280[index].integer = 0;
    }
    self->values_280[66].integer = 0;
    self->values_280[67].integer = 0;
    self->values_280[68].integer = 0x3f800000;
    self->values_280[69].integer = 0x3f800000;
    self->values_280[70].integer = 0;
    self->values_280[71].integer = 0;
    for (int index = 72; index <= 84; ++index) {
        self->values_280[index].integer = 0;
    }
    self->values_280[88].integer = 0;
    for (int index = 90; index <= 97; index += 2) {
        self->values_280[index].integer = 0;
        self->values_280[index + 1].integer = 0x3f800000;
    }
    self->values_280[98].integer = 0;
    self->values_280[100].integer = 0;
    self->values_280[101].integer = 0;
    self->all_skill_items_418.clear();
    self->item_overpower_enabled_424 = 0;
    self->item_overpower_ready_425 = 0;
    self->item_overpower_stats_428.clear();
    self->values_434[0].integer = 0;
    self->values_434[1].integer = -1;
    self->values_434[2].integer = -1;
    self->value_440 = 0;
    self->values_444[0].integer = 0;
    self->values_444[1].integer = 0;
    self->values_444[2].integer = 0;
    self->values_000[35].integer = 0;
    self->values_000[36].integer = 0;
    self->rebirth_450.clear();
    self->conditional_values_468.clear();
    self->variable_stats_474.clear();
    return self;
}

// IDA_EA: 0x8919CE8
// IDA_NAME: _ZN22EquipmentParameterInfoC2Ev
// DEMANGLED: EquipmentParameterInfo::EquipmentParameterInfo(void)
EquipmentParameterInfo::EquipmentParameterInfo() {
    ++g_EquipmentParameterInfoCount;
    g_EquipmentParameterInfoSize = 1152 * g_EquipmentParameterInfoCount;
    clear();
}

// IDA_EA: 0x83662CA
// IDA_NAME: _ZN22EquipmentParameterInfoaSERKS_
// DEMANGLED: EquipmentParameterInfo::operator=(EquipmentParameterInfo const&)
EquipmentParameterInfo& EquipmentParameterInfo::operator=(
    const EquipmentParameterInfo& rhs_value) {
    for (int index = 0; index < 80; ++index) {
        values_000[index] = rhs_value.values_000[index];
    }
    for (int index = 0; index < 4; ++index) {
        values_140[index] = rhs_value.values_140[index];
    }
    value_150 = rhs_value.value_150;
    reserved_151 = rhs_value.reserved_151;
    reserved_152 = rhs_value.reserved_152;
    reserved_153 = rhs_value.reserved_153;
    value_154 = rhs_value.value_154;
    values_158 = rhs_value.values_158;
    value_164 = rhs_value.value_164;
    value_168 = rhs_value.value_168;
    value_16c = rhs_value.value_16c;
    value_170 = rhs_value.value_170;
    value_174 = rhs_value.value_174;
    reserved_175 = rhs_value.reserved_175;
    reserved_176 = rhs_value.reserved_176;
    reserved_177 = rhs_value.reserved_177;
    value_178 = rhs_value.value_178;
    values_17c = rhs_value.values_17c;
    aura_infos_188 = rhs_value.aura_infos_188;
    values_194 = rhs_value.values_194;
    for (int index = 0; index < 6; ++index) {
        values_1a0[index] = rhs_value.values_1a0[index];
    }
    skill_data_up_1b8 = rhs_value.skill_data_up_1b8;
    strings_1c4 = rhs_value.strings_1c4;
    skill_data_up_1dc = rhs_value.skill_data_up_1dc;
    skill_data_up_1e8 = rhs_value.skill_data_up_1e8;
    strings_1f4 = rhs_value.strings_1f4;
    aura_appendages_200 = rhs_value.aura_appendages_200;
    value_20c = rhs_value.value_20c;
    value_210 = rhs_value.value_210;
    hardcoding_parameters_214 = rhs_value.hardcoding_parameters_214;
    for (int index = 0; index < 3; ++index) {
        values_220[index] = rhs_value.values_220[index];
    }
    value_22c = rhs_value.value_22c;
    reserved_22d = rhs_value.reserved_22d;
    reserved_22e = rhs_value.reserved_22e;
    reserved_22f = rhs_value.reserved_22f;
    value_230 = rhs_value.value_230;
    value_234 = rhs_value.value_234;
    value_238 = rhs_value.value_238;
    value_23c = rhs_value.value_23c;
    value_23d = rhs_value.value_23d;
    reserved_23e = rhs_value.reserved_23e;
    recovery_statuses_240 = rhs_value.recovery_statuses_240;
    skill_levelups_24c = rhs_value.skill_levelups_24c;
    skill_levelups_258 = rhs_value.skill_levelups_258;
    avatar_select_abilities_264 = rhs_value.avatar_select_abilities_264;
    value_270 = rhs_value.value_270;
    value_274 = rhs_value.value_274;
    value_278 = rhs_value.value_278;
    value_27c = rhs_value.value_27c;
    for (int index = 0; index < 102; ++index) {
        values_280[index] = rhs_value.values_280[index];
    }
    all_skill_items_418 = rhs_value.all_skill_items_418;
    item_overpower_enabled_424 = rhs_value.item_overpower_enabled_424;
    item_overpower_ready_425 = rhs_value.item_overpower_ready_425;
    reserved_426 = rhs_value.reserved_426;
    item_overpower_stats_428 = rhs_value.item_overpower_stats_428;
    for (int index = 0; index < 3; ++index) {
        values_434[index] = rhs_value.values_434[index];
        values_444[index] = rhs_value.values_444[index];
    }
    value_440 = rhs_value.value_440;
    reserved_441 = rhs_value.reserved_441;
    reserved_442 = rhs_value.reserved_442;
    reserved_443 = rhs_value.reserved_443;
    rebirth_450 = rhs_value.rebirth_450;
    conditional_values_468 = rhs_value.conditional_values_468;
    variable_stats_474 = rhs_value.variable_stats_474;
    equipment_parameter_0828::Copy(this, &rhs_value);
    return *this;
}

// IDA_EA: 0x891A0D4
// IDA_NAME: _ZN22EquipmentParameterInfoC2ERKS_
// DEMANGLED: EquipmentParameterInfo::EquipmentParameterInfo(EquipmentParameterInfo const&)
EquipmentParameterInfo::EquipmentParameterInfo(
    const EquipmentParameterInfo& rhs_value) {
    ++g_EquipmentParameterInfoCount;
    g_EquipmentParameterInfoSize = 1152 * g_EquipmentParameterInfoCount;
    *this = rhs_value;
}

// IDA_EA: 0x89C2C42
// IDA_NAME: _ZN8PositionC2Ev
// DEMANGLED: Position::Position(void)
Position::Position() {
    x = 0;
    y = 0;
    z = 0;
}

// IDA_EA: 0x8513A6E
// IDA_NAME: _ZN16WpBonusPointDataC2Ev
// DEMANGLED: WpBonusPointData::WpBonusPointData(void)
WpBonusPointData::WpBonusPointData() {
    clear();
}

// IDA_EA: 0x8513A82
// IDA_NAME: _ZN16WpBonusPointData5clearEv
// DEMANGLED: WpBonusPointData::clear(void)
WpBonusPointData* WpBonusPointData::clear() {
    type = 2;
    value = 0;
    return this;
}

// IDA_EA: 0x8513A9A
// IDA_NAME: _ZN17ExpBonusPointDataC2Ev
// DEMANGLED: ExpBonusPointData::ExpBonusPointData(void)
ExpBonusPointData::ExpBonusPointData() {
    clear();
}

// IDA_EA: 0x8513AAE
// IDA_NAME: _ZN17ExpBonusPointData5clearEv
// DEMANGLED: ExpBonusPointData::clear(void)
ExpBonusPointData* ExpBonusPointData::clear() {
    type = 2;
    value = 0;
    return this;
}

// IDA_EA: 0x898F084
// IDA_NAME: _ZN17STEquipmentScript13artifact_infoC2Ev
// DEMANGLED: STEquipmentScript::artifact_info::artifact_info(void)
STEquipmentScript::artifact_info::artifact_info() {
    usable_type = 0;
    grade = 0;
    rarity = 0;
    physical_attack = 0.0f;
    magical_attack = 0.0f;
    physical_defense = 0.0f;
    magical_defense = 0.0f;
    strength = 0.0f;
    vitality = 0.0f;
    spirit = 0.0f;
    item_index = 0;
}

// IDA_EA: 0x898F0FE
// IDA_NAME: _ZNK17STEquipmentScript13artifact_info8isUsableEv
// DEMANGLED: STEquipmentScript::artifact_info::isUsable(void)const
int STEquipmentScript::artifact_info::isUsable() const {
    return usable_type || grade || rarity || physical_attack != 0.0f ||
           magical_attack != 0.0f || physical_defense != 0.0f ||
           magical_defense != 0.0f || strength != 0.0f || vitality != 0.0f ||
           spirit != 0.0f || item_index;
}

// IDA_EA: 0x898DB82
// IDA_NAME: _ZN17STEquipmentScriptC2Ev
// DEMANGLED: STEquipmentScript::STEquipmentScript(void)
STEquipmentScript::STEquipmentScript() {
    Clear();
}

STEquipmentScript::AuraPositionData::AuraPositionData()
    : character_job(oldsrv::config::kCharacterJobInvalid),
      x(0),
      y(0),
      variant(0),
      value_count(0),
      reserved_11(0),
      reserved_12(0) {}

// IDA_EA: 0x898E9E4
// IDA_NAME: _ZN17STEquipmentScript5ClearEv
// DEMANGLED: STEquipmentScript::Clear(void)
int STEquipmentScript::Clear() {
    STEquipmentScript* self = this;
    item.Clear();
    self->value_6bc = 0;
    self->value_764 = 0;
    self->strings_6a4[0].clear();
    self->strings_6a4[2].clear();
    self->strings_6a4[5].clear();
    self->set_name_668.clear();
    self->values_178[0] = -1;
    self->values_178[1] = 70;
    self->values_178[2] = -1;
    self->values_178[3] = -1;
    self->values_178[4] = -1;
    self->string_1a4.clear();
    self->strings_1a8.clear();
    // 0828 CLIENT_EA 0x1571079: an equipment script has no valid slot until
    // [equipment type] is parsed.  Using artifact-green (26) here turns any
    // prematurely terminated avatar script into creature equipment.
    self->values_1b4[0] = oldsrv::config::kEquipmentTypeInvalid;
    self->values_1b4[1] = 0;
    self->values_1b4[2] = 0;
    self->parameter_1c0.clear();
    self->animation_infos_640.clear();
    self->strings_658.clear();
    self->directory_664.clear();
    self->values_66c.clear();
    self->set_parameters_678.clear();
    self->level_parameters_690.clear();
    self->value_69c = 0;
    self->value_6a0 = -1;
    self->avatar_type_select_6c0.clear();
    self->avatar_emblems_6cc.clear();
    self->value_6d8 = -1;
    self->value_6dc = -1;
    self->value_6e0 = 0;
    self->value_6e4 = -1;
    self->impossible_upgrade_6f4 = 0;
    self->impossible_amplify_6f5 = 0;
    self->impossible_disjoint_6f6 = 0;
    self->item.value_d4 = 0;
    self->value_6f8 = 0;
    self->usable_modules_714.clear();
    self->usable_modules_714.insert(20);
    self->value_72c = 0;
    self->value_72d = 0;
    self->value_730 = 0;
    self->value_734 = 0;
    self->values_768.clear();
    self->values_774.clear();
    self->values_780.clear();
    self->value_78c = 0;
    self->value_790 = 0;
    self->value_794 = 0;
    self->value_798 = -1;
    self->value_79c = -1;
    self->values_7a0.clear();
    self->clear_avatar_7ac = 0;
    self->strings_7b0.clear();
    self->value_7bc = 0;
    self->value_7c0 = -1;
    self->value_7c4 = -1;
    self->value_7c8 = 0;
    self->enchant_data_7cc.clear();
    self->pairs_7d8.clear();
    self->pairs_7e4.clear();
    self->string_7f0.clear();
    self->string_7f4.clear();
    self->key_command_804 = 0;
    self->value_808 = 3;
    self->character_name_x_80c = 0;
    self->character_name_y_80e = 0;
    self->value_810 = 0;
    self->value_814 = 2;
    self->value_818 = 0;
    self->value_82c = 0;
    self->values_830.clear();
    self->string_83c.clear();
    self->back_images_840.clear();
    self->effects_84c.clear();
    self->strings_858.clear();
    self->values_864.clear();
    self->string_878.clear();
    self->value_87c = 0;
    self->value_880 = 0xff;
    self->value_884 = 0;
    self->value_888 = 0;
    self->value_889 = 1;
    self->value_88a = 0;
    self->value_88b = 0;
    self->value_88c = 0;
    self->value_890 = 0;
    self->full_avatar_flag = 0;
    self->rental_service_type = 0;
    self->skin_emblem_socket_enabled = 0;
    self->hidden_grow_avatar_types.clear();
    self->dictionary_text.clear();
    self->upgrade_limit_table_name.clear();
    self->equipment_grade_text_id = 5;
    self->seed_correction_index = -1;
    self->equipment_growth_type = 0;
    self->equipment_growth_stage = 0;
    self->reserved_8b6 = 0;
    self->spectrum_avatar_rgba = 0;
    self->spectrum_avatar_parameter_5 = 0;
    self->spectrum_avatar_parameter_6 = 2;
    self->weapon_overspec_stats.clear();
    self->weapon_overspec_item_categories.clear();
    self->aura_position_mode = 0;
    self->aura_position_data.clear();
    return 0;
}

// IDA_EA: 0x898EFA8
// IDA_NAME: _ZN17STEquipmentScript10addAniInfoE17ENUM_CHARACTERJOB
// DEMANGLED: STEquipmentScript::addAniInfo(ENUM_CHARACTERJOB)
EquipmentAniInfoScript* STEquipmentScript::addAniInfo(ENUM_CHARACTERJOB job) {
    return &animation_infos_640[job];
}

// IDA_EA: 0x898EF1A
// IDA_NAME: _ZN17STEquipmentScript10getAniInfoE17ENUM_CHARACTERJOB
// DEMANGLED: STEquipmentScript::getAniInfo(ENUM_CHARACTERJOB)
EquipmentAniInfoScript* STEquipmentScript::getAniInfo(ENUM_CHARACTERJOB job) {
    std::map<ENUM_CHARACTERJOB, EquipmentAniInfoScript>::iterator found =
        animation_infos_640.find(job);
    if (found != animation_infos_640.end()) {
        return &found->second;
    }
    if (job == static_cast<ENUM_CHARACTERJOB>(
                   oldsrv::config::kCharacterJobCommon)) {
        return 0;
    }
    return getAniInfo(static_cast<ENUM_CHARACTERJOB>(
        oldsrv::config::kCharacterJobCommon));
}

// IDA_EA: 0x898EFC8
// IDA_NAME: _ZN17STEquipmentScript17isUpgradableLevelEN15ItemUpgradeType1TEi
// DEMANGLED: STEquipmentScript::isUpgradableLevel(ItemUpgradeType::T,int)
bool STEquipmentScript::isUpgradableLevel(ItemUpgradeType::T upgrade_type,
                                          int level) {
    std::map<ItemUpgradeType::T, std::pair<int, int> >::iterator found =
        upgradable_levels_6fc.find(upgrade_type);
    if (found != upgradable_levels_6fc.end() &&
        (found->second.first > level || found->second.second < level)) {
        return false;
    }
    return true;
}

// IDA_EA: 0x898F066
// IDA_NAME: _ZN17STEquipmentScript22isExistUpgradableLevelEv
// DEMANGLED: STEquipmentScript::isExistUpgradableLevel(void)
bool STEquipmentScript::isExistUpgradableLevel() {
    return upgradable_levels_6fc.size() != 0;
}

// IDA_EA: 0x83D6AC4
// IDA_NAME: _ZN16STKeyCommandUnitC2ERKS_
// DEMANGLED: STKeyCommandUnit::STKeyCommandUnit(STKeyCommandUnit const&)
STKeyCommandUnit::STKeyCommandUnit(const STKeyCommandUnit& rhs)
    : keys(rhs.keys), command(rhs.command), interval(rhs.interval) {}

// IDA_EA: 0x83DB2AE
// IDA_NAME: _ZN16STKeyCommandUnitD2Ev
// DEMANGLED: STKeyCommandUnit::~STKeyCommandUnit()
STKeyCommandUnit::~STKeyCommandUnit() {}

// IDA_EA: 0x83E4C6A
// IDA_NAME: _ZN16STKeyCommandUnitaSERKS_
// DEMANGLED: STKeyCommandUnit::operator=(STKeyCommandUnit const&)
STKeyCommandUnit& STKeyCommandUnit::operator=(const STKeyCommandUnit& rhs) {
    keys = rhs.keys;
    command = rhs.command;
    interval = rhs.interval;
    return *this;
}

// IDA_EA: 0x88C414C
// IDA_NAME: _ZN16STKeyCommandUnit5clearEv
// DEMANGLED: STKeyCommandUnit::clear(void)
STKeyCommandUnit* STKeyCommandUnit::clear() {
    keys.clear();
    command = 0;
    interval = 300;
    return this;
}

// IDA_EA: 0x88C4174
// IDA_NAME: _ZN16STKeyCommandUnitC2Ev
// DEMANGLED: STKeyCommandUnit::STKeyCommandUnit(void)
STKeyCommandUnit::STKeyCommandUnit() : keys(), command(0), interval(300) {
    clear();
}

// IDA_EA: 0x83660EC
// IDA_NAME: _ZN12STKeyCommand5clearEv
// DEMANGLED: STKeyCommand::clear(void)
STKeyCommand* STKeyCommand::clear() {
    key_commands.clear();
    command_states.clear();
    type = 1;
    value_1c = 0;
    enabled = 1;
    return this;
}

// IDA_EA: 0x8366128
// IDA_NAME: _ZN12STKeyCommandC2Ev
// DEMANGLED: STKeyCommand::STKeyCommand(void)
STKeyCommand::STKeyCommand()
    : key_commands(), command_states(), type(1), value_1c(0), enabled(1),
      reserved_21(0), reserved_22(0), reserved_23(0) {
    clear();
}

// IDA_EA: 0x83755FC
// IDA_NAME: _ZN12STKeyCommandD2Ev
// DEMANGLED: STKeyCommand::~STKeyCommand()
STKeyCommand::~STKeyCommand() {}

// IDA_EA: 0x88C167A
// IDA_NAME: _Z13importCommandP12STKeyCommand
// DEMANGLED: importCommand(STKeyCommand *)
int importCommand(STKeyCommand* command) {
    if (command == 0) {
        return 0;
    }

    STKeyCommandUnit unit;
    command->key_commands.clear();
    while (1) {
        const DNFLEX_TOKEN token = g_dnfLex.getToken();
        if (token == DNF_TOK_EOF || token == DNF_TOK_TYPE) {
            break;
        }

        if (token == DNF_TOK_CUSTOM_DATA) {
            const std::map<std::string, int>::const_iterator found =
                g_mapKeyNameIndexTable.find(
                    g_dnfLex.getCustomData());
            if (found != g_mapKeyNameIndexTable.end()) {
                unit.keys.push_back(found->second);
            }
        } else if (token == DNF_TOK_DECIMAL) {
            unit.command = g_dnfLex.getDecimal();
        } else if (token == DNF_TOK_STRING) {
            const char* state = g_dnfLex.getString();
            if (std::strcmp(state, "DOWN") == 0) {
                command->type = 0;
            } else if (std::strcmp(state, "DOWN ONCE") == 0) {
                command->type = 1;
            } else if (std::strcmp(state, "UP") == 0) {
                command->type = 2;
            } else if (std::strcmp(state, "UP ONCE") == 0) {
                command->type = 3;
            }
        } else if (token == DNF_TOK_CONNECTOR) {
            const char connector = *g_dnfLex.getConnector();
            if (connector == '&') {
                if (!unit.keys.empty()) {
                    command->command_states = unit.keys;
                }
            } else {
                if (connector == '!') {
                    unit.interval = 150;
                } else if (connector == ',') {
                    unit.interval = 300;
                } else if (connector == '~') {
                    unit.interval = 1000;
                }
                if (!unit.keys.empty()) {
                    command->key_commands.push_back(unit);
                }
            }
            unit.clear();
        }
    }
    if (!unit.keys.empty()) {
        command->key_commands.push_back(unit);
    }
    return 1;
}

// IDA_EA: 0x85141C4
// IDA_NAME: _ZN7DyeInfoD2Ev
// DEMANGLED: DyeInfo::~DyeInfo()
DyeInfo::~DyeInfo() {}

// IDA_EA: 0x89C36DA
// IDA_NAME: _ZN12STItemScript10SEXPERTJOBD2Ev
// DEMANGLED: STItemScript::SEXPERTJOB::~SEXPERTJOB()
STItemScript::SEXPERTJOB::~SEXPERTJOB() {}

// IDA_EA: 0x89C3AF6
// IDA_NAME: _ZN12STItemScript10SEXPERTJOBaSERKS0_
// DEMANGLED: STItemScript::SEXPERTJOB::operator=(STItemScript::SEXPERTJOB const&)
STItemScript::SEXPERTJOB& STItemScript::SEXPERTJOB::operator=(
    const SEXPERTJOB& rhs) {
    result_variation = rhs.result_variation;
    additional_exp = rhs.additional_exp;
    disjoint_big_win_rate = rhs.disjoint_big_win_rate;
    disjoint_result_variation = rhs.disjoint_result_variation;
    material_variation = rhs.material_variation;
    compound_rate = rhs.compound_rate;
    usable = rhs.usable;
    return *this;
}

// IDA_EA: 0x89D4C7A
// IDA_NAME: _ZN12STItemScript10SEXPERTJOB7SUSABLED2Ev
// DEMANGLED: STItemScript::SEXPERTJOB::SUSABLE::~SUSABLE()
STItemScript::SEXPERTJOB::SUSABLE::~SUSABLE() {}

// IDA_EA: 0x89C2C64
// IDA_NAME: _ZN6effect8STEffect5clearEv
// DEMANGLED: effect::STEffect::clear(void)
effect::STEffect* effect::STEffect::clear() {
    attach_target = 0;
    type = 0;
    attach_position = 0;
    z_revision = 0;
    index = 0;
    options = 0;
    module_type = 20;
    effect_name.clear();
    return this;
}

// IDA_EA: 0x89C2CB4
// IDA_NAME: _ZN6effect8STEffectC2Ev
// DEMANGLED: effect::STEffect::STEffect(void)
effect::STEffect::STEffect() : effect_name() {
    clear();
}

// IDA_EA: 0x89CB7C0
// IDA_NAME: _ZN6effect8STEffectC2ERKS0_
// DEMANGLED: effect::STEffect::STEffect(effect::STEffect const&)
effect::STEffect::STEffect(const STEffect& other) : effect_name(other.effect_name) {
    attach_target = other.attach_target;
    type = other.type;
    attach_position = other.attach_position;
    z_revision = other.z_revision;
    index = other.index;
    options = other.options;
    module_type = other.module_type;
}

// IDA_EA: 0x89CB8BA
// IDA_NAME: _ZN6effect8STEffectaSERKS0_
// DEMANGLED: effect::STEffect::operator=(effect::STEffect const&)
effect::STEffect& effect::STEffect::operator=(const STEffect& other) {
    attach_target = other.attach_target;
    type = other.type;
    attach_position = other.attach_position;
    z_revision = other.z_revision;
    index = other.index;
    options = other.options;
    module_type = other.module_type;
    effect_name = other.effect_name;
    return *this;
}

// IDA_EA: 0x89C4FD4
// IDA_NAME: _ZN6effect8STEffectD2Ev
// DEMANGLED: effect::STEffect::~STEffect()
effect::STEffect::~STEffect() {}

// IDA_EA: 0x83DB300
// IDA_NAME: _ZN22EquipmentParameterInfo11SASPROPERTYD2Ev
// DEMANGLED: EquipmentParameterInfo::SASPROPERTY::~SASPROPERTY()
EquipmentParameterInfo::SASPROPERTY::~SASPROPERTY() {}

// IDA_EA: 0x89D2CF2
// IDA_NAME: _ZN22EquipmentParameterInfo26EquipmentAuraAppendageDataD2Ev
// DEMANGLED: EquipmentParameterInfo::EquipmentAuraAppendageData::~EquipmentAuraAppendageData()
EquipmentParameterInfo::EquipmentAuraAppendageData::~EquipmentAuraAppendageData() {}

// IDA_EA: 0x891A4C6
// IDA_NAME: _ZN22EquipmentParameterInfoD2Ev
// DEMANGLED: EquipmentParameterInfo::~EquipmentParameterInfo()
EquipmentParameterInfo::~EquipmentParameterInfo() {
    equipment_parameter_0828::Clear(this);
    --g_EquipmentParameterInfoCount;
    g_EquipmentParameterInfoSize = 1152 * g_EquipmentParameterInfoCount;
}

// IDA_EA: 0x89C2E48
// IDA_NAME: _ZN22EquipmentParameterInfo26getEquipmentPhysicalAttackEi
// DEMANGLED: EquipmentParameterInfo::getEquipmentPhysicalAttack(int)
char* EquipmentParameterInfo::getEquipmentPhysicalAttack(int index) {
    return values_000[21 + index].bytes;
}

// IDA_EA: 0x89C2E5C
// IDA_NAME: _ZN22EquipmentParameterInfo27getEquipmentPhysicalDefenseEi
// DEMANGLED: EquipmentParameterInfo::getEquipmentPhysicalDefense(int)
char* EquipmentParameterInfo::getEquipmentPhysicalDefense(int index) {
    return values_000[23 + index].bytes;
}

// IDA_EA: 0x89C2E70
// IDA_NAME: _ZN22EquipmentParameterInfo25getEquipmentMagicalAttackEi
// DEMANGLED: EquipmentParameterInfo::getEquipmentMagicalAttack(int)
char* EquipmentParameterInfo::getEquipmentMagicalAttack(int index) {
    return values_000[25 + index].bytes;
}

// IDA_EA: 0x89C2E84
// IDA_NAME: _ZN22EquipmentParameterInfo26getEquipmentMagicalDefenseEi
// DEMANGLED: EquipmentParameterInfo::getEquipmentMagicalDefense(int)
char* EquipmentParameterInfo::getEquipmentMagicalDefense(int index) {
    return values_000[27 + index].bytes;
}

// IDA_EA: 0x89C2E98
// IDA_NAME: _ZN22EquipmentParameterInfo9addStringESs
// DEMANGLED: EquipmentParameterInfo::addString(std::string)
int EquipmentParameterInfo::addString(std::string value) {
    std::map<int, std::string>* strings = &strings_1c4;

    int result = -1;
    for (std::map<int, std::string>::iterator it = strings->begin();
         it != strings->end();
         ++it) {
        if (it->second == value) {
            result = it->first;
            break;
        }
    }

    if (result == -1) {
        result = static_cast<int>(strings->size()) + 1;
        (*strings)[result] = value;
    }
    return result;
}

// IDA_EA: 0x89C36F0
// IDA_NAME: _ZN12STItemScriptD2Ev
// DEMANGLED: STItemScript::~STItemScript()
STItemScript::~STItemScript() {
    item_script_0828::SetEventRewardItem(this, false);
}

// IDA_EA: 0x89C3B6C
// IDA_NAME: _ZN12STItemScriptaSERKS_
// DEMANGLED: STItemScript::operator=(STItemScript const&)
STItemScript& STItemScript::operator=(const STItemScript& rhs_value) {
    STItemScript* self = this;
    const STItemScript* rhs = &rhs_value;
    self->index = rhs->index;
    self->grade = rhs->grade;
    for (int index = 0; index < 7; ++index) {
        self->values_08[index] = rhs->values_08[index];
    }
    self->value_24 = rhs->value_24;
    self->value_28 = rhs->value_28;
    self->value_2c = rhs->value_2c;
    self->value_30 = rhs->value_30;
    self->value_31 = rhs->value_31;
    self->value_32 = rhs->value_32;
    self->value_34 = rhs->value_34;
    for (int index = 0; index < oldsrv::config::kCharacterJobCount; ++index) {
        self->usable_job_flags[index] = rhs->usable_job_flags[index];
    }
    self->usable_jobs = rhs->usable_jobs;
    for (int index = 0; index < 4; ++index) {
        self->values_5c[index] = rhs->values_5c[index];
    }
    self->icon = rhs->icon;
    self->icon_index = rhs->icon_index;
    self->field_image = rhs->field_image;
    self->field_image_index = rhs->field_image_index;
    self->tarotcard_view = rhs->tarotcard_view;
    self->tarotcard_view_index = rhs->tarotcard_view_index;
    self->name = rhs->name;
    self->int_data = rhs->int_data;
    self->string_data = rhs->string_data;
    self->value_a0 = rhs->value_a0;
    self->value_a1 = rhs->value_a1;
    self->value_a2 = rhs->value_a2;
    self->value_a3 = rhs->value_a3;
    self->values_a4[0] = rhs->values_a4[0];
    self->values_a4[1] = rhs->values_a4[1];
    for (int index = 0; index < 6; ++index) {
        self->strings_ac[index] = rhs->strings_ac[index];
    }
    for (int index = 0; index < 3; ++index) {
        self->values_c4[index] = rhs->values_c4[index];
    }
    self->value_d0 = rhs->value_d0;
    self->value_d1 = rhs->value_d1;
    self->value_d2 = rhs->value_d2;
    self->value_d3 = rhs->value_d3;
    self->value_d4 = rhs->value_d4;
    self->value_d8 = rhs->value_d8;
    self->value_d9 = rhs->value_d9;
    self->value_da = rhs->value_da;
    self->value_db = rhs->value_db;
    self->usable_worlds = rhs->usable_worlds;
    self->values_f4[0] = rhs->values_f4[0];
    self->values_f4[1] = rhs->values_f4[1];
    self->value_fc = rhs->value_fc;
    self->value_fd = rhs->value_fd;
    self->value_fe = rhs->value_fe;
    self->value_ff = rhs->value_ff;
    self->value_100 = rhs->value_100;
    self->value_104 = rhs->value_104;
    self->special_monster_drop = rhs->special_monster_drop;
    self->value_10e = rhs->value_10e;
    self->value_10f = rhs->value_10f;
    self->special_drop_items = rhs->special_drop_items;
    self->item_categories = rhs->item_categories;
    self->usable_upgrade_character_level = rhs->usable_upgrade_character_level;
    self->value_136 = rhs->value_136;
    self->expert_job = rhs->expert_job;
    self->dye_info = rhs->dye_info;
    self->value_174 = rhs->value_174;
    item_script_0828::Copy(self, rhs);
    return *this;
}

// IDA_EA: 0x89D15C2
// IDA_NAME: _ZN18STChangeStatusRateD2Ev
// DEMANGLED: STChangeStatusRate::~STChangeStatusRate()
STChangeStatusRate::~STChangeStatusRate() {}

// IDA_EA: 0x89D1132
// IDA_NAME: _ZN22stSetItemBackImageInfoD2Ev
// DEMANGLED: stSetItemBackImageInfo::~stSetItemBackImageInfo()
stSetItemBackImageInfo::~stSetItemBackImageInfo() {}

// IDA_EA: 0x898E298
// IDA_NAME: _ZN17STEquipmentScriptD2Ev
// DEMANGLED: STEquipmentScript::~STEquipmentScript()
STEquipmentScript::~STEquipmentScript() {
    STKeyCommand* key_command = key_command_804;
    if (key_command != 0) {
        key_command->~STKeyCommand();
        ::operator delete(key_command);
    }
    key_command_804 = 0;
}

// IDA_EA: 0x8991856
// IDA_NAME: _Z33EquipmentScriptSpecialMonsterDropP17STEquipmentScriptSsPKc
// DEMANGLED: EquipmentScriptSpecialMonsterDrop(STEquipmentScript *,std::string,char const*)
int EquipmentScriptSpecialMonsterDrop(STEquipmentScript* script,
                                      std::string tag,
                                      const char*) {
    if (tag == "[special monster drop]") {
        bool success = false;
        const int value = ScanInt(&success);
        if (!success) {
            return 0;
        }
        script->item.special_monster_drop.enabled = value != 0;
        return 1;
    }

    std::string text;
    if (tag == "[difficulty for extra drop]") {
        while (ScanStr(&text) == 1) {
            if (text == "[easy]") {
                script->item.special_monster_drop.extra_drop_easy = 1;
            } else if (text == "[medium]") {
                script->item.special_monster_drop.extra_drop_medium = 1;
            } else if (text == "[hard]") {
                script->item.special_monster_drop.extra_drop_hard = 1;
            } else if (text == "[ultimate]") {
                script->item.special_monster_drop.extra_drop_ultimate = 1;
            } else if (text == "[hero]") {
                script->item.special_monster_drop.extra_drop_hero = 1;
            } else if (text == "[all]") {
                script->item.special_monster_drop.extra_drop_easy = 1;
                script->item.special_monster_drop.extra_drop_medium = 1;
                script->item.special_monster_drop.extra_drop_hard = 1;
                script->item.special_monster_drop.extra_drop_ultimate = 1;
                script->item.special_monster_drop.extra_drop_hero = 1;
            }
        }
        return 1;
    }

    if (tag == "[dungeon type for extra drop]") {
        while (ScanStr(&text) == 1) {
            if (text == "[normal]") {
                script->item.special_monster_drop.dungeon_normal = 1;
            } else if (text == "[ancient]") {
                script->item.special_monster_drop.dungeon_ancient = 1;
            } else if (text == "[dimension]") {
                script->item.special_monster_drop.dungeon_dimension = 1;
            } else if (text == "[all]") {
                script->item.special_monster_drop.dungeon_normal = 1;
                script->item.special_monster_drop.dungeon_ancient = 1;
                script->item.special_monster_drop.dungeon_dimension = 1;
            }
        }
        return 1;
    }
    // IDA 0x8991856 is the terminal compatibility handler for the equipment
    // grammar.  It reports success when no special-drop tag was recognized;
    // this deliberately lets the outer importer continue past extension and
    // closing tags consumed by adjacent grammar blocks.
    return 1;
}

// IDA_EA: 0x8990F1F
// IDA_NAME: _Z16GetEquipmentTypePKc
// DEMANGLED: GetEquipmentType(char const*)
int GetEquipmentType(const char* text) {
    static const char* kTags[] = {
        "[hat avatar]", "[hair avatar]", "[face avatar]", "[coat avatar]",
        "[pants avatar]", "[shoes avatar]", "[breast avatar]",
        "[waist avatar]", "[skin avatar]", "[aurora avatar]",
        "[weapon avatar]", "[weapon]", "[title name]", "[coat]",
        "[shoulder]", "[pants]", "[shoes]", "[waist]", "[amulet]",
        "[wrist]", "[ring]", "[support]", "[magic stone]", "[creature]",
        "[artifact red]", "[artifact blue]", "[artifact green]"};
    for (int i = 0; i < oldsrv::config::kEquipmentTypeCount; ++i) {
        if (std::strcmp(text, kTags[i]) == 0) {
            return i;
        }
    }
    return oldsrv::config::kEquipmentTypeInvalid;
}

// CLIENT_EA: 0x14DFB60
// CLIENT_NAME: GetGrowAvatarTypeByName
// The client compares these PVF strings exactly and returns 37 when none
// matches. Several spellings are intentionally retained from the client.
ENUM_GROW_AVATAR_TYPE GetGrowAvatarTypeByName(const char* text) {
    static const char* const kGrowAvatarTypeNames[GROW_AVATAR_TYPE_INVALID] = {
        "Weapon Master Gloves",
        "Soul Bringer Aura",
        "Berserker Red Eye",
        "Asura Red Eye Patch",
        "Demonic Swordman Black Hand",
        "Nen Master Aura",
        "Striker Wrist Wrap",
        "Street Fighter Band",
        "Grappler Hair Band",
        "Ranger Cigarette",
        "Launcher Heavy Weapon",
        "Mechanic Robot",
        "Spitfire Magazine",
        "Ranger Necklace",
        "Crusader Aura",
        "Infighter Aura",
        "Exocist Ghost",
        "Avenger Aura",
        "Elemantal Master Aura",
        "Summoner Book",
        "Summoner Floor Mark",
        "Battle Mage Gloves",
        "Witch Aid Kit",
        "Creator Coordination",
        "Rogue White Eye",
        "Necromencer Aura",
        "Elemental Bomber Elemental",
        "Glacial Master Breath",
        "Sword Master Light",
        "Demon Slayer Aura",
        "JianShen Blue Eye",
        "JianShen Sword",
        "Darklord Aura",
        "Blood Evil Injury",
        "Blood Evil Blood",
        "Indaratian Tattoo",
        "Indaratian Power"
    };

    if (text == 0) {
        return GROW_AVATAR_TYPE_INVALID;
    }
    for (int type = GROW_AVATAR_WEAPON_MASTER_GLOVES;
         type < GROW_AVATAR_TYPE_INVALID; ++type) {
        if (std::strcmp(text, kGrowAvatarTypeNames[type]) == 0) {
            return static_cast<ENUM_GROW_AVATAR_TYPE>(type);
        }
    }
    return GROW_AVATAR_TYPE_INVALID;
}

// CLIENT_EA: 0x154DFF0
// CLIENT_NAME: GetEquipmentGradeTextIdByName
int GetEquipmentGradeTextIdByName(const char* text) {
    if (text == 0) {
        return 5;
    }
    if (std::strcmp(text, "lowest") == 0) {
        return 23207;
    }
    if (std::strcmp(text, "lower") == 0) {
        return 4006;
    }
    if (std::strcmp(text, "normal") == 0) {
        return 27171;
    }
    if (std::strcmp(text, "upper") == 0) {
        return 24048;
    }
    if (std::strcmp(text, "superlative") == 0) {
        return 29296;
    }
    return 5;
}

// IDA_EA: 0x8990F66
// IDA_NAME: _Z19getAvatarEmblemTypePKc
// DEMANGLED: getAvatarEmblemType(char const*)
int getAvatarEmblemType(const char* text) {
    if (std::strncmp(text, "[A socket]", 10u) == 0) return 1;
    if (std::strncmp(text, "[B socket]", 10u) == 0) return 2;
    if (std::strncmp(text, "[C socket]", 10u) == 0) return 4;
    if (std::strncmp(text, "[D socket]", 10u) == 0) return 8;
    if (std::strncmp(text, "[S socket]", 10u) == 0) return 16;
    if (std::strncmp(text, "[M socket]", 10u) == 0) return 65519;
    if (std::strncmp(text, "[All socket]", 12u) == 0) return 65535;
    return 0;
}

// IDA_EA: 0x89BF6CA
// IDA_NAME: _ZL20GetExpertJobTypeListP12STItemScript
// DEMANGLED: GetExpertJobTypeList(STItemScript *)
static __attribute__((noinline, used)) int GetExpertJobTypeList(
    STItemScript* script) {
    if (!script) {
        return 0;
    }
    std::vector<STItemScript::SEXPERTJOB::SUSABLE>* usable =
        &script->expert_job.usable;
    usable->clear();

    std::string text;
    while (ScanStr(&text) == 1) {
        int type = 0;
        if (text == "enchanter") {
            type = 1;
        } else if (text == "alchemist") {
            type = 2;
        } else if (text == "disjointer") {
            type = 3;
        } else if (text == "doll_controller") {
            type = 4;
        } else if (text == "all") {
            type = 5;
        } else {
            return 0;
        }
        bool success = false;
        const int value = ScanInt(&success);
        if (!success) {
            return 0;
        }
        STItemScript::SEXPERTJOB::SUSABLE entry;
        entry.type = type;
        entry.value = value;
        usable->push_back(entry);
    }
    return !usable->empty();
}

// IDA_EA: 0x89BFC2F
// IDA_NAME: _ZL34GetExpertJobValueMaterialVariationP12STItemScript
// DEMANGLED: GetExpertJobValueMaterialVariation(STItemScript *)
static __attribute__((noinline, used)) bool
GetExpertJobValueMaterialVariation(STItemScript* script) {
    if (!script) {
        return false;
    }
    bool success = true;
    script->expert_job.material_variation = ScanFloat(&success);
    return script->expert_job.material_variation > -100.0f && success;
}

// IDA_EA: 0x89BFC84
// IDA_NAME: _ZL29GetExpertJobValueCompoundRateP12STItemScript
// DEMANGLED: GetExpertJobValueCompoundRate(STItemScript *)
static __attribute__((noinline, used)) bool GetExpertJobValueCompoundRate(
    STItemScript* script) {
    if (!script) {
        return false;
    }
    bool success = true;
    script->expert_job.compound_rate = ScanFloat(&success);
    return success;
}

// IDA_EA: 0x89BFCB5
// IDA_NAME: _ZL27GetExpertJobResultVariationP12STItemScript
// DEMANGLED: GetExpertJobResultVariation(STItemScript *)
static __attribute__((noinline, used)) bool GetExpertJobResultVariation(
    STItemScript* script) {
    if (!script) {
        return false;
    }
    bool success = true;
    script->expert_job.result_variation.value_00 = ScanFloat(&success);
    if (success) {
        script->expert_job.result_variation.value_04 =
            ScanFloat(&success);
    }
    return success;
}

// IDA_EA: 0x89BFD0B
// IDA_NAME: _ZL30GetExpertJobDisjointBigWinRateP12STItemScript
// DEMANGLED: GetExpertJobDisjointBigWinRate(STItemScript *)
static __attribute__((noinline, used)) bool GetExpertJobDisjointBigWinRate(
    STItemScript* script) {
    if (!script) {
        return false;
    }
    bool success = true;
    script->expert_job.disjoint_big_win_rate = ScanFloat(&success);
    return success;
}

// IDA_EA: 0x89BFD3C
// IDA_NAME: _ZL35GetExpertJobDisjointResultVariationP12STItemScript
// DEMANGLED: GetExpertJobDisjointResultVariation(STItemScript *)
static __attribute__((noinline, used)) bool
GetExpertJobDisjointResultVariation(STItemScript* script) {
    if (!script) {
        return false;
    }
    bool success = true;
    script->expert_job.disjoint_result_variation =
        ScanFloat(&success);
    return success;
}

// IDA_EA: 0x89BFD6D
// IDA_NAME: _ZL29GetExpertJobAdditionalGainExpP12STItemScript
// DEMANGLED: GetExpertJobAdditionalGainExp(STItemScript *)
static __attribute__((noinline, used)) bool GetExpertJobAdditionalGainExp(
    STItemScript* script) {
    if (!script) {
        return false;
    }
    bool success = true;
    script->expert_job.additional_exp.value_00 = ScanFloat(&success);
    if (success) {
        script->expert_job.additional_exp.value_04 = ScanInt(&success);
    }
    return success;
}

// CLIENT_EA: 0x154CBE0
// CLIENT_NAME: ReadItemCategoryList
static int GetOfficialItemCategory(const std::string& text,
                                   int* category) {
    if (category == 0) {
        return 0;
    }
    if (text == "monster card") {
        *category = STItemScript::ITEM_CATEGORY_MONSTER_CARD;
    } else if (text == "cannot bind") {
        *category = STItemScript::ITEM_CATEGORY_CANNOT_BIND;
    } else if (text == "boss drop") {
        *category = STItemScript::ITEM_CATEGORY_BOSS_DROP;
    } else if (text == "expertjob") {
        *category = STItemScript::ITEM_CATEGORY_EXPERT_JOB;
    } else if (text == "recipe") {
        *category = STItemScript::ITEM_CATEGORY_RECIPE;
    } else if (text == "legacy") {
        *category = STItemScript::ITEM_CATEGORY_LEGACY;
    } else {
        return 0;
    }
    return 1;
}

int ReadItemCategoryList(std::set<int>* categories) {
    if (categories == 0) {
        return 0;
    }
    std::string text;
    while (ScanStr(&text) == 1) {
        int category = STItemScript::ITEM_CATEGORY_NONE;
        if (GetOfficialItemCategory(text, &category) != 1) {
            return 0;
        }
        categories->insert(category);
    }
    return 1;
}

// CUSTOM_PVF_0828_COMPAT: china_artifact_R14.equ in the user's compiled
// 0828-format PVF contains `[item category] `no random`` and then the
// canonical standalone `[no random]` tag.  The 0627 server importer supports
// only category values 1..3, while the 0828 client reader @0x154CBE0 supports
// 1,2,3,4,5,7 and rejects every other category.  Translate this one exact,
// equipment-only spelling to the already-proven no-random flag.  The shared
// stackable/weapon-overspec reader remains official and every other unknown
// category still fails closed.
static int ReadEquipmentItemCategoryList(STEquipmentScript* script) {
    if (script == 0) {
        return 0;
    }
    std::string text;
    while (ScanStr(&text) == 1) {
        if (text == "no random") {
            script->value_72c = 1;
            continue;
        }
        int category = STItemScript::ITEM_CATEGORY_NONE;
        if (GetOfficialItemCategory(text, &category) != 1) {
            return 0;
        }
        script->item.item_categories.insert(category);
    }
    return 1;
}

// IDA_EA: 0x89BFDC5
// IDA_NAME: _ZL15GetItemCategoryP12STItemScript
// DEMANGLED: GetItemCategory(STItemScript *)
static __attribute__((noinline, used)) int GetItemCategory(
    STItemScript* script) {
    if (!script) {
        return 0;
    }
    std::set<int>* categories = &script->item_categories;
    std::string text;
    while (ScanStr(&text) == 1) {
        int category = 0;
        if (text == "monster card") {
            category = 1;
        } else if (text == "cannot bind") {
            category = 2;
        } else if (text == "boss drop") {
            category = 3;
        } else {
            return 0;
        }
        categories->insert(category);
    }
    return 1;
}

// IDA_EA: 0x89C019B
// IDA_NAME: _ZL19GetAuraAvatarOptionP17STEquipmentScript
// DEMANGLED: GetAuraAvatarOption(STEquipmentScript *)
static __attribute__((noinline, used)) bool GetAuraAvatarOption(
    STEquipmentScript* script) {
    if (!script) {
        return false;
    }
    bool success = true;
    std::string text;
    ScanStr(&text);
    if (text == "[party teleport]") {
        script->value_880 = 0;
        script->value_884 = ScanInt(&success);
    } else if (text == "[none]") {
        script->value_880 = 1;
        script->value_884 = 0;
    } else if (text == "[solo teleport]") {
        script->value_880 = 2;
        script->value_884 = ScanInt(&success);
    } else {
        return false;
    }
    return success;
}

static float float_from_bits(int value) {
    float out = 0.0f;
    std::memcpy(&out, &value, sizeof(out));
    return out;
}

// IDA_EA: 0x8991086
// IDA_NAME: _Z15GetSkillLevelUpPSt6vectorIfSaIfEE
// DEMANGLED: GetSkillLevelUp(std::vector<float,std::allocator<float>> *)
bool GetSkillLevelUp(std::vector<float>* values)
    __asm__("_Z15GetSkillLevelUpPSt6vectorIfSaIfEE");
bool GetSkillLevelUp(std::vector<float>* values) {
    std::string text;
    bool is_type = false;
    int scanned = -1;
    while (ScanTypeOrStr(is_type, text, true) == 1) {
        if (is_type) {
            return text == "[/skill levelup]";
        }
        values->push_back(float_from_bits(1105723392));
        if (text == "[common]") {
            values->push_back(float_from_bits(1093664768));
        } else {
            values->push_back(static_cast<float>(
                static_cast<unsigned int>(::getCharacterJob(text.c_str()))));
        }
        if (!ScanInt(&scanned)) {
            return false;
        }
        values->push_back(static_cast<float>(scanned));
        if (!ScanInt(&scanned)) {
            return false;
        }
        values->push_back(static_cast<float>(scanned));
    }
    return false;
}

// IDA_EA: 0x8991384
// IDA_NAME: _Z18GetStatByConditionPSt6vectorIfSaIfEE
// DEMANGLED: GetStatByCondition(std::vector<float,std::allocator<float>> *)
int GetStatByCondition(std::vector<float>* values)
    __asm__("_Z18GetStatByConditionPSt6vectorIfSaIfEE");
int GetStatByCondition(std::vector<float>* values) {
    values->push_back(float_from_bits(1108869120));
    std::string text;
    if (ScanStr(&text) != 1) {
        return 0;
    }
    const float status = static_cast<float>(getEquipmentActiveStatusType(text));
    if (ScanStr(&text) != 1) {
        return 0;
    }
    const float is_percent = (text == "%") ? 1.0f : 0.0f;
    bool success = false;
    const float value = static_cast<float>(ScanInt(&success));
    if (!success) {
        return 0;
    }
    values->push_back(status);
    values->push_back(is_percent);
    values->push_back(value);
    return 1;
}

// IDA_EA: 0x89914B9
// IDA_NAME: _Z12GetStatValuePSt6vectorIfSaIfEE
// DEMANGLED: GetStatValue(std::vector<float,std::allocator<float>> *)
int GetStatValue(std::vector<float>* values)
    __asm__("_Z12GetStatValuePSt6vectorIfSaIfEE");
int GetStatValue(std::vector<float>* values) {
    std::string text;
    if (ScanStr(&text) != 1) {
        return 0;
    }
    const float status = static_cast<float>(getEquipmentActiveStatusType(text));
    if (ScanStr(&text) != 1) {
        return 0;
    }
    const float is_percent = (text == "%") ? 1.0f : 0.0f;
    bool success = false;
    const float value = static_cast<float>(ScanInt(&success));
    if (!success) {
        return 0;
    }
    values->push_back(status);
    values->push_back(is_percent);
    values->push_back(value);
    return 1;
}

// IDA_EA: 0x89915D4
// IDA_NAME: _Z7GetStatPSt6vectorIfSaIfEE
// DEMANGLED: GetStat(std::vector<float,std::allocator<float>> *)
int GetStat(std::vector<float>* values) __asm__("_Z7GetStatPSt6vectorIfSaIfEE");
int GetStat(std::vector<float>* values) {
    values->push_back(float_from_bits(1094713344));
    return GetStatValue(values);
}

// IDA_EA: 0x89BD844
// IDA_NAME: _ZL15GetItemGrowtypePN22EquipmentParameterInfo11SASPROPERTYE
// DEMANGLED: GetItemGrowtype(EquipmentParameterInfo::SASPROPERTY *)
static __attribute__((noinline, used)) bool GetItemGrowtype(
    EquipmentParameterInfo::SASPROPERTY* property) {
    if (!property) {
        return false;
    }
    std::string text;
    if (ScanStr(&text) != 1) {
        return false;
    }
    property->item_growtype = ::getCharacterJob(text.c_str());
    return ScanInt(&property->item_id) != 0;
}

// IDA_EA: 0x89BD8EA
// IDA_NAME: _ZL22GetSkillApplyConditionPN22EquipmentParameterInfo11SASPROPERTYE
// DEMANGLED: GetSkillApplyCondition(EquipmentParameterInfo::SASPROPERTY *)
static __attribute__((noinline, used)) int GetSkillApplyCondition(
    EquipmentParameterInfo::SASPROPERTY* property) {
    if (!property) {
        return 0;
    }
    std::string tag;
    bool done = false;
    while (!done && ScanType(tag, true) == 1) {
        if (tag == "[/skill apply condition]") {
            done = true;
        } else if (tag == "[lower bound level]") {
            if (!ScanInt(&property->lower_bound_level)) {
                return 0;
            }
        } else if (tag == "[upper bound level]") {
            if (!ScanInt(&property->upper_bound_level)) {
                return 0;
            }
        } else if (tag == "[value]") {
            if (!ScanInt(&property->value)) {
                return 0;
            }
        } else if (tag == "[skill group]") {
            std::string group;
            if (ScanStr(&group) != 1) {
                return 0;
            }
            if (group == "[passive]") {
                property->skill_group = 1;
            } else if (group == "[active]") {
                property->skill_group = 0;
            } else if (group == "[all]") {
                property->skill_group = -1;
            } else {
                return 0;
            }
        } else if (tag == "[extra condition]") {
            int flags = 0;
            std::string condition;
            while (ScanStr(&condition) == 1) {
                if (condition == "[normal skill]") {
                    flags |= 1;
                } else if (condition == "[consume item skill]") {
                    flags |= 2;
                } else if (condition == "[ex skill]") {
                    flags |= 4;
                } else if (condition == "[except for ex skill]") {
                    flags |= 3;
                } else {
                    return 0;
                }
            }
            if (!flags) {
                return 0;
            }
            property->extra_condition = flags;
        } else {
            return 0;
        }
    }
    return done ? 1 : 0;
}

// IDA_EA: 0x89BE70C
// IDA_NAME: _ZL13GetSkillPhasePN22EquipmentParameterInfo11SASPROPERTYE
// DEMANGLED: GetSkillPhase(EquipmentParameterInfo::SASPROPERTY *)
static __attribute__((noinline, used)) int GetSkillPhase(
    EquipmentParameterInfo::SASPROPERTY* property) {
    if (!property) {
        return 0;
    }
    std::string text;
    if (ScanStr(&text) != 1) {
        return 0;
    }
    int phase = 0;
    if (text == "[all]") {
        phase = 5;
    } else if (text == "[common]") {
        phase = 4;
    } else if (text == "[first]") {
        phase = 0;
    } else if (text == "[second]") {
        phase = 1;
    } else if (text == "[third]") {
        phase = 2;
    } else if (text == "[fourth]") {
        phase = 3;
    } else {
        return 0;
    }
    property->skill_phase = phase;
    return 1;
}

// IDA_EA: 0x89BEC41
// IDA_NAME: _ZL18importAllSkillItemP22EquipmentParameterInfo
// DEMANGLED: importAllSkillItem(EquipmentParameterInfo *)
static __attribute__((noinline, used)) int importAllSkillItem(
    EquipmentParameterInfo* info) {
    EquipmentParameterInfo::SASPROPERTY property;
    property.item_growtype = 11;
    property.item_id = 0;
    property.skill_group = -1;
    property.lower_bound_level = 0;
    property.upper_bound_level = 0;
    property.value = 0;
    property.extra_condition = 7;
    property.skill_phase = 5;

    bool done = false;
    bool ok = true;
    std::string tag;
    while (ok && !done && ScanType(tag, true) == 1) {
        if (tag == "[/all skill item]") {
            done = true;
        } else if (tag == "[item growtype]") {
            ok = GetItemGrowtype(&property);
        } else if (tag == "[skill apply condition]") {
            ok = GetSkillApplyCondition(&property) == 1;
        } else if (tag == "[skill phase]") {
            ok = GetSkillPhase(&property) == 1;
        } else if (tag == "[skill levelup]") {
            ok = GetSkillLevelUp(&property.values);
        } else {
            ok = false;
        }
    }

    if (ok && done &&
        (property.item_growtype == 11 || property.skill_phase == 5 ||
         property.skill_phase == 4) &&
        static_cast<unsigned int>(property.lower_bound_level) <= 0x46u &&
        static_cast<unsigned int>(property.upper_bound_level) <= 0x46u &&
        property.lower_bound_level <= property.upper_bound_level &&
        property.value > 0) {
        info->all_skill_items_418.push_back(property);
    } else {
        ok = false;
    }
    return ok ? 1 : 0;
}

// IDA_EA: 0x89BF290
// IDA_NAME: _ZL23importItemOverpowerDescP22EquipmentParameterInfo
// DEMANGLED: importItemOverpowerDesc(EquipmentParameterInfo *)
static __attribute__((noinline, used)) int importItemOverpowerDesc(
    EquipmentParameterInfo* info) {
    std::vector<std::vector<float> >* stats =
        &info->item_overpower_stats_428;
    bool done = false;
    std::string tag;
    while (!done && ScanType(tag, true) == 1) {
        if (tag == "[/item overpower desc]") {
            done = true;
        } else if (tag == "[stat]") {
            std::vector<float> stat;
            if (GetStat(&stat)) {
                stats->push_back(stat);
            }
        } else {
            return 0;
        }
    }
    if (!done || stats->empty()) {
        return 0;
    }
    info->item_overpower_enabled_424 = 1;
    info->item_overpower_ready_425 = 1;
    return 1;
}

// CLIENT_EA: 0x1554470
// CLIENT_NAME: EquipmentParameterInfo_ImportWeaponOverspecPerItem
static __attribute__((noinline, used)) int ImportWeaponOverspecPerItem(
    STEquipmentScript* script) {
    if (script == 0) {
        return 0;
    }

    bool done = false;
    std::string tag;
    while (!done && ScanType(tag, true) == 1) {
        if (tag == "[/weapon overspec per item]") {
            done = true;
        } else if (tag == "[stat]") {
            std::vector<float> stat;
            if (GetStat(&stat)) {
                script->weapon_overspec_stats.push_back(stat);
            }
        } else if (tag == "[item category]") {
            if (ReadItemCategoryList(
                    &script->weapon_overspec_item_categories) != 1) {
                return 0;
            }
        } else {
            return 0;
        }
    }
    return done && !script->weapon_overspec_stats.empty() ? 1 : 0;
}

// CLIENT_EA: sub_154C220 @0x154C220.  The 0828 client owns exactly three
// values in this closed block at EquipmentParameterInfo+0xBB4/+0xBB8/+0xBBC.
// The old server prefix ends at 0x480, so publish the same typed record through
// the ABI-external sidecar only after the complete block validates.
static int ImportSkillCooltimeSetItemRate(EquipmentParameterInfo* info) {
    if (!info) return 0;

    equipment_parameter_0828::SkillCooltimeSetItemRate parsed;
    unsigned int seen = 0;
    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[/skill cooltime set item rate]") {
            // sub_154C220 only requires all three scalar reads and the
            // explicit closing tag.  Do not invent level/rate ranges that
            // would reject future official PVF values.
            if (seen != 7u) {
                return 0;
            }
            equipment_parameter_0828::SetSkillCooltimeSetItemRate(
                info, parsed);
            return 1;
        }

        bool success = false;
        if (tag == "[lower bound level]") {
            if ((seen & 1u) != 0u) return 0;
            parsed.lower_bound_level = ScanInt(&success);
            seen |= 1u;
        } else if (tag == "[upper bound level]") {
            if ((seen & 2u) != 0u) return 0;
            parsed.upper_bound_level = ScanInt(&success);
            seen |= 2u;
        } else if (tag == "[cooltime rate]") {
            if ((seen & 4u) != 0u) return 0;
            parsed.cooltime_rate = ScanFloat(&success);
            seen |= 4u;
        } else {
            return 0;
        }
        if (!success) return 0;
    }
    return 0;
}

// IDA_EA: 0x89A75EA
// IDA_NAME: _Z28importEquipmentParameterInfoP22EquipmentParameterInfoRSsPKcRiS4_
// DEMANGLED: importEquipmentParameterInfo(EquipmentParameterInfo *,std::string &,char const*,int &,int &)
int importEquipmentParameterInfo(EquipmentParameterInfo* info,
                                 std::string& tag,
                                 const char* filename,
                                 int& first,
                                 int& second) {
#define PARAM_LOW_WORD(offset) (info->values_000[(offset) / 4])
#define PARAM_HIGH_WORD(offset) (info->values_280[((offset) - 0x280) / 4])
#define PARAM_INT(offset) \
    (((offset) < 0x140 ? PARAM_LOW_WORD(offset) : PARAM_HIGH_WORD(offset)).integer)
#define PARAM_FLOAT(offset) \
    (((offset) < 0x140 ? PARAM_LOW_WORD(offset) : PARAM_HIGH_WORD(offset)).real)

    if (tag == "[skill cooltime set item rate]") {
        return ImportSkillCooltimeSetItemRate(info);
    }
    if (tag == "[HP MAX]") {
        PARAM_INT(0x04) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[HP MAX rate]") {
        PARAM_FLOAT(0x08) = ScanFloat(1, 0);
        return 1;
    }
    if (tag == "[MP MAX]") {
        PARAM_INT(0x0c) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[MP MAX rate]") {
        PARAM_FLOAT(0x10) = ScanFloat(1, 0);
        return 1;
    }
    if (tag == "[physical attack]") {
        PARAM_INT(0x14) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[physical defense]") {
        PARAM_INT(0x18) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[magical attack]") {
        PARAM_INT(0x1c) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[magical defense]") {
        PARAM_INT(0x24) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[all stat]") {
        const int value = ScanInt(static_cast<bool*>(0));
        PARAM_INT(0x14) = value;
        PARAM_INT(0x18) = value;
        PARAM_INT(0x1c) = value;
        PARAM_INT(0x24) = value;
        return 1;
    }

    if (tag == "[equipment physical attack]") {
        info->values_000[21].integer =
            ScanInt(static_cast<bool*>(0));
        info->values_000[22].integer =
            ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[equipment physical defense]") {
        info->values_000[23].integer =
            ScanInt(static_cast<bool*>(0));
        info->values_000[24].integer =
            ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[equipment magical attack]") {
        info->values_000[25].integer =
            ScanInt(static_cast<bool*>(0));
        info->values_000[26].integer =
            ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[equipment magical defense]") {
        info->values_000[27].integer =
            ScanInt(static_cast<bool*>(0));
        info->values_000[28].integer =
            ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[additional effect index]") {
        info->values_194.push_back(ScanInt(static_cast<bool*>(0)));
        return 1;
    }
    if (tag == "[recovery status]") {
        std::vector<std::vector<float> >* recovery =
            &info->recovery_statuses_240;
        recovery->clear();
        const int count = ScanInt(static_cast<bool*>(0));
        recovery->resize(count);
        if (count > 0) {
            bool success = false;
            float value = ScanFloat(&success);
            for (int i = 0; success; ++i) {
                (*recovery)[i % count].push_back(value);
                value = ScanFloat(&success);
            }
        }
        return 1;
    }
    if (tag == "[separate attack]") {
        PARAM_INT(0x388) = ScanInt(static_cast<bool*>(0));
        PARAM_INT(0x38c) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[separate rate]") {
        PARAM_FLOAT(0x390) = ScanFloat(static_cast<bool*>(0));
        PARAM_FLOAT(0x394) = ScanFloat(static_cast<bool*>(0));
        return 1;
    }

    if (tag == "[physical absolute damage]") {
        PARAM_INT(0x74) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[physical damage reduce]") {
        PARAM_INT(0x78) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[physical absolute defense]") {
        PARAM_FLOAT(0x7c) = ScanFloat(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[magical absolute damage]") {
        PARAM_INT(0x80) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[magical damage reduce]") {
        PARAM_INT(0x84) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[magical absolute defense]") {
        PARAM_FLOAT(0x88) = ScanFloat(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[fire resistance]") {
        PARAM_INT(0xb0) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[water resistance]") {
        PARAM_INT(0xb4) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[dark resistance]") {
        PARAM_INT(0xb8) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[light resistance]") {
        PARAM_INT(0xbc) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[all elemental resistance]") {
        PARAM_INT(0xc0) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[slow resistance]") {
        PARAM_INT(0xc4) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[freeze resistance]") {
        PARAM_INT(0xc8) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[poison resistance]" ||
        tag == "[posion resistance]") {
        // OFFICIAL_PVF_0828: every RandomOptions_603_PoisonResistance row
        // uses the historical `posion` spelling.  It is the same poison
        // tolerance slot (0xCC); accepting the alias keeps the canonical
        // spelling valid while preventing the 0828 option row from being
        // skipped by the shared equipment/random-option value importer.
        PARAM_INT(0xcc) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[stun resistance]") {
        PARAM_INT(0xd0) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[curse resistance]") {
        PARAM_INT(0xd4) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[blind resistance]") {
        PARAM_INT(0xd8) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[lightning resistance]") {
        PARAM_INT(0xdc) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[stone resistance]") {
        PARAM_INT(0xe0) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[sleep resistance]") {
        PARAM_INT(0xe4) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[burn resistance]" ||
        tag == "[deelement resistance]") {
        // 0828's avatar sample uses the historical deelement spelling.  The
        // DB synchronizer proves this exact slot as ref_deekement (index 58),
        // immediately after ref_sleep and before ref_deadlystrike.
        PARAM_INT(0xe8) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[weapon break resistance]" ||
        tag == "[deadlystrike resistance]") {
        // Same 0828 compatibility pair for DB column ref_deadlystrike
        // (index 59).  Keep the older weapon-break spelling as an alias.
        PARAM_INT(0xec) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[bleeding resistance]") {
        PARAM_INT(0xf0) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[piercing resistance]") {
        PARAM_INT(0xf4) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[stuck resistance]") {
        PARAM_INT(0xf8) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[confuse resistance]") {
        PARAM_INT(0xfc) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[hold resistance]") {
        PARAM_INT(0x100) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[armor break resistance]") {
        PARAM_INT(0x104) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[all activestatus resistance]") {
        PARAM_INT(0x108) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[inventory limit]") {
        PARAM_INT(0x10c) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[HP regen speed]") {
        PARAM_INT(0x110) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[MP regen speed]") {
        PARAM_INT(0x114) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[MP regen speed rate]") {
        PARAM_FLOAT(0x118) = ScanFloat(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[move speed]") {
        PARAM_INT(0x11c) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[x move dash speed]") {
        PARAM_FLOAT(0x120) = ScanFloat(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[y move dash speed]") {
        PARAM_FLOAT(0x124) = ScanFloat(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[attack speed]") {
        PARAM_INT(0x128) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[cast speed]") {
        PARAM_INT(0x12c) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[hit recovery]") {
        PARAM_INT(0x130) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[jump power]" || tag == "[jump]") {
        PARAM_INT(0x134) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[elemental property]") {
        std::string text;
        ScanStr(&text);
        info->values_140[0].integer = getElement(text.c_str());
        return 1;
    }
    if (tag == "[physical critical hit]") {
        PARAM_FLOAT(0x138) = ScanFloat(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[magical critical hit]") {
        PARAM_FLOAT(0x13c) = ScanFloat(static_cast<bool*>(0));
        return 1;
    }

    if (tag == "[add absolute damage]") {
        std::string text;
        ScanStr(&text);
        if (text == "[physical]") {
            info->values_1a0[0].integer = 0;
        } else if (text == "[magical]") {
            info->values_1a0[0].integer = 1;
        }
        info->values_1a0[1].real = ScanFloat(static_cast<bool*>(0));
        info->values_1a0[2].integer = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[add absolute defense]") {
        std::string text;
        ScanStr(&text);
        if (text == "[physical]") {
            info->values_1a0[3].integer = 2;
        } else if (text == "[magical]") {
            info->values_1a0[3].integer = 3;
        }
        info->values_1a0[4].real = ScanFloat(static_cast<bool*>(0));
        info->values_1a0[5].integer = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[item aura]") {
        ItemAuraInfo aura;
        aura.active_status_type = 0;
        aura.use_percent = 0;
        aura.reserved_05 = 0;
        aura.reserved_06 = 0;
        aura.reserved_07 = 0;
        aura.value = 0.0f;
        aura.duration = 0;
        std::string text;
        ScanStr(&text);
        aura.active_status_type = getEquipmentActiveStatusType(text);
        ScanStr(&text);
        aura.use_percent = text == "%";
        aura.value = ScanFloat(static_cast<bool*>(0));
        aura.duration = ScanInt(static_cast<bool*>(0));
        info->aura_infos_188.push_back(aura);
        return 1;
    }
    if (tag == "[skill data up]") {
        importSkillDataUp(info->skill_data_up_1b8);
        return 1;
    }
    if (tag == "[push aside]") {
        info->values_220[0].integer = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[lift up]") {
        info->values_220[1].integer = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[stuck]") {
        info->values_220[2].real = ScanFloat(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[ignore defense]") {
        info->value_22c = ScanInt(static_cast<bool*>(0)) != 0;
        return 1;
    }
    if (tag == "[human damage rate]") {
        info->value_230.real = ScanFloat(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[fort damage rate]") {
        info->value_234.real = ScanFloat(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[monster damage rate]") {
        info->value_238.real = ScanFloat(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[HP drain]") {
        info->value_23c = ScanInt(static_cast<bool*>(0)) != 0;
        return 1;
    }
    if (tag == "[MP drain]") {
        info->value_23d = ScanInt(static_cast<bool*>(0)) != 0;
        return 1;
    }
    if (tag == "[skill levelup]") {
        info->skill_levelups_24c.clear();
        std::string text;
        while (ScanStr(&text) == 1) {
            LevelUpSkill value;
            value.clear();
            if (text == "[common]") {
                value.skill_type = 11;
            } else {
                value.skill_type = ::getCharacterJob(text.c_str());
            }
            value.skill_id = ScanInt(static_cast<bool*>(0));
            value.skill_level = ScanInt(static_cast<bool*>(0));
            info->skill_levelups_24c.push_back(value);
        }
        return 1;
    }
    if (tag == "[avatar select ability]") {
        info->avatar_select_abilities_264.clear();
        // 0828 changed this block from the 0627 tag-only sequence to a
        // keyed table: <option key> <ability tag> <payload>.  The client
        // stores it in std::map<int, STAvatarAbilityVariation>, so duplicate
        // keys are first-wins and the 100-entry limit applies to unique keys.
        std::map<int, STAvatarAbilityVariation> parsed;
        bool next_is_type = false;
        bool next_ok = false;
        std::string next_tag;
        int option_key = ScanTypeOrInt(next_is_type, next_tag, next_ok, false);
        while (next_ok && !next_is_type) {
            if (option_key < 0 || option_key > 0xff) {
                return -1;
            }

            std::string text;
            if (ScanStr(&text) != 1) {
                return -1;
            }
            STAvatarAbilityVariation value;
            value.ability_index = getAvatarAbilityVariationIndex(text);
            value.sign_type = 0;
            value.value = 0.0f;
            value.level_up_skill.clear();
            if (value.ability_index < 0) {
                return -1;
            }
            if (value.ability_index == 43) {
                if (ScanStr(&text) != 1) {
                    return -1;
                }
                value.level_up_skill.skill_type = (text == "[common]")
                    ? oldsrv::config::kCharacterJobCommon
                    : ::getCharacterJob(text.c_str());
                if (value.level_up_skill.skill_type ==
                        oldsrv::config::kCharacterJobCommon &&
                    text != "[common]") {
                    return -1;
                }
                bool skill_ok = false;
                value.level_up_skill.skill_id = ScanInt(&skill_ok);
                if (!skill_ok) {
                    return -1;
                }
                value.level_up_skill.skill_level = ScanInt(&skill_ok);
                if (!skill_ok) {
                    return -1;
                }
            } else {
                if (ScanStr(&text) != 1) {
                    return -1;
                }
                if (text == "+") {
                    value.sign_type = 0;
                } else if (text == "*") {
                    value.sign_type = 1;
                } else {
                    return -1;
                }
                bool value_ok = false;
                value.value = ScanFloat(&value_ok);
                if (!value_ok) {
                    return -1;
                }
            }
            if (parsed.size() < 100u) {
                parsed.insert(std::make_pair(option_key, value));
            }
            option_key = ScanTypeOrInt(next_is_type, next_tag, next_ok, false);
        }

        if (!parsed.empty()) {
            const size_t extent =
                static_cast<size_t>(parsed.rbegin()->first) + 1u;
            info->avatar_select_abilities_264.resize(extent);
            for (size_t key = 0u; key < extent; ++key) {
                info->avatar_select_abilities_264[key].ability_index = -1;
            }
            for (std::map<int, STAvatarAbilityVariation>::const_iterator it =
                     parsed.begin();
                 it != parsed.end(); ++it) {
                info->avatar_select_abilities_264[
                    static_cast<size_t>(it->first)] = it->second;
            }
        }
        if (next_ok && next_is_type) {
            // ScanTypeOrInt has already consumed the first tag following the
            // keyed table.  Return it to the outer equipment grammar instead
            // of asking ScanType to skip past its payload.
            tag = next_tag;
            return 2;
        }
        return 1;
    }
    if (tag == "[room list move speed rate]") {
        info->value_274.real = ScanFloat(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[anti evil]") {
        PARAM_INT(0x280) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[rigidity]") {
        PARAM_INT(0x284) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[fire attack]") {
        PARAM_INT(0x288) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[water attack]") {
        PARAM_INT(0x28c) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[dark attack]") {
        PARAM_INT(0x290) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[light attack]") {
        PARAM_INT(0x294) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[all elemental attack]") {
        PARAM_INT(0x298) = ScanInt(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[active status]") {
        std::string text;
        ScanStr(&text);
        info->values_140[1].integer = getEquipmentActiveStatusType(text);
        info->values_140[2].real = ScanFloat(static_cast<bool*>(0));
        info->values_140[3].integer = ScanInt(static_cast<bool*>(0));
        info->value_150 = ScanInt(static_cast<bool*>(0)) != 0;
        info->value_154.integer = ScanInt(static_cast<bool*>(0));
        scanEquipmentActiveStatusTail(text, info->values_158);
        return 1;
    }
    if (tag == "[attacked active status]") {
        std::string text;
        ScanStr(&text);
        info->value_168.integer = getEquipmentActiveStatusType(text);
        info->value_16c.real = ScanFloat(static_cast<bool*>(0));
        info->value_170.integer = ScanInt(static_cast<bool*>(0));
        info->value_174 = ScanInt(static_cast<bool*>(0)) != 0;
        info->value_178.integer = ScanInt(static_cast<bool*>(0));
        scanEquipmentActiveStatusTail(text, info->values_17c);
        return 1;
    }
    if (tag == "[active status probability]" ||
        tag == "[active status level]") {
        std::string text;
        ScanStr(&text);
        const int status = getActiveStatusToEnumValue(text);
        const int value = ScanInt(static_cast<bool*>(0));
        const int base_index =
            tag == "[active status probability]" ? 167 : 185;
        if (status == 19) {
            for (int i = 0; i <= 17; ++i) {
                PARAM_INT(4 * (base_index + i)) = value;
            }
        } else if (status != 18 && status <= 17) {
            PARAM_INT(4 * (base_index + status)) = value;
        }
        return 1;
    }
    if (tag == "[active status control info]") {
        std::string text;
        ScanStr(&text);
        const int status = getActiveStatusToEnumValue(text);
        const int probability = ScanInt(static_cast<bool*>(0));
        const int level = ScanInt(static_cast<bool*>(0));
        const int control = ScanInt(static_cast<bool*>(0));
        if (status == 19) {
            for (int i = 0; i <= 17; ++i) {
                PARAM_INT(4 * (167 + i)) = probability;
                PARAM_INT(4 * (185 + i)) = level;
                PARAM_INT(4 * (203 + i)) = control;
            }
        } else if (status != 18 && status <= 17) {
            PARAM_INT(4 * (167 + status)) = probability;
            PARAM_INT(4 * (185 + status)) = level;
            PARAM_INT(4 * (203 + status)) = control;
            switch (status) {
                case 0:
                    ScanInt(static_cast<bool*>(0));
                    ScanInt(static_cast<bool*>(0));
                    break;
                case 2:
                    PARAM_INT(0x374) = ScanInt(static_cast<bool*>(0));
                    break;
                case 4:
                    ScanInt(static_cast<bool*>(0));
                    break;
                case 5:
                    ScanInt(static_cast<bool*>(0));
                    ScanInt(static_cast<bool*>(0));
                    break;
                case 6:
                    PARAM_INT(0x378) = ScanInt(static_cast<bool*>(0));
                    break;
                case 7:
                    PARAM_INT(0x37c) = ScanInt(static_cast<bool*>(0));
                    break;
                case 9:
                    PARAM_INT(0x380) = ScanInt(static_cast<bool*>(0));
                    ScanInt(static_cast<bool*>(0));
                    ScanInt(static_cast<bool*>(0));
                    break;
                case 10:
                    ScanInt(static_cast<bool*>(0));
                    break;
                case 11:
                    PARAM_INT(0x384) = ScanInt(static_cast<bool*>(0));
                    break;
                case 17:
                    ScanInt(static_cast<bool*>(0));
                    break;
                default:
                    break;
            }
        }
        return 1;
    }
    if (tag == "[account bind linear ability]") {
        return 1;
    }
    if (tag == "[level linear ability]") {
        bool success = false;
        const int level = ScanInt(&success);
        if (!success) {
            return 0;
        }
        PARAM_INT(0x28) = level;
        PARAM_INT(0x2c) = ScanInt(static_cast<bool*>(0));
        std::string inner;
        while (ScanType(inner, true) == 1) {
            if (inner == "[physical attack]") {
                for (int i = 0; i <= 1; ++i) {
                    PARAM_INT(4 * (12 + i)) =
                        ScanInt(static_cast<bool*>(0));
                }
            } else if (inner == "[magical attack]") {
                for (int i = 0; i <= 1; ++i) {
                    PARAM_INT(4 * (16 + i)) =
                        ScanInt(static_cast<bool*>(0));
                }
            } else if (inner == "[physical defense]") {
                for (int i = 0; i <= 1; ++i) {
                    PARAM_INT(4 * (14 + i)) =
                        ScanInt(static_cast<bool*>(0));
                }
            } else if (inner == "[magical defense]") {
                for (int i = 0; i <= 1; ++i) {
                    PARAM_INT(4 * (18 + i)) =
                        ScanInt(static_cast<bool*>(0));
                }
            } else if (inner == "[equipment physical attack]") {
                for (int i = 0; i <= 3; ++i) {
                    PARAM_INT(4 * (232 + i)) =
                        ScanInt(static_cast<bool*>(0));
                }
            } else if (inner == "[equipment magical attack]") {
                for (int i = 0; i <= 3; ++i) {
                    PARAM_INT(4 * (240 + i)) =
                        ScanInt(static_cast<bool*>(0));
                }
            } else if (inner == "[equipment physical defense]") {
                for (int i = 0; i <= 3; ++i) {
                    PARAM_INT(4 * (236 + i)) =
                        ScanInt(static_cast<bool*>(0));
                }
            } else if (inner == "[equipment magical defense]") {
                for (int i = 0; i <= 3; ++i) {
                    PARAM_INT(4 * (244 + i)) =
                        ScanInt(static_cast<bool*>(0));
                }
            } else if (inner == "[grade]") {
                for (int i = 0; i <= 1; ++i) {
                    PARAM_INT(4 * (248 + i)) =
                        ScanInt(static_cast<bool*>(0));
                }
            } else if (inner == "[separate attack]") {
                for (int i = 0; i <= 3; ++i) {
                    PARAM_INT(4 * (250 + i)) =
                        ScanInt(static_cast<bool*>(0));
                }
            } else if (inner == "[separate rate]") {
                for (int i = 0; i <= 3; ++i) {
                    PARAM_FLOAT(4 * (254 + i)) =
                        ScanFloat(static_cast<bool*>(0));
                }
            } else if (inner == "[repair price]") {
                for (int i = 0; i <= 1; ++i) {
                    PARAM_INT(4 * (258 + i)) =
                        ScanInt(static_cast<bool*>(0));
                }
            } else if (inner == "[/level linear ability]") {
                break;
            }
        }
        return 1;
    }
    if (tag == "[item overpower part]") {
        info->item_overpower_ready_425 = 1;
        return 1;
    }
    if (tag == "[all skill item]") {
        return importAllSkillItem(info);
    }
    if (tag == "[item overpower desc]") {
        return importItemOverpowerDesc(info);
    }
    if (tag == "[ice appendage]") {
        EquipmentParameterInfo::EquipmentAuraAppendageData data;
        data.type = 0;
        data.value_04 = ScanInt(static_cast<bool*>(0));
        data.value_08 = ScanInt(static_cast<bool*>(0));
        data.value_0c = ScanInt(static_cast<bool*>(0));
        data.value_10 = 0;
        info->aura_appendages_200.push_back(data);
        return 1;
    }
    if (tag == "[curse appendage]") {
        EquipmentParameterInfo::EquipmentAuraAppendageData data;
        data.type = 1;
        data.value_04 = ScanInt(static_cast<bool*>(0));
        data.value_08 = ScanInt(static_cast<bool*>(0));
        data.value_0c = ScanInt(static_cast<bool*>(0));
        data.value_10 = ScanInt(static_cast<bool*>(0));
        info->aura_appendages_200.push_back(data);
        return 1;
    }
    if (tag == "[upgrade cost discount]") {
        info->values_444[0].real = ScanFloat(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[upgrade prob increase]") {
        info->values_444[1].real = ScanFloat(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[assault cost discount]") {
        info->values_444[2].real = ScanFloat(static_cast<bool*>(0));
        return 1;
    }
    if (tag == "[decrease critical damage]") {
        std::string text;
        ScanStr(&text);
        const float value =
            static_cast<float>(ScanInt(static_cast<bool*>(0)));
        if (text == "%") {
            info->value_20c.real = value / 100.0f;
        } else {
            info->value_210.real = value;
        }
        return 1;
    }
    if (tag == "[hardcoding parameter]") {
        bool success = false;
        int value = ScanInt(&success);
        while (success) {
            info->hardcoding_parameters_214.push_back(value);
            value = ScanInt(&success);
        }
        return 1;
    }
    if (tag == "[rebirth]") {
        std::string inner;
        while (ScanType(inner, true) == 1) {
            if (inner == "[probability]") {
                info->rebirth_450.probability =
                    ScanInt(static_cast<bool*>(0));
            } else if (inner == "[consume item]") {
                info->rebirth_450.consume_item =
                    ScanInt(static_cast<bool*>(0));
                info->rebirth_450.consume_item_count =
                    ScanInt(static_cast<bool*>(0));
            } else if (inner == "[hp recovery]") {
                info->rebirth_450.hp_recovery =
                    ScanInt(static_cast<bool*>(0));
            } else if (inner == "[mp recovery]") {
                info->rebirth_450.mp_recovery =
                    ScanInt(static_cast<bool*>(0));
            } else if (inner == "[cooltime]") {
                info->rebirth_450.cooltime =
                    ScanInt(static_cast<bool*>(0));
            } else {
                break;
            }
        }
        return 1;
    }
    if (tag == "[potion physical attack]" ||
        tag == "[potion magical attack]") {
        return 1;
    }
    if (tag == "[variable stat]") {
        info->variable_stats_474.clear();
        VariableStat value;
        while (scanVariableStat(value)) {
            info->variable_stats_474.push_back(value);
        }
        return 1;
    }

    if (tag == "[if]") {
        ++first;
        scanIf(info, 0);
        return 1;
    }
    if (tag == "[then]") {
        if (first <= second) {
            first = -first;
            second = -second;
            return 0;
        }
        ++second;
        return scanThen(std::string(filename ? filename : ""), info, 0);
    }
    if (tag == "[multiple then]") {
        std::string child_tag;
        while (ScanType(child_tag, true) == 1 &&
               child_tag != "[/multiple then]") {
            if (child_tag == "[then]" &&
                scanThen(std::string(filename ? filename : ""), info, 0) != 1) {
                return 0;
            }
        }
        ++second;
        return 1;
    }

#undef PARAM_FLOAT
#undef PARAM_INT
#undef PARAM_HIGH_WORD
#undef PARAM_LOW_WORD
    return 0;
}

// IDA_EA: 0x89BF67F
// IDA_NAME: _Z21scanRandomOptionValueP22EquipmentParameterInfoRSs
// DEMANGLED: scanRandomOptionValue(EquipmentParameterInfo *,std::string &)
int scanRandomOptionValue(EquipmentParameterInfo* info, std::string& tag) {
    if (info == 0) {
        return 1;
    }

    int first = 0;
    int second = 0;
    return importEquipmentParameterInfo(info, tag, 0, first, second);
}

// IDA_EA: 0x8A97610
// IDA_NAME: _Z14isEndModuleTagPSsi
// DEMANGLED: isEndModuleTag(std::string *,int)
int isEndModuleTag(std::string* tag, int module_index) {
    return tag != 0 && module_index >= 0 && module_index <= 16 &&
           *tag == g_module_type_end_tags[module_index];
}

// IDA_EA: 0x8A97637
// IDA_NAME: _Z19processModuleTagSetRiS_PSsib
// DEMANGLED: processModuleTagSet(int &,int &,std::string *,int,bool)
int processModuleTagSet(int& tag_index,
                        int& parsed_module_type,
                        std::string* tag,
                        int module_type,
                        bool parse_equipment) {
    (void)module_type;
    const int current_module_type = 3;
    tag_index = -1;
    parsed_module_type = -1;
    if (tag == 0) {
        return 1;
    }

    bool is_end_tag = false;
    for (int index = 0; index <= 16; ++index) {
        if (*tag == g_module_type_start_tags[index] ||
            isEndModuleTag(tag, index)) {
            tag_index = index;
            is_end_tag = isEndModuleTag(tag, index) != 0;
            if (index != current_module_type &&
                std::strcmp(g_module_type_start_tags[index],
                            g_module_type_start_tags[current_module_type]) == 0) {
                tag_index = current_module_type;
            }
            break;
        }
    }

    if (tag_index == current_module_type) {
        if (!is_end_tag) {
            parsed_module_type = current_module_type;
        }
    } else if (tag_index != -1) {
        if (std::strcmp(g_module_type_start_tags[tag_index],
                        "[dungeon]") == 0) {
            if (!is_end_tag) {
                parsed_module_type = current_module_type;
                LogManager::logFormat(
                    0,
                    "../RDARScriptSkill.cpp",
                    "processModuleTagSet",
                    0xC0,
                    "skillscript separating : \xBC\xBA\xB0\xF8 : "
                    "\xB1\xE2\xBA\xBB\xB0\xAA \xB8\xF0\xB5\xE2\xC5\xC2"
                    "\xB1\xD7\xC0\xCC\xB9\xC7\xB7\xCE \xC1\xF8\xC7\xE0 tag %s",
                    tag->c_str());
            }
        } else {
            bool reached_end_tag = false;
            do {
                const int scan_result = ScanType(*tag, false);
                if (isEndModuleTag(tag, tag_index)) {
                    reached_end_tag = true;
                    continue;
                }

                const DNFLEX_TOKEN token = g_dnfLex.getLastToken();
                if (token == DNF_TOK_EOF || token == DNF_TOK_ERROR ||
                    (scan_result != 1 && token != DNF_TOK_TYPE)) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptSkill.cpp",
                        "processModuleTagSet",
                        0xC0,
                        "module closing tag missing: module=%d line=%d "
                        "token='%s'",
                        tag_index,
                        g_dnfLex.getLastLineNo(),
                        g_dnfLex.getLastTokenDataAsString().c_str());
                    return 0;
                }
            } while (!reached_end_tag);
            return 0;
        }
    }

    if (is_end_tag) {
        return 0;
    }
    if (!parse_equipment && tag_index != -1) {
        ScanType(*tag, false);
    }
    return 1;
}
// IDA_EA: 0x89C1CB9
// IDA_NAME: _Z24setEquipmentAniScriptLayPKSt3mapISs20STEquipmentAniScriptSt4lessISsESaISt4pairIKSsS0_EEEPS0_PSsiSB_P17STEquipmentScript17ENUM_CHARACTERJOBN16equipmentAniType1TE
// DEMANGLED: setEquipmentAniScriptLay(std::map<std::string,STEquipmentAniScript,std::less<std::string>,std::allocator<std::pair<std::string const,STEquipmentAniScript>>> const*,STEquipmentAniScript*,std::string *,int,std::string *,STEquipmentScript *,ENUM_CHARACTERJOB,equipmentAniType::T)
bool setEquipmentAniScriptLay(
    const std::map<std::string, STEquipmentAniScript>* templates,
    STEquipmentAniScript* destination,
    std::string* template_name,
    int layer,
    std::string* path,
    STEquipmentScript* script,
    int job,
    int animation_type)
    __asm__("_Z24setEquipmentAniScriptLayPKSt3mapISs20STEquipmentAniScriptSt4lessISsESaISt4pairIKSsS0_EEEPS0_PSsiSB_P17STEquipmentScript17ENUM_CHARACTERJOBN16equipmentAniType1TE");
bool setEquipmentAniScriptLay(
    const std::map<std::string, STEquipmentAniScript>* templates,
    STEquipmentAniScript* destination,
    std::string* template_name,
    int layer,
    std::string* path,
    STEquipmentScript* script,
    int job,
    int animation_type) {
    char formatted[512];
    std::memset(formatted, 0, sizeof(formatted));

    const STEquipmentAniScript& source =
        templates->find(*template_name)->second;
    if (destination->layer != 0) {
        EquipmentAniInfoScript* ani_info =
            script->addAniInfo(static_cast<ENUM_CHARACTERJOB>(job));
        if (animation_type == 2) {
            if (!ani_info->layer_lists.empty()) {
                ani_info->layer_lists.back().push_back(*destination);
            }
        } else if (animation_type == 1) {
            ani_info->layer_ani.push_back(*destination);
        } else {
            ani_info->base_ani.push_back(*destination);
        }
    }

    destination->clear();
    destination->layer = layer != 0 ? layer : source.layer;
    destination->reserved_motion = *path;

    std::snprintf(formatted, sizeof(formatted), source.waiting_motion.c_str(),
                  path->c_str());
    destination->waiting_motion = formatted;
    std::snprintf(formatted, sizeof(formatted), source.move_motion.c_str(),
                  path->c_str());
    destination->move_motion = formatted;
    std::snprintf(formatted, sizeof(formatted), source.sit_motion.c_str(),
                  path->c_str());
    destination->sit_motion = formatted;
    for (int index = 0; index < 2; ++index) {
        std::snprintf(formatted, sizeof(formatted),
                      source.damage_motion[index].c_str(), path->c_str());
        destination->damage_motion[index] = formatted;
    }
    std::snprintf(formatted, sizeof(formatted), source.down_motion.c_str(),
                  path->c_str());
    destination->down_motion = formatted;
    std::snprintf(formatted, sizeof(formatted), source.overturn_motion.c_str(),
                  path->c_str());
    destination->overturn_motion = formatted;
    std::snprintf(formatted, sizeof(formatted), source.jump_motion.c_str(),
                  path->c_str());
    destination->jump_motion = formatted;
    std::snprintf(formatted, sizeof(formatted),
                  source.jumpattack_motion.c_str(), path->c_str());
    destination->jumpattack_motion = formatted;
    std::snprintf(formatted, sizeof(formatted), source.rest_motion.c_str(),
                  path->c_str());
    destination->rest_motion = formatted;
    for (int index = 0; index < 4; ++index) {
        std::snprintf(formatted, sizeof(formatted),
                      source.throw_motion_first[index].c_str(), path->c_str());
        destination->throw_motion_first[index] = formatted;
        std::snprintf(formatted, sizeof(formatted),
                      source.throw_motion_second[index].c_str(), path->c_str());
        destination->throw_motion_second[index] = formatted;
    }
    std::snprintf(formatted, sizeof(formatted), source.dash_motion.c_str(),
                  path->c_str());
    destination->dash_motion = formatted;
    std::snprintf(formatted, sizeof(formatted), source.dashattack_motion.c_str(),
                  path->c_str());
    destination->dashattack_motion = formatted;
    std::snprintf(formatted, sizeof(formatted), source.getitem_motion.c_str(),
                  path->c_str());
    destination->getitem_motion = formatted;
    std::snprintf(formatted, sizeof(formatted), source.buff_motion.c_str(),
                  path->c_str());
    destination->buff_motion = formatted;
    std::snprintf(formatted, sizeof(formatted), source.back_motion.c_str(),
                  path->c_str());
    destination->back_motion = formatted;

    for (std::vector<std::string>::const_iterator it =
             source.attack_motion.begin();
         it != source.attack_motion.end(); ++it) {
        std::snprintf(formatted, sizeof(formatted), it->c_str(), path->c_str());
        destination->attack_motion.push_back(formatted);
    }
    for (std::vector<std::string>::const_iterator it =
             source.etc_motion.begin();
         it != source.etc_motion.end(); ++it) {
        std::snprintf(formatted, sizeof(formatted), it->c_str(), path->c_str());
        destination->etc_motion.push_back(formatted);
    }
    return false;
}
// IDA_EA: 0x88C1A0D
// IDA_NAME: _Z12importEffectRN6effect8STEffectE
// DEMANGLED: importEffect(effect::STEffect &)
int importEffect(effect::STEffect& effect)
    __asm__("_Z12importEffectRN6effect8STEffectE");
int importEffect(effect::STEffect& effect) {
    std::string value;
    std::string tag;
    while (true) {
        ScanType(tag, true);
        if (tag == "[attach target]") {
            ScanStr(&value);
            if (value == "delegator") {
                effect.attach_target = 0;
            } else if (value == "target") {
                effect.attach_target = 1;
            }
        } else if (tag == "[type]") {
            ScanStr(&value);
            if (value == "animation") {
                effect.type = 0;
            } else if (value == "particle") {
                effect.type = 1;
            }
        } else if (tag == "[attach pos]") {
            ScanStr(&value);
            if (value == "bottom") {
                effect.attach_position = 2;
            } else if (value == "middle") {
                effect.attach_position = 1;
            } else if (value == "top") {
                effect.attach_position = 0;
            }
        } else if (tag == "[z revision]") {
            effect.z_revision = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[index]") {
            effect.index = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[option]") {
            while (ScanStr(&value) == 1) {
                if (value == "keep direction") {
                    effect.options |= 1u;
                } else if (value == "floor") {
                    effect.options |= 2u;
                } else if (value == "auto remove") {
                    effect.options |= 4u;
                } else if (value == "apply custom color") {
                    effect.options |= 8u;
                } else if (value == "sync speed") {
                    effect.options |= 0x10u;
                } else if (value == "copy horizon") {
                    effect.options |= 0x20u;
                } else if (value == "copy vertical") {
                    effect.options |= 0x40u;
                } else if (value == "resize") {
                    effect.options |= 0x80u;
                }
            }
        } else if (tag == "[module]") {
            effect.module_type = scanModuleType();
        } else if (tag == "[/module]") {
            continue;
        } else if (tag == "[file name]") {
            ScanStr(&effect.effect_name);
        } else {
            break;
        }
    }
    return 1;
}

static bool CommitAuraPositionRecord(
    STEquipmentScript::AuraPositionData* current,
    bool* has_current,
    std::vector<STEquipmentScript::AuraPositionData>* parsed) {
    if (!current || !has_current || !parsed) return false;
    if (!*has_current) return true;
    if (current->value_count < 2 || current->value_count > 3 ||
        current->character_job < 0 ||
        current->character_job >= oldsrv::config::kCharacterJobCount) {
        return false;
    }
    parsed->push_back(*current);
    *current = STEquipmentScript::AuraPositionData();
    *has_current = false;
    return true;
}

// CLIENT_EA: ImportEquipmentScript @0x1576B27.  This is client presentation
// state, but its full official record must be consumed and validated so that
// the following [aura ability] tag cannot disappear behind scalar recovery.
static bool ImportAuraPositionData(STEquipmentScript* script) {
    if (!script) return false;

    bool success = false;
    const int mode = ScanInt(&success);
    // ImportEquipmentScript @0x1576B27 forwards the integer unchanged to the
    // aura-position reader; no enum/range check is present in the evidence.
    if (!success) return false;

    std::vector<STEquipmentScript::AuraPositionData> parsed;
    STEquipmentScript::AuraPositionData current;
    bool has_current = false;
    while (true) {
        int int_value = 0;
        float float_value = 0.0f;
        std::string text;
        const DNFLEX_TOKEN token =
            ScanAll(&int_value, &float_value, &text);
        if (token == DNF_TOK_TYPE) {
            if (text != "[/aura pos datas]" ||
                !CommitAuraPositionRecord(&current, &has_current, &parsed) ||
                parsed.empty()) {
                return false;
            }
            script->aura_position_mode = mode;
            script->aura_position_data.swap(parsed);
            return true;
        }
        if (token == DNF_TOK_STRING) {
            if (!CommitAuraPositionRecord(&current, &has_current, &parsed)) {
                return false;
            }
            const int job = ::getCharacterJob(text.c_str());
            if (job < 0 || job >= oldsrv::config::kCharacterJobCount) {
                return false;
            }
            current.character_job = job;
            has_current = true;
            continue;
        }
        if (token == DNF_TOK_DECIMAL || token == DNF_TOK_HEXADECIMAL) {
            if (!has_current || current.value_count >= 3) return false;
            if (current.value_count == 0) {
                current.x = int_value;
            } else if (current.value_count == 1) {
                current.y = int_value;
            } else {
                current.variant = int_value;
            }
            ++current.value_count;
            continue;
        }
        return false;
    }
}
// IDA_EA: 0x8991AEB
// IDA_NAME: _Z21ImportEquipmentScriptP17STEquipmentScriptPKcbii
// DEMANGLED: ImportEquipmentScript(STEquipmentScript *,char const*,bool,int,int)
int ImportEquipmentScript(STEquipmentScript* script,
                          const char* filename,
                          bool unknown,
                          int module_type,
                          int set_id) {
    if (script == 0 || filename == 0) {
        return 0;
    }

    if (loadRDARScriptFile(g_szEquipmentScriptBaseDirectory,
                                   filename) != 1) {
        return 0;
    }

#define EQUIP_FAIL()                                                        \
    do {                                                                    \
        std::fprintf(                                                       \
            stderr,                                                        \
            "[equipment-parser][IMPORT_FAIL] file=%s tag=%s line=%d "     \
            "token=%s\n",                                                \
            filename,                                                      \
            tag.c_str(),                                                   \
            g_dnfLex.getLastLineNo(),                                      \
            g_dnfLex.getLastTokenDataAsString().c_str());                  \
        unloadRDARScriptFile();                                             \
        return 0;                                                           \
    } while (false)

    script->Clear();
    getDirectoryPath(script->directory_664,
                             "%s%s",
                             g_szEquipmentScriptBaseDirectory,
                             filename);

    EquipmentParameterInfo* active_parameter = &script->parameter_1c0;
    bool active_piece_set_scope = false;
    // IDA locals v336/v335 start at zero and are reset for each parameter
    // scope.  They are parser state, not an unsupported-tag sentinel.
    int parameter_first = 0;
    int parameter_second = 0;

    int animation_job = oldsrv::config::kCharacterJobInvalid;
    bool expand_animation = false;
    int level_section_animation = -1;
    int normal_layer_variation = 0;
    int expand_layer_variation = 0;
    int level_layer_variation = 0;
    std::string normal_animation_path;
    std::string expand_animation_path;
    std::string level_animation_path;
    STEquipmentAniScript normal_animation;
    STEquipmentAniScript expand_animation_script;
    STEquipmentAniScript level_animation;
    GrowthEquipmentInfo level_growth_info;

    std::string tag;
    bool has_buffered_tag = false;
    while (has_buffered_tag || ScanType(tag, true) == 1) {
        has_buffered_tag = false;
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
        } else if (tag == "[dictionary text]") {
            // CLIENT_EA: ImportEquipmentScript_ReadDictionaryText
            // @0x1574290.
            ScanStr(&script->dictionary_text);
        } else if (tag == "[basic explain]") {
            ScanStr(&script->strings_6a4[0]);
        } else if (tag == "[detail explain]") {
            ScanStr(&script->strings_6a4[1]);
        } else if (tag == "[parameter basic explain]") {
            ScanStr(&script->parameter_1c0.value_278);
        } else if (tag == "[parameter detail explain]") {
            ScanStr(&script->parameter_1c0.value_27c);
        } else if (tag == "[attack info]") {
            ScanStr(&script->strings_6a4[5]);
        } else if (tag == "[fullset explain]") {
            ScanStr(&script->strings_6a4[2]);
        } else if (tag == "[fullset basic explain]") {
            ScanStr(&script->strings_6a4[3]);
        } else if (tag == "[fullset detail explain]") {
            ScanStr(&script->strings_6a4[4]);
        } else if (tag == "[set name]") {
            ScanStr(&script->set_name_668);
        } else if (tag == "[grade]") {
            script->item.grade = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[equipment grade]") {
            // CLIENT_EA: ImportEquipmentScript_SetEquipmentGradeTextId
            // @0x157B0F1.
            std::string grade_name;
            if (ScanStr(&grade_name) != 1) {
                EQUIP_FAIL();
            }
            script->equipment_grade_text_id =
                GetEquipmentGradeTextIdByName(grade_name.c_str());
        } else if (tag == "[spectrum]") {
            const int r = ScanInt(static_cast<bool*>(0));
            const int g = ScanInt(static_cast<bool*>(0));
            const int b = ScanInt(static_cast<bool*>(0));
            const int a = ScanInt(static_cast<bool*>(0));
            script->value_790 = ((a & 0xff) << 24) |
                                ((b & 0xff) << 16) |
                                ((g & 0xff) << 8) | (r & 0xff);
            script->value_794 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[spectrum avatar]") {
            // CLIENT_EA: ImportEquipmentScript_ReadSpectrumAvatarParameters
            // @0x1574404.
            const int r = ScanInt(static_cast<bool*>(0));
            const int g = ScanInt(static_cast<bool*>(0));
            const int b = ScanInt(static_cast<bool*>(0));
            const int a = ScanInt(static_cast<bool*>(0));
            script->spectrum_avatar_rgba =
                (static_cast<uint32_t>(a) & 0xffu) << 24 |
                (static_cast<uint32_t>(b) & 0xffu) << 16 |
                (static_cast<uint32_t>(g) & 0xffu) << 8 |
                (static_cast<uint32_t>(r) & 0xffu);
            script->spectrum_avatar_parameter_5 =
                ScanInt(static_cast<bool*>(0));
            script->spectrum_avatar_parameter_6 =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[clear avatar]") {
            script->clear_avatar_7ac =
                ScanInt(static_cast<bool*>(0)) == 1;
        } else if (tag == "[attach type]") {
            std::string text;
            ScanStr(&text);
            if (text == "[free]") {
                script->item.values_08[0] = 0;
            } else if (text == "[trade]") {
                script->item.values_08[0] = 1;
            } else if (text == "[trade delete]") {
                script->item.values_08[0] = 2;
            } else if (text == "[sealing]") {
                script->item.values_08[0] = 3;
            } else if (text == "[sealing trade]") {
                script->item.values_08[0] = 4;
            } else if (text == "[account]") {
                script->item.values_08[0] = 5;
            }
        } else if (tag == "[creation rate]") {
            script->item.values_08[1] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[rarity]") {
            script->item.values_08[2] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[weight]") {
            script->item.value_34 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[icon]") {
            ScanStr(&script->item.icon);
            script->item.icon_index = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[aura hud icon]") {
            script->values_178[4] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[tarotcard view]") {
            ScanStr(&script->item.tarotcard_view);
            script->item.tarotcard_view_index =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[extra icon]") {
            std::string text;
            ScanStr(&text);
            script->strings_18c.push_back(text);
            script->values_198.push_back(
                ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[field image]") {
            ScanStr(&script->item.field_image);
            script->item.field_image_index =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[field animation]") {
            ScanStr(&script->string_1a4);
        } else if (tag == "[custom animation]") {
            std::vector<std::string>* animations = &script->strings_1a8;
            animations->clear();
            std::string text;
            while (ScanStr(&text) == 1) {
                animations->push_back(text);
            }
        } else if (tag == "[expand ani]") {
            EquipmentAniInfoScript* ani_info =
                script->addAniInfo(
                    static_cast<ENUM_CHARACTERJOB>(animation_job));
            expand_animation = true;
            ani_info->unknown_24 = ScanInt(static_cast<bool*>(0));
            ani_info->unknown_28 = ScanInt(static_cast<bool*>(0));
            ani_info->unknown_2C = ScanInt(static_cast<bool*>(0));
            ani_info->unknown_38 = ScanInt(static_cast<bool*>(0));
            ani_info->unknown_14 = 1;
        } else if (tag == "[/expand ani]") {
            expand_animation = false;
        } else if (tag == "[expand path]") {
            ScanStr(&script->addAniInfo(
                         static_cast<ENUM_CHARACTERJOB>(animation_job))
                         ->unknown_string_4C);
        } else if (tag == "[level section ani]") {
            bool success = false;
            const int item_index = ScanInt(&success);
            std::string job_name;
            if (success && ScanStr(&job_name) == 1) {
                animation_job = ::getCharacterJob(job_name.c_str());
                level_growth_info.clear();
                level_growth_info.item_index = item_index;
                level_growth_info.equipment_slot = script->values_1b4[0];
                level_section_animation = item_index;
                script->value_69c = 1;
                script->addAniInfo(
                    static_cast<ENUM_CHARACTERJOB>(animation_job))
                    ->layer_lists.push_back(std::vector<STEquipmentAniScript>());
            }
        } else if (tag == "[/level section ani]") {
            EquipmentAniInfoScript* ani_info =
                script->addAniInfo(
                    static_cast<ENUM_CHARACTERJOB>(animation_job));
            if (level_animation.layer != 0) {
                if (!ani_info->layer_lists.empty()) {
                    ani_info->layer_lists.back().push_back(level_animation);
                }
                level_animation.clear();
            }
            ani_info->growth_infos.push_back(level_growth_info);
            level_growth_info.clear();
            level_section_animation = -1;
        } else if (tag == "[level section path]") {
            ScanStr(&level_growth_info.ani_name);
        } else if (tag == "[layer variation]") {
            if (expand_animation) {
                expand_layer_variation = ScanInt(static_cast<bool*>(0));
                ScanStr(&expand_animation_path);
            } else if (level_section_animation == -1) {
                normal_layer_variation = ScanInt(static_cast<bool*>(0));
                ScanStr(&normal_animation_path);
            } else {
                bool success = false;
                const int layer = ScanInt(&success);
                if (success) {
                    ScanStr(&level_animation_path);
                    level_layer_variation = layer;
                }
            }
        } else if (tag == "[equipment ani script]") {
            std::string animation_name;
            ScanStr(&animation_name);
            const std::map<std::string, STEquipmentAniScript>* templates =
                getMapEquipmentAniScript();
            if (expand_animation) {
                setEquipmentAniScriptLay(templates, &expand_animation_script,
                                         &animation_name, expand_layer_variation,
                                         &expand_animation_path, script,
                                         animation_job, 1);
            } else if (level_section_animation == -1) {
                setEquipmentAniScriptLay(templates, &normal_animation,
                                         &animation_name, normal_layer_variation,
                                         &normal_animation_path, script,
                                         animation_job, 0);
            } else {
                setEquipmentAniScriptLay(templates, &level_animation,
                                         &animation_name, level_layer_variation,
                                         &level_animation_path, script,
                                         animation_job, 2);
            }
        } else if (tag == "[LAYER]") {
            const int parsed_layer = ScanInt(static_cast<bool*>(0));
            EquipmentAniInfoScript* ani_info =
                script->addAniInfo(
                    static_cast<ENUM_CHARACTERJOB>(animation_job));
            if (expand_animation) {
                if (expand_animation_script.layer != 0) {
                    ani_info->layer_ani.push_back(expand_animation_script);
                }
                expand_animation_script.clear();
                expand_animation_script.layer = expand_layer_variation != 0
                    ? expand_layer_variation : parsed_layer;
                expand_animation_script.reserved_motion = expand_animation_path;
            } else if (level_section_animation == -1) {
                if (normal_animation.layer != 0) {
                    ani_info->base_ani.push_back(normal_animation);
                }
                normal_animation.clear();
                normal_animation.layer = normal_layer_variation != 0
                    ? normal_layer_variation : parsed_layer;
                normal_animation.reserved_motion = normal_animation_path;
            } else {
                if (level_animation.layer != 0 &&
                    !ani_info->layer_lists.empty()) {
                    ani_info->layer_lists.back().push_back(level_animation);
                }
                level_animation.clear();
                level_animation.layer = level_layer_variation != 0
                    ? level_layer_variation : parsed_layer;
                level_animation.reserved_motion = level_animation_path;
            }
        } else if (tag == "[waiting motion]" || tag == "[move motion]" ||
                   tag == "[sit motion]" || tag == "[damage motion 1]" ||
                   tag == "[damage motion 2]" || tag == "[down motion]" ||
                   tag == "[overturn motion]" || tag == "[jump motion]" ||
                   tag == "[jumpattack motion]" || tag == "[rest motion]" ||
                   tag == "[throw motion 1-1]" ||
                   tag == "[throw motion 1-2]" ||
                   tag == "[throw motion 2-1]" ||
                   tag == "[throw motion 2-2]" ||
                   tag == "[throw motion 3-1]" ||
                   tag == "[throw motion 3-2]" ||
                   tag == "[throw motion 4-1]" ||
                   tag == "[throw motion 4-2]" || tag == "[dash motion]" ||
                   tag == "[dashattack motion]" || tag == "[getitem motion]" ||
                   tag == "[buff motion]" || tag == "[back motion]") {
            std::string motion_format;
            ScanStr(&motion_format);
            STEquipmentAniScript* animation = &normal_animation;
            const std::string* path = &normal_animation_path;
            if (expand_animation) {
                animation = &expand_animation_script;
                path = &expand_animation_path;
            } else if (level_section_animation != -1) {
                animation = &level_animation;
                path = &level_animation_path;
            }
            std::string* motion = 0;
            if (tag == "[waiting motion]") {
                motion = &animation->waiting_motion;
            } else if (tag == "[move motion]") {
                motion = &animation->move_motion;
            } else if (tag == "[sit motion]") {
                motion = &animation->sit_motion;
            } else if (tag == "[damage motion 1]") {
                motion = &animation->damage_motion[0];
            } else if (tag == "[damage motion 2]") {
                motion = &animation->damage_motion[1];
            } else if (tag == "[down motion]") {
                motion = &animation->down_motion;
            } else if (tag == "[overturn motion]") {
                motion = &animation->overturn_motion;
            } else if (tag == "[jump motion]") {
                motion = &animation->jump_motion;
            } else if (tag == "[jumpattack motion]") {
                motion = &animation->jumpattack_motion;
            } else if (tag == "[rest motion]") {
                motion = &animation->rest_motion;
            } else if (tag == "[throw motion 1-1]") {
                motion = &animation->throw_motion_first[0];
            } else if (tag == "[throw motion 1-2]") {
                motion = &animation->throw_motion_first[1];
            } else if (tag == "[throw motion 2-1]") {
                motion = &animation->throw_motion_second[0];
            } else if (tag == "[throw motion 2-2]") {
                motion = &animation->throw_motion_second[1];
            } else if (tag == "[throw motion 3-1]") {
                motion = &animation->throw_motion_first[2];
            } else if (tag == "[throw motion 3-2]") {
                motion = &animation->throw_motion_second[2];
            } else if (tag == "[throw motion 4-1]") {
                motion = &animation->throw_motion_first[3];
            } else if (tag == "[throw motion 4-2]") {
                motion = &animation->throw_motion_second[3];
            } else if (tag == "[dash motion]") {
                motion = &animation->dash_motion;
            } else if (tag == "[dashattack motion]") {
                motion = &animation->dashattack_motion;
            } else if (tag == "[getitem motion]") {
                motion = &animation->getitem_motion;
            } else if (tag == "[buff motion]") {
                motion = &animation->buff_motion;
            } else {
                motion = &animation->back_motion;
            }
            char formatted[512];
            std::memset(formatted, 0, sizeof(formatted));
            std::snprintf(formatted, sizeof(formatted), motion_format.c_str(),
                          path->c_str());
            *motion = formatted;
        } else if (tag == "[attack motion]" || tag == "[etc motion]") {
            STEquipmentAniScript* animation = &normal_animation;
            const std::string* path = &normal_animation_path;
            if (expand_animation) {
                animation = &expand_animation_script;
                path = &expand_animation_path;
            } else if (level_section_animation != -1) {
                animation = &level_animation;
                path = &level_animation_path;
            }
            std::vector<std::string>* motions = tag == "[attack motion]"
                ? &animation->attack_motion : &animation->etc_motion;
            motions->clear();
            std::string motion_format;
            while (ScanStr(&motion_format) == 1) {
                char formatted[512];
                std::memset(formatted, 0, sizeof(formatted));
                std::snprintf(formatted, sizeof(formatted), motion_format.c_str(),
                              path->c_str());
                motions->push_back(formatted);
            }
        } else if (tag == "[animation job]") {
            std::string job_name;
            ScanStr(&job_name);
            EquipmentAniInfoScript* ani_info =
                script->addAniInfo(
                    static_cast<ENUM_CHARACTERJOB>(animation_job));
            if (normal_animation.layer != 0) {
                ani_info->base_ani.push_back(normal_animation);
                normal_animation.layer = 0;
            }
            if (expand_animation_script.layer != 0) {
                ani_info->layer_ani.push_back(expand_animation_script);
                expand_animation_script.layer = 0;
            }
            if (level_animation.layer != 0) {
                if (!ani_info->layer_lists.empty()) {
                    ani_info->layer_lists.back().push_back(level_animation);
                }
                level_animation.layer = 0;
            }
            animation_job = ::getCharacterJob(job_name.c_str());
        } else if (tag == "[variation]") {
            EquipmentAniInfoScript* ani_info =
                script->addAniInfo(
                    static_cast<ENUM_CHARACTERJOB>(animation_job));
            if (expand_animation) {
                ani_info->unknown_30 = ScanInt(static_cast<bool*>(0));
                ani_info->unknown_34 = ScanInt(static_cast<bool*>(0));
            } else if (level_section_animation == -1) {
                ani_info->unknown_0C = ScanInt(static_cast<bool*>(0));
                ani_info->unknown_10 = ScanInt(static_cast<bool*>(0));
            } else {
                level_growth_info.required_level =
                    ScanInt(static_cast<bool*>(0));
                level_growth_info.grow_type =
                    ScanInt(static_cast<bool*>(0));
            }
        } else if (tag == "[setback img]") {
            ScanStr(&script->string_83c);
            std::vector<stSetItemBackImageInfo>* images =
                &script->back_images_840;
            while (true) {
                stSetItemBackImageInfo image;
                bool success = false;
                image.values_00[0] = ScanInt(&success);
                if (!success) {
                    break;
                }
                image.values_00[1] = ScanInt(&success);
                if (!success) {
                    break;
                }
                image.values_00[2] = ScanInt(&success);
                if (!success) {
                    break;
                }
                images->push_back(image);
            }
        } else if (tag == "[/setback img]") {
            continue;
        } else if (tag == "[effect]") {
            effect::STEffect effect_data;
            if (importEffect(effect_data) != 0) {
                script->effects_84c.push_back(effect_data);
            }
        } else if (tag == "[usable job]") {
            std::string text;
            while (ScanStr(&text) == 1) {
                const int job = ::getCharacterJob(text.c_str());
                if (job == oldsrv::config::kCharacterJobCommon) {
                    for (int index = 0;
                         index < oldsrv::config::kCharacterJobCount;
                         ++index) {
                        script->item.usable_job_flags[index] = 1;
                    }
                    script->item.value_a0 = 1;
                } else if (job >= 0 &&
                           job < oldsrv::config::kCharacterJobCount) {
                    script->item.usable_job_flags[job] = 1;
                }
            }
        } else if (tag == "[suitable job]") {
            std::set<int>* jobs = &script->item.usable_jobs;
            jobs->clear();
            std::string text;
            while (ScanStr(&text) == 1) {
                const int job = ::getCharacterJob(text.c_str());
                if (job == oldsrv::config::kCharacterJobCommon) {
                    for (int i = 0;
                         i < oldsrv::config::kCharacterJobCount;
                         ++i) {
                        jobs->insert(i);
                    }
                } else if (job >= 0 &&
                           job < oldsrv::config::kCharacterJobCount) {
                    jobs->insert(job);
                }
            }
        } else if (tag == "[max having count]") {
            script->item.values_5c[1] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[minimum level]") {
            script->item.values_5c[0] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[maximum level]") {
            script->values_178[1] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[equipment grow]") {
            // CLIENT_EA: ImportEquipmentScript_ReadEquipmentGrowthTypeAndStage
            // @0x1579B89.
            bool success = false;
            const int growth_type = ScanInt(&success);
            if (!success) {
                EQUIP_FAIL();
            }
            const int growth_stage = ScanInt(&success);
            if (!success) {
                EQUIP_FAIL();
            }
            script->equipment_growth_type =
                static_cast<uint8_t>(growth_type);
            script->equipment_growth_stage =
                static_cast<uint8_t>(growth_stage);
        } else if (tag == "[seed correction index]") {
            // CLIENT_EA: ImportEquipmentScript_ReadSeedCorrectionIndex
            // @0x1579C86.
            script->seed_correction_index =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[upgrade limit table]") {
            // CLIENT_EA: ImportEquipmentScript_ReadUpgradeLimitTableName
            // @0x157464C.
            ScanStr(&script->upgrade_limit_table_name);
        } else if (tag == "[minimum rank]") {
            script->item.values_5c[2] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[control index]") {
            script->values_178[2] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[required skill]") {
            script->values_178[3] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[cool time]") {
            script->item.values_5c[3] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[usable even peace]") {
            script->value_78c = ScanInt(static_cast<bool*>(0)) == 1;
        } else if (tag == "[equipment type]") {
            std::string text;
            ScanStr(&text);
            script->item.strings_ac[4] = text;
            script->values_1b4[0] = GetEquipmentType(text.c_str());
            script->item.values_a4[0] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[sub type]") {
            if (expand_animation) {
                script->addAniInfo(
                    static_cast<ENUM_CHARACTERJOB>(animation_job))->unknown_3C =
                    ScanInt(static_cast<bool*>(0));
            } else if (level_section_animation == -1) {
                script->values_1b4[1] = ScanInt(static_cast<bool*>(0));
            } else {
                level_growth_info.unknown_10 =
                    ScanInt(static_cast<bool*>(0));
            }
        } else if (tag == "[durability]") {
            script->values_1b4[2] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[creature species]") {
            script->value_6bc = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[creature minimum level]") {
            script->artifact_738.usable_type =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[hide layer]") {
            std::vector<int>* values = &script->values_774;
            values->clear();
            bool success = false;
            int value = ScanInt(&success);
            while (success) {
                values->push_back(value);
                value = ScanInt(&success);
            }
        } else if (tag == "[hide growtype avatar]") {
            std::vector<int>* values = &script->values_780;
            values->clear();
            bool success = false;
            int value = ScanInt(&success);
            while (success) {
                values->push_back(value);
                value = ScanInt(&success);
            }
        } else if (tag == "[hide grow avatar]") {
            // CLIENT_EA: parser @0x157B032, name conversion @0x157B057,
            // invalid sentinel branch @0x157B065, append @0x157B0C8.
            std::string grow_avatar_name;
            while (ScanStr(&grow_avatar_name) == 1) {
                const ENUM_GROW_AVATAR_TYPE grow_avatar_type =
                    GetGrowAvatarTypeByName(grow_avatar_name.c_str());
                if (grow_avatar_type != GROW_AVATAR_TYPE_INVALID) {
                    script->hidden_grow_avatar_types.push_back(
                        grow_avatar_type);
                }
            }
        } else if (tag == "[usable period]") {
            script->item.values_f4[0] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[expiration date]") {
            script->item.values_f4[1] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[weapon full avatar]") {
            script->value_72d = 1;
        } else if (tag == "[full avatar]") {
            // The tag is a valueless equipment-prototype classification.
            script->full_avatar_flag = 1;
        } else if (tag == "[Rental Service]" ||
                   tag == "[rental service]") {
            bool success = false;
            const int rental_service_type = ScanInt(&success);
            if (!success || rental_service_type < 1 ||
                rental_service_type > 3) {
                EQUIP_FAIL();
            }
            script->rental_service_type =
                static_cast<uint8_t>(rental_service_type);
        } else if (tag == "[skin emblem socket]") {
            // CLIENT_EA: token registration @0x15740F5, token 0x96C,
            // valueless handler @0x157510C.
            script->skin_emblem_socket_enabled = 1;
        } else if (tag == "[hide equipment]") {
            std::vector<int>* values = &script->values_768;
            values->clear();
            std::string text;
            while (ScanStr(&text) == 1) {
                if (text == "[/hide equipment]") {
                    break;
                }
                int equipment_type = oldsrv::config::kEquipmentTypeInvalid;
                if (text == "[hat avatar]") {
                    equipment_type = 0;
                } else if (text == "[hair avatar]") {
                    equipment_type = 1;
                } else if (text == "[face avatar]") {
                    equipment_type = 2;
                } else if (text == "[coat avatar]") {
                    equipment_type = 3;
                } else if (text == "[pants avatar]") {
                    equipment_type = 4;
                } else if (text == "[shoes avatar]") {
                    equipment_type = 5;
                } else if (text == "[breast avatar]") {
                    equipment_type = 6;
                } else if (text == "[waist avatar]") {
                    equipment_type = 7;
                } else if (text == "[skin avatar]") {
                    equipment_type = 8;
                } else if (text == "[aurora avatar]") {
                    equipment_type = 9;
                } else if (text == "[weapon avatar]") {
                    equipment_type = 10;
                } else if (text == "[weapon]") {
                    equipment_type = 11;
                } else if (text == "[title name]") {
                    equipment_type = 12;
                }
                if (equipment_type != oldsrv::config::kEquipmentTypeInvalid) {
                    values->push_back(equipment_type);
                }
            }
        } else if (tag == "[emblem socket default]") {
            std::vector<unsigned short>* sockets = &script->avatar_emblems_6cc;
            std::string text;
            while (ScanStr(&text) == 1) {
                const int socket_type = getAvatarEmblemType(text.c_str());
                if (socket_type == 0) {
                    EQUIP_FAIL();
                }
                sockets->push_back(static_cast<unsigned short>(socket_type));
            }
            if (sockets->size() > 5u) {
                EQUIP_FAIL();
            }
        } else if (tag == "[usable module]") {
            std::set<int>* modules = &script->usable_modules_714;
            modules->clear();
            for (int value = scanModuleType(); value != 17; value = scanModuleType()) {
                modules->insert(value);
            }
        } else if (tag == "[usable world]") {
            std::set<int>* worlds = &script->item.usable_worlds;
            worlds->clear();
            for (int value = scanWorldType(); value != 2; value = scanWorldType()) {
                worlds->insert(value);
            }
        } else if (tag == "[not accumulate]") {
            // CLIENT_EA: ImportEquipmentScript @0x1575D7D reads a set-map
            // key, materializes that parameter entry, then enables its
            // EquipmentParameterInfo+0xAEC flag.
            bool success = false;
            const int parameter_key = ScanInt(&success);
            // The client uses the complete signed integer as the set-map key.
            // Preserve it exactly instead of guessing a piece-count enum.
            if (!success) {
                EQUIP_FAIL();
            }
            EquipmentParameterInfo& parameter =
                script->set_parameters_678[parameter_key];
            equipment_parameter_0828::SetNotAccumulate(&parameter, true);
        } else if (tag == "[piece set ability]") {
            bool success = false;
            const int piece_set_id = ScanInt(&success);
            if (!success) {
                EQUIP_FAIL();
            }
            EquipmentParameterInfo& parameter =
                script->set_parameters_678[piece_set_id];
            parameter.clear();
            parameter.values_280[101].integer = 1;
            active_parameter = &parameter;
            active_piece_set_scope = true;
            parameter_first = 0;
            parameter_second = 0;
        } else if (tag == "[set ability]") {
            EquipmentParameterInfo& parameter = script->set_parameters_678[0];
            parameter.clear();
            parameter.values_280[101].integer = 1;
            active_parameter = &parameter;
            active_piece_set_scope = false;
            parameter_first = 0;
            parameter_second = 0;
        } else if (tag == "[/set ability]") {
            script->parameter_1c0.conditional_values_468 =
                active_parameter->conditional_values_468;
            active_parameter = &script->parameter_1c0;
            active_piece_set_scope = false;
        } else if (tag == "[/piece set ability]") {
            active_parameter = &script->parameter_1c0;
            active_piece_set_scope = false;
        } else if (tag == "[level section ability]") {
            int section_value = 0;
            if (ScanInt(&section_value) != 1) {
                EQUIP_FAIL();
            }
            script->level_parameters_690.push_back(EquipmentParameterInfo());
            active_parameter = &script->level_parameters_690.back();
            active_piece_set_scope = false;
            parameter_first = 0;
            parameter_second = 0;
        } else if (tag == "[/level section ability]") {
            active_parameter = &script->parameter_1c0;
            active_piece_set_scope = false;
        } else if (tag == "[avatar type select]") {
            std::vector<stAvatarTypeSelect_t>* selections =
                &script->avatar_type_select_6c0;
            stAvatarTypeSelect_t default_selection;
            default_selection.init();
            default_selection.usable_period = 7;
            selections->push_back(default_selection);

            while (true) {
                stAvatarTypeSelect_t selection;
                selection.init();
                bool success = false;
                selection.usable_period = ScanInt(&success);
                if (!success) {
                    break;
                }
                selection.gold_price = ScanInt(&success);
                if (!success) {
                    EQUIP_FAIL();
                }
                selection.win_point_price = ScanInt(&success);
                if (!success) {
                    EQUIP_FAIL();
                }
                selection.cera_price = ScanInt(&success);
                if (!success) {
                    EQUIP_FAIL();
                }
                const int emblem_count = ScanInt(&success);
                if (!success || emblem_count < 0 || emblem_count > 5 ||
                    static_cast<unsigned int>(emblem_count) +
                            script->avatar_emblems_6cc.size() >
                        5u) {
                    EQUIP_FAIL();
                }
                selection.socket_count = emblem_count;
                for (int index = 0; index < emblem_count; ++index) {
                    std::string emblem;
                    if (ScanStr(&emblem) != 1) {
                        EQUIP_FAIL();
                    }
                    const int emblem_type = getAvatarEmblemType(emblem.c_str());
                    if (emblem_type == 0) {
                        EQUIP_FAIL();
                    }
                    selection.socket_types[index] =
                        static_cast<unsigned short>(emblem_type);
                }
                selections->push_back(selection);
            }
        } else if (tag == "[avatar func filter]") {
            bool success = false;
            script->value_6d8 = ScanInt(&success);
            if (!success) {
                EQUIP_FAIL();
            }
        } else if (tag == "[change status on change hp]") {
            std::vector<STChangeStatusRate>* rates =
                &script->change_status_rates_6e8;
            std::string status;
            while (ScanStr(&status) == 1) {
                if (status == "[/change status on change hp]") {
                    break;
                }
                const int appendage_type = loadAppendageType(status);
                if (appendage_type == -1) {
                    continue;
                }
                STChangeStatusRate rate;
                rate.active_status_type = appendage_type;
                bool success = true;
                rate.values_04[0] = static_cast<float>(ScanInt(&success));
                rate.values_04[2] = static_cast<float>(ScanInt(&success));
                rate.values_04[1] = static_cast<float>(ScanInt(&success));
                rate.values_04[3] = static_cast<float>(ScanInt(&success));
                if (success) {
                    rates->push_back(rate);
                }
            }
        } else if (tag == "[limit upgradable level]") {
            std::string upgrade_name;
            if (ScanStr(&upgrade_name) != 1) {
                // The 0828 PVF uses an empty closed block for equipment that
                // has no permitted upgrade-level range. Keep the map empty;
                // the following parser iteration consumes the closing tag.
                continue;
            }
            bool first_success = false;
            bool second_success = false;
            int minimum_level = ScanInt(&first_success);
            int maximum_level = ScanInt(&second_success);
            if (!first_success) {
                minimum_level = -1;
            }
            if (!second_success) {
                maximum_level = -1;
            }

            int upgrade_type = 5;
            if (upgrade_name == "all upgrade") {
                upgrade_type = 0;
            } else if (upgrade_name == "normal upgrade") {
                upgrade_type = 1;
            } else if (upgrade_name == "amplify upgrade") {
                upgrade_type = 2;
            } else if (upgrade_name == "separate upgrade") {
                upgrade_type = 3;
            } else if (upgrade_name == "3rd chronicle upgrade") {
                upgrade_type = 4;
            }

            if (minimum_level == -1) {
                minimum_level = 1;
            }
            if (maximum_level == -1) {
                maximum_level = oldsrv::config::kCharacterLevelMaximum;
            }
            std::map<ItemUpgradeType::T, std::pair<int, int> >* levels =
                &script->upgradable_levels_6fc;
            if (upgrade_type == 0 &&
                (first_success || second_success)) {
                for (int type = 1; type <= 4; ++type) {
                    const ItemUpgradeType::T typed_type =
                        static_cast<ItemUpgradeType::T>(type);
                    if (levels->find(typed_type) == levels->end()) {
                        levels->insert(std::make_pair(
                            typed_type,
                            std::make_pair(minimum_level, maximum_level)));
                    }
                }
            } else if (upgrade_type >= 1 && upgrade_type <= 4 &&
                       (first_success || second_success)) {
                const ItemUpgradeType::T typed_type =
                    static_cast<ItemUpgradeType::T>(upgrade_type);
                if (levels->find(typed_type) == levels->end()) {
                    levels->insert(std::make_pair(
                        typed_type,
                        std::make_pair(minimum_level, maximum_level)));
                }
            }
        } else if (tag == "[/limit upgradable level]") {
            continue;
        } else if (tag == "[character name y revision]") {
            script->character_name_y_80e = static_cast<unsigned short>(
                ScanInt(static_cast<bool*>(0)));
            script->character_name_x_80c = static_cast<unsigned short>(
                ScanInt(static_cast<bool*>(0)));
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
                if (!success || priority > 4) {
                    EQUIP_FAIL();
                }
                script->item.special_drop_items.push_back(
                    std::make_pair(job, priority));
            }
        } else if (tag == "[particle]") {
            std::string particle_type;
            if (ScanStr(&particle_type) != 1) {
                EQUIP_FAIL();
            }
            if (particle_type == "same position") {
                if (ScanStr(&script->string_7f4) != 1) {
                    EQUIP_FAIL();
                }
                script->position_7f8.x = ScanInt(static_cast<bool*>(0));
                script->position_7f8.y = ScanInt(static_cast<bool*>(0));
                script->position_7f8.z = ScanInt(static_cast<bool*>(0));
                std::string end_tag;
                if (ScanType(end_tag, true) != 1) {
                    EQUIP_FAIL();
                }
            }
        } else if (tag == "[impossible contents]") {
            std::string content;
            while (ScanStr(&content) == 1) {
                if (content == "upgrade") {
                    script->impossible_upgrade_6f4 = 1;
                } else if (content == "amplify upgrade") {
                    script->impossible_amplify_6f5 = 1;
                } else if (content == "disjoint") {
                    script->impossible_disjoint_6f6 = 1;
                } else if (content == "gift") {
                    script->item.value_d0 = 1;
                } else if (content == "trade") {
                    script->item.value_d1 = 1;
                } else if (content == "separate upgrade") {
                    script->item.value_d4 = 1;
                } else if (content == "charac cargo") {
                    script->item.value_da = 1;
                } else if (content == "ban redeemitem") {
                    script->item.value_d8 = 1;
                }
            }
        } else if (tag == "[item group name]") {
            std::string group_name;
            if (ScanStr(&group_name) != 1) {
                EQUIP_FAIL();
            }
            std::map<std::string, int>* groups = getItemGroupNameTable();
            std::map<std::string, int>::iterator found = groups->find(group_name);
            if (found == groups->end()) {
                EQUIP_FAIL();
            }
            script->item.value_100 = found->second;
        } else if (tag == "[emancipate]") {
            script->item.value_10e = 1;
            std::string section;
            while (ScanType(section, true) == 1) {
                std::vector<std::pair<int, int> >* values = 0;
                if (section == "[equipment grow]") {
                    // The 0828 dragon-growth corpus nests this tag inside
                    // [emancipate], before its material/next-stage lists.
                    bool success = false;
                    const int growth_type = ScanInt(&success);
                    if (!success) { EQUIP_FAIL(); }
                    const int growth_stage = ScanInt(&success);
                    if (!success) { EQUIP_FAIL(); }
                    script->equipment_growth_type = static_cast<uint8_t>(growth_type);
                    script->equipment_growth_stage = static_cast<uint8_t>(growth_stage);
                    continue;
                } else if (section == "[/input]" || section == "[/output]") {
                    continue;
                } else if (section == "[input]") {
                    values = &script->pairs_7d8;
                } else if (section == "[output]") {
                    values = &script->pairs_7e4;
                } else if (section == "[emancipate explain]") {
                    if (ScanStr(&script->string_7f0) != 1) {
                        EQUIP_FAIL();
                    }
                    continue;
                } else {
                    if (section != "[/emancipate]") {
                        tag = section;
                        has_buffered_tag = true;
                    }
                    break;
                }
                bool success = false;
                int item_id = ScanInt(&success);
                while (success) {
                    const int count = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    values->push_back(std::make_pair(item_id, count));
                    item_id = ScanInt(&success);
                }
            }
        } else if (tag == "[creature physical attack]") {
            script->artifact_738.grade = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[creature magical attack]") {
            script->artifact_738.rarity = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[creature skill consume mp rate]") {
            script->artifact_738.physical_attack = ScanFloat(2, 0);
        } else if (tag == "[creature skill charge time rate]") {
            script->artifact_738.magical_attack = ScanFloat(2, 0);
        } else if (tag == "[creature skill over charge time rate]") {
            script->artifact_738.physical_defense = ScanFloat(2, 0);
        } else if (tag == "[creature experience amount rate]") {
            script->artifact_738.magical_defense = ScanFloat(2, 0);
        } else if (tag == "[creature physical critical hit]") {
            script->artifact_738.strength = ScanFloat(1, 0);
        } else if (tag == "[creature magical critical hit]") {
            script->artifact_738.vitality = ScanFloat(1, 0);
        } else if (tag == "[avatar task num]") {
            ScanInt(static_cast<bool*>(0));
        } else if (tag == "[creature stuck]") {
            script->artifact_738.spirit = ScanFloat(1, 0);
        } else if (tag == "[creature piercing]") {
            script->artifact_738.item_index =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[output index]") {
            script->value_764 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[price]") {
            script->item.values_08[3] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[value]") {
            script->item.value_24 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[repair price]") {
            script->values_178[0] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[finish point price]") {
            script->item.values_08[6] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[cash]") {
            script->item.values_08[4] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[medal]") {
            script->item.values_08[5] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[usable upgrade chracter level]") {
            script->item.usable_upgrade_character_level =
                static_cast<unsigned short>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[need material]") {
            script->item.value_28 = ScanInt(static_cast<bool*>(0));
            script->item.value_2c = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[recieve postal message]") {
            ScanStr(&script->item.value_174);
        } else if (tag == "[Uncompoundable]") {
            bool success = false;
            const int value = ScanInt(&success);
            if (success && value == 1) {
                script->value_82c = 1;
            }
        } else if (tag == "[remove avatar]") {
            std::vector<int>* modules = &script->values_830;
            std::string text;
            while (ScanStr(&text) == 1) {
                modules->push_back(getModuleType(text.c_str()));
            }
        } else if (tag == "[auction free]") {
            script->item.value_30 = 1;
        } else if (tag == "[hidden option]") {
            script->item.value_31 = 1;
        } else if (tag == "[set item]") {
            std::vector<int>* values = &script->values_66c;
            values->clear();
            bool success = false;
            int value = ScanInt(&success);
            while (success) {
                values->push_back(value);
                value = ScanInt(&success);
            }
        } else if (tag == "[set item master]") {
            script->value_6a0 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[int data]") {
            std::vector<int>* values = &script->item.int_data;
            bool success = false;
            int value = ScanInt(&success);
            while (success) {
                values->push_back(value);
                value = ScanInt(&success);
            }
        } else if (tag == "[string data]") {
            std::vector<std::string>* values = &script->item.string_data;
            std::string text;
            while (ScanStr(&text) == 1) {
                values->push_back(text);
            }
        } else if (tag == "[packagable]") {
            script->item.value_fc =
                ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[icon mark]") {
            ScanStr(&script->item.strings_ac[5]);
            script->item.values_c4[0] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[part set index]") {
            script->value_798 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[effect part set index]") {
            script->value_79c = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[reference effect part set index]") {
            script->values_7a0.push_back(
                ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[image packs]") {
            std::vector<std::string>* values = &script->strings_7b0;
            std::string text;
            while (ScanStr(&text) == 1) {
                values->push_back(text);
            }
        } else if (tag == "[fixed select type]") {
            script->value_6e0 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[not amplify]") {
            script->value_890 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[change type ultimateSkillCurScene]") {
            script->value_6dc = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[move wav]") {
            ScanStr(&script->item.name);
        } else if (tag == "[custom wav]") {
            if (expand_animation) {
                std::vector<std::string>* values =
                    &script->addAniInfo(
                        static_cast<ENUM_CHARACTERJOB>(animation_job))->string_list;
                values->clear();
                std::string text;
                while (ScanStr(&text) == 1) {
                    values->push_back(text);
                }
            } else {
                std::vector<std::string>* values = &script->strings_658;
                values->clear();
                std::string text;
                while (ScanStr(&text) == 1) {
                    values->push_back(text);
                }
            }
        } else if (tag == "[command]") {
            STKeyCommand* command = new STKeyCommand();
            script->key_command_804 = command;
            importCommand(command);
        } else if (tag == "[npc gift disallowance]") {
            bool success = false;
            script->item.value_104 = ScanInt(&success) != 1;
            if (!success) {
                EQUIP_FAIL();
            }
        } else if (tag == "[hardcoding cooltime]") {
            bool success = false;
            script->value_6e4 = ScanInt(&success);
            if (!success) {
                EQUIP_FAIL();
            }
        } else if (tag == "[epic routing]") {
            bool success = false;
            script->value_7bc = ScanInt(&success) == 1;
            if (!success) {
                EQUIP_FAIL();
            }
        } else if (tag == "[special ability]") {
            bool success = false;
            script->value_7c0 = ScanInt(&success);
        } else if (tag == "[chat emoticon index]") {
            bool success = false;
            script->value_7c4 = ScanInt(&success);
        } else if (tag == "[required full set for chatting emoticon]") {
            bool success = false;
            script->value_7c8 = ScanInt(&success) != 0;
        } else if (tag == "[guild power war point]") {
            script->item.values_c4[1] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[user power war point]") {
            script->item.values_c4[2] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[hp recovery add rate]") {
            bool success = false;
            script->value_730 = ScanInt(&success);
        } else if (tag == "[mp recovery add rate]") {
            bool success = false;
            script->value_734 = ScanInt(&success);
        } else if (tag == "[possible kiri protect]") {
            script->item.value_d9 = 1;
        } else if (tag == "[event reward item]") {
            // CLIENT_EA: ImportEquipmentScript @0x1573F27.  The marker is
            // valueless and belongs to the common STItemScript extension.
            item_script_0828::SetEventRewardItem(&script->item, true);
        } else if (tag == "[quest item drop rate bonus]") {
            script->value_810 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[Force Result Item Rule]") {
            script->value_814 = ScanInt(static_cast<bool*>(0));
            script->value_818 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[usable period after unsealing]") {
            script->value_6f8 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[no random]") {
            script->value_72c = 1;
        } else if (tag == "[msg balloon on mucu skill]") {
            script->value_87c = ScanInt(static_cast<bool*>(0));
            ScanStr(&script->string_878);
        } else if (tag == "[aurora graphic effects]") {
            const int count = ScanInt(static_cast<bool*>(0));
            for (int i = 0; i < count; ++i) {
                script->values_864.push_back(
                    ScanInt(static_cast<bool*>(0)) == 1);
                std::string text;
                ScanStr(&text);
                script->strings_858.push_back(text);
            }
        } else if (tag == "[random option]") {
            script->value_888 = 1;
            script->value_889 =
                static_cast<unsigned char>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[enable dye]") {
            script->value_88a =
                ScanInt(static_cast<bool*>(0)) != 0;
            script->value_88b =
                ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[war room point bonus]") {
            std::string text;
            if (ScanStr(&text) != 1) {
                continue;
            }
            script->wp_bonus_81c.type = getCalculationType(text.c_str());
            if (script->wp_bonus_81c.type == 2) {
                EQUIP_FAIL();
            }
            bool success = false;
            script->wp_bonus_81c.value = ScanInt(&success);
            if (!success) {
                EQUIP_FAIL();
            }
        } else if (tag == "[war room monster exp bonus]") {
            std::string text;
            if (ScanStr(&text) != 1) {
                continue;
            }
            script->exp_bonus_824.type = getCalculationType(text.c_str());
            if (script->exp_bonus_824.type == 2) {
                EQUIP_FAIL();
            }
            bool success = false;
            script->exp_bonus_824.value = ScanInt(&success);
            if (!success) {
                EQUIP_FAIL();
            }
        } else if (tag == "[expertjob only]") {
            if (GetExpertJobTypeList(&script->item) != 1) {
                EQUIP_FAIL();
            }
        } else if (tag == "[prof material variation]") {
            if (!GetExpertJobValueMaterialVariation(&script->item)) {
                EQUIP_FAIL();
            }
        } else if (tag == "[prof compound rate]") {
            if (!GetExpertJobValueCompoundRate(&script->item)) {
                EQUIP_FAIL();
            }
        } else if (tag == "[prof result variation]") {
            if (!GetExpertJobResultVariation(&script->item)) {
                EQUIP_FAIL();
            }
        } else if (tag == "[prof disjoint big win rate]") {
            if (!GetExpertJobDisjointBigWinRate(&script->item)) {
                EQUIP_FAIL();
            }
        } else if (tag == "[prof disjoint result variation]") {
            if (!GetExpertJobDisjointResultVariation(&script->item)) {
                EQUIP_FAIL();
            }
        } else if (tag == "[prof additional gain exp]") {
            if (!GetExpertJobAdditionalGainExp(&script->item)) {
                EQUIP_FAIL();
            }
        } else if (tag == "[character item check]") {
            std::vector<STEnchantSystemData>* values =
                &script->enchant_data_7cc;
            bool success = false;
            std::string equipment_type;
            while (1) {
                STEnchantSystemData value;
                value.value_00 = ScanInt(&success);
                if (!success) {
                    break;
                }
                // Original ImportEquipmentScript: job(int) at +0, first
                // grow(byte) at +4, equipment type(int) at +8.  Swapping
                // the last two makes every ordinary slot fail job/grow checks.
                value.value_04 = static_cast<uint8_t>(ScanInt(&success));
                if (!success) {
                    break;
                }
                if (ScanStr(&equipment_type) != 1) {
                    EQUIP_FAIL();
                }
                value.value_08 = getEquipmentTypeToEnumValue(equipment_type);
                values->push_back(value);
            }
        } else if (tag == "[weapon overspec per item]") {
            if (active_parameter != &script->parameter_1c0 ||
                ImportWeaponOverspecPerItem(script) != 1) {
                EQUIP_FAIL();
            }
        } else if (tag == "[item category]") {
            if (ReadEquipmentItemCategoryList(script) != 1) {
                EQUIP_FAIL();
            }
        } else if (tag == "[aura pos datas]") {
            if (!ImportAuraPositionData(script)) {
                EQUIP_FAIL();
            }
        } else if (tag == "[aura ability]") {
            GetAuraAvatarOption(script);
        } else if (tag == "[/aura ability]") {
            continue;
        } else {
            int module_tag_index = -1;
            int parsed_module_type = -1;
            const int module_result = processModuleTagSet(module_tag_index,
                                                           parsed_module_type,
                                                           &tag,
                                                           module_type,
                                                           true);
            if (module_tag_index != -1) {
                if (parsed_module_type != -1 && module_result != 0) {
                    script->value_808 = parsed_module_type;
                }
                continue;
            }
            const int parameter_result = importEquipmentParameterInfo(
                active_parameter,
                tag,
                filename,
                parameter_first,
                parameter_second);
            if (parameter_result > 0) {
                has_buffered_tag = parameter_result == 2;
                continue;
            }
            if (parameter_result < 0) {
                EQUIP_FAIL();
            }
            if (EquipmentScriptSpecialMonsterDrop(script, tag, filename) != 1) {
                EQUIP_FAIL();
            }
        }
    }

    // A nested parameter block is atomic.  Do not publish a Helix/set script
    // that reached EOF or a scalar mismatch before its explicit outer close.
    if (active_parameter != &script->parameter_1c0) {
        // The official 0828 part-set corpus contains six scripts whose final
        // [piece set ability] block is terminated by physical EOF instead of
        // [/piece set ability] (3choroset6 and the event/Helix variants).
        // The original importer accepts that EOF form.  Keep fail-closed
        // behavior for every other nested scope and for non-EOF truncation.
        if (!active_piece_set_scope ||
            g_dnfLex.getLastToken() != DNF_TOK_EOF) {
            EQUIP_FAIL();
        }
        active_parameter = &script->parameter_1c0;
        active_piece_set_scope = false;
    }

    if (normal_animation.layer != 0) {
        script->addAniInfo(static_cast<ENUM_CHARACTERJOB>(animation_job))
            ->base_ani.push_back(
            normal_animation);
    }
    if (expand_animation_script.layer != 0) {
        script->addAniInfo(static_cast<ENUM_CHARACTERJOB>(animation_job))
            ->layer_ani.push_back(
            expand_animation_script);
    }
    if (level_animation.layer != 0) {
        EquipmentAniInfoScript* ani_info =
            script->addAniInfo(
                static_cast<ENUM_CHARACTERJOB>(animation_job));
        if (!ani_info->layer_lists.empty()) {
            ani_info->layer_lists.back().push_back(level_animation);
        }
    }

    if (script->values_178[0] == -1) {
        script->values_178[0] = script->item.values_08[3];
        if (script->item.value_24 != -1) {
            script->values_178[0] = script->item.value_24;
        }
    }
    if (script->item.value_24 == -1) {
        script->item.value_24 = script->item.values_08[3];
    }
    script->item.index = set_id == -1
        ? g_stEquipmentScriptFileList.FindIndexByFullScan(filename)
        : set_id;
    unloadRDARScriptFile();

#undef EQUIP_FAIL
    return 1;
}
