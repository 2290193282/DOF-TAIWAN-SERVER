#include "old_data/PVF/etc/independent_drop_parameter_script.h"
#include "old_data/PVF/creature/creature_script.h"
#include "old_data/PVF/aicharacter/datamanager/ai_character_script.h"
#include "old_data/PVF/data/datamanager/module_tag_table.h"
#include "old_data/PVF/aicharacter/datamanager/monster_manager.h"
#include "old_data/PVF/aicharacter/datamanager/monster_script.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/datamanager/script_file_list.h"
#include "old_data/PVF/data/datamanager/sync_script.h"
#include "common/manager/character_script_public_abi.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"

#include <cstdio>
#include <cstring>
#include <map>
#include <new>
#include <utility>
#include <vector>

extern "C++" int get_rand_int_global(int limit) __asm__("_Z12get_rand_inti");

namespace {

int& MonsterScriptIndex(STMonsterScript* script) {
    return script->index;
}

std::vector<stMonsterIndependentDrop_t>* MonsterScriptIndependentDrops(
    STMonsterScript* script) {
    return &script->independent_drops;
}

int MonsterScriptLevelRangeBegin(const STMonsterScript* script) {
    return script->level_range[0];
}

int MonsterScriptLevelRangeEnd(const STMonsterScript* script) {
    return script->level_range[1];
}

std::vector<stIndependentDropInfo>* IndependentDropInfos() {
    return &G_CDataManager()->get_independent_drop_parameter_script()
                ->independent_drop_info_;
}

bool IsCurrentPvfStatusBoundTag(const std::string& tag) {
    static const char* const kTags[] = {
        "[slow]", "[freeze]", "[poison]", "[stun]", "[curse]",
        "[blind]", "[lightning]", "[stone]", "[sleep]", "[burn]",
        "[weapon break]", "[bleeding]", "[piercing]", "[stuck]",
        "[confuse]", "[hold]", "[armor break]"
    };
    for (size_t index = 0; index != sizeof(kTags) / sizeof(kTags[0]);
         ++index) {
        if (tag == kTags[index]) {
            return true;
        }
    }
    return false;
}

bool ConsumeCurrentPvfStatusBound(const char* closing_tag) {
    std::string tag;
    if (ScanType(tag, true) != 1) {
        return false;
    }

    if (tag == "[all]") {
        while (ScanType(tag, true) == 1) {
            if (tag == "[/all]") {
                break;
            }
            if (!IsCurrentPvfStatusBoundTag(tag)) {
                return false;
            }
            bool success = false;
            (void)ScanInt(&success);
            if (!success) {
                return false;
            }
        }
        return tag == "[/all]" && ScanType(tag, true) == 1 &&
               tag == closing_tag;
    }

    // Some current PVFs carry an explicit five-difficulty table instead of
    // the shared [all] form.  It has a fixed normal→slayer order.
    static const char* const kOpenTags[] = {
        "[normal]", "[expert]", "[master]", "[king]", "[slayer]"
    };
    static const char* const kCloseTags[] = {
        "[/normal]", "[/expert]", "[/master]", "[/king]", "[/slayer]"
    };
    if (tag != kOpenTags[0]) {
        return false;
    }
    for (size_t group = 0; group != 5; ++group) {
        if (group != 0) {
            if (ScanType(tag, true) != 1 || tag != kOpenTags[group]) {
                return false;
            }
        }
        while (ScanType(tag, true) == 1) {
            if (tag == kCloseTags[group]) {
                break;
            }
            if (!IsCurrentPvfStatusBoundTag(tag)) {
                return false;
            }
            bool success = false;
            (void)ScanInt(&success);
            if (!success) {
                return false;
            }
        }
        if (tag != kCloseTags[group]) {
            return false;
        }
    }
    return ScanType(tag, true) == 1 && tag == closing_tag;
}

bool ConsumeCurrentPvfStatusDamageReduction(const char* closing_tag) {
    std::string tag;
    if (ScanType(tag, true) != 1 || tag != "[all]") {
        return false;
    }
    while (ScanType(tag, true) == 1) {
        if (tag == "[/all]") {
            break;
        }
        if (!IsCurrentPvfStatusBoundTag(tag)) {
            return false;
        }
        bool success = false;
        (void)ScanFloat(&success);
        if (!success) {
            return false;
        }
    }
    return tag == "[/all]" && ScanType(tag, true) == 1 &&
           tag == closing_tag;
}

bool ConsumeCurrentPvfImmuneActiveStatus(const char* closing_tag) {
    bool is_type = false;
    std::string value;
    while (ScanTypeOrStr(is_type, value, true) == 1) {
        if (is_type) {
            return value == closing_tag;
        }
        // Reuse the original server's canonical active-status vocabulary;
        // unknown names are rejected instead of silently changing semantics.
        if (getActiveStatusType(value) == 18) {
            return false;
        }
    }
    return false;
}

bool ScanCurrentPvfType(const char* expected) {
    std::string tag;
    return ScanType(tag, true) == 1 && tag == expected;
}

bool ScanCurrentPvfInt() {
    bool success = false;
    (void)ScanInt(&success);
    return success;
}

bool ScanCurrentPvfIntValue(int expected) {
    bool success = false;
    const int value = ScanInt(&success);
    return success && value == expected;
}

bool ScanCurrentPvfFloatValue(float expected) {
    float value = 0.0f;
    return ScanFloat(&value) == 1 && value == expected;
}

bool ConsumeCurrentPvfEmergencyStateRecovery() {
    // This nested state-recovery program is absent from the 0x1cc4 old
    // STMonsterScript ABI.  Accept only the finite forms present in the
    // current PVF; do not create an unproven timer/state-machine field.
    if (!ScanCurrentPvfType("[condition]") ||
        !ScanCurrentPvfType("[time after setstate]") ||
        !ScanCurrentPvfInt() || !ScanCurrentPvfType("[/condition]") ||
        !ScanCurrentPvfType("[action]")) {
        return false;
    }

    std::string action;
    if (ScanType(action, true) != 1) {
        return false;
    }
    if (action == "[set stay action]") {
    } else if (action == "[set attack action]") {
        if (!ScanCurrentPvfInt()) {
            return false;
        }
    } else {
        return false;
    }
    if (!ScanCurrentPvfType("[/action]")) {
        return false;
    }

    std::string trailer;
    if (ScanType(trailer, true) != 1) {
        return false;
    }
    if (trailer == "[/emergency state recovery]") {
        return true;
    }
    if (trailer != "[ignore condition]" ||
        !ScanCurrentPvfType("[attack action]")) {
        return false;
    }

    bool is_type = false;
    bool success = false;
    std::string token;
    int attack_indices = 0;
    while (true) {
        (void)ScanTypeOrInt(is_type, token, success, true);
        if (!success) {
            return false;
        }
        if (is_type) {
            if (token != "[/attack action]") {
                return false;
            }
            break;
        }
        if (++attack_indices > 13) {
            return false;
        }
    }
    if (attack_indices != 0 && attack_indices != 13) {
        return false;
    }
    return ScanCurrentPvfType("[etc action]") && ScanCurrentPvfInt() &&
           ScanCurrentPvfType("[/etc action]") &&
           ScanCurrentPvfType("[/ignore condition]") &&
           ScanCurrentPvfType("[/emergency state recovery]");
}

bool ConsumeCurrentPvfMinimumToleranceAndDefence() {
    std::string field;
    if (ScanType(field, true) != 1) {
        return false;
    }

    if (field == "[all]") {
        bool success = false;
        if (!ScanCurrentPvfType("[physical]")) {
            return false;
        }
        (void)ScanFloat(&success);
        if (!success || !ScanCurrentPvfInt() ||
            !ScanCurrentPvfType("[magical]")) {
            return false;
        }
        (void)ScanFloat(&success);
        if (!success || !ScanCurrentPvfInt() ||
            !ScanCurrentPvfType("[active status defence]")) {
            return false;
        }
        (void)ScanFloat(&success);
        return success && ScanCurrentPvfType("[/all]") &&
               ScanCurrentPvfType("[/set minimum tolerance and defence]");
    }

    static const char* const kOpenTags[] = {
        "[normal]", "[expert]", "[master]", "[king]", "[slayer]"
    };
    static const char* const kCloseTags[] = {
        "[/normal]", "[/expert]", "[/master]", "[/king]", "[/slayer]"
    };
    if (field != kOpenTags[0]) {
        return false;
    }
    for (size_t group = 0; group != 5; ++group) {
        if (group != 0 &&
            (ScanType(field, true) != 1 || field != kOpenTags[group])) {
            return false;
        }
        bool success = false;
        if (ScanType(field, true) != 1) {
            return false;
        }
        if (field == kCloseTags[group]) {
            continue;
        }
        if (field != "[physical]") {
            return false;
        }
        (void)ScanFloat(&success);
        if (!success) {
            return false;
        }
        (void)ScanFloat(&success);
        if (!success || !ScanCurrentPvfType("[magical]")) {
            return false;
        }
        (void)ScanFloat(&success);
        if (!success) {
            return false;
        }
        (void)ScanFloat(&success);
        if (!success || ScanType(field, true) != 1) {
            return false;
        }
        if (field == "[active status defence]") {
            (void)ScanFloat(&success);
            if (!success || !ScanCurrentPvfType(kCloseTags[group])) {
                return false;
            }
        } else if (field != kCloseTags[group]) {
            return false;
        }
    }
    return ScanCurrentPvfType("[/set minimum tolerance and defence]");
}

bool ConsumeCurrentPvfInitCooltime() {
    bool is_type = false;
    bool success = false;
    std::string token;
    int value_count = 0;
    while (true) {
        (void)ScanTypeOrInt(is_type, token, success, true);
        if (!success) {
            return false;
        }
        if (is_type) {
            if (token != "[/init cooltime]") {
                return false;
            }
            break;
        }
        ++value_count;
    }
    // Current PVF only supplies these four vector widths.  The original
    // monster record has no init-cooltime vector, so do not fabricate one.
    return value_count == 0 || value_count == 3 || value_count == 16 ||
           value_count == 82;
}

bool ConsumeCurrentPvfChangeActionByCondition() {
    // This post-original block selects client .act files as an object's HP
    // falls.  There is no action-replacement table in the old monster ABI;
    // consume only the one complete, typed four-stage form in this PVF.
    static const int kThresholds[] = {20, 40, 60, 80};
    for (size_t index = 0;
         index != sizeof(kThresholds) / sizeof(kThresholds[0]); ++index) {
        if (!ScanCurrentPvfType("[condition]") ||
            !ScanCurrentPvfType("[HP is lower than]")) {
            return false;
        }
        bool success = false;
        const int threshold = ScanInt(&success);
        if (!success || threshold != kThresholds[index]) {
            return false;
        }

        if (!ScanCurrentPvfType("[change action]") ||
            !ScanCurrentPvfType("[STAY]")) {
            return false;
        }
        std::string action_path;
        if (ScanStr(&action_path) != 1 || action_path.empty()) {
            return false;
        }

        for (int damage_index = 0; damage_index != 2; ++damage_index) {
            if (!ScanCurrentPvfType("[change action]") ||
                !ScanCurrentPvfType("[DAMAGE]")) {
                return false;
            }
            const int action_index = ScanInt(&success);
            if (!success || action_index != damage_index ||
                ScanStr(&action_path) != 1 || action_path.empty()) {
                return false;
            }
        }
    }
    return ScanCurrentPvfType("[/change action by condition]");
}

bool ConsumeCurrentPvfHitstunResistanceSystem() {
    // Post-original hitstun recovery timer.  The old ABI has no storage for
    // this system, so recognize the only complete current-PVF form instead
    // of aliasing it to an unrelated resistance member.
    bool success = false;
    return ScanCurrentPvfType("[increase time]") &&
           ScanInt(&success) == 1000 && success &&
           ScanCurrentPvfType("[decrease time]") &&
           ScanInt(&success) == 5000 && success &&
           ScanCurrentPvfType("[/hitstun resistance system]");
}

bool ConsumeCurrentPvfStrengthenDamagedEffect() {
    // Two Ozma files carry this later client hit-effect program.  No field
    // for it exists in the 0x1cc4 server record, so keep the strict grammar
    // check and do not manufacture a parallel visual-state machine.
    std::string state;
    if (!ScanCurrentPvfType("[enable]") ||
        !ScanCurrentPvfIntValue(1) ||
        !ScanCurrentPvfType("[apply absolute time]") ||
        !ScanCurrentPvfIntValue(200) ||
        !ScanCurrentPvfType("[effective state]") ||
        ScanStr(&state) != 1 || state != "all" ||
        !ScanCurrentPvfType("[/effective state]") ||
        !ScanCurrentPvfType("[shaking]") ||
        !ScanCurrentPvfType("[min]") || !ScanCurrentPvfIntValue(1) ||
        !ScanCurrentPvfType("[max]") || !ScanCurrentPvfIntValue(3) ||
        !ScanCurrentPvfType("[apply hitstun shaking power rate]") ||
        !ScanCurrentPvfFloatValue(1.0f) ||
        !ScanCurrentPvfType("[shaking frequency]") ||
        !ScanCurrentPvfIntValue(30) ||
        !ScanCurrentPvfType("[use shaking dissipation]") ||
        !ScanCurrentPvfType("[/shaking]") ||
        !ScanCurrentPvfType("[layer effect]") ||
        !ScanCurrentPvfType("[frequency rate]") ||
        !ScanCurrentPvfFloatValue(1.0f) ||
        !ScanCurrentPvfType("[alpha max]") ||
        !ScanCurrentPvfIntValue(100) ||
        !ScanCurrentPvfType("[body only]") ||
        !ScanCurrentPvfType("[/layer effect]")) {
        return false;
    }
    return ScanCurrentPvfType("[/strengthen damaged effect]");
}

bool ConsumeCurrentPvfConditionalAmplifyDamage() {
    // Later conditional damage modifier.  It is not represented in the
    // legacy record, but this exact three-file form must remain stream-safe.
    std::string state;
    return ScanCurrentPvfType("[state]") && ScanStr(&state) == 1 &&
           state == "hold" && ScanCurrentPvfType("[/state]") &&
           ScanCurrentPvfType("[damage rate]") &&
           ScanCurrentPvfIntValue(25) &&
           ScanCurrentPvfType("[/conditional amplify damage]");
}

bool ConsumeCurrentPvfStringData() {
    bool is_type = false;
    std::string value;
    while (ScanTypeOrStr(is_type, value, true) == 1) {
        if (is_type) {
            return value == "[/string data]";
        }
        // Values are script-defined string keys.  The original monster ABI
        // contains no vector for this newer block, but the typed stream and
        // its explicit closing tag still must be consumed exactly.
    }
    return false;
}

bool ConsumeCurrentPvfCreateVar() {
    // The current Ozma scripts declare interpreter-local object variables
    // here.  STMonsterScript predates that interpreter extension and has no
    // storage for those declarations; accepting only its documented o:<name>
    // lexical form keeps the surrounding stream aligned without treating an
    // arbitrary future block as valid data.
    bool has_declaration = false;
    bool is_type = false;
    std::string value;
    while (ScanTypeOrStr(is_type, value, true) == 1) {
        if (is_type) {
            return has_declaration && value == "[/create var]";
        }
        if (value.size() <= 2 || value[0] != 'o' || value[1] != ':') {
            return false;
        }
        has_declaration = true;
    }
    return false;
}

bool IsCurrentPvfResistValueEnableState(const std::string& state) {
    static const char* const kStates[] = {
        "all", "attack", "damage", "hold", "sit", "sit forever",
        "stand", "summon end"
    };
    for (size_t index = 0; index != sizeof(kStates) / sizeof(kStates[0]);
         ++index) {
        if (state == kStates[index]) {
            return true;
        }
    }
    return false;
}

bool ConsumeCurrentPvfResistValue() {
    // The feature was added after the original monster ABI.  These are the
    // eight grammar variants used by the current PVF; all fields are checked
    // so that this compatibility reader cannot hide an unknown script block.
    std::string field;
    if (ScanType(field, true) != 1) {
        return false;
    }
    if (field == "[/resist value]") {
        return true;
    }
    if (field != "[value]" || !ScanCurrentPvfInt() ||
        ScanType(field, true) != 1) {
        return false;
    }
    if (field == "[/resist value]") {
        return true;
    }

    if (field == "[angry cooltime]") {
        if (!ScanCurrentPvfInt() || ScanType(field, true) != 1) {
            return false;
        }
    }
    if (field == "[passive]") {
        for (int index = 0; index != 4; ++index) {
            if (!ScanCurrentPvfInt()) {
                return false;
            }
        }
        if (ScanType(field, true) != 1) {
            return false;
        }
        if (field == "[crazymode keep]") {
            if (!ScanCurrentPvfInt() || ScanType(field, true) != 1) {
                return false;
            }
        }
    }
    if (field == "[action delay]") {
        if (!ScanCurrentPvfInt() || ScanType(field, true) != 1) {
            return false;
        }
    }

    while (field == "[action]") {
        bool is_string = false;
        bool success = false;
        std::string action;
        (void)ScanStrOrInt(is_string, action, success, true);
        if (!success) {
            return false;
        }
        if (!is_string && ScanStr(&action) != 1) {
            return false;
        }
        if (ScanType(field, true) != 1) {
            return false;
        }
    }

    if (field == "[action condition]") {
        if (!ScanCurrentPvfInt() || ScanType(field, true) != 1 ||
            (field != "[hold]" && field != "[accept damage]") ||
            !ScanCurrentPvfInt() ||
            !ScanCurrentPvfType("[second interval]") ||
            !ScanCurrentPvfType("[/action condition]") ||
            ScanType(field, true) != 1) {
            return false;
        }
    }

    if (field != "[enable state]") {
        return false;
    }
    bool is_type = false;
    std::string state;
    int state_count = 0;
    while (ScanTypeOrStr(is_type, state, true) == 1) {
        if (is_type) {
            break;
        }
        if (!IsCurrentPvfResistValueEnableState(state)) {
            return false;
        }
        ++state_count;
    }
    return is_type && state_count != 0 && state == "[/enable state]" &&
           ScanCurrentPvfType("[/resist value]");
}

bool ConsumeCurrentPvfResistEntrySpeechTail(std::string& next_field,
                                             bool speech_required) {
    std::string speech_type;
    if (ScanStr(&speech_type) != 1) {
        return false;
    }
    if (speech_type.empty()) {
        return !speech_required && ScanType(next_field, true) == 1;
    }
    if (speech_type != "[speech]" || ScanStr(&speech_type) != 1 ||
        !ScanCurrentPvfInt() || !ScanCurrentPvfInt()) {
        return false;
    }
    return ScanType(next_field, true) == 1;
}

bool ConsumeCurrentPvfResistEntry() {
    // This post-original event table is not representable in the old monster
    // record.  Parse its eight known event records exactly; unknown events or
    // a malformed speech tail are rejected instead of being skipped.
    std::string field;
    if (ScanType(field, true) != 1) {
        return false;
    }
    while (field != "[/resist entry]") {
        std::string text;
        if (field == "[hold]") {
            if (!ScanCurrentPvfInt() || !ScanCurrentPvfInt() ||
                !ConsumeCurrentPvfResistEntrySpeechTail(field, false)) {
                return false;
            }
        } else if (field == "[on set action]") {
            if (ScanStr(&text) != 1 || !ScanCurrentPvfInt() ||
                !ScanCurrentPvfInt() || !ScanCurrentPvfInt() ||
                !ConsumeCurrentPvfResistEntrySpeechTail(field, true)) {
                return false;
            }
        } else if (field == "[active status]") {
            if (ScanStr(&text) != 1 || !ScanCurrentPvfInt() ||
                !ScanCurrentPvfInt() ||
                !ConsumeCurrentPvfResistEntrySpeechTail(field, true)) {
                return false;
            }
        } else if (field == "[accept damage]") {
            if (ScanStr(&text) != 1 || !ScanCurrentPvfInt() ||
                !ScanCurrentPvfInt() ||
                !ConsumeCurrentPvfResistEntrySpeechTail(field, false)) {
                return false;
            }
        } else if (field == "[accept hit]" || field == "[cube]" ||
                   field == "[character down]") {
            if (!ScanCurrentPvfInt() || !ScanCurrentPvfInt() ||
                !ConsumeCurrentPvfResistEntrySpeechTail(field, false)) {
                return false;
            }
        } else if (field == "[character kill]" ||
                   field == "[second interval]") {
            if (!ScanCurrentPvfInt() || !ScanCurrentPvfInt() ||
                !ConsumeCurrentPvfResistEntrySpeechTail(field, false)) {
                return false;
            }
        } else {
            return false;
        }
    }
    return true;
}

}  // namespace

// IDA_EA: 0x8349672
// IDA_NAME: _Z16_drop_item_indexiRKSt6vectorISt4pairIiiESaIS1_EE
// DEMANGLED: _drop_item_index(int,std::vector<std::pair<int,int>> const&)
int _drop_item_index(int random_value,
                     const std::vector<std::pair<int, int> >& items) {
    int cumulative_weight = 0;
    for (std::vector<std::pair<int, int> >::const_iterator item =
             items.begin();
         item != items.end();
         ++item) {
        cumulative_weight += item->second;
        if (random_value < cumulative_weight) {
            return item->first;
        }
    }
    return -2;
}

// IDA_EA: 0x830E6C6
// IDA_NAME: _ZNK8CMonster16getMonsterScriptEv
// DEMANGLED: CMonster::getMonsterScript(void)const
STMonsterScript* CMonster::getMonsterScript() const {
    return const_cast<STMonsterScript*>(&script_);
}

bool CMonster::is_getup_invincible() const {
    return script_.getup_invincible != 0;
}

bool CMonster::is_deathless() const {
    return script_.deathless != 0;
}

bool CMonster::is_hp_recovery_impossible() const {
    return script_.impossible_hp_recovery != 0;
}

bool CMonster::is_appendage_disabled() const {
    return script_.disable_appendage != 0;
}

float CMonster::champion_fastmove_factor() const {
    return script_.has_champion_fastmove_factor
               ? script_.champion_fastmove_factor
               : 1.0f;
}

float CMonster::champion_fastattack_factor() const {
    return script_.has_champion_fastattack_factor
               ? script_.champion_fastattack_factor
               : 1.0f;
}

// IDA_EA: 0x834F650
// IDA_NAME: _ZN8CMonster11set_monsterER15STMonsterScript
// DEMANGLED: CMonster::set_monster(STMonsterScript &)
int CMonster::set_monster(STMonsterScript& script) {
    index_ = MonsterScriptIndex(&script);
    for (int index = 0; index <= 3; ++index) {
        special_item_lists_[index] = script.special_item_lists[index];
    }
    special_drop_total_rate_ = script.value_88;
    special_drop_items_ = script.special_drop_items;
    raw_58_ = script.level_range[0];
    raw_5c_ = script.values_3f4[0];
    war_room_spawn_weight_ = script.value_c4c;
    is_hell_monster_ = script.value_3a8;
    death_tower_specify_items_ = script.death_tower_specify_items;
    script_ = script;
    return 1;
}

// IDA_EA: 0x83748CE
// IDA_NAME: _ZN8CMonster9get_indexEv
// DEMANGLED: CMonster::get_index(void)
int CMonster::get_index() { return index_; }

// IDA_EA: 0x8379DCE
// IDA_NAME: _ZN8CMonsterD2Ev
// DEMANGLED: CMonster::~CMonster()
CMonster::~CMonster() {
}

// IDA_EA: 0x8379EA4
// IDA_NAME: _ZN8CMonsterC2Ev
// DEMANGLED: CMonster::CMonster(void)
CMonster::CMonster()
    : raw_00_(0),
      index_(0),
      special_item_lists_(),
      special_drop_total_rate_(0),
      special_drop_items_(),
      death_tower_specify_items_(),
      is_hell_monster_(0),
      raw_58_(0),
      raw_5c_(0),
      war_room_spawn_weight_(0) {}

// IDA_EA: 0x8539DF4
// IDA_NAME: _ZNK8CMonster13IsHellMonsterEv
// DEMANGLED: CMonster::IsHellMonster(void)const
int CMonster::IsHellMonster() const { return is_hell_monster_; }

// IDA_EA: 0x834F746
// IDA_NAME: _ZNK8CMonster25get_specialItem_list_sizeEj
// DEMANGLED: CMonster::get_specialItem_list_size(unsigned int)const
int CMonster::get_specialItem_list_size(unsigned int drop_kind) const {
    if (drop_kind > 3) {
        return 0;
    }
    return static_cast<int>(special_item_lists_[drop_kind].size());
}

// IDA_EA: 0x834F776
// IDA_NAME: _ZNK8CMonster15drop_item_indexEj
// DEMANGLED: CMonster::drop_item_index(unsigned int)const
int CMonster::drop_item_index(unsigned int drop_kind) const {
    if (special_drop_items_.empty()) {
        return _drop_item_index(get_rand_int_global(10000),
                                special_item_lists_[drop_kind]);
    }
    return _drop_item_index(get_rand_int_global(special_drop_total_rate_),
                            special_drop_items_);
}

// IDA_EA: 0x834F7E4
// IDA_NAME: _ZNK8CMonster15drop_item_indexEji
// DEMANGLED: CMonster::drop_item_index(unsigned int,int)const
int CMonster::drop_item_index(unsigned int drop_kind, int random_value) const {
    if (special_drop_items_.empty()) {
        return _drop_item_index(random_value, special_item_lists_[drop_kind]);
    }
    return _drop_item_index(get_rand_int_global(special_drop_total_rate_),
                            special_drop_items_);
}

// IDA_EA: 0x834F84A
// IDA_NAME: _ZNK8CMonster25deathTowerSpecifyItemDropEv
// DEMANGLED: CMonster::deathTowerSpecifyItemDrop(void)const
int CMonster::deathTowerSpecifyItemDrop() const {
    return _drop_item_index(get_rand_int_global(10000),
                            death_tower_specify_items_);
}

// IDA_EA: 0x834F90E
// IDA_NAME: _ZN15CMonsterManagerC2Ev
// DEMANGLED: CMonsterManager::CMonsterManager(void)
CMonsterManager::CMonsterManager()
    : category_monsters_(), monsters_() {}

// IDA_EA: 0x834F954
// IDA_NAME: _ZN15CMonsterManagerD2Ev
// DEMANGLED: CMonsterManager::~CMonsterManager()
CMonsterManager::~CMonsterManager() { destroy(); }

// IDA_EA: 0x834F9BE
// IDA_NAME: _ZN15CMonsterManager7destroyEv
// DEMANGLED: CMonsterManager::destroy(void)
int CMonsterManager::destroy() {
    for (__gnu_cxx::hash_map<int, CMonster*>::iterator it =
             monsters_.begin();
         it != monsters_.end();
         ++it) {
        delete it->second;
    }
    monsters_.clear();
    return 1;
}

// IDA_EA: 0x834FA84
// IDA_NAME: _ZN15CMonsterManager13load_monstersEv
// DEMANGLED: CMonsterManager::load_monsters(void)
int CMonsterManager::load_monsters() {
    sync_script::CSyncScript* sync =
        sync_script::GetInstanceSyncScript();
    if (sync->truncate_monster_info_table() != true) {
        std::puts("Fail truncate_monster_info_table");
        return 0;
    }

    if (g_stMonsterScriptFileList.empty()) {
        return 1;
    }
    for (STScriptFileList::const_iterator it =
             g_stMonsterScriptFileList.begin();
         it != g_stMonsterScriptFileList.end();
        ++it) {
        const char* filename = it->second.c_str();
        if (filename == 0) {
            break;
        }

        STMonsterScript script;
        const int import_result =
            ImportMonsterScript(&script, 3, filename, false);
        if (import_result != 1) {
            std::printf("Monster Script Error: %s\n", filename);
            return 0;
        }

        if (sync->insert_monster_info_to_db(script) != 1) {
            std::printf("insert_monster_category Error: %s\n", filename);
            return 0;
        }

        std::vector<stIndependentDropInfo>* independent_drops =
            IndependentDropInfos();
        for (std::vector<stIndependentDropInfo>::iterator drop_it =
                 independent_drops->begin();
             drop_it != independent_drops->end();
             ++drop_it) {
            if (drop_it->dungeon_index == 0 &&
                drop_it->start_level == MonsterScriptIndex(&script)) {
                MonsterScriptIndependentDrops(&script)->push_back(
                    drop_it->drop);
            }
        }

        CMonster* monster = new CMonster;
        monster->set_monster(script);
        for (int level = MonsterScriptLevelRangeBegin(&script);
             level <= MonsterScriptLevelRangeEnd(&script);
             ++level) {
            insert_monster(level, monster->get_index());
        }
        insert_monster(monster->get_index(), monster);
    }

    return 1;
}

// IDA_EA: 0x834FDAC
// IDA_NAME: _ZN15CMonsterManager14insert_monsterEii
// DEMANGLED: CMonsterManager::insert_monster(int,int)
int CMonsterManager::insert_monster(int category, int monster_index) {
    category_monsters_.insert(std::make_pair(category, monster_index));
    return 1;
}

// IDA_EA: 0x834FDEE
// IDA_NAME: _ZN15CMonsterManager14insert_monsterEiP8CMonster
// DEMANGLED: CMonsterManager::insert_monster(int,CMonster *)
int CMonsterManager::insert_monster(int monster_index, CMonster* monster) {
    if (monsters_.find(monster_index) != monsters_.end()) {
        return 0;
    }
    monsters_.insert(std::make_pair(monster_index, monster));
    return 1;
}

// IDA_EA: 0x834FE9E
// IDA_NAME: _ZN15CMonsterManager12find_monsterEi
// DEMANGLED: CMonsterManager::find_monster(int)
CMonster* CMonsterManager::find_monster(int monster_index) {
    __gnu_cxx::hash_map<int, CMonster*>::iterator it =
        monsters_.find(monster_index);
    if (it == monsters_.end()) {
        return 0;
    }
    return it->second;
}

// IDA_EA: 0x834FF20
// IDA_NAME: _ZN15CMonsterManager19get_levelrandom_mobEi
// DEMANGLED: CMonsterManager::get_levelrandom_mob(int)
int CMonsterManager::get_levelrandom_mob(int level) {
    const int count = static_cast<int>(category_monsters_.count(level));
    if (count <= 0) {
        return 0;
    }

    int pick = get_rand_int_global(count);
    std::pair<std::multimap<int, int>::const_iterator,
              std::multimap<int, int>::const_iterator> range =
        category_monsters_.equal_range(level);
    std::multimap<int, int>::const_iterator it = range.first;
    while (pick > 0 && it != range.second) {
        --pick;
        ++it;
    }
    if (it == range.second) {
        return 0;
    }
    return it->second;
}

// IDA_EA: 0x8A4DEA0
// IDA_NAME: _ZN13CatchItemInfoD2Ev
// DEMANGLED: CatchItemInfo::~CatchItemInfo()
CatchItemInfo::~CatchItemInfo() {}

// IDA_EA: 0x88BA480
// IDA_NAME: _ZNK26STMonsterParameterCategory11getValueIntEv
// DEMANGLED: STMonsterParameterCategory::getValueInt(void)const
int STMonsterParameterCategory::getValueInt() const {
    return value_x100 / 100;
}

// IDA_EA: 0x88BA4A2
// IDA_NAME: _ZNK26STMonsterParameterCategory13getValueFloatEv
// DEMANGLED: STMonsterParameterCategory::getValueFloat(void)const
long double STMonsterParameterCategory::getValueFloat() const {
    return static_cast<long double>(value_x100) / 100.0;
}

// IDA_EA: 0x8A3CD6C
// IDA_NAME: _ZN26STMonsterParameterCategoryC2Ev
// DEMANGLED: STMonsterParameterCategory::STMonsterParameterCategory(void)
STMonsterParameterCategory::STMonsterParameterCategory()
    : parameter_type(14), parameter_index(0), value_x100(0) {}

// IDA_EA: 0x8A4BCBC
// IDA_NAME: _ZN26STMonsterParameterCategoryD2Ev
// DEMANGLED: STMonsterParameterCategory::~STMonsterParameterCategory()
STMonsterParameterCategory::~STMonsterParameterCategory() {}

// IDA_EA: 0x8A3D5FE
// IDA_NAME: _ZN14STAttackMethod5clearEv
// DEMANGLED: STAttackMethod::clear(void)
STAttackMethod* STAttackMethod::clear() {
    for (int index = 0; index <= 12; ++index) {
        values[index] = 0;
    }
    return this;
}

// IDA_EA: 0x8A3D690
// IDA_NAME: _ZN14STAttackMethodC2Ev
// DEMANGLED: STAttackMethod::STAttackMethod(void)
STAttackMethod::STAttackMethod() { clear(); }

// IDA_EA: 0x8A4BED8
// IDA_NAME: _ZN14STAttackMethodD2Ev
// DEMANGLED: STAttackMethod::~STAttackMethod()
STAttackMethod::~STAttackMethod() {}

// IDA_EA: 0x8A3D4A0
// IDA_NAME: _ZN12STDifficulty5clearEv
// DEMANGLED: STDifficulty::clear(void)
STDifficulty* STDifficulty::clear() {
    level = 0;
    value = 0;
    return this;
}

// IDA_EA: 0x8A3D4B8
// IDA_NAME: _ZN12STDifficultyC2Ev
// DEMANGLED: STDifficulty::STDifficulty(void)
STDifficulty::STDifficulty() { clear(); }

// IDA_EA: 0x8A4D8CE
// IDA_NAME: _ZN12STDifficultyD2Ev
// DEMANGLED: STDifficulty::~STDifficulty()
STDifficulty::~STDifficulty() {}

// IDA_EA: 0x8A3D542
// IDA_NAME: _ZN10STAtkSuper5clearEv
// DEMANGLED: STAtkSuper::clear(void)
STAtkSuper* STAtkSuper::clear() {
    for (int index = 0; index <= 5; ++index) {
        values[index] = -1;
    }
    return this;
}

// IDA_EA: 0x8A3D582
// IDA_NAME: _ZN10STAtkSuperC2Ev
// DEMANGLED: STAtkSuper::STAtkSuper(void)
STAtkSuper::STAtkSuper() { clear(); }

// IDA_EA: 0x8A4D502
// IDA_NAME: _ZN10STAtkSuperD2Ev
// DEMANGLED: STAtkSuper::~STAtkSuper()
STAtkSuper::~STAtkSuper() {}

// IDA_EA: 0x8A3D340
// IDA_NAME: _ZN6STDash5clearEv
// DEMANGLED: STDash::clear(void)
STDash* STDash::clear() {
    values[0] = 0;
    values[1] = -1;
    for (int index = 2; index <= 9; ++index) {
        values[index] = 0;
    }
    return this;
}

// IDA_EA: 0x8A3D3A8
// IDA_NAME: _ZN6STDashC2Ev
// DEMANGLED: STDash::STDash(void)
STDash::STDash() { clear(); }

// IDA_EA: 0x8A4CCFE
// IDA_NAME: _ZN6STDashD2Ev
// DEMANGLED: STDash::~STDash()
STDash::~STDash() {}

// IDA_EA: 0x8A3D0DE
// IDA_NAME: _ZN15STCounterAttack5clearEv
// DEMANGLED: STCounterAttack::clear(void)
STCounterAttack* STCounterAttack::clear() {
    probability = -1;
    attack_index = -1;
    damage_type = -1;
    use_need_hp = 0;
    need_hp = 0;
    enable = 0;
    return this;
}

// IDA_EA: 0x8A3D118
// IDA_NAME: _ZN15STCounterAttackC2Ev
// DEMANGLED: STCounterAttack::STCounterAttack(void)
STCounterAttack::STCounterAttack() { clear(); }

// IDA_EA: 0x8A4C4E2
// IDA_NAME: _ZN15STCounterAttackD2Ev
// DEMANGLED: STCounterAttack::~STCounterAttack()
STCounterAttack::~STCounterAttack() {}

// IDA_EA: 0x8A3D3BC
// IDA_NAME: _ZN6STHold5clearEv
// DEMANGLED: STHold::clear(void)
STHold* STHold::clear() {
    for (int index = 0; index <= 8; ++index) {
        values[index] = 0;
    }
    return this;
}

// IDA_EA: 0x8A3D41A
// IDA_NAME: _ZN6STHoldC2Ev
// DEMANGLED: STHold::STHold(void)
STHold::STHold() { clear(); }

// IDA_EA: 0x8A4CF02
// IDA_NAME: _ZN6STHoldD2Ev
// DEMANGLED: STHold::~STHold()
STHold::~STHold() {}

// IDA_EA: 0x8A3D12C
// IDA_NAME: _ZN12STJumpAttack5clearEv
// DEMANGLED: STJumpAttack::clear(void)
STJumpAttack* STJumpAttack::clear() {
    attack_index = -1;
    jump_height = -1;
    jump_speed = -1;
    collision_animation_index = -1;
    enemy_distance_rate = 1.0f;
    jump_type = -1;
    jump_direction = 0;
    start_frame_index = 1;
    end_frame_index = 0;
    limited_x_coordinate = 0;
    limited_y_coordinate = 0;
    return this;
}

// IDA_EA: 0x8A3D1A0
// IDA_NAME: _ZN12STJumpAttackC2Ev
// DEMANGLED: STJumpAttack::STJumpAttack(void)
STJumpAttack::STJumpAttack() { clear(); }

// IDA_EA: 0x8A4C6F2
// IDA_NAME: _ZN12STJumpAttackD2Ev
// DEMANGLED: STJumpAttack::~STJumpAttack()
STJumpAttack::~STJumpAttack() {}

// IDA_EA: 0x8A3D248
// IDA_NAME: _ZN12STSummonInfo5clearEv
// DEMANGLED: STSummonInfo::clear(void)
STSummonInfo* STSummonInfo::clear() {
    for (int index = 0; index <= 3; ++index) {
        values[index] = 0;
    }
    return this;
}

// IDA_EA: 0x8A3D274
// IDA_NAME: _ZN12STSummonInfoC2Ev
// DEMANGLED: STSummonInfo::STSummonInfo(void)
STSummonInfo::STSummonInfo() { clear(); }

// IDA_EA: 0x8A4C900
// IDA_NAME: _ZN12STSummonInfoD2Ev
// DEMANGLED: STSummonInfo::~STSummonInfo()
STSummonInfo::~STSummonInfo() {}

// IDA_EA: 0x83D4336
// IDA_NAME: _ZN8STSummonC2ERKS_
// DEMANGLED: STSummon::STSummon(STSummon const&)
STSummon::STSummon(const STSummon& other)
    : values_00(), summons(other.summons), values_1c() {
    for (int index = 0; index <= 3; ++index) {
        values_00[index] = other.values_00[index];
    }
    for (int index = 0; index <= 4; ++index) {
        values_1c[index] = other.values_1c[index];
    }
}

// IDA_EA: 0x83DC27C
// IDA_NAME: _ZN8STSummonD2Ev
// DEMANGLED: STSummon::~STSummon()
STSummon::~STSummon() {}

// IDA_EA: 0x83E17EA
// IDA_NAME: _ZN8STSummonaSERKS_
// DEMANGLED: STSummon::operator=(STSummon const&)
STSummon& STSummon::operator=(const STSummon& other) {
    for (int index = 0; index <= 3; ++index) {
        values_00[index] = other.values_00[index];
    }
    summons = other.summons;
    for (int index = 0; index <= 4; ++index) {
        values_1c[index] = other.values_1c[index];
    }
    return *this;
}

// IDA_EA: 0x8A3D288
// IDA_NAME: _ZN8STSummon5clearEv
// DEMANGLED: STSummon::clear(void)
STSummon* STSummon::clear() {
    values_00[0] = 0;
    values_00[1] = -1;
    values_00[2] = 0;
    values_00[3] = 0;
    summons.clear();
    for (int index = 0; index <= 4; ++index) {
        values_1c[index] = 0;
    }
    return this;
}

// IDA_EA: 0x8A3D2F8
// IDA_NAME: _ZN8STSummonC2Ev
// DEMANGLED: STSummon::STSummon(void)
STSummon::STSummon() : values_00(), summons(), values_1c() { clear(); }

// IDA_EA: 0x8A3D42E
// IDA_NAME: _ZN6STWarp5clearEv
// DEMANGLED: STWarp::clear(void)
STWarp* STWarp::clear() {
    function_index = -1;
    attack_index = -1;
    warp_style = -1;
    after_attack = -1;
    after_appear_time = -1.0f;
    cool_time = -1;
    target_offset = -1;
    passive_object_index = -1;
    target_category = 44;
    return this;
}

// IDA_EA: 0x8A3D48C
// IDA_NAME: _ZN6STWarpC2Ev
// DEMANGLED: STWarp::STWarp(void)
STWarp::STWarp() { clear(); }

// IDA_EA: 0x8A4D106
// IDA_NAME: _ZN6STWarpD2Ev
// DEMANGLED: STWarp::~STWarp()
STWarp::~STWarp() {}

// IDA_EA: 0x83D4D64
// IDA_NAME: _ZN10STCoolTimeC2ERKS_
// DEMANGLED: STCoolTime::STCoolTime(STCoolTime const&)
STCoolTime::STCoolTime(const STCoolTime& other)
    : value_00(other.value_00), values_04(other.values_04) {}

// IDA_EA: 0x83DC2D0
// IDA_NAME: _ZN10STCoolTimeD2Ev
// DEMANGLED: STCoolTime::~STCoolTime()
STCoolTime::~STCoolTime() {}

// IDA_EA: 0x83E23D6
// IDA_NAME: _ZN10STCoolTimeaSERKS_
// DEMANGLED: STCoolTime::operator=(STCoolTime const&)
STCoolTime& STCoolTime::operator=(const STCoolTime& other) {
    value_00 = other.value_00;
    values_04 = other.values_04;
    return *this;
}

// IDA_EA: 0x8A3D596
// IDA_NAME: _ZN10STCoolTime5clearEv
// DEMANGLED: STCoolTime::clear(void)
void STCoolTime::clear() {
    value_00 = 0;
    values_04.clear();
}

// IDA_EA: 0x8A3D5B6
// IDA_NAME: _ZN10STCoolTimeC2Ev
// DEMANGLED: STCoolTime::STCoolTime(void)
STCoolTime::STCoolTime() : value_00(0), values_04() { clear(); }

// IDA_EA: 0x83E2624
// IDA_NAME: _ZN15STInitialTargetaSERKS_
// DEMANGLED: STInitialTarget::operator=(STInitialTarget const&)
STInitialTarget& STInitialTarget::operator=(const STInitialTarget& other) {
    target_index = other.target_index;
    target_name = other.target_name;
    value_08 = other.value_08;
    value_0c = other.value_0c;
    return *this;
}

// IDA_EA: 0x83ED0FE
// IDA_NAME: _ZN15STInitialTargetD2Ev
// DEMANGLED: STInitialTarget::~STInitialTarget()
STInitialTarget::~STInitialTarget() {}

// IDA_EA: 0x83EE8BA
// IDA_NAME: _ZN15STInitialTargetC2ERKS_
// DEMANGLED: STInitialTarget::STInitialTarget(STInitialTarget const&)
STInitialTarget::STInitialTarget(const STInitialTarget& other)
    : target_index(other.target_index),
      target_name(other.target_name),
      value_08(other.value_08),
      value_0c(other.value_0c) {}

// IDA_EA: 0x8A3D892
// IDA_NAME: _ZN15STInitialTargetC2Ev
// DEMANGLED: STInitialTarget::STInitialTarget(void)
STInitialTarget::STInitialTarget()
    : target_index(-1), target_name(), value_08(-1), value_0c(1) {}

// IDA_EA: 0x83D55D2
// IDA_NAME: _ZN19STMosterAttackReadyC2ERKS_
// DEMANGLED: STMosterAttackReady::STMosterAttackReady(STMosterAttackReady const&)
STMosterAttackReady::STMosterAttackReady(const STMosterAttackReady& other)
    : layer_indices(other.layer_indices),
      motion_index(other.motion_index),
      effect_animation_files(other.effect_animation_files) {}

// IDA_EA: 0x83DC3C6
// IDA_NAME: _ZN19STMosterAttackReadyD2Ev
// DEMANGLED: STMosterAttackReady::~STMosterAttackReady()
STMosterAttackReady::~STMosterAttackReady() {}

// IDA_EA: 0x83E33B8
// IDA_NAME: _ZN19STMosterAttackReadyaSERKS_
// DEMANGLED: STMosterAttackReady::operator=(STMosterAttackReady const&)
STMosterAttackReady& STMosterAttackReady::operator=(
    const STMosterAttackReady& other) {
    layer_indices = other.layer_indices;
    motion_index = other.motion_index;
    effect_animation_files = other.effect_animation_files;
    return *this;
}

// IDA_EA: 0x8A3D1B4
// IDA_NAME: _ZN19STMosterAttackReady5clearEv
// DEMANGLED: STMosterAttackReady::clear(void)
void STMosterAttackReady::clear() {
    motion_index = -1;
    effect_animation_files.clear();
    layer_indices.clear();
}

// IDA_EA: 0x8A3D1E0
// IDA_NAME: _ZN19STMosterAttackReadyC2Ev
// DEMANGLED: STMosterAttackReady::STMosterAttackReady(void)
STMosterAttackReady::STMosterAttackReady()
    : layer_indices(), motion_index(0), effect_animation_files() {
    clear();
}

// IDA_EA: 0x83D3D4E
// IDA_NAME: _ZN13STThrowAttackC2ERKS_
// DEMANGLED: STThrowAttack::STThrowAttack(STThrowAttack const&)
STThrowAttack::STThrowAttack(const STThrowAttack& other)
    : passive_object_paths(other.passive_object_paths),
      value_0c(other.value_0c),
      value_10(other.value_10),
      value_14(other.value_14),
      start_x_coordinates(other.start_x_coordinates),
      start_y_coordinates(other.start_y_coordinates),
      start_z_coordinates(other.start_z_coordinates),
      attack_number(other.attack_number),
      throw_sound(other.throw_sound),
      values_44(),
      values_58(other.values_58) {
    for (int index = 0; index <= 4; ++index) {
        values_44[index] = other.values_44[index];
    }
}

// IDA_EA: 0x83DC172
// IDA_NAME: _ZN13STThrowAttackD2Ev
// DEMANGLED: STThrowAttack::~STThrowAttack()
STThrowAttack::~STThrowAttack() {}

// IDA_EA: 0x83E1200
// IDA_NAME: _ZN13STThrowAttackaSERKS_
// DEMANGLED: STThrowAttack::operator=(STThrowAttack const&)
STThrowAttack& STThrowAttack::operator=(const STThrowAttack& other) {
    passive_object_paths = other.passive_object_paths;
    value_0c = other.value_0c;
    value_10 = other.value_10;
    value_14 = other.value_14;
    start_x_coordinates = other.start_x_coordinates;
    start_y_coordinates = other.start_y_coordinates;
    start_z_coordinates = other.start_z_coordinates;
    attack_number = other.attack_number;
    throw_sound = other.throw_sound;
    for (int index = 0; index <= 4; ++index) {
        values_44[index] = other.values_44[index];
    }
    values_58 = other.values_58;
    return *this;
}

// IDA_EA: 0x8A3CF40
// IDA_NAME: _ZN13STThrowAttack5clearEv
// DEMANGLED: STThrowAttack::clear(void)
STThrowAttack* STThrowAttack::clear() {
    value_0c = 0;
    value_10 = -1;
    value_14 = -1;
    start_x_coordinates.clear();
    start_y_coordinates.clear();
    start_z_coordinates.clear();
    attack_number = -1;
    values_44[0] = -1;
    values_44[1] = -1;
    values_44[2] = 0;
    values_44[3] = -1;
    values_44[4] = 0;
    values_58.clear();
    return this;
}

// IDA_EA: 0x8A3CFDA
// IDA_NAME: _ZN13STThrowAttackC2Ev
// DEMANGLED: STThrowAttack::STThrowAttack(void)
STThrowAttack::STThrowAttack()
    : passive_object_paths(),
      value_0c(0),
      value_10(0),
      value_14(0),
      start_x_coordinates(),
      start_y_coordinates(),
      start_z_coordinates(),
      attack_number(0),
      throw_sound(),
      values_44(),
      values_58() {
    clear();
}

// IDA_EA: 0x83D5B08
// IDA_NAME: _ZN14STRidableSkillC2ERKS_
// DEMANGLED: STRidableSkill::STRidableSkill(STRidableSkill const&)
STRidableSkill::STRidableSkill(const STRidableSkill& other)
    : pattern(other.pattern),
      image_path(other.image_path),
      keys(),
      parameters(),
      modes(),
      values_a(),
      values_b(),
      values_c() {
    for (int index = 0; index <= 7; ++index) {
        keys[index] = other.keys[index];
        parameters[index] = other.parameters[index];
        modes[index] = other.modes[index];
        values_a[index] = other.values_a[index];
        values_b[index] = other.values_b[index];
        values_c[index] = other.values_c[index];
    }
}

// IDA_EA: 0x83DC460
// IDA_NAME: _ZN14STRidableSkillD2Ev
// DEMANGLED: STRidableSkill::~STRidableSkill()
STRidableSkill::~STRidableSkill() {}

// IDA_EA: 0x83E389C
// IDA_NAME: _ZN14STRidableSkillaSERKS_
// DEMANGLED: STRidableSkill::operator=(STRidableSkill const&)
STRidableSkill& STRidableSkill::operator=(const STRidableSkill& other) {
    pattern = other.pattern;
    image_path = other.image_path;
    for (int index = 0; index <= 7; ++index) {
        keys[index] = other.keys[index];
        parameters[index] = other.parameters[index];
        modes[index] = other.modes[index];
        values_a[index] = other.values_a[index];
        values_b[index] = other.values_b[index];
        values_c[index] = other.values_c[index];
    }
    return *this;
}

// IDA_EA: 0x8A3D73E
// IDA_NAME: _ZN14STRidableSkillC2Ev
// DEMANGLED: STRidableSkill::STRidableSkill(void)
STRidableSkill::STRidableSkill()
    : pattern(0),
      image_path(),
      keys(),
      parameters(),
      modes(),
      values_a(),
      values_b(),
      values_c() {
    image_path.clear();
    for (int index = 0; index <= 7; ++index) {
        keys[index] = -1;
        modes[index] = 6;
        values_a[index] = -1;
        values_b[index] = -1;
        values_c[index] = -1;
    }
}

// IDA_EA: 0x83D5F64
// IDA_NAME: _ZN21STRidableSkillExplainC2ERKS_
// DEMANGLED: STRidableSkillExplain::STRidableSkillExplain(STRidableSkillExplain const&)
STRidableSkillExplain::STRidableSkillExplain(
    const STRidableSkillExplain& other)
    : title(other.title),
      subtitle(other.subtitle),
      category(other.category),
      detail(other.detail),
      pattern(other.pattern) {}

// IDA_EA: 0x83DC4DE
// IDA_NAME: _ZN21STRidableSkillExplainD2Ev
// DEMANGLED: STRidableSkillExplain::~STRidableSkillExplain()
STRidableSkillExplain::~STRidableSkillExplain() {}

// IDA_EA: 0x83E3CAE
// IDA_NAME: _ZN21STRidableSkillExplainaSERKS_
// DEMANGLED: STRidableSkillExplain::operator=(STRidableSkillExplain const&)
STRidableSkillExplain& STRidableSkillExplain::operator=(
    const STRidableSkillExplain& other) {
    title = other.title;
    subtitle = other.subtitle;
    category = other.category;
    detail = other.detail;
    pattern = other.pattern;
    return *this;
}

// IDA_EA: 0x8A3D6A4
// IDA_NAME: _ZN21STRidableSkillExplainC2Ev
// DEMANGLED: STRidableSkillExplain::STRidableSkillExplain(void)
STRidableSkillExplain::STRidableSkillExplain()
    : title(), subtitle(), category(), detail(), pattern(0) {}

// IDA_EA: 0x88CFAB4
// IDA_NAME: _ZN26stEnamyDropAvatarElement_tD2Ev
// DEMANGLED: stEnamyDropAvatarElement_t::~stEnamyDropAvatarElement_t()
stEnamyDropAvatarElement_t::~stEnamyDropAvatarElement_t() {}

// IDA_EA: 0x8366EA8
// IDA_NAME: _ZN23stEnamyDropAvatarInfo_taSERKS_
// DEMANGLED: stEnamyDropAvatarInfo_t::operator=(stEnamyDropAvatarInfo_t const&)
stEnamyDropAvatarInfo_t& stEnamyDropAvatarInfo_t::operator=(
    const stEnamyDropAvatarInfo_t& other) {
    base_rate = other.base_rate;
    drop_count = other.drop_count;
    for (int index = 0; index <= 3; ++index) {
        part_rates[index] = other.part_rates[index];
    }
    for (int index = 0; index <= 4; ++index) {
        grade_rates[index] = other.grade_rates[index];
    }
    total_rate = other.total_rate;
    elements = other.elements;
    return *this;
}

// IDA_EA: 0x8368DE8
// IDA_NAME: _ZN23stEnamyDropAvatarInfo_tC2Ev
// DEMANGLED: stEnamyDropAvatarInfo_t::stEnamyDropAvatarInfo_t(void)
stEnamyDropAvatarInfo_t::stEnamyDropAvatarInfo_t() : elements() {}

// IDA_EA: 0x8368DFE
// IDA_NAME: _ZN23stEnamyDropAvatarInfo_tD2Ev
// DEMANGLED: stEnamyDropAvatarInfo_t::~stEnamyDropAvatarInfo_t()
stEnamyDropAvatarInfo_t::~stEnamyDropAvatarInfo_t() {}

// IDA_EA: 0x88CC10C
// IDA_NAME: _ZN23stEnamyDropAvatarInfo_t5clearEv
// DEMANGLED: stEnamyDropAvatarInfo_t::clear(void)
void stEnamyDropAvatarInfo_t::clear() {
    base_rate = 0;
    drop_count = 0;
    for (int index = 0; index <= 3; ++index) {
        part_rates[index] = 0.0f;
    }
    for (int index = 0; index <= 4; ++index) {
        grade_rates[index] = 0.0f;
    }
    total_rate = 0;
    elements.clear();
}

// IDA_EA: 0x8368D7E
// IDA_NAME: _ZN18STMonsterEquipment5clearEv
// DEMANGLED: STMonsterEquipment::clear(void)
void STMonsterEquipment::clear() {
    std::vector<std::string>::clear();
}

// IDA_EA: 0x8368D92
// IDA_NAME: _ZN18STMonsterEquipmentC2Ev
// DEMANGLED: STMonsterEquipment::STMonsterEquipment(void)
STMonsterEquipment::STMonsterEquipment() : std::vector<std::string>() {
    clear();
}

// IDA_EA: 0x8368DD4
// IDA_NAME: _ZN18STMonsterEquipmentD2Ev
// DEMANGLED: STMonsterEquipment::~STMonsterEquipment()
STMonsterEquipment::~STMonsterEquipment() {}

// IDA_EA: 0x8377976
// IDA_NAME: _ZN18STMonsterEquipmentaSERKS_
// DEMANGLED: STMonsterEquipment::operator=(STMonsterEquipment const&)
STMonsterEquipment& STMonsterEquipment::operator=(
    const STMonsterEquipment& other) {
    std::vector<std::string>::operator=(other);
    return *this;
}

// IDA_EA: 0x8368E14
// IDA_NAME: _ZN15STMonsterScriptC2Ev
// DEMANGLED: STMonsterScript::STMonsterScript(void)
STMonsterScript::STMonsterScript() {
    Clear();
}

// IDA_EA: 0x8377994
// IDA_NAME: _ZN15STMonsterScriptaSERKS_
// DEMANGLED: STMonsterScript::operator=(STMonsterScript const&)
STMonsterScript& STMonsterScript::operator=(const STMonsterScript& other) {
    index = other.index;
    name = other.name;
    name_08 = other.name_08;
    for (int index_0c = 0; index_0c <= 3; ++index_0c) {
        values_0c[index_0c] = other.values_0c[index_0c];
        special_item_lists[index_0c] = other.special_item_lists[index_0c];
    }
    values_1c = other.values_1c;
    death_tower_specify_items = other.death_tower_specify_items;
    for (int index_64 = 0; index_64 <= 2; ++index_64) {
        values_64[index_64] = other.values_64[index_64];
    }
    catch_items = other.catch_items;
    parameter_categories = other.parameter_categories;
    value_88 = other.value_88;
    special_drop_items = other.special_drop_items;
    burning_death_path = other.burning_death_path;
    throw_attacks = other.throw_attacks;
    counter_attacks = other.counter_attacks;
    jump_attacks = other.jump_attacks;
    summons = other.summons;
    dash_attacks = other.dash_attacks;
    hold_attacks = other.hold_attacks;
    warp_attacks = other.warp_attacks;
    difficulties = other.difficulties;
    event_supers = other.event_supers;
    attack_supers = other.attack_supers;
    cooltimes = other.cooltimes;
    value_120 = other.value_120;
    base_parameters = other.base_parameters;
    ai_info = other.ai_info;
    value_33c = other.value_33c;
    initial_targets = other.initial_targets;
    value_368 = other.value_368;
    values_36c[0] = other.values_36c[0];
    values_36c[1] = other.values_36c[1];
    value_374 = other.value_374;
    for (int index_378 = 0; index_378 <= 2; ++index_378) {
        values_378[index_378] = other.values_378[index_378];
    }
    guards = other.guards;
    equipment = other.equipment;
    ready_actions = other.ready_actions;
    value_3a8 = other.value_3a8;
    independent_drops = other.independent_drops;
    value_3b8 = other.value_3b8;
    value_3bc = other.value_3bc;
    value_3c0 = other.value_3c0;
    for (int index_3c4 = 0; index_3c4 <= 9; ++index_3c4) {
        values_3c4[index_3c4] = other.values_3c4[index_3c4];
    }
    for (int index_3ec = 0; index_3ec <= 1; ++index_3ec) {
        level_range[index_3ec] = other.level_range[index_3ec];
    }
    for (int index_3f4 = 0; index_3f4 <= 7; ++index_3f4) {
        values_3f4[index_3f4] = other.values_3f4[index_3f4];
    }
    for (int index_414 = 0; index_414 <= 33; ++index_414) {
        values_414[index_414] = other.values_414[index_414];
    }
    for (int index_49c = 0; index_49c <= 21; ++index_49c) {
        values_49c[index_49c] = other.values_49c[index_49c];
    }
    for (int index_4f4 = 0; index_4f4 <= 4; ++index_4f4) {
        values_4f4[index_4f4] = other.values_4f4[index_4f4];
    }
    for (int index_508 = 0; index_508 <= 7; ++index_508) {
        values_508[index_508] = other.values_508[index_508];
    }
    values_528[0] = other.values_528[0];
    values_528[1] = other.values_528[1];
    strings_530[0] = other.strings_530[0];
    strings_530[1] = other.strings_530[1];
    for (int index_538 = 0; index_538 <= 3; ++index_538) {
        values_538[index_538] = other.values_538[index_538];
    }
    value_548 = other.value_548;
    value_54c = other.value_54c;
    value_550 = other.value_550;
    values_554 = other.values_554;
    values_560 = other.values_560;
    value_56c = other.value_56c;
    value_570 = other.value_570;
    for (int index_574 = 0; index_574 <= 2; ++index_574) {
        values_574[index_574] = other.values_574[index_574];
    }
    values_580[0] = other.values_580[0];
    values_580[1] = other.values_580[1];
    for (int index_584 = 0; index_584 <= 2; ++index_584) {
        values_584[index_584] = other.values_584[index_584];
        values_590[index_584] = other.values_590[index_584];
    }
    for (int index_594 = 0; index_594 <= 3; ++index_594) {
        values_594[index_594] = other.values_594[index_594];
    }
    values_5a4[0] = other.values_5a4[0];
    values_5a4[1] = other.values_5a4[1];
    value_5a8 = other.value_5a8;
    value_5ac = other.value_5ac;
    value_5b0 = other.value_5b0;
    for (int index_5b4 = 0; index_5b4 <= 5; ++index_5b4) {
        strings_5b4[index_5b4] = other.strings_5b4[index_5b4];
    }
    for (int index_5cc = 0; index_5cc <= 1; ++index_5cc) {
        strings_5cc[index_5cc] = other.strings_5cc[index_5cc];
    }
    for (int index_5d4 = 0; index_5d4 <= 3; ++index_5d4) {
        strings_5d4[index_5d4] = other.strings_5d4[index_5d4];
    }
    values_5e4 = other.values_5e4;
    values_5f0 = other.values_5f0;
    for (int index_5fc = 0; index_5fc <= 5; ++index_5fc) {
        strings_5fc[index_5fc] = other.strings_5fc[index_5fc];
    }
    for (int index_614 = 0; index_614 <= 1; ++index_614) {
        strings_614[index_614] = other.strings_614[index_614];
    }
    for (int index_61c = 0; index_61c <= 3; ++index_61c) {
        strings_61c[index_61c] = other.strings_61c[index_61c];
    }
    values_62c = other.values_62c;
    values_638 = other.values_638;
    for (int index_644 = 0; index_644 <= 4; ++index_644) {
        strings_644[index_644] = other.strings_644[index_644];
    }
    for (int index_658 = 0; index_658 <= 6; ++index_658) {
        values_658[index_658] = other.values_658[index_658];
    }
    value_674 = other.value_674;
    values_678[0] = other.values_678[0];
    values_678[1] = other.values_678[1];
    value_680 = other.value_680;
    value_684 = other.value_684;
    patterns = other.patterns;
    for (int index_694 = 0; index_694 <= 8; ++index_694) {
        strings_694[index_694] = other.strings_694[index_694];
    }
    values_6b8 = other.values_6b8;
    values_6c4 = other.values_6c4;
    value_6d0 = other.value_6d0;
    values_6d4 = other.values_6d4;
    value_6e0 = other.value_6e0;
    value_6e4 = other.value_6e4;
    attack_methods = other.attack_methods;
    for (int index_6f4 = 0; index_6f4 <= 4; ++index_6f4) {
        values_6f4[index_6f4] = other.values_6f4[index_6f4];
    }
    value_708 = other.value_708;
    for (int motion_index = 0; motion_index <= 110; ++motion_index) {
        motion_data[motion_index] = other.motion_data[motion_index];
    }
    values_c40 = other.values_c40;
    value_c4c = other.value_c4c;
    value_c50 = other.value_c50;
    for (int index_c54 = 0; index_c54 <= 6; ++index_c54) {
        values_c54[index_c54] = other.values_c54[index_c54];
    }
    values_c70[0] = other.values_c70[0];
    values_c70[1] = other.values_c70[1];
    for (int index_c74 = 0; index_c74 <= 2; ++index_c74) {
        values_c74[index_c74] = other.values_c74[index_c74];
    }
    values_c80 = other.values_c80;
    values_c8c = other.values_c8c;
    enemy_drop_avatar = other.enemy_drop_avatar;
    value_cd4 = other.value_cd4;
    value_cd8 = other.value_cd8;
    ridable_skills = other.ridable_skills;
    ridable_skill_explains = other.ridable_skill_explains;
    for (int difficulty_index = 0; difficulty_index <= 199;
         ++difficulty_index) {
        for (int parameter_index = 0; parameter_index <= 4;
             ++parameter_index) {
            physical_attack_by_difficulty[difficulty_index][parameter_index] =
                other.physical_attack_by_difficulty[difficulty_index]
                                                 [parameter_index];
        }
    }
    values_1c94[0] = other.values_1c94[0];
    values_1c94[1] = other.values_1c94[1];
    value_1c9c = other.value_1c9c;
    value_1ca0 = other.value_1ca0;
    value_1ca4 = other.value_1ca4;
    value_1ca8 = other.value_1ca8;
    values_1cac[0] = other.values_1cac[0];
    values_1cac[1] = other.values_1cac[1];
    can_beat_indices = other.can_beat_indices;
    value_1cc0 = other.value_1cc0;
    getup_invincible = other.getup_invincible;
    deathless = other.deathless;
    impossible_hp_recovery = other.impossible_hp_recovery;
    disable_appendage = other.disable_appendage;
    has_champion_fastmove_factor =
        other.has_champion_fastmove_factor;
    has_champion_fastattack_factor =
        other.has_champion_fastattack_factor;
    reserved_1cca[0] = other.reserved_1cca[0];
    reserved_1cca[1] = other.reserved_1cca[1];
    champion_fastmove_factor = other.champion_fastmove_factor;
    champion_fastattack_factor = other.champion_fastattack_factor;
    return *this;
}

// IDA_EA: 0x8378DAC
// IDA_NAME: _ZN15STMonsterScriptD2Ev
// DEMANGLED: STMonsterScript::~STMonsterScript()
STMonsterScript::~STMonsterScript() {}

// IDA_EA: 0x8A398AA
// IDA_NAME: _ZN15STMonsterScript5ClearEv
// DEMANGLED: STMonsterScript::Clear(void)
STMonsterScript* STMonsterScript::Clear() {
    value_c50 = 0;
    value_3a8 = 0;
    ai_info.clear();
    value_33c = 1;
    value_368 = 1;
    values_36c[0] = 0;
    values_36c[1] = 0;
    value_374 = 0;
    values_378[0] = 0;
    values_378[1] = 0;
    values_378[2] = 0;
    index = 0;
    name.clear();
    name_08.clear();
    values_0c[0] = 40;
    values_0c[1] = 20;
    values_0c[2] = 0;
    values_0c[3] = 1;
    values_1c.clear();
    for (int index_28 = 0; index_28 <= 3; ++index_28) {
        special_item_lists[index_28].clear();
    }
    parameter_categories.clear();
    value_3b8 = 1;
    value_3bc = 0;
    value_3c0 = 0;
    for (int index_3c4 = 0; index_3c4 <= 9; ++index_3c4) {
        values_3c4[index_3c4] = 0;
    }
    for (int index_3ec = 0; index_3ec <= 1; ++index_3ec) {
        level_range[index_3ec] = 0;
    }
    for (int index_3f4 = 0; index_3f4 <= 7; ++index_3f4) {
        values_3f4[index_3f4] = 0;
    }
    for (int index_414 = 0; index_414 <= 33; ++index_414) {
        values_414[index_414] = 0;
    }
    for (int index_49c = 0; index_49c <= 21; ++index_49c) {
        values_49c[index_49c] = 0;
    }
    values_49c[14] = 100;
    values_49c[15] = 100;
    for (int index_4f4 = 0; index_4f4 <= 3; ++index_4f4) {
        values_4f4[index_4f4] = 0;
    }
    values_4f4[4] = 200;
    for (int index_508 = 0; index_508 <= 7; ++index_508) {
        values_508[index_508] = 0;
    }
    values_528[0] = 0;
    values_528[1] = 0;
    strings_530[0].clear();
    strings_530[1].clear();
    values_538[0] = 100;
    values_538[1] = 200;
    values_538[2] = -1082130432;
    values_538[3] = -1082130432;
    value_548 = 0;
    value_54c = 20.0f;
    value_550 = -300.0f;
    values_554.clear();
    values_560.clear();
    value_56c = 0;
    values_574[0] = 1000;
    values_574[1] = 50;
    values_574[2] = 1000;
    values_580[0] = 0;
    values_580[1] = 0;
    for (int index_584 = 0; index_584 <= 2; ++index_584) {
        values_584[index_584] = 0;
        values_590[index_584] = 0;
    }
    for (int index_594 = 0; index_594 <= 3; ++index_594) {
        values_594[index_594] = 0;
    }
    values_5a4[0] = 0;
    values_5a4[1] = 0;
    value_5a8 = 0;
    value_5ac.clear();
    value_5b0 = 0;
    strings_5b4[0].clear();
    strings_5b4[1].clear();
    strings_5b4[5].clear();
    strings_5cc[0].clear();
    strings_5cc[1].clear();
    for (int index_5d4 = 0; index_5d4 <= 3; ++index_5d4) {
        strings_5d4[index_5d4].clear();
    }
    values_5e4.clear();
    values_5f0.clear();
    for (int index_5fc = 0; index_5fc <= 5; ++index_5fc) {
        strings_5fc[index_5fc].clear();
    }
    for (int index_614 = 0; index_614 <= 1; ++index_614) {
        strings_614[index_614].clear();
    }
    for (int index_61c = 0; index_61c <= 3; ++index_61c) {
        strings_61c[index_61c].clear();
    }
    values_62c.clear();
    values_638.clear();
    strings_644[0].clear();
    strings_644[1].clear();
    strings_644[2].clear();
    strings_644[3].clear();
    strings_644[4].clear();
    for (int index_658 = 0; index_658 <= 6; ++index_658) {
        values_658[index_658] = 0;
    }
    value_674 = 0;
    values_678[0] = 0;
    values_678[1] = 0;
    value_680 = 0;
    value_684 = 0;
    patterns.clear();
    for (int index_694 = 0; index_694 <= 8; ++index_694) {
        strings_694[index_694].clear();
    }
    values_6b8.clear();
    values_6c4.clear();
    values_6d4.clear();
    value_6e0 = -1;
    value_6e4 = 0;
    attack_methods.clear();
    for (int index_6f4 = 0; index_6f4 <= 4; ++index_6f4) {
        values_6f4[index_6f4] = 0;
    }
    value_708.clear();
    for (int motion_index = 0; motion_index <= 110; ++motion_index) {
        motion_data[motion_index].clear();
    }
    values_c40.clear();
    value_c4c = 0;
    value_c50 = 0;
    for (int index_c54 = 0; index_c54 <= 6; ++index_c54) {
        values_c54[index_c54] = 0;
    }
    values_c70[0] = 0;
    values_c70[1] = 0;
    values_c74[0] = 0;
    values_c74[1] = 0;
    values_c74[2] = 0;
    values_c80.clear();
    values_c8c.clear();
    enemy_drop_avatar.clear();
    value_cd4 = 0;
    value_cd8 = 4;
    for (int difficulty_index = 0; difficulty_index <= 199;
         ++difficulty_index) {
        for (int parameter_index = 0; parameter_index <= 4;
             ++parameter_index) {
            physical_attack_by_difficulty[difficulty_index][parameter_index] =
                -1.0f;
        }
    }
    values_1c94[0] = 0;
    values_1c94[1] = 0;
    value_1c9c = 3;
    value_1ca0 = 0;
    value_1ca4 = 0.0f;
    value_1ca8 = 0;
    catch_items.clear();
    values_1cac[0] = -1;
    values_1cac[1] = -1;
    can_beat_indices.clear();
    strings_5fc[2].clear();
    strings_5fc[3].clear();
    strings_5fc[4].clear();
    values_658[0] = -1;
    values_658[1] = 0;
    values_658[2] = 0;
    values_658[3] = 0;
    values_658[4] = 0;
    values_658[5] = 0;
    values_658[6] = 0;
    value_674 = 0;
    values_678[0] = 255;
    values_678[1] = 255;
    value_680 = 0;
    value_684 = 4;
    value_1cc0 = 0;
    getup_invincible = 0;
    deathless = 0;
    impossible_hp_recovery = 0;
    disable_appendage = 0;
    has_champion_fastmove_factor = 0;
    has_champion_fastattack_factor = 0;
    reserved_1cca[0] = 0;
    reserved_1cca[1] = 0;
    champion_fastmove_factor = 1.0f;
    champion_fastattack_factor = 1.0f;
    return this;
}

// IDA_EA: 0x8A3C67D
// IDA_NAME: _Z16isValidExtensionRKSsS0_
// DEMANGLED: isValidExtension(std::string const&,std::string const&)
int isValidExtension(const std::string& filename,
                     const std::string& extension) {
    const size_t filename_length = filename.length();
    const size_t extension_length = extension.length();
    if (filename_length == 0) {
        return 1;
    }
    if (filename_length < extension_length) {
        return 0;
    }
    return filename.substr(filename_length - extension_length,
                           extension_length) == extension;
}

// IDA_EA: 0x8A3C736
// IDA_NAME: _Z22ImportMonsterBaseTableP15STMonsterScriptPKc
// DEMANGLED: ImportMonsterBaseTable(STMonsterScript *,char const*)
int ImportMonsterBaseTable(STMonsterScript* script, const char* filename) {
    if (!script || !filename ||
        !loadRDARScriptFile(g_szMonsterScriptBaseDirectory, filename)) {
        return 0;
    }

    bool success = false;
    for (size_t index = 0;; ++index) {
        MonsterBaseParameter parameter;
        parameter.monster_level = ScanInt(&success);
        if (!success) {
            break;
        }

        parameter.physical_attack = ScanInt(static_cast<bool*>(0));
        parameter.physical_defense = ScanInt(static_cast<bool*>(0));
        parameter.magical_attack = ScanInt(static_cast<bool*>(0));
        parameter.magical_defense = ScanInt(static_cast<bool*>(0));
        parameter.hp_max = ScanInt(static_cast<bool*>(0));
        parameter.mp_max = ScanInt(static_cast<bool*>(0));
        parameter.equipment_physical_attack =
            ScanInt(static_cast<bool*>(0));
        parameter.equipment_physical_defense =
            ScanInt(static_cast<bool*>(0));
        parameter.equipment_magical_attack =
            ScanInt(static_cast<bool*>(0));
        parameter.equipment_magical_defense =
            ScanInt(static_cast<bool*>(0));
        parameter.fractional_combat_bonus_x100 =
            ScanInt(static_cast<bool*>(0));
        parameter.stuck_resistance_bonus_x10 =
            ScanInt(static_cast<bool*>(0));
        for (int element = 0; element <= 3; ++element) {
            parameter.element_tolerance[element] =
                ScanInt(static_cast<bool*>(0));
        }
        for (int status = 0; status <= 3; ++status) {
            parameter.element_attack[status] =
                ScanInt(static_cast<bool*>(0));
        }
        parameter.hit_recovery = ScanInt(static_cast<bool*>(0));
        parameter.rigidity = ScanInt(static_cast<bool*>(0));
        parameter.active_status_resistance =
            ScanInt(static_cast<bool*>(0));

        if (index == script->base_parameters.size()) {
            script->base_parameters.push_back(parameter);
        } else {
            if (index > script->base_parameters.size()) {
                script->base_parameters.resize(index + 1);
            }
            script->base_parameters[index] = parameter;
        }
    }
    return 1;
}

// IDA_EA: 0x8A12827
// IDA_NAME: _Z19ImportMonsterScriptP15STMonsterScriptiPKcb
// DEMANGLED: ImportMonsterScript(STMonsterScript *,int,char const*,bool)
int ImportMonsterScript(STMonsterScript* script,
                        int module_type,
                        const char* filename,
                        bool server_only) {
    if (!script || !filename ||
        !loadRDARScriptFile(g_szMonsterScriptBaseDirectory, filename)) {
        return 0;
    }

    script->Clear();
    getDirectoryPath(script->value_708,
                     "%s%s",
                     g_szMonsterScriptBaseDirectory,
                     filename);
    script->ai_info.value_0C = script->value_708;
    script->throw_attacks.clear();
    script->counter_attacks.clear();
    script->jump_attacks.clear();
    script->equipment.clear();
    script->ready_actions.clear();

    static const char* const kParameterCategoryName[14] = {
        "[HP MAX]",
        "[MP MAX]",
        "[PHYSICAL_ATTACK]",
        "[PHYSICAL_DEFENSE]",
        "[MAGICAL_ATTACK]",
        "[MAGICAL_DEFENSE]",
        "[EQUIPMENT_PHYSICAL_ATTACK]",
        "[EQUIPMENT_PHYSICAL_DEFENSE]",
        "[EQUIPMENT_MAGICAL_ATTACK]",
        "[EQUIPMENT_MAGICAL_DEFENSE]",
        "[ELEMENT_TOLERANCE_FIRE]",
        "[ELEMENT_TOLERANCE_WATER]",
        "[ELEMENT_TOLERANCE_DARK]",
        "[ELEMENT_TOLERANCE_LIGHT]",
    };
    static const char* const kActiveStatusTag[17] = {
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
        "[piercing resistance]",
        "[stuck resistance]",
        "[confuse resistance]",
        "[hold resistance]",
        "[armor break resistance]",
    };

    bool has_name = false;
    bool allow_current_pvf_action_list_close = false;
    bool allow_current_pvf_emergency_state_recovery_close = false;
    bool pending_champion_multi_factor = false;
    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (pending_champion_multi_factor &&
            tag != "[fastmove]" && tag != "[fastattack]") {
            return 0;
        }
        if (tag != "[/action list]") {
            allow_current_pvf_action_list_close = false;
        }
        if (tag != "[/emergency state recovery]") {
            allow_current_pvf_emergency_state_recovery_close = false;
        }
        int module_tag_index = -1;
        int parsed_module_type = -1;
        processModuleTagSet(module_tag_index,
                            parsed_module_type,
                            &tag,
                            module_type,
                            false);
        if (module_tag_index != -1) {
            if (parsed_module_type != -1) {
                script->value_1c9c = parsed_module_type;
            }
            continue;
        }

        if (tag == "[dungeon mode script]") {
            std::string dungeon_mode;
            std::string difficulty_mode;
            if (ScanStr(&dungeon_mode) != 1 ||
                ScanStr(&difficulty_mode) != 1 ||
                !((dungeon_mode == "battleship" &&
                   difficulty_mode == "all") ||
                  (dungeon_mode == "level acc" &&
                   difficulty_mode == "master"))) {
                return 0;
            }
            // Current PVF wraps an otherwise normal monster script in this
            // mode envelope.  Continue through its real inner tags so all
            // original fields are populated; old STMonsterScript has no
            // separate selector for the two validated current-PVF modes.
        } else if (tag == "[/dungeon mode script]") {
            // The wrapper's closing tag is empty.  Its inner tags were read
            // by the regular importer above, preserving their old ABI data.
        } else if (tag == "[name]") {
            if (ScanStr(&script->name) != 1) {
                return 0;
            }
            script->name_08 = script->name;
            has_name = true;
        } else if (tag == "[width]") {
            script->values_0c[0] = ScanInt(static_cast<bool*>(0));
            script->values_0c[1] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[floating height]") {
            script->values_0c[2] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[layer]") {
            std::string layer;
            if (ScanStr(&layer) != 1) {
                return 0;
            }
            script->values_0c[3] = getDrawLayer(layer.c_str());
        } else if (tag == "[category]") {
            script->values_1c.clear();
            bool is_type = false;
            std::string category;
            while (ScanTypeOrStr(is_type, category, true) == 1) {
                if (is_type) {
                    break;
                }
                const int category_value = getObjectCategory(category);
                if (category_value != 44) {
                    script->values_1c.push_back(category_value);
                }
            }
            if (!is_type || category != "[/category]") {
                return 0;
            }
        } else if (tag == "[ability category]") {
            script->parameter_categories.clear();
            std::string category;
            while (ScanStr(&category) == 1) {
                STMonsterParameterCategory parameter;
                for (int index = 0; index <= 13; ++index) {
                    if (category == kParameterCategoryName[index]) {
                        parameter.parameter_type = index;
                        break;
                    }
                }

                std::string operator_name;
                ScanStr(&operator_name);
                if (operator_name == "*") {
                    parameter.parameter_index = 1;
                }
                parameter.value_x100 = ScanInt(static_cast<bool*>(0));
                script->parameter_categories.push_back(parameter);
            }
        } else if (tag == "[ambient sound]") {
            if (ScanStr(&script->strings_694[6]) != 1) {
                return 0;
            }
        } else if (tag == "[attack sound]") {
            if (ScanStr(&script->strings_694[5]) != 1) {
                return 0;
            }
        } else if (tag == "[appear sound]") {
            if (ScanStr(&script->strings_694[4]) != 1) {
                return 0;
            }
        } else if (tag == "[damage sound]") {
            if (ScanStr(&script->strings_694[2]) != 1) {
                return 0;
            }
        } else if (tag == "[die sound]") {
            if (ScanStr(&script->strings_694[3]) != 1) {
                return 0;
            }
        } else if (tag == "[level]") {
            script->level_range[0] = ScanInt(static_cast<bool*>(0));
            script->level_range[1] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[fire resistance]") {
            script->values_3f4[0] = ScanInt(static_cast<bool*>(0));
            script->values_3f4[1] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[water resistance]") {
            script->values_3f4[2] = ScanInt(static_cast<bool*>(0));
            script->values_3f4[3] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[dark resistance]") {
            script->values_3f4[4] = ScanInt(static_cast<bool*>(0));
            script->values_3f4[5] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[light resistance]") {
            script->values_3f4[6] = ScanInt(static_cast<bool*>(0));
            script->values_3f4[7] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[HP regen speed]") {
            script->values_49c[0] = ScanInt(static_cast<bool*>(0));
            script->values_49c[1] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[MP regen speed]") {
            script->values_49c[2] = ScanInt(static_cast<bool*>(0));
            script->values_49c[3] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[move speed]") {
            script->values_49c[4] = ScanInt(static_cast<bool*>(0));
            script->values_49c[5] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[attack speed]") {
            script->values_49c[6] = ScanInt(static_cast<bool*>(0));
            script->values_49c[7] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[cast speed]") {
            script->values_49c[8] = ScanInt(static_cast<bool*>(0));
            script->values_49c[9] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[hit recovery]") {
            script->values_49c[10] = ScanInt(static_cast<bool*>(0));
            script->values_49c[11] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[jump power]") {
            script->values_49c[12] = ScanInt(static_cast<bool*>(0));
            script->values_49c[13] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[jump speed]") {
            script->values_49c[14] = ScanInt(static_cast<bool*>(0));
            script->values_49c[15] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[normal dash speed]") {
            script->values_49c[16] = ScanInt(static_cast<bool*>(0));
            script->values_49c[17] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[slant dash speed]") {
            script->values_49c[18] = ScanInt(static_cast<bool*>(0));
            script->values_49c[19] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[weight]") {
            script->values_49c[20] = ScanInt(static_cast<bool*>(0));
            script->values_49c[21] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[sight]") {
            script->values_4f4[4] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[diving sight]") {
            // IDA @0x8A12827 consumes both values without publishing either
            // one to STMonsterScript.  They belong to the client-side diving
            // sight extension, but must be read to preserve stream alignment.
            (void)ScanInt(static_cast<bool*>(0));
            (void)ScanInt(static_cast<bool*>(0));
        } else if (tag == "[emergency state recovery]") {
            if (!ConsumeCurrentPvfEmergencyStateRecovery()) {
                return 0;
            }
        } else if (tag == "[reduce active status damage]") {
            // Post-original per-status damage reductions have no field in
            // STMonsterScript.  Their PVF form is still fully checked so a
            // malformed/unknown status cannot be hidden by compatibility.
            if (!ConsumeCurrentPvfStatusDamageReduction(
                    "[/reduce active status damage]")) {
                return 0;
            }
        } else if (tag == "[set minimum active status resistance]") {
            // Current PVFs add a client-side resistance floor.  The original
            // STMonsterScript ABI has no storage or consumer for that floor;
            // validate its finite nested grammar rather than invent state.
            if (!ConsumeCurrentPvfStatusBound(
                    "[/set minimum active status resistance]")) {
                return 0;
            }
        } else if (tag == "[set max status resistance]") {
            if (!ConsumeCurrentPvfStatusBound(
                    "[/set max status resistance]")) {
                return 0;
            }
        } else if (tag == "[immune active status]") {
            // This current-PVF block names statuses that must be immune.  The
            // original STMonsterScript record has no corresponding member, so
            // keep the original server state unchanged but strictly validate
            // the existing active-status vocabulary and terminating tag.
            if (!ConsumeCurrentPvfImmuneActiveStatus(
                    "[/immune active status]")) {
                return 0;
            }
        } else if (tag == "[set minimum tolerance and defence]") {
            if (!ConsumeCurrentPvfMinimumToleranceAndDefence()) {
                return 0;
            }
        } else if (tag == "[resist value]") {
            if (!ConsumeCurrentPvfResistValue()) {
                return 0;
            }
        } else if (tag == "[resist entry]") {
            if (!ConsumeCurrentPvfResistEntry()) {
                return 0;
            }
        } else if (tag == "[leave body]") {
            // No corresponding member exists in the old monster ABI.
            // Preserve parser alignment only after validating its one flag.
            bool success = false;
            (void)ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[set position name]") {
            // Current PVFs use exactly three coordinate integers here.  The
            // original importer has neither this tag nor an ABI member for
            // it, so keep old runtime state intact after strict consumption.
            for (int index = 0; index != 3; ++index) {
                bool success = false;
                (void)ScanInt(&success);
                if (!success) {
                    return 0;
                }
            }
        } else if (tag == "[show hp state gague]") {
            // Current PVF leaf marker; no payload and no IDA-proven server
            // field exist in this old monster record.
        } else if (tag == "[use boss hp gague ui]") {
            // Another current-PVF UI-only leaf; it has no payload and is not
            // represented by a server-side member in the old ABI.
        } else if (tag == "[boss aura effect hide]") {
            // Current UI presentation flag; its one integer has no old
            // STMonsterScript target member but must be type-checked.
            if (!ScanCurrentPvfInt()) {
                return 0;
            }
        } else if (tag == "[init cooltime]") {
            if (!ConsumeCurrentPvfInitCooltime()) {
                return 0;
            }
        } else if (tag == "[string data]") {
            if (!ConsumeCurrentPvfStringData()) {
                return 0;
            }
        } else if (tag == "[character job type]") {
            // Single current-PVF table: exactly 65 job-selector integers.
            // No member for it exists in the original monster ABI.
            for (int index = 0; index != 65; ++index) {
                if (!ScanCurrentPvfInt()) {
                    return 0;
                }
            }
            if (!ScanCurrentPvfType("[/character job type]")) {
                return 0;
            }
        } else if (tag == "[no champion]") {
            // Current monster-selection flag; consume its required integer
            // without aliasing it to an unrelated original record field.
            if (!ScanCurrentPvfInt()) {
                return 0;
            }
        } else if (tag == "[holdable off]") {
            if (!ScanCurrentPvfInt()) {
                return 0;
            }
        } else if (tag == "[ignore active status]") {
            if (!ScanCurrentPvfInt()) {
                return 0;
            }
        } else if (tag == "[hold action]") {
            // Singleton current-PVF motion reference, absent from the old
            // record layout.  Consume its required action filename exactly.
            std::string action;
            if (ScanStr(&action) != 1) {
                return 0;
            }
        } else if (tag == "[is named monster]") {
            // Current-PVF marker with no distinct slot in the old ABI.  Do
            // not alias it to value_3b8: that is the original [life max]
            // field at this record offset.
            bool success = false;
            (void)ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[groop monster]") {
            // Misspelled current-PVF marker.  It is an explicit empty leaf;
            // the original record has no corresponding server-side field.
        } else if (tag == "[monster object type]") {
            // Current PVF adds a client object classification.  The two
            // shipped records use the sole [change action] value; the old
            // monster ABI exposes no corresponding server-side member.
            std::string object_type;
            if (ScanStr(&object_type) != 1 ||
                object_type != "[change action]") {
                return 0;
            }
        } else if (tag == "[draw hit object hp]") {
            // Client-only hit-object HP overlay.  The current package has
            // three explicit disabled (0) entries; no server record field
            // exists in the original ABI, so reject any unproven mode.
            bool success = false;
            const int enabled = ScanInt(&success);
            if (!success || enabled != 0) {
                return 0;
            }
        } else if (tag == "[follow move map]") {
            // Current map-follow selector for the one plover helper object.
            // The old monster record has no map-follow member or lifecycle;
            // only the established enabled serialization is admissible.
            bool success = false;
            const int enabled = ScanInt(&success);
            if (!success || enabled != 1) {
                return 0;
            }
        } else if (tag == "[enable states]") {
            // Current executable-state filter for the Luke guards.  Its two
            // client-facing names have no old STMonsterScript storage, but
            // the closed stream must be consumed exactly.
            std::string state;
            if (ScanStr(&state) != 1 || state != "attack" ||
                ScanStr(&state) != 1 || state != "die" ||
                !ScanCurrentPvfType("[/enable states]")) {
                return 0;
            }
        } else if (tag == "[change action by condition]") {
            if (!ConsumeCurrentPvfChangeActionByCondition()) {
                return 0;
            }
        } else if (tag == "[pass type]") {
            // This collision classification belongs to the newer passive
            // object model; STMonsterScript has no equivalent member.  The
            // CentralUnit record carries only this established value.
            std::string pass_type;
            if (ScanStr(&pass_type) != 1 || pass_type != "[do not pass]") {
                return 0;
            }
        } else if (tag == "[matching attack info]" ||
                   tag == "[matching etc attack info]") {
            // Newer empty markers selecting positional matching in the
            // attack-info tables.  The old record owns the two vectors but
            // has no distinct marker bit to populate.
        } else if (tag == "[hitstun resistance]") {
            bool success = false;
            const int resistance = ScanInt(&success);
            if (!success || (resistance != 90 && resistance != 100)) {
                return 0;
            }
        } else if (tag == "[hitstun resistance system]") {
            if (!ConsumeCurrentPvfHitstunResistanceSystem()) {
                return 0;
            }
        } else if (tag == "[strengthen damaged effect]") {
            if (!ConsumeCurrentPvfStrengthenDamagedEffect()) {
                return 0;
            }
        } else if (tag == "[hold immune system]") {
            // Current hold-immunity timer triplet, absent from the old
            // record.  All shipped uses share this exact numeric leaf.
            if (!ScanCurrentPvfIntValue(8000) ||
                !ScanCurrentPvfIntValue(45000) ||
                !ScanCurrentPvfFloatValue(0.25f)) {
                return 0;
            }
        } else if (tag == "[conditional amplify damage]") {
            if (!ConsumeCurrentPvfConditionalAmplifyDamage()) {
                return 0;
            }
        } else if (tag == "[create var]") {
            if (!ConsumeCurrentPvfCreateVar()) {
                return 0;
            }
        } else if (tag == "[pseudo boss]") {
            // Empty classification marker introduced after the legacy monster
            // ABI.  It deliberately has no surrogate field in STMonsterScript.
        } else if (tag == "[die cinematic]") {
            // The sole current-PVF cinematic declaration is a post-legacy
            // renderer cue.  Its payload has no corresponding member in the
            // old 0x1cc4 monster record, so keep acceptance exact rather than
            // silently swallowing a generic cinematic program.
            std::string state;
            if (!ScanCurrentPvfIntValue(12968) || ScanStr(&state) != 1 ||
                state != "stay" ||
                !ScanCurrentPvfType("[/die cinematic]")) {
                return 0;
            }
        } else if (tag == "[hp destroy]") {
            // The monster-side extension is disabled in the sole current
            // occurrence.  Do not alias passive-object destruction state.
            if (!ScanCurrentPvfIntValue(0)) {
                return 0;
            }
        } else if (tag == "[ozma apc enemy monster]") {
            // Empty current-PVF Ozma classification marker.
        } else if (tag == "[warlike]") {
            script->values_574[1] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[attack delay]") {
            script->values_574[2] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[targeting time term]") {
            script->values_574[0] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[targeting on attack]") {
            script->values_580[0] =
                static_cast<uint8_t>(ScanInt(static_cast<bool*>(0)) != 0);
        } else if (tag == "[targeting on damage]") {
            script->values_580[1] =
                static_cast<uint8_t>(ScanInt(static_cast<bool*>(0)) != 0);
        } else if (tag == "[targeting downed]") {
            script->values_5a4[0] =
                static_cast<uint8_t>(ScanInt(static_cast<bool*>(0)) != 0);
        } else if (tag == "[targeting high level]") {
            if (ScanInt(static_cast<bool*>(0)) != 0) {
                script->values_580[1] = 2;
            }
        } else if (tag == "[targeting low level]") {
            if (ScanInt(static_cast<bool*>(0)) != 0) {
                script->values_580[1] = 1;
            }
        } else if (tag == "[targeting high HP]") {
            if (ScanInt(static_cast<bool*>(0)) != 0) {
                script->values_584[0] = 2;
            }
        } else if (tag == "[targeting low HP]") {
            if (ScanInt(static_cast<bool*>(0)) != 0) {
                script->values_584[0] = 1;
            }
        } else if (tag == "[targeting low physical attack]") {
            script->values_594[0] = ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[targeting low physical defense]") {
            script->values_594[1] = ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[targeting low magical attack]") {
            script->values_594[2] = ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[targeting low magical defense]") {
            script->values_594[3] = ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[targeting from direction]") {
            const int enabled = ScanInt(static_cast<bool*>(0)) != 0;
            script->values_590[0] =
                (script->values_590[0] & ~0x00FF0000) |
                (enabled << 16);
        } else if (tag == "[targeting nearest]") {
            script->values_5a4[1] =
                static_cast<uint8_t>(ScanInt(static_cast<bool*>(0)) != 0);
        } else if (tag == "[targeting attacker]") {
            script->values_5a4[1] = static_cast<uint8_t>(
                ScanInt(static_cast<bool*>(0)) != 0);
        } else if (tag == "[superarmor on attack]") {
            const int enabled = ScanInt(static_cast<bool*>(0));
            script->values_5a4[1] = static_cast<uint8_t>(
                (script->values_5a4[1] & 1u) | (enabled ? 2u : 0u));
            script->value_5a8 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[waiting motion]") {
            if (ScanStr(&script->strings_5b4[0]) != 1 ||
                !isValidExtension(script->strings_5b4[0], ".ani")) {
                return 0;
            }
        } else if (tag == "[move motion]") {
            if (ScanStr(&script->strings_5b4[1]) != 1 ||
                !isValidExtension(script->strings_5b4[1], ".ani")) {
                return 0;
            }
        } else if (tag == "[sit motion]") {
            if (ScanStr(&script->strings_5b4[5]) != 1 ||
                !isValidExtension(script->strings_5b4[5], ".ani")) {
                return 0;
            }
        } else if (tag == "[damage motion 1]") {
            if (ScanStr(&script->strings_5cc[0]) != 1 ||
                !isValidExtension(script->strings_5cc[0], ".ani")) {
                return 0;
            }
        } else if (tag == "[damage motion 2]") {
            if (ScanStr(&script->strings_5cc[1]) != 1 ||
                !isValidExtension(script->strings_5cc[1], ".ani")) {
                return 0;
            }
        } else if (tag == "[down motion]") {
            if (ScanStr(&script->strings_5d4[0]) != 1 ||
                !isValidExtension(script->strings_5d4[0], ".ani")) {
                return 0;
            }
        } else if (tag == "[overturn motion]") {
            if (ScanStr(&script->strings_5d4[1]) != 1 ||
                !isValidExtension(script->strings_5d4[1], ".ani")) {
                return 0;
            }
        } else if (tag == "[jump motion]") {
            if (ScanStr(&script->strings_5d4[2]) != 1 ||
                !isValidExtension(script->strings_5d4[2], ".ani")) {
                return 0;
            }
        } else if (tag == "[jumpattack motion]") {
            if (ScanStr(&script->strings_5d4[3]) != 1 ||
                !isValidExtension(script->strings_5d4[3], ".ani")) {
                return 0;
            }
        } else if (tag == "[waiting action]") {
            if (ScanStr(&script->strings_5fc[0]) != 1) {
                return 0;
            }
        } else if (tag == "[move action]") {
            if (ScanStr(&script->strings_5fc[1]) != 1) {
                return 0;
            }
        } else if (tag == "[sit action]") {
            if (ScanStr(&script->strings_5fc[5]) != 1) {
                return 0;
            }
        } else if (tag == "[damage action 1]") {
            if (ScanStr(&script->strings_614[0]) != 1) {
                return 0;
            }
        } else if (tag == "[damage action 2]") {
            if (ScanStr(&script->strings_614[1]) != 1) {
                return 0;
            }
        } else if (tag == "[down action]") {
            if (ScanStr(&script->strings_61c[0]) != 1) {
                return 0;
            }
        } else if (tag == "[overturn action]") {
            if (ScanStr(&script->strings_61c[1]) != 1) {
                return 0;
            }
        } else if (tag == "[jump action]") {
            if (ScanStr(&script->strings_61c[2]) != 1) {
                return 0;
            }
        } else if (tag == "[jumpattack action]") {
            if (ScanStr(&script->strings_61c[3]) != 1) {
                return 0;
            }
        } else if (tag == "[basic action]") {
            if (ScanStr(&script->strings_694[1]) != 1) {
                return 0;
            }
        } else if (tag == "[etc action]") {
            script->values_638.clear();
            std::string action;
            while (ScanStr(&action) == 1) {
                script->values_638.push_back(action);
            }
        } else if (tag == "[come in action]") {
            if (ScanStr(&script->strings_530[0]) != 1) {
                return 0;
            }
            script->values_538[0] = ScanInt(static_cast<bool*>(0));
            script->values_538[1] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[come in action ex]") {
            if (ScanStr(&script->strings_530[0]) != 1 ||
                ScanStr(&script->strings_530[1]) != 1) {
                return 0;
            }
            script->values_538[0] = ScanInt(static_cast<bool*>(0));
            script->values_538[1] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[attack action]" || tag == "[action list]") {
            script->values_62c.clear();
            std::string action;
            while (ScanStr(&action) == 1) {
                script->values_62c.push_back(action);
            }
        } else if (tag == "[jumpattack info]") {
            if (ScanStr(&script->strings_694[8]) != 1) {
                return 0;
            }
        } else if (tag == "[attack info]") {
            script->values_6b8.clear();
            std::string info;
            while (ScanStr(&info) == 1) {
                script->values_6b8.push_back(info);
            }
        } else if (tag == "[etc attack info]") {
            script->values_6c4.clear();
            std::string info;
            while (ScanStr(&info) == 1) {
                script->values_6c4.push_back(info);
            }
        } else if (tag == "[int data]") {
            script->values_6d4.clear();
            bool success = false;
            while (true) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->values_6d4.push_back(value);
            }
        } else if (tag == "[attack motion]") {
            script->values_5e4.clear();
            bool is_type = false;
            std::string motion;
            while (ScanTypeOrStr(is_type, motion, true) == 1) {
                if (is_type) {
                    break;
                }
                if (!isValidExtension(motion, ".ani")) {
                    return 0;
                }
                script->values_5e4.push_back(motion);
            }
            if (!is_type || motion != "[/attack motion]") {
                return 0;
            }
        } else if (tag == "[etc motion]") {
            script->values_5f0.clear();
            bool is_type = false;
            std::string motion;
            while (ScanTypeOrStr(is_type, motion, true) == 1) {
                if (is_type) {
                    break;
                }
                if (!isValidExtension(motion, ".ani")) {
                    return 0;
                }
                script->values_5f0.push_back(motion);
            }
            if (!is_type || motion != "[/etc motion]") {
                return 0;
            }
        } else if (tag == "[last action]") {
            if (ScanStr(&script->strings_644[0]) != 1) {
                return 0;
            }
            // A small number of current PVFs retain this historical stray
            // closer immediately after [last action], without opening an
            // [action list] block.  It has no payload and must not be
            // accepted anywhere else.
            allow_current_pvf_action_list_close = true;
        } else if (tag == "[/action list]") {
            if (!allow_current_pvf_action_list_close) {
                return 0;
            }
            allow_current_pvf_action_list_close = false;
        } else if (tag == "[proc action]") {
            if (ScanStr(&script->strings_644[1]) != 1) {
                return 0;
            }
        } else if (tag == "[mounting action]") {
            if (ScanStr(&script->strings_644[2]) != 1) {
                return 0;
            }
        } else if (tag == "[mounted action]") {
            if (ScanStr(&script->strings_644[3]) != 1) {
                return 0;
            }
        } else if (tag == "[unmounting action]") {
            if (ScanStr(&script->strings_644[4]) != 1) {
                return 0;
            }
        } else if (tag == "[item]") {
            script->special_item_lists[0].clear();
            bool success = false;
            while (true) {
                const int item_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->special_item_lists[0].push_back(
                    std::make_pair(item_index,
                                   ScanInt(static_cast<bool*>(0))));
            }
        } else if (tag == "[common champion drop item]") {
            script->special_item_lists[1].clear();
            bool success = false;
            while (true) {
                const int item_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->special_item_lists[1].push_back(
                    std::make_pair(item_index,
                                   ScanInt(static_cast<bool*>(0))));
            }
        } else if (tag == "[boss drop item]") {
            script->special_item_lists[3].clear();
            bool success = false;
            while (true) {
                const int item_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->special_item_lists[3].push_back(
                    std::make_pair(item_index,
                                   ScanInt(static_cast<bool*>(0))));
            }
        } else if (tag == "[death tower item]") {
            script->death_tower_specify_items.clear();
            bool success = false;
            while (true) {
                const int item_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->death_tower_specify_items.push_back(
                    std::make_pair(item_index,
                                   ScanInt(static_cast<bool*>(0))));
            }
        } else if (tag == "[hell monster]") {
            script->value_3a8 = static_cast<uint8_t>(
                ScanInt(static_cast<bool*>(0)) != 0);
        } else if (tag == "[burning death path]") {
            if (ScanStr(&script->burning_death_path) != 1) {
                return 0;
            }
        } else if (tag == "[equipment]") {
            script->equipment.clear();
            const int equipment_count = ScanInt(static_cast<bool*>(0));
            std::string equipment_path;
            for (int index = 0; index < equipment_count; ++index) {
                if (ScanStr(&equipment_path) != 1) {
                    return 0;
                }
                script->equipment.push_back(equipment_path);
            }
            // IDA consumes the following equipment-set token but does not
            // retain it in STMonsterEquipment.  In the original scanner this
            // is deliberately unchecked: current PVFs use the type token
            // [/equipment] here, which must be consumed before the next
            // top-level ScanType call.
            (void)ScanStr(&equipment_path);
        } else if (tag == "[ready action]") {
            STMosterAttackReady action;
            bool saw_motion_index = false;
            bool saw_effect_entries = false;
            bool closed = false;
            while (!closed) {
                bool is_type = false;
                std::string action_tag;
                if (ScanTypeOrStr(is_type, action_tag, true) != 1) {
                    return 0;
                }
                if (is_type) {
                    if (action_tag == "[/ready action]") {
                        closed = true;
                        continue;
                    }
                    return 0;
                }
                if (action_tag == "[motion index]") {
                    if (saw_motion_index) {
                        return 0;
                    }
                    bool success = false;
                    action.motion_index = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                    saw_motion_index = true;
                } else if (action_tag == "[effect ani file & layer index]") {
                    if (saw_effect_entries) {
                        return 0;
                    }
                    bool success = false;
                    const int declared_count = ScanInt(&success);
                    if (!success || declared_count < 0) {
                        return 0;
                    }
                    action.effect_animation_files.reserve(
                        static_cast<size_t>(declared_count) + 1u);
                    action.layer_indices.reserve(
                        static_cast<size_t>(declared_count) + 1u);
                    int entry_count = 0;
                    while (true) {
                        bool entry_is_type = false;
                        std::string effect_path;
                        if (ScanTypeOrStr(entry_is_type, effect_path, true) != 1) {
                            return 0;
                        }
                        if (entry_is_type) {
                            if (effect_path != "[/ready action]") {
                                return 0;
                            }
                            closed = true;
                            break;
                        }
                        const int layer_index = ScanInt(&success);
                        if (!success) {
                            return 0;
                        }
                        action.effect_animation_files.push_back(effect_path);
                        action.layer_indices.push_back(layer_index);
                        ++entry_count;
                    }
                    // Original scripts use a count.  Current scripts use the
                    // final zero-based index, so both real encodings have a
                    // bounded representation in the same old vector fields.
                    if (entry_count != declared_count &&
                        entry_count != declared_count + 1) {
                        return 0;
                    }
                    saw_effect_entries = true;
                } else {
                    return 0;
                }
            }
            if (!saw_motion_index || !saw_effect_entries) {
                return 0;
            }
            script->ready_actions.push_back(action);
        } else if (tag == "[level of difficulty]") {
            STDifficulty difficulty;
            bool is_type = false;
            std::string difficulty_tag;
            while (ScanTypeOrStr(is_type, difficulty_tag, true) == 1) {
                if (!is_type) {
                    continue;
                }
                if (difficulty_tag == "[/level of difficulty]") {
                    break;
                }
                if (difficulty_tag == "[easy]" ||
                    difficulty_tag == "[medium]" ||
                    difficulty_tag == "[hard]" ||
                    difficulty_tag == "[ultimate]" ||
                    difficulty_tag == "[hero]" ||
                    // The original ABI stores only the five ordered
                    // STDifficulty records.  Later PVFs renamed those five
                    // serialized slots without changing their two-int
                    // payload or their order.
                    difficulty_tag == "[normal]" ||
                    difficulty_tag == "[ex]" ||
                    difficulty_tag == "[expert]" ||
                    difficulty_tag == "[master]" ||
                    difficulty_tag == "[king]" ||
                    difficulty_tag == "[slayer]") {
                    difficulty.level = ScanInt(static_cast<bool*>(0));
                    difficulty.value = ScanInt(static_cast<bool*>(0));
                    script->difficulties.push_back(difficulty);
                }
            }
            if (!is_type || difficulty_tag != "[/level of difficulty]") {
                return 0;
            }
        } else if (tag == "[pattern]") {
            std::vector<int> pattern;
            bool success = false;
            while (true) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                pattern.push_back(value);
            }
            if (!pattern.empty()) {
                script->patterns.push_back(pattern);
            }
        } else if (tag == "[jump attack]") {
            STJumpAttack attack;
            std::string field;
            while (ScanStr(&field) == 1) {
                if (field == "[attack index]") {
                    attack.attack_index = ScanInt(static_cast<bool*>(0));
                } else if (field == "[start frame index]") {
                    attack.start_frame_index = ScanInt(static_cast<bool*>(0));
                } else if (field == "[end frame index]") {
                    attack.end_frame_index = ScanInt(static_cast<bool*>(0));
                } else if (field == "[jump height]") {
                    attack.jump_height = ScanInt(static_cast<bool*>(0));
                } else if (field == "[jump speed]") {
                    attack.jump_speed = ScanInt(static_cast<bool*>(0));
                } else if (field == "[on collision custom animation index]") {
                    attack.collision_animation_index =
                        ScanInt(static_cast<bool*>(0));
                } else if (field == "[ememy distance rate]") {
                    attack.enemy_distance_rate =
                        ScanFloat(static_cast<bool*>(0));
                } else if (field == "[limited x cood]") {
                    attack.limited_x_coordinate =
                        ScanInt(static_cast<bool*>(0));
                } else if (field == "[limited y cood]") {
                    attack.limited_y_coordinate =
                        ScanInt(static_cast<bool*>(0));
                } else if (field == "[jump type]") {
                    attack.jump_type = ScanInt(static_cast<bool*>(0));
                } else if (field == "[jump direction]") {
                    attack.jump_direction = ScanInt(static_cast<bool*>(0));
                }
            }
            script->jump_attacks.push_back(attack);
        } else if (tag == "[summon]") {
            STSummon summon;
            std::string field;
            while (ScanStr(&field) == 1) {
                if (field == "[function index]") {
                    summon.values_00[0] = ScanInt(static_cast<bool*>(0));
                } else if (field == "[attack index]") {
                    summon.values_00[1] = ScanInt(static_cast<bool*>(0));
                } else if (field == "[pos random]") {
                    summon.values_00[2] = ScanInt(static_cast<bool*>(0));
                } else if (field == "[monster kind]") {
                    summon.values_00[3] = ScanInt(static_cast<bool*>(0));
                } else if (field == "[summon term]") {
                    summon.values_1c[0] = ScanInt(static_cast<bool*>(0));
                } else if (field == "[probability]") {
                    summon.values_1c[1] = ScanInt(static_cast<bool*>(0));
                } else if (field == "[enterance effect]") {
                    summon.values_1c[2] = ScanInt(static_cast<bool*>(0));
                } else if (field == "[max number]") {
                    summon.values_1c[3] = ScanInt(static_cast<bool*>(0));
                } else if (field == "[max frequency]") {
                    summon.values_1c[4] = ScanInt(static_cast<bool*>(0));
                } else if (field == "[summon]") {
                    const int count = ScanInt(static_cast<bool*>(0));
                    summon.summons.reserve(count);
                    for (int index = 0; index < count; ++index) {
                        STSummonInfo info;
                        for (int value = 0; value <= 3; ++value) {
                            info.values[value] =
                                ScanInt(static_cast<bool*>(0));
                        }
                        summon.summons.push_back(info);
                    }
                }
            }
            script->summons.push_back(summon);
        } else if (tag == "[dash attack]") {
            STDash dash;
            std::string field;
            while (ScanStr(&field) == 1) {
                if (field == "[function index]") {
                    dash.values[0] = ScanInt(static_cast<bool*>(0));
                } else if (field == "[attack index]") {
                    dash.values[1] = ScanInt(static_cast<bool*>(0));
                } else if (field == "[shakeable]") {
                    dash.values[2] = ScanInt(static_cast<bool*>(0));
                    dash.values[3] = ScanInt(static_cast<bool*>(0));
                    dash.values[4] = ScanInt(static_cast<bool*>(0));
                } else if (field == "[first speed]") {
                    dash.values[5] = ScanInt(static_cast<bool*>(0));
                } else if (field == "[last speed]") {
                    dash.values[6] = ScanInt(static_cast<bool*>(0));
                } else if (field == "[dash time]") {
                    dash.values[7] = ScanInt(static_cast<bool*>(0));
                } else if (field == "[dash ready time]") {
                    dash.values[8] = ScanInt(static_cast<bool*>(0));
                } else if (field == "[animation first index]") {
                    dash.values[9] = ScanInt(static_cast<bool*>(0));
                }
            }
            script->dash_attacks.push_back(dash);
        } else if (tag == "[hold attack]") {
            STHold hold;
            std::string field;
            while (ScanStr(&field) == 1) {
                if (field == "[function index]") {
                    hold.values[0] = ScanInt(static_cast<bool*>(0));
                } else if (field == "[attack index]") {
                    hold.values[1] = ScanInt(static_cast<bool*>(0));
                } else if (field == "[offset]") {
                    hold.values[2] = ScanInt(static_cast<bool*>(0));
                    hold.values[3] = ScanInt(static_cast<bool*>(0));
                } else if (field == "[animation index]") {
                    hold.values[4] = ScanInt(static_cast<bool*>(0));
                } else if (field == "[min hold]") {
                    hold.values[5] = ScanInt(static_cast<bool*>(0));
                } else if (field == "[max hold]") {
                    hold.values[6] = ScanInt(static_cast<bool*>(0));
                }
            }
            script->hold_attacks.push_back(hold);
        } else if (tag == "[warp attack]") {
            STWarp warp;
            std::string field;
            while (ScanStr(&field) == 1) {
                if (field == "[function index]") {
                    warp.function_index = ScanInt(static_cast<bool*>(0));
                } else if (field == "[attack index]") {
                    warp.attack_index = ScanInt(static_cast<bool*>(0));
                } else if (field == "[warp style]") {
                    warp.warp_style = ScanInt(static_cast<bool*>(0));
                } else if (field == "[after attack]") {
                    warp.after_attack = ScanInt(static_cast<bool*>(0));
                } else if (field == "[after appear time]") {
                    const int raw_after_appear_time =
                        ScanInt(static_cast<bool*>(0));
                    std::memcpy(&warp.after_appear_time,
                                &raw_after_appear_time,
                                sizeof(warp.after_appear_time));
                } else if (field == "[cool time]") {
                    warp.cool_time = ScanInt(static_cast<bool*>(0));
                } else if (field == "[target offset]") {
                    warp.target_offset = ScanInt(static_cast<bool*>(0));
                } else if (field == "[passive object index]") {
                    warp.passive_object_index = ScanInt(static_cast<bool*>(0));
                } else if (field == "[category of target]") {
                    std::string category;
                    if (ScanStr(&category) != 1) {
                        return 0;
                    }
                    if (category == "null") {
                        warp.target_category = -2;
                    } else if (category == "all") {
                        warp.target_category = -1;
                    } else {
                        warp.target_category = getObjectCategory(category);
                    }
                }
            }
            script->warp_attacks.push_back(warp);
        } else if (tag == "[event super on scr]") {
            STEventSuper event_super;
            std::string field;
            while (ScanStr(&field) == 1) {
                if (field == "[on time]") {
                    event_super.first_dword = 0;
                    event_super.second_dword =
                        ScanInt(static_cast<bool*>(0));
                    event_super.third_dword =
                        ScanInt(static_cast<bool*>(0));
                    event_super.fourth_dword =
                        ScanInt(static_cast<bool*>(0)) != 0;
                } else if (field == "[on hp]") {
                    event_super.first_dword = 1;
                    event_super.second_dword =
                        ScanInt(static_cast<bool*>(0));
                    event_super.third_dword =
                        ScanInt(static_cast<bool*>(0));
                    event_super.fourth_dword =
                        ScanInt(static_cast<bool*>(0)) != 0;
                } else if (field == "[always]") {
                    event_super.first_dword = 2;
                    event_super.second_dword =
                        ScanInt(static_cast<bool*>(0));
                    event_super.fourth_dword =
                        ScanInt(static_cast<bool*>(0)) != 0;
                } else if (field == "[counter]") {
                    event_super.first_dword = 3;
                    event_super.second_dword =
                        ScanInt(static_cast<bool*>(0));
                    event_super.third_dword =
                        ScanInt(static_cast<bool*>(0));
                    event_super.fourth_dword =
                        ScanInt(static_cast<bool*>(0)) != 0;
                } else if (field == "[breakable]") {
                    event_super.first_flag = 0;
                    event_super.fifth_dword = ScanInt(static_cast<bool*>(0));
                    event_super.sixth_dword = ScanInt(static_cast<bool*>(0));
                    event_super.seventh_dword = ScanInt(static_cast<bool*>(0));
                    event_super.eighth_dword = ScanInt(static_cast<bool*>(0));
                } else if (field == "[half]") {
                    event_super.first_flag = 1;
                    event_super.fifth_dword = ScanInt(static_cast<bool*>(0));
                    event_super.sixth_dword = ScanInt(static_cast<bool*>(0));
                    event_super.seventh_dword = ScanInt(static_cast<bool*>(0));
                    event_super.eighth_dword = ScanInt(static_cast<bool*>(0));
                } else if (field == "[unbreakable]") {
                    event_super.first_flag = 2;
                    event_super.fifth_dword = ScanInt(static_cast<bool*>(0));
                    event_super.sixth_dword = ScanInt(static_cast<bool*>(0));
                    event_super.seventh_dword = ScanInt(static_cast<bool*>(0));
                    event_super.eighth_dword = ScanInt(static_cast<bool*>(0));
                } else if (field == "[with blast]") {
                    event_super.second_flag = 1;
                }
            }
            script->event_supers.push_back(event_super);
        } else if (tag == "[attack kind]") {
            script->attack_methods.clear();
            while (true) {
                STAttackMethod method;
                bool success = false;
                const float first_value = ScanFloat(&success);
                if (!success) {
                    break;
                }
                const float second_value =
                    ScanFloat(static_cast<bool*>(0));
                const float third_value =
                    ScanFloat(static_cast<bool*>(0));
                method.values[0] =
                    ScanInt(static_cast<bool*>(0)) != 0;
                std::memcpy(&method.values[1],
                            &first_value,
                            sizeof(first_value));
                std::memcpy(&method.values[2],
                            &second_value,
                            sizeof(second_value));
                std::memcpy(&method.values[3],
                            &third_value,
                            sizeof(third_value));
                for (int index = 4; index <= 11; ++index) {
                    method.values[index] =
                        ScanInt(static_cast<bool*>(0));
                }
                script->attack_methods.push_back(method);
            }
        } else if (tag == "[catch item]") {
            CatchItemInfo item;
            item.item_index = ScanInt(static_cast<bool*>(0));
            item.count = ScanInt(static_cast<bool*>(0));
            item.probability = ScanFloat(static_cast<bool*>(0));
            std::string boundary;
            ScanType(boundary, true);
            script->catch_items.push_back(item);
        } else if (tag == "[counter attack]") {
            const int count = ScanInt(static_cast<bool*>(0));
            script->counter_attacks.clear();
            script->counter_attacks.reserve(count);
            for (int index = 0; index < count; ++index) {
                STCounterAttack counter_attack;
                std::string field;
                while (ScanStr(&field) == 1) {
                    if (field == "[probability]") {
                        counter_attack.probability =
                            ScanInt(static_cast<bool*>(0));
                    } else if (field == "[attack index]") {
                        counter_attack.attack_index =
                            ScanInt(static_cast<bool*>(0));
                    } else if (field == "[damage type]") {
                        std::string attack_type;
                        if (ScanStr(&attack_type) != 1) {
                            return 0;
                        }
                        if (attack_type == "[approach attack]") {
                            counter_attack.damage_type = 0;
                        } else if (attack_type == "[range attack]") {
                            counter_attack.damage_type = 1;
                        } else if (attack_type == "[all attack]") {
                            counter_attack.damage_type = 2;
                        }
                    } else if (field == "[need hp]") {
                        counter_attack.need_hp =
                            ScanInt(static_cast<bool*>(0));
                        counter_attack.use_need_hp = 1;
                    }
                }
                script->counter_attacks.push_back(counter_attack);
            }
        } else if (tag == "[speech on situation]") {
            for (int index = 0; index <= 110; ++index) {
                script->motion_data[index].clear();
            }
            std::string situation;
            while (ScanStr(&situation) == 1) {
                std::string speech;
                if (ScanStr(&speech) != 1) {
                    return 0;
                }
                const int attack_situation =
                    ScanInt(static_cast<bool*>(0));
                int situation_index = -1;
                if (situation == "[on attack]") {
                    situation_index = attack_situation;
                } else if (situation == "[on damage]") {
                    situation_index = 101;
                } else if (situation == "[on die]") {
                    situation_index = 102;
                } else if (situation == "[on revenge]") {
                    situation_index = 103;
                } else if (situation == "[on etc]") {
                    situation_index = 104;
                } else if (situation == "[on appear]") {
                    situation_index = 105;
                } else if (situation == "[on disappear]") {
                    situation_index = 106;
                } else if (situation == "[on go to help]") {
                    situation_index = 107;
                } else if (situation == "[on stand]") {
                    situation_index = 108;
                } else if (situation == "[on village]") {
                    situation_index = 110;
                }
                if (situation_index >= 0 && situation_index <= 110) {
                    script->motion_data[situation_index].push_back(speech);
                }
            }
        } else if (tag == "[fix move direction]") {
            if (ScanInt(static_cast<bool*>(0)) != 0) {
                script->value_c50 = 1;
            }
        } else if (tag == "[throw attack]") {
            STThrowAttack throw_attack;
            int passive_object_count = 0;
            std::string field;
            while (ScanStr(&field) == 1) {
                if (field == "[passive object number]") {
                    passive_object_count = ScanInt(static_cast<bool*>(0));
                    throw_attack.value_0c = passive_object_count;
                } else if (field == "[passive object filename]") {
                    throw_attack.passive_object_paths.clear();
                    throw_attack.passive_object_paths.reserve(
                        passive_object_count);
                    for (int index = 0; index < passive_object_count; ++index) {
                        std::string path;
                        if (ScanStr(&path) != 1) {
                            return 0;
                        }
                        throw_attack.passive_object_paths.push_back(path);
                    }
                } else if (field == "[passive object index]") {
                    throw_attack.value_10 =
                        ScanInt(static_cast<bool*>(0));
                } else if (field == "[power]") {
                    throw_attack.value_14 =
                        ScanInt(static_cast<bool*>(0));
                } else if (field == "[throw frame]") {
                    throw_attack.values_44[0] =
                        ScanInt(static_cast<bool*>(0));
                } else if (field == "[passive object start x cood]") {
                    throw_attack.start_x_coordinates.resize(passive_object_count);
                    for (int index = 0; index < passive_object_count; ++index) {
                        throw_attack.start_x_coordinates[index] =
                            ScanInt(static_cast<bool*>(0));
                    }
                } else if (field == "[passive object start y cood]") {
                    throw_attack.start_y_coordinates.resize(passive_object_count);
                    for (int index = 0; index < passive_object_count; ++index) {
                        throw_attack.start_y_coordinates[index] =
                            ScanInt(static_cast<bool*>(0));
                    }
                } else if (field == "[passive object start z cood]") {
                    throw_attack.start_z_coordinates.resize(passive_object_count);
                    for (int index = 0; index < passive_object_count; ++index) {
                        throw_attack.start_z_coordinates[index] =
                            ScanInt(static_cast<bool*>(0));
                    }
                } else if (field == "[direction]") {
                    throw_attack.values_44[1] =
                        ScanInt(static_cast<bool*>(0));
                } else if (field == "[attack number]") {
                    throw_attack.attack_number =
                        ScanInt(static_cast<bool*>(0));
                } else if (field == "[attack index]") {
                    throw_attack.values_44[2] =
                        ScanInt(static_cast<bool*>(0));
                } else if (field == "[throw sound]") {
                    if (ScanStr(&throw_attack.throw_sound) != 1) {
                        return 0;
                    }
                } else if (field == "[object type]") {
                    throw_attack.values_44[3] =
                        ScanInt(static_cast<bool*>(0));
                } else if (field == "[start kind]") {
                    throw_attack.values_44[4] =
                        ScanInt(static_cast<bool*>(0));
                }
            }
            script->throw_attacks.push_back(throw_attack);
        } else if (tag == "[atk super on scr]") {
            script->attack_supers.clear();
            while (true) {
                STAtkSuper attack_super;
                bool success = false;
                attack_super.values[1] = ScanInt(&success);
                if (!success) {
                    break;
                }
                std::string kind;
                if (ScanStr(&kind) != 1) {
                    return 0;
                }
                if (kind == "[breakable]") {
                    attack_super.values[0] = 13;
                } else if (kind == "[half]") {
                    attack_super.values[0] = 14;
                } else if (kind == "[unbreakable]") {
                    attack_super.values[0] = 15;
                }
                for (int value = 2; value <= 5; ++value) {
                    attack_super.values[value] =
                        ScanInt(static_cast<bool*>(0));
                }
                script->attack_supers.push_back(attack_super);
            }
        } else if (tag == "[cooltime]") {
            STCoolTime cooltime;
            while (true) {
                bool is_type = false;
                bool success = false;
                std::string field;
                const int value =
                    ScanTypeOrInt(is_type, field, success, true);
                if (!success) {
                    break;
                }
                if (!is_type) {
                    cooltime.values_04.push_back(value);
                } else if (field == "[Pattern]") {
                    cooltime.value_00 = ScanInt(static_cast<bool*>(0));
                } else if (field == "[/cooltime]") {
                    break;
                }
            }
            script->cooltimes.push_back(cooltime);
        } else if (tag == "[common champion elemental property]") {
            script->values_c40.clear();
            std::string element_name;
            while (ScanStr(&element_name) == 1) {
                for (int element = 0; element <= 4; ++element) {
                    const ENUM_ELEMENT typed_element =
                        static_cast<ENUM_ELEMENT>(element);
                    if (element_name == getElementName(typed_element)) {
                        script->values_c40.push_back(typed_element);
                        break;
                    }
                }
            }
        } else if (tag == "[super champion drop item]") {
            script->special_item_lists[2].clear();
            bool success = false;
            while (true) {
                const int item_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->special_item_lists[2].push_back(
                    std::make_pair(item_index,
                                   ScanInt(static_cast<bool*>(0))));
            }
        } else if (tag == "[out of targetting]") {
            script->value_6e4 = static_cast<uint8_t>(
                ScanInt(static_cast<bool*>(0)) != 0);
        } else if (tag == "[die effect]") {
            script->values_6f4[0] = ScanInt(static_cast<bool*>(0));
            script->values_6f4[1] = ScanInt(static_cast<bool*>(0));
            script->values_6f4[2] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[keep distance]") {
            script->value_6e0 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[is on the object]") {
            script->value_374 = static_cast<uint8_t>(
                ScanInt(static_cast<bool*>(0)) != 0);
            script->values_378[0] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[spawn prob]") {
            script->value_c4c = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[drop item offset]") {
            script->values_36c[0] = ScanInt(static_cast<bool*>(0));
            script->values_36c[1] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[is grabable]") {
            script->value_368 = static_cast<uint8_t>(
                ScanInt(static_cast<bool*>(0)) != 0);
        } else if (tag == "[reflection object]") {
            script->values_c80.clear();
            script->values_c8c.clear();
            bool success = false;
            while (true) {
                const int object_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int reflection_index = ScanInt(static_cast<bool*>(0));
                script->values_c8c.push_back(object_index);
                script->values_c80.push_back(reflection_index);
            }
        } else if (tag == "[set damage check]") {
            script->values_378[2] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[change direction by target]") {
            script->value_33c = static_cast<uint8_t>(
                ScanInt(static_cast<bool*>(0)) != 0);
        } else if (tag == "[is direction to move]") {
            script->values_378[1] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[absolute damage resistance rate]") {
            const float value = ScanFloat(static_cast<bool*>(0));
            std::memcpy(&script->values_c54[0], &value, sizeof(value));
        } else if (tag == "[hp regen rate]") {
            const float hp_regen = ScanFloat(static_cast<bool*>(0));
            const float mp_regen = ScanFloat(static_cast<bool*>(0));
            std::memcpy(&script->values_538[2],
                        &hp_regen,
                        sizeof(hp_regen));
            std::memcpy(&script->values_538[3],
                        &mp_regen,
                        sizeof(mp_regen));
        } else if (tag == "[absolute defense ignore rate]") {
            const float value = ScanFloat(static_cast<bool*>(0));
            std::memcpy(&script->values_c54[1], &value, sizeof(value));
        } else if (tag == "[lie frame]") {
            for (int index = 0; index <= 4; ++index) {
                script->values_c54[index + 2] =
                    ScanInt(static_cast<bool*>(0));
            }
        } else if (tag == "[forced knockback]") {
            script->values_1cac[1] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[No Combo Rate Monster]") {
            script->value_1ca8 = 1;
        } else if (tag == "[/emergency state recovery]") {
            // Some current Ozma PVFs contain this historical stray closer
            // directly after a complete [ai pattern] block, without the
            // corresponding opener.  It has no payload; retaining the
            // adjacency guard preserves the original parser's strictness for
            // every other occurrence.
            if (!allow_current_pvf_emergency_state_recovery_close) {
                return 0;
            }
            allow_current_pvf_emergency_state_recovery_close = false;
        } else if (importAIInfoScript(&script->ai_info, &tag, true)) {
            if (tag == "[ai pattern]") {
                allow_current_pvf_emergency_state_recovery_close = true;
            }
        } else if (tag == "[physical attack besed on diff]") {
            for (int row = 0; row <= 199; ++row) {
                bool success = false;
                const int difficulty_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                for (int column = 0; column <= 4; ++column) {
                    if (ScanFloat(
                            &script->physical_attack_by_difficulty[
                                difficulty_index - 1][column]) != 1) {
                        return 0;
                    }
                }
            }
        } else if (tag == "[summon skill status up type]") {
            script->value_3c0 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[ability table]") {
            script->value_3bc = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[summon skill status up rate]") {
            for (int index = 0; index <= 9; ++index) {
                const float value = ScanFloat(static_cast<bool*>(0));
                std::memcpy(&script->values_3c4[index],
                            &value,
                            sizeof(value));
            }
        } else if (tag == "[specific gen rate item]") {
            bool success = false;
            script->value_88 = ScanInt(&success);
            if (!success) {
                return 0;
            }
            script->special_drop_items.clear();
            while (true) {
                const int item_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int rate = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                script->special_drop_items.push_back(
                    std::make_pair(item_index, rate));
            }
        } else if (tag == "[stuckbonus on damage]") {
            for (int index = 0; index <= 3; ++index) {
                script->values_4f4[index] =
                    ScanInt(static_cast<bool*>(0));
            }
        } else if (tag == "[add gravity in aerial]") {
            script->value_548 = static_cast<uint8_t>(
                ScanInt(static_cast<bool*>(0)) != 0);
            script->value_54c = ScanFloat(static_cast<bool*>(0));
            script->value_550 = ScanFloat(static_cast<bool*>(0));
        } else if (tag == "[attack down prob]") {
            bool success = false;
            size_t index = 0;
            while (true) {
                const float probability = ScanFloat(&success);
                if (!success) {
                    break;
                }
                // The original loop scans until the next non-float token,
                // thereby consuming [/attack down prob] after the final
                // probability.  A script with more values than attack
                // methods is malformed rather than a reason to overrun the
                // recovered vector ABI.
                if (index >= script->attack_methods.size()) {
                    return 0;
                }
                std::memcpy(&script->attack_methods[index].values[1],
                            &probability,
                            sizeof(probability));
                ++index;
            }
        } else if (tag == "[damage time balance]") {
            for (int index = 0; index <= 3; ++index) {
                bool success = false;
                const int value = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                script->values_554.push_back(value);
            }
        } else if (tag == "[stun time balance]") {
            for (int index = 0; index <= 3; ++index) {
                bool success = false;
                const int value = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                script->values_560.push_back(value);
            }
        } else if (tag == "[explain]") {
            if (ScanStr(&script->value_6d0) != 1) {
                return 0;
            }
        } else if (tag == "[consume mp]") {
            script->value_120.clear();
            bool success = false;
            while (true) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->value_120.push_back(value);
            }
        } else if (tag == "[strong]") {
            script->value_1ca0 = ScanInt(static_cast<bool*>(0));
            script->value_1ca4 = ScanFloat(static_cast<bool*>(0));
        } else if (tag == "[weak]") {
            script->value_1ca0 = -ScanInt(static_cast<bool*>(0));
            script->value_1ca4 = ScanFloat(static_cast<bool*>(0));
        } else if (tag == "[monster title]") {
            if (ScanStr(&script->strings_694[0]) != 1) {
                return 0;
            }
        } else if (tag == "[moveback action]") {
            if (ScanStr(&script->strings_5fc[2]) != 1) {
                return 0;
            }
        } else if (tag == "[dash action]") {
            if (ScanStr(&script->strings_5fc[3]) != 1) {
                return 0;
            }
        } else if (tag == "[dashattack action]") {
            if (ScanStr(&script->strings_5fc[4]) != 1) {
                return 0;
            }
        } else if (tag == "[initial target]") {
            STInitialTarget target;
            std::string field;
            while (ScanType(field, true) == 1) {
                if (field == "[/initial target]") {
                    break;
                }
                if (field == "[initial target Permanent]") {
                    target.value_0c = static_cast<uint8_t>(
                        ScanInt(static_cast<bool*>(0)) == 1);
                } else {
                    target.target_name = field;
                    target.target_index = ScanInt(static_cast<bool*>(0));
                }
            }
            script->initial_targets.push_back(target);
        } else if (tag == "[overhead gauge type]") {
            script->values_c74[0] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[overhead gauge pos]") {
            script->values_c74[1] = ScanInt(static_cast<bool*>(0));
            script->values_c74[2] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[overhead gauge image]") {
            // Later monster data selects two client gauge-image indices.
            // values_c74 already owns the legacy type/X/Y triplet, so keep
            // its verified layout and consume the two new indices strictly.
            bool success = false;
            const int normal_image = ScanInt(&success);
            if (!success || normal_image < 0) {
                return 0;
            }
            const int damaged_image = ScanInt(&success);
            if (!success || damaged_image < 0) {
                return 0;
            }
        } else if (tag == "[fire attack]") {
            script->values_508[0] = ScanInt(static_cast<bool*>(0));
            script->values_508[1] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[water attack]") {
            script->values_508[2] = ScanInt(static_cast<bool*>(0));
            script->values_508[3] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[dark attack]") {
            script->values_508[4] = ScanInt(static_cast<bool*>(0));
            script->values_508[5] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[light attack]") {
            script->values_508[6] = ScanInt(static_cast<bool*>(0));
            script->values_508[7] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[rigidity]") {
            script->values_528[0] = ScanInt(static_cast<bool*>(0));
            script->values_528[1] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[ridableskill explanation]") {
            STRidableSkillExplain explain;
            bool is_type = false;
            std::string field;
            while (ScanTypeOrStr(is_type, field, true) == 1) {
                if (!is_type) {
                    continue;
                }
                if (field == "[/ridableskill explanation]") {
                    break;
                }
                if (field == "[title]") {
                    if (ScanStr(&explain.title) != 1) {
                        return 0;
                    }
                } else if (field == "[subtitle]") {
                    if (ScanStr(&explain.subtitle) != 1) {
                        return 0;
                    }
                } else if (field == "[category]") {
                    if (ScanStr(&explain.category) != 1) {
                        return 0;
                    }
                } else if (field == "[detail]") {
                    if (ScanStr(&explain.detail) != 1) {
                        return 0;
                    }
                } else if (field == "[pattern]") {
                    explain.pattern = ScanInt(static_cast<bool*>(0));
                }
            }
            script->ridable_skill_explains.push_back(explain);
        } else if (tag == "[ridableskill definition]") {
            STRidableSkill skill;
            bool closed = false;
            bool is_type = false;
            std::string field;
            while (ScanTypeOrStr(is_type, field, true) == 1) {
                if (!is_type) {
                    continue;
                }
                if (field == "[/ridableskill definition]") {
                    closed = true;
                    break;
                }
                if (field == "[pattern]") {
                    skill.pattern = ScanInt(static_cast<bool*>(0));
                } else if (field == "[image path]") {
                    if (ScanStr(&skill.image_path) != 1) {
                        return 0;
                    }
                } else if (field == "[skill]") {
                    for (int index = 0; index <= 7; ++index) {
                        skill.keys[index] = ScanInt(static_cast<bool*>(0));
                        skill.values_a[index] =
                            ScanInt(static_cast<bool*>(0));
                        std::string mode_name;
                        if (ScanStr(&mode_name) != 1) {
                            return 0;
                        }

                        int mode = 6;
                        if (mode_name == "normal") {
                            mode = 0;
                        } else if (mode_name == "defaultcancel") {
                            mode = 1;
                        } else if (mode_name == "movable") {
                            mode = 2;
                        } else if (mode_name == "charge") {
                            mode = 3;
                        } else if (mode_name == "evasion") {
                            mode = 4;
                        } else if (mode_name == "super") {
                            mode = 5;
                        }
                        skill.modes[index] = mode;

                        if (mode == 0 || mode == 1 || mode == 2 ||
                            mode == 4 || mode == 5) {
                            const int count =
                                ScanInt(static_cast<bool*>(0));
                            skill.parameters[index].clear();
                            skill.parameters[index].reserve(count);
                            for (int parameter_index = 0;
                                 parameter_index < count;
                                 ++parameter_index) {
                                skill.parameters[index].push_back(
                                    ScanInt(static_cast<bool*>(0)));
                            }
                        } else if (mode == 3) {
                            skill.values_b[index] =
                                ScanInt(static_cast<bool*>(0));
                            skill.values_c[index] =
                                ScanInt(static_cast<bool*>(0));
                        }
                    }
                }
            }
            if (!closed) {
                return 0;
            }
            script->ridable_skills.push_back(skill);
        } else if (tag == "[fix direction]") {
            bool is_string = false;
            bool success = false;
            std::string direction;
            const int value =
                ScanStrOrInt(is_string, direction, success, true);
            if (!success) {
                return 0;
            }
            if (!is_string) {
                script->value_cd4 = static_cast<uint8_t>(value != 0);
            } else {
                script->value_cd4 = 1;
                if (direction == "[left]") {
                    script->value_cd8 = 0;
                } else if (direction == "[right]") {
                    script->value_cd8 = 1;
                } else {
                    return 0;
                }
            }
        } else if (tag == "[control passive object]") {
            bool closed = false;
            std::string field;
            while (ScanType(field, true) == 1) {
                if (field == "[/control passive object]") {
                    closed = true;
                    break;
                }
                if (field == "[index]") {
                    bool success = false;
                    script->values_658[0] = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                } else if (field == "[x pos]") {
                    bool success = false;
                    script->values_658[1] = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                } else if (field == "[y pos]") {
                    bool success = false;
                    script->values_658[2] = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                } else if (field == "[width]") {
                    bool success = false;
                    script->values_658[3] = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                } else if (field == "[height]") {
                    bool success = false;
                    script->values_658[4] = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                } else if (field == "[velocity x]") {
                    bool success = false;
                    const float velocity = ScanFloat(&success);
                    if (!success) {
                        return 0;
                    }
                    std::memcpy(&script->values_658[5],
                                &velocity,
                                sizeof(velocity));
                } else if (field == "[velocity y]") {
                    bool success = false;
                    const float velocity = ScanFloat(&success);
                    if (!success) {
                        return 0;
                    }
                    std::memcpy(&script->values_658[6],
                                &velocity,
                                sizeof(velocity));
                } else if (field == "[option]") {
                    std::string option;
                    if (ScanStr(&option) != 1 || option != "convert y to z") {
                        return 0;
                    }
                    script->value_674 = 1;
                } else if (field == "[transparency]") {
                    bool transparency_closed = false;
                    std::string transparency_field;
                    while (ScanType(transparency_field, true) == 1) {
                        if (transparency_field == "[/transparency]") {
                            transparency_closed = true;
                            break;
                        }
                        if (transparency_field == "[master]") {
                            bool success = false;
                            script->values_678[0] = ScanInt(&success);
                            if (!success) {
                                return 0;
                            }
                        } else if (transparency_field == "[others]") {
                            bool success = false;
                            script->values_678[1] = ScanInt(&success);
                            if (!success) {
                                return 0;
                            }
                        } else if (transparency_field == "[option]") {
                            std::string option;
                            if (ScanStr(&option) != 1 || option != "inherit") {
                                return 0;
                            }
                            script->value_680 = 1;
                        } else {
                            return 0;
                        }
                    }
                    if (!transparency_closed) {
                        return 0;
                    }
                } else if (field == "[fix direction]") {
                    std::string direction;
                    if (ScanStr(&direction) != 1) {
                        return 0;
                    }
                    if (direction == "[left]") {
                        script->value_684 = 0;
                    } else if (direction == "[right]") {
                        script->value_684 = 1;
                    } else {
                        return 0;
                    }
                }
            }
            if (!closed) {
                return 0;
            }
        } else if (tag == "[destroy count]") {
            script->values_6f4[3] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[destroy count damage]") {
            script->values_6f4[4] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[flying mark]") {
            script->values_c70[0] = 1;
            script->values_c70[1] = static_cast<uint8_t>(
                ScanInt(static_cast<bool*>(0)) == 1);
        } else if (tag == "[base table]") {
            std::string table_filename;
            if (ScanStr(&table_filename) != 1) {
                return 0;
            }
            script->base_parameters.clear();
            if (ImportMonsterBaseTable(script, table_filename.c_str()) != 1 ||
                script->base_parameters.empty()) {
                return 0;
            }
        } else if (tag == "[fixed height]") {
            script->values_1cac[0] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[etc sound]") {
            if (ScanStr(&script->strings_694[7]) != 1) {
                return 0;
            }
        } else if (tag == "[can beat index]") {
            script->can_beat_indices.clear();
            bool is_type = false;
            bool success = false;
            std::string closing_tag;
            while (true) {
                const int value =
                    ScanTypeOrInt(is_type, closing_tag, success, true);
                if (!success) {
                    return 0;
                }
                if (is_type) {
                    break;
                }
                script->can_beat_indices.push_back(value);
            }
            if (closing_tag != "[/can beat index]") {
                return 0;
            }
        } else if (tag == "[life max]") {
            script->value_3b8 = static_cast<uint8_t>(
                ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[start immune time]") {
            bool success = false;
            script->value_1cc0 = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[non targeted]") {
            script->value_56c = static_cast<uint8_t>(
                ScanInt(static_cast<bool*>(0)) != 0);
        } else if (tag == "[target appendage]") {
            if (ScanStr(&script->value_570) != 1) {
                return 0;
            }
        } else if (tag == "[face image]") {
            if (ScanStr(&script->value_5ac) != 1) {
                return 0;
            }
            script->value_5b0 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[drop avatar item]") {
            for (int part = 0; part <= 3; ++part) {
                if (ScanFloat(&script->enemy_drop_avatar.part_rates[part]) != 1) {
                    return 0;
                }
            }
            for (int grade = 0; grade <= 4; ++grade) {
                if (ScanFloat(&script->enemy_drop_avatar.grade_rates[grade]) != 1) {
                    return 0;
                }
            }
            bool success = false;
            script->enemy_drop_avatar.base_rate = ScanInt(&success);
            if (!success) {
                return 0;
            }
            script->enemy_drop_avatar.drop_count = ScanInt(&success);
            if (!success) {
                return 0;
            }
            script->enemy_drop_avatar.elements.clear();
            script->enemy_drop_avatar.total_rate = 0;
            while (true) {
                stEnamyDropAvatarElement_t element;
                element.item_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int probability = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                element.avatar_part = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                script->enemy_drop_avatar.total_rate += probability;
                element.accumulated_rate =
                    script->enemy_drop_avatar.total_rate;
                script->enemy_drop_avatar.elements.push_back(element);
            }
        } else if (tag == "[limit slowdown atkspeed]") {
            script->values_1c94[0] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[limit slowdown movespeed]") {
            script->values_1c94[1] = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[slow motion at damaged super armor]") {
            // This is a later client-animation cue.  The old server's
            // 0x1cc4 STMonsterScript has no member or downstream state
            // machine for it, but the serialized leaf must be consumed
            // exactly so it cannot desynchronize the next monster tag.
            bool success = false;
            (void)ScanInt(&success);
            if (!success) {
                return 0;
            }
            float slow_motion_scale = 0.0f;
            if (ScanFloat(&slow_motion_scale) != 1 ||
                ScanFloat(&slow_motion_scale) != 1) {
                return 0;
            }
            std::string effect_name;
            if (ScanStr(&effect_name) != 1 || effect_name != "none") {
                return 0;
            }
        } else if (tag == "[getup invincible]") {
            // CLIENT_EA sub_15A0960 / 0x15A5C94: the 0828 client stores a
            // literal-one test at its appended monster-script byte +0x45dc.
            bool success = false;
            const int enabled = ScanInt(&success);
            if (!success) {
                return 0;
            }
            script->getup_invincible = enabled == 1 ? 1 : 0;
        } else if (tag == "[deathless]") {
            // CLIENT_EA 0x15AB968: literal-one assignment through the
            // client's boolean-property wrapper at new-script +0x5c8.
            bool success = false;
            const int enabled = ScanInt(&success);
            if (!success) {
                return 0;
            }
            script->deathless = enabled == 1 ? 1 : 0;
        } else if (tag == "[impossible hp recovery]") {
            // CLIENT_EA 0x15ABA14: nonzero test -> new-script +0x45dd.
            bool success = false;
            const int enabled = ScanInt(&success);
            if (!success) {
                return 0;
            }
            script->impossible_hp_recovery = enabled != 0 ? 1 : 0;
        } else if (tag == "[disable appendage]") {
            // CLIENT_EA 0x15ABA30: nonzero test -> new-script +0x45de.
            bool success = false;
            const int enabled = ScanInt(&success);
            if (!success) {
                return 0;
            }
            script->disable_appendage = enabled != 0 ? 1 : 0;
        } else if (tag == "[champion multi factor]") {
            if (pending_champion_multi_factor) {
                return 0;
            }
            pending_champion_multi_factor = true;
        } else if (tag == "[fastmove]" || tag == "[fastattack]") {
            if (!pending_champion_multi_factor) {
                return 0;
            }
            bool success = false;
            const float multiplier = ScanFloat(&success);
            if (!success || multiplier <= 0.0f) {
                return 0;
            }
            // CLIENT_EA 0x15AB98E stores the numeric value in the 0828
            // champion-factor map keyed by the nested tag.  The official
            // 0828 PVF inventory contains exactly these two keys.
            if (tag == "[fastmove]") {
                script->champion_fastmove_factor = multiplier;
                script->has_champion_fastmove_factor = 1;
            } else {
                script->champion_fastattack_factor = multiplier;
                script->has_champion_fastattack_factor = 1;
            }
            pending_champion_multi_factor = false;
        } else {
            bool found_active_status = false;
            for (int index = 0; index <= 16; ++index) {
                if (tag == kActiveStatusTag[index]) {
                    script->values_414[2 * index] =
                        ScanInt(static_cast<bool*>(0));
                    script->values_414[2 * index + 1] =
                        ScanInt(static_cast<bool*>(0));
                    found_active_status = true;
                    break;
                }
            }
            if (!found_active_status) {
                return 0;
            }
        }

        // IDA 0x8A22308..0x8A2232D: the server-only importer finishes as
        // soon as [name] has been consumed, but it still resolves the real
        // list identity through the same common success exit as normal EOF.
        if (server_only && has_name) {
            script->index =
                g_stMonsterScriptFileList.FindIndexByFullScan(filename);
            return 1;
        }
    }

    // IDA LABEL_4 -> LABEL_2093: ScanType EOF is the ordinary successful
    // completion path and always writes STMonsterScript+0.  Without this,
    // CMonsterManager loads every script under index zero and later
    // find_monster(monster_id) rejects valid CMD42 deaths.
    if (pending_champion_multi_factor) {
        return 0;
    }
    script->index =
        g_stMonsterScriptFileList.FindIndexByFullScan(filename);
    return 1;
}
