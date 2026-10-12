#include "old_game/player/core/premium_runtime.h"

namespace WongWork {

// IDA_EA: 0x8694A54
// IDA_NAME: _ZNK8WongWork12CUserPremium24isAffectedUnlimitFatigueEv
// DEMANGLED: WongWork::CUserPremium::isAffectedUnlimitFatigue(void)const
int CUserPremium::isAffectedUnlimitFatigue() const {
    return static_cast<unsigned char>(unlimited_fatigue_);
}

}  // namespace WongWork
