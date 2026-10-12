#include "old_data/PVF/etc/datamanager_optional_features_0828.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"

#include <algorithm>
#include <cfloat>
#include <set>
#include <utility>

CounterRevisionNumber::CounterRevisionNumber()
    : kind(COUNTER_REVISION_NUMBER_INTEGER),
      integer_value(0),
      float_value(0.0f) {}

CounterRevisionEntry::CounterRevisionEntry() {}

LeagueEmblemBalanceConfig::LeagueEmblemBalanceConfig()
    : all_stat(0), ban_platinum(0) {}

UniqueExperienceRewardRecord::UniqueExperienceRewardRecord() {
    for (int index = 0; index < 4; ++index) {
        values[index] = 0;
    }
}

UniqueExperienceConfig::UniqueExperienceConfig() : equip_item_upgrade(0) {}

void DataManagerOptionalFeatureStore::clear() {
    counter_revision_files.clear();
    league_emblem_balance = LeagueEmblemBalanceConfig();
    blocked_random_options.clear();
    unique_experience = UniqueExperienceConfig();
}

void DataManagerOptionalFeatureStore::swap(
    DataManagerOptionalFeatureStore& other) {
    counter_revision_files.swap(other.counter_revision_files);
    std::swap(league_emblem_balance.all_stat,
              other.league_emblem_balance.all_stat);
    std::swap(league_emblem_balance.ban_platinum,
              other.league_emblem_balance.ban_platinum);
    blocked_random_options.swap(other.blocked_random_options);
    std::swap(unique_experience.equip_item_upgrade,
              other.unique_experience.equip_item_upgrade);
    unique_experience.reward_records.swap(
        other.unique_experience.reward_records);
}

namespace {

struct OptionalFeatureToken {
    OptionalFeatureToken(DNFLEX_TOKEN token_type,
                         int integer,
                         float real,
                         const std::string& token_text)
        : type(token_type),
          int_value(integer),
          float_value(real),
          text(token_text) {}

    DNFLEX_TOKEN type;
    int int_value;
    float float_value;
    std::string text;
};

struct CounterRevisionFileSpec {
    const char* path;
    const char* source_job;
    int expected_grow_tables;
    int expected_entries;
};

const CounterRevisionFileSpec kCounterRevisionFiles[] = {
    {"Etc/PvPRevisionTable/CharacterRevisionTable/atfighter.etc",
     "[at fighter]", 5, 220},
    {"Etc/PvPRevisionTable/CharacterRevisionTable/atgunner.etc",
     "[at gunner]", 5, 220},
    {"Etc/PvPRevisionTable/CharacterRevisionTable/atmage.etc",
     "[at mage]", 3, 132},
    {"Etc/PvPRevisionTable/CharacterRevisionTable/fighter.etc",
     "[fighter]", 5, 220},
    {"Etc/PvPRevisionTable/CharacterRevisionTable/gunner.etc",
     "[gunner]", 5, 220},
    {"Etc/PvPRevisionTable/CharacterRevisionTable/mage.etc",
     "[mage]", 5, 220},
    {"Etc/PvPRevisionTable/CharacterRevisionTable/priest.etc",
     "[priest]", 5, 220},
    {"Etc/PvPRevisionTable/CharacterRevisionTable/swordman.etc",
     "[swordman]", 5, 220},
    {"Etc/PvPRevisionTable/CharacterRevisionTable/thief.etc",
     "[thief]", 3, 132},
};

enum {
    COUNTER_REVISION_TOTAL_GROW_TABLE_COUNT = 41,
    COUNTER_REVISION_TOTAL_ENTRY_COUNT = 1804,
    BLOCKED_RANDOM_OPTION_CONTEXT_COUNT = 1,
    BLOCKED_RANDOM_OPTION_VALUE_COUNT = 11,
    UNIQUE_EXPERIENCE_REWARD_COUNT = 46
};

DataManagerOptionalFeatureStore g_optional_feature_store;

bool IsIntegerToken(DNFLEX_TOKEN token) {
    return token == DNF_TOK_DECIMAL || token == DNF_TOK_HEXADECIMAL;
}

bool IsNumberToken(DNFLEX_TOKEN token) {
    return IsIntegerToken(token) || token == DNF_TOK_FLOAT;
}

bool ReadScriptTokens(const char* path,
                      std::vector<OptionalFeatureToken>* output) {
    if (path == 0 || output == 0 ||
        !loadRDARScriptFile("", path)) {
        return false;
    }

    std::vector<OptionalFeatureToken> next;
    bool valid = true;
    while (valid) {
        int int_value = 0;
        float float_value = 0.0f;
        std::string text;
        const DNFLEX_TOKEN token =
            ScanAll(&int_value, &float_value, &text);
        if (token == DNF_TOK_EOF) {
            break;
        }
        if (token == DNF_TOK_ERROR || token == DNF_TOK_INCLUDE ||
            token == DNF_TOK_CONNECTOR || token == DNF_TOK_PAIR_INT ||
            token == DNF_TOK_PAIR_STR || token == DNF_TOK_CUSTOM_DATA) {
            valid = false;
            break;
        }
        next.push_back(
            OptionalFeatureToken(token, int_value, float_value, text));
    }
    unloadRDARScriptFile();

    if (!valid || next.empty()) {
        return false;
    }
    output->swap(next);
    return true;
}

bool ReadTag(const std::vector<OptionalFeatureToken>& tokens,
             std::size_t* cursor,
             const char* expected) {
    if (cursor == 0 || expected == 0 || *cursor >= tokens.size() ||
        tokens[*cursor].type != DNF_TOK_TYPE ||
        tokens[*cursor].text != expected) {
        return false;
    }
    ++*cursor;
    return true;
}

bool ReadString(const std::vector<OptionalFeatureToken>& tokens,
                std::size_t* cursor,
                std::string* output) {
    if (cursor == 0 || output == 0 || *cursor >= tokens.size() ||
        tokens[*cursor].type != DNF_TOK_STRING ||
        tokens[*cursor].text.empty()) {
        return false;
    }
    *output = tokens[*cursor].text;
    ++*cursor;
    return true;
}

bool ReadInteger(const std::vector<OptionalFeatureToken>& tokens,
                 std::size_t* cursor,
                 int* output) {
    if (cursor == 0 || output == 0 || *cursor >= tokens.size() ||
        !IsIntegerToken(tokens[*cursor].type)) {
        return false;
    }
    *output = tokens[*cursor].int_value;
    ++*cursor;
    return true;
}

bool ReadCounterRevisionNumber(
    const std::vector<OptionalFeatureToken>& tokens,
    std::size_t* cursor,
    CounterRevisionNumber* output) {
    if (cursor == 0 || output == 0 || *cursor >= tokens.size() ||
        !IsNumberToken(tokens[*cursor].type)) {
        return false;
    }
    const OptionalFeatureToken& token = tokens[*cursor];
    if (IsIntegerToken(token.type)) {
        output->kind = COUNTER_REVISION_NUMBER_INTEGER;
        output->integer_value = token.int_value;
        output->float_value = 0.0f;
    } else {
        if (token.float_value != token.float_value ||
            token.float_value > FLT_MAX || token.float_value < -FLT_MAX) {
            return false;
        }
        output->kind = COUNTER_REVISION_NUMBER_FLOAT;
        output->integer_value = 0;
        output->float_value = token.float_value;
    }
    ++*cursor;
    return true;
}

bool ParseCounterRevisionFile(const CounterRevisionFileSpec& spec,
                              CounterRevisionFile* output) {
    if (output == 0) {
        return false;
    }
    std::vector<OptionalFeatureToken> tokens;
    if (!ReadScriptTokens(spec.path, &tokens)) {
        return false;
    }

    CounterRevisionFile next;
    next.script_path = spec.path;
    std::set<std::pair<std::string, std::string> > source_keys;
    int entry_count = 0;
    std::size_t cursor = 0u;
    if (!ReadTag(tokens, &cursor, "[counter revision table]")) {
        return false;
    }

    while (cursor < tokens.size() &&
           !(tokens[cursor].type == DNF_TOK_TYPE &&
             tokens[cursor].text == "[/counter revision table]")) {
        CounterRevisionGrowTable table;
        if (!ReadTag(tokens, &cursor, "[table grow type]") ||
            !ReadString(tokens, &cursor, &table.source_job) ||
            !ReadString(tokens, &cursor, &table.source_grow_type) ||
            table.source_job != spec.source_job ||
            !source_keys.insert(std::make_pair(table.source_job,
                                               table.source_grow_type)).second ||
            !ReadTag(tokens, &cursor, "[data entry]")) {
            return false;
        }

        std::set<std::pair<std::string, std::string> > target_keys;
        while (cursor < tokens.size() &&
               !(tokens[cursor].type == DNF_TOK_TYPE &&
                 tokens[cursor].text == "[/data entry]")) {
            CounterRevisionEntry entry;
            if (!ReadString(tokens, &cursor, &entry.target_job) ||
                !ReadString(tokens, &cursor, &entry.target_grow_type) ||
                !target_keys.insert(std::make_pair(entry.target_job,
                                                   entry.target_grow_type)).second) {
                return false;
            }
            for (int index = 0; index < COUNTER_REVISION_VALUE_COUNT; ++index) {
                if (!ReadCounterRevisionNumber(tokens, &cursor,
                                               &entry.values[index])) {
                    return false;
                }
            }
            if (cursor < tokens.size() &&
                tokens[cursor].type != DNF_TOK_STRING &&
                tokens[cursor].type != DNF_TOK_TYPE) {
                return false;
            }
            table.entries.push_back(entry);
            ++entry_count;
        }

        if (table.entries.empty() ||
            !ReadTag(tokens, &cursor, "[/data entry]") ||
            !ReadTag(tokens, &cursor, "[/table grow type]")) {
            return false;
        }
        next.grow_tables.push_back(table);
    }

    if (next.grow_tables.size() !=
            static_cast<std::size_t>(spec.expected_grow_tables) ||
        entry_count != spec.expected_entries ||
        !ReadTag(tokens, &cursor, "[/counter revision table]") ||
        cursor != tokens.size()) {
        return false;
    }
    output->script_path.swap(next.script_path);
    output->grow_tables.swap(next.grow_tables);
    return true;
}

bool ParseLeagueEmblemBalance(LeagueEmblemBalanceConfig* output) {
    if (output == 0) {
        return false;
    }
    std::vector<OptionalFeatureToken> tokens;
    if (!ReadScriptTokens("Etc/LeagueEmblemBalanceEtc.etc", &tokens)) {
        return false;
    }

    LeagueEmblemBalanceConfig next;
    unsigned int seen = 0u;
    std::size_t cursor = 0u;
    while (cursor < tokens.size()) {
        if (tokens[cursor].type != DNF_TOK_TYPE) {
            return false;
        }
        const std::string tag = tokens[cursor].text;
        ++cursor;
        if (tag == "[all stat]") {
            if ((seen & 1u) != 0u ||
                !ReadInteger(tokens, &cursor, &next.all_stat)) {
                return false;
            }
            seen |= 1u;
        } else if (tag == "[ban platinum]") {
            if ((seen & 2u) != 0u ||
                !ReadInteger(tokens, &cursor, &next.ban_platinum)) {
                return false;
            }
            seen |= 2u;
        } else {
            return false;
        }
    }
    if (seen != 3u) {
        return false;
    }
    *output = next;
    return true;
}

bool ParseBlockedRandomOptions(
    std::vector<BlockedRandomOptionContext>* output) {
    if (output == 0) {
        return false;
    }
    std::vector<OptionalFeatureToken> tokens;
    if (!ReadScriptTokens("Etc/RandomOption/BlockedOptions.etc", &tokens)) {
        return false;
    }

    std::vector<BlockedRandomOptionContext> next;
    std::set<std::string> contexts;
    int option_count = 0;
    std::size_t cursor = 0u;
    if (!ReadTag(tokens, &cursor, "[blocked random option]")) {
        return false;
    }
    while (cursor < tokens.size() &&
           !(tokens[cursor].type == DNF_TOK_TYPE &&
             tokens[cursor].text == "[/blocked random option]")) {
        BlockedRandomOptionContext row;
        if (!ReadString(tokens, &cursor, &row.context) ||
            !contexts.insert(row.context).second) {
            return false;
        }
        std::set<int> option_indices;
        while (cursor < tokens.size() &&
               IsIntegerToken(tokens[cursor].type)) {
            int value = 0;
            if (!ReadInteger(tokens, &cursor, &value) || value < 0 ||
                !option_indices.insert(value).second) {
                return false;
            }
            row.option_indices.push_back(value);
            ++option_count;
        }
        if (row.option_indices.empty()) {
            return false;
        }
        next.push_back(row);
    }
    if (next.size() != BLOCKED_RANDOM_OPTION_CONTEXT_COUNT ||
        option_count != BLOCKED_RANDOM_OPTION_VALUE_COUNT ||
        !ReadTag(tokens, &cursor, "[/blocked random option]") ||
        cursor != tokens.size()) {
        return false;
    }
    output->swap(next);
    return true;
}

bool ParseUniqueExperience(UniqueExperienceConfig* output) {
    if (output == 0) {
        return false;
    }
    std::vector<OptionalFeatureToken> tokens;
    if (!ReadScriptTokens("Etc/unique_experience.etc", &tokens)) {
        return false;
    }

    UniqueExperienceConfig next;
    bool saw_upgrade = false;
    bool saw_rewards = false;
    std::set<std::pair<std::string, int> > reward_keys;
    std::size_t cursor = 0u;
    while (cursor < tokens.size()) {
        if (tokens[cursor].type != DNF_TOK_TYPE) {
            return false;
        }
        const std::string tag = tokens[cursor].text;
        ++cursor;
        if (tag == "[equip item upgrade]") {
            if (saw_upgrade ||
                !ReadInteger(tokens, &cursor, &next.equip_item_upgrade)) {
                return false;
            }
            saw_upgrade = true;
        } else if (tag == "[equip item reward]") {
            if (saw_rewards) {
                return false;
            }
            saw_rewards = true;
            while (cursor < tokens.size() &&
                   !(tokens[cursor].type == DNF_TOK_TYPE &&
                     tokens[cursor].text == "[/equip item reward]")) {
                if (tokens[cursor].type != DNF_TOK_TYPE ||
                    tokens[cursor].text.size() < 3u ||
                    tokens[cursor].text[1] == '/') {
                    return false;
                }
                UniqueExperienceRewardRecord record;
                record.job_tag = tokens[cursor].text;
                ++cursor;
                for (int index = 0; index < 4; ++index) {
                    if (!ReadInteger(tokens, &cursor, &record.values[index])) {
                        return false;
                    }
                }
                if (!reward_keys.insert(
                        std::make_pair(record.job_tag,
                                       record.values[0])).second) {
                    return false;
                }
                const std::string close_tag =
                    "[/" + record.job_tag.substr(1u);
                if (!ReadTag(tokens, &cursor, close_tag.c_str())) {
                    return false;
                }
                next.reward_records.push_back(record);
            }
            if (!ReadTag(tokens, &cursor, "[/equip item reward]")) {
                return false;
            }
        } else {
            return false;
        }
    }
    if (!saw_upgrade || !saw_rewards ||
        next.reward_records.size() != UNIQUE_EXPERIENCE_REWARD_COUNT) {
        return false;
    }
    output->equip_item_upgrade = next.equip_item_upgrade;
    output->reward_records.swap(next.reward_records);
    return true;
}

}  // namespace

int ImportDataManagerOptionalFeatures() {
    DataManagerOptionalFeatureStore next;
    std::set<std::pair<std::string, std::string> > all_source_keys;
    int total_grow_tables = 0;
    int total_entries = 0;
    const std::size_t file_count =
        sizeof(kCounterRevisionFiles) / sizeof(kCounterRevisionFiles[0]);
    for (std::size_t index = 0u; index < file_count; ++index) {
        CounterRevisionFile file;
        if (!ParseCounterRevisionFile(kCounterRevisionFiles[index], &file)) {
            return 0;
        }
        for (std::size_t table_index = 0u;
             table_index < file.grow_tables.size(); ++table_index) {
            const CounterRevisionGrowTable& table =
                file.grow_tables[table_index];
            if (!all_source_keys.insert(
                    std::make_pair(table.source_job,
                                   table.source_grow_type)).second) {
                return 0;
            }
            ++total_grow_tables;
            total_entries += static_cast<int>(table.entries.size());
        }
        next.counter_revision_files.push_back(file);
    }
    if (total_grow_tables != COUNTER_REVISION_TOTAL_GROW_TABLE_COUNT ||
        total_entries != COUNTER_REVISION_TOTAL_ENTRY_COUNT ||
        !ParseLeagueEmblemBalance(&next.league_emblem_balance) ||
        !ParseBlockedRandomOptions(&next.blocked_random_options) ||
        !ParseUniqueExperience(&next.unique_experience)) {
        return 0;
    }

    g_optional_feature_store.swap(next);
    return 1;
}

DataManagerOptionalFeatureStore GetDataManagerOptionalFeatureStore() {
    return g_optional_feature_store;
}

std::vector<CounterRevisionFile> GetCounterRevisionFiles() {
    return g_optional_feature_store.counter_revision_files;
}

bool FindCounterRevisionGrowTable(
    const std::string& source_job,
    const std::string& source_grow_type,
    CounterRevisionGrowTable* output) {
    if (output == 0) {
        return false;
    }
    const std::vector<CounterRevisionFile>& files =
        g_optional_feature_store.counter_revision_files;
    for (std::size_t file_index = 0u; file_index < files.size(); ++file_index) {
        const std::vector<CounterRevisionGrowTable>& tables =
            files[file_index].grow_tables;
        for (std::size_t table_index = 0u;
             table_index < tables.size(); ++table_index) {
            if (tables[table_index].source_job == source_job &&
                tables[table_index].source_grow_type == source_grow_type) {
                *output = tables[table_index];
                return true;
            }
        }
    }
    return false;
}

bool FindCounterRevisionEntry(
    const std::string& source_job,
    const std::string& source_grow_type,
    const std::string& target_job,
    const std::string& target_grow_type,
    CounterRevisionEntry* output) {
    if (output == 0) {
        return false;
    }
    CounterRevisionGrowTable table;
    if (!FindCounterRevisionGrowTable(source_job, source_grow_type, &table)) {
        return false;
    }
    for (std::size_t index = 0u; index < table.entries.size(); ++index) {
        if (table.entries[index].target_job == target_job &&
            table.entries[index].target_grow_type == target_grow_type) {
            *output = table.entries[index];
            return true;
        }
    }
    return false;
}

LeagueEmblemBalanceConfig GetLeagueEmblemBalanceConfig() {
    return g_optional_feature_store.league_emblem_balance;
}

bool FindBlockedRandomOptionContext(
    const std::string& context,
    BlockedRandomOptionContext* output) {
    if (output == 0) {
        return false;
    }
    const std::vector<BlockedRandomOptionContext>& rows =
        g_optional_feature_store.blocked_random_options;
    for (std::size_t index = 0u; index < rows.size(); ++index) {
        if (rows[index].context == context) {
            *output = rows[index];
            return true;
        }
    }
    return false;
}

UniqueExperienceConfig GetUniqueExperienceConfig() {
    return g_optional_feature_store.unique_experience;
}

int GetUniqueExperienceEquipItemUpgrade() {
    return g_optional_feature_store.unique_experience.equip_item_upgrade;
}
