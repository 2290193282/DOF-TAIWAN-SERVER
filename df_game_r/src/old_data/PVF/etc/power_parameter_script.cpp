#include "old_data/PVF/etc/power_parameter_script.h"

#include "old_data/PVF/common/rdar_script.h"

#include <string>


typedef char stVictoryPenalty_Size_Must_Be_0x10[
    (sizeof(stVictoryPenalty) == 0x10) ? 1 : -1];
typedef char PowerParameterScript_Size_Must_Be_0xD8[
    (sizeof(PowerParameterScript) == 0xD8) ? 1 : -1];

// IDA_EA: 0x837187A
// IDA_NAME: _ZN20PowerParameterScript5clearEv
// DEMANGLED: PowerParameterScript::clear(void)
PowerParameterScript* PowerParameterScript::clear() {
    value_00 = 0;
    value_04 = 0;
    value_08 = 0;
    value_0c = 0;
    value_10 = 0;
    ghost_revival_base_time = 0;
    value_1c = 0;
    value_30 = 0;
    value_34 = 0;
    value_48 = 0;
    value_4c = 0;
    value_50 = 0;
    value_54 = 0;
    values_20.clear();
    value_58 = 0;
    value_5c = 5;
    assault_super_state_after_powerwar_duration_seconds = 5;
    values_70.clear();
    value_7c = 0;
    value_80 = 0;
    value_84 = 0;
    value_88 = 0;
    value_8c = 0;
    value_90 = 0;
    value_94.first = 0;
    value_94.second = 0;
    values_38.clear();
    value_44 = 0;
    value_64 = 0;
    value_68 = 0;
    value_a8 = 0;
    values_ac.clear();
    value_b8 = 0;
    victory_penalties.clear();
    break_bonus.clear();
    victorious_reward_time_minutes = 0;
    return this;
}

// IDA_EA: 0x8371A26
// IDA_NAME: _ZN20PowerParameterScriptC2Ev
// DEMANGLED: PowerParameterScript::PowerParameterScript(void)
PowerParameterScript::PowerParameterScript() {
    clear();
}

// IDA_EA: 0x837D634
// IDA_NAME: _ZN20PowerParameterScriptD2Ev
// DEMANGLED: PowerParameterScript::~PowerParameterScript()
PowerParameterScript::~PowerParameterScript() {}

// IDA_EA: 0x8A59C6E
// IDA_NAME: _ZN20PowerParameterScript10GetPenaltyEi
// DEMANGLED: PowerParameterScript::GetPenalty(int)
stVictoryPenalty PowerParameterScript::GetPenalty(int32_t value) {
    for (std::vector<stVictoryPenalty>::const_iterator it =
             victory_penalties.begin();
         it != victory_penalties.end();
         ++it) {
        if (it->value_04 >= value) {
            return *it;
        }
    }
    return victory_penalties[victory_penalties.size() - 1];
}

// IDA_EA: 0x8A59D58
// IDA_NAME: _ZN20PowerParameterScript13GetBreakBonusEi
// DEMANGLED: PowerParameterScript::GetBreakBonus(int)
int32_t PowerParameterScript::GetBreakBonus(int32_t index) {
    int32_t size = static_cast<int32_t>(break_bonus.size());
    if (index < size) {
        return break_bonus[index];
    }
    return break_bonus[size - 1];
}

// IDA_EA: 0x8A59DB4
// IDA_NAME: _ZN20PowerParameterScript19checkPowerWarServerEii
// DEMANGLED: PowerParameterScript::checkPowerWarServer(int,int)
int32_t PowerParameterScript::checkPowerWarServer(
    int32_t server_group,
    int32_t channel) {
    for (std::vector<std::pair<int, int> >::const_iterator it =
             power_war_servers.begin();
         it != power_war_servers.end();
         ++it) {
        if (it->first == server_group && it->second == channel) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8A592BC
// IDA_NAME: _Z26importPowerParameterScriptP20PowerParameterScriptPKc
// DEMANGLED: importPowerParameterScript(PowerParameterScript *,char const*)
int importPowerParameterScript(PowerParameterScript* script,
                               const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    PowerParameterScript* target = script;
    target->clear();

    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[power join cost]") {
            target->value_00 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[power join guild level]") {
            target->value_04 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[power join quest]") {
            target->value_08 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[kill point to end power war]") {
            target->value_0c = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[exp reward]") {
            target->value_10 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[easy assault range]") {
            target->value_2c = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[ghost revival base time]") {
            target->ghost_revival_base_time =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[respon penalty time]") {
            target->respon_penalty_time = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[powerwar point multiple number]") {
            target->value_54 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[refresh power war process info]") {
            target->value_30 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[abuser ban time]") {
            target->value_34 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[power up ratio]") {
            target->value_1c = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[power up assault count]") {
            target->value_4c = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[power up assault victory ratio]") {
            target->value_50 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[power up generate period]") {
            target->value_48 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[power up noti time]") {
            while (1) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                target->values_20.push_back(value);
            }
        } else if (tag == "[straight victories]") {
            target->value_58 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[item initial cooltime]") {
            target->value_6c = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[kill count effect range]") {
            while (1) {
                std::pair<int, int> value;
                value.first = ScanInt(&success);
                if (!success) {
                    break;
                }
                value.second = ScanInt(static_cast<bool*>(0));
                target->values_70.push_back(value);
            }
        } else if (tag == "[basic powerwar point]") {
            target->value_7c = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[first grade bonus powerwar point]") {
            target->value_80 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[ranking bonus grade]") {
            target->value_84 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[ranking bonus decrease point]") {
            target->value_88 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[decrease powerwar point per day]") {
            target->value_8c = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[required user powerwar point 4 dungeon]") {
            target->value_90 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[power war dungeon info]") {
            target->value_94.first = ScanInt(static_cast<bool*>(0));
            target->value_94.second = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[power up bonus]") {
            while (1) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                target->values_38.push_back(value);
            }
        } else if (tag == "[pc room bonus]") {
            int value = 0;
            value = ScanInt(&success);
            target->value_44 = value;
        } else if (tag ==
                   "[time assault super power state on join powerwar]") {
            target->value_5c = ScanInt(static_cast<bool*>(0));
        } else if (tag ==
                   "[time assault super power state after powerwar]") {
            target->assault_super_state_after_powerwar_duration_seconds =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[powerwar enter level limit]") {
            target->value_64 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[powerwar enter age limit]") {
            target->value_68 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[power war channel]") {
            while (1) {
                const int server_group = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int channel = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                target->power_war_servers.push_back(
                    std::make_pair(server_group, channel));
            }
        } else if (tag == "[power up point mod]") {
            target->value_a8 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[power join cost modify]") {
            while (1) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                target->values_ac.push_back(value);
            }
        } else if (tag == "[victorious reward time]") {
            target->victorious_reward_time_minutes =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[straight victory penalty]") {
            while (1) {
                stVictoryPenalty value;
                value.value_00 = ScanInt(&success);
                if (!success) {
                    break;
                }
                value.value_04 = ScanInt(&success);
                if (!success) {
                    break;
                }
                value.value_08 = ScanInt(&success);
                if (!success) {
                    break;
                }
                value.value_0c = ScanInt(&success);
                if (!success) {
                    break;
                }
                target->victory_penalties.push_back(value);
            }
        } else if (tag == "[straight victory break bonus]") {
            while (1) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                target->break_bonus.push_back(value);
            }
        } else if (tag == "[power rejoin panalty]") {
            target->value_b8 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[power parameter end]") {
            success = true;
            break;
        }
    }
    return success ? 1 : 0;
}
