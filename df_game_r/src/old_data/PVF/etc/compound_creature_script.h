#ifndef OLDSRV_OLD_DATA_PVF_ETC_COMPOUND_CREATURE_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_ETC_COMPOUND_CREATURE_SCRIPT_H_

#include <stdint.h>

#include <vector>


namespace compound_creature {

struct WeightedResult {
    WeightedResult();
    WeightedResult(int item_id_value, int weight_value);

    int32_t item_id;
    int32_t weight;
};

struct Config {
    Config();

    int32_t rare_rate;
    int32_t egg_rates[3];
    std::vector<WeightedResult> normal_results;
    std::vector<WeightedResult> rare_results;
};

// Loads Etc/compoundcreaturejpn.etc into an immutable typed process-side
// table.  A failed reload leaves the previous valid configuration untouched.
bool ImportCompoundCreatureScript(const char* filename);
bool IsLoaded();

// RECONSTRUCTED_RANDOM: the client proves only the two-stage result model and
// the PVF rates/weights.  The server combines rare_rate with the egg-count
// bonus on a 10,000-point gate, then selects by weight inside that pool.
bool SelectResult(int egg_count, int& item_id, bool& rare);

// `[Uncompoundable]` belongs to STEquipmentScript in 0828.  Keeping this bit
// in a sidecar preserves the recovered 0627 CEquipItem binary layout.
void RegisterEquipmentPrototype(int item_id, bool uncompoundable);
bool IsUncompoundable(int item_id);

}  // namespace compound_creature

#endif  // OLDSRV_OLD_DATA_PVF_ETC_COMPOUND_CREATURE_SCRIPT_H_
