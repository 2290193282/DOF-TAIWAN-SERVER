#include "old_game/pvp/battlefield/battle_field.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/server_utility.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/dungeon/tower/tower_rank_blood_abi.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/inventory/inventory.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x8306F5C
// IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData14stRewardItem_t5resetEv
// DEMANGLED: CBattle_Field::CBloodClearRewardData::stRewardItem_t::reset(void)
bool CBattle_Field::CBloodClearRewardData::stRewardItem_t::reset() {
    item_count_ = 0;
    for (int index = 0; index <= 9; ++index) {
        items_[index].reset();
        // IDA 0x8306F91 writes the item's +0x02 key after reset().
        // Inven_Item retains that proven packed field in its canonical ABI;
        // writing it through the declared packed member preserves +0x02.
        items_[index].item_id_ = -1;
    }
    return false;
}

// IDA_EA: 0x8306EE0
// IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData5resetEv
// DEMANGLED: CBattle_Field::CBloodClearRewardData::reset(void)
CBattle_Field::CBloodClearRewardData*
CBattle_Field::CBloodClearRewardData::reset() {
    current_blood_round_ = 0;
    play_time_msec_ = 0;
    previous_play_time_msec_ = 0;
    blood_clear_ = 0;
    for (int index = 0; index <= 3; ++index) {
        reward_exp_[index] = 0;
        reward_items_[index].reset();
    }
    ultimate_round_clear_point_ = 0;
    return this;
}

// IDA_EA: 0x8306FAA
// IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData17onStartBloodRoundEv
// DEMANGLED: CBattle_Field::CBloodClearRewardData::onStartBloodRound(void)
int CBattle_Field::CBloodClearRewardData::onStartBloodRound() {
    blood_round_start_tick_ = static_cast<uint32_t>(
        GlobalData::s_systemTime_.getCurTickCount());
    return static_cast<int>(blood_round_start_tick_);
}

// IDA_EA: 0x8306FC4
// IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData18onFinishBloodRoundEbsP6CPartyPK8CDungeon
// DEMANGLED: CBattle_Field::CBloodClearRewardData::onFinishBloodRound(bool,short,CParty *,CDungeon const*)
bool CBattle_Field::CBloodClearRewardData::onFinishBloodRound(
    bool blood_clear,
    short blood_round,
    CParty* party,
    const CDungeon* dungeon) {
    if (blood_clear) {
        setBloodClear(true);
    }
    current_blood_round_ = blood_round;
    blood_round_finish_tick_ = static_cast<uint32_t>(
        GlobalData::s_systemTime_.getCurTickCount());
    const unsigned int elapsed =
        static_cast<unsigned int>(getBloodRoundClearTime());
    addPlayTime(elapsed);
    const unsigned int total_play_seconds =
        static_cast<unsigned int>(getPlayTime()) / 1000u;
    const unsigned int round_seconds =
        static_cast<unsigned int>(getBloodRoundClearTime()) / 1000u;
    return party->historyBloodRoundClear(
        current_blood_round_,
        round_seconds,
        total_play_seconds,
        dungeon->GetDungeonName());
}

// IDA_EA: 0x8307096
// IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData26incUltimateRoundClearPointEi
// DEMANGLED: CBattle_Field::CBloodClearRewardData::incUltimateRoundClearPoint(int)
CBattle_Field::CBloodClearRewardData*
CBattle_Field::CBloodClearRewardData::incUltimateRoundClearPoint(
    int next_round_difficulty) {
    const stBloodDungeon_t& blood_dungeon =
        G_CDataManager()->get_blood_dungeon();
    ultimate_round_clear_point_ +=
        blood_dungeon.ultimate_difficulty_points[next_round_difficulty];
    return this;
}

// IDA_EA: 0x83070CA
// IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData21getUltimateRewardItemEv
// DEMANGLED: CBattle_Field::CBloodClearRewardData::getUltimateRewardItem(void)
int CBattle_Field::CBloodClearRewardData::getUltimateRewardItem() {
    const stBloodDungeon_t& blood_dungeon =
        G_CDataManager()->get_blood_dungeon();
    const std::map<int, STUltimateRewardProb>::const_iterator found =
        blood_dungeon.ultimate_reward_prob.find(getUltimateRoundClearPoint());
    if (found == blood_dungeon.ultimate_reward_prob.end()) {
        return -1;
    }

    const int random_value = static_cast<int>(get_rand_int(100));
    const int ultimate_reward_rate = found->second.second_dword;
    if (random_value <= ultimate_reward_rate) {
        return 1252;
    }
    const int normal_reward_rate =
        found->second.first_dword + ultimate_reward_rate;
    return random_value > normal_reward_rate ? -1 : 1251;
}

// IDA_EA: 0x830EC3A
// IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData11addPlayTimeEj
// DEMANGLED: CBattle_Field::CBloodClearRewardData::addPlayTime(unsigned int)
CBattle_Field::CBloodClearRewardData*
CBattle_Field::CBloodClearRewardData::addPlayTime(unsigned int elapsed_msec) {
    previous_play_time_msec_ = play_time_msec_;
    play_time_msec_ += elapsed_msec;
    return this;
}

// IDA_EA: 0x830EC5A
// IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData11getPlayTimeEv
// DEMANGLED: CBattle_Field::CBloodClearRewardData::getPlayTime(void)
int CBattle_Field::CBloodClearRewardData::getPlayTime() {
    return static_cast<int>(play_time_msec_);
}

// IDA_EA: 0x830EC64
// IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData22getBloodRoundClearTimeEv
// DEMANGLED: CBattle_Field::CBloodClearRewardData::getBloodRoundClearTime(void)
int CBattle_Field::CBloodClearRewardData::getBloodRoundClearTime() {
    return static_cast<int>(blood_round_finish_tick_ - blood_round_start_tick_);
}

// IDA_EA: 0x830EC7C
// IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData13setBloodClearEb
// DEMANGLED: CBattle_Field::CBloodClearRewardData::setBloodClear(bool)
CBattle_Field::CBloodClearRewardData*
CBattle_Field::CBloodClearRewardData::setBloodClear(bool blood_clear) {
    blood_clear_ = static_cast<uint8_t>(blood_clear);
    return this;
}

// IDA_EA: 0x830EC94
// IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData26getUltimateRoundClearPointEv
// DEMANGLED: CBattle_Field::CBloodClearRewardData::getUltimateRoundClearPoint(void)
int CBattle_Field::CBloodClearRewardData::getUltimateRoundClearPoint() {
    return ultimate_round_clear_point_;
}

// IDA_EA: 0x85BF0E6
// IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData20getCurrentBloodRoundEv
// DEMANGLED: CBattle_Field::CBloodClearRewardData::getCurrentBloodRound(void)
int CBattle_Field::CBloodClearRewardData::getCurrentBloodRound() {
    return current_blood_round_;
}

// IDA_EA: 0x85BF0F2
// IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData20setCurrentBloodRoundEi
// DEMANGLED: CBattle_Field::CBloodClearRewardData::setCurrentBloodRound(int)
CBattle_Field::CBloodClearRewardData*
CBattle_Field::CBloodClearRewardData::setCurrentBloodRound(int blood_round) {
    current_blood_round_ = blood_round;
    return this;
}

// IDA_EA: 0x85BF100
// IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData18getRewardItemCountEi
// DEMANGLED: CBattle_Field::CBloodClearRewardData::getRewardItemCount(int)
int CBattle_Field::CBloodClearRewardData::getRewardItemCount(int reward_group) {
    return reward_items_[reward_group].item_count_;
}

// IDA_EA: 0x85BF11A
// IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData13getRewardItemEi
// DEMANGLED: CBattle_Field::CBloodClearRewardData::getRewardItem(int)
Inven_Item*
CBattle_Field::CBloodClearRewardData::getRewardItem(int reward_group) {
    return reward_items_[reward_group].items_;
}

// IDA_EA: 0x85BF132
// IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData12getRewardExpEi
// DEMANGLED: CBattle_Field::CBloodClearRewardData::getRewardExp(int)
int CBattle_Field::CBloodClearRewardData::getRewardExp(int reward_group) {
    return static_cast<int>(reward_exp_[reward_group]);
}

// IDA_EA: 0x85BF144
// IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData12setRewardExpEij
// DEMANGLED: CBattle_Field::CBloodClearRewardData::setRewardExp(int,unsigned int)
CBattle_Field::CBloodClearRewardData*
CBattle_Field::CBloodClearRewardData::setRewardExp(int reward_group,
                                                    unsigned int reward_exp) {
    reward_exp_[reward_group] = reward_exp;
    return this;
}

// IDA_EA: 0x85BF15A
// IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData13setRewardItemEii
// DEMANGLED: CBattle_Field::CBloodClearRewardData::setRewardItem(int,int)
int CBattle_Field::CBloodClearRewardData::setRewardItem(int reward_group,
                                                         int item_count) {
    reward_items_[reward_group].item_count_ = item_count;
    return item_count;
}

// IDA_EA: 0x85BF176
// IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData16setRewarItemCopyE10Inven_Itemii
// DEMANGLED: CBattle_Field::CBloodClearRewardData::setRewarItemCopy(Inven_Item,int,int)
Inven_Item* CBattle_Field::CBloodClearRewardData::setRewarItemCopy(
    Inven_Item item,
    int reward_group,
    int item_slot) {
    return reward_items_[reward_group].items_[item_slot].setCopy(item);
}

// IDA_EA: 0x85BF1A8
// IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData20setRewardItemAddInfoEfii
// DEMANGLED: CBattle_Field::CBloodClearRewardData::setRewardItemAddInfo(float,int,int)
Inven_Item* CBattle_Field::CBloodClearRewardData::setRewardItemAddInfo(
    float multiplier,
    int reward_group,
    int item_slot) {
    Inven_Item& item = reward_items_[reward_group].items_[item_slot];
    const int scaled_add_info = static_cast<int>(
        static_cast<long double>(item.get_add_info()) * multiplier);
    return item.set_add_info(scaled_add_info);
}

// IDA_EA: 0x85BF21C
// IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData12isBloodClearEv
// DEMANGLED: CBattle_Field::CBloodClearRewardData::isBloodClear(void)
int CBattle_Field::CBloodClearRewardData::isBloodClear() {
    return blood_clear_;
}
