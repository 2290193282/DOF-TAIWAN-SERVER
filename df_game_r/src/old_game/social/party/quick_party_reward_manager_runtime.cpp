#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"

#include <cstring>
#include <map>

#include "infra/log/format/trace.h"
#include "old_data/PVF/etc/quick_party_script.h"
#include "old_game/social/party/party.h"

namespace QuickParty {
namespace {

typedef std::map<int, LevelDataRandomReward> RandomRewardMap;

RandomRewardMap::iterator FindRandomReward(QuickPartyData& data,
                                           int random_buff_type) {
    return data.random_rewards.find(random_buff_type);
}

bool HasMemberLevelRow(const std::vector<LevelData>& rows,
                       CParty* party,
                       int& row_index) {
    row_index = party->get_member_count() - 2;
    return row_index >= 0 &&
           row_index < static_cast<int>(rows.size());
}

}  // namespace

// IDA_EA: 0x826DC30
// IDA_NAME: _ZN10QuickParty14RewardCardGoldC2ENS_15BasicRewardTypeERNS_14QuickPartyDataE
RewardCardGold::RewardCardGold(BasicRewardType reward_type,
                               QuickPartyData& data)
    : IBasicReward(data) {
    (void)reward_type;
}

// IDA_EA: 0x826D070
// IDA_NAME: _ZN10QuickParty14RewardCardGold7processENS_16RewardData_ParamERNS_17RewardData_ResultE
int RewardCardGold::process(RewardData_Param param, RewardData_Result& result) {
    int row_index = 0;
    const std::vector<LevelData>& rows =
        quick_party_data->basic_reward.level_data;
    if (HasMemberLevelRow(rows, param.party, row_index)) {
        const float rate = static_cast<float>(rows[row_index].raw_04) /
                               100.0f +
                           1.0f;
        result.raw_00 = static_cast<int>(
            static_cast<long double>(result.raw_00) * rate);
    }
    return reinterpret_cast<intptr_t>(&result);
}

// IDA_EA: 0x826DC54
// IDA_NAME: _ZN10QuickParty19HellPartyDifficultyC2ENS_15BasicRewardTypeERNS_14QuickPartyDataE
HellPartyDifficulty::HellPartyDifficulty(BasicRewardType reward_type,
                                         QuickPartyData& data)
    : IBasicReward(data) {
    (void)reward_type;
}

// IDA_EA: 0x826D116
// IDA_NAME: _ZN10QuickParty19HellPartyDifficulty7processENS_16RewardData_ParamERNS_17RewardData_ResultE
int HellPartyDifficulty::process(RewardData_Param param,
                                 RewardData_Result& result) {
    const int row_index = param.party->get_member_count() - 2;
    const std::vector<LevelDataHell>& rows =
        quick_party_data->basic_reward.hell_level_data;
    if (row_index >= 0 && row_index < static_cast<int>(rows.size())) {
        result.raw_00 = rows[row_index].raw_04;
        result.raw_04 = rows[row_index].raw_00;
    }
    return reinterpret_cast<intptr_t>(&result);
}

// IDA_EA: 0x826DCA2
// IDA_NAME: _ZN10QuickParty14KillMonsterExpC2ENS_14RandomBuffTypeERNS_14QuickPartyDataE
KillMonsterExp::KillMonsterExp(RandomBuffType type, QuickPartyData& data)
    : IRandomwReward(data), random_buff_type(static_cast<int>(type)) {}

// IDA_EA: 0x826D198
// IDA_NAME: _ZN10QuickParty14KillMonsterExp7processENS_16RewardData_ParamERNS_17RewardData_ResultE
int KillMonsterExp::process(RewardData_Param param, RewardData_Result& result) {
    RandomRewardMap::iterator reward =
        FindRandomReward(*quick_party_data, random_buff_type);
    if (reward == quick_party_data->random_rewards.end()) {
        return 1;
    }

    int row_index = 0;
    if (HasMemberLevelRow(reward->second.level_data_08, param.party,
                          row_index)) {
        const float rate =
            static_cast<float>(reward->second.level_data_08[row_index].raw_04) /
                100.0f +
            1.0f;
        result.raw_00 = static_cast<int>(
            static_cast<long double>(result.raw_00) * rate);
    }
    return reinterpret_cast<intptr_t>(&result);
}

// IDA_EA: 0x826DCCE
// IDA_NAME: _ZN10QuickParty17EquipItemDropRateC2ENS_14RandomBuffTypeERNS_14QuickPartyDataE
// DEMANGLED: QuickParty::EquipItemDropRate::EquipItemDropRate(
//     QuickParty::RandomBuffType,QuickParty::QuickPartyData &)
EquipItemDropRate::EquipItemDropRate(RandomBuffType type,
                                     QuickPartyData& data)
    : IRandomwReward(data), random_buff_type(static_cast<int>(type)) {}

// IDA_EA: 0x826D2AA
// IDA_NAME: _ZN10QuickParty17EquipItemDropRate7processENS_16RewardData_ParamERNS_17RewardData_ResultE
// DEMANGLED: QuickParty::EquipItemDropRate::process(
//     QuickParty::RewardData_Param,QuickParty::RewardData_Result &)
int EquipItemDropRate::process(RewardData_Param param,
                               RewardData_Result& result) {
    RandomRewardMap::iterator reward =
        FindRandomReward(*quick_party_data, random_buff_type);
    if (reward == quick_party_data->random_rewards.end()) {
        return 1;
    }

    const int row_index = param.party->get_member_count() - 2;
    if (row_index < 0 ||
        row_index >= static_cast<int>(reward->second.level_data_08.size())) {
        return row_index;
    }

    result.raw_14 =
        static_cast<float>(reward->second.level_data_08[row_index].raw_04) /
            100.0f +
        1.0f;
    return reinterpret_cast<intptr_t>(&result);
}

// IDA_EA: 0x826DCFA
// IDA_NAME: _ZN10QuickParty15CointLimitCountC2ENS_14RandomBuffTypeERNS_14QuickPartyDataE
CointLimitCount::CointLimitCount(RandomBuffType type, QuickPartyData& data)
    : IRandomwReward(data), random_buff_type(static_cast<int>(type)) {}

// IDA_EA: 0x826D4AC
// IDA_NAME: _ZN10QuickParty15CointLimitCount7processENS_16RewardData_ParamERNS_17RewardData_ResultE
int CointLimitCount::process(RewardData_Param param,
                             RewardData_Result& result) {
    RandomRewardMap::iterator reward =
        FindRandomReward(*quick_party_data, random_buff_type);
    if (reward == quick_party_data->random_rewards.end()) {
        return 1;
    }

    int row_index = 0;
    if (HasMemberLevelRow(reward->second.level_data_08, param.party,
                          row_index)) {
        result.raw_00 -= reward->second.level_data_08[row_index].raw_04;
        return param.party->increase_dungeon_coin_count(result.raw_00);
    }
    return row_index;
}

// IDA_EA: 0x826DD26
// IDA_NAME: _ZN10QuickParty26ReturnAncientDungeonTicketC2ENS_14RandomBuffTypeERNS_14QuickPartyDataE
ReturnAncientDungeonTicket::ReturnAncientDungeonTicket(RandomBuffType type,
                                                       QuickPartyData& data)
    : IRandomwReward(data), random_buff_type(static_cast<int>(type)) {}

// IDA_EA: 0x826D598
// IDA_NAME: _ZN10QuickParty26ReturnAncientDungeonTicket7processENS_16RewardData_ParamERNS_17RewardData_ResultE
int ReturnAncientDungeonTicket::process(RewardData_Param param,
                                        RewardData_Result& result) {
    (void)result;
    return param.party->increase_ancient_dungeon_ticket();
}

// IDA_EA: 0x826D6B0
// IDA_NAME: _ZN10QuickParty24CQuickPartyRewardManager4initEv
int CQuickPartyRewardManager::init() {
    if (importQuickPartyScript("Etc/QuickParty.etc", this) == 1) {
        reward_type_objects[0][11] = new RewardCardGold(
            static_cast<BasicRewardType>(11), *this);
        reward_type_objects[0][12] = new HellPartyDifficulty(
            static_cast<BasicRewardType>(12), *this);
        reward_type_objects[1][0] = new KillMonsterExp(
            static_cast<RandomBuffType>(0), *this);
        reward_type_objects[1][3] = new EquipItemDropRate(
            static_cast<RandomBuffType>(3), *this);
        reward_type_objects[1][9] = new CointLimitCount(
            static_cast<RandomBuffType>(9), *this);
        reward_type_objects[1][10] = new ReturnAncientDungeonTicket(
            static_cast<RandomBuffType>(10), *this);
        set_rand_buff_total_prob();
        return 1;
    }

    cMyTrace trace("bool QuickParty::CQuickPartyRewardManager::init()", 146,
                   0);
    trace("QUICK_PARTY_LOG : QuickParty.etc FILE LOAD FAIL!!");
    return 0;
}

}  // namespace QuickParty
