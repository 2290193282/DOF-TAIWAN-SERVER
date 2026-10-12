#include "old_game/progression/quest/quest_runtime.h"

// IDA_DATA: `g_alchemistInfo` is the first dword of the two-value alchemist
// parameter block imported from ExpertJob.etc.
extern "C" int g_alchemistInfo;

// IDA_EA: 0x85D1360
// IDA_NAME: _ZN10expert_job10CAlchemist25GetItemUpgradeSuccessRateEv
// DEMANGLED: expert_job::CAlchemist::GetItemUpgradeSuccessRate(void)
long double expert_job::CAlchemist::GetItemUpgradeSuccessRate() {
    return static_cast<long double>(g_alchemistInfo) / 10000.0L;
}
