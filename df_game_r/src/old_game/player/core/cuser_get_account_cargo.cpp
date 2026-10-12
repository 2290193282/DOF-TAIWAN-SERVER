#include "old_game/player/core/cuser.h"


// IDA_EA: 0x822FC22
// IDA_NAME: _ZN5CUser15GetAccountCargoEv
// DEMANGLED: CUser::GetAccountCargo(void)
CAccountCargo* CUser::GetAccountCargo() {
    return &account_cargo_;
}
