#include "old_data/PVF/etc/item_making_skill_script.h"

#include "old_data/PVF/common/rdar_script.h"

#include <string>
#include <utility>


// IDA_EA: 0x891B84C
// IDA_NAME: _ZN21stUpgradeWithCompoundC2Ev
// DEMANGLED: stUpgradeWithCompound::stUpgradeWithCompound(void)
stUpgradeWithCompound::stUpgradeWithCompound() {
    clear();
}

// IDA_EA: 0x891B860
// IDA_NAME: _ZN21stUpgradeWithCompound5clearEv
// DEMANGLED: stUpgradeWithCompound::clear(void)
stUpgradeWithCompound* stUpgradeWithCompound::clear() {
    min_compound_value = 0;
    max_compound_value = 0;
    compound_exp = 0;
    compound_range = 0;
    min_upgrade_value = 0;
    max_upgrade_value = 0;
    upgrade_exp = 0;
    upgrade_range = 0;
    return this;
}

// IDA_EA: 0x8370FF2
// IDA_NAME: _ZN17stItemMakingSkillC2Ev
// DEMANGLED: stItemMakingSkill::stItemMakingSkill(void)
stItemMakingSkill::stItemMakingSkill()
    : upgrade_with_compound(),
      exp_limits() {
    clear();
}

// IDA_EA: 0x837105A
// IDA_NAME: _ZN17stItemMakingSkill5clearEv
// DEMANGLED: stItemMakingSkill::clear(void)
stItemMakingSkill* stItemMakingSkill::clear() {
    upgrade_with_compound.clear();
    exp_limits.clear();
    itemmaking_exp_min = 0;
    itemmaking_exp_max = 0;
    itemmaking_level_limit = 0;
    return this;
}

// IDA_EA: 0x837DCE2
// IDA_NAME: _ZN17stItemMakingSkillD2Ev
// DEMANGLED: stItemMakingSkill::~stItemMakingSkill()
stItemMakingSkill::~stItemMakingSkill() {}

// IDA_EA: 0x890F564
// IDA_NAME: _Z21ImportItemMakingSkillPKcR17stItemMakingSkill
// DEMANGLED: ImportItemMakingSkill(char const*,stItemMakingSkill &)
int ImportItemMakingSkill(const char* filename, stItemMakingSkill& script) {
    if (!loadRDARScriptFile("", filename)) {
        return 10142;
    }

    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[itemmaking exp limit]") {
            while (1) {
                const int exp_limit = ScanInt(&success);
                if (!success) {
                    break;
                }

                const int level_limit = ScanInt(&success);
                if (!success) {
                    return 10162;
                }

                script.exp_limits.push_back(
                    std::make_pair(exp_limit, level_limit));
            }
        } else if (tag == "[itemmaking exp]") {
            script.itemmaking_exp_min = ScanInt(&success);
            if (!success) {
                return 10171;
            }

            script.itemmaking_exp_max = ScanInt(&success);
            if (!success) {
                return 10175;
            }
        } else if (tag == "[itemmaking level limit]") {
            script.itemmaking_level_limit = ScanInt(&success);
            if (!success) {
                return 10181;
            }
        } else if (tag == "[upgrade with compound]") {
            while (1) {
                stUpgradeWithCompound row;
                const int compound_type = ScanInt(&success);
                if (!success) {
                    break;
                }

                row.min_compound_value = ScanInt(&success);
                if (!success) {
                    return 10195;
                }

                row.max_compound_value = ScanInt(&success);
                if (!success) {
                    return 10199;
                }

                row.compound_exp = ScanInt(&success);
                if (!success) {
                    return 10203;
                }

                row.compound_range =
                    1000 * (row.max_compound_value -
                            row.min_compound_value) +
                    999;

                row.min_upgrade_value = ScanInt(&success);
                if (!success) {
                    return 10209;
                }

                row.max_upgrade_value = ScanInt(&success);
                if (!success) {
                    return 10213;
                }

                row.upgrade_exp = ScanInt(&success);
                if (!success) {
                    return 10217;
                }

                row.upgrade_range =
                    1000 * (row.max_upgrade_value -
                            row.min_upgrade_value) +
                    999;

                script.upgrade_with_compound.insert(
                    std::make_pair(compound_type, row));
            }
        }
    }

    return 0;
}
