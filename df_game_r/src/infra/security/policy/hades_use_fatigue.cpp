#include "infra/security/policy/hades_public_abi.h"

#include <stdint.h>

namespace XNuclear {

// IDA_EA: 0x84B9552
// IDA_NAME: _ZN8XNuclear6CHades10UseFatigueEj
// DEMANGLED: XNuclear::CHades::UseFatigue(unsigned int)
CHades* CHades::UseFatigue(unsigned int /*fatigue*/) {
    needLogin();

    ++fatigue_counter_;
    CHades* result = reinterpret_cast<CHades*>(
        static_cast<uintptr_t>(fatigue_counter_));
    if (fatigue_counter_ > 1u) {
        StdCateUpdate(1u, 0u);
        result = this;
        fatigue_counter_ = 0u;
    }
    return result;
}

}  // namespace XNuclear
