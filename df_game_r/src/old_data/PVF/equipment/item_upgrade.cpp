#include "old_data/PVF/equipment/item_upgrade.h"

#include <time.h>

#include "old_game/economy/equipment/item_amplifier_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/mt_rand.h"

// A-owned GlobalData storage, defined under the old data symbol.  B consumes
// it here but does not duplicate its provider.

namespace WongWork {

// IDA_EA: 0x85461C8
// IDA_NAME: _ZN8WongWork12CItemUpgradeC2Ev
// DEMANGLED: WongWork::CItemUpgrade::CItemUpgrade(void)
CItemUpgrade::CItemUpgrade()
    : CItemUpgradeTable(),
      IItemUpgrade(),
      random_limit_(100000u) {
    const unsigned long seed = static_cast<unsigned long>(::time(0));
    random_ = new CMTRand(seed);
    for (int index = 0; index <= 99; ++index) {
        random_->randInt(random_limit_);
    }
}

// IDA_EA: 0x85462C6
// IDA_NAME: _ZN8WongWork12CItemUpgradeD2Ev
// DEMANGLED: WongWork::CItemUpgrade::~CItemUpgrade()
CItemUpgrade::~CItemUpgrade() {
    delete random_;
    random_ = 0;
}

// IDA_EA: 0x8549148
// IDA_NAME: _ZNK8WongWork12CItemUpgrade28getUpgradeItemRepairCostRateEib
// DEMANGLED: WongWork::CItemUpgrade::getUpgradeItemRepairCostRate(int,bool)const
float CItemUpgrade::getUpgradeItemRepairCostRate(int upgrade,
                                                 bool has_ability) const {
    const upgrade_table_t* table = has_ability
        ? GlobalData::s_itemAmplifier_->getUpgradeInfoTable()
        : CItemUpgradeTable::getUpgradeInfoTable();
    return table->repair_cost_rates[upgrade];
}

// IDA_EA: 0x854919A
// IDA_NAME: _ZN8WongWork12CItemUpgrade17LoadUpgradeScriptEPKc
// DEMANGLED: WongWork::CItemUpgrade::LoadUpgradeScript(char const*)
int CItemUpgrade::LoadUpgradeScript(const char* filename) {
    const int result = CItemUpgradeTable::loadUpgradeScript(filename);
    return result == 1 && !upgrade_infos.empty() ? 1 : 0;
}

// IDA_EA: 0x85491B8
// IDA_NAME: _ZN8WongWork12CItemUpgrade29Load3rdChronicleUpgradeScriptEPKc
// DEMANGLED: WongWork::CItemUpgrade::Load3rdChronicleUpgradeScript(char const*)
int CItemUpgrade::Load3rdChronicleUpgradeScript(const char* filename) {
    if (upgrade_infos.empty()) {
        return 0;
    }

    const int result = CItemUpgradeTable::loadUpgradeScript(filename);
    return result == 1 && !upgrade_infos.empty() ? 1 : 0;
}

}  // namespace WongWork
