#ifndef OLDSRV_OLD_DATA_PVF_ETC_CARGO_TRANSPORT_STONE_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_ETC_CARGO_TRANSPORT_STONE_SCRIPT_H_

#include <stdint.h>

#include <map>
#include <set>


class CEquipItem;

namespace cargo_transport {

// 0828 Etc/CargoTransportStone.etc.  This is process-wide immutable PVF
// configuration after CDataManager::init(); it is deliberately not stored in
// either the character DB or the account-cargo DB.
struct Rule {
    Rule();

    int32_t stone_type;
    std::map<int, int> grade_brackets;
    std::map<int, int> rarity_weights;
    std::set<int> enabled_equipment_types;
    std::set<int> excluded_item_ids;
    std::set<int> included_item_ids;
    bool excludes_growth_type;
    int32_t ui_index;
};

// Loads and validates the complete type 0/1/2 rule table.  A temporary table
// is parsed first, so a failed reload never publishes a partial configuration.
bool ImportCargoTransportStoneScript(const char* filename);
bool IsLoaded();
const Rule* FindRule(int stone_type);

// Returns the required material count or -1 when the equipment is not
// transportable under this rule.  The calculation follows client
// sub_15EDBC0/sub_15EDDD0: exact ten-level bracket, rarity multiplier,
// equipment type, except-index/growth and final include-index override.
int GetRequiredMaterialCount(const Rule& rule,
                             const CEquipItem& equipment,
                             int item_id);

// The 0828 client treats either a non-empty `[expand path]` or two positive
// `[equipment grow]` bytes as growth equipment for CargoTransportStone's
// `growthType` exclusion.  Adding those identities to the fixed 0627
// CEquipItem ABI would move every following member, so the prototype importer
// records the exact result in this process-local sidecar.
void RegisterEquipmentGrowthPrototype(int item_id,
                                      uint8_t growth_type,
                                      uint8_t growth_stage,
                                      bool has_expand_path);
bool IsEquipmentGrowthPrototype(int item_id);

}  // namespace cargo_transport

#endif  // OLDSRV_OLD_DATA_PVF_ETC_CARGO_TRANSPORT_STONE_SCRIPT_H_
