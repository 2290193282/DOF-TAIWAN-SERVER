#include "old_game/pvp/battlefield/battle_field.h"

#include <algorithm>

#include "infra/network/packet/packet_guard.h"

namespace {

bool ContainsMember(const std::vector<int>& members, int member_slot) {
    return std::find(members.begin(), members.end(), member_slot) !=
           members.end();
}

}  // namespace

// IDA_EA: 0x8284D08
// IDA_NAME: _ZN24CTournamentDungeonReward20isCardSelectCompleteE32ENUM_TOURNAMENT_REWARD_CARD_TYPERKSt6vectorIiSaIiEE
// DEMANGLED: CTournamentDungeonReward::isCardSelectComplete(
//     ENUM_TOURNAMENT_REWARD_CARD_TYPE,std::vector<int,std::allocator<int>> const&)
bool CTournamentDungeonReward::isCardSelectComplete(
    ENUM_TOURNAMENT_REWARD_CARD_TYPE card_type,
    const std::vector<int>& valid_members) {
    if (valid_members.empty()) {
        return false;
    }

    if (card_type == ENUM_TOURNAMENT_REWARD_CARD_TYPE_ALL) {
        for (int card_group = 0; card_group <= 1; ++card_group) {
            for (int reward_kind = 0; reward_kind <= 1; ++reward_kind) {
                if (!ContainsMember(
                        valid_members,
                        static_cast<int>(
                            rewards_[card_group][reward_kind].reward_card_type))) {
                    return false;
                }
            }
        }
        return true;
    }

    for (int reward_kind = 0; reward_kind <= 1; ++reward_kind) {
        if (!ContainsMember(
                valid_members,
                static_cast<int>(
                    rewards_[static_cast<int>(card_type)][reward_kind]
                        .reward_card_type))) {
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x8284EE4
// IDA_NAME: _ZN24CTournamentDungeonReward10SelectCardEicc
// DEMANGLED: CTournamentDungeonReward::SelectCard(int,char,char)
bool CTournamentDungeonReward::SelectCard(int member_slot,
                                          char card_group,
                                          char reward_kind) {
    if (card_group < 0 || card_group >= 2 || reward_kind < 0 ||
        reward_kind >= 2 || member_slot < 0 || member_slot >= 4) {
        return false;
    }

    TournamentRewardType& reward =
        rewards_[static_cast<int>(card_group)][static_cast<int>(reward_kind)];
    if (reward.reward_card_type != -1) {
        return false;
    }

    reward.reward_card_type = static_cast<int8_t>(member_slot);
    return true;
}

// IDA_EA: 0x8284FA4
// IDA_NAME: _ZN24CTournamentDungeonReward16MakeSelectPacketEP11PacketGuard
// DEMANGLED: CTournamentDungeonReward::MakeSelectPacket(PacketGuard *)
bool CTournamentDungeonReward::MakeSelectPacket(PacketGuard* packet) {
    for (int card_group = 0; card_group <= 1; ++card_group) {
        packet->put_byte(2);
        for (int reward_kind = 0; reward_kind <= 1; ++reward_kind) {
            packet->put_byte(
                rewards_[card_group][reward_kind].reward_card_type);
        }
    }
    return false;
}

// IDA_EA: 0x8285284
// IDA_NAME: _ZN24CTournamentDungeonReward18isMemberSelectCardE32ENUM_TOURNAMENT_REWARD_CARD_TYPEi
// DEMANGLED: CTournamentDungeonReward::isMemberSelectCard(
//     ENUM_TOURNAMENT_REWARD_CARD_TYPE,int)
bool CTournamentDungeonReward::isMemberSelectCard(
    ENUM_TOURNAMENT_REWARD_CARD_TYPE card_type,
    int member_slot) {
    for (int reward_kind = 0; reward_kind <= 1; ++reward_kind) {
        if (rewards_[static_cast<int>(card_type)][reward_kind]
                .reward_card_type == member_slot) {
            return true;
        }
    }
    return false;
}

// IDA_EA: 0x828523C
// IDA_NAME: _ZN24CTournamentDungeonReward15ForceSelectCardERSt6vectorIiSaIiEEb
// DEMANGLED: CTournamentDungeonReward::ForceSelectCard(
//     std::vector<int,std::allocator<int>> &,bool)
bool CTournamentDungeonReward::ForceSelectCard(std::vector<int>& valid_members,
                                               bool tournament_victory) {
    bool result = _ForceSelectCard(ENUM_TOURNAMENT_REWARD_CARD_TYPE_0,
                                   valid_members);
    if (tournament_victory) {
        result = _ForceSelectCard(ENUM_TOURNAMENT_REWARD_CARD_TYPE_1,
                                  valid_members);
    }
    return result;
}

// IDA_EA: 0x82853B8
// IDA_NAME: _ZN24CTournamentDungeonReward16_ForceSelectCardE32ENUM_TOURNAMENT_REWARD_CARD_TYPERKSt6vectorIiSaIiEE
// DEMANGLED: CTournamentDungeonReward::_ForceSelectCard(
//     ENUM_TOURNAMENT_REWARD_CARD_TYPE,std::vector<int,std::allocator<int>> const&)
bool CTournamentDungeonReward::_ForceSelectCard(
    ENUM_TOURNAMENT_REWARD_CARD_TYPE card_type,
    const std::vector<int>& valid_members) {
    bool result = static_cast<int>(card_type) != 0;
    if (static_cast<int>(card_type) > 1) {
        return result;
    }

    for (int reward_kind = 0; reward_kind <= 1; ++reward_kind) {
        if (rewards_[static_cast<int>(card_type)][reward_kind]
                .reward_card_type != -1) {
            continue;
        }
        for (std::vector<int>::const_iterator member = valid_members.begin();
             member != valid_members.end();
             ++member) {
            if (!isMemberSelectCard(card_type, *member)) {
                // The IDA loop intentionally continues after this assignment;
                // the last eligible member therefore wins the forced choice.
                rewards_[static_cast<int>(card_type)][reward_kind]
                    .reward_card_type = static_cast<int8_t>(*member);
            }
        }
    }
    return result;
}
