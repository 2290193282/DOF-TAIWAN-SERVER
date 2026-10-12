#ifndef OLDSRV_OLD_GAME_DUNGEON_INSTANCE_INSTANT_REINFORCE_H_
#define OLDSRV_OLD_GAME_DUNGEON_INSTANCE_INSTANT_REINFORCE_H_

#include <stdint.h>

#include <vector>

#include "old_data/PVF/etc/instant_reinforce.h"

class CParty;
class CUser;
class PacketBuf;
struct _Additioal_info;

namespace instant_reinforce {

// Copy-only combat projection.  No member is written into the character DB,
// the mastered skill tree, or CUser's persistent _Additioal_info.
struct AppliedSnapshot {
    AppliedSnapshot();

    int map_index;
    int select_index;
    int skill_header;
    uint64_t generation;
    std::vector<SkillGrant> skills;
    std::vector<StatusGrant> statuses;
    std::vector<int> effects;
};

// Shared lifecycle hook target.  The caller must pass the live flag captured
// before CParty::set_charac_live mutates it and the authoritative new state.
void ObserveAuthoritativeLifeTransition(CUser* user,
                                        CParty* party,
                                        int previous_live,
                                        int new_state);

void ResetUser(CUser* user);
void ResetParty(CParty* party);

// CMD555 is an empty sender-only notification.  Malformed payload returns 1;
// every well-formed business rejection is silent and returns 0 (no ACK).
int HandleRequest(CUser* user, PacketBuf& packet);

// These are the only server-visible buff consumers.  They project an active
// party/map rule onto caller-owned combat scratch data and never persist it.
bool CopyAppliedSnapshot(CUser* user, AppliedSnapshot* output);
int GetEffectiveSkillLevel(CUser* user, int skill_index, int base_level);
bool ApplyToStatusSnapshot(CUser* user, _Additioal_info* scratch);

}  // namespace instant_reinforce_0828

#endif  // OLDSRV_OLD_GAME_DUNGEON_INSTANCE_INSTANT_REINFORCE_0828_H_
