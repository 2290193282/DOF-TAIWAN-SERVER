#include "old_game/social/party/party.h"

#include <vector>

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/server_utility.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"

// IDA_EA: 0x859B814
// IDA_NAME: _ZN6CParty18choose_random_buffEN10QuickParty17RandomBuffDungeonE
// DEMANGLED: CParty::choose_random_buff(QuickParty::RandomBuffDungeon)
int CParty::choose_random_buff(QuickParty::RandomBuffDungeon dungeon_type) {
    if (is_quick_party() != 1 || static_cast<int>(dungeon_type) > 6) {
        return 11;
    }

    QuickParty::CQuickPartyRewardManager* const rewards =
        G_CGameManager()->GetQuickPartyRewardManager();
    const int total_probability = rewards->get_random_buff_prob(
        static_cast<QuickParty::RandomBuffType>(11), dungeon_type);
    if (total_probability == 0) {
        return 11;
    }

    const int random_value = static_cast<int>(get_rand_int(total_probability));
    int accumulated_probability = 0;
    for (int buff_type = 0; buff_type <= 10; ++buff_type) {
        accumulated_probability += rewards->get_random_buff_prob(
            static_cast<QuickParty::RandomBuffType>(buff_type), dungeon_type);
        if (random_value <= accumulated_probability) {
            random_buff_type_ = buff_type;
            return random_buff_type_;
        }
    }
    return 11;
}

// IDA_EA: 0x859B8FA
// IDA_NAME: _ZN6CParty17set_random_rewardEN10QuickParty14RandomBuffTypeENS0_16RewardData_ParamERNS0_17RewardData_ResultE
// DEMANGLED: CParty::set_random_reward(QuickParty::RandomBuffType,QuickParty::RewardData_Param,QuickParty::RewardData_Result &)
int CParty::set_random_reward(QuickParty::RandomBuffType reward_type,
                              QuickParty::RewardData_Param parameter,
                              QuickParty::RewardData_Result& result) {
    int status = is_quick_party() ^ 1;
    if (!status && random_buff_type_ != 11 &&
        random_buff_type_ == static_cast<int>(reward_type)) {
        QuickParty::IRewardType* const reward =
            static_cast<QuickParty::IRewardType*>(
                G_CGameManager()->GetQuickPartyRewardManager()
                    ->get_reward_type_object(
                        static_cast<QuickParty::ENUM_REWARD_TYPE>(1),
                        static_cast<unsigned int>(reward_type)));
        if (reward) {
            status = reward->process(parameter, result);
        }
    }
    return status;
}

// IDA_EA: 0x859B992
// IDA_NAME:
// _ZN6CParty16set_basic_rewardEN10QuickParty15BasicRewardTypeENS0_16RewardData_ParamERNS0_17RewardData_ResultE
// DEMANGLED: CParty::set_basic_reward(QuickParty::BasicRewardType,
//             QuickParty::RewardData_Param,QuickParty::RewardData_Result &)
char CParty::set_basic_reward(QuickParty::BasicRewardType reward_type,
                              QuickParty::RewardData_Param parameter,
                              QuickParty::RewardData_Result& result) {
    char status = static_cast<char>(is_quick_party() ^ 1);
    if (!status) {
        const std::vector<int>* dungeon_idx_list =
            G_CGameManager()->GetQuickPartyRewardManager()
                ->get_dungeon_idx_list();
        std::vector<int>::const_iterator current = dungeon_idx_list->begin();

        dungeon_idx_list = G_CGameManager()->GetQuickPartyRewardManager()
                               ->get_dungeon_idx_list();
        const std::vector<int>::const_iterator end = dungeon_idx_list->end();
        while (current != end) {
            status = static_cast<char>(parameter.raw_04 == *current);
            if (status) {
                return status;
            }
            ++current;
        }

        QuickParty::IRewardType* const reward =
            static_cast<QuickParty::IRewardType*>(
                G_CGameManager()->GetQuickPartyRewardManager()
                    ->get_reward_type_object(
                        static_cast<QuickParty::ENUM_REWARD_TYPE>(0),
                        static_cast<unsigned int>(reward_type)));
        if (reward) {
            status = static_cast<char>(reward->process(parameter, result));
        }
    }
    return status;
}
