#include "old_data/PVF/town/arad_village_attack_reward.h"

#include "old_data/PVF/common/arad_script.h"
#include "infra/log/format/trace.h"

#include <algorithm>
#include <cstring>

namespace {

char* Mutable(const char* text) {
    return const_cast<char*>(text);
}

}  // namespace

// IDA_EA: 0x81A4222
// IDA_NAME: _ZN28Arad_VillageAttackRewardItem4DataC2Ev
// DEMANGLED: Arad_VillageAttackRewardItem::Data::Data(void)
Arad_VillageAttackRewardItem::Data::Data()
    : win_point(0), item_index(0), item_count(0) {}

// IDA_EA: 0x84ED148
// IDA_NAME: _ZN28Arad_VillageAttackRewardItem4DataC2Eimi
// DEMANGLED: Arad_VillageAttackRewardItem::Data::Data(int,unsigned long,int)
Arad_VillageAttackRewardItem::Data::Data(int win_point_value,
                                         unsigned long item_index_value,
                                         int item_count_value)
    : win_point(win_point_value),
      item_index(item_index_value),
      item_count(item_count_value) {}

// IDA_EA: 0x81A4244
// IDA_NAME: _ZN28Arad_VillageAttackRewardItem4DataeqERKS0_
// DEMANGLED: Arad_VillageAttackRewardItem::Data::operator==(Arad_VillageAttackRewardItem::Data const&)
bool Arad_VillageAttackRewardItem::Data::operator==(const Data& rhs) {
    return win_point == rhs.win_point;
}

// IDA_EA: 0x81A778C
// IDA_NAME: _ZN28Arad_VillageAttackRewardItem4DataD2Ev
// DEMANGLED: Arad_VillageAttackRewardItem::Data::~Data()
Arad_VillageAttackRewardItem::Data::~Data() {}

// IDA_EA: 0x81A4258
// IDA_NAME: _ZN28Arad_VillageAttackRewardItem8RankDataC2Ev
// DEMANGLED: Arad_VillageAttackRewardItem::RankData::RankData(void)
Arad_VillageAttackRewardItem::RankData::RankData() {
    std::memset(this, 0, sizeof(*this));
}

// IDA_EA: 0x81A79D8
// IDA_NAME: _ZN28Arad_VillageAttackRewardItem8RankDataD2Ev
// DEMANGLED: Arad_VillageAttackRewardItem::RankData::~RankData()
Arad_VillageAttackRewardItem::RankData::~RankData() {}

// IDA_EA: 0x8188DBE
// IDA_NAME: _ZN28Arad_VillageAttackRewardItemC2Ev
// DEMANGLED: Arad_VillageAttackRewardItem::Arad_VillageAttackRewardItem(void)
Arad_VillageAttackRewardItem::Arad_VillageAttackRewardItem()
    : Arad_ScriptLoader(), reward_items_(), ranking_rewards_() {}

// IDA_EA: 0x8188E1E
// IDA_NAME: _ZN28Arad_VillageAttackRewardItemD2Ev
// DEMANGLED: Arad_VillageAttackRewardItem::~Arad_VillageAttackRewardItem()
Arad_VillageAttackRewardItem::~Arad_VillageAttackRewardItem() {}

// IDA_EA: 0x81A3B48
// IDA_NAME: _ZN28Arad_VillageAttackRewardItem7parsingEPc
// DEMANGLED: Arad_VillageAttackRewardItem::parsing(char *)
int Arad_VillageAttackRewardItem::parsing(char* buffer) {
    Arad_Script script;
    if (script.Load(buffer, "reward_item") != 1) {
        return 562;
    }
    for (int row = 0; row < script.GetRowNum(); ++row) {
        Data current;
        current.win_point = script.GetInt(row, Mutable("win_point"));
        current.item_index = static_cast<unsigned long>(
            script.GetInt(row, Mutable("item_index")));
        current.item_count = script.GetInt(row, Mutable("item_count"));
        Data existing;
        if (find_reward_item(current.win_point, existing) != 0) {
            LogManager::logFormat(
                1, "localjapan/Arad_Script_Loader.cpp",
                "virtual int Arad_VillageAttackRewardItem::parsing(char*)",
                581,
                "[ERROR] Already exist data. (point:%d, item:%u, count:%d)",
                current.win_point, current.item_index, current.item_count);
            return 581;
        }
        reward_items_.push_back(current);
    }

    if (script.Load(buffer, "reward_ranker") != 1) {
        return 588;
    }
    for (int row = 0; row < script.GetRowNum(); ++row) {
        RankData current;
        current.item_index = static_cast<unsigned long>(
            script.GetInt(row, Mutable("item_index")));
        current.item_count = script.GetInt(row, Mutable("item_count"));
        current.item_index2 = static_cast<unsigned long>(
            script.GetInt(row, Mutable("item_index2")));
        current.item_count2 = script.GetInt(row, Mutable("item_count2"));
        ranking_rewards_.push_back(current);
    }
    return 0;
}

// IDA_EA: 0x81A3E3C
// IDA_NAME: _ZN28Arad_VillageAttackRewardItem16find_reward_itemEiRNS_4DataE
// DEMANGLED: Arad_VillageAttackRewardItem::find_reward_item(int,Arad_VillageAttackRewardItem::Data &)
int Arad_VillageAttackRewardItem::find_reward_item(int win_point,
                                                   Data& out_data) {
    const Data key(win_point, 0, 0);
    std::vector<Data>::iterator it =
        std::find(reward_items_.begin(), reward_items_.end(), key);
    if (it == reward_items_.end()) {
        return 0;
    }
    out_data = *it;
    return 1;
}

// IDA_EA: 0x81A3EF0
// IDA_NAME: _ZN28Arad_VillageAttackRewardItem24find_ranking_reward_itemEsRNS_8RankDataE
// DEMANGLED: Arad_VillageAttackRewardItem::find_ranking_reward_item(short,Arad_VillageAttackRewardItem::RankData &)
int Arad_VillageAttackRewardItem::find_ranking_reward_item(
    short rank_index,
    RankData& out_data) {
    if (rank_index < 0 ||
        static_cast<short>(ranking_rewards_.size()) <= rank_index) {
        return 0;
    }
    out_data = ranking_rewards_[rank_index];
    return 1;
}
