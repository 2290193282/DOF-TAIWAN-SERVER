#include "old_game/player/core/cuser.h"

// IDA_EA: 0x8686DA0
// IDA_NAME: _ZN15cUserHistoryLog27SelectCubeForPremiumServiceEi
// DEMANGLED: cUserHistoryLog::SelectCubeForPremiumService(int)
int cUserHistoryLog::SelectCubeForPremiumService(int premium_type) {
    if (!owner_) {
        return 0;
    }
    return owner_->LogHistory("SelectCubeForPremiumService,%d", premium_type);
}
