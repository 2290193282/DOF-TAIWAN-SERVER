#ifndef OLDSRV_OLD_DATA_PVF_ETC_POWER_PARAMETER_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_ETC_POWER_PARAMETER_SCRIPT_H_

#include <stddef.h>
#include <stdint.h>
#include <utility>
#include <vector>


// IDA_EA: 0x837187A, 0x8371A26, 0x837D634, 0x8A59C6E, 0x8A59D58, 0x8A59DB4, 0x8A592BC.
struct stVictoryPenalty {
    int32_t value_00;
    int32_t value_04;
    int32_t value_08;
    int32_t value_0c;
};

// IDA unit: 12_pvf_datamanager_scripts/PowerParameterScript.
class PowerParameterScript {
public:
    PowerParameterScript();
    ~PowerParameterScript();

    PowerParameterScript* clear();
    stVictoryPenalty GetPenalty(int32_t value);
    int32_t GetBreakBonus(int32_t index);
    int32_t checkPowerWarServer(int32_t server_group, int32_t channel);

    int32_t value_00;
    int32_t value_04;
    int32_t value_08;
    int32_t value_0c;
    int32_t value_10;
    // Power.etc [ghost revival base time].  CPowerManager multiplies this
    // base duration by the response penalty before entering ghost state.
    int32_t ghost_revival_base_time;            // +0x14
    // Power.etc keeps the original misspelled [respon penalty time] tag.
    // It is the upper bound used by GetPowerWarResponPenalty.
    int32_t respon_penalty_time;                 // +0x18
    int32_t value_1c;
    std::vector<int> values_20;
    int32_t value_2c;
    int32_t value_30;
    int32_t value_34;
    std::vector<int> values_38;
    int32_t value_44;
    int32_t value_48;
    int32_t value_4c;
    int32_t value_50;
    int32_t value_54;
    int32_t value_58;
    int32_t value_5c;
    // Power.etc tag: [time assault super power state after powerwar].
    // IDA consumers read CDataManager +0xA688 (this +0x60).
    int32_t assault_super_state_after_powerwar_duration_seconds;
    int32_t value_64;
    int32_t value_68;
    int32_t value_6c;
    std::vector<std::pair<int, int> > values_70;
    int32_t value_7c;
    int32_t value_80;
    int32_t value_84;
    int32_t value_88;
    int32_t value_8c;
    int32_t value_90;
    std::pair<int, int> value_94;
    std::vector<std::pair<int, int> > power_war_servers;
    int32_t value_a8;
    std::vector<int> values_ac;
    int32_t value_b8;
    std::vector<stVictoryPenalty> victory_penalties;
    std::vector<int> break_bonus;
    int32_t victorious_reward_time_minutes;
};

int importPowerParameterScript(PowerParameterScript* script,
                               const char* filename);

static_assert(sizeof(stVictoryPenalty) == 0x10u,
              "stVictoryPenalty IDA ABI size must stay 0x10");
static_assert(sizeof(PowerParameterScript) == 0xd8u,
              "PowerParameterScript IDA ABI size must stay 0xd8");
static_assert(offsetof(PowerParameterScript, values_20) == 0x20u &&
                  offsetof(PowerParameterScript, values_38) == 0x38u &&
                  offsetof(PowerParameterScript, values_70) == 0x70u &&
                  offsetof(PowerParameterScript, power_war_servers) == 0x9cu &&
                  offsetof(PowerParameterScript, victory_penalties) == 0xbcu &&
                  offsetof(PowerParameterScript, break_bonus) == 0xc8u &&
                  offsetof(PowerParameterScript,
                           victorious_reward_time_minutes) == 0xd4u,
              "PowerParameterScript IDA container offsets must stay exact");
static_assert(offsetof(PowerParameterScript, ghost_revival_base_time) ==
                      0x14u &&
                  offsetof(PowerParameterScript, respon_penalty_time) ==
                      0x18u,
              "PowerParameterScript revival-policy offsets must stay exact");

#endif  // OLDSRV_OLD_DATA_PVF_ETC_POWER_PARAMETER_SCRIPT_H_
