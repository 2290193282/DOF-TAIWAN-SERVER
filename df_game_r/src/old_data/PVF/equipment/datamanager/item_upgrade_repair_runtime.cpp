#include "common/manager/find_price_abi.h"

// The CItemUpgrade and CDataManager providers moved to the B-owned canonical
// PVF sources.  This legacy unit retains only the unrelated global price helper
// until its gameplay owner migrates it.

// IDA_EA: 0x898C8FC
// IDA_NAME: _Z23getEquipmentRepairPriceiiiibf
// DEMANGLED: getEquipmentRepairPrice(int,int,int,int,bool,float)
int getEquipmentRepairPrice(int growth_repair_cost,
                            int current_endurance,
                            int maximum_endurance,
                            int growth_grade,
                            bool special_repair,
                            float upgrade_repair_rate)
    __asm__("_Z23getEquipmentRepairPriceiiiibf");
int getEquipmentRepairPrice(int growth_repair_cost,
                            int current_endurance,
                            int maximum_endurance,
                            int growth_grade,
                            bool special_repair,
                            float upgrade_repair_rate) {
    (void)special_repair;
    if (maximum_endurance == 0 || maximum_endurance == current_endurance) {
        return 0;
    }

    const float price =
        static_cast<float>(growth_repair_cost * (growth_grade + 5) / 10) *
        itemRepairCost_ / static_cast<float>(maximum_endurance) *
        static_cast<float>(maximum_endurance - current_endurance);
    return static_cast<int>(price * upgrade_repair_rate);
}
