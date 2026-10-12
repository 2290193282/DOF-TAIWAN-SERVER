#include "old_game/economy/inventory/inventory.h"

#include <map>
#include <utility>

typedef std::map<ItemUpgradeType::T, std::pair<int, int> >
    UpgradeLevelRangeMap;

// IDA_EA: 0x85136EC
// IDA_NAME: _ZN10CEquipItem17isUpgradableLevelEN15ItemUpgradeType1TEi
// DEMANGLED: CEquipItem::isUpgradableLevel(ItemUpgradeType::T,int)
bool CEquipItem::isUpgradableLevel(ItemUpgradeType::T upgrade_type,
                                   int charac_level) {
    UpgradeLevelRangeMap& ranges = upgrade_level_limits_;
    UpgradeLevelRangeMap::iterator found = ranges.find(upgrade_type);
    return found == ranges.end() ||
           (found->second.first <= charac_level &&
            charac_level <= found->second.second);
}
