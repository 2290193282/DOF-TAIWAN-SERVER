#ifndef OLDSRV_GLOBAL_CDATA_MANAGER_TAIL_PUBLIC_ABI_H_
#define OLDSRV_GLOBAL_CDATA_MANAGER_TAIL_PUBLIC_ABI_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "old_game/activity/schedule/event_value_types.h"

class CDataManager;

// A/public value ABI for old CDataManager/PVF tail slots. Importer and runtime
// behavior remains in the owning PVF/event/PVP modules.

// IDA unit: 99_misc_global/PcBangItemRentarData.
struct PcBangItemRentarData {
    PcBangItemRentarData();
    ~PcBangItemRentarData();

    EventRewardSection* getSectionData(int section_value);
    std::vector<std::vector<EventRewardItemInfo> >*
    getRewardSelectionList(int selection_group);

    std::vector<EventRewardSection> sections;
    std::map<int, std::vector<std::vector<EventRewardItemInfo> > >
        selection_groups;
};

static_assert(sizeof(PcBangItemRentarData) == 0x24u,
              "PcBangItemRentarData IDA ABI size must stay 0x24");
static_assert(offsetof(PcBangItemRentarData, selection_groups) == 0x0cu,
              "PcBangItemRentarData selection_groups offset must stay 0x0c");

// IDA unit: 99_misc_global/DimensionCoinInfo.
struct DimensionCoinInfo {
    DimensionCoinInfo();

    uint32_t item_id;
    uint8_t by_character;
    uint8_t reserved_05[3];
};

static_assert(sizeof(DimensionCoinInfo) == 0x08u,
              "DimensionCoinInfo IDA ABI size must stay 0x08");
static_assert(offsetof(DimensionCoinInfo, by_character) == 0x04u,
              "DimensionCoinInfo by_character offset must stay 0x04");

// IDA unit: 99_misc_global/DimensionActivationData.
struct DimensionActivationData {
    DimensionActivationData();
    ~DimensionActivationData();

    uint32_t active_flag;
    std::map<int, DimensionCoinInfo> coin_infos;
    DimensionActivationRewardData reward_data;
};

static_assert(sizeof(DimensionActivationData) == 0x58u,
              "DimensionActivationData IDA ABI size must stay 0x58");
static_assert(offsetof(DimensionActivationData, coin_infos) == 0x04u,
              "DimensionActivationData coin_infos offset must stay 0x04");
static_assert(offsetof(DimensionActivationData, reward_data) == 0x1cu,
              "DimensionActivationData reward_data offset must stay 0x1c");

// IDA unit: 06_userinfo_stat_inventory/AvatarVariation public color values.
namespace AvatarVariation {

struct colorRGB {
    colorRGB();

    bool operator==(const colorRGB& rhs) const;

    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

static_assert(sizeof(colorRGB) == 0x03u,
              "AvatarVariation::colorRGB IDA ABI size must stay 0x03");

struct AvatarColorInfo {
    AvatarColorInfo();
    ~AvatarColorInfo();

    std::map<int, colorRGB> colors;
    std::map<int, std::string> names;
};

static_assert(sizeof(AvatarColorInfo) == 0x30u,
              "AvatarVariation::AvatarColorInfo IDA ABI size must stay 0x30");
static_assert(offsetof(AvatarColorInfo, names) == 0x18u,
              "AvatarVariation::AvatarColorInfo names offset must stay 0x18");

}  // namespace AvatarVariation

// IDA unit: 05_world_enter_village/stMercenaryCompetitionArea.
struct stMercenaryCompetitionArea {
    stMercenaryCompetitionArea();
    stMercenaryCompetitionArea(const stMercenaryCompetitionArea& rhs);
    ~stMercenaryCompetitionArea();

    stMercenaryCompetitionArea& operator=(
        const stMercenaryCompetitionArea& rhs);

    uint16_t area_index;
    uint8_t reserved_02[2];
    std::vector<stMercenaryRewardGroup> reward_groups;
    uint32_t value_10;
    uint16_t value_14;
    uint8_t reserved_16[2];
};

static_assert(sizeof(stMercenaryCompetitionArea) == 0x18u,
              "stMercenaryCompetitionArea IDA ABI size must stay 0x18");
static_assert(offsetof(stMercenaryCompetitionArea, reward_groups) == 0x04u,
              "stMercenaryCompetitionArea reward_groups offset must stay 0x04");

// IDA unit: 99_misc_global/stMercenarySystemInfo.
struct stMercenarySystemInfo {
    stMercenarySystemInfo();
    ~stMercenarySystemInfo();

    uint32_t duration_sec;
    uint16_t default_count;
    uint8_t reserved_06[2];
    std::vector<std::pair<int, std::pair<int, int> > > level_ranges;
    std::vector<std::pair<int, float> > first_rates;
    std::vector<std::pair<int, float> > second_rates;
    std::string script_name;
    std::vector<std::pair<int, float> > third_rates;
    uint32_t value_3c;
    std::vector<stMercenaryCompetitionArea> competition_areas;
};

static_assert(sizeof(stMercenarySystemInfo) == 0x4cu,
              "stMercenarySystemInfo IDA ABI size must stay 0x4c");
static_assert(offsetof(stMercenarySystemInfo, level_ranges) == 0x08u,
              "stMercenarySystemInfo level_ranges offset must stay 0x08");
static_assert(offsetof(stMercenarySystemInfo, script_name) == 0x2cu,
              "stMercenarySystemInfo script_name offset must stay 0x2c");
static_assert(offsetof(stMercenarySystemInfo, competition_areas) == 0x40u,
              "stMercenarySystemInfo competition_areas offset must stay 0x40");


using ::DimensionActivationData;
using ::DimensionCoinInfo;
using ::PcBangItemRentarData;
using ::stMercenaryCompetitionArea;
using ::stMercenarySystemInfo;

// CDataManager C2/D2 embeds these two rental-script values as named members.
// Keep their physical owner behind typed public accessors.
PcBangItemRentarData* CDataManagerPcBangItemRentarData(CDataManager* manager);
DimensionActivationData* CDataManagerDimensionActivationData(
    CDataManager* manager);

struct PvPSkillTreeParameterScript;
struct EventAvatarListScript;
class DeleteInvalidItemScript;

// IDA_EA: 0x8372E10 / 0x837DD8C.
// CDataManager embeds EventAvatarListScript at old offset +0xB508.
// IDA_EA: 0x8AC1D40 / 0x8AC1DCE.
// CDataManager embeds DeleteInvalidItemScript at old offset +0xB658.

#endif  // OLDSRV_GLOBAL_CDATA_MANAGER_TAIL_PUBLIC_ABI_H_
