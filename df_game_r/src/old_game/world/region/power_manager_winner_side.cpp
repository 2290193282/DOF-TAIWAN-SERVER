#include "old_game/pvp/battlefield/power_war_runtime.h"

// IDA_EA: 0x837501A
// IDA_NAME: _ZN13CPowerManager13GetWinnerSideEv
// DEMANGLED: CPowerManager::GetWinnerSide(void)
// CPowerManager::C2 initializes this byte and SetPowerInfo updates it; no
// independent B-owned state is introduced.
int CPowerManager::GetWinnerSide() {
    return static_cast<int>(winner_side_);
}
