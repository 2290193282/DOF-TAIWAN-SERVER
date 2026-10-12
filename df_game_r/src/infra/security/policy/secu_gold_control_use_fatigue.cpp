#include "infra/security/policy/secu_gold_control_abi.h"

// IDA_EA: 0x8287552
// IDA_NAME: _ZN16Secu_GoldControl10UseFatigueEi
// DEMANGLED: Secu_GoldControl::UseFatigue(int)
Secu_GoldControl* Secu_GoldControl::UseFatigue(int fatigue) {
    this->CheckDate();
    used_fatigue_ += static_cast<uint32_t>(fatigue);
    return this;
}
