#include "old_data/PVF/etc/compound_creature_script.h"

#include <limits.h>
#include <pthread.h>

#include <set>
#include <string>
#include <utility>

#include "common/manager/mt_rand.h"
#include "old_data/PVF/common/rdar_script.h"


namespace compound_creature {
namespace {

Config& PublishedConfig() {
    static Config config;
    return config;
}

bool& LoadedFlag() {
    static bool loaded = false;
    return loaded;
}

std::set<int>& UncompoundablePrototypes() {
    static std::set<int> item_ids;
    return item_ids;
}

pthread_mutex_t g_random_mutex = PTHREAD_MUTEX_INITIALIZER;

class RandomLock {
public:
    RandomLock() { pthread_mutex_lock(&g_random_mutex); }
    ~RandomLock() { pthread_mutex_unlock(&g_random_mutex); }

private:
    RandomLock(const RandomLock&);
    RandomLock& operator=(const RandomLock&);
};

CMTRand& Random() {
    static CMTRand random;
    return random;
}

bool ReadSingleInt(const char* closing_tag, int* value) {
    bool success = false;
    const int parsed = ScanInt(&success);
    std::string tag;
    if (!success || parsed < 0 || parsed > 10000 ||
        ScanType(tag, true) != 1 || tag != closing_tag) {
        return false;
    }
    *value = parsed;
    return true;
}

bool ReadEggRates(int values[3]) {
    for (int index = 0; index != 3; ++index) {
        bool is_type = false;
        bool success = false;
        std::string token;
        const int value = ScanTypeOrInt(is_type, token, success, true);
        if (!success || is_type || value < 0 || value > 10000) return false;
        values[index] = value;
    }
    std::string closing;
    return ScanType(closing, true) == 1 && closing == "[/egg rate]";
}

bool ReadWeightedResults(const char* closing_tag,
                         std::vector<WeightedResult>* results) {
    std::set<int> item_ids;
    int64_t total_weight = 0;
    while (1) {
        bool is_type = false;
        bool success = false;
        std::string token;
        const int item_id = ScanTypeOrInt(is_type, token, success, true);
        if (!success) return false;
        if (is_type) {
            return token == closing_tag && !results->empty() &&
                   total_weight > 0 && total_weight <= INT_MAX;
        }

        const int weight = ScanTypeOrInt(is_type, token, success, true);
        if (!success || is_type || item_id <= 0 || weight <= 0 ||
            !item_ids.insert(item_id).second) {
            return false;
        }
        total_weight += weight;
        if (total_weight > INT_MAX) return false;
        results->push_back(WeightedResult(item_id, weight));
    }
}

bool ChooseWeighted(const std::vector<WeightedResult>& pool, int& item_id) {
    int total_weight = 0;
    for (std::vector<WeightedResult>::const_iterator it = pool.begin();
         it != pool.end(); ++it) {
        if (it->weight <= 0 || total_weight > INT_MAX - it->weight) {
            return false;
        }
        total_weight += it->weight;
    }
    if (total_weight <= 0) return false;

    unsigned long limit = static_cast<unsigned long>(total_weight - 1);
    int roll = static_cast<int>(Random().randInt(limit));
    for (std::vector<WeightedResult>::const_iterator it = pool.begin();
         it != pool.end(); ++it) {
        if (roll < it->weight) {
            item_id = it->item_id;
            return true;
        }
        roll -= it->weight;
    }
    return false;
}

}  // namespace

WeightedResult::WeightedResult() : item_id(0), weight(0) {}

WeightedResult::WeightedResult(int item_id_value, int weight_value)
    : item_id(item_id_value), weight(weight_value) {}

Config::Config()
    : rare_rate(0), normal_results(), rare_results() {
    egg_rates[0] = 0;
    egg_rates[1] = 0;
    egg_rates[2] = 0;
}

bool ImportCompoundCreatureScript(const char* filename) {
    if (!filename || !loadRDARScriptFile("", filename)) return false;

    Config parsed;
    int fields = 0;
    bool valid = true;
    std::string tag;
    while (valid && ScanType(tag, true) == 1) {
        if (tag == "[rare rate]") {
            if ((fields & 1) != 0 ||
                !ReadSingleInt("[/rare rate]", &parsed.rare_rate)) {
                valid = false;
            }
            fields |= 1;
        } else if (tag == "[egg rate]") {
            if ((fields & 2) != 0 || !ReadEggRates(parsed.egg_rates)) {
                valid = false;
            }
            fields |= 2;
        } else if (tag == "[Creature]") {
            if ((fields & 4) != 0 ||
                !ReadWeightedResults("[/Creature]",
                                     &parsed.normal_results)) {
                valid = false;
            }
            fields |= 4;
        } else if (tag == "[Rare Creature]") {
            if ((fields & 8) != 0 ||
                !ReadWeightedResults("[/Rare Creature]",
                                     &parsed.rare_results)) {
                valid = false;
            }
            fields |= 8;
        } else {
            valid = false;
        }
    }
    unloadRDARScriptFile();

    if (!valid || fields != 15 || parsed.normal_results.empty() ||
        parsed.rare_results.empty()) {
        return false;
    }
    PublishedConfig() = parsed;
    LoadedFlag() = true;
    return true;
}

bool IsLoaded() {
    return LoadedFlag();
}

bool SelectResult(int egg_count, int& item_id, bool& rare) {
    if (!LoadedFlag() || egg_count < 0 || egg_count > 2) return false;
    const Config& config = PublishedConfig();

    int threshold = config.rare_rate;
    if (egg_count > 0) threshold += config.egg_rates[egg_count - 1];
    if (threshold < 0) threshold = 0;
    if (threshold > 10000) threshold = 10000;

    RandomLock lock;
    const unsigned long gate_limit = 9999;
    rare = static_cast<int>(Random().randInt(gate_limit)) < threshold;
    const std::vector<WeightedResult>& pool =
        rare ? config.rare_results : config.normal_results;
    return ChooseWeighted(pool, item_id);
}

void RegisterEquipmentPrototype(int item_id, bool uncompoundable) {
    std::set<int>& item_ids = UncompoundablePrototypes();
    item_ids.erase(item_id);
    if (item_id > 0 && uncompoundable) item_ids.insert(item_id);
}

bool IsUncompoundable(int item_id) {
    return UncompoundablePrototypes().find(item_id) !=
           UncompoundablePrototypes().end();
}

}  // namespace compound_creature
