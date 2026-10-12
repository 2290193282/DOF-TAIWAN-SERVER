#ifndef OLDSRV_GLOBAL_GROWTH_BOOSTER_ABI_H_
#define OLDSRV_GLOBAL_GROWTH_BOOSTER_ABI_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <vector>

#include "old_game/activity/schedule/event_value_types.h"
#include "common/types/stackable_value_types.h"

typedef std::map<int, std::vector<std::vector<STStackablePackageData> > >
    GrowthPowerPackageRewardMap;

namespace oldsrv {
namespace policy {

// The 0828 deployment keeps the growth/experience booster, but deliberately
// disables the newbie artifact-equipment selector and its CMD429 grant path.
static const bool kGrowthPowerEquipmentRewardEnabled = false;

}  // namespace policy
}  // namespace oldsrv

// IDA unit: 99_misc_global/STBreakAwaySection.
struct STBreakAwaySection {
    STBreakAwaySection();
    ~STBreakAwaySection();

    uint8_t index;       // +0x00
    uint8_t low_level;   // +0x01
    uint8_t high_level;  // +0x02
};

static_assert(sizeof(STBreakAwaySection) == 0x03u,
              "STBreakAwaySection IDA ABI size must stay 0x03");

// IDA unit: 08_quest_skill_growth/STGrowthPowerData.
struct STGrowthPowerData {
    STGrowthPowerData();
    ~STGrowthPowerData();

    int get_break_away_section_low_level(int charac_level);
    STBreakAwaySection get_break_away_section_data(int charac_level);
    int get_idx_break_away_section(int charac_level);

    std::vector<STBreakAwaySection> break_away_sections;  // +0x00
    std::vector<STExpReward> exp_rewards;         // +0x0c
    GrowthPowerPackageRewardMap package_rewards;          // +0x18
};

static_assert(sizeof(STGrowthPowerData) == 0x30u,
              "STGrowthPowerData IDA ABI size must stay 0x30");
static_assert(offsetof(STGrowthPowerData, exp_rewards) == 0x0cu,
              "STGrowthPowerData exp_rewards offset must stay 0x0c");
static_assert(offsetof(STGrowthPowerData, package_rewards) == 0x18u,
              "STGrowthPowerData package_rewards offset must stay 0x18");

// IDA unit: 99_misc_global/BoosterPoint.
struct BoosterPoint {
    ~BoosterPoint();

    uint32_t item_index;  // +0x00
    uint8_t point;        // +0x04
};

static_assert(sizeof(BoosterPoint) == 0x08u,
              "BoosterPoint IDA ABI size must stay 0x08");
static_assert(offsetof(BoosterPoint, point) == 0x04u,
              "BoosterPoint point offset must stay 0x04");

// IDA unit: 99_misc_global/BoosterGageData.
struct BoosterGageData {
    BoosterGageData();
    ~BoosterGageData();

    int getBoosterPoint(unsigned long item_index);

    std::vector<BoosterPoint> booster_points;  // +0x00
    uint8_t max_gage;                          // +0x0c
    uint8_t sub_gage;                          // +0x0d
    uint8_t reward_multiple;                   // +0x0e
};

static_assert(sizeof(BoosterGageData) == 0x10u,
              "BoosterGageData IDA ABI size must stay 0x10");
static_assert(offsetof(BoosterGageData, max_gage) == 0x0cu,
              "BoosterGageData max_gage offset must stay 0x0c");

// IDA_EA: 0x8911EE3
// IDA_NAME: _Z17importGrowthPowerPcP17STGrowthPowerData
// DEMANGLED: importGrowthPower(char *,STGrowthPowerData *)
int importGrowthPower(char* filename, STGrowthPowerData* data);

// IDA_EA: 0x8913A7E
// IDA_NAME: _Z17importBoosterGagePcP15BoosterGageData
// DEMANGLED: importBoosterGage(char *,BoosterGageData *)
int importBoosterGage(char* filename, BoosterGageData* data);


using ::BoosterGageData;
using ::BoosterPoint;
using ::GrowthPowerPackageRewardMap;
using ::STBreakAwaySection;
using ::STGrowthPowerData;
using ::importBoosterGage;
using ::importGrowthPower;


#endif  // OLDSRV_GLOBAL_GROWTH_BOOSTER_ABI_H_
