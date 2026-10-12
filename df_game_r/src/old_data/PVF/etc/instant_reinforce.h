#ifndef OLDSRV_OLD_DATA_PVF_ETC_INSTANT_REINFORCE_H_
#define OLDSRV_OLD_DATA_PVF_ETC_INSTANT_REINFORCE_H_

#include <vector>

namespace instant_reinforce {

struct SkillGrant {
    SkillGrant();
    SkillGrant(int skill, int granted_level);

    int skill_index;
    int level;
};

struct StatusGrant {
    StatusGrant();
    StatusGrant(int type, int value);

    int status_type;
    int additive_value;
};

struct Rule {
    Rule();

    int map_index;
    // -1 means the PVF rule has no [select index] discriminator.
    int select_index;
    bool on_revive_after_corpse;
    // The first scalar in [skill].  Client sub_1641B20 stores it separately
    // (record +0x04, default 12) and then scans pairs until [/skill]; it is
    // not a pair count.  Its downstream meaning is not invented here.
    int skill_header;
    std::vector<SkillGrant> skills;
    std::vector<StatusGrant> statuses;
    std::vector<int> effects;
};

// Parses the complete 140828 Etc/DungeonEtc/InstantReinforceInfo.etc into a
// temporary table and publishes it only after cross-rule validation succeeds.
bool ImportConfig(const char* filename);
bool IsConfigLoaded();
unsigned int GetRuleCount();
unsigned int GetMapVariantCount(int map_index);
bool HasMap(int map_index);

// A map with multiple rules requires an exact non-negative select index.  A
// singleton rule without [select index] is resolved independently of select.
bool ResolveRule(int map_index, int select_index, Rule* output);

}  // namespace instant_reinforce_0828

#endif  // OLDSRV_OLD_DATA_PVF_ETC_INSTANT_REINFORCE_0828_H_
