#include "old_data/PVF/data/datamanager/server_parameter_script.h"

#include "common/config.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "common/manager/global_tools.h"
#include "old_data/PVF/etc/assault_parameter_types.h"
#include "old_data/PVF/etc/blood_dungeon_reward_info.h"
#include "common/manager/shared_server_message_abi.h"

#include <algorithm>
#include <cstring>
#include <new>
#include <string.h>
#include <utility>

// IDA_EA: 0x8A8CDB0.
// IDA_NAME: _ZN15ItemFarmingDataC2Ev
// DEMANGLED: ItemFarmingData::ItemFarmingData(void)
ItemFarmingData::ItemFarmingData() : list(), total_rate(0) {
    // The original C2 explicitly clears the just-constructed vector.
    list.clear();
}

// IDA_EA: 0x853AC12.
// IDA_NAME: _ZN15ItemFarmingDataC2ERKS_
// DEMANGLED: ItemFarmingData::ItemFarmingData(ItemFarmingData const&)
ItemFarmingData::ItemFarmingData(const ItemFarmingData& rhs)
    : list(rhs.list), total_rate(rhs.total_rate) {}

// IDA_EA: 0x83CD0BA.
// IDA_NAME: _ZN15ItemFarmingDataD2Ev
// DEMANGLED: ItemFarmingData::~ItemFarmingData(void)
ItemFarmingData::~ItemFarmingData() {}

// IDA_EA: 0x8A8CC22
// IDA_NAME: _ZN26stUpgradeItemTicketParam_tC2Ev
// DEMANGLED: stUpgradeItemTicketParam_t::stUpgradeItemTicketParam_t(void)
stUpgradeItemTicketParam_t::stUpgradeItemTicketParam_t()
    : minimum_upgrade(0),
      maximum_upgrade(0),
      penalty_protection(0),
      jpn_only(0) {}

// IDA_EA: 0x84A60AA
// IDA_NAME: _ZN21ServerParameterScript12PvPPlayerKeyC2Ehhh
// DEMANGLED: ServerParameterScript::PvPPlayerKey::PvPPlayerKey(unsigned char,unsigned char,unsigned char)
ServerParameterScript::PvPPlayerKey::PvPPlayerKey(
    unsigned char charac_job_value,
    unsigned char first_grow_type_value,
    unsigned char pvp_grade_value)
    : charac_job(charac_job_value),
      first_grow_type(first_grow_type_value),
      pvp_grade(pvp_grade_value) {}

// IDA_EA: 0x84A6034
// IDA_NAME: _ZNK21ServerParameterScript12PvPPlayerKeyltERKS0_
// DEMANGLED: ServerParameterScript::PvPPlayerKey::operator<(ServerParameterScript::PvPPlayerKey const&)const
bool ServerParameterScript::PvPPlayerKey::operator<(
    const PvPPlayerKey& rhs) const {
    if (charac_job < rhs.charac_job) {
        return true;
    }
    if (charac_job != rhs.charac_job) {
        return false;
    }
    if (first_grow_type < rhs.first_grow_type) {
        return true;
    }
    return first_grow_type == rhs.first_grow_type &&
           pvp_grade < rhs.pvp_grade;
}

// IDA_EA: 0x836F2CC
// IDA_NAME: _ZN31stFatigueBatteryConstantTable_tC2Ev
// DEMANGLED: stFatigueBatteryConstantTable_t::stFatigueBatteryConstantTable_t(void)
stFatigueBatteryConstantTable_t::stFatigueBatteryConstantTable_t() {
    // Keep the writes intentionally narrower than a value-initialization.
    // IDA 0x836F2CC does not initialize the recovered padding at +0x0a,
    // +0x11 or +0x1a3.  The +0x1ac dword is the later recovered
    // [pc room party exp bonus rate] member, not tail padding.
    limit_fatigue_battery_charging_amount_ = 0;
    reserved_002_ = 0;
    end_level_fatigue_battery_weight_ = 0.0f;
    character_level_fatigue_battery_constance_ = 40;
    limit_fatigue_battery_level_ = 0;
    fatigue_battery_money_amount_ = 0;
    fatigue_battery_level_for_money_active_ =
        oldsrv::config::kFatigueBatteryMoneyActivationLevelDefault;

    // The original memset starts at +0x12 for 0x191 bytes: all 200 table
    // entries and only the first byte at +0x1a2.
    memset(limit_grownup_buff_count_by_level_,
           0,
           sizeof(limit_grownup_buff_count_by_level_));
    reserved_1a2_[0] = 0;

    fatigue_battery_per_grownup_buff_active_ = 1;
    grownup_buff_per_fatigue_battery_active_ = 0;
    min_fatigue_battery_per_buff_active_ = 0;
    reserved_1aa_ = 0;
    pc_room_party_exp_bonus_rate_ = 0.0f;
}

// IDA_EA: 0x84A84B6
// IDA_NAME: _ZNK31stFatigueBatteryConstantTable_t36getLimitGrownupBuffCntFatigueBatteryEi
// DEMANGLED:
// stFatigueBatteryConstantTable_t::getLimitGrownupBuffCntFatigueBattery(int)const
int stFatigueBatteryConstantTable_t::getLimitGrownupBuffCntFatigueBattery(
    int charac_level) const {
    if (charac_level - 1 <= fatigue_battery_level_for_money_active_ &&
        charac_level > 0) {
        return limit_grownup_buff_count_by_level_[charac_level - 1];
    }
    return 0;
}

// IDA_EA: 0x836F38C
// IDA_NAME: _ZN21ServerParameterScript5clearEv
// DEMANGLED: ServerParameterScript::clear(void)
void* ServerParameterScript::clear() {
    script_version_ = 0;
    memset(version_digits_, 0, sizeof(version_digits_));
    clear_exp_bonus_rate_ = 1.0f;
    monster_exp_bonus_rate_ = 1.0f;
    for (int index = 0; index <= 3; ++index) {
        party_user_number_exp_bonus_rates_[index] = 1.0f;
        starter_server_party_exp_bonus_rates_[index] = 1.0f;
        starter_server_quest_item_drop_bonus_rates_[index] = 1.0f;
        dungeon_difficulty_exp_bonus_rates_[index] = 1.0f;
    }
    for (int index = 0; index <= 4; ++index) {
        clear_rank_exp_bonus_rates_[index] = 1.0f;
    }

    recover_stamina_prices_.clear();
    assault_prices_.clear();
    assault_price_time_weight_limit_ = 0;
    assault_price_time_limit_seconds_ = 0;
    assault_price_level_gap_weight_ = 0.0f;
    assault_chaos_point_weight_table_.clear();

    memset(pre_lottery_policy_values_, 0,
           sizeof(pre_lottery_policy_values_));
    memset(&lottery_upgrade_policy_, 0, sizeof(lottery_upgrade_policy_));
    lottery_upgrade_policy_.broadcast_minimum_rarity = -1;

    burning_fatigue_parameters_.clear();
    luck_points_.clear();
    rare_luck_point_deductions_.clear();
    unique_luck_point_deductions_.clear();
    luck_point_deduction_gaps_.clear();
    point_per_fatigue_ = 0;
    event_point_per_fatigue_ = 0;
    point_per_play_time_ = 0;
    play_time_for_point_ = 0;
    damaged_equipment_ratio_ = 0;
    fatigue_battery_constant_table_.pc_room_party_exp_bonus_rate_ = 0.0f;

    escalade_tutorial_rewards_.clear();
    pvp_live_event_rewards_.clear();
    pvp_live_event_play_count_ = 0;
    pvp_exp_penalty_revisions_.clear();
    unusable_equipment_info_.clear();
    daily_match_count_ = 0;
    unusable_items_.clear();
    usable_items_by_equipment_type_.clear();
    dungeon_inout_times_.clear();
    closed_dungeons_.clear();
    blood_dungeon_reward_info_.admission_increase.clear();
    blood_dungeon_reward_info_.price_average.clear();
    blood_dungeon_reward_info_.lotto_point.clear();
    lotto_cost_ = 0;
    jackpot_ = 0;
    for (int row = 0; row <= 6; ++row) {
        emblem_compound_info_[row].money_cost = -1;
    }
    memset(recipe_upgrade_table_, 0, sizeof(recipe_upgrade_table_));
    memset(recipe_amplify_table_, 0, sizeof(recipe_amplify_table_));

    compound_avatar_upper_rate_bonus_ = 0;
    compound_avatar_lower_rate_reduction_ = 0;
    compound_avatar_master_rate_bonus_ = 0;
    broadcast_item_ids_.clear();
    timed_item_gives_.clear();
    member_bonus_fatigue_charac_limit_ = 0;
    member_bonus_fatigue_account_limit_ = 0;
    clean_chatting_counts_.clear();
    max_clean_chatting_count_ = 0;
    duplicate_message_penalty_ = 1;
    max_monster_drop_per_map_ = 0;
    max_monster_count_per_map_ = 0;
    memset(training_seal_gain_counts_, 0,
           sizeof(training_seal_gain_counts_));
    training_seal_bonus_count_ = 0;
    pvp_channel_grade_penalties_.clear();
    training_seal_give_.clear();
    training_first_give_.clear();
    training_first_give_count_by_grade_.clear();
    pvp_item_give_grades_.clear();
    assault_chaos_point_weights_.clear();
    assault_level_ghost_penalties_.clear();
    assault_chaos_point_ghost_penalties_.clear();
    return memset(drop_routine_by_party_, 0,
                  sizeof(drop_routine_by_party_));
}

uint32_t ServerParameterScript::get_script_version() const {
    return script_version_;
}

uint32_t ServerParameterScript::get_antibot_server_version() const {
    return antibot_server_version_;
}

void ServerParameterScript::set_antibot_server_version(uint32_t version) {
    antibot_server_version_ = version;
}

uint8_t ServerParameterScript::get_antibot_version_digit(
    unsigned int index) const {
    return index < 4u ? version_digits_[index] : 0u;
}

void ServerParameterScript::apply_accept_handshake_reward_weights() {
    memcpy(party_user_number_exp_bonus_rates_,
           starter_server_party_exp_bonus_rates_,
           sizeof(party_user_number_exp_bonus_rates_));
}

int ServerParameterScript::get_assault_price_time_weight_limit() const {
    return assault_price_time_weight_limit_;
}

int ServerParameterScript::get_assault_price_time_limit_seconds() const {
    return assault_price_time_limit_seconds_;
}

float ServerParameterScript::get_assault_price_level_gap_weight() const {
    return assault_price_level_gap_weight_;
}

int ServerParameterScript::get_compound_avatar_upper_rate_bonus() const {
    return compound_avatar_upper_rate_bonus_;
}

int ServerParameterScript::get_compound_avatar_lower_rate_reduction() const {
    return compound_avatar_lower_rate_reduction_;
}

int ServerParameterScript::get_compound_avatar_master_rate_bonus() const {
    return compound_avatar_master_rate_bonus_;
}

const BroadcastItemIdVector&
ServerParameterScript::get_broadcast_item_ids() const {
    return broadcast_item_ids_;
}

const TimedItemGiveEntryVector&
ServerParameterScript::get_timed_item_gives() const {
    return timed_item_gives_;
}

int ServerParameterScript::get_normal_fatigue_luck_point_rate() const {
    return point_per_fatigue_;
}

int ServerParameterScript::get_premium_fatigue_luck_point_rate() const {
    return event_point_per_fatigue_;
}

ItemGenRefVector* ServerParameterScript::get_item_gen_ref_vector_0() {
    return &dungeon_drop_probabilities_;
}

ItemGenRefVector* ServerParameterScript::get_item_gen_ref_vector_1() {
    return &result_reward_probabilities_;
}

const ::BurningFatigueParamVector&
ServerParameterScript::get_burning_fatigue_params() const {
    return burning_fatigue_parameters_;
}

int ServerParameterScript::get_event_dungeon_clear_reward_item_id() const {
    return event_dungeon_clear_reward_item_id_;
}

float ServerParameterScript::get_premium_gold_card_commission_multiplier()
    const {
    return premium_gold_card_event_.card_cost;
}

const LotteryUpgradePolicy&
ServerParameterScript::get_lottery_upgrade_policy() const {
    return lottery_upgrade_policy_;
}

int ServerParameterScript::get_prevent_prefix_count() const {
    return prevent_prefix_count_;
}

const char* ServerParameterScript::get_prevent_prefix(int index) const {
    return prevent_prefixes_[index];
}

// IDA_DATA: importServerParameterScript @0x8A870B7 handles
// [black list help parameter] at 0x8A88ECE.  The two ScanInt results are
// stored at +0x34C and +0x350 (0x8A88EFA/0x8A88F12), and
// DB_ReqBlackCount @0x8424E73/@0x8424F06 reads them through the enclosing
// CDataManager at +0x3B4/+0x3B8.
int ServerParameterScript::getBlackListHelpPeriodDays() const {
    return black_list_help_period_days_;
}

int ServerParameterScript::getBlackListHelpOffsetPointsPerPeriod() const {
    return black_list_help_points_per_period_;
}

const AuctionRegisterItemLimitVector&
ServerParameterScript::get_auction_register_item_limits() const {
    return auction_register_item_limits_;
}

const ConditionEnterRevengeVector&
ServerParameterScript::get_condition_enter_revenge() const {
    return condition_enter_revenge_;
}

int ServerParameterScript::get_max_monster_drop_per_map() const {
    return max_monster_drop_per_map_;
}

UpgradeItemTicketMap& ServerParameterScript::get_upgrade_item_tickets() {
    return upgrade_item_tickets_;
}

const UpgradeItemTicketMap&
ServerParameterScript::get_upgrade_item_tickets() const {
    return upgrade_item_tickets_;
}

ItemFarmingDataMap& ServerParameterScript::get_item_farming_data() {
    return item_farming_data_;
}

const ItemFarmingDataMap&
ServerParameterScript::get_item_farming_data() const {
    return item_farming_data_;
}

const IntFloatPairVector&
ServerParameterScript::get_drop_bonus_of_dungeon_level_increase() const {
    return drop_bonus_of_dungeon_level_increase_;
}

int ServerParameterScript::get_member_bonus_fatigue_charac_limit() const {
    return member_bonus_fatigue_charac_limit_;
}

int ServerParameterScript::get_member_bonus_fatigue_account_limit() const {
    return member_bonus_fatigue_account_limit_;
}

uint16_t ServerParameterScript::get_weekend_bonus_fatigue() const {
    return weekend_bonus_.fatigue;
}

int ServerParameterScript::get_weekend_bonus_exp() const {
    return weekend_bonus_.exp;
}

uint8_t ServerParameterScript::getDropRoutineByParty(
    int member_count) const {
    if (member_count < 1 || member_count > 4) {
        return 0;
    }
    return drop_routine_by_party_[member_count - 1];
}

int ServerParameterScript::get_damaged_equipment_ratio() const {
    return damaged_equipment_ratio_;
}

float ServerParameterScript::get_monster_kind_drop_bonus_rate(int kind) const {
    return monster_kind_drop_bonus_rates_[kind];
}

float ServerParameterScript::get_dungeon_difficulty_exp_bonus_rate(
    int level_index) const {
    return dungeon_difficulty_exp_bonus_rates_[level_index];
}

float ServerParameterScript::get_quest_item_drop_bonus_starter_server(
    int member_count) const {
    return starter_server_quest_item_drop_bonus_rates_[member_count - 1];
}

stSharedServerMessageInfo*
ServerParameterScript::get_shared_server_message_info() {
    return &shared_server_message_info_;
}

const stSharedServerMessageInfo*
ServerParameterScript::get_shared_server_message_info() const {
    return &shared_server_message_info_;
}

stFatigueBatteryConstantTable_t&
ServerParameterScript::get_fatigue_battery_constant_table() {
    return fatigue_battery_constant_table_;
}

const stFatigueBatteryConstantTable_t&
ServerParameterScript::get_fatigue_battery_constant_table() const {
    return fatigue_battery_constant_table_;
}

// IDA_EA: 0x836FCA4
// IDA_NAME: _ZN21ServerParameterScriptC2Ev
// DEMANGLED: ServerParameterScript::ServerParameterScript(void)
ServerParameterScript::ServerParameterScript() {
    prevent_prefix_count_ = 0;
    memset(prevent_prefixes_, 0, sizeof(prevent_prefixes_));
    black_list_help_period_days_ = 0;
    black_list_help_points_per_period_ = 0;
    memset(version_digits_, 0, sizeof(version_digits_));
    script_version_ = 0;
    antibot_server_version_ = 0;
    clear_exp_bonus_rate_ = 0.0f;
    monster_exp_bonus_rate_ = 0.0f;
    memset(party_user_number_exp_bonus_rates_,
           0,
           sizeof(party_user_number_exp_bonus_rates_));
    memset(starter_server_party_exp_bonus_rates_,
           0,
           sizeof(starter_server_party_exp_bonus_rates_));
    memset(starter_server_quest_item_drop_bonus_rates_,
           0,
           sizeof(starter_server_quest_item_drop_bonus_rates_));
    memset(dungeon_difficulty_exp_bonus_rates_,
           0,
           sizeof(dungeon_difficulty_exp_bonus_rates_));
    memset(clear_rank_exp_bonus_rates_,
           0,
           sizeof(clear_rank_exp_bonus_rates_));
    memset(dungeon_difficulty_drop_bonus_rates_, 0,
           sizeof(dungeon_difficulty_drop_bonus_rates_));
    memset(party_user_number_drop_bonus_rates_, 0,
           sizeof(party_user_number_drop_bonus_rates_));
    memset(monster_kind_drop_bonus_rates_,
           0,
           sizeof(monster_kind_drop_bonus_rates_));
    memset(dungeon_difficulty_reward_bonus_rates_, 0,
           sizeof(dungeon_difficulty_reward_bonus_rates_));
    memset(party_user_number_reward_bonus_rates_, 0,
           sizeof(party_user_number_reward_bonus_rates_));
    item_decision_basis_ = 0;
    memset(equipment_rarity_decision_basis_, 0,
           sizeof(equipment_rarity_decision_basis_));
    memset(stackable_rarity_decision_basis_, 0,
           sizeof(stackable_rarity_decision_basis_));
    event_dungeon_clear_reward_item_id_ = 0;
    weekend_bonus_.fatigue = 0;
    weekend_bonus_.reserved_02 = 0;
    weekend_bonus_.exp = 0;
    memset(drop_routine_by_party_, 0, sizeof(drop_routine_by_party_));
    memset(pre_lottery_policy_values_, 0,
           sizeof(pre_lottery_policy_values_));
    memset(&lottery_upgrade_policy_, 0, sizeof(lottery_upgrade_policy_));
    assault_price_time_weight_limit_ = 0;
    assault_price_time_limit_seconds_ = 0;
    assault_price_level_gap_weight_ = 0.0f;
    increase_abuse_point_ = 0;
    decrease_abuse_point_ = 0;
    abuse_point_penalty_ = 0;
    decrease_experience_point_ = 0;
    lotto_cost_ = 0;
    jackpot_ = 0;
    memset(emblem_compound_info_, 0, sizeof(emblem_compound_info_));
    memset(recipe_upgrade_table_, 0, sizeof(recipe_upgrade_table_));
    memset(recipe_amplify_table_, 0, sizeof(recipe_amplify_table_));
    memset(recipe_upgrade_table_second_, 0,
           sizeof(recipe_upgrade_table_second_));
    memset(recipe_amplify_table_second_, 0,
           sizeof(recipe_amplify_table_second_));
    member_bonus_fatigue_charac_limit_ = 0;
    member_bonus_fatigue_account_limit_ = 0;
    memset(&premium_gold_card_event_, 0,
           sizeof(premium_gold_card_event_));
    max_clean_chatting_count_ = 0;
    duplicate_message_penalty_ = 0;
    max_monster_drop_per_map_ = 0;
    max_monster_count_per_map_ = 0;
    memset(training_seal_gain_counts_, 0,
           sizeof(training_seal_gain_counts_));
    training_seal_bonus_count_ = 0;
    reserved_497a_ = 0;
    damaged_equipment_ratio_ = 0;
    compound_avatar_upper_rate_bonus_ = 0;
    compound_avatar_lower_rate_reduction_ = 0;
    compound_avatar_master_rate_bonus_ = 0;

    clear();
}

// IDA_EA: 0x837BB42
// IDA_NAME: _ZN21ServerParameterScriptD2Ev
// DEMANGLED: ServerParameterScript::~ServerParameterScript()
ServerParameterScript::~ServerParameterScript() {}

// IDA_EA: 0x822ADFC
// IDA_NAME: _ZN21ServerParameterScript17get_pvp_item_giveEi
// DEMANGLED: ServerParameterScript::get_pvp_item_give(int)
int ServerParameterScript::get_pvp_item_give(int pvp_grade) const {
    int result = -1;
    const IntVector& grades = pvp_item_give_grades_;
    for (std::vector<int>::const_iterator it = grades.begin();
         it != grades.end();
         ++it) {
        if (*it > pvp_grade) {
            return result;
        }
        result = *it;
    }
    return result;
}

// IDA_EA: 0x826868C
// IDA_NAME: _ZN21ServerParameterScript14setDungeonOpenEib
// DEMANGLED: ServerParameterScript::setDungeonOpen(int,bool)
unsigned char ServerParameterScript::setDungeonOpen(int dungeon_id, bool open) {
    IntVector& closed_dungeons = closed_dungeons_;
    IntVector::iterator it =
        std::find(closed_dungeons.begin(), closed_dungeons.end(), dungeon_id);
    if (open) {
        if (it != closed_dungeons.end()) {
            closed_dungeons.erase(it);
            return 1;
        }
    } else if (it == closed_dungeons.end()) {
        closed_dungeons.push_back(dungeon_id);
        return 1;
    }
    return 0;
}

// IDA_EA: 0x82687FC
// IDA_NAME: _ZN21ServerParameterScript13isDungeonOpenEi
// DEMANGLED: ServerParameterScript::isDungeonOpen(int)
int ServerParameterScript::isDungeonOpen(int dungeon_id) const {
    const IntVector& closed_dungeons = closed_dungeons_;
    return std::find(closed_dungeons.begin(),
                     closed_dungeons.end(),
                     dungeon_id) == closed_dungeons.end();
}

const DungeonInoutTimeMap&
ServerParameterScript::get_dungeon_inout_times() const {
    // ServerParameterScript::C2/D2 own this exact map at +0xC8 and
    // importServerParameterScript fills it from [dungeon inout time].
    return dungeon_inout_times_;
}

// IDA_EA: 0x82694FA
// IDA_NAME: _ZN21ServerParameterScript21GetPvPLiveEventRewardEv
// DEMANGLED: ServerParameterScript::GetPvPLiveEventReward(void)
std::vector<std::pair<int, int> >*
ServerParameterScript::GetPvPLiveEventReward() {
    return &pvp_live_event_rewards_;
}

// IDA_EA: 0x8269508
// IDA_NAME: _ZN21ServerParameterScript24GetPvPLiveEventPlayCountEv
// DEMANGLED: ServerParameterScript::GetPvPLiveEventPlayCount(void)
int ServerParameterScript::GetPvPLiveEventPlayCount() const {
    return pvp_live_event_play_count_;
}

// IDA_EA: 0x82F03BC
// IDA_NAME: _ZN21ServerParameterScript15getAssaultPriceEi
// DEMANGLED: ServerParameterScript::getAssaultPrice(int)
int ServerParameterScript::getAssaultPrice(int charac_level) const {
    const IntVector& prices = assault_prices_;
    if (charac_level <= 0 || charac_level > 200) {
        return 0;
    }
    const unsigned int index = static_cast<unsigned int>(charac_level - 1);
    if (index >= prices.size()) {
        return 0;
    }
    return prices[index];
}

// IDA_EA: 0x82F03F8
// IDA_NAME: _ZN21ServerParameterScript27getAssaultUserRunawayNoticeEv
// DEMANGLED: ServerParameterScript::getAssaultUserRunawayNotice(void)
const char* ServerParameterScript::getAssaultUserRunawayNotice() const {
    return assault_user_runaway_notice_.c_str();
}

// IDA_EA: 0x82F0410
// IDA_NAME: _ZN21ServerParameterScript19getChaosPointWeightEi
// DEMANGLED: ServerParameterScript::getChaosPointWeight(int)
long double ServerParameterScript::getChaosPointWeight(int chaos_point) const {
    const FloatVector& weights = assault_chaos_point_weight_table_;
    if (chaos_point <= 0 || weights.empty()) {
        return 0.0L;
    }
    unsigned int index = static_cast<unsigned int>(chaos_point - 1);
    if (index >= weights.size()) {
        index = static_cast<unsigned int>(weights.size() - 1);
    }
    return weights[index];
}

// IDA_EA: 0x83703E2
// IDA_NAME: _ZNK21ServerParameterScript21CheckTraningFirstGiveEv
// DEMANGLED: ServerParameterScript::CheckTraningFirstGive(void)const
int ServerParameterScript::CheckTraningFirstGive() const {
    const IntIntMap& counts = training_first_give_count_by_grade_;
    if (counts.empty()) {
        return 0;
    }

    bool first = true;
    int first_count = 0;
    for (IntIntMap::const_iterator it = counts.begin();
         it != counts.end();
         ++it) {
        if (first) {
            first_count = it->second;
            first = false;
        } else if (it->second != first_count) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x84A60E2
// IDA_NAME: _ZNK21ServerParameterScript18GetTraningSealGiveEi
// DEMANGLED: ServerParameterScript::GetTraningSealGive(int)const
int ServerParameterScript::GetTraningSealGive(int pvp_grade) const {
    const IntIntMap& seal_give = training_seal_give_;
    if (seal_give.empty()) {
        return 0;
    }

    IntIntMap::const_iterator it = seal_give.find(pvp_grade);
    if (it != seal_give.end()) {
        return it->second;
    }
    it = seal_give.end();
    --it;
    return it->second;
}

// IDA_EA: 0x84A61C4
// IDA_NAME: _ZN21ServerParameterScript19GetTraningFirstGiveEiiiRSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: ServerParameterScript::GetTraningFirstGive(int,int,int,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &)
int ServerParameterScript::GetTraningFirstGive(
    int charac_job,
    int first_grow_type,
    int pvp_grade,
    std::vector<std::pair<int, int> >& reward) const {
    const IntIntMap& counts = training_first_give_count_by_grade_;
    IntIntMap::const_iterator count_it = counts.begin();
    while (count_it != counts.end()) {
        if (count_it->first >= pvp_grade) {
            pvp_grade = count_it->first;
            break;
        }
        ++count_it;
    }
    if (count_it == counts.end()) {
        --count_it;
        pvp_grade = count_it->first;
    }

    PvPPlayerKey key(static_cast<unsigned char>(charac_job),
                     static_cast<unsigned char>(first_grow_type),
                     static_cast<unsigned char>(pvp_grade));
    const PvpFirstGiveMap& first_give = training_first_give_;
    PvpFirstGiveMap::const_iterator it = first_give.find(key);
    if (it == first_give.end()) {
        return 0;
    }
    reward.insert(reward.end(), it->second.begin(), it->second.end());
    return 1;
}

int ServerParameterScript::get_training_seal_gain_count(
    bool relay_battle,
    bool won,
    bool bonus) const {
    const unsigned int index =
        (relay_battle ? 4u : 0u) + (bonus ? 2u : 0u) + (won ? 0u : 1u);
    return training_seal_gain_counts_[index];
}

uint16_t ServerParameterScript::get_training_seal_bonus_count() const {
    return training_seal_bonus_count_;
}

// IDA_EA: 0x8568AF6
// IDA_NAME: _ZN21ServerParameterScript26CheckUnusableEquipmentInfoE28ENUM_UNUSABLE_EQUIPMENT_TYPEi
// DEMANGLED: ServerParameterScript::CheckUnusableEquipmentInfo(ENUM_UNUSABLE_EQUIPMENT_TYPE,int)
int ServerParameterScript::CheckUnusableEquipmentInfo(
    int unusable_equipment_type,
    int usable_level) {
    UnusableEquipmentVector& values = unusable_equipment_info_;
    for (UnusableEquipmentVector::iterator it = values.begin();
         it != values.end();
         ++it) {
        if (it->equipment_kind == unusable_equipment_type) {
            return (it->minimum_level || it->maximum_level) &&
                   (it->minimum_level > usable_level ||
                    it->maximum_level < usable_level);
        }
    }
    return 1;
}

// IDA_EA: 0x8568BB0
// IDA_NAME: _ZN21ServerParameterScript18GetDailyMatchCountEv
// DEMANGLED: ServerParameterScript::GetDailyMatchCount(void)
int ServerParameterScript::GetDailyMatchCount() const {
    return daily_match_count_;
}

// IDA_EA: 0x8568BBE
// IDA_NAME: _ZNK21ServerParameterScript17CheckUnusableItemE18ENUM_EQUIPMENTTYPEm
// DEMANGLED: ServerParameterScript::CheckUnusableItem(ENUM_EQUIPMENTTYPE,unsigned long)const
int ServerParameterScript::CheckUnusableItem(
    int equipment_type,
    unsigned long item_id) const {
    const uint32_t item_id_32 = static_cast<uint32_t>(item_id);
    const UIntSet& unusable_items = unusable_items_;
    if (unusable_items.find(item_id_32) != unusable_items.end()) {
        return 0;
    }

    const IntUIntSetMap& usable_items = usable_items_by_equipment_type_;
    IntUIntSetMap::const_iterator type_it = usable_items.find(equipment_type);
    int result = 1;
    if (type_it != usable_items.end()) {
        if (type_it->second.find(item_id_32) == type_it->second.end()) {
            return 0;
        }
    }
    return result;
}

// IDA_EA: 0x85BEAF6
// IDA_NAME: _ZN21ServerParameterScript15GetPriceAverageEi
// DEMANGLED: ServerParameterScript::GetPriceAverage(int)
int ServerParameterScript::GetPriceAverage(int charac_level) const {
    const PairIntVector& values = blood_dungeon_reward_info_.price_average;
    if (values.empty()) {
        return 0;
    }
    for (PairIntVector::const_iterator it = values.begin();
         it != values.end();
         ++it) {
        if (it->first >= charac_level) {
            return it->second;
        }
    }
    return values.back().second;
}

// IDA_EA: 0x85BEBB6
// IDA_NAME: _ZN21ServerParameterScript20GetAdmissionIncreaseEi
// DEMANGLED: ServerParameterScript::GetAdmissionIncrease(int)
int ServerParameterScript::GetAdmissionIncrease(int enter_count) const {
    const PairIntVector& values =
        blood_dungeon_reward_info_.admission_increase;
    if (values.empty()) {
        return 0;
    }
    for (PairIntVector::const_iterator it = values.begin();
         it != values.end();
         ++it) {
        if (it->first > enter_count) {
            return it->second;
        }
    }
    return values.back().second;
}

// IDA_EA: 0x85BEC76
// IDA_NAME: _ZN21ServerParameterScript13GetLottePointEi
// DEMANGLED: ServerParameterScript::GetLottePoint(int)
int ServerParameterScript::GetLottePoint(int enter_count) const {
    const PairIntVector& values = blood_dungeon_reward_info_.lotto_point;
    if (values.empty()) {
        return 0;
    }
    for (PairIntVector::const_iterator it = values.begin();
         it != values.end();
         ++it) {
        if (it->first > enter_count) {
            return it->second;
        }
    }
    return values.back().second;
}

// IDA_EA: 0x85BED36
// IDA_NAME: _ZN21ServerParameterScript12GetLottoCostEv
// DEMANGLED: ServerParameterScript::GetLottoCost(void)
int ServerParameterScript::GetLottoCost() const {
    return lotto_cost_;
}

// IDA_EA: 0x85BED44
// IDA_NAME: _ZN21ServerParameterScript10GetJackpotEv
// DEMANGLED: ServerParameterScript::GetJackpot(void)
int ServerParameterScript::GetJackpot() const {
    return jackpot_;
}

// IDA_EA: 0x86938A2
// IDA_NAME: _ZN21ServerParameterScript22getRecoverStaminaPriceEii
// DEMANGLED: ServerParameterScript::getRecoverStaminaPrice(int,int)
int ServerParameterScript::getRecoverStaminaPrice(
    int charac_level,
    int discount_percent) const {
    const IntVector& prices = recover_stamina_prices_;
    const int size = static_cast<int>(prices.size());
    const int index = std::min(size, charac_level) - 1;
    const int base_price = prices[index];
    return base_price * (100 - discount_percent) / 90;
}

// IDA_EA: 0x8693918
// IDA_NAME: _ZN21ServerParameterScript21GetPvPPenaltyRevisionEi
// DEMANGLED: ServerParameterScript::GetPvPPenaltyRevision(int)
long double ServerParameterScript::GetPvPPenaltyRevision(int pvp_grade) const {
    const IntIntMap& revisions = pvp_exp_penalty_revisions_;
    IntIntMap::const_iterator it = revisions.find(pvp_grade);
    if (it != revisions.end()) {
        return static_cast<long double>(it->second) / 100.0L;
    }
    return 1.0L;
}

// IDA_EA: 0x8693994
// IDA_NAME: _ZNK21ServerParameterScript25GetPvPChannelGradePenaltyEi
// DEMANGLED: ServerParameterScript::GetPvPChannelGradePenalty(int)const
int ServerParameterScript::GetPvPChannelGradePenalty(int pvp_grade) const {
    const IntIntMap& penalties = pvp_channel_grade_penalties_;
    if (penalties.empty()) {
        return 0;
    }
    IntIntMap::const_iterator it = penalties.find(pvp_grade);
    if (it != penalties.end()) {
        return it->second;
    }
    if (penalties.size() <= 2u) {
        return 0;
    }
    it = penalties.end();
    --it;
    return it->second;
}

// IDA_EA: 0x86D17EC
// IDA_NAME: _ZN21ServerParameterScript21GetCleanChattingCountEi
// DEMANGLED: ServerParameterScript::GetCleanChattingCount(int)
int ServerParameterScript::GetCleanChattingCount(int charac_level) {
    PairIntVector& values = clean_chatting_counts_;
    if (values.empty()) {
        return 0;
    }
    for (PairIntVector::const_iterator it = values.begin();
         it != values.end();
         ++it) {
        if (it->first >= charac_level) {
            return it->second;
        }
    }
    return values.back().second;
}

// IDA_EA: 0x86D18AC
// IDA_NAME: _ZN21ServerParameterScript24GetMaxCleanChattingCountEv
// DEMANGLED: ServerParameterScript::GetMaxCleanChattingCount(void)
int ServerParameterScript::GetMaxCleanChattingCount() {
    return max_clean_chatting_count_;
}

// IDA_EA: 0x86D18BA
// IDA_NAME: _ZN21ServerParameterScript20GetPaneltyDupMessageEv
// DEMANGLED: ServerParameterScript::GetPaneltyDupMessage(void)
int ServerParameterScript::GetPaneltyDupMessage() {
    return duplicate_message_penalty_;
}

// IDA_EA: 0x8A8C28C
// IDA_NAME: _ZNK21ServerParameterScript17GetExpBonusWeightE15eRankBonusIndex
// DEMANGLED: ServerParameterScript::GetExpBonusWeight(eRankBonusIndex)const
long double ServerParameterScript::GetExpBonusWeight(
    unsigned int rank_bonus_index) const {
    if (rank_bonus_index > 4u) {
        return 0.0L;
    }
    return clear_rank_exp_bonus_rates_[rank_bonus_index];
}

float ServerParameterScript::GetClearDungeonPartyExpWeight(
    int member_count) const {
    if (member_count < 0 || member_count > 4) {
        return 0.0f;
    }
    return member_count == 0
               ? monster_exp_bonus_rate_
               : party_user_number_exp_bonus_rates_[member_count - 1];
}

float ServerParameterScript::GetClearDungeonDifficultyExpWeight(
    int difficulty) const {
    if (difficulty < 0 || difficulty > 4) {
        return 0.0f;
    }
    return dungeon_difficulty_exp_bonus_rates_[difficulty];
}

// IDA_EA: 0x8A8C2C0
// IDA_NAME: _ZN21ServerParameterScript34CheckCollectDungeonStatisticServerEii
// DEMANGLED: ServerParameterScript::CheckCollectDungeonStatisticServer(int,int)
int ServerParameterScript::CheckCollectDungeonStatisticServer(
    int server_group,
    int channel_no) const {
    stServerIndex index;
    index.server_group = server_group;
    index.channel_no = channel_no;
    const ServerIndexBoolMap& servers = collect_dungeon_statistic_servers_;
    ServerIndexBoolMap::const_iterator it = servers.find(index);
    if (it != servers.end()) {
        return it->second;
    }
    return 0;
}

// IDA_EA: 0x8A8C372
// IDA_NAME: _ZN21ServerParameterScript16CheckLightServerEi
// DEMANGLED: ServerParameterScript::CheckLightServer(int)
int ServerParameterScript::CheckLightServer(int server_group) {
    bool light_server = false;
    IntBoolMap& servers = light_servers_;
    IntBoolMap::iterator it = servers.find(server_group);
    if (it != servers.end()) {
        light_server = it->second;
        SetLightServerFlag(light_server);
    }
    return light_server;
}

// IDA_EA: 0x8A8C3EE
// IDA_NAME: _ZN21ServerParameterScript18checkAssaultServerEii
// DEMANGLED: ServerParameterScript::checkAssaultServer(int,int)
int ServerParameterScript::checkAssaultServer(int server_group,
                                              int channel_no) const {
    const PairIntVector& servers = assault_servers_;
    for (PairIntVector::const_iterator it = servers.begin();
         it != servers.end();
         ++it) {
        if (it->first == server_group && it->second == channel_no) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8A8C486
// IDA_NAME: _ZN21ServerParameterScript12getLuckPointEiRi
// DEMANGLED: ServerParameterScript::getLuckPoint(int,int &)
int ServerParameterScript::getLuckPoint(int point, int& luck_point) const {
    const LuckPointVector& values = luck_points_;
    for (unsigned int index = 0; index < values.size(); ++index) {
        if (values[index].point_threshold >= point) {
            luck_point = values[index].awarded_luck_point;
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8A8C4FE
// IDA_NAME: _ZN21ServerParameterScript25getLuckPointDeductionRateEii
// DEMANGLED: ServerParameterScript::getLuckPointDeductionRate(int,int)
int ServerParameterScript::getLuckPointDeductionRate(int charac_level,
                                                     int point) const {
    const LuckPointDeductionVector& rates = rare_luck_point_deductions_;
    const IntVector& gaps = luck_point_deduction_gaps_;
    for (unsigned int index = 0; index < rates.size(); ++index) {
        if (rates[index].maximum_character_level >= charac_level) {
            for (int gap_index = 0; gap_index <= 2; ++gap_index) {
                if (static_cast<unsigned int>(gap_index) < gaps.size() &&
                    static_cast<unsigned int>(gap_index) <
                        rates[index].deduction_rates.size() &&
                    gaps[gap_index] <= point) {
                    return rates[index].deduction_rates[gap_index];
                }
            }
            if (rates[index].deduction_rates.size() > 3u) {
                return rates[index].deduction_rates[3];
            }
            return 0;
        }
    }
    return 0;
}

int ServerParameterScript::getPremiumGoldCardRarityDecision(int rarity) const {
    if (rarity < 0 || rarity > 5) {
        return 0;
    }
    return premium_gold_card_event_.rarity_decision[rarity];
}

bool ServerParameterScript::getPremiumGoldCardDropRule(
    int index,
    int& minimum_level,
    int& maximum_level,
    int& probability,
    unsigned int& item_id) const {
    if (index < 0 || index >= 10) {
        return false;
    }

    const stLevelCond4PremiumGoldCard& rule =
        premium_gold_card_event_.drop_rules[index];
    minimum_level = rule.minimum_level;
    maximum_level = rule.maximum_level;
    probability = rule.probability;
    item_id = static_cast<unsigned int>(rule.item_id);
    return true;
}

// IDA_EA: 0x8A8C5F2
// IDA_NAME: _ZN21ServerParameterScript25GetEscaladeTutorialRewardEj
// DEMANGLED: ServerParameterScript::GetEscaladeTutorialReward(unsigned int)
std::vector<std::pair<int, int> >*
ServerParameterScript::GetEscaladeTutorialReward(unsigned int tutorial_id) {
    IntPairVectorMap& rewards = escalade_tutorial_rewards_;
    IntPairVectorMap::iterator it =
        rewards.find(static_cast<int>(tutorial_id));
    if (it != rewards.end()) {
        return &it->second;
    }
    return 0;
}

// IDA_EA: 0x8A8C668
// IDA_NAME: _ZN21ServerParameterScript31getAssaultPriceChaospointWeightEi
// DEMANGLED: ServerParameterScript::getAssaultPriceChaospointWeight(int)
long double ServerParameterScript::getAssaultPriceChaospointWeight(
    int chaos_point) const {
    const AssaultChaosPointWeightVector& weights =
        assault_chaos_point_weights_;
    if (weights.empty()) {
        return 1.0L;
    }
    if (chaos_point > 1000) {
        chaos_point = 1000;
    }

    stAssaultChaosPointWeight selected;
    for (AssaultChaosPointWeightVector::const_iterator it = weights.begin();
         it != weights.end();
         ++it) {
        if (it->min <= chaos_point && it->max >= chaos_point) {
            selected = *it;
            break;
        }
    }
    if (selected.from_weight == 0.0f || selected.max == selected.min) {
        return 1.0L;
    }

    float result = selected.from_weight +
                   (selected.to_weight - selected.from_weight) /
                       static_cast<long double>(selected.max - selected.min) *
                       static_cast<long double>(chaos_point - selected.min);
    if (result < selected.from_weight) {
        return selected.from_weight;
    }
    if (result > selected.to_weight) {
        return selected.to_weight;
    }
    return result;
}

// IDA_EA: 0x8A8C804
// IDA_NAME: _ZN21ServerParameterScript19getAssaultGhostTimeEii
// DEMANGLED: ServerParameterScript::getAssaultGhostTime(int,int)
int ServerParameterScript::getAssaultGhostTime(int charac_level,
                                               int chaos_point) const {
    const AssaultLevelGhostPanaltyVector& level_penalties =
        assault_level_ghost_penalties_;
    if (level_penalties.empty()) {
        return 180;
    }

    stAssaultLevelGhostPanalty level_selected;
    for (AssaultLevelGhostPanaltyVector::const_iterator it =
             level_penalties.begin();
         it != level_penalties.end();
         ++it) {
        if (it->min_level <= charac_level && it->max_level >= charac_level) {
            level_selected = *it;
            break;
        }
    }
    unsigned int ghost_time = 0;
    if (level_selected.max_level != level_selected.min_level) {
        ghost_time =
            level_selected.min_time +
            (level_selected.max_time - level_selected.min_time) /
                (level_selected.max_level - level_selected.min_level) *
                (charac_level - level_selected.min_level);
    }
    if (static_cast<unsigned int>(level_selected.min_time) <= ghost_time) {
        if (static_cast<unsigned int>(level_selected.max_time) < ghost_time) {
            ghost_time = static_cast<unsigned int>(level_selected.max_time);
        }
    } else {
        ghost_time = static_cast<unsigned int>(level_selected.min_time);
    }

    const AssaultChaosPointGhostPanaltyVector& chaos_penalties =
        assault_chaos_point_ghost_penalties_;
    if (chaos_penalties.empty()) {
        return ghost_time / 1000u;
    }
    if (chaos_point > 1000) {
        chaos_point = 1000;
    }

    stAssaultChaosPointGhostPanalty chaos_selected;
    for (AssaultChaosPointGhostPanaltyVector::const_iterator it =
             chaos_penalties.begin();
         it != chaos_penalties.end();
         ++it) {
        if (it->min <= chaos_point && it->max >= chaos_point) {
            chaos_selected = *it;
            break;
        }
    }
    float weight = 1.0f;
    if (chaos_selected.max != chaos_selected.min) {
        weight = chaos_selected.from_weight +
                 (chaos_selected.to_weight - chaos_selected.from_weight) /
                     static_cast<long double>(chaos_selected.max -
                                              chaos_selected.min) *
                     static_cast<long double>(chaos_point -
                                              chaos_selected.min);
    }
    if (weight < chaos_selected.from_weight) {
        weight = chaos_selected.from_weight;
    } else if (weight > chaos_selected.to_weight) {
        weight = chaos_selected.to_weight;
    }
    return static_cast<unsigned int>(
               static_cast<long double>(ghost_time) * weight) /
           1000u;
}

// IDA_EA: 0x8A870B7
// IDA_NAME: _Z27importServerParameterScriptP21ServerParameterScriptPKc
// DEMANGLED: importServerParameterScript(ServerParameterScript *,char const*)
int importServerParameterScript(ServerParameterScript* script,
                                const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    script->clear();

    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[nation code]") {
            ScanInt(static_cast<bool*>(0));
        } else if (tag == "[script version]") {
            script->script_version_ = ScanInt(static_cast<bool*>(0));
            g_new_scriptVersion = script->script_version_;
        } else if (tag == "[version]") {
            for (int index = 0; index <= 3; ++index) {
                script->version_digits_[index] =
                    static_cast<unsigned char>(
                        ScanInt(static_cast<bool*>(0)));
            }
        } else if (tag == "[clear exp bonusrate]") {
            script->clear_exp_bonus_rate_ =
                ScanFloat(static_cast<bool*>(0));
        } else if (tag == "[monster exp bonusrate]") {
            script->monster_exp_bonus_rate_ =
                ScanFloat(static_cast<bool*>(0));
        } else if (tag == "[party user number exp bonusrate]") {
            for (int index = 0; index <= 3; ++index) {
                script->party_user_number_exp_bonus_rates_[index] =
                    ScanFloat(static_cast<bool*>(0));
            }
        } else if (tag ==
                   "[party user number exp bonusrate starter server]") {
            for (int index = 0; index <= 3; ++index) {
                script->starter_server_party_exp_bonus_rates_[index] =
                    ScanFloat(static_cast<bool*>(0));
            }
        } else if (tag == "[quest item drop bonus starter server]") {
            for (int index = 0; index <= 3; ++index) {
                script->starter_server_quest_item_drop_bonus_rates_[index] =
                    ScanFloat(static_cast<bool*>(0));
            }
        } else if (tag == "[dungeon difficulty exp bonusrate]") {
            for (int index = 0; index <= 4; ++index) {
                script->dungeon_difficulty_exp_bonus_rates_[index] =
                    ScanFloat(static_cast<bool*>(0));
            }
        } else if (tag == "[clear rank exp bonusrate]") {
            for (int index = 0; index <= 4; ++index) {
                script->clear_rank_exp_bonus_rates_[index] =
                    ScanFloat(static_cast<bool*>(0));
            }
        } else if (tag == "[drop prob number]") {
            sItemGenRef value;
            memset(&value, 0, sizeof(value));
            script->dungeon_drop_probabilities_
                .resize(static_cast<unsigned int>(
                            ScanInt(static_cast<bool*>(0))),
                        value);
        } else if (tag == "[drop prob]") {
            ItemGenRefVector& values = script->dungeon_drop_probabilities_;
            for (unsigned int index = 0; index < values.size(); ++index) {
                values[index].value_00 = ScanInt(static_cast<bool*>(0));
                values[index].value_04 = ScanInt(static_cast<bool*>(0));
                values[index].value_08 = ScanInt(static_cast<bool*>(0));
                values[index].rate = ScanInt(static_cast<bool*>(0));
                values[index].value_0c = 1000;
            }
        } else if (tag == "[dungeon difficulty drop bonusrate]") {
            for (int index = 0; index <= 4; ++index) {
                script->dungeon_difficulty_drop_bonus_rates_[index] =
                    ScanFloat(static_cast<bool*>(0));
            }
        } else if (tag == "[party user number drop bonusrate]") {
            for (int index = 0; index <= 3; ++index) {
                script->party_user_number_drop_bonus_rates_[index] =
                    ScanFloat(static_cast<bool*>(0));
            }
        } else if (tag == "[drop bonusrate of monster kind]") {
            for (int index = 0; index <= 3; ++index) {
                script->monster_kind_drop_bonus_rates_[index] =
                    ScanFloat(static_cast<bool*>(0));
            }
        } else if (tag == "[result reward prob number]") {
            sItemGenRef value;
            memset(&value, 0, sizeof(value));
            script->result_reward_probabilities_
                .resize(static_cast<unsigned int>(
                            ScanInt(static_cast<bool*>(0))),
                        value);
        } else if (tag == "[result reward prob]") {
            ItemGenRefVector& values = script->result_reward_probabilities_;
            for (unsigned int index = 0; index < values.size(); ++index) {
                values[index].value_00 = ScanInt(static_cast<bool*>(0));
                values[index].value_04 = ScanInt(static_cast<bool*>(0));
                values[index].value_08 = 0;
                values[index].rate = ScanInt(static_cast<bool*>(0));
                values[index].value_0c = 1000;
            }
        } else if (tag == "[dungeon difficulty reward bonusrate]") {
            for (int index = 0; index <= 4; ++index) {
                script->dungeon_difficulty_reward_bonus_rates_[index] =
                    ScanFloat(static_cast<bool*>(0));
            }
        } else if (tag == "[party user number reward bonusrate]") {
            for (int index = 0; index <= 3; ++index) {
                script->party_user_number_reward_bonus_rates_[index] =
                    ScanFloat(static_cast<bool*>(0));
            }
        } else if (tag == "[basis of item dicision]") {
            script->item_decision_basis_ = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[basis of equipment rarity dicision]") {
            for (int index = 0; index <= 3; ++index) {
                script->equipment_rarity_decision_basis_[index] =
                    ScanInt(static_cast<bool*>(0));
            }
        } else if (tag == "[basis of stackable rarity dicision]") {
            for (int index = 0; index <= 3; ++index) {
                script->stackable_rarity_decision_basis_[index] =
                    ScanInt(static_cast<bool*>(0));
            }
        } else if (tag == "[collect dungeon statistic server]") {
            ServerIndexBoolMap& servers =
                script->collect_dungeon_statistic_servers_;
            while (1) {
                stServerIndex index;
                index.server_group = ScanInt(&success);
                if (!success) {
                    break;
                }
                index.channel_no = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                servers.insert(std::make_pair(index, true));
            }
        } else if (tag == "[stamina recovery cost]") {
            IntVector& prices = script->recover_stamina_prices_;
            prices.clear();
            while (1) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                prices.push_back(value);
            }
        } else if (tag == "[assault cost]") {
            IntVector& prices = script->assault_prices_;
            prices.clear();
            while (1) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                prices.push_back(value);
            }
        } else if (tag == "[assault channel]") {
            PairIntVector& servers = script->assault_servers_;
            while (1) {
                const int server_group = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int channel_no = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                servers.push_back(std::make_pair(server_group, channel_no));
            }
        } else if (tag == "[assault user runaway notice]") {
            success = ScanStr(&script->assault_user_runaway_notice_) == 1;
            if (!success) {
                return 0;
            }
        } else if (tag == "[increase abuse point]") {
            script->increase_abuse_point_ = ScanInt(&success);
            if (!success) {
                return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
            }
        } else if (tag == "[decrease abuse point]") {
            script->decrease_abuse_point_ = ScanInt(&success);
            if (!success) {
                return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
            }
        } else if (tag == "[abuse point penalty]") {
            script->abuse_point_penalty_ = ScanInt(&success);
            if (!success) {
                return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
            }
        } else if (tag == "[decrease experience point]") {
            script->decrease_experience_point_ = ScanInt(&success);
            if (!success) {
                return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
            }
        } else if (tag == "[upgrade with legacy]") {
            LotteryUpgradePolicy& policy = script->lottery_upgrade_policy_;
            policy.first_upgrade_base = ScanInt(static_cast<bool*>(0));
            policy.first_upgrade_ceiling = ScanInt(static_cast<bool*>(0));
            policy.first_band_probability = ScanInt(static_cast<bool*>(0));
            policy.first_band_roll_range =
                1000 * (policy.first_upgrade_ceiling -
                        policy.first_upgrade_base) +
                999;
            policy.second_upgrade_base = ScanInt(static_cast<bool*>(0));
            policy.second_upgrade_ceiling = ScanInt(static_cast<bool*>(0));
            policy.second_band_probability = ScanInt(static_cast<bool*>(0));
            policy.second_band_roll_range =
                1000 * (policy.second_upgrade_ceiling -
                        policy.second_upgrade_base) +
                999;
            policy.third_upgrade_base = ScanInt(static_cast<bool*>(0));
            policy.third_upgrade_ceiling = ScanInt(static_cast<bool*>(0));
            policy.third_band_probability = ScanInt(static_cast<bool*>(0));
            policy.third_band_roll_range =
                1000 * (policy.third_upgrade_ceiling -
                        policy.third_upgrade_base) +
                999;
        } else if (tag == "[notice legacy with rarity]") {
            script->lottery_upgrade_policy_.broadcast_minimum_rarity =
                ScanInt(&success);
            if (!success) {
                return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
            }
        } else if (tag == "[notice legacy with type]") {
            memset(script->lottery_upgrade_policy_.broadcast_avatar_item_types,
                   0,
                   sizeof(script->lottery_upgrade_policy_
                              .broadcast_avatar_item_types));
            std::string type_text;
            while (ScanStr(&type_text) == 1) {
                const int type = GetEquipmentTypeStringToEnum(type_text);
                script->lottery_upgrade_policy_
                    .broadcast_avatar_item_types[type] = 1;
            }
        } else if (tag == "[notice legacy with upgrade]") {
            script->lottery_upgrade_policy_.broadcast_minimum_upgrade =
                ScanInt(&success);
            if (!success) {
                return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
            }
        } else if (tag == "[light server no]") {
            IntBoolMap& servers = script->light_servers_;
            while (1) {
                const int server_no = ScanInt(&success);
                if (!success) {
                    break;
                }
                servers.insert(std::make_pair(server_no, true));
            }
        } else if (tag == "[burning fatigue event param]") {
            BurningFatigueParamVector& values =
                script->burning_fatigue_parameters_;
            while (1) {
                stBurningFatigueParam_t value;
                value.fatigue =
                    static_cast<uint16_t>(ScanInt(&success));
                if (!success) {
                    break;
                }
                value.bonus_exp_rate = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                value.refill_fatigue =
                    static_cast<uint16_t>(ScanInt(&success));
                if (!success) {
                    return 0;
                }
                value.value_10 = ScanFloat(&success);
                if (!success) {
                    return 0;
                }
                value.value_14 = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                value.value_0c = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                for (int index = 0; index <= 3; ++index) {
                    value.raw_18[index] =
                        static_cast<unsigned char>(ScanInt(&success));
                    if (!success) {
                        return 0;
                    }
                }
                values.push_back(value);
            }
        } else if (tag == "[fatigue event param]") {
            FatigueEventParamVector& values =
                script->fatigue_event_parameters_;
            while (1) {
                stFatigueEventParam_t value;
                value.word_00 =
                    static_cast<uint16_t>(ScanInt(&success));
                if (!success) {
                    break;
                }
                value.dword_04 = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                for (int index = 0; index <= 3; ++index) {
                    value.bytes_08[index] =
                        static_cast<unsigned char>(ScanInt(&success));
                    if (!success) {
                        return 0;
                    }
                }
                values.push_back(value);
            }
        } else if (tag == "[light server notice]") {
            LightServerNoticeVector& notices =
                script->light_server_notices_;
            while (1) {
                stLightServerNotice_t notice;
                notice.server_group = ScanInt(&success);
                if (!success) {
                    break;
                }
                notice.channel_no = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                success = ScanStr(&notice.message) == 1;
                if (!success) {
                    return 0;
                }
                notices.push_back(notice);
            }
        } else if (tag == "[assault term panalty]") {
            script->assault_price_time_weight_limit_ =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[assault term panalty keep up time]") {
            script->assault_price_time_limit_seconds_ =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[assault level panalty]") {
            script->assault_price_level_gap_weight_ =
                ScanFloat(static_cast<bool*>(0));
        } else if (tag == "[assault chaos point weight]") {
            FloatVector& weights =
                script->assault_chaos_point_weight_table_;
            weights.clear();
            while (1) {
                const float value = ScanFloat(&success);
                if (!success) {
                    break;
                }
                if (weights.size()) {
                    for (int index = 0; index <= 4; ++index) {
                        weights.push_back(value);
                        if (weights.size() > 0x63u) {
                            break;
                        }
                    }
                } else {
                    weights.push_back(value);
                    weights.push_back(value);
                }
            }
        } else if (tag == "[luck point]") {
            LuckPointVector& values = script->luck_points_;
            while (1) {
                ScanInt(&success);
                if (!success) {
                    break;
                }
                stLuckPoint_t value;
                value.point_threshold = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                value.awarded_luck_point = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                values.push_back(value);
            }
        } else if (tag == "[luck point deduction rare]") {
            LuckPointDeductionVector& values =
                script->rare_luck_point_deductions_;
            while (1) {
                ScanInt(&success);
                if (!success) {
                    break;
                }
                stLuckPointDeduction_t value;
                value.maximum_character_level = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                value.deduction_rates.reserve(4u);
                for (int index = 0; index <= 3; ++index) {
                    const int rate = ScanInt(&success);
                    value.deduction_rates.push_back(rate);
                    if (!success) {
                        return 0;
                    }
                }
                values.push_back(value);
            }
        } else if (tag == "[luck point deduction unique]") {
            LuckPointDeductionVector& values =
                script->unique_luck_point_deductions_;
            while (1) {
                ScanInt(&success);
                if (!success) {
                    break;
                }
                stLuckPointDeduction_t value;
                value.maximum_character_level = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                value.deduction_rates.reserve(4u);
                for (int index = 0; index <= 3; ++index) {
                    const int rate = ScanInt(&success);
                    value.deduction_rates.push_back(rate);
                    if (!success) {
                        return 0;
                    }
                }
                values.push_back(value);
            }
        } else if (tag == "[luck point deduction gap]") {
            IntVector& gaps = script->luck_point_deduction_gaps_;
            gaps.reserve(4u);
            for (int index = 0; index <= 2; ++index) {
                ScanInt(&success);
                if (!success) {
                    break;
                }
                const int gap = ScanInt(&success);
                gaps.push_back(gap);
                if (!success) {
                    return 0;
                }
            }
            ScanInt(&success);
            if (!success) {
                return 0;
            }
            ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[point per fatigue]") {
            script->point_per_fatigue_ = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[event point per fatigue]") {
            script->event_point_per_fatigue_ =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[point per play time]") {
            script->point_per_play_time_ = ScanInt(static_cast<bool*>(0));
            script->play_time_for_point_ = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[black list help parameter]") {
            script->black_list_help_period_days_ =
                ScanInt(static_cast<bool*>(0));
            script->black_list_help_points_per_period_ =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[unify server prefix]") {
            script->prevent_prefix_count_ = ScanInt(static_cast<bool*>(0));
            for (int index = 0; index < script->prevent_prefix_count_;
                 ++index) {
                ScanStr(script->prevent_prefixes_[index],
                        sizeof(script->prevent_prefixes_[index]));
            }
        } else if (tag == "[damaged equipment ratio]") {
            script->damaged_equipment_ratio_ =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[limit fatigue battery charging amount]") {
            script->get_fatigue_battery_constant_table()
                .limit_fatigue_battery_charging_amount_ =
                static_cast<uint16_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[end level fatigue battery weight]") {
            script->get_fatigue_battery_constant_table()
                .end_level_fatigue_battery_weight_ =
                ScanFloat(static_cast<bool*>(0));
        } else if (tag == "[character level fatigue battery constance]") {
            script->get_fatigue_battery_constant_table()
                .character_level_fatigue_battery_constance_ =
                static_cast<unsigned char>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[limit fatigue battery level]") {
            script->get_fatigue_battery_constant_table()
                .limit_fatigue_battery_level_ =
                static_cast<unsigned char>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[fatigue battery money amount]") {
            script->get_fatigue_battery_constant_table()
                .fatigue_battery_money_amount_ =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[fatigue battery level for money active]") {
            script->get_fatigue_battery_constant_table()
                .fatigue_battery_level_for_money_active_ =
                static_cast<unsigned char>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[min fatigue battery per buff active]") {
            script->get_fatigue_battery_constant_table()
                .min_fatigue_battery_per_buff_active_ =
                static_cast<uint16_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[limit grownup buff cnt fatigue battery]") {
            for (int index = 0;; ++index) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->get_fatigue_battery_constant_table()
                    .limit_grownup_buff_count_by_level_[index] =
                    static_cast<uint16_t>(value);
            }
        } else if (tag == "[fatigue battery per grownup buff active]") {
            script->get_fatigue_battery_constant_table()
                .fatigue_battery_per_grownup_buff_active_ =
                static_cast<uint16_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[grownup buff per fatigue battery active]") {
            script->get_fatigue_battery_constant_table()
                .grownup_buff_per_fatigue_battery_active_ =
                static_cast<uint16_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[pc room party exp bonus rate]") {
            script->fatigue_battery_constant_table_
                .pc_room_party_exp_bonus_rate_ = ScanFloat(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[escalade tutorial reward]") {
            IntPairVectorMap& rewards =
                script->escalade_tutorial_rewards_;
            while (1) {
                const int tutorial_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int item_id = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                const int item_count = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                rewards[tutorial_id].push_back(
                    std::make_pair(item_id, item_count));
            }
        } else if (tag == "[server message index onoff]") {
            std::vector<int>& values =
                script->shared_server_message_info_.onoff;
            while (1) {
                const int index = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int on = ScanInt(&success);
                if (!success) {
                    break;
                }
                if (on == 1) {
                    values.push_back(index);
                }
            }
        } else if (tag == "[server message index kind]") {
            PairIntVector& values =
                script->shared_server_message_info_.index_kind;
            while (1) {
                const int first = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int second = ScanInt(&success);
                if (!success) {
                    break;
                }
                values.push_back(std::make_pair(first, second));
            }
        } else if (tag == "[server message event kind]") {
            PairIntVector& values =
                script->shared_server_message_info_.event_kind;
            while (1) {
                const int first = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int second = ScanInt(&success);
                if (!success) {
                    break;
                }
                values.push_back(std::make_pair(first, second));
            }
        } else if (tag == "[price average]") {
            PairIntVector& values =
                script->blood_dungeon_reward_info_.price_average;
            while (1) {
                ScanInt(&success);
                if (!success) {
                    break;
                }
                const int level = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                const int price = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                values.push_back(std::make_pair(level, price));
            }
        } else if (tag == "[lotto cost]") {
            script->lotto_cost_ = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[jackpot]") {
            script->jackpot_ = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[admission increase]") {
            PairIntVector& values =
                script->blood_dungeon_reward_info_.admission_increase;
            while (1) {
                const int first = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int second = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                values.push_back(std::make_pair(first, second));
            }
        } else if (tag == "[lotto point]") {
            PairIntVector& values =
                script->blood_dungeon_reward_info_.lotto_point;
            while (1) {
                const int first = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int second = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                values.push_back(std::make_pair(first, second));
            }
        } else if (tag == "[emblem compound info]") {
            while (1) {
                const int rarity_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                EmblemCompoundInfo& info =
                    script->emblem_compound_info_[rarity_index];
                info.money_cost = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                for (int index = 0; index <= 4; ++index) {
                    info.result_item_ids[index] = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                }
            }
        } else if (tag == "[recipe upgrade table]") {
            for (int row = 0; row <= 31; ++row) {
                ScanInt(&success);
                for (int column = 0; column <= 31; ++column) {
                    script->recipe_upgrade_table_[row].rates[column] =
                        ScanInt(&success);
                    if (!success) {
                        break;
                    }
                }
            }
            ScanType(tag, true);
        } else if (tag == "[recipe amplify table]") {
            for (int row = 0; row <= 31; ++row) {
                ScanInt(&success);
                for (int column = 0; column <= 31; ++column) {
                    script->recipe_amplify_table_[row].rates[column] =
                        ScanInt(&success);
                    if (!success) {
                        break;
                    }
                }
            }
            ScanType(tag, true);
        } else if (tag == "[recipe upgrade table 2nd]") {
            for (int row = 0; row <= 31; ++row) {
                ScanInt(&success);
                for (int column = 0; column <= 31; ++column) {
                    script->recipe_upgrade_table_second_[row].rates[column] =
                        ScanInt(&success);
                    if (!success) {
                        break;
                    }
                }
            }
            ScanType(tag, true);
        } else if (tag == "[recipe amplify table 2nd]") {
            for (int row = 0; row <= 31; ++row) {
                ScanInt(&success);
                for (int column = 0; column <= 31; ++column) {
                    script->recipe_amplify_table_second_[row].rates[column] =
                        ScanInt(&success);
                    if (!success) {
                        break;
                    }
                }
            }
            ScanType(tag, true);
        } else if (tag == "[full level reward]") {
            IntUIntMap& rewards = script->full_level_rewards_;
            while (1) {
                const int level = ScanInt(&success);
                if (!success) {
                    break;
                }
                const uint32_t item_id =
                    static_cast<uint32_t>(ScanInt(&success));
                if (!success) {
                    return 0;
                }
                rewards.insert(std::make_pair(level, item_id));
            }
        } else if (tag == "[ontime event reward]") {
            PairIntVector& rewards = script->ontime_event_rewards_;
            while (1) {
                const int item_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int item_count = ScanInt(&success);
                if (!success) {
                    break;
                }
                rewards.push_back(std::make_pair(item_id, item_count));
            }
        } else if (tag == "[upper bind cube rate bonus]") {
            script->compound_avatar_upper_rate_bonus_ = ScanInt(&success);
            if (!success) {
                return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
            }
        } else if (tag == "[lower bind cube rate bonus]") {
            script->compound_avatar_lower_rate_reduction_ = ScanInt(&success);
            if (!success) {
                return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
            }
        // 0828 PVF extension.  The English tag and the matching
        // [material_master] entry in the avatar compound tables prove that
        // this is a third independent bind-cube probability modifier.
        // There is no 0627 server IDA return-code/address evidence for this
        // branch, so its validation errors remain a generic parser failure.
        } else if (tag == "[master bind cube rate bonus]") {
            script->compound_avatar_master_rate_bonus_ = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[broadcastitem]") {
            BroadcastItemIdVector& item_ids = script->broadcast_item_ids_;
            while (1) {
                const int item_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                item_ids.push_back(static_cast<uint32_t>(item_id));
            }
        } else if (tag == "[item give in time]") {
            TimedItemGiveEntryVector& entries = script->timed_item_gives_;
            while (1) {
                TimedItemGiveEntry entry;
                if (ScanStr(&entry.begin_time_text) != 1) {
                    break;
                }
                if (ScanStr(&entry.end_time_text) != 1) {
                    return 0;
                }
                entry.item_id = static_cast<uint32_t>(ScanInt(&success));
                if (!success) {
                    return 0;
                }
                entry.count = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                entry.raw_value = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                entries.push_back(entry);
            }
        } else if (tag == "[pvp live event play count]") {
            script->pvp_live_event_play_count_ = ScanInt(&success);
            if (!success) {
                return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
            }
        } else if (tag == "[pvp live event reward]") {
            PairIntVector& rewards = script->pvp_live_event_rewards_;
            while (1) {
                const int first = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int second = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                rewards.push_back(std::make_pair(first, second));
            }
        } else if (tag == "[pvp exp penalty revision]") {
            IntIntMap& revisions = script->pvp_exp_penalty_revisions_;
            while (1) {
                const int grade = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int revision = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                revisions.insert(std::make_pair(grade, revision));
            }
        } else if (tag == "[unusable equipment info]") {
            UnusableEquipmentVector& values =
                script->unusable_equipment_info_;
            while (1) {
                std::string type_text;
                if (ScanStr(&type_text) != 1) {
                    break;
                }
                stUnusableEquipmentInfo value;
                value.equipment_kind = 0;
                if (type_text == "relic") {
                    value.equipment_kind = 2;
                } else if (type_text == "unique") {
                    value.equipment_kind = 3;
                } else if (type_text == "choronicle") {
                    value.equipment_kind = 4;
                } else if (type_text == "epic") {
                    value.equipment_kind = 5;
                } else if (type_text == "rare_avatar") {
                    value.equipment_kind = 6;
                } else if (type_text == "common") {
                    value.equipment_kind = 7;
                } else if (type_text == "uncommon") {
                    value.equipment_kind = 8;
                } else if (type_text == "rare") {
                    value.equipment_kind = 9;
                } else if (type_text == "rare_creature") {
                    value.equipment_kind = 10;
                } else if (type_text == "real_rare") {
                    value.equipment_kind = 1;
                } else {
                    return 0;
                }
                value.minimum_level = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                value.maximum_level = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                values.push_back(value);
            }
        } else if (tag == "[daily match count]") {
            script->daily_match_count_ = ScanInt(&success);
            if (!success) {
                return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
            }
        } else if (tag == "[unusable items]") {
            UIntSet& items = script->unusable_items_;
            while (1) {
                const int item_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                items.insert(static_cast<uint32_t>(item_id));
            }
        } else if (tag == "[usable items]") {
            IntUIntSetMap& items =
                script->usable_items_by_equipment_type_;
            while (1) {
                const int equipment_type = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int item_id = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                items[equipment_type].insert(static_cast<uint32_t>(item_id));
            }
        } else if (tag == "[dungeon inout time]") {
            DungeonInoutTimeMap& values = script->dungeon_inout_times_;
            while (1) {
                const int dungeon_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                STDungeonInoutTime value;
                value.start_hour = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                value.end_hour = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                value.start_minute = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                value.end_minute = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                if (value.start_hour > value.end_hour ||
                    value.start_minute > value.end_minute ||
                    value.end_hour == 24 ||
                    value.end_minute == 60) {
                    return 0;
                }
                values.insert(std::make_pair(dungeon_id, value));
            }
        } else if (tag == "[member bonus fatigue limit]") {
            script->member_bonus_fatigue_charac_limit_ =
                ScanInt(&success);
            if (!success) {
                return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
            }
            script->member_bonus_fatigue_account_limit_ =
                ScanInt(&success);
            if (!success) {
                return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
            }
        } else if (tag == "[premiun card cost]") {
            script->premium_gold_card_event_.card_cost =
                ScanFloat(&success);
            if (!success) {
                return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
            }
        } else if (tag == "[premium card drop]") {
            ServerParameterScript::stPremiumGoldCardEvent& event =
                script->premium_gold_card_event_;
            int index = 0;
            for (; index <= 9; ++index) {
                event.drop_rules[index].minimum_level = ScanInt(&success);
                if (!success) {
                    break;
                }
                event.drop_rules[index].maximum_level = ScanInt(&success);
                if (!success) {
                    break;
                }
                event.drop_rules[index].probability = ScanInt(&success);
                if (!success) {
                    break;
                }
                event.drop_rules[index].item_id = ScanInt(&success);
                if (!success) {
                    break;
                }
            }
            event.drop_rule_count = index;
        } else if (tag == "[premium rarity dicision]") {
            ServerParameterScript::stPremiumGoldCardEvent& event =
                script->premium_gold_card_event_;
            for (int index = 0; index <= 5; ++index) {
                event.rarity_decision[index] = ScanInt(&success);
                if (!success) {
                    return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
                }
            }
        } else if (tag == "[max clean chatting count]") {
            script->max_clean_chatting_count_ = ScanInt(&success);
            if (!success) {
                return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
            }
        } else if (tag == "[clean chatting count]") {
            PairIntVector& values = script->clean_chatting_counts_;
            while (1) {
                ScanInt(&success);
                if (!success) {
                    break;
                }
                const int level = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                const int count = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                values.push_back(std::make_pair(level, count));
            }
        } else if (tag == "[clean chatting duplication]") {
            script->duplicate_message_penalty_ = ScanInt(&success);
            if (!success) {
                return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
            }
        } else if (tag == "[max monster drop per map]") {
            script->max_monster_drop_per_map_ = ScanInt(&success);
            if (!success) {
                return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
            }
        } else if (tag == "[max monster count per map]") {
            script->max_monster_count_per_map_ = ScanInt(&success);
            if (!success) {
                return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
            }
        } else if (tag == "[private training seal gain count]") {
            for (int index = 0; index <= 3; ++index) {
                script->training_seal_gain_counts_[index] =
                    static_cast<unsigned char>(ScanInt(&success));
                if (!success) {
                    return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
                }
            }
        } else if (tag == "[relay battle training seal gain count]") {
            for (int index = 4; index <= 7; ++index) {
                script->training_seal_gain_counts_[index] =
                    static_cast<unsigned char>(ScanInt(&success));
                if (!success) {
                    return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
                }
            }
        } else if (tag == "[training seal bonus count]") {
            script->training_seal_bonus_count_ =
                static_cast<uint16_t>(ScanInt(&success));
            if (!success) {
                return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
            }
        } else if (tag == "[training seal give]") {
            IntIntMap& seal_give = script->training_seal_give_;
            while (1) {
                const int grade = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int count = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                seal_give.insert(std::make_pair(grade, count));
            }
        } else if (tag == "[pvp channel grade penalty]") {
            IntIntMap& penalties =
                script->pvp_channel_grade_penalties_;
            while (1) {
                const int grade = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int penalty = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                penalties.insert(std::make_pair(grade, penalty));
            }
        } else if (tag == "[training first give]") {
            while (1) {
                const int pvp_grade = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int charac_job = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                const int first_grow_type = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                const int count = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                PairIntVector reward;
                for (int index = 0; index < count; ++index) {
                    const int item = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                    const int item_count = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                    reward.push_back(std::make_pair(item, item_count));
                }
                ServerParameterScript::PvPPlayerKey key(
                    static_cast<unsigned char>(charac_job),
                    static_cast<unsigned char>(first_grow_type),
                    static_cast<unsigned char>(pvp_grade));
                script->training_first_give_
                    .insert(std::make_pair(key, reward));
                IntIntMap& counts =
                    script->training_first_give_count_by_grade_;
                IntIntMap::iterator it = counts.find(pvp_grade);
                if (it != counts.end()) {
                    ++it->second;
                } else {
                    counts.insert(std::make_pair(pvp_grade, 1));
                }
            }
        } else if (tag == "[pvp item give]") {
            IntVector& grades = script->pvp_item_give_grades_;
            while (1) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                grades.push_back(value);
            }
        } else if (tag == "[bring up event]") {
            if (ScanType(tag, true) == 1) {
                if (tag == "[get eventitem fatigue]") {
                    IntVector& values =
                        script->bring_up_event_parameters_.fatigue_items;
                    while (1) {
                        const int value = ScanInt(&success);
                        if (!success) {
                            break;
                        }
                        values.push_back(value);
                    }
                }
                if (ScanType(tag, true) == 1) {
                    if (tag != "[get reward count]") {
                        continue;
                    }
                    script->bring_up_event_parameters_.first_dword =
                        ScanInt(&success);
                    if (!success) {
                        return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
                    }
                    script->bring_up_event_parameters_.second_dword =
                        ScanInt(&success);
                    if (!success) {
                        return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
                    }
                } else {
                    return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
                }
            } else {
                return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
            }
        } else if (tag == "[protect of upgrade down jpn]") {
            UpgradeItemTicketMap& tickets = script->upgrade_item_tickets_;
            while (1) {
                const uint32_t item_id =
                    static_cast<uint32_t>(ScanInt(&success));
                if (!success) {
                    break;
                }
                stUpgradeItemTicketParam_t value;
                value.minimum_upgrade =
                    static_cast<unsigned char>(ScanInt(&success));
                if (!success) {
                    break;
                }
                value.maximum_upgrade =
                    static_cast<unsigned char>(ScanInt(&success));
                if (!success) {
                    break;
                }
                value.penalty_protection =
                    static_cast<unsigned char>(ScanInt(&success));
                if (!success) {
                    break;
                }
                value.jpn_only = 0;
                const int jpn_flag = ScanInt(&success);
                if (!success) {
                    break;
                }
                if (jpn_flag > 0) {
                    value.jpn_only = kUpgradeProtectionJpnOnly;
                }
                tickets.insert(std::make_pair(item_id, value));
            }
        } else if (tag == "[protect of upgrade down]") {
            UpgradeItemTicketMap& tickets = script->upgrade_item_tickets_;
            while (1) {
                const uint32_t item_id =
                    static_cast<uint32_t>(ScanInt(&success));
                if (!success) {
                    break;
                }
                stUpgradeItemTicketParam_t value;
                value.minimum_upgrade =
                    static_cast<unsigned char>(ScanInt(&success));
                if (!success) {
                    break;
                }
                value.maximum_upgrade =
                    static_cast<unsigned char>(ScanInt(&success));
                if (!success) {
                    break;
                }
                value.penalty_protection =
                    static_cast<unsigned char>(ScanInt(&success));
                if (!success) {
                    break;
                }
                value.jpn_only = 0;
                tickets.insert(std::make_pair(item_id, value));
            }
        } else if (tag == "[protect of reinforcement down]" ||
                   tag == "[protect of amplification down]") {
            // 0828 compatibility extension.  The official four-column
            // policy remains intact; these tags only type the otherwise
            // ambiguous fourth byte so reinforcement and amplification
            // scrolls cannot consume one another.
            const uint8_t scope =
                tag == "[protect of reinforcement down]"
                    ? static_cast<uint8_t>(
                          kUpgradeProtectionReinforcement)
                    : static_cast<uint8_t>(
                          kUpgradeProtectionAmplification);
            UpgradeItemTicketMap& tickets = script->upgrade_item_tickets_;
            while (1) {
                const uint32_t item_id =
                    static_cast<uint32_t>(ScanInt(&success));
                if (!success) {
                    break;
                }
                stUpgradeItemTicketParam_t value;
                value.minimum_upgrade =
                    static_cast<unsigned char>(ScanInt(&success));
                if (!success) {
                    break;
                }
                value.maximum_upgrade =
                    static_cast<unsigned char>(ScanInt(&success));
                if (!success) {
                    break;
                }
                value.penalty_protection =
                    static_cast<unsigned char>(ScanInt(&success));
                if (!success) {
                    break;
                }
                value.jpn_only = scope;
                // An explicit typed row is authoritative when a transitional
                // PVF also contains an older untyped row for the same item.
                tickets[item_id] = value;
            }
        } else if (tag == "[weekend bonus fatigue]") {
            script->weekend_bonus_.fatigue =
                static_cast<uint16_t>(ScanInt(&success));
            if (!success) {
                return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
            }
        } else if (tag == "[weekend bonus exp]") {
            script->weekend_bonus_.exp = ScanInt(&success);
            if (!success) {
                return script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
            }
        } else if (tag == "[drop routin by party]") {
            for (int index = 0; index <= 3; ++index) {
                script->drop_routine_by_party_[index] =
                    static_cast<unsigned char>(ScanInt(&success));
                if (!success) {
                    break;
                }
            }
        } else if (tag == "[drop upgrade rarity]") {
            ItemFarmingDataMap& values = script->item_farming_data_;
            while (1) {
                std::string rarity_text;
                if (ScanStr(&rarity_text) != 1) {
                    break;
                }
                int rarity = 6;
                if (rarity_text == "[common]") {
                    rarity = 0;
                }
                if (rarity_text == "[uncommon]") {
                    rarity = 1;
                }
                if (rarity_text == "[rare]") {
                    rarity = 2;
                }
                if (rarity_text == "[unique]") {
                    rarity = 3;
                }
                if (rarity_text == "[epic]") {
                    rarity = 4;
                }
                if (rarity_text == "[chronicle]") {
                    rarity = 5;
                }

                ItemFarmingData data;
                data.total_rate = 0;
                while (1) {
                    const int item_id = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    const int rate = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                    data.total_rate += rate;
                    data.list.push_back(std::make_pair(item_id, rate));
                }
                values.insert(std::make_pair(rarity, data));
            }
        } else if (tag == "[auction register item limit]") {
            AuctionRegisterItemLimitVector& values =
                script->auction_register_item_limits_;
            while (1) {
                AuctionRegisterItemLimit value;
                value.item_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                value.start_sec = ScanInt(&success);
                if (!success) {
                    break;
                }
                value.end_sec = ScanInt(&success);
                if (!success) {
                    break;
                }
                values.push_back(value);
            }
        } else if (tag == "[drop bonus of dungeon level increase]") {
            IntFloatPairVector& values =
                script->drop_bonus_of_dungeon_level_increase_;
            while (1) {
                const int level = ScanInt(&success);
                if (!success) {
                    break;
                }
                const float weight = ScanFloat(&success);
                if (!success) {
                    return 0;
                }
                values.push_back(std::make_pair(level, weight));
            }
        } else if (tag == "[my assault chaos point weight]") {
            AssaultChaosPointWeightVector& values =
                script->assault_chaos_point_weights_;
            while (1) {
                stAssaultChaosPointWeight value;
                value.min = ScanInt(&success);
                if (!success) {
                    break;
                }
                value.max = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                value.from_weight = ScanFloat(&success);
                if (!success) {
                    return 0;
                }
                value.to_weight = ScanFloat(&success);
                if (!success) {
                    return 0;
                }
                values.push_back(value);
            }
        } else if (tag == "[assault level ghost panalty]") {
            AssaultLevelGhostPanaltyVector& values =
                script->assault_level_ghost_penalties_;
            while (1) {
                stAssaultLevelGhostPanalty value;
                value.min_level = ScanInt(&success);
                if (!success) {
                    break;
                }
                value.max_level = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                value.min_time = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                value.max_time = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                values.push_back(value);
            }
        } else if (tag == "[assault chaos point ghost panalty]") {
            AssaultChaosPointGhostPanaltyVector& values =
                script->assault_chaos_point_ghost_penalties_;
            while (1) {
                stAssaultChaosPointGhostPanalty value;
                value.min = ScanInt(&success);
                if (!success) {
                    break;
                }
                value.max = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                value.from_weight = ScanFloat(&success);
                if (!success) {
                    return 0;
                }
                value.to_weight = ScanFloat(&success);
                if (!success) {
                    return 0;
                }
                values.push_back(value);
            }
        } else {
            return 0;
        }
    }

    // IDA_EA: 0x8A8C233 (LABEL_662): an EOF after a recognized tag is
    // successful only when the importer sentinel was initialized.
    const bool imported =
        script->lottery_upgrade_policy_.broadcast_minimum_rarity != -1;
    if (imported) {
        // The 140828 cash shop sells three destruction-protection tickets
        // which are absent from the official serverparameter policy table.
        // Preserve an explicit PVF override and only supply missing aliases.
        struct Alias {
            uint32_t item_id;
            uint8_t scope;
        };
        const Alias aliases[] = {
            {7804u, kUpgradeProtectionReinforcement},
            {8050u, kUpgradeProtectionReinforcement},
            {10014791u, kUpgradeProtectionAmplification},
        };
        UpgradeItemTicketMap& tickets = script->upgrade_item_tickets_;
        for (unsigned int index = 0;
             index < sizeof(aliases) / sizeof(aliases[0]); ++index) {
            if (tickets.find(aliases[index].item_id) != tickets.end()) {
                continue;
            }
            stUpgradeItemTicketParam_t policy;
            policy.minimum_upgrade = 10u;
            policy.maximum_upgrade = 14u;
            policy.penalty_protection = 0u;
            policy.jpn_only = aliases[index].scope;
            tickets.insert(std::make_pair(aliases[index].item_id, policy));
        }
    }
    return imported;
}
