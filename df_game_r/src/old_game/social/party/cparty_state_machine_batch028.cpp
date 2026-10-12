#include "old_game/social/party/party.h"

#include <cstdlib>
#include <cstring>
#include <stdint.h>

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/map/map_script_types.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/timer/timer_queue.h"

namespace {
const char kEmptyCharacName[] = "";

}  // namespace

// IDA_EA: 0x84B42A8
// IDA_NAME: _ZN6CParty20getPassedMapInfoListEv
// DEMANGLED: CParty::getPassedMapInfoList(void)
std::vector<MapInfo>* CParty::getPassedMapInfoList() {
    return &passed_map_info_list_;
}

// IDA_EA: 0x859A210
// IDA_NAME: _ZN6CParty20is_limit_minimum_ageEiRi
// DEMANGLED: CParty::is_limit_minimum_age(int,int &)
int CParty::is_limit_minimum_age(int minimum_age, int& unique_id) {
    for (int seat = 0; seat <= 3; ++seat) {
        if (_checkValidUser(seat) &&
            members_[seat].user->CheckLimitMiniumAge(minimum_age,
                                                      unique_id) != 1) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x859A830
// IDA_NAME: _ZN6CParty15ClearBadP2PUserEv
// DEMANGLED: CParty::ClearBadP2PUser(void)
char CParty::ClearBadP2PUser() {
    _ClearConnectP2PAbsolute(0);
    _ClearConnectP2PAbsolute(1);
    int maximum = 0;
    int minimum = 0;
    char result = 0;
    for (int connection_count = 2;; ++connection_count) {
        result = static_cast<char>(connection_count <= 3);
        if (connection_count > 3) {
            break;
        }
        _GetConnectP2PMaxMinPoint(maximum, minimum);
        result = static_cast<char>(minimum);
        if (maximum == minimum) {
            break;
        }
        _ClearConnectP2PRelative(minimum);
    }
    return result;
}

// IDA_EA: 0x859B234
// IDA_NAME: _ZN6CParty14SetPartyInfoUIERK18MSG_PARTY_SET_INFO
// DEMANGLED: CParty::SetPartyInfoUI(MSG_PARTY_SET_INFO const&)
CParty* CParty::SetPartyInfoUI(const MSG_PARTY_SET_INFO& message) {
    setTitle(message.title);
    setTitleIndex(message.title_index);
    setUserMax(message.user_max);
    setDungIndex(message.dungeon_index);
    return setDungDiffi(message.dungeon_difficulty);
}

// IDA_EA: 0x859BC58
// IDA_NAME: _ZN6CParty27increase_dungeon_coin_countEi
// DEMANGLED: CParty::increase_dungeon_coin_count(int)
int CParty::increase_dungeon_coin_count(int count) {
    CDungeon* const dungeon = battle_field_.dungeon_;
    if (!dungeon) {
        return 0;
    }
    const int dungeon_coin = dungeon->coin_limit_;
    if (dungeon_coin != -1) {
        return static_cast<int>(
            reinterpret_cast<intptr_t>(SetUsedCoinCount(count)));
    }
    return dungeon_coin;
}

// IDA_EA: 0x85A22A8
// IDA_NAME: _ZN6CParty17SelectEPLPCommandEi
// DEMANGLED: CParty::SelectEPLPCommand(int)
int CParty::SelectEPLPCommand(int delay_sec) {
    SetEPLPState(3);
    const unsigned int key = static_cast<unsigned int>(
        gen_timer_key(TIMER_MESSAGE_EPLP_RETURN_VILLAGE));
    return G_TimerQueue()->InsertTimer(
        1,
        GetPartyIndex(),
        TIMER_MESSAGE_EPLP_RETURN_VILLAGE,
        delay_sec,
        key,
        0);
}

// IDA_EA: 0x85A23DC
// IDA_NAME: _ZN6CParty18getMonsterTotalExpEjciffb
// DEMANGLED: CParty::getMonsterTotalExp(unsigned int,char,int,float,float,bool)
int CParty::getMonsterTotalExp(unsigned int base_exp,
                               char rank,
                               int level_index,
                               float level_weight,
                               float dungeon_weight,
                               bool triple_exp) {
    const ServerParameterScript* const parameters =
        G_CDataManager()->get_server_parameter_script();
    float rank_weight = rank > 3
                            ? 1.0f
                            : parameters->get_monster_kind_drop_bonus_rate(
                                  rank);
    if (triple_exp) {
        rank_weight *= 3.0f;
    }
    const float monster_level_weight =
        parameters->get_dungeon_difficulty_exp_bonus_rate(level_index);
    const long double exp =
        static_cast<long double>(base_exp) / 2.0L * dungeon_weight *
        monster_level_weight * level_weight * rank_weight;
    return static_cast<int>(static_cast<int64_t>(exp));
}

// IDA_EA: 0x85B210C
// IDA_NAME: _ZN6CParty29insert_time_dungeon_clear_msgEi
// DEMANGLED: CParty::insert_time_dungeon_clear_msg(int)
int CParty::insert_time_dungeon_clear_msg(int delay_sec) {
    const unsigned int key = static_cast<unsigned int>(
        gen_timer_key(TIMER_MESSAGE_DUNGEON_CLEAR));
    return G_TimerQueue()->InsertTimer(
        1, party_index_, TIMER_MESSAGE_DUNGEON_CLEAR, delay_sec, key, 0);
}

// IDA_EA: 0x85B216A
// IDA_NAME: _ZN6CParty27insert_time_card_select_msgEi
// DEMANGLED: CParty::insert_time_card_select_msg(int)
int CParty::insert_time_card_select_msg(int delay_sec) {
    const unsigned int key = static_cast<unsigned int>(
        gen_timer_key(TIMER_MESSAGE_CARD_SELECT));
    return G_TimerQueue()->InsertTimer(
        1, party_index_, TIMER_MESSAGE_CARD_SELECT, delay_sec, key, 0);
}

// IDA_EA: 0x85B21C8
// IDA_NAME: _ZN6CParty36insert_time_clear_dungeon_reward_msgEi
// DEMANGLED: CParty::insert_time_clear_dungeon_reward_msg(int)
int CParty::insert_time_clear_dungeon_reward_msg(int delay_sec) {
    const unsigned int key = static_cast<unsigned int>(
        gen_timer_key(TIMER_MESSAGE_CLEAR_DUNGEON_REWARD));
    return G_TimerQueue()->InsertTimer(
        1,
        party_index_,
        TIMER_MESSAGE_CLEAR_DUNGEON_REWARD,
        delay_sec,
        key,
        0);
}

// IDA_EA: 0x85B2226
// IDA_NAME: _ZN6CParty36insert_time_dungeon_score_scroll_msgEi
// DEMANGLED: CParty::insert_time_dungeon_score_scroll_msg(int)
int CParty::insert_time_dungeon_score_scroll_msg(int delay_sec) {
    const unsigned int key = static_cast<unsigned int>(
        gen_timer_key(TIMER_MESSAGE_CLEAR_DUNGEON_SCORE_SCROLL));
    return G_TimerQueue()->InsertTimer(
        1,
        party_index_,
        TIMER_MESSAGE_CLEAR_DUNGEON_SCORE_SCROLL,
        delay_sec,
        key,
        0);
}

// IDA_EA: 0x85B2284
// IDA_NAME: _ZN6CParty33insert_time_card_select_right_msgEi
// DEMANGLED: CParty::insert_time_card_select_right_msg(int)
int CParty::insert_time_card_select_right_msg(int delay_sec) {
    const unsigned int key = static_cast<unsigned int>(
        gen_timer_key(TIMER_MESSAGE_CLEAR_DUNGEON_CARD_SELECT_RIGHT));
    return G_TimerQueue()->InsertTimer(
        1,
        party_index_,
        TIMER_MESSAGE_CLEAR_DUNGEON_CARD_SELECT_RIGHT,
        delay_sec,
        key,
        0);
}

// IDA_EA: 0x85B22E2
// IDA_NAME: _ZN6CParty26IncreaseSetPlayResultCountEP5CUser
// DEMANGLED: CParty::IncreaseSetPlayResultCount(CUser *)
int CParty::IncreaseSetPlayResultCount(CUser* user) {
    const int seat = GetMemberSlotNo(user);
    if (seat >= 0) {
        members_[seat].state = 1;
        // The original returns the legacy per-seat anchor beginning at the
        // host_ member, while the meaningful side effect is members_[seat].
        // Keep that exact address sequence without reopening CParty as bytes.
        return static_cast<int>(reinterpret_cast<intptr_t>(&host_) +
                                sizeof(cMember) * seat);
    }
    return seat;
}

// IDA_EA: 0x85B2384
// IDA_NAME: _ZN6CParty26GetNoSendSetPlayResultUserEi
// DEMANGLED: CParty::GetNoSendSetPlayResultUser(int)
const char* CParty::GetNoSendSetPlayResultUser(int seat) {
    CUser* const user = members_[seat].user;
    if (user && members_[seat].state && user->getCurCharacR()) {
        return user->getCurCharacName();
    }
    return kEmptyCharacName;
}

// IDA_EA: 0x85B35F4
// IDA_NAME: _ZN6CParty21send_dungeon_fail_msgEv
// DEMANGLED: CParty::send_dungeon_fail_msg(void)
int CParty::send_dungeon_fail_msg() {
    const int delay_sec = battle_field_.village_monster_uid_ ? 3 : 10;
    const unsigned int key = static_cast<unsigned int>(
        gen_timer_key(TIMER_MESSAGE_DUNGEON_FAIL));
    return G_TimerQueue()->InsertTimer(
        1, party_index_, TIMER_MESSAGE_DUNGEON_FAIL, delay_sec, key, 0);
}

// IDA_EA: 0x85B494E
// IDA_NAME: _ZN6CParty22SendCardSelectCompleteEv
// DEMANGLED: CParty::SendCardSelectComplete(void)
void CParty::SendCardSelectComplete() {
    PacketGuard packet;
    packet.put_header(0, 67);
    packet.finalize(true);
    send_to_party(packet);
}

// IDA_EA: 0x85B59CA
// IDA_NAME: _ZN6CParty25get_rate_level_differenceEv
// DEMANGLED: CParty::get_rate_level_difference(void)
int CParty::get_rate_level_difference() {
    CDungeon* const dungeon = battle_field_.GetDungeon();
    if (!dungeon) {
        return 0;
    }
    const int difference =
        std::abs(dungeon->get_standard_level() - getMemberMaxLevel());
    if (difference <= 10) {
        return 1;
    }
    return difference > 19 ? 3 : 2;
}

// IDA_EA: 0x85B690A
// IDA_NAME: _ZN6CParty19sub_hellparty_valueEv
// DEMANGLED: CParty::sub_hellparty_value(void)
int CParty::sub_hellparty_value() {
    float& value = battle_data_.hell_party_value_total();
    float next_value = value * battle_field_.hell_party_sub_rate();
    const int minimum = battle_field_.hell_party_min_value();
    if (static_cast<int>(next_value) < minimum) {
        next_value = static_cast<float>(minimum);
    }
    value = next_value;
    send_hellParty_value();
    return 0;
}

// IDA_EA: 0x85B697C
// IDA_NAME: _ZN6CParty20send_hellParty_valueEv
// DEMANGLED: CParty::send_hellParty_value(void)
void CParty::send_hellParty_value() {
    PacketGuard packet;
    packet.put_header(0, 207);
    const int percentage = static_cast<int>(
        battle_data_.hell_party_value_total() * 100.0L /
        battle_field_.hell_party_value_max());
    packet.put_int(percentage);
    packet.finalize(true);
    send_to_party(packet);
}

// IDA_EA: 0x85B58AC
// IDA_NAME: _ZN6CParty19cal_hellparty_valueEiii
// DEMANGLED: CParty::cal_hellparty_value(int,int,int)
int CParty::cal_hellparty_value(int dungeon_difficulty,
                                int level_difference_rate,
                                int other_member_count) {
    if (level_difference_rate == 0 || other_member_count < 0 ||
        dungeon_difficulty > 4) {
        return 0;
    }

    const CDungeon* const dungeon = battle_field_.GetDungeon();
    if (!dungeon) {
        return 0;
    }

    const float extra_rate = dungeon->ancient_dungeon_ == 1
                                 ? battle_field_.hell_party_extra_clear_rate()
                                 : 0.0f;
    float& value = battle_data_.hell_party_value_total();
    value += static_cast<float>(battle_field_.hell_party_clear_base_value()) *
             battle_field_.hell_party_level_rate(
                 static_cast<unsigned int>(level_difference_rate)) *
             (battle_field_.hell_party_difficulty_rate(
                  static_cast<unsigned int>(dungeon_difficulty)) +
              1.0f + extra_rate +
              battle_field_.hell_party_member_rate(
                  static_cast<unsigned int>(other_member_count)));
    if (static_cast<int>(value) > battle_field_.hell_party_value_max()) {
        value = static_cast<float>(battle_field_.hell_party_value_max());
    }
    send_hellParty_value();
    return 0;
}

// IDA_EA: 0x85B669A
// IDA_NAME: _ZN6CParty15enumPartyMemberEPFbP5CUserE
// DEMANGLED: CParty::enumPartyMember(bool (*)(CUser *))
char CParty::enumPartyMember(bool (*callback)(CUser*)) {
    char result = 0;
    for (int seat = 0;; ++seat) {
        result = static_cast<char>(seat <= 3);
        if (seat > 3) {
            break;
        }
        if (_checkValidUser(seat)) {
            result = static_cast<char>(!callback(members_[seat].user));
            if (result) {
                break;
            }
        }
    }
    return result;
}

// IDA_EA: 0x85BA0D8
// IDA_NAME: _ZN6CParty21SetMemberRoutingStateEP5CUserc
// DEMANGLED: CParty::SetMemberRoutingState(CUser *,char)
char CParty::SetMemberRoutingState(CUser* user, char state) {
    for (int seat = 0; seat <= 3; ++seat) {
        if (_checkValidUser(seat) == 1 && members_[seat].user == user) {
            return static_cast<char>(
                routing_data_.SetMemberRoutingState(seat, state));
        }
    }
    return 0;
}

// IDA_EA: 0x85BD1DC
// IDA_NAME: _ZN6CParty19GetValidPartyMemberERSt6vectorIiSaIiEE
// DEMANGLED: CParty::GetValidPartyMember(std::vector<int,std::allocator<int>> &)
bool CParty::GetValidPartyMember(std::vector<int>& members) {
    members.clear();
    int seat = 0;
    for (; seat <= 3; ++seat) {
        if (checkValidUser(seat)) {
            members.push_back(seat);
        }
    }
    return seat <= 3;
}

// IDA_EA: 0x85BDC00
// IDA_NAME: _ZN6CParty27GetPartyMemberForceDropFlagEv
// DEMANGLED: CParty::GetPartyMemberForceDropFlag(void)
int CParty::GetPartyMemberForceDropFlag() {
    for (int seat = 0; seat <= 3; ++seat) {
        CUser* const user = get_user(seat);
        if (user && user->isGMUser() && user->getForceDropFlag()) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x85BDCEC
// IDA_NAME: _ZN6CParty18resetMemberUseCoinEv
// DEMANGLED: CParty::resetMemberUseCoin(void)
bool CParty::resetMemberUseCoin() {
    int seat = 0;
    for (; seat <= 3; ++seat) {
        members_[seat].used_coin_count = 0;
    }
    return seat <= 3;
}

// IDA_EA: 0x85BE9C8
// IDA_NAME: _ZN6CParty19IsUseFatigueDungeonEv
// DEMANGLED: CParty::IsUseFatigueDungeon(void)
int CParty::IsUseFatigueDungeon() {
    CDungeon* const dungeon = battle_field_.dungeon_;
    if (battle_field_.instance_dungeon_mode_ != 1 && dungeon &&
        !dungeon->blood_dungeon_type_ &&
        dungeon->village_attack_revenge_dungeon_ != 1 &&
        dungeon->isNoFatigueDungeon() != 1) {
        return 1;
    }
    return 0;
}

// IDA_EA: 0x85BEA4A
// IDA_NAME: _ZN6CParty13CheckMapClearEv
// DEMANGLED: CParty::CheckMapClear(void)
int CParty::CheckMapClear() {
    return battle_field_.check_grid_clear();
}

// IDA_EA: 0x85BF77A
// IDA_NAME: _ZN6CParty8setTitleEPKc
// DEMANGLED: CParty::setTitle(char const*)
char* CParty::setTitle(const char* title) {
    return std::strncpy(title_, title, 0x20u);
}

// IDA_EA: 0x85BF850
// IDA_NAME: _ZN6CParty16ClearMapHitCountEv
// DEMANGLED: CParty::ClearMapHitCount(void)
CParty* CParty::ClearMapHitCount() {
    map_hit_statistics_.map_count_ = 0;
    map_hit_statistics_.total_client_hit_count_ = 0;
    std::memset(map_hit_statistics_.client_hit_counts_,
                0,
                sizeof(map_hit_statistics_.client_hit_counts_));
    map_hit_statistics_.total_monster_field_a1a_ = 0;
    std::memset(map_hit_statistics_.monster_field_a1a_,
                0,
                sizeof(map_hit_statistics_.monster_field_a1a_));
    map_hit_statistics_.total_monster_field_a1e_ = 0;
    std::memset(map_hit_statistics_.monster_field_a1e_,
                0,
                sizeof(map_hit_statistics_.monster_field_a1e_));
    map_hit_statistics_.has_a1e_tenth_multiple_ = 0;
    std::memset(skill_quest_use_counts_, 0, sizeof(skill_quest_use_counts_));
    map_hit_auxiliary_count_ = 0;
    hit_time_divisor_ = 0;
    return this;
}

// IDA_EA: 0x860B59C
// IDA_NAME: _ZN6CParty15SetStartRoutingEv
// DEMANGLED: CParty::SetStartRouting(void)
bool CParty::SetStartRouting() {
    if (routing_data_.GetRoutingState() != 1) {
        return routing_data_.SetRoutingState(1) != 0;
    }
    return false;
}

// IDA_EA: 0x860B5CC
// IDA_NAME: _ZN6CParty14SetRoutingItemER10Inven_Itemj
// DEMANGLED: CParty::SetRoutingItem(Inven_Item &,unsigned int)
CParty::CItemRoutingData* CParty::SetRoutingItem(Inven_Item& item,
                                                 unsigned int unique_id) {
    return routing_data_.SetRoutingItemIndex(item, unique_id);
}

// IDA_EA: 0x860B602
// IDA_NAME: _ZN6CParty19GetRoutingInvenItemEv
// DEMANGLED: CParty::GetRoutingInvenItem(void)
Inven_Item* CParty::GetRoutingInvenItem() {
    return routing_data_.GetRoutingInvenItem();
}
