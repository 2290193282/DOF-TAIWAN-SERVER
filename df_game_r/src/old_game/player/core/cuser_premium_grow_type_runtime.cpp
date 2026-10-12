#include "old_game/player/core/premium_runtime.h"

namespace WongWork {

// IDA_EA: 0x8609398
// IDA_NAME: _ZNK8WongWork12CUserPremium17getOverSkillLevelEv
// DEMANGLED: WongWork::CUserPremium::getOverSkillLevel(void)const
int CUserPremium::getOverSkillLevel() const {
    return static_cast<int>(over_skill_level_);
}

}  // namespace WongWork
