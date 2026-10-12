#ifndef OLDSRV_OLD_DATA_PVF_ETC_DATAMANAGER_OPTIONAL_FEATURES_0828_H_
#define OLDSRV_OLD_DATA_PVF_ETC_DATAMANAGER_OPTIONAL_FEATURES_0828_H_

#include <string>
#include <vector>

// These resources are present in the official 0828 PVF, but neither the 0627
// server pseudo-code nor the current game tree identifies an authoritative
// gameplay transaction for them.  Keep the scanner token kind and decoded
// value losslessly in a rebuild-owned sidecar; this does not promise original
// textual spelling.  Read-only access is not permission to apply league,
// broadcast-PvP, equipment-upgrade, or reward policy.
enum { COUNTER_REVISION_VALUE_COUNT = 20 };

enum CounterRevisionNumberKind {
    COUNTER_REVISION_NUMBER_INTEGER = 0,
    COUNTER_REVISION_NUMBER_FLOAT = 1
};

struct CounterRevisionNumber {
    CounterRevisionNumber();

    CounterRevisionNumberKind kind;
    int integer_value;
    float float_value;
};

struct CounterRevisionEntry {
    CounterRevisionEntry();

    std::string target_job;
    std::string target_grow_type;
    CounterRevisionNumber values[COUNTER_REVISION_VALUE_COUNT];
};

struct CounterRevisionGrowTable {
    std::string source_job;
    std::string source_grow_type;
    std::vector<CounterRevisionEntry> entries;
};

struct CounterRevisionFile {
    std::string script_path;
    std::vector<CounterRevisionGrowTable> grow_tables;
};

struct LeagueEmblemBalanceConfig {
    LeagueEmblemBalanceConfig();

    int all_stat;
    int ban_platinum;
};

struct BlockedRandomOptionContext {
    std::string context;
    std::vector<int> option_indices;
};

struct UniqueExperienceRewardRecord {
    UniqueExperienceRewardRecord();

    std::string job_tag;
    int values[4];
};

struct UniqueExperienceConfig {
    UniqueExperienceConfig();

    int equip_item_upgrade;
    std::vector<UniqueExperienceRewardRecord> reward_records;
};

struct DataManagerOptionalFeatureStore {
    std::vector<CounterRevisionFile> counter_revision_files;
    LeagueEmblemBalanceConfig league_emblem_balance;
    std::vector<BlockedRandomOptionContext> blocked_random_options;
    UniqueExperienceConfig unique_experience;

    void clear();
    void swap(DataManagerOptionalFeatureStore& other);
};

// Loads all twelve official resources into a temporary generation and
// publishes only when every grammar and required wrapper validates.
int ImportDataManagerOptionalFeatures();

DataManagerOptionalFeatureStore GetDataManagerOptionalFeatureStore();
std::vector<CounterRevisionFile> GetCounterRevisionFiles();
bool FindCounterRevisionGrowTable(
    const std::string& source_job,
    const std::string& source_grow_type,
    CounterRevisionGrowTable* output);
bool FindCounterRevisionEntry(
    const std::string& source_job,
    const std::string& source_grow_type,
    const std::string& target_job,
    const std::string& target_grow_type,
    CounterRevisionEntry* output);
LeagueEmblemBalanceConfig GetLeagueEmblemBalanceConfig();
bool FindBlockedRandomOptionContext(
    const std::string& context,
    BlockedRandomOptionContext* output);
UniqueExperienceConfig GetUniqueExperienceConfig();
int GetUniqueExperienceEquipItemUpgrade();

#endif  // OLDSRV_OLD_DATA_PVF_ETC_DATAMANAGER_OPTIONAL_FEATURES_0828_H_
