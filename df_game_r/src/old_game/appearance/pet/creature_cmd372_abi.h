#ifndef OLDSRV_OLD_GAME_CREATURE_AVATAR_CREATURE_CMD372_ABI_H_
#define OLDSRV_OLD_GAME_CREATURE_AVATAR_CREATURE_CMD372_ABI_H_

// C-owned CMD372 prerequisite ABI.  The PVF parser still constructs this
// object, but the recovery-time fields and their direct gameplay consumers are
// one C creature-avatar boundary.

#include <stddef.h>
#include <stdint.h>

#include <set>
#include <vector>

struct CreatureScript;

namespace user_creature {

class CCreatureScript {
public:
    CCreatureScript();
    ~CCreatureScript();
    CCreatureScript* Set(CreatureScript* script);
    int GetItemEvolutionCreatureId(int creature_id);
    int IsAvailableArtifactSlot(int slot_type);
    CCreatureScript* SetItemId(int item_id);
    int GetItemId();
    bool IsExistEvolutionQuest();
    int GetEvolutionCreatureId();
    int validEventEvlouteCreature(int creature_id);
    int isEvoluationCreature(int creature_id);
    int GetEvolutionLevel();
    int GetMaxLevel();

    // IDA_EA: 0x833F258
    // IDA_NAME: _ZN13user_creature15CCreatureScript20GetSkillRecoveryTimeEv
    // DEMANGLED: user_creature::CCreatureScript::GetSkillRecoveryTime(void)
    int GetSkillRecoveryTime();

    // IDA_EA: 0x833F264
    // IDA_NAME: _ZN13user_creature15CCreatureScript24GetOverSkillRecoveryTimeEv
    // DEMANGLED: user_creature::CCreatureScript::GetOverSkillRecoveryTime(void)
    int GetOverSkillRecoveryTime();

    // IDA_EA: 0x833F270
    // IDA_NAME: _ZN13user_creature15CCreatureScript22GetLearnOverSkillLevelEv
    // DEMANGLED: user_creature::CCreatureScript::GetLearnOverSkillLevel(void)
    int GetLearnOverSkillLevel();

    int validItemExp(int item_id);
    int validItem_AType(int item_id);
    int validItem_BType(int item_id);
    int IsItemGrowCreature();
    int GetTypeSelectEvolutionLevel();
    int GetGrowItemCountforLevelUp();
    int GET_ATypeCreature();
    int GET_BTypeCreature();

private:
    friend struct CCreatureScriptCmd372LayoutVerifier;

    int evolution_quest;
    int evolution_creature_id;
    std::vector<int> evolution_creature_ids;
    int evolution_level;
    int max_level;
    int item_id;
    unsigned char* artifact_slots;
    int skill_recovery_time;
    int over_skill_recovery_time;
    uint8_t learn_overskill_level;
    uint8_t learn_overskill_gap[3];
    int unknown_30;
    std::set<int> item_evolution_creature_ids;
    bool using_exp_item;
    int type_select_evolution_level;
    int a_type_creature;
    int b_type_creature;
    int a_type_item_index;
    int b_type_item_index;
    int item_count_for_levelup;
    std::vector<int> event_evolute_creatures;
};

// Keep offset assertions beside the C-owned declaration.  This verifier is a
// friend solely so the field layout can remain private to the canonical ABI.
struct CCreatureScriptCmd372LayoutVerifier {
    static_assert(offsetof(CCreatureScript, skill_recovery_time) == 0x24u,
                  "CCreatureScript skill recovery must stay at IDA +0x24");
    static_assert(offsetof(CCreatureScript, over_skill_recovery_time) == 0x28u,
                  "CCreatureScript over-skill recovery must stay at IDA +0x28");
    static_assert(offsetof(CCreatureScript, learn_overskill_level) == 0x2cu,
                  "CCreatureScript learn-over-skill level must stay at IDA +0x2c");
    static_assert(offsetof(CCreatureScript, unknown_30) == 0x30u,
                  "CCreatureScript next word must stay at IDA +0x30");
};

static_assert(sizeof(CCreatureScript) == 0x74u,
              "CCreatureScript IDA ABI size must stay 0x74");

}  // namespace user_creature

#endif  // OLDSRV_OLD_GAME_CREATURE_AVATAR_CREATURE_CMD372_ABI_H_
