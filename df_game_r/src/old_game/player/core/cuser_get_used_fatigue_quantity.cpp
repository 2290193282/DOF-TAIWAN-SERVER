#include "old_game/player/core/cuser.h"


// IDA_EA: 0x86975E0
// IDA_NAME: _ZN5CUser31GetCurCharacUsedFatigueQuantityEv
// DEMANGLED: CUser::GetCurCharacUsedFatigueQuantity(void)
// Active reconstructed-tree ABI: _ZN5CUser31GetCurCharacUsedFatigueQuantityEv.
int CUser::GetCurCharacUsedFatigueQuantity() {
    return static_cast<int>(used_fatigue_);
}
