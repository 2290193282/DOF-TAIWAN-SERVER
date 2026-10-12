#ifndef OLDSRV_OLD_GAME_APPEARANCE_PET_CREATURE_FILLPOINT_RUNTIME_H_
#define OLDSRV_OLD_GAME_APPEARANCE_PET_CREATURE_FILLPOINT_RUNTIME_H_

#include <stdint.h>

#include <vector>

class CUser;
struct CreatureScript;

namespace CreatureFillPoint {

// Server-owned projection of a general creature skill.  The client sends
// only fill_point, so the formal owner keeps the skill id for evidence and
// rejects ambiguous/missing script records instead of trusting that value.
struct SkillConfig {
    SkillConfig();

    int skill_id;
    int fill_point;
    int recovery_time_ms;
    int learn_skill_level;
    // 0828 CreatureScriptSkillInfo::[preload Passive Index].  The client
    // treats these as passive-resource dependencies of this exact skill
    // node.  Preserve the ordered list in the runtime projection; do not
    // reinterpret the indices as active skills on the server.
    std::vector<int> preload_passive_indices;
};

// CCreatureScriptMgr stages the complete Creature.lst projection and publishes
// it only after every .cre succeeds.  Standalone registration still replaces
// one published item atomically.  This sidecar deliberately does not consume
// restored CCreatureScript ABI storage.
void BeginScriptCacheLoad();
void CommitScriptCacheLoad();
void AbortScriptCacheLoad();
void ResetScriptCache();
bool RegisterCreatureScript(int item_id, const CreatureScript& script);
bool ResolveGeneralSkill(int item_id,
                         uint32_t requested_fill_point,
                         int creature_level,
                         SkillConfig& config);
// Exposes the ordered passive-resource dependency list for both general and
// over-skill nodes.  Empty/absent lists are deliberately indistinguishable,
// matching the default-empty vector stored by the 0828 client importer.
bool ResolveSkillPassivePreloads(int item_id,
                                 int skill_id,
                                 bool over_skill,
                                 std::vector<int>& passive_indices);

// CMD549 has no skill id or active-creature uid on the wire.  A pending
// reservation prevents duplicate dispatch on the same user while keeping the
// committed timestamp separate until the stomach mutation is accepted.
bool BeginCooldown(CUser* user,
                   int charac_no,
                   int creature_uid,
                   int recovery_time_ms,
                   uint32_t now_tick);
void CommitCooldown(CUser* user,
                    int charac_no,
                    int creature_uid,
                    uint32_t now_tick);
void CancelCooldown(CUser* user,
                    int charac_no,
                    int creature_uid,
                    uint32_t now_tick);
void ClearCooldown(CUser* user);

}  // namespace CreatureFillPoint

#endif  // OLDSRV_OLD_GAME_APPEARANCE_PET_CREATURE_FILLPOINT_RUNTIME_H_
