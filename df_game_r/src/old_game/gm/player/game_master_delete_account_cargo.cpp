#include "old_game/gm/player/game_master_delete_account_cargo.h"

#include "old_game/economy/inventory/inventory.h"

namespace game_master {

// IDA_EA: 0x82A84CE
// IDA_NAME: _ZN11game_master19CDeleteAccountCargoC2Ev
// DEMANGLED: game_master::CDeleteAccountCargo::CDeleteAccountCargo(void)
CDeleteAccountCargo::CDeleteAccountCargo() : CCommand() {}

// IDA_EA: 0x84ABDB4
// IDA_NAME: _ZN11game_master19CDeleteAccountCargo7executeEv
// DEMANGLED: game_master::CDeleteAccountCargo::execute(void)
void CDeleteAccountCargo::execute() {
    CAccountCargo::DeleteAccountCargo(GetUser());
}

}  // namespace game_master
