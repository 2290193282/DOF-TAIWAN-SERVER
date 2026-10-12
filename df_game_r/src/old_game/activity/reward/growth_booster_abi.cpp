#include "old_game/activity/reward/growth_booster_abi.h"

#include <new>

// IDA_EA: 0x822AFB0
// IDA_NAME: _ZN18STBreakAwaySectionC2Ev
// DEMANGLED: STBreakAwaySection::STBreakAwaySection(void)
STBreakAwaySection::STBreakAwaySection()
    : index(0), low_level(0), high_level(0) {}

// IDA_EA: 0x8941ABC
// IDA_NAME: _ZN18STBreakAwaySectionD2Ev
// DEMANGLED: STBreakAwaySection::~STBreakAwaySection()
STBreakAwaySection::~STBreakAwaySection() {}

// IDA_EA: 0x822AFD6
// IDA_NAME: _ZN17STGrowthPowerData32get_break_away_section_low_levelEi
// DEMANGLED: STGrowthPowerData::get_break_away_section_low_level(int)
int STGrowthPowerData::get_break_away_section_low_level(int charac_level) {
    int low_level_result = -1;
    for (std::vector<STBreakAwaySection>::const_iterator it =
             break_away_sections.begin();
         it != break_away_sections.end(); ++it) {
        if (it->low_level > charac_level) {
            return low_level_result;
        }
        low_level_result = it->low_level;
    }
    return low_level_result;
}

// IDA_EA: 0x822B07E
// IDA_NAME: _ZN17STGrowthPowerData27get_break_away_section_dataEi
// DEMANGLED: STGrowthPowerData::get_break_away_section_data(int)
STBreakAwaySection STGrowthPowerData::get_break_away_section_data(
    int charac_level) {
    STBreakAwaySection section;
    for (std::vector<STBreakAwaySection>::const_iterator it =
             break_away_sections.begin();
         it != break_away_sections.end() && it->low_level <= charac_level;
         ++it) {
        section = *it;
    }
    return section;
}

// IDA_EA: 0x822B136
// IDA_NAME: _ZN17STGrowthPowerData26get_idx_break_away_sectionEi
// DEMANGLED: STGrowthPowerData::get_idx_break_away_section(int)
int STGrowthPowerData::get_idx_break_away_section(int charac_level) {
    int index = 0;
    for (std::vector<STBreakAwaySection>::const_iterator it =
             break_away_sections.begin();
         it != break_away_sections.end(); ++it, ++index) {
        if (it->low_level <= charac_level && it->high_level > charac_level) {
            return index;
        }
    }
    return -1;
}

// IDA_EA: 0x83711FE
// IDA_NAME: _ZN17STGrowthPowerDataC2Ev
// DEMANGLED: STGrowthPowerData::STGrowthPowerData(void)
STGrowthPowerData::STGrowthPowerData()
    : break_away_sections(), exp_rewards(), package_rewards() {
    break_away_sections.clear();
    exp_rewards.clear();
    package_rewards.clear();
}

// IDA_EA: 0x837D290
// IDA_NAME: _ZN17STGrowthPowerDataD2Ev
// DEMANGLED: STGrowthPowerData::~STGrowthPowerData()
STGrowthPowerData::~STGrowthPowerData() {}

// IDA_EA: 0x8942DBE
// IDA_NAME: _ZN12BoosterPointD2Ev
// DEMANGLED: BoosterPoint::~BoosterPoint()
BoosterPoint::~BoosterPoint() {}

// IDA_EA: 0x80DD4D4
// IDA_NAME: _ZN15BoosterGageData15getBoosterPointEm
// DEMANGLED: BoosterGageData::getBoosterPoint(unsigned long)
int BoosterGageData::getBoosterPoint(unsigned long item_index) {
    for (std::vector<BoosterPoint>::const_iterator it =
             booster_points.begin();
         it != booster_points.end(); ++it) {
        if (it->item_index == item_index) {
            return it->point;
        }
    }
    return 0;
}

// IDA_EA: 0x83712A8
// IDA_NAME: _ZN15BoosterGageDataC2Ev
// DEMANGLED: BoosterGageData::BoosterGageData(void)
BoosterGageData::BoosterGageData()
    : booster_points(), max_gage(0), sub_gage(0), reward_multiple(0) {
    booster_points.clear();
}

// IDA_EA: 0x837D39E
// IDA_NAME: _ZN15BoosterGageDataD2Ev
// DEMANGLED: BoosterGageData::~BoosterGageData()
BoosterGageData::~BoosterGageData() {}
