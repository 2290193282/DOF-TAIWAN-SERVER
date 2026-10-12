#include "old_game/player/core/premium_runtime.h"

namespace WongWork {

// IDA_EA: 0x8694A64
// IDA_NAME: _ZN8WongWork12CUserPremium12GetGoldBonusEi
// DEMANGLED: WongWork::CUserPremium::GetGoldBonus(int)
int CUserPremium::GetGoldBonus(int charac_level) {
    return gold_bonus_ ? 100 * charac_level * charac_level : 0;
}

}  // namespace WongWork
