#include "old_game/player/core/character_load_mercenary.h"

#include "common/manager/cdata_manager_tail_public_abi.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_list.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/etc/independent_drop_parameter_script.h"
#include "old_data/db/mercenary/db_mercenary_log_info.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/mt_rand.h"
#include "common/manager/system_time.h"

#include <cstdio>
#include <cstring>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>


namespace MercenaryHardCodingSettingValue {

const unsigned int ITEM_DROP_MAX_VALUE = 10000u;
const std::string MERCENARY_BASE_POSTAL_MESSAGE("game_server_msg_222");

}  // namespace MercenaryHardCodingSettingValue

namespace {

const stMercenarySystemInfo& MercenarySystemInfo() {
    return *G_CDataManager()->get_mercenary_system_info();
}

int SignedIndex(char value) {
    return static_cast<signed char>(value);
}

std::pair<int, int> FindLevelReward(
    int level,
    const std::vector<std::pair<int, std::pair<int, int> > >& level_ranges) {
    unsigned int selected_index = 0;
    int selected_level = 0;
    for (unsigned int index = 0;
         index < static_cast<unsigned int>(level_ranges.size());
         ++index) {
        if (level_ranges[index].first <= level &&
            level_ranges[index].first >= selected_level) {
            selected_level = level_ranges[index].first;
            selected_index = index;
        }
    }
    return level_ranges[selected_index].second;
}

template <typename T>
T FindRelativeProbability(const std::vector<std::pair<int, T> >& values,
                          int dice) {
    int accumulated = 0;
    for (unsigned int index = 0;
         index < static_cast<unsigned int>(values.size());
         ++index) {
        accumulated += values[index].first;
        if (accumulated >= dice) {
            return values[index].second;
        }
    }
    return values[0].second;
}

void MaterializeMercenaryRewardItem(int item_id, Inven_Item& item) {
    item.item_id_ = item_id;
    item.set_add_info(1);
    G_CDataManager()->get_item_list()->create_item(
        static_cast<ENUM_ITEM_CREATE_TYPE>(1), item, 1);
}

}  // namespace

// IDA_EA: 0x81B2B34
// IDA_NAME: _ZNK21CHARAC_LOAD_MERCENARY25CheckCompetitionAreaIndexEci
// DEMANGLED: CHARAC_LOAD_MERCENARY::CheckCompetitionAreaIndex(char,int)const
bool CHARAC_LOAD_MERCENARY::CheckCompetitionAreaIndex(
    char area_index,
    int charac_level) const {
    const std::vector<stMercenaryCompetitionArea>& areas =
        MercenarySystemInfo().competition_areas;
    if (areas.empty()) {
        return false;
    }

    const int signed_index = SignedIndex(area_index);
    if (static_cast<unsigned int>(areas.size()) <
        static_cast<unsigned int>(signed_index)) {
        return false;
    }

    return static_cast<int16_t>(
               areas[static_cast<unsigned int>(signed_index)].value_14) <=
           charac_level;
}

// IDA_EA: 0x81B2BC4
// IDA_NAME: _ZNK21CHARAC_LOAD_MERCENARY27CheckCompetitionPeriodIndexEc
// DEMANGLED: CHARAC_LOAD_MERCENARY::CheckCompetitionPeriodIndex(char)const
bool CHARAC_LOAD_MERCENARY::CheckCompetitionPeriodIndex(
    char period_index) const {
    const std::vector<std::pair<int, float> >& periods =
        MercenarySystemInfo().first_rates;
    if (periods.empty()) {
        return false;
    }
    return static_cast<unsigned int>(periods.size()) >=
           static_cast<unsigned int>(SignedIndex(period_index));
}

// IDA_EA: 0x81B2C1E
// IDA_NAME: _ZNK21CHARAC_LOAD_MERCENARY14GetPeriodBonusEv
// DEMANGLED: CHARAC_LOAD_MERCENARY::GetPeriodBonus(void)const
float CHARAC_LOAD_MERCENARY::GetPeriodBonus() const {
    const std::vector<std::pair<int, float> >& periods =
        MercenarySystemInfo().first_rates;
    const signed char index =
        static_cast<signed char>(competition_period);
    if (periods.empty() ||
        static_cast<signed char>(periods.size()) < index) {
        return 0.0f;
    }
    return periods[static_cast<unsigned int>(index)].second;
}

// IDA_EA: 0x81B2C94
// IDA_NAME: _ZNK21CHARAC_LOAD_MERCENARY14GetAvatarBonusEv
// DEMANGLED: CHARAC_LOAD_MERCENARY::GetAvatarBonus(void)const
float CHARAC_LOAD_MERCENARY::GetAvatarBonus() const {
    const std::vector<std::pair<int, float> >& avatars =
        MercenarySystemInfo().second_rates;
    const signed char index =
        static_cast<signed char>(avatar_bonus_index);
    if (avatars.empty() ||
        static_cast<signed char>(avatars.size()) < index) {
        return 0.0f;
    }
    return avatars[static_cast<unsigned int>(index)].second;
}

// IDA_EA: 0x81B2D0A
// IDA_NAME: _ZNK21CHARAC_LOAD_MERCENARY17GetRewardCriticalEi
// DEMANGLED: CHARAC_LOAD_MERCENARY::GetRewardCritical(int)const
float CHARAC_LOAD_MERCENARY::GetRewardCritical(int reward_dice) const {
    const stMercenarySystemInfo& info = MercenarySystemInfo();
    if (reward_dice < 0 ||
        static_cast<int32_t>(info.value_3c) < reward_dice) {
        return 0.0f;
    }
    return FindRelativeProbability(info.third_rates, reward_dice);
}

// IDA_EA: 0x81B2D6A
// IDA_NAME: _ZNK21CHARAC_LOAD_MERCENARY16CalcItemDropRateEP5CUser
// DEMANGLED: CHARAC_LOAD_MERCENARY::CalcItemDropRate(CUser *)const
int CHARAC_LOAD_MERCENARY::CalcItemDropRate(CUser* user) const {
    const std::pair<int, int> reward = FindLevelReward(
        static_cast<int16_t>(level), MercenarySystemInfo().level_ranges);
    const int base_rate = reward.second;
    const float period_bonus = GetPeriodBonus();
    const float avatar_bonus = GetAvatarBonus();
    const int period_term = GetCompetitionPeriodTerm(0);
    const int drop_rate = static_cast<int>(
        static_cast<long double>(base_rate) * period_bonus * avatar_bonus *
        static_cast<long double>(period_term));

    if (user && static_cast<uint8_t>(user->isGMUser()) != 0) {
        static char buffer[256];
        std::sprintf(buffer,
                     "\nbase:%d * period:%.2f * avatar:%.2f * term:%d = %d\n",
                     base_rate,
                     period_bonus,
                     avatar_bonus,
                     period_term,
                     drop_rate);
        SendMessageToCUser(user, buffer);
    }
    return drop_rate > 0 ? drop_rate : -1;
}

// IDA_EA: 0x81B2E86
// IDA_NAME: _ZNK21CHARAC_LOAD_MERCENARY19GetCompetitionStateEv
// DEMANGLED: CHARAC_LOAD_MERCENARY::GetCompetitionState(void)const
int CHARAC_LOAD_MERCENARY::GetCompetitionState() const {
    const uint32_t current_sec =
        static_cast<uint32_t>(GlobalData::s_systemTime_.getCurSec());
    if (static_cast<int32_t>(end_time - current_sec) > 0) {
        return 1;
    }
    return competition_area == 0xffu ? 0 : 2;
}

// IDA_EA: 0x81B2ED2
// IDA_NAME: _ZNK21CHARAC_LOAD_MERCENARY26GetCompetitionPeriodSecondEc
// DEMANGLED: CHARAC_LOAD_MERCENARY::GetCompetitionPeriodSecond(char)const
int CHARAC_LOAD_MERCENARY::GetCompetitionPeriodSecond(
    char period_index) const {
    if (!CheckCompetitionPeriodIndex(period_index)) {
        return 0;
    }
    const stMercenarySystemInfo& info = MercenarySystemInfo();
    const int count =
        info.first_rates[static_cast<unsigned int>(
            SignedIndex(period_index))].first;
    return static_cast<int>(
        info.duration_sec * static_cast<uint32_t>(count));
}

// IDA_EA: 0x81B2F32
// IDA_NAME: _ZNK21CHARAC_LOAD_MERCENARY24GetCompetitionPeriodTermEPb
// DEMANGLED: CHARAC_LOAD_MERCENARY::GetCompetitionPeriodTerm(bool *)const
int CHARAC_LOAD_MERCENARY::GetCompetitionPeriodTerm(
    bool* partial_period) const {
    const char period_index = static_cast<char>(competition_period);
    if (!CheckCompetitionPeriodIndex(period_index)) {
        return 0;
    }

    const stMercenarySystemInfo& info = MercenarySystemInfo();
    const std::vector<std::pair<int, float> >& periods = info.first_rates;
    const signed char index = static_cast<signed char>(competition_period);
    if (periods.empty() ||
        static_cast<signed char>(periods.size()) < index) {
        return 0;
    }

    const uint32_t elapsed =
        static_cast<uint32_t>(GlobalData::s_systemTime_.getCurSec()) -
        start_time;
    const int period_term =
        static_cast<int>(elapsed / info.duration_sec);
    const int maximum_term =
        periods[static_cast<unsigned int>(index)].first;
    if (maximum_term <= period_term) {
        return maximum_term;
    }
    if (partial_period) {
        *partial_period = true;
    }
    return period_term;
}

// IDA_EA: 0x81B3046
// IDA_NAME: _ZNK21CHARAC_LOAD_MERCENARY13GetRewardGoldEP5CUserR25competition_reward_result
// DEMANGLED: CHARAC_LOAD_MERCENARY::GetRewardGold(
//             CUser *,competition_reward_result &)const
int CHARAC_LOAD_MERCENARY::GetRewardGold(
    CUser* user,
    competition_reward_result& result) const {
    const stMercenarySystemInfo& info = MercenarySystemInfo();
    const std::pair<int, int> reward =
        FindLevelReward(static_cast<int16_t>(level), info.level_ranges);
    const int base_gold = reward.first;
    const float period_bonus = GetPeriodBonus();
    const float avatar_bonus = GetAvatarBonus();
    const int period_term = GetCompetitionPeriodTerm(0);
    const long double rounding =
        static_cast<long double>(0.0099999998f);
    const int gold_before_critical = static_cast<int>(
        static_cast<long double>(base_gold) * period_bonus *
            static_cast<long double>(period_term) * avatar_bonus +
        rounding);

    static CMTRand probability_reward_item;
    const unsigned long critical_limit =
        static_cast<unsigned long>(info.value_3c);
    const int critical_dice =
        static_cast<int>(probability_reward_item.randInt(critical_limit));
    const float critical = GetRewardCritical(critical_dice);

    if (static_cast<uint8_t>(user->isGMUser()) != 0) {
        static char buffer[256];
        std::sprintf(
            buffer,
            "term:%d, base gold:%d\nperiod:%.2f, avatar:%.2f\n"
            "%d * %.2f * %d * %.2f = %d\n"
            "critical Dice:%d/10,000 value:%.2f",
            period_term,
            base_gold,
            period_bonus,
            avatar_bonus,
            base_gold,
            period_bonus,
            period_term,
            avatar_bonus,
            gold_before_critical,
            critical_dice,
            critical);
        SendMessageToCUser(user, buffer);
    }

    const int total_gold = static_cast<int>(
        static_cast<long double>(gold_before_critical) * critical +
        rounding);
    result.first_dword = static_cast<int>(
        static_cast<long double>(base_gold) * period_bonus *
            static_cast<long double>(period_term) * critical +
        rounding);
    result.second_dword = total_gold - result.first_dword;
    return total_gold;
}

// IDA_EA: 0x81B3276
// IDA_NAME: _ZN21CHARAC_LOAD_MERCENARY13GetRewardItemEP5CUserR25competition_reward_resultR10Inven_ItemRSs
// DEMANGLED: CHARAC_LOAD_MERCENARY::GetRewardItem(
//             CUser *,competition_reward_result &,Inven_Item &,std::string &)
int CHARAC_LOAD_MERCENARY::GetRewardItem(
    CUser* user,
    competition_reward_result& result,
    Inven_Item& item,
    std::string& server_message) {
    if (!user) {
        return 0;
    }

    item.reset();
    server_message =
        MercenaryHardCodingSettingValue::MERCENARY_BASE_POSTAL_MESSAGE;
    if (!CheckCompetitionAreaIndex(
            static_cast<char>(competition_area),
            static_cast<int16_t>(level))) {
        return 0;
    }

    stMercenarySystemInfo& info =
        *G_CDataManager()->get_mercenary_system_info();
    const int initial_area_index =
        static_cast<signed char>(competition_area);
    if (info.competition_areas[
            static_cast<unsigned int>(initial_area_index)].value_14 == 0) {
        competition_area = 0;
    }

    static CMTRand probability_reward_item;
    static int item_drop_dice;
    static int group_dice;
    static int mob_dice;
    static int item_dice;
    static char buffer[256];

    const int drop_rate = CalcItemDropRate(user);
    const unsigned long item_drop_limit =
        MercenaryHardCodingSettingValue::ITEM_DROP_MAX_VALUE;
    item_drop_dice =
        static_cast<int>(probability_reward_item.randInt(item_drop_limit));
    if (user->mercenary_reward_item_dice_override()) {
        item_drop_dice = 0;
    }
    if (item_drop_dice >= drop_rate) {
        return 0;
    }

    const int area_index =
        static_cast<signed char>(competition_area);
    stMercenaryCompetitionArea& area =
        info.competition_areas[static_cast<unsigned int>(area_index)];
    const unsigned long group_limit =
        static_cast<unsigned long>(area.value_10);
    group_dice =
        static_cast<int>(probability_reward_item.randInt(group_limit));
    if (static_cast<uint8_t>(user->isGMUser()) != 0) {
        std::sprintf(buffer,
                     "dice: %d/%d\n",
                     group_dice,
                     area.value_10);
        SendMessageToCUser(user, buffer);
    }

    int accumulated = 0;
    unsigned int group_index = 0;
    for (; group_index < static_cast<unsigned int>(area.reward_groups.size());
         ++group_index) {
        accumulated += area.reward_groups[group_index].first_dword;
        if (accumulated >= group_dice) {
            break;
        }
    }
    if (group_index >= static_cast<unsigned int>(area.reward_groups.size())) {
        group_index = 0;
    }

    stMercenaryRewardGroup& group = area.reward_groups[group_index];
    server_message = group.group_name;
    if (!group.first_rewards.empty()) {
        const unsigned long mob_limit =
            static_cast<unsigned long>(group.second_dword);
        mob_dice =
            static_cast<int>(probability_reward_item.randInt(mob_limit));
        const int monster_index =
            FindRelativeProbability(group.first_rewards, mob_dice);

        IndependentDropParameterScript* drop_script =
            G_CDataManager()->get_independent_drop_parameter_script();
        std::map<int, std::pair<int, int> >::const_iterator range =
            drop_script->file_index_range_.find(monster_index);
        if (range == drop_script->file_index_range_.end()) {
            return 0;
        }

        const int list_index = range->second.first;
        if (static_cast<int>(drop_script->independent_drop_list_.size()) <=
            list_index) {
            return 0;
        }

        const std::vector<std::pair<int, int> >& drops =
            drop_script->independent_drop_list_[
                static_cast<unsigned int>(list_index)].drops;
        const unsigned long item_limit =
            static_cast<unsigned long>(range->second.second);
        item_dice =
            static_cast<int>(probability_reward_item.randInt(item_limit));
        accumulated = 0;
        for (unsigned int index = 0;
             index < static_cast<unsigned int>(drops.size());
             ++index) {
            accumulated += drops[index].second;
            if (item_dice <= accumulated) {
                MaterializeMercenaryRewardItem(drops[index].first, item);
                result.success_flag = 1;
                return 1;
            }
        }
        return 0;
    }

    if (group.second_rewards.empty()) {
        return 0;
    }

    const unsigned long item_limit =
        static_cast<unsigned long>(group.third_dword);
    item_dice =
        static_cast<int>(probability_reward_item.randInt(item_limit));
    accumulated = 0;
    for (unsigned int index = 0;
         index < static_cast<unsigned int>(group.second_rewards.size());
         ++index) {
        accumulated += group.second_rewards[index].first;
        if (item_dice <= accumulated) {
            MaterializeMercenaryRewardItem(
                group.second_rewards[index].second, item);
            result.success_flag = 1;
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x81B3814
// IDA_NAME: _ZN21CHARAC_LOAD_MERCENARY11CompetitionEP5CUsercc
// DEMANGLED: CHARAC_LOAD_MERCENARY::Competition(CUser *,char,char)
int CHARAC_LOAD_MERCENARY::Competition(CUser* user,
                                       char competition_type,
                                       char competition_count) {
    char selected_area = competition_count;
    if (!user) {
        return 0;
    }

    competition_reward_result previous_reward;
    Reward(user, previous_reward);
    if (GetCompetitionState() != 0) {
        return 0;
    }
    if (!CheckCompetitionPeriodIndex(competition_type)) {
        return 0;
    }
    if (!CheckCompetitionAreaIndex(
            competition_count, static_cast<int16_t>(level))) {
        return 0;
    }

    stMercenarySystemInfo& info =
        *G_CDataManager()->get_mercenary_system_info();
    const int requested_area =
        static_cast<signed char>(competition_count);
    if (info.competition_areas[
            static_cast<unsigned int>(requested_area)].area_index ==
        0xffffu) {
        static CMTRand probability_area;
        std::vector<int> candidates;
        for (int index = 0;
             index < static_cast<int>(info.competition_areas.size());
             ++index) {
            const stMercenaryCompetitionArea& area =
                info.competition_areas[static_cast<unsigned int>(index)];
            if (area.value_14 != 0 &&
                static_cast<int16_t>(area.value_14) <=
                    static_cast<int16_t>(level)) {
                candidates.push_back(index);
            }
        }

        const unsigned long candidate_limit =
            static_cast<unsigned long>(candidates.size() - 1);
        const unsigned int candidate_dice =
            probability_area.randInt(candidate_limit);
        const int candidate = candidates[candidate_dice];
        selected_area = CheckCompetitionAreaIndex(
                            static_cast<char>(candidate),
                            static_cast<int16_t>(level))
                            ? static_cast<char>(candidate)
                            : 0;
    }

    const int duration =
        GetCompetitionPeriodSecond(competition_type);
    if (duration == 0) {
        return 0;
    }

    alter();
    start_time =
        static_cast<uint32_t>(GlobalData::s_systemTime_.getCurSec());
    end_time = start_time + static_cast<uint32_t>(duration);
    competition_area = static_cast<uint8_t>(selected_area);
    competition_period = static_cast<uint8_t>(competition_type);
    DB_MercenaryLogInfo::makeRequest(
        static_cast<unsigned int>(user->get_acc_id()),
        charac_no,
        start_time,
        end_time,
        0,
        competition_type,
        selected_area,
        -1,
        -1,
        -1);
    return 1;
}

// IDA_EA: 0x81B3B68
// IDA_NAME: _ZN21CHARAC_LOAD_MERCENARY6RewardEP5CUserR25competition_reward_result
// DEMANGLED: CHARAC_LOAD_MERCENARY::Reward(
//             CUser *,competition_reward_result &)
int CHARAC_LOAD_MERCENARY::Reward(
    CUser* user,
    competition_reward_result& result) {
    if (!user) {
        return 0;
    }
    if (GetCompetitionState() == 0) {
        return 0;
    }

    static Inven_Item empty_item;
    static const std::string mail_title(
        g_scriptStringManager_.findString(
            4, "game_server_msg_225", 0));
    static std::string server_message;
    static char str_message[256];

    bool partial_period = false;
    const int reward_gold = GetRewardGold(user, result);
    const int reward_item =
        GetRewardItem(user, result, empty_item, server_message);
    if (reward_item != 1) {
        server_message =
            MercenaryHardCodingSettingValue::MERCENARY_BASE_POSTAL_MESSAGE;
    }

    if (reward_gold <= 0) {
        const int period_term =
            GetCompetitionPeriodTerm(&partial_period);
        user->history_log().MercenaryReturn(
            charac_no,
            static_cast<int>(start_time),
            static_cast<int>(end_time),
            period_term,
            reward_gold,
            0);
        start_time = 0;
        end_time = 0;
        competition_area = 0xffu;
        competition_period = 0xffu;
        alter();
        return 1;
    }

    const char* localized_message =
        g_scriptStringManager_.findString(
            4, server_message.c_str(), 0);
    std::strncpy(str_message, localized_message, 0xffu);
    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        mail_title.c_str(),
        empty_item,
        static_cast<unsigned int>(reward_gold),
        charac_no,
        str_message,
        static_cast<int>(std::strlen(str_message)),
        30,
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
        false,
        false);

    const int period_term =
        GetCompetitionPeriodTerm(&partial_period);
    user->history_log().MercenaryReturn(
        charac_no,
        static_cast<int>(start_time),
        static_cast<int>(end_time),
        period_term,
        reward_gold,
        0);

    const char return_type = partial_period ? 2 : 1;
    const int reward_item_id = empty_item.getKey();
    CItem* reward_item_data =
        G_CDataManager()->find_item(reward_item_id);
    int reward_item_rarity = -1;
    if (reward_item_data) {
        reward_item_rarity = reward_item_data->get_rarity();
    }

    DB_MercenaryLogInfo::makeRequest(
        static_cast<unsigned int>(user->get_acc_id()),
        charac_no,
        start_time,
        static_cast<uint32_t>(GlobalData::s_systemTime_.getCurSec()),
        return_type,
        static_cast<char>(competition_period),
        static_cast<char>(competition_area),
        reward_gold,
        reward_item_id,
        reward_item_rarity);

    start_time = 0;
    end_time = 0;
    competition_area = 0xffu;
    competition_period = 0xffu;
    alter();
    return 1;
}
