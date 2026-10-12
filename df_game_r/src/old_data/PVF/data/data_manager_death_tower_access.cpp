#include "old_data/PVF/data/data_manager.h"

// B-owned typed reconstruction boundary, not an old exported member ABI.
// IDA_EA: CDataManager::C2 @0x8355B30, instruction @0x8355CF0.
// IDA_NAME: _ZN12CDataManagerC2Ev.  It constructs stDeathTower_t at +0x5198,
// immediately after the +0x5180 tower-index map and immediately before the
// +0x5E30 stBloodDungeon_t object.
const stDeathTower_t& CDataManager::get_death_tower() const {
    return death_tower_;
}

// Typed B-owned boundary for CDeathTower::_beginTowerClearProcess @0x846791F.
// The old body indexes stDeathTower_t::reward_card_num with the current
// stage without an independent bounds check.
int CDataManager::get_death_tower_reward_card_num(unsigned int stage) const {
    return death_tower_.reward_card_num[stage];
}

// Typed B-owned boundary for CDeathTower::_beginTowerClearProcess @0x8467936.
// This is the adjacent illusion tower table, with the same original stage
// precondition as the normal reward-card path.
int CDataManager::get_death_tower_illusion_reward_card_num(
    unsigned int stage) const {
    return death_tower_.illusion_reward_card_num[stage];
}
