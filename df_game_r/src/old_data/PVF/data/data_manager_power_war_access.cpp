#include "old_data/PVF/data/data_manager.h"

#include <stddef.h>

#include "old_data/PVF/etc/power_parameter_script.h"

// CDataManager::C2/D2 and its Power.etc importer construct/destroy the exact
// PowerParameterScript embedded at +0xa628.  These B-owned named bridges keep
// PowerWar consumers out of CDataManager's opaque remainder storage.
// IDA layout evidence: CDataManager::init @0x8358332, PowerParameterAt in
// data_manager_init.cpp, importPowerParameterScript @0x8A592BC.
PowerParameterScript* CDataManager::get_power_war_parameter_script() {
    return &power_parameter_script_;
}

const PowerParameterScript* CDataManager::get_power_war_parameter_script() const {
    return &power_parameter_script_;
}

int CDataManager::get_power_war_point_multiplier() const {
    return get_power_war_parameter_script()->value_54;
}

int CDataManager::get_power_war_straight_victories() const {
    return get_power_war_parameter_script()->value_58;
}

int CDataManager::get_power_war_victorious_reward_minutes() const {
    return get_power_war_parameter_script()->victorious_reward_time_minutes;
}

int CDataManager::get_power_war_basic_point() const {
    return get_power_war_parameter_script()->value_7c;
}

int CDataManager::
get_power_war_assault_superstate_after_duration_seconds() const {
    return get_power_war_parameter_script()
        ->assault_super_state_after_powerwar_duration_seconds;
}

stVictoryPenalty CDataManager::get_power_war_victory_penalty(
    int party_victory) const {
    return const_cast<PowerParameterScript*>(
        get_power_war_parameter_script())->GetPenalty(party_victory);
}

int CDataManager::get_power_war_break_bonus(int party_victory) const {
    return const_cast<PowerParameterScript*>(
        get_power_war_parameter_script())->GetBreakBonus(party_victory);
}

static_assert(
    offsetof(PowerParameterScript, value_54) == 0x54u &&
        offsetof(PowerParameterScript, value_58) == 0x58u &&
        offsetof(PowerParameterScript, value_7c) == 0x7cu &&
        offsetof(PowerParameterScript, victorious_reward_time_minutes) ==
            0xd4u &&
        offsetof(PowerParameterScript,
                 assault_super_state_after_powerwar_duration_seconds) ==
            0x60u,
    "Power.etc PvP access offsets changed");
