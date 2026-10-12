#include "old_game/social/party/party.h"

#include "common/config.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/player/core/cuser.h"
#include "infra/log/format/trace.h"

// IDA_EA: 0x8151A70
// IDA_NAME: _ZN6CParty17getMemberLevelGapEv
// DEMANGLED: CParty::getMemberLevelGap(void)
int CParty::getMemberLevelGap() {
    return _getMemberLevelGap();
}

// IDA_EA: 0x822D81E
// IDA_NAME: _ZN6CParty17SetRecvResultFlagEb
// DEMANGLED: CParty::SetRecvResultFlag(bool)
CParty* CParty::SetRecvResultFlag(bool received) {
    recv_result_flag_ = received;
    return this;
}

// IDA_EA: 0x822D852
// IDA_NAME: _ZN6CParty12setDungIndexEt
// DEMANGLED: CParty::setDungIndex(unsigned short)
CParty* CParty::setDungIndex(unsigned short dungeon_index) {
    dungeon_index_ = dungeon_index;
    return this;
}

// IDA_EA: 0x822D870
// IDA_NAME: _ZN6CParty12setDungDiffiEh
// DEMANGLED: CParty::setDungDiffi(unsigned char)
CParty* CParty::setDungDiffi(unsigned char dungeon_difficulty) {
    dungeon_difficulty_ = dungeon_difficulty;
    return this;
}

// IDA_EA: 0x822D88C
// IDA_NAME: _ZN6CParty13IsAutoCreatedEv
// DEMANGLED: CParty::IsAutoCreated(void)
int CParty::IsAutoCreated() {
    return auto_created_;
}

// IDA_EA: 0x822D89C
// IDA_NAME: _ZN6CParty23get_dungeon_clear_stateEv
// DEMANGLED: CParty::get_dungeon_clear_state(void)
int CParty::get_dungeon_clear_state() {
    return dungeon_clear_state_;
}

// IDA_EA: 0x822D8F6
// IDA_NAME: _ZN6CParty13checkBossRoomEv
// DEMANGLED: CParty::checkBossRoom(void)
bool CParty::checkBossRoom() {
    return battle_field_.check_end_point();
}

// IDA_EA: 0x822D962
// IDA_NAME: _ZN6CParty23set_gm_random_buff_typeEN10QuickParty14RandomBuffTypeE
// DEMANGLED: CParty::set_gm_random_buff_type(QuickParty::RandomBuffType)
CParty* CParty::set_gm_random_buff_type(
    QuickParty::RandomBuffType buff_type) {
    gm_random_buff_type_ = static_cast<int>(buff_type);
    return this;
}

// IDA_EA: 0x826525E
// IDA_NAME: _ZN6CParty14checkStartRoomEv
// DEMANGLED: CParty::checkStartRoom(void)
bool CParty::checkStartRoom() {
    return battle_field_.check_start_point();
}

// IDA_EA: 0x826B960
// IDA_NAME: _ZN6CParty13SetSinglePlayEb
// DEMANGLED: CParty::SetSinglePlay(bool)
CParty* CParty::SetSinglePlay(bool single_play) {
    single_play_ = single_play;
    return this;
}

// IDA_EA: 0x826B990
// IDA_NAME: _ZN6CParty14SetAutoCreatedEb
// DEMANGLED: CParty::SetAutoCreated(bool)
CParty* CParty::SetAutoCreated(bool auto_created) {
    auto_created_ = auto_created;
    return this;
}

// IDA_EA: 0x827AB3A
// IDA_NAME: _ZN6CParty15IsFirstMapClearEv
// DEMANGLED: CParty::IsFirstMapClear(void)
int CParty::IsFirstMapClear() {
    return first_map_clear_;
}

// IDA_EA: 0x827AB4A
// IDA_NAME: _ZN6CParty16SetFirstMapClearEb
// DEMANGLED: CParty::SetFirstMapClear(bool)
CParty* CParty::SetFirstMapClear(bool first_map_clear) {
    first_map_clear_ = first_map_clear;
    return this;
}

// IDA_EA: 0x82F07A4
// IDA_NAME: _ZN6CParty11IsExistUserEP5CUser
// DEMANGLED: CParty::IsExistUser(CUser *)
int CParty::IsExistUser(CUser* user) {
    for (int seat = 0; seat <= 3; ++seat) {
        if (members_[seat].user == user) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x82F07EC
// IDA_NAME: _ZN6CParty20getStraightVictoriesEv
// DEMANGLED: CParty::getStraightVictories(void)
int CParty::getStraightVictories() {
    return straight_victories_;
}

// IDA_EA: 0x82F07FC
// IDA_NAME: _ZN6CParty20incStraightVictoriesEv
// DEMANGLED: CParty::incStraightVictories(void)
CParty* CParty::incStraightVictories() {
    ++straight_victories_;
    return this;
}

// IDA_EA: 0x82F0818
// IDA_NAME: _ZN6CParty22resetStraightVictoriesEv
// DEMANGLED: CParty::resetStraightVictories(void)
CParty* CParty::resetStraightVictories() {
    straight_victories_ = 0;
    return this;
}

// IDA_EA: 0x830ED3A
// IDA_NAME: _ZN6CParty20get_random_buff_typeEv
// DEMANGLED: CParty::get_random_buff_type(void)
int CParty::get_random_buff_type() {
    return random_buff_type_;
}

// IDA_EA: 0x84B426E
// IDA_NAME: _ZN6CParty20setStraightVictoriesEt
// DEMANGLED: CParty::setStraightVictories(unsigned short)
CParty* CParty::setStraightVictories(unsigned short victories) {
    straight_victories_ = victories;
    return this;
}

// IDA_EA: 0x84B428C
// IDA_NAME: _ZN6CParty19setDungeonMapSavingEb
// DEMANGLED: CParty::setDungeonMapSaving(bool)
CParty* CParty::setDungeonMapSaving(bool saving) {
    dungeon_map_saving_ = saving;
    return this;
}

// IDA_EA: 0x8550FEE
// IDA_NAME: _ZNK6CParty36Get_party_overlapped_drop_ratio_rareEv
// DEMANGLED: CParty::Get_party_overlapped_drop_ratio_rare(void)const
int CParty::Get_party_overlapped_drop_ratio_rare() const {
    return party_overlapped_drop_ratio_rare_;
}

// IDA_EA: 0x8550FFC
// IDA_NAME: _ZNK6CParty38Get_party_overlapped_drop_ratio_uniqueEv
// DEMANGLED: CParty::Get_party_overlapped_drop_ratio_unique(void)const
int CParty::Get_party_overlapped_drop_ratio_unique() const {
    return party_overlapped_drop_ratio_unique_;
}

// IDA_EA: 0x859A1AE
// IDA_NAME: _ZN6CParty15get_total_levelEv
// DEMANGLED: CParty::get_total_level(void)
int CParty::get_total_level() {
    int total_level = 0;
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) != 0u) {
            total_level += members_[seat].user->get_charac_level();
        }
    }
    return total_level;
}

// IDA_EA: 0x859A318
// IDA_NAME: _ZN6CParty28CanDoPartyActionAssalutStateEv
// DEMANGLED: CParty::CanDoPartyActionAssalutState(void)
bool CParty::CanDoPartyActionAssalutState() {
    const int assault_state = static_cast<signed char>(GetAssaultState());
    return assault_state <= 1 || assault_state > 6;
}

// IDA_EA: 0x859A8BC
// IDA_NAME: _ZN6CParty14get_live_countEv
// DEMANGLED: CParty::get_live_count(void)
int CParty::get_live_count() {
    int live_count = 0;
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) == 1u &&
            battle_data_.member_live(seat) != 0) {
            ++live_count;
        }
    }
    return live_count;
}

// IDA_EA: 0x859A918
// IDA_NAME: _ZN6CParty24get_live_count_enter_mapEv
// DEMANGLED: CParty::get_live_count_enter_map(void)
int CParty::get_live_count_enter_map() {
    int live_count = 0;
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) == 1u &&
            battle_data_.member_enter_map(seat) != 0) {
            ++live_count;
        }
    }
    return live_count;
}

// IDA_EA: 0x85A2320
// IDA_NAME: _ZNK6CParty20GetExpPenaltyPercentEii
// DEMANGLED: CParty::GetExpPenaltyPercent(int,int)const
int CParty::GetExpPenaltyPercent(int character_level,
                                 int monster_level) const {
    if (character_level > 19) {
        if (character_level < monster_level) {
            return 100 * character_level / monster_level;
        }
        const unsigned int gap = character_level - monster_level;
        if (gap > 5u && gap <= 9u) {
            return 20 * (10 - static_cast<int>(gap));
        }
        if (gap > 9u) {
            return 5;
        }
    } else {
        const int difference = character_level - monster_level;
        const unsigned int gap = difference < 0
                                     ? static_cast<unsigned int>(-difference)
                                     : static_cast<unsigned int>(difference);
        if (gap > 5u && gap <= 9u) {
            return 20 * (10 - static_cast<int>(gap));
        }
        if (gap > 9u) {
            return 5;
        }
    }
    return 100;
}

// IDA_EA: 0x85AD9B4
// IDA_NAME: _ZN6CParty25calDeadUserDungeonPenaltyEi
// DEMANGLED: CParty::calDeadUserDungeonPenalty(int)
long double CParty::calDeadUserDungeonPenalty(int seat) {
    float penalty = 0.0f;
    const int map_move_count = battle_data_.map_move_count();
    if (seat >= 0) {
        if (map_move_count > 1) {
            penalty = static_cast<long double>(
                          battle_data_.member_map_move_count(seat)) /
                      static_cast<long double>(map_move_count - 1);
            if (penalty < 0.30000001) {
                return 0.0L;
            }
        }
        return penalty;
    }
    return 0.0L;
}

// IDA_EA: 0x85B231E
// IDA_NAME: _ZN6CParty21GetSetPlayResultCountEv
// DEMANGLED: CParty::GetSetPlayResultCount(void)
int CParty::GetSetPlayResultCount() {
    int result_count = 0;
    for (int seat = 0; seat <= 3; ++seat) {
        if (members_[seat].user != 0 && members_[seat].state != 0) {
            ++result_count;
        }
    }
    return result_count;
}

// IDA_EA: 0x85B401C
// IDA_NAME: _ZN6CParty23HaveAlreadySelectedCardEc22eClearRewardCardType_t
// DEMANGLED: CParty::HaveAlreadySelectedCard(char,eClearRewardCardType_t)
int CParty::HaveAlreadySelectedCard(char card,
                                    eClearRewardCardType_t card_type) {
    const int type = static_cast<int>(card_type);
    if (type > 1) {
        return 0;
    }
    for (int seat = 0; seat <= 3; ++seat) {
        if (selected_cards_[type][seat] == static_cast<unsigned char>(card)) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x85B4734
// IDA_NAME: _ZN6CParty12IsSelectCardEc22eClearRewardCardType_t
// DEMANGLED: CParty::IsSelectCard(char,eClearRewardCardType_t)
int CParty::IsSelectCard(char card, eClearRewardCardType_t card_type) {
    const int type = static_cast<int>(card_type);
    for (int seat = 0; seat <= 3; ++seat) {
        if (selected_cards_[type][seat] == static_cast<unsigned char>(card)) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x85B4786
// IDA_NAME: _ZN6CParty18CompleteCardSelectEv
// DEMANGLED: CParty::CompleteCardSelect(void)
bool CParty::CompleteCardSelect() {
    for (int card = 0; card <= 3; ++card) {
        if (selected_cards_[0][card] != 0xffu) {
            continue;
        }

        for (int member = 0; member <= 3; ++member) {
            if (static_cast<unsigned char>(_checkValidUser(member)) == 1u &&
                static_cast<unsigned char>(
                    IsSelectCard(static_cast<char>(member),
                                 static_cast<eClearRewardCardType_t>(0))) !=
                    1u) {
                selected_cards_[0][card] = static_cast<uint8_t>(member);
                if (selected_cards_[0][card] == 0xffu) {
                    LogManager::logFormat(
                        1,
                        "party.cpp",
                        "void CParty::CompleteCardSelect()",
                        0x36f8,
                        "INIT_CARD_NUM != dungeon_reward_card_[i]===>> "
                        "dungeon_reward_card_[i] = %d, i = %d, z = %d",
                        selected_cards_[0][card],
                        card,
                        member);
                }
                break;
            }
        }
    }
    // The original loop's terminal comparison is false after card == 4.
    return false;
}

// IDA_EA: 0x85B489C
// IDA_NAME: _ZN6CParty29IsCompleteCardSelectOfMembersE22eClearRewardCardType_t
// DEMANGLED: CParty::IsCompleteCardSelectOfMembers(eClearRewardCardType_t)
int CParty::IsCompleteCardSelectOfMembers(
    eClearRewardCardType_t card_type) {
    const int type = static_cast<int>(card_type);
    if (type < 0 || type > 2) {
        return 0;
    }

    const int first_group = type == 2 ? 0 : type;
    const int last_group = type == 2 ? 1 : type;
    for (int group = first_group; group <= last_group; ++group) {
        int owner_count[4] = {0, 0, 0, 0};
        for (int card = 0; card <= 3; ++card) {
            const unsigned int owner = selected_cards_[group][card];
            if (owner < 4u && _checkValidUser(static_cast<int>(owner)) == 1) {
                ++owner_count[owner];
            }
        }
        for (int member = 0; member <= 3; ++member) {
            if (_checkValidUser(member) == 1 && owner_count[member] != 1) {
                return 0;
            }
        }
    }
    return 1;
}

// IDA_EA: 0x85BB5B0
// IDA_NAME: _ZN6CParty15isCheckUseSkillEii
// DEMANGLED: CParty::isCheckUseSkill(int,int)
bool CParty::isCheckUseSkill(int character_job, int skill_index) {
    return skill_index <= 168 &&
           (character_job != oldsrv::config::kCharacterJobSwordman ||
            skill_index != 46) &&
           (character_job != oldsrv::config::kCharacterJobFighter ||
            skill_index != 5) &&
           (character_job != oldsrv::config::kCharacterJobGunner ||
            skill_index != 4) &&
           (character_job != oldsrv::config::kCharacterJobMage ||
            skill_index != 12) &&
           (character_job != oldsrv::config::kCharacterJobPriest ||
            skill_index != 1) &&
           (character_job != oldsrv::config::kCharacterJobAtGunner ||
            skill_index != 4) &&
           (character_job != oldsrv::config::kCharacterJobThief ||
            skill_index != 1) &&
           (character_job != oldsrv::config::kCharacterJobAtFighter ||
            skill_index != 5);
}

// IDA_EA: 0x85BCA8A
// IDA_NAME: _ZN6CParty15SetWeekendEventEb
// DEMANGLED: CParty::SetWeekendEvent(bool)
CParty* CParty::SetWeekendEvent(bool active) {
    weekend_event_ = active;
    return this;
}

// IDA_EA: 0x85BCAA6
// IDA_NAME: _ZN6CParty14IsWeekendEventEv
// DEMANGLED: CParty::IsWeekendEvent(void)
int CParty::IsWeekendEvent() {
    return weekend_event_;
}

// IDA_EA: 0x85BD6AC
// IDA_NAME: _ZN6CParty30GetTournamentDungeonClearStateEv
// DEMANGLED: CParty::GetTournamentDungeonClearState(void)
int CParty::GetTournamentDungeonClearState() {
    return tournament_dungeon_clear_state_;
}

// IDA_EA: 0x85BD79E
// IDA_NAME: _ZN6CParty19isTournamentVictoryEv
// DEMANGLED: CParty::isTournamentVictory(void)
int CParty::isTournamentVictory() {
    return tournament_victory_;
}

// IDA_EA: 0x85BE54C
// IDA_NAME: _ZN6CParty23GetPartyMemberCoinLimitEv
// DEMANGLED: CParty::GetPartyMemberCoinLimit(void)
int CParty::GetPartyMemberCoinLimit() {
    return party_member_coin_limit_;
}

// IDA_EA: 0x85BE55A
// IDA_NAME: _ZN6CParty23SetPartyMemberCoinLimitEi
// DEMANGLED: CParty::SetPartyMemberCoinLimit(int)
CParty* CParty::SetPartyMemberCoinLimit(int coin_limit) {
    party_member_coin_limit_ = coin_limit;
    return this;
}

// IDA_EA: 0x85BF6F8
// IDA_NAME: _ZN6CParty22IsPremiumGoldCardPartyEv
// DEMANGLED: CParty::IsPremiumGoldCardParty(void)
int CParty::IsPremiumGoldCardParty() {
    return premium_gold_card_party_;
}

// IDA_EA: 0x85BF708
// IDA_NAME: _ZN6CParty23SetPremiumGoldCardPartyEv
// DEMANGLED: CParty::SetPremiumGoldCardParty(void)
CParty* CParty::SetPremiumGoldCardParty() {
    premium_gold_card_party_ = 1;
    return this;
}

// IDA_EA: 0x85BF728
// IDA_NAME: _ZN6CParty29GetPremiumGoldCardDefaultItemEv
// DEMANGLED: CParty::GetPremiumGoldCardDefaultItem(void)
int CParty::GetPremiumGoldCardDefaultItem() {
    return premium_gold_card_default_item_;
}

// IDA_EA: 0x85BF736
// IDA_NAME: _ZN6CParty29SetPremiumGoldCardDefaultItemEj
// DEMANGLED: CParty::SetPremiumGoldCardDefaultItem(unsigned int)
CParty* CParty::SetPremiumGoldCardDefaultItem(unsigned int item_id) {
    premium_gold_card_default_item_ = item_id;
    return this;
}

// IDA_EA: 0x85BF75A
// IDA_NAME: _ZN6CParty22setStartGamePartyCountEi
// DEMANGLED: CParty::setStartGamePartyCount(int)
CParty* CParty::setStartGamePartyCount(int member_count) {
    start_game_party_count_ = member_count;
    return this;
}

// IDA_EA: 0x85BF76C
// IDA_NAME: _ZN6CParty22getStartGamePartyCountEv
// DEMANGLED: CParty::getStartGamePartyCount(void)
int CParty::getStartGamePartyCount() {
    return start_game_party_count_;
}

// IDA_EA: 0x85BF7A2
// IDA_NAME: _ZN6CParty13setTitleIndexEh
// DEMANGLED: CParty::setTitleIndex(unsigned char)
CParty* CParty::setTitleIndex(unsigned char title_index) {
    title_index_ = title_index;
    return this;
}

// IDA_EA: 0x85BF7BE
// IDA_NAME: _ZN6CParty10setUserMaxEh
// DEMANGLED: CParty::setUserMax(unsigned char)
CParty* CParty::setUserMax(unsigned char user_max) {
    // CLIENT_IDA_EVIDENCE: the ordinary-party CMD12 UI initializes this
    // value to four at 0x118E672 and sends it from object +0x1a0 at
    // 0x118E1DD..0x118E1E5.  NOTI9 installs eight generic client slots at
    // 0x5981C2..0x598221, but CParty owns only members_[4]; values above four
    // must not create a second ordinary-party capacity model.  The client
    // selector is stored as zero-based and sent as selector+1, so its valid
    // wire range is 1..4; clamp malformed zero as well as values above four.
    user_max_ = user_max < 1u ? 1u : (user_max > 4u ? 4u : user_max);
    return this;
}

// IDA_EA: 0x85BF81E
// IDA_NAME: _ZN6CParty15GetAssaultStateEv
// DEMANGLED: CParty::GetAssaultState(void)
int CParty::GetAssaultState() {
    return assault_state_;
}

// IDA_EA: 0x85BF83E
// IDA_NAME: _ZN6CParty16SetUsedCoinCountEi
// DEMANGLED: CParty::SetUsedCoinCount(int)
CParty* CParty::SetUsedCoinCount(int used_coin_count) {
    used_coin_count_ = used_coin_count;
    return this;
}

// IDA_EA: 0x860B5EE
// IDA_NAME: _ZN6CParty16GetRoutingItemIDEv
// DEMANGLED: CParty::GetRoutingItemID(void)
int CParty::GetRoutingItemID() {
    return routing_data_.GetRoutingItemIndex();
}
