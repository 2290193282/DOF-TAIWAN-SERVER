#include "old_game/gm/dungeon/game_master_pcroom_premium_cmd.h"

#include <cstdlib>

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_game/player/core/premium_runtime.h"
#include "common/manager/system_time.h"

namespace game_master {

// IDA_EA: 0x82A822E
// IDA_NAME: _ZN11game_master17CPcroomPremiumCmdC2Ev
// DEMANGLED: game_master::CPcroomPremiumCmd::CPcroomPremiumCmd(void)
CPcroomPremiumCmd::CPcroomPremiumCmd() : CCommand() {}

// IDA_EA: 0x84A9DB0
// IDA_NAME: _ZN11game_master17CPcroomPremiumCmd5parseEv
// DEMANGLED: game_master::CPcroomPremiumCmd::parse(void)
void CPcroomPremiumCmd::parse() {
    std::string& command_tail = parser_->command_tail();
    trim(command_tail);
    premium_duration_second_ = std::atoi(command_tail.c_str());
}

// IDA_EA: 0x84A9DE8
// IDA_NAME: _ZN11game_master17CPcroomPremiumCmd7executeEv
// DEMANGLED: game_master::CPcroomPremiumCmd::execute(void)
void CPcroomPremiumCmd::execute() {
    const long current_second =
        static_cast<long>(GlobalData::s_systemTime_.getCurSec());
    WongWork::CHandlePremium::handleSetUserPCRoom(
        GetUser(), current_second, current_second + premium_duration_second_);
}

}  // namespace game_master
