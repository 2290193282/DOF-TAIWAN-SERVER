#include "old_data/PVF/etc/equipment_growth.h"

#include "old_data/PVF/common/rdar_script.h"

#include <algorithm>
#include <map>
#include <string>


namespace equipment_growth {
namespace {

struct StageRule {
    StageRule()
        : maximum(0),
          transition_flag(0),
          fatigue_threshold(0),
          fatigue_percent(100) {}

    uint16_t maximum;
    uint8_t transition_flag;
    uint8_t fatigue_threshold;
    uint8_t fatigue_percent;
};

struct Config {
    Config()
        : group(0),
          max_level(0),
          death_point(0),
          monster_kind_percent(),
          difficulty_percent(),
          stages() {}

    int group;
    int max_level;
    int death_point;
    std::map<int, int> monster_kind_percent;
    std::map<int, int> difficulty_percent;
    std::map<int, StageRule> stages;
};

typedef std::map<int, PrototypeIdentity> PrototypeTable;

Config& PublishedConfig() {
    static Config value;
    return value;
}

bool& LoadedFlag() {
    static bool value = false;
    return value;
}

PrototypeTable& Prototypes() {
    static PrototypeTable value;
    return value;
}

int NormalizeMonsterKindForGrowth(int monster_kind) {
    // The 0627 battle-field ABI uses zero for an ordinary monster, while the
    // 0828 equipment_grow.etc correction table starts its ordinary bucket at
    // one.  The remaining server values already overlap the PVF domain, so do
    // not shift champion/named/boss/special values.
    return monster_kind == 0 ? 1 : monster_kind;
}

bool ScanRequiredInt(int* value) {
    if (!value) return false;
    bool success = false;
    *value = ScanInt(&success);
    return success;
}

bool ScanRequiredString(std::string* value) {
    return value && ScanStr(value) == 1;
}

bool ScanRequiredTag(const char* expected) {
    std::string tag;
    return expected && ScanType(tag, true) == 1 && tag == expected;
}

bool ParseAcquire(Config* config) {
    if (!config) return false;

    std::string source;
    int value = 0;
    if (!ScanRequiredString(&source) || source != "death" ||
        !ScanRequiredTag("[point]") || !ScanRequiredInt(&value) ||
        value <= 0 || value > 0xffff) {
        return false;
    }
    config->death_point = value;

    if (!ScanRequiredTag("[kind]") || !ScanRequiredInt(&value) ||
        value <= 0 || value > 32) {
        return false;
    }
    const int kind_count = value;
    for (int index = 0; index < kind_count; ++index) {
        int kind = 0;
        int percent = 0;
        if (!ScanRequiredInt(&kind) || !ScanRequiredInt(&percent) ||
            kind < 0 || kind > 255 || percent < 0 || percent > 10000 ||
            !config->monster_kind_percent.insert(
                std::make_pair(kind, percent)).second) {
            return false;
        }
    }

    // The 0828 file contains the compact six-integer level-correction row
    // `3 3 10 5 90 10`.  Its exact server-side interpolation is not present
    // in the client binary.  Consume and validate the complete row here, but
    // do not invent an interpretation in CalculateDeathAward.
    if (!ScanRequiredTag("[level correction]")) return false;
    int level_correction[6];
    for (int index = 0; index < 6; ++index) {
        if (!ScanRequiredInt(&level_correction[index]) ||
            level_correction[index] < 0 ||
            level_correction[index] > 10000) {
            return false;
        }
    }
    (void)level_correction;

    if (!ScanRequiredTag("[difficulty correction]")) return false;
    for (int difficulty = 0; difficulty < 5; ++difficulty) {
        int percent = 0;
        if (!ScanRequiredInt(&percent) || percent < 0 || percent > 10000) {
            return false;
        }
        config->difficulty_percent.insert(
            std::make_pair(difficulty, percent));
    }
    return ScanRequiredTag("[/exp acquire]");
}

bool ParseStage(Config* config) {
    if (!config) return false;

    int stage = 0;
    int maximum = 0;
    int transition_flag = 0;
    if (!ScanRequiredInt(&stage) || !ScanRequiredInt(&maximum) ||
        !ScanRequiredInt(&transition_flag) || stage <= 0 || stage > 255 ||
        maximum < 0 || maximum > 0xffff || transition_flag < 0 ||
        transition_flag > 1) {
        return false;
    }

    StageRule rule;
    rule.maximum = static_cast<uint16_t>(maximum);
    rule.transition_flag = static_cast<uint8_t>(transition_flag);
    if (maximum != 0) {
        std::string penalty_name;
        int threshold = 0;
        int percent = 0;
        if (!ScanRequiredTag("[penalty]") ||
            !ScanRequiredString(&penalty_name) ||
            penalty_name != "fatigue" || !ScanRequiredInt(&threshold) ||
            !ScanRequiredInt(&percent) || threshold <= 0 ||
            threshold > 255 || percent < 0 || percent > 100 ||
            !ScanRequiredTag("[/penalty]")) {
            return false;
        }
        rule.fatigue_threshold = static_cast<uint8_t>(threshold);
        rule.fatigue_percent = static_cast<uint8_t>(percent);
    }
    if (!ScanRequiredTag("[/exp base]") ||
        !config->stages.insert(std::make_pair(stage, rule)).second) {
        return false;
    }
    return true;
}

bool ValidateConfig(const Config& config) {
    if (config.group <= 0 || config.group > 255 || config.max_level <= 0 ||
        config.max_level > 255 || config.death_point <= 0 ||
        config.monster_kind_percent.empty() ||
        config.difficulty_percent.size() != 5u ||
        config.stages.size() != static_cast<size_t>(config.max_level)) {
        return false;
    }
    for (int stage = 1; stage <= config.max_level; ++stage) {
        const std::map<int, StageRule>::const_iterator found =
            config.stages.find(stage);
        if (found == config.stages.end()) return false;
        if (stage == config.max_level) {
            if (found->second.maximum != 0) return false;
        } else if (found->second.maximum == 0 ||
                   found->second.fatigue_threshold == 0) {
            return false;
        }
    }
    return true;
}

}  // namespace


PrototypeIdentity::PrototypeIdentity() : type(0), stage(0) {}

bool ImportEquipmentGrowthScript(const char* filename) {
    if (!filename || !loadRDARScriptFile("", filename)) return false;

    Config parsed;
    bool valid = ScanRequiredTag("[group]") &&
                 ScanRequiredInt(&parsed.group) && parsed.group > 0 &&
                 ScanRequiredTag("[max level]") &&
                 ScanRequiredInt(&parsed.max_level) &&
                 parsed.max_level > 0 &&
                 ScanRequiredTag("[exp acquire]") && ParseAcquire(&parsed);
    bool closed_group = false;
    while (valid) {
        std::string tag;
        if (ScanType(tag, true) != 1) {
            valid = false;
            break;
        }
        if (tag == "[/group]") {
            closed_group = true;
            break;
        }
        if (tag != "[exp base]" || !ParseStage(&parsed)) {
            valid = false;
            break;
        }
    }
    unloadRDARScriptFile();

    if (!valid || !closed_group || !ValidateConfig(parsed)) return false;
    PublishedConfig() = parsed;
    LoadedFlag() = true;
    return true;
}

bool IsConfigLoaded() {
    return LoadedFlag();
}

void RegisterPrototype(int item_id, uint8_t type, uint8_t stage) {
    PrototypeTable& table = Prototypes();
    table.erase(item_id);
    if (item_id <= 0 || type == 0 || stage == 0) return;

    PrototypeIdentity identity;
    identity.type = type;
    identity.stage = stage;
    table.insert(std::make_pair(item_id, identity));
}

bool FindPrototype(int item_id, PrototypeIdentity* identity) {
    if (identity) *identity = PrototypeIdentity();
    const PrototypeTable::const_iterator found = Prototypes().find(item_id);
    if (found == Prototypes().end()) return false;
    if (identity) *identity = found->second;
    return true;
}

unsigned int GetPrototypeCount() {
    return static_cast<unsigned int>(Prototypes().size());
}

uint16_t GetStageMaximum(uint8_t type, uint8_t stage) {
    if (!LoadedFlag() || type != PublishedConfig().group) return 0;
    const std::map<int, StageRule>::const_iterator found =
        PublishedConfig().stages.find(stage);
    return found == PublishedConfig().stages.end() ? 0
                                                   : found->second.maximum;
}

uint16_t CalculateDeathAward(uint8_t type,
                             uint8_t stage,
                             int monster_kind,
                             int monster_level,
                             int character_level,
                             int dungeon_difficulty,
                             uint8_t current_fatigue_state,
                             uint8_t* next_fatigue_state) {
    (void)monster_level;
    (void)character_level;
    if (next_fatigue_state) *next_fatigue_state = current_fatigue_state;
    if (!LoadedFlag() || type != PublishedConfig().group ||
        GetStageMaximum(type, stage) == 0) {
        return 0;
    }

    const int growth_monster_kind = NormalizeMonsterKindForGrowth(monster_kind);
    const std::map<int, int>::const_iterator kind =
        PublishedConfig().monster_kind_percent.find(growth_monster_kind);
    const std::map<int, int>::const_iterator difficulty =
        PublishedConfig().difficulty_percent.find(dungeon_difficulty);
    const std::map<int, StageRule>::const_iterator stage_rule =
        PublishedConfig().stages.find(stage);
    if (kind == PublishedConfig().monster_kind_percent.end() ||
        difficulty == PublishedConfig().difficulty_percent.end() ||
        stage_rule == PublishedConfig().stages.end()) {
        return 0;
    }

    long award = PublishedConfig().death_point;
    award = award * kind->second / 100;
    award = award * difficulty->second / 100;

    const uint8_t next_state = current_fatigue_state == 0xffu
                                   ? 0xffu
                                   : static_cast<uint8_t>(
                                         current_fatigue_state + 1u);
    if (next_fatigue_state) *next_fatigue_state = next_state;
    if (next_state >= stage_rule->second.fatigue_threshold) {
        award = award * stage_rule->second.fatigue_percent / 100;
    }
    if (award <= 0 && kind->second > 0 && difficulty->second > 0) award = 1;
    return static_cast<uint16_t>(std::min<long>(award, 0xffffL));
}

}  // namespace equipment_growth_0828
