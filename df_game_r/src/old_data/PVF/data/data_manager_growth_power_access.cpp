#include "old_data/PVF/data/data_manager.h"

#include "old_game/activity/reward/growth_booster_abi.h"
#include "old_data/PVF/event/seria_blessing_script.h"

// CDataManager::C2 @0x8355B30 constructs stSeriaBlessingScript at +0x5094
// and STGrowthPowerData at +0x8710; CDataManager::D2 @0x8356B68 destroys
// precisely the same two objects in reverse order.  These are B-owned
// internal accessors, not additional old-server ABI entry points.
stSeriaBlessingScript* CDataManager::get_seria_blessing_script() {
    return &seria_blessing_script_;
}

STGrowthPowerData* CDataManager::get_growth_power_data() {
    return &growth_power_data_;
}

const FairPvpEquipmentMap& CDataManager::get_fair_pvp_equipment_map() const {
    return fair_pvp_equipment_;
}
