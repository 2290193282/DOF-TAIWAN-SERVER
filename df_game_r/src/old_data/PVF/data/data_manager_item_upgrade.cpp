#include "old_data/PVF/data/data_manager.h"

#include "old_data/PVF/equipment/item_upgrade.h"

// IDA_EA: 0x83635D4
// IDA_NAME: _ZN12CDataManager28getUpgradeItemRepairCostRateEib
// DEMANGLED: CDataManager::getUpgradeItemRepairCostRate(int,bool)
float CDataManager::getUpgradeItemRepairCostRate(int upgrade,
                                                 bool has_ability) {
    return item_upgrade_->getUpgradeItemRepairCostRate(upgrade, has_ability);
}

WongWork::CItemUpgrade* CDataManager::get_item_upgrade() {
    return item_upgrade_;
}

WongWork::CItemUpgrade_Separate* CDataManager::get_item_upgrade_separate() {
    return item_upgrade_sep_;
}
