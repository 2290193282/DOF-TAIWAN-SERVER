#include "common/manager/cdata_manager_tail_public_abi.h"

#include "common/manager/data_manager.h"
#include "common/manager/npc_relation_condition_lifecycle_abi.h"

#include <new>
#include <set>


namespace {

typedef std::set<int> EventAvatarIndexSet;
typedef std::vector<int> ConditionLevelDungeonVector;
}  // namespace

PcBangItemRentarData* CDataManagerPcBangItemRentarData(
    CDataManager* manager) {
    return manager ? manager->get_pc_bang_item_rental_data() : 0;
}

DimensionActivationData* CDataManagerDimensionActivationData(
    CDataManager* manager) {
    return manager ? manager->get_dimension_activation_data() : 0;
}

// IDA_EA: 0x837C27A
// IDA_NAME: _ZN20PcBangItemRentarDataC2Ev
// DEMANGLED: PcBangItemRentarData::PcBangItemRentarData(void)
PcBangItemRentarData::PcBangItemRentarData()
    : sections(), selection_groups() {}

// IDA_EA: 0x837C2C0
// IDA_NAME: _ZN20PcBangItemRentarDataD2Ev
// DEMANGLED: PcBangItemRentarData::~PcBangItemRentarData()
PcBangItemRentarData::~PcBangItemRentarData() {}

// IDA_EA: 0x8915518
// IDA_NAME: _ZN20PcBangItemRentarData14getSectionDataEi
// DEMANGLED: PcBangItemRentarData::getSectionData(int)
EventRewardSection* PcBangItemRentarData::getSectionData(
    int section_value) {
    for (std::vector<EventRewardSection>::reverse_iterator it =
             sections.rbegin();
         it != sections.rend(); ++it) {
        if (it->byte_01 <= section_value) {
            return &*it;
        }
    }
    return 0;
}

// IDA_EA: 0x891559C
// IDA_NAME: _ZN20PcBangItemRentarData22getRewardSelectionListEi
// DEMANGLED: PcBangItemRentarData::getRewardSelectionList(int)
std::vector<std::vector<EventRewardItemInfo> >*
PcBangItemRentarData::getRewardSelectionList(int selection_group) {
    std::map<int, std::vector<std::vector<EventRewardItemInfo> > >::
        iterator it = selection_groups.find(selection_group);
    if (it != selection_groups.end()) {
        return &it->second;
    }
    return 0;
}

// IDA_EA: 0x891BCB6
// IDA_NAME: _ZN17DimensionCoinInfoC2Ev
// DEMANGLED: DimensionCoinInfo::DimensionCoinInfo(void)
DimensionCoinInfo::DimensionCoinInfo()
    : item_id(0), by_character(0), reserved_05() {}

// IDA_EA: 0x8371422
// IDA_NAME: _ZN23DimensionActivationDataC2Ev
// DEMANGLED: DimensionActivationData::DimensionActivationData(void)
DimensionActivationData::DimensionActivationData()
    : active_flag(0), coin_infos(), reward_data() {
    coin_infos.clear();
}

// IDA_EA: 0x837C306
// IDA_NAME: _ZN23DimensionActivationDataD2Ev
// DEMANGLED: DimensionActivationData::~DimensionActivationData()
DimensionActivationData::~DimensionActivationData() {}

namespace AvatarVariation {

// IDA_EA: 0x88D43F0
// IDA_NAME: _ZN15AvatarVariation8colorRGBC2Ev
// DEMANGLED: AvatarVariation::colorRGB::colorRGB(void)
colorRGB::colorRGB() : red(0xffu), green(0xffu), blue(0xffu) {}

// IDA_EA: 0x88D440A
// IDA_NAME: _ZN15AvatarVariation8colorRGBeqERKS0_
// DEMANGLED: AvatarVariation::colorRGB::operator==(AvatarVariation::colorRGB const&)
bool colorRGB::operator==(const colorRGB& rhs) const {
    return red == rhs.red && green == rhs.green && blue == rhs.blue;
}

// IDA_EA: 0x837D312
// IDA_NAME: _ZN15AvatarVariation15AvatarColorInfoC2Ev
// DEMANGLED: AvatarVariation::AvatarColorInfo::AvatarColorInfo(void)
AvatarColorInfo::AvatarColorInfo() : colors(), names() {}

// IDA_EA: 0x837D358
// IDA_NAME: _ZN15AvatarVariation15AvatarColorInfoD2Ev
// DEMANGLED: AvatarVariation::AvatarColorInfo::~AvatarColorInfo()
AvatarColorInfo::~AvatarColorInfo() {}

}  // namespace AvatarVariation

// IDA_EA: 0x89F7B28
// IDA_NAME: _ZN26stMercenaryCompetitionAreaC2Ev
// DEMANGLED: stMercenaryCompetitionArea::stMercenaryCompetitionArea(void)
stMercenaryCompetitionArea::stMercenaryCompetitionArea()
    : area_index(0),
      reserved_02(),
      reward_groups(),
      value_10(0),
      value_14(0),
      reserved_16() {}

// IDA_EA: 0x89F83E2
// IDA_NAME: _ZN26stMercenaryCompetitionAreaC2ERKS_
// DEMANGLED: stMercenaryCompetitionArea::stMercenaryCompetitionArea(stMercenaryCompetitionArea const&)
stMercenaryCompetitionArea::stMercenaryCompetitionArea(
    const stMercenaryCompetitionArea& rhs)
    : area_index(rhs.area_index),
      reserved_02(),
      reward_groups(rhs.reward_groups),
      value_10(rhs.value_10),
      value_14(rhs.value_14),
      reserved_16() {}

// IDA_EA: 0x83DD70E
// IDA_NAME: _ZN26stMercenaryCompetitionAreaD2Ev
// DEMANGLED: stMercenaryCompetitionArea::~stMercenaryCompetitionArea()
stMercenaryCompetitionArea::~stMercenaryCompetitionArea() {}

// IDA_EA: 0x89F84AC
// IDA_NAME: _ZN26stMercenaryCompetitionAreaaSERKS_
// DEMANGLED: stMercenaryCompetitionArea::operator=(stMercenaryCompetitionArea const&)
stMercenaryCompetitionArea& stMercenaryCompetitionArea::operator=(
    const stMercenaryCompetitionArea& rhs) {
    if (this != &rhs) {
        area_index = rhs.area_index;
        reward_groups = rhs.reward_groups;
        value_10 = rhs.value_10;
        value_14 = rhs.value_14;
    }
    return *this;
}

// IDA_EA: 0x8373328
// IDA_NAME: _ZN21stMercenarySystemInfoC2Ev
// DEMANGLED: stMercenarySystemInfo::stMercenarySystemInfo(void)
stMercenarySystemInfo::stMercenarySystemInfo()
    : duration_sec(3600),
      default_count(200),
      reserved_06(),
      level_ranges(),
      first_rates(),
      second_rates(),
      script_name(),
      third_rates(),
      value_3c(0),
      competition_areas() {}

// IDA_EA: 0x837DB42
// IDA_NAME: _ZN21stMercenarySystemInfoD2Ev
// DEMANGLED: stMercenarySystemInfo::~stMercenarySystemInfo()
stMercenarySystemInfo::~stMercenarySystemInfo() {}
