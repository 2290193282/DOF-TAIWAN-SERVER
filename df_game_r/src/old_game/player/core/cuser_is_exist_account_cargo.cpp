#include "old_game/player/core/cuser.h"

#include "old_game/economy/inventory/inventory.h"

// IDA_EA: 0x822FC30
// IDA_NAME: _ZN5CUser19IsExistAccountCargoEv
// DEMANGLED: CUser::IsExistAccountCargo(void)
bool CUser::IsExistAccountCargo() {
    return account_cargo_.GetCapacity() != 0;
}
