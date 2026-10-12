#include "old_game/progression/quest/quest_runtime.h"

#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"

// IDA_EA: 0x8352CC2
// IDA_NAME: _ZN5Quest16check_power_sideEc
// DEMANGLED: Quest::check_power_side(char)
int Quest::check_power_side(char power_side) {
    if (belong_to_winning_power == 0xFFu) {
        return 1;
    }

    const int winner_side = GlobalData::s_power_manager->GetWinnerSide();
    if (belong_to_winning_power != 0) {
        if (belong_to_winning_power != 1u) {
            LogManager::logFormat(1, "data_manager.cpp",
                                  "bool Quest::check_power_side(char)",
                                  0x1161, "Quest Script Error q_index(%d)",
                                  quest_index);
            return 0;
        }
        if (GlobalData::s_power_manager->GetWinnerSide() != power_side) {
            return 1;
        }
    } else if (winner_side == power_side) {
        return 1;
    }
    return 0;
}
