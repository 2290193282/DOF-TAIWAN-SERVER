#include "old_game/gm/item/game_master_get_cash_inven_command.h"

#include <cstdlib>

#include "common/manager/global_tools.h"

namespace game_master {

// IDA_EA: 0x82A85CA
// IDA_NAME: _ZN11game_master13CGetCashInvenC2Ev
// DEMANGLED: game_master::CGetCashInven::CGetCashInven(void)
CGetCashInven::CGetCashInven() : CCommand() {}

// IDA_EA: 0x84AC80A
// IDA_NAME: _ZN11game_master13CGetCashInven5parseEv
// DEMANGLED: game_master::CGetCashInven::parse(void)
void CGetCashInven::parse() {
    std::string& command_tail = parser_->command_tail();
    trim(command_tail);
    item_space_ = static_cast<int16_t>(std::atoi(command_tail.c_str()));
}

// IDA_EA: 0x84AC846
// IDA_NAME: _ZN11game_master13CGetCashInven7executeEv
// DEMANGLED: game_master::CGetCashInven::execute(void)
void CGetCashInven::execute() {
    // The original body is exactly `push ebp; mov ebp, esp; pop ebp; ret`.
}

}  // namespace game_master
