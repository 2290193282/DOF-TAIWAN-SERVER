#ifndef OLDSRV_OLD_DATA_PVF_ETC_BLOOD_DUNGEON_REWARD_INFO_H_
#define OLDSRV_OLD_DATA_PVF_ETC_BLOOD_DUNGEON_REWARD_INFO_H_

#include <utility>
#include <vector>

struct stBloodDungeonRewardInfo {
    // IDA_EA: 0x836FB6C
    // IDA_NAME: _ZN24stBloodDungeonRewardInfoC2Ev
    // DEMANGLED: stBloodDungeonRewardInfo::stBloodDungeonRewardInfo(void)
    stBloodDungeonRewardInfo();

    // IDA_EA: 0x836FBD8
    // IDA_NAME: _ZN24stBloodDungeonRewardInfoD2Ev
    // DEMANGLED: stBloodDungeonRewardInfo::~stBloodDungeonRewardInfo()
    ~stBloodDungeonRewardInfo();

    std::vector<std::pair<int, int> > price_average;
    std::vector<std::pair<int, int> > admission_increase;
    std::vector<std::pair<int, int> > lotto_point;
};

static_assert(sizeof(stBloodDungeonRewardInfo) == 0x24u,
              "stBloodDungeonRewardInfo i386 layout drift");

#endif  // OLDSRV_OLD_DATA_PVF_ETC_BLOOD_DUNGEON_REWARD_INFO_H_
