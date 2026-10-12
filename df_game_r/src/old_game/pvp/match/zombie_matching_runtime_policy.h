#ifndef OLDSRV_OLD_GAME_PVP_MATCH_ZOMBIE_MATCHING_RUNTIME_POLICY_H_
#define OLDSRV_OLD_GAME_PVP_MATCH_ZOMBIE_MATCHING_RUNTIME_POLICY_H_

#include <stddef.h>

namespace zombie_matching_runtime_policy {

enum {
    kOfficialMinimumMembers = 5,
    kOfficialMaximumMembers = 6,
    // Matching/search is exact-six.  The five-player ZombieMode.etc tuple is
    // still imported by zombie_mode_result for the later loading fallback;
    // it is not an authority to create a five-user search cohort.
    kOfficialMatchingMembers = 6,
};

inline bool IsRuntimeCohortSize(size_t value) {
    return value == static_cast<size_t>(kOfficialMatchingMembers);
}

}  // namespace zombie_matching_runtime_policy

#endif  // OLDSRV_OLD_GAME_PVP_MATCH_ZOMBIE_MATCHING_RUNTIME_POLICY_H_
