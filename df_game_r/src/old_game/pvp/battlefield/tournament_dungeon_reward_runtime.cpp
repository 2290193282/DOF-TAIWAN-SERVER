#include "old_game/pvp/battlefield/battle_field.h"

#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/server_utility.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/data/item_generator_types.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x8284890
// IDA_NAME: _ZN24CTournamentDungeonReward10MakeRewardEPKSt3mapIi20TournamentRewardRateSt4lessIiESaISt4pairIKiS1_EEEPK24TournamentRewardItemRateib
// DEMANGLED: CTournamentDungeonReward::MakeReward(
//     std::map<int,TournamentRewardRate> const*,TournamentRewardItemRate const*,int,bool)
int CTournamentDungeonReward::MakeReward(
    const std::map<int, TournamentRewardRate>* reward_rates,
    const TournamentRewardItemRate* reward_items,
    int member_count,
    bool tournament_victory) {
    bool result = tournament_victory;
    if (!reward_rates || !reward_items) {
        return result;
    }

    int reward_rate_index = tournament_victory ? member_count : member_count - 1;
    if (reward_rate_index < 0) {
        reward_rate_index = 0;
    } else if (reward_rate_index > 4) {
        reward_rate_index = 4;
    }

    std::map<int, TournamentRewardRate>::const_iterator rate =
        reward_rates->find(reward_rate_index);
    if (rate == reward_rates->end()) {
        return result;
    }

    // MakeReward's public ABI accepts a const map, while the original helper
    // ABI takes a non-const reference.  DecisionRewardType only reads the
    // rate record, so preserve both contracts without casting away constness.
    TournamentRewardRate selected_rate = rate->second;
    result = DecisionRewardType(
        ENUM_TOURNAMENT_REWARD_CARD_TYPE_0, selected_rate, reward_items);
    if (member_count == 4 && tournament_victory) {
        rate = reward_rates->find(5);
        if (rate != reward_rates->end()) {
            TournamentRewardRate victory_rate = rate->second;
            return DecisionRewardType(
                ENUM_TOURNAMENT_REWARD_CARD_TYPE_1, victory_rate, reward_items);
        }
    }
    return result;
}

// IDA_EA: 0x8284A24
// IDA_NAME: _ZN24CTournamentDungeonReward18DecisionRewardTypeE32ENUM_TOURNAMENT_REWARD_CARD_TYPER20TournamentRewardRatePK24TournamentRewardItemRate
// DEMANGLED: CTournamentDungeonReward::DecisionRewardType(
//     ENUM_TOURNAMENT_REWARD_CARD_TYPE,TournamentRewardRate &,TournamentRewardItemRate const*)
bool CTournamentDungeonReward::DecisionRewardType(
    ENUM_TOURNAMENT_REWARD_CARD_TYPE card_type,
    TournamentRewardRate& reward_rate,
    const TournamentRewardItemRate* reward_items) {
    if (!reward_items) {
        return false;
    }

    for (int reward_kind = 0; reward_kind <= 1; ++reward_kind) {
        const int random_value =
            static_cast<int>(get_rand_int(reward_rate.total_rate + 1));
        TournamentRewardType& reward =
            rewards_[static_cast<int>(card_type)][reward_kind];
        if (reward_rate.gold_reward_rate >= random_value) {
            reward.first_dword = 0;
        } else if (reward_rate.gold_reward_rate +
                       reward_rate.item_reward_rate >=
                   random_value) {
            const int item_random =
                static_cast<int>(get_rand_int(reward_items->total_rate)) + 1;
            int accumulated_rate = 0;
            for (std::vector<TournamentRewardItem>::const_iterator item =
                     reward_items->reward_items.begin();
                 item != reward_items->reward_items.end();
                 ++item) {
                accumulated_rate += item->selection_rate;
                if (item_random <= accumulated_rate) {
                    reward.first_dword = item->item_id;
                    reward.second_dword = item->item_count;
                    break;
                }
            }
        }
    }

    // The original loop leaves EAX false after its terminal condition.
    return false;
}

// IDA_EA: 0x8284B8E
// IDA_NAME: _ZN24CTournamentDungeonReward13SetRewardGoldERN8WongWork18stGenerateResult_tEf
// DEMANGLED: CTournamentDungeonReward::SetRewardGold(WongWork::stGenerateResult_t &,float)
bool CTournamentDungeonReward::SetRewardGold(
    WongWork::stGenerateResult_t& generated,
    float gold_scale) {
    for (int card_type = 0; card_type <= 1; ++card_type) {
        for (int reward_kind = 0; reward_kind <= 1; ++reward_kind) {
            TournamentRewardType& reward = rewards_[card_type][reward_kind];
            if (reward.first_dword == 0 && !generated.items.empty()) {
                reward.second_dword = static_cast<int>(
                    static_cast<long double>(generated.items.front().add_info_) *
                    gold_scale);
            }
        }
    }

    // As at 0x8284B8E, the loop's final condition supplies a false result.
    return false;
}

// IDA_EA: 0x8284CAA
// IDA_NAME: _ZN24CTournamentDungeonReward16MakeRewardPacketEP11PacketGuardb
// DEMANGLED: CTournamentDungeonReward::MakeRewardPacket(PacketGuard *,bool)
int CTournamentDungeonReward::MakeRewardPacket(PacketGuard* packet,
                                                bool tournament_victory) {
    _MakeRewardPacket(ENUM_TOURNAMENT_REWARD_CARD_TYPE_0, packet);
    if (tournament_victory) {
        return _MakeRewardPacket(ENUM_TOURNAMENT_REWARD_CARD_TYPE_1, packet);
    }
    return packet->put_byte(0);
}

// IDA_EA: 0x828502E
// IDA_NAME: _ZN24CTournamentDungeonReward15GetItemFromCardE32ENUM_TOURNAMENT_REWARD_CARD_TYPEiP10Inven_Itemj
// DEMANGLED: CTournamentDungeonReward::GetItemFromCard(
//     ENUM_TOURNAMENT_REWARD_CARD_TYPE,int,Inven_Item *,unsigned int)
int CTournamentDungeonReward::GetItemFromCard(
    ENUM_TOURNAMENT_REWARD_CARD_TYPE card_type,
    int member_slot,
    Inven_Item* output_items,
    unsigned int output_item_count) {
    if (!output_items || output_item_count > 2 ||
        static_cast<int>(card_type) < 0 || static_cast<int>(card_type) >= 2) {
        return false;
    }

    TournamentRewardType* selected_reward = 0;
    for (int reward_kind = 0; reward_kind <= 1; ++reward_kind) {
        TournamentRewardType& reward =
            rewards_[static_cast<int>(card_type)][reward_kind];
        if (reward.reward_card_type == member_slot) {
            selected_reward = &reward;
            break;
        }
    }
    if (!selected_reward) {
        return false;
    }

    output_items[0].reset();
    output_items[1].reset();
    if (selected_reward->first_dword == -1) {
        return false;
    }
    if (selected_reward->first_dword == 0) {
        output_items[0].set_add_info(selected_reward->second_dword);
        return true;
    }

    output_items[1].item_id_ = selected_reward->first_dword;
    G_CDataManager()->get_item_list()->create_item(
        static_cast<ENUM_ITEM_CREATE_TYPE>(1),
        output_items[1],
        selected_reward->second_dword);
    return true;
}

// IDA_EA: 0x82852E4
// IDA_NAME: _ZN24CTournamentDungeonReward17_MakeRewardPacketE32ENUM_TOURNAMENT_REWARD_CARD_TYPEP11PacketGuard
// DEMANGLED: CTournamentDungeonReward::_MakeRewardPacket(
//     ENUM_TOURNAMENT_REWARD_CARD_TYPE,PacketGuard *)
char CTournamentDungeonReward::_MakeRewardPacket(
    ENUM_TOURNAMENT_REWARD_CARD_TYPE card_type,
    PacketGuard* packet) {
    if (static_cast<int>(card_type) > 1) {
        return static_cast<char>(card_type);
    }

    packet->put_byte(2);
    for (int reward_kind = 0; reward_kind <= 1; ++reward_kind) {
        const TournamentRewardType& reward =
            rewards_[static_cast<int>(card_type)][reward_kind];
        packet->put_int(reward.first_dword);
        packet->put_int(reward.second_dword);
        packet->put_short(0);
    }
    return false;
}
