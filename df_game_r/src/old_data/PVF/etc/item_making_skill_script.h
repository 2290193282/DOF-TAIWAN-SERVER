#ifndef OLDSRV_OLD_DATA_PVF_ETC_ITEM_MAKING_SKILL_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_ETC_ITEM_MAKING_SKILL_SCRIPT_H_

#include <stdint.h>

#include <map>
#include <utility>
#include <vector>


// IDA_EA: 0x891B84C / 0x891B860
// IDA unit: stUpgradeWithCompound, carried by the ItemMakingSkill
// DataManager/PVF table restored in this module.
struct stUpgradeWithCompound {
    stUpgradeWithCompound();

    stUpgradeWithCompound* clear();

    int32_t min_compound_value;
    int32_t max_compound_value;
    int32_t compound_exp;
    int32_t compound_range;
    int32_t min_upgrade_value;
    int32_t max_upgrade_value;
    int32_t upgrade_exp;
    int32_t upgrade_range;
};

// IDA_EA: 0x8370FF2 / 0x837105A / 0x837DCE2
// IDA unit: stItemMakingSkill.
struct stItemMakingSkill {
    stItemMakingSkill();
    ~stItemMakingSkill();

    stItemMakingSkill* clear();

    std::map<int, stUpgradeWithCompound> upgrade_with_compound;
    std::vector<std::pair<int, int> > exp_limits;
    int32_t itemmaking_exp_min;
    int32_t itemmaking_exp_max;
    int32_t itemmaking_level_limit;
};

// IDA_EA: 0x890F564
// IDA_NAME: _Z21ImportItemMakingSkillPKcR17stItemMakingSkill
// DEMANGLED: ImportItemMakingSkill(char const*,stItemMakingSkill &)
int ImportItemMakingSkill(const char* filename, stItemMakingSkill& script);

static_assert(sizeof(stUpgradeWithCompound) == 0x20u,
              "stUpgradeWithCompound IDA ABI size must stay 0x20");
static_assert(sizeof(stItemMakingSkill) == 0x30u,
              "stItemMakingSkill IDA ABI size must stay 0x30");

#endif  // OLDSRV_OLD_DATA_PVF_ETC_ITEM_MAKING_SKILL_SCRIPT_H_
