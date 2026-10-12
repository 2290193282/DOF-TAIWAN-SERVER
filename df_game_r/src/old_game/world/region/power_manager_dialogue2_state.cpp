#include "old_game/world/region/power_manager_dialogue2_state.h"

#include "old_game/pvp/battlefield/power_war_runtime.h"

// The two IDA identities once staged here are now canonical CPowerManager
// member definitions in power_manager_runtime.cpp.  Keep this non-manifest
// evidence unit free of a second provider: the real member bodies access
// power_a_/power_b_ and power_war_start_* directly.
#if defined(__i386__)
static_assert(sizeof(CPowerManager) == 0x208u,
              "CPowerManager canonical i386 layout drifted");
#endif
