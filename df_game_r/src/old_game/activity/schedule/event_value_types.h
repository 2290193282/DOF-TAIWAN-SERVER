#ifndef OLDSRV_11_EVENT_RUNTIME_EVENT_VALUE_TYPES_H_
#define OLDSRV_11_EVENT_RUNTIME_EVENT_VALUE_TYPES_H_

#include "common/config.h"
#include "old_data/db/event/db_select_create_dnf_event_info.h"

#include <stddef.h>
#include <stdint.h>

#include <ctime>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "old_game/activity/schedule/lottery_info.h"
#include "old_game/activity/schedule/multi_box_lottery_info.h"
#include "old_game/activity/schedule/in_game_event_script.h"

#include "old_data/PVF/event/event_upgrade_jar_script.h"
#include "old_data/PVF/event/action_send_mail.h"
#include "old_data/PVF/event/event_avatar_list_script.h"
#include "old_data/PVF/event/event_etc_script.h"
#include "old_data/PVF/event/in_game_event_info_import.h"
#include "old_data/PVF/event/in_game_event_script_import.h"
#include "old_data/PVF/event/level_reward_scripts.h"
#include "old_data/PVF/event/datamanager/present_response_script_event.h"
#include "old_data/PVF/arad/datamanager/arad_one_plus_card_event_item_list.h"

#include "old_game/economy/mail/mail_trade_value_types.h"
#include "old_game/activity/schedule/levelup_support_script_services.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/activity/reward/tournament_reward_type.h"
#include "old_game/activity/schedule/advance_altar_data.h"
#include "old_game/activity/reward/advance_altar.h"
#include "old_game/activity/reward/event_base.h"
#include "common/types/stackable_value_types.h"
#include "common/manager/taiwan_interface_inter_dispatcher.h"


class Stream;
class CUser;
struct stUnlimitChallengeInfo;
enum ENUM_ERROR : int;

// IDA unit: 11_event_runtime/STRewardDataKey.
struct STRewardDataKey {
    STRewardDataKey();

    bool operator<(const STRewardDataKey& rhs) const;

    uint8_t first;
    uint8_t second;
    uint8_t third;
};

// IDA unit: 11_event_runtime/STRewardData.
struct STRewardData {
    STRewardData();

    int item_id;
    int item_count;
    int item_period;
    int reward_type;
    int reward_value;
};

// IDA unit: 11_event_runtime/STRewardDataList.
struct STRewardDataList {
    STRewardDataList();
    STRewardDataList(const STRewardDataList& rhs);
    ~STRewardDataList();

    std::list<STRewardData> reward_data;
};

// IDA unit: 11_event_runtime/STBreakAwayRewards.
struct STBreakAwayRewards {
    STBreakAwayRewards();

    STBreakAwayRewards* clear();
    STBreakAwayRewards& operator=(const STBreakAwayRewards& rhs);

    uint32_t item_id;             // +0x00: selected reward item
    uint32_t item_count;          // +0x04: item/additional currency count
    uint32_t luck_point;          // +0x08: accumulated break-away luck point
    uint32_t reward_point;        // +0x0c: point/cera value used for selection
    uint32_t reward_order;        // +0x10: ENUM_BREAK_AWAY_REWARD_ORDER value
};

static_assert(sizeof(STBreakAwayRewards) == 0x14u &&
                  offsetof(STBreakAwayRewards, item_id) == 0x00u &&
                  offsetof(STBreakAwayRewards, item_count) == 0x04u &&
                  offsetof(STBreakAwayRewards, luck_point) == 0x08u &&
                  offsetof(STBreakAwayRewards, reward_point) == 0x0cu &&
                  offsetof(STBreakAwayRewards, reward_order) == 0x10u,
              "STBreakAwayRewards fields must retain the 0627 ABI");

// IDA unit: 11_event_runtime/EventRewardItemInfo.
struct EventRewardItemInfo {
    EventRewardItemInfo();
    EventRewardItemInfo(const EventRewardItemInfo& rhs);
    ~EventRewardItemInfo();

    EventRewardItemInfo* clear();
    EventRewardItemInfo& operator=(const EventRewardItemInfo& rhs);

    uint32_t first_dword;
    uint32_t second_dword;
    uint16_t word_08;
    uint8_t reserved_0a[2];
    std::vector<int> values;
};

// IDA unit: 11_event_runtime/EventRewardSection.
struct EventRewardSection {
    EventRewardSection();
    EventRewardSection(const EventRewardSection& rhs);
    ~EventRewardSection();

    EventRewardSection& operator=(const EventRewardSection& rhs);

    uint8_t byte_00;
    uint8_t byte_01;
    uint8_t byte_02;
    uint8_t reserved_03;
    std::vector<uint8_t> values;
};

// IDA unit: 11_event_runtime/DimensionActivationRewardData.
struct DimensionActivationRewardData {
    DimensionActivationRewardData();
    ~DimensionActivationRewardData();

    EventRewardSection* getSectionData(int section_value);
    std::vector<EventRewardItemInfo>* getRewardGiveItems(
        int reward_index,
        int charac_job,
        int first_grow_type);
    std::vector<std::vector<EventRewardItemInfo> >* getRewardGiveItemList(
        int charac_job,
        int first_grow_type);
    std::vector<std::vector<EventRewardItemInfo> >* getRewardSelectItemList(
        int select_group);

    std::vector<EventRewardSection> sections;
    std::map<int, std::vector<std::vector<EventRewardItemInfo> > >
        select_groups;
    std::map<std::pair<int, int>,
             std::vector<std::vector<EventRewardItemInfo> > > give_groups;
};

// IDA unit: 11_event_runtime/RewardItemInfo.
struct RewardItemInfo {
    RewardItemInfo();
    ~RewardItemInfo();

    int item_id;
    int item_count;
    int item_type;
    int period;
};

// IDA unit: 11_event_runtime/RewardUserData.
struct RewardUserData {
    RewardUserData();

    uint32_t first_dword;
    uint32_t second_dword;
    uint8_t byte_08;
    uint8_t reserved_09[3];
};

// IDA unit: 11_event_runtime/CRewardUserList.
class CRewardUserList {
public:
    CRewardUserList();
    ~CRewardUserList();

    int insert(unsigned int account_id, int charac_no, int ontime_id);
    bool is_exist(unsigned int account_id);
    int delete_mark(unsigned int account_id);
    int clear();
    int get_index_size();

private:
    friend class COnTimeEvent;

    std::map<unsigned int, RewardUserData> reward_users;
    std::deque<unsigned int> account_index;
};

// IDA unit: 11_event_runtime/COnTimeEvent.
// IDA_EA: ctor/start/end/local-state
// @0x81BB826/@0x81BB8AE/@0x81BB8CA/@0x81BB91A/@0x81BB96C
// @0x81BB978/@0x81BB986/@0x81BB994/@0x81BB9BA/@0x81BB9C6
// @0x81BBCC8/@0x81BBCE0/@0x81BC07A/@0x81BC0DC/@0x81BC0FA
// @0x81BC106/@0x81BC112.
class COnTimeEvent : public CEventBase {
public:
    COnTimeEvent();
    virtual ~COnTimeEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);

    int get_reward_state();
    COnTimeEvent* set_reward_state(int state);
    COnTimeEvent* set_ontime_id(int ontime_id);
    COnTimeEvent* set_reward_end_time(int seconds_from_now);
    int get_reward_end_time();
    int get_ontime_reward_user_count();
    // IDA_EA: 0x81BB9DC
    // IDA_NAME: _ZN12COnTimeEvent18insert_reward_userEP5CUser
    // DEMANGLED: COnTimeEvent::insert_reward_user(CUser *)
    int insert_reward_user(CUser* user);
    // IDA_EA: 0x81BBC64
    // IDA_NAME: _ZN12COnTimeEvent18delete_reward_userEP5CUser
    // DEMANGLED: COnTimeEvent::delete_reward_user(CUser *)
    int delete_reward_user(CUser* user);
    COnTimeEvent* set_ontime_reward_item(int item_no, int item_count);
    int process_set_reward_start(int ontime_id,
                                 int item_no,
                                 int item_count,
                                 int reward_seconds);
    int get_ontime_id();
    int get_ontime_reward_item_no();
    int get_ontime_reward_item_count();
    int process_send_reward_to_user();

private:
    int ontime_id_;
    int reward_state_;
    CRewardUserList reward_users_;
    int reward_end_time_;
    int reward_item_no_;
    int reward_item_count_;
};

// IDA unit: 11_event_runtime/PostalReward.
struct PostalReward {
    PostalReward();
    PostalReward(const PostalReward& rhs);
    ~PostalReward();

    PostalReward* clear();
    PostalReward& operator=(const PostalReward& rhs);

    std::string first_string;
    std::string second_string;
    uint32_t first_dword;
    uint32_t second_dword;
};

// IDA unit: 11_event_runtime/STExpReward.
struct STExpReward {
    STExpReward();
    ~STExpReward();

    uint8_t section_index;
    uint8_t gained_exp_bonus_rate;
    uint8_t clear_reward_exp_bonus_rate;
    uint8_t additional_reward_rate;
};

static_assert(sizeof(STExpReward) == 0x4,
              "STExpReward must preserve the original i386 ABI size");
static_assert(offsetof(STExpReward, section_index) == 0x0,
              "STExpReward::section_index ABI offset mismatch");
static_assert(offsetof(STExpReward, gained_exp_bonus_rate) == 0x1,
              "STExpReward::gained_exp_bonus_rate ABI offset mismatch");
static_assert(offsetof(STExpReward, clear_reward_exp_bonus_rate) == 0x2,
              "STExpReward::clear_reward_exp_bonus_rate ABI offset mismatch");
static_assert(offsetof(STExpReward, additional_reward_rate) == 0x3,
              "STExpReward::additional_reward_rate ABI offset mismatch");

// IDA unit: 11_event_runtime/stTempRewardInfo.
struct stTempRewardInfo {
    stTempRewardInfo();

    uint32_t first_dword;
    uint32_t second_dword;
};

// IDA unit: 11_event_runtime/stClearRewardBonusExp_t.
struct stClearRewardBonusExp_t {
    stClearRewardBonusExp_t();

    uint32_t avatar_bonus_exp;              // +0x00
    uint32_t rank_bonus_exp;                // +0x04
    uint32_t event_bonus_type;              // +0x08
    uint32_t event_bonus_exp;               // +0x0c
    uint32_t burning_fatigue_bonus_exp;     // +0x10
    uint32_t reserved_bonus_exp;            // +0x14: parameter slot 6
    uint32_t premium_advantage_bonus_exp;   // +0x18
    uint32_t server_event_bonus_exp;        // +0x1c
    uint32_t channel_bonus_exp;             // +0x20
    uint32_t creature_bonus_exp;            // +0x24
    uint32_t upper_member_bonus_exp;        // +0x28
    uint32_t period_exp_item_bonus_exp;      // +0x2c
    uint32_t same_pc_room_party_bonus_exp;  // +0x30
    uint32_t return_user_party_bonus_exp;   // +0x34
    uint32_t new_character_party_bonus_exp; // +0x38
    uint32_t character_manage_bonus_exp;    // +0x3c
    uint32_t growth_power_bonus_exp;        // +0x40
    uint32_t premium_exp_bonus_exp;         // +0x44
};

// IDA unit: 11_event_runtime/stClearRewardExpParameter_t.
struct stClearRewardExpParameter_t {
    stClearRewardExpParameter_t();

    float death_penalty_rate;             // +0x00
    float rank_bonus_rate;                // +0x04
    float avatar_bonus_rate;              // +0x08
    float creature_bonus_rate;            // +0x0c
    float pc_room_burning_rate;            // +0x10
    float double_exp_event_rate;           // +0x14
    float reserved_bonus_rate;             // +0x18: unused slot 6
    uint32_t burning_fatigue_bonus_percent;// +0x1c
    float channel_bonus_rate;              // +0x20
    float premium_advantage_rate;          // +0x24
    float server_event_rate;               // +0x28
    float period_exp_item_rate;            // +0x2c
    float global_effect_rate;              // +0x30
    float same_pc_room_party_rate;         // +0x34
    float return_user_party_rate;          // +0x38
    float character_manage_rate;           // +0x3c
    float growth_power_rate;               // +0x40
    float premium_exp_rate;                // +0x44
    float new_character_party_rate;        // +0x48
    float reserved_rate_19;                // +0x4c: no 0627 consumer
    float reserved_rate_20;                // +0x50: no 0627 consumer
    uint8_t reserved_flag_54;              // +0x54: no 0627 consumer
    uint8_t reserved_flag_55;              // +0x55: no 0627 consumer
    uint8_t reserved_56[2];
    int32_t member_slot;                   // +0x58
    int32_t rank_bonus_index;              // +0x5c
};

static_assert(sizeof(stClearRewardBonusExp_t) == 0x48u &&
                  offsetof(stClearRewardBonusExp_t, avatar_bonus_exp) == 0x00u &&
                  offsetof(stClearRewardBonusExp_t, event_bonus_type) == 0x08u &&
                  offsetof(stClearRewardBonusExp_t, upper_member_bonus_exp) == 0x28u &&
                  offsetof(stClearRewardBonusExp_t, premium_exp_bonus_exp) == 0x44u,
              "stClearRewardBonusExp_t fields must retain the 0627 ABI");
static_assert(sizeof(stClearRewardExpParameter_t) == 0x60u &&
                  offsetof(stClearRewardExpParameter_t, death_penalty_rate) == 0x00u &&
                  offsetof(stClearRewardExpParameter_t, burning_fatigue_bonus_percent) == 0x1cu &&
                  offsetof(stClearRewardExpParameter_t, new_character_party_rate) == 0x48u &&
                  offsetof(stClearRewardExpParameter_t, reserved_flag_54) == 0x54u &&
                  offsetof(stClearRewardExpParameter_t, member_slot) == 0x58u &&
                  offsetof(stClearRewardExpParameter_t, rank_bonus_index) == 0x5cu,
              "stClearRewardExpParameter_t fields must retain the 0627 ABI");

// IDA unit: 11_event_runtime/stFatigueEventParam_t.
struct stFatigueEventParam_t {
    stFatigueEventParam_t();
    ~stFatigueEventParam_t();

    uint16_t word_00;
    uint8_t reserved_02[2];
    uint32_t dword_04;
    uint8_t bytes_08[4];
};

// IDA unit: 11_event_runtime/stTitleRewardBasis.
struct stTitleRewardBasis {
    stTitleRewardBasis();
    ~stTitleRewardBasis();

    int32_t reward_kind;
    uint8_t flag_04;
    uint8_t reserved_05[3];
    void* reward_action;
};

// IDA unit: 11_event_runtime/stRewardChangeStatus.
struct stRewardChangeStatus : public stTitleRewardBasis {
    stRewardChangeStatus();

    int32_t status_type;
    int32_t value_type;
    int32_t status_value;
};

// IDA unit: 11_event_runtime/stRewardGiveEmotion.
struct stRewardGiveEmotion : public stTitleRewardBasis {
    stRewardGiveEmotion();

    int32_t emotion_id;
};

// IDA unit: 11_event_runtime/stRewardPostal.
struct stRewardPostal : public stTitleRewardBasis {
    stRewardPostal();

    std::string mail_title;
    std::string mail_text;
    std::string sender_name;
    uint32_t first_dword;
    uint32_t second_dword;
};

// IDA unit: 11_event_runtime/stConditionEventRewardInfo.
struct stConditionEventRewardInfo {
    stConditionEventRewardInfo();
    ~stConditionEventRewardInfo();

    int32_t first_dword;
    int32_t second_dword;
    uint8_t first_flag;
    uint8_t second_flag;
    uint8_t reserved_0a[2];
};

// IDA unit: 11_event_runtime/stConditionEventInfo.
struct stConditionEventInfo {
    stConditionEventInfo();
    ~stConditionEventInfo();

    int32_t first_dword;
    int32_t second_dword;
    std::map<int, std::pair<int, int> > condition_matches;
    std::multimap<int, stConditionEventRewardInfo> condition_rewards;
    std::multimap<int, stConditionEventRewardInfo> quick_party_rewards;
};

// IDA unit: 11_event_runtime/stEventIdolBringUpParam_t.
struct stEventIdolBringUpParam_t {
    stEventIdolBringUpParam_t();
    ~stEventIdolBringUpParam_t();

    int32_t first_dword;
    int32_t second_dword;
    std::vector<int> fatigue_items;
};

// IDA unit: 11_event_runtime/stEVENTADDINFO.
struct stEVENTADDINFO {
    ~stEVENTADDINFO();

    int32_t event_index;
    int16_t begin_value;
    int16_t end_value;
};

// IDA unit: 11_event_runtime/stUnlimitChallengeRewardInfo.
struct stUnlimitChallengeRewardInfo {
    ~stUnlimitChallengeRewardInfo();

    uint16_t level;
    uint8_t job;
    uint8_t reserved_03;
    int32_t item_id;
    int32_t item_count;
};

// IDA unit: 11_event_runtime/stEventMonster_t.
struct stEventMonster_t {
    ~stEventMonster_t();

    int32_t monster_id;
    int32_t status_value;
    int32_t probability;
    int32_t status_first;
    int32_t status_second;
};

// IDA unit: 11_event_runtime/stSecondEventMonster_t.
struct stSecondEventMonster_t {
    ~stSecondEventMonster_t();

    int32_t monster_id;
    int32_t status_value;
    int32_t probability;
    int32_t status_first;
    int32_t status_second;
};

// IDA unit: 11_event_runtime/TournamentRewardRate.
struct TournamentRewardRate {
    TournamentRewardRate();

    int32_t gold_reward_rate;  // +0x00
    int32_t item_reward_rate;  // +0x04
    int32_t no_reward_rate;    // +0x08
    int32_t total_rate;        // +0x0c
};

// IDA unit: 11_event_runtime/TournamentRewardItem.
struct TournamentRewardItem {
    TournamentRewardItem();
    ~TournamentRewardItem();

    int32_t item_id;         // +0x00
    int32_t selection_rate;  // +0x04
    int32_t item_count;      // +0x08
};

static_assert(sizeof(TournamentRewardRate) == 0x10u &&
                  offsetof(TournamentRewardRate, gold_reward_rate) == 0x00u &&
                  offsetof(TournamentRewardRate, item_reward_rate) == 0x04u &&
                  offsetof(TournamentRewardRate, no_reward_rate) == 0x08u &&
                  offsetof(TournamentRewardRate, total_rate) == 0x0cu,
              "TournamentRewardRate fields must retain the 0627 ABI");
static_assert(sizeof(TournamentRewardItem) == 0x0cu &&
                  offsetof(TournamentRewardItem, item_id) == 0x00u &&
                  offsetof(TournamentRewardItem, selection_rate) == 0x04u &&
                  offsetof(TournamentRewardItem, item_count) == 0x08u,
              "TournamentRewardItem fields must retain the 0627 ABI");

// IDA unit: 11_event_runtime/TournamentRewardItemRate.
struct TournamentRewardItemRate {
    TournamentRewardItemRate();
    ~TournamentRewardItemRate();

    TournamentRewardItemRate* Clear();
    TournamentRewardItemRate& operator=(const TournamentRewardItemRate& rhs);

    std::vector<TournamentRewardItem> reward_items;
    int32_t total_rate;
};

// IDA unit: 11_event_runtime/STUltimateRewardProb.
struct STUltimateRewardProb {
    STUltimateRewardProb();

    int32_t first_dword;
    int32_t second_dword;
};

// IDA unit: 11_event_runtime/competition_reward_result.
struct competition_reward_result {
    competition_reward_result();

    int32_t first_dword;
    int32_t second_dword;
    uint8_t success_flag;
    uint8_t reserved_09[3];
};

// IDA unit: 11_event_runtime/STEventSuper.
struct STEventSuper {
    STEventSuper();
    ~STEventSuper();

    STEventSuper* clear();

    int32_t first_dword;
    int32_t second_dword;
    int32_t third_dword;
    int32_t fourth_dword;
    uint8_t first_flag;
    uint8_t reserved_11[3];
    int32_t fifth_dword;
    int32_t sixth_dword;
    int32_t seventh_dword;
    int32_t eighth_dword;
    uint8_t second_flag;
    uint8_t reserved_25[3];
};

// LevelUp/Return-user reward data belongs to B's PVF parser provider.  C's
// event runtime consumes the canonical declaration above without a duplicate
// body or layout.

// IDA unit: 11_event_runtime/CLottery_NeedMoney.
class CLottery_NeedMoney {
public:
    CLottery_NeedMoney();
    ~CLottery_NeedMoney();

    void add_lottery_needMoney(unsigned int item_index,
                               unsigned int need_money);
    int isNeedMoney(unsigned int item_index) const;

private:
    std::map<unsigned int, unsigned int> need_money_by_item;
};

// IDA unit: 11_event_runtime/stMercenaryRewardGroup.
struct stMercenaryRewardGroup {
    stMercenaryRewardGroup();
    stMercenaryRewardGroup(const stMercenaryRewardGroup& rhs);
    ~stMercenaryRewardGroup();

    stMercenaryRewardGroup& operator=(const stMercenaryRewardGroup& rhs);

    uint32_t first_dword;
    std::string group_name;
    std::vector<std::pair<int, int> > first_rewards;
    std::vector<std::pair<int, int> > second_rewards;
    uint32_t second_dword;
    uint32_t third_dword;
};

// IDA unit: 11_event_runtime/BingoData.
struct BingoData {
    BingoData();

    BingoData* clear();

    uint32_t bingo_count;
    uint8_t line_count;
    uint8_t selected_count;
    uint8_t reward_state;
};

// IDA unit: 11_event_runtime/CLevelRewardTable.
struct CLevelRewardTable {
    CLevelRewardTable();

    int GetProperLevelIndex(unsigned int level);
    int GetProperJobIndex(char job);
    int GetRewardItemNo(int job_index, int level_index);
    int GetItemCount(int job_index, int level_index);
    char* GetMailTitle(int level_index);
    char* GetMailString(int level_index);
    CLevelRewardTable* SetJobAffection(bool enabled);
    int GetJobAffection();

    uint32_t proper_levels[
        oldsrv::config::kLevelRewardTableLevelCapacity];
    uint32_t reward_item_no[
        oldsrv::config::kLevelRewardTableJobIndexCount]
        [oldsrv::config::kLevelRewardTableLevelCapacity];
    uint32_t item_count[
        oldsrv::config::kLevelRewardTableJobIndexCount]
        [oldsrv::config::kLevelRewardTableLevelCapacity];
    char mail_title[
        oldsrv::config::kLevelRewardTableLevelCapacity][21];
    char mail_string[
        oldsrv::config::kLevelRewardTableLevelCapacity][256];
    uint8_t job_affection;
};

// IDA unit: 11_event_runtime/CLevelRewardTableMgr.
class CLevelRewardTableMgr {
public:
    CLevelRewardTableMgr();
    ~CLevelRewardTableMgr();

    static CLevelRewardTableMgr* GetInstance();

    bool IsChallengingJob(char job);
    // IDA_EA: 0x8687828
    // IDA_NAME: _ZN20CLevelRewardTableMgr16SetChallengeInfoER22stUnlimitChallengeInfo
    // DEMANGLED: CLevelRewardTableMgr::SetChallengeInfo(stUnlimitChallengeInfo&)
    bool SetChallengeInfo(stUnlimitChallengeInfo& challenge_info);
    int IsHaveStartLevelReward();
    int IsOnServer(int server_id);
    CLevelRewardTable* GetProperLevelRewardTableR() const;
    CLevelRewardTable* GetProperLevelRewardTableR(int index) const;
    int IsOn();

private:
    static CLevelRewardTableMgr* levelRewardTableMgr;

    uint8_t on;
    uint8_t reserved_01[3];
    CLevelRewardTable* proper_level_reward_table;
    std::vector<int> server_ids;
};

// IDA unit: 11_event_runtime/InGameEventInfo.
struct InGameEventInfo {
    InGameEventInfo();
    InGameEventInfo(const InGameEventInfo& rhs);
    ~InGameEventInfo();

    InGameEventInfo& operator=(const InGameEventInfo& rhs);

    uint32_t first_dword;
    uint32_t second_dword;
    std::string event_name;
    uint32_t third_dword;
    uint32_t fourth_dword;
    uint32_t fifth_dword;
};

// IDA unit: 11_event_runtime/InGameEventInfoList.
struct InGameEventInfoList {
    InGameEventInfoList();
    ~InGameEventInfoList();

    InGameEventInfo* insertEventInfo(int event_id, InGameEventInfo& event_info);

    std::map<int, InGameEventInfo> events;
};

// IDA unit: 11_event_runtime/AttendanceScript.
struct AttendanceScript : public InGameEventScript {
    AttendanceScript();

    std::string db_table_name;
    int fatigue_condition;
    std::vector<std::pair<int, int> > reward_items_by_day;
    std::pair<int, int> final_reward_item;
    int final_day_condition;
    std::string mail_title;
    Action_SendMail mail_data;
    std::string attend_mail_text;
    std::string final_mail_text;
};

// IDA unit: 11_event_runtime/ExchangeRandomItemRewardScript.
struct ExchangeRandomItemRewardScript : public InGameEventScript {
    ExchangeRandomItemRewardScript();

    void reset();

    uint8_t enabled;
    uint8_t reserved_09[3];
    std::vector<std::pair<int, int> > exchange_items;
    std::vector<int> reward_items;
    Action_SendMail send_mail;
};

// IDA unit: 11_event_runtime/LevelupSupportEventManger::EventInfo.
class LevelupSupportEventManger
    : public ARAD::Singleton<LevelupSupportEventManger>,
      public ARAD::ScriptInterface {
public:
    struct EventInfo {
        EventInfo(uint16_t level_value, uint16_t state_value);
        ~EventInfo();

        uint16_t level;
        uint16_t state;
    };

    LevelupSupportEventManger();
    ~LevelupSupportEventManger();

    virtual bool Load();
    virtual int Destroy();

    // IDA_EA: 0x814701E
    // IDA_NAME:
    // _ZN25LevelupSupportEventManger15SendCharacStateEP5CUseri
    static void SendCharacState(CUser* user, int charac_level);

    // IDA_EA: 0x8147164
    // IDA_NAME: _ZN25LevelupSupportEventManger12CheckNewStepEi
    bool CheckNewStep(int charac_level);

    // IDA_EA: 0x81471C6
    // IDA_NAME:
    // _ZN25LevelupSupportEventManger15UpdateEventInfoEP5CUser
    void UpdateEventInfo(CUser* user);

    // IDA_EA: 0x8147012
    // IDA_NAME: _ZNK25LevelupSupportEventManger19GetDefaultEventInfoEv
    // DEMANGLED: LevelupSupportEventManger::GetDefaultEventInfo(void)const
    const std::vector<EventInfo>& GetDefaultEventInfo() const;

    // IDA_EA: 0x8147362
    // IDA_NAME: _ZN25LevelupSupportEventManger12GetStepIndexEi
    // DEMANGLED: LevelupSupportEventManger::GetStepIndex(int)
    int GetStepIndex(int requested_level);

    // IDA_EA: 0x81473CC
    // IDA_NAME: _ZN25LevelupSupportEventManger10InsertItemEP5CUseriR10ENUM_ERROR
    // DEMANGLED: LevelupSupportEventManger::InsertItem(CUser *,int,ENUM_ERROR &)
    void InsertItem(CUser* user, int step_index, ENUM_ERROR& error);

    // IDA_EA: 0x8147724
    // IDA_NAME: _ZN25LevelupSupportEventManger17GetItemInfoVectorEi
    // DEMANGLED: LevelupSupportEventManger::GetItemInfoVector(int)
    std::vector<LevelupSupportSciprt::ItemInfo>* GetItemInfoVector(
        int requested_level);

private:
    LevelupSupportSciprt script_;
    std::map<int, int> step_indices_;
    std::vector<EventInfo> default_event_info_;
};

namespace ARAD {
template <>
Singleton<LevelupSupportEventManger>::Singleton();
template <>
LevelupSupportEventManger* Singleton<LevelupSupportEventManger>::Get();
}  // namespace ARAD

// IDA unit: 11_event_runtime/LevelupSupportEvent.
// IDA_EA: ctor/dtor/start/end
// @0x81169F8/@0x8116A14/@0x8116A44/@0x8116A62/@0x8116AB4/@0x8116ACE.
class LevelupSupportEvent : public CEventBase {
public:
    LevelupSupportEvent();
    virtual ~LevelupSupportEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/LevelupSupport2ndEvent.
// IDA_EA: ctor/dtor/start/end
// @0x811B988/@0x811B9A4/@0x811B9D4/@0x811B9F2/@0x811BA44/@0x811BA5E.
class LevelupSupport2ndEvent : public CEventBase {
public:
    LevelupSupport2ndEvent();
    virtual ~LevelupSupport2ndEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/CreateDnfEventSciprt.
class CreateDnfEventSciprt {
public:
    struct ItemInfo {
        ItemInfo(int32_t item_index_value, int item_count_value);
        ~ItemInfo();

        int32_t item_index;
        int16_t item_count;
        uint16_t reserved;
    };

    CreateDnfEventSciprt();
    ~CreateDnfEventSciprt();

    int loadScript();
    int clearScript();
    char GetItemInfo(std::vector<std::pair<int, int> >& out, int day);
    int GetItemInfo(std::vector<std::pair<int, int> >& out);

private:
    std::map<int, std::vector<ItemInfo>* > reward_items_by_day;
};

// IDA unit: 11_event_runtime/CreateDnfEventSciprtServer.
// IDA_EA: Load/Destroy/ctor/dtor @0x816451E/@0x8164590/
// @0x81648AC/@0x81648E2.
class CreateDnfEventSciprtServer {
public:
    CreateDnfEventSciprtServer();
    ~CreateDnfEventSciprtServer();

    virtual bool Load();
    virtual int Destroy();

    CreateDnfEventSciprt script;
};

namespace ARAD {

// IDA unit: 11_event_runtime/ARAD::Singleton<CreateDnfEventSciprtServer>.
template <typename T>
class Singleton;

template <>
class Singleton<CreateDnfEventSciprtServer> {
public:
    Singleton();
    static CreateDnfEventSciprtServer* Get();
};

}  // namespace ARAD

namespace StackablePackage {

// IDA unit: 11_event_runtime/StackablePackage/EventProcessor.
struct EventProcessor {
    int GetEventType() const;
    int GetEventId() const;

    void* vptr;
    int event_type;
    int event_id;
};

// IDA unit: 11_event_runtime/StackablePackage/FindControllableEventProcessor.
struct FindControllableEventProcessor {
    explicit FindControllableEventProcessor(int item_id_value);

    bool operator()(const std::pair<const int, EventProcessor*>& entry) const;

    int item_id;
};

}  // namespace StackablePackage

// New-account step rewards are declared by B's canonical PVF header above.

// IDA unit: 11_event_runtime/CEventStayTime.
// IDA_EA: event lifecycle/accessors/timer/reset
// @0x816BDA0/@0x816BE18/@0x816BE60/@0x816BEBC/@0x816BEC8
// @0x816BED4/@0x816BEFA/@0x816BF6A/@0x816C066/@0x816C08A.
class CEventStayTime : public CEventBase {
public:
    // IDA_DATA_EA: 0x8B800A0
    // IDA_NAME: _ZN14CEventStayTime12kRewardItem_E
    // The four read-only bytes at the original address are 0x2920908d.
    // They are consumed by InterCheckStayTimeEvent only.
    static const int kRewardItem_;

    struct RewardGoGoFighter {
        RewardGoGoFighter(int reward_stay_time_value,
                          long check_time_value);

        // IDA_EA: 0x863C018
        // IDA_NAME: _ZN14CEventStayTime17RewardGoGoFighterclEP5CUser
        // DEMANGLED: CEventStayTime::RewardGoGoFighter::operator()(CUser *)
        void operator()(CUser* user);

        int reward_stay_time;
        long check_time;
    };

    CEventStayTime();
    virtual ~CEventStayTime();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
    virtual void dailyresetData();

    int getRewardStayTime();
    int getIntervalStayTime();
    int IsFirstReset();
    int RegistNextTimer();

private:
    uint32_t reward_stay_time_;
    uint32_t interval_stay_time_;
};

// IDA unit: 11_event_runtime/CUnlimitFatigueEvent.
class CUnlimitFatigueEvent : public CEventBase {
public:
    CUnlimitFatigueEvent();
    virtual ~CUnlimitFatigueEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/CDoubleGoldCardEvent.
class CDoubleGoldCardEvent : public CEventBase {
public:
    CDoubleGoldCardEvent();
    virtual ~CDoubleGoldCardEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/CDimensionActivationEvent.
// IDA_EA: ctor/dtor/start/end/dimension-inout/free-revival
// @0x80EEA84/@0x80EEAA0/@0x80EEAD0/@0x80EEAEE/@0x80EEB4A/@0x80EEB64/
// @0x80EEBC0/@0x80EEC58/@0x80EECA4.
class CDimensionActivationEvent : public CEventBase {
public:
    CDimensionActivationEvent();
    virtual ~CDimensionActivationEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);

    char set_dungeon_play_count();
    int reset_dungeon_play_count();
    bool checkFreeRevivalCondition(int dungeon_index);
};

// IDA unit: 11_event_runtime/CEventAdvanceAltarOpen.
class CEventAdvanceAltarOpen : public CEventBase {
public:
    CEventAdvanceAltarOpen();
    virtual ~CEventAdvanceAltarOpen();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/CAssaultOnOffEvent.
class CAssaultOnOffEvent : public CEventBase {
public:
    CAssaultOnOffEvent();
    virtual ~CAssaultOnOffEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/CConditionEvent.
class CConditionEvent : public CEventBase {
public:
    CConditionEvent();
    virtual ~CConditionEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/CDnFLeaguePromoteFirstEvent.
class CDnFLeaguePromoteFirstEvent : public CEventBase {
public:
    CDnFLeaguePromoteFirstEvent();
    virtual ~CDnFLeaguePromoteFirstEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/CDnFLeaguePromoteSecondEvent.
class CDnFLeaguePromoteSecondEvent : public CEventBase {
public:
    CDnFLeaguePromoteSecondEvent();
    virtual ~CDnFLeaguePromoteSecondEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/CBurnigGoldMonsterEvent.
class CBurnigGoldMonsterEvent : public CEventBase {
public:
    CBurnigGoldMonsterEvent();
    virtual ~CBurnigGoldMonsterEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/Arad_MomijiEvent.
class Arad_MomijiEvent : public CEventBase {
public:
    Arad_MomijiEvent();
    virtual ~Arad_MomijiEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/CAradRyosikaEvent.
// IDA_EA: ctor/dtor/start/end/apply-date
// @0x81A0EA0/@0x81A0ED0/@0x81A0F00/@0x81A0F1E/@0x81A0F8A/@0x81A0FBE/@0x81A0FDA.
class CAradRyosikaEvent : public CEventBase {
public:
    CAradRyosikaEvent();
    virtual ~CAradRyosikaEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);

    int ApplyDatePeriod(CUser* user);

    // IDA_EA: 0x81A0FE4
    // IDA_NAME: _ZN17CAradRyosikaEvent8SendMailEP5CUseriii
    // DEMANGLED: CAradRyosikaEvent::SendMail(CUser *,int,int,int)
    int SendMail(CUser* user,
                 int receiver_charac_no,
                 int usable_level,
                 int charac_job);

    // IDA_EA: 0x81A107C
    // IDA_NAME: _ZN17CAradRyosikaEvent7DoEventEP5CUseriii
    // DEMANGLED: CAradRyosikaEvent::DoEvent(CUser *,int,int,int)
    void DoEvent(CUser* user,
                 int receiver_charac_no,
                 int unused,
                 int charac_job);

private:
    int32_t step_;
    int32_t job_;
};

// IDA unit: 11_event_runtime/AttendanceEvent.
class AttendanceEvent : public CEventBase {
public:
    AttendanceEvent();
    virtual ~AttendanceEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);

    int getRewordItem();
};

// IDA unit: 11_event_runtime/CClearRewardCardEvent.
class CClearRewardCardEvent : public CEventBase {
public:
    CClearRewardCardEvent();
    virtual ~CClearRewardCardEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/CCoinEventOnCharCreate.
class CCoinEventOnCharCreate : public CEventBase {
public:
    CCoinEventOnCharCreate();
    virtual ~CCoinEventOnCharCreate();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);

    int GetCoinNoOnCreateCharac();

private:
    uint32_t coin_no_on_create_charac_;
};

// IDA unit: 11_event_runtime/CCoinEventPerDay.
class CCoinEventPerDay : public CEventBase {
public:
    CCoinEventPerDay();
    virtual ~CCoinEventPerDay();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);

    int GetCoinNoPerDay(int level);

private:
    uint8_t coin_no_level_27_plus_;
};

// IDA unit: 11_event_runtime/CCoinRefillEvent.
// IDA_EA: ctor/dtor/start/end/time helpers
// @0x832BDF4/@0x832BE2E/@0x832BE5E/@0x832BE7C/@0x832BE82/@0x832C048/
// @0x832C0B8/@0x832C148/@0x832C1FE.
class CCoinRefillEvent : public CEventBase {
public:
    CCoinRefillEvent();
    virtual ~CCoinRefillEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);

    int getLastEventTime() const;
    int getNextEventTime(const std::tm& local_time) const;
    int registNextRefillTimer() const;

private:
    int32_t event_time_count_;
    int32_t event_times_[4];
};

// IDA unit: 11_event_runtime/CCollectArchieveEventLog.
class CCollectArchieveEventLog : public CEventBase {
public:
    CCollectArchieveEventLog();
    virtual ~CCollectArchieveEventLog();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/CExpDoubleEvent.
class CExpDoubleEvent : public CEventBase {
public:
    CExpDoubleEvent();
    virtual ~CExpDoubleEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);

    int GetExpFactor();

private:
    uint32_t exp_factor_;
};

// IDA unit: 11_event_runtime/CItemDropRatioEvent.
// IDA_EA: ctor/dtor/start/end
// @0x85331D0/@0x85331F6/@0x8533226/@0x8533244/@0x85332B6/@0x8533340.
class CItemDropRatioEvent : public CEventBase {
public:
    CItemDropRatioEvent();
    virtual ~CItemDropRatioEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);

private:
    uint32_t item_drop_rate_;
};

// IDA unit: 11_event_runtime/CMaxFatigueFactorEvent.
// IDA_EA: getter/ctor/dtor/start/end
// @0x812CE54/@0x81B2718/@0x81B273E/@0x81B276E/@0x81B278C/@0x81B27E8/@0x81B284C.
class CMaxFatigueFactorEvent : public CEventBase {
public:
    CMaxFatigueFactorEvent();
    virtual ~CMaxFatigueFactorEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);

    int GetMaxFatigueFactor();

private:
    uint32_t max_fatigue_factor_;
};

// IDA unit: 11_event_runtime/CFatigueAttendance.
class CFatigueAttendance : public CEventBase {
public:
    CFatigueAttendance();
    virtual ~CFatigueAttendance();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/CFatigueBuffEvent.
class CFatigueBuffEvent : public CEventBase {
public:
    CFatigueBuffEvent();
    virtual ~CFatigueBuffEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/CFatigueGiveItemEvent.
class CFatigueGiveItemEvent : public CEventBase {
public:
    CFatigueGiveItemEvent();
    virtual ~CFatigueGiveItemEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/CGoldCardBlankItemEvent.
class CGoldCardBlankItemEvent : public CEventBase {
public:
    CGoldCardBlankItemEvent();
    virtual ~CGoldCardBlankItemEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/CMemberExpRewardEvent.
class CMemberExpRewardEvent : public CEventBase {
public:
    CMemberExpRewardEvent();
    virtual ~CMemberExpRewardEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);

    int GetMemberExpRewardRatio();

private:
    uint32_t member_exp_reward_ratio_;
};

// IDA unit: 11_event_runtime/CNoNeedGoldOnGuildCreateEvent.
class CNoNeedGoldOnGuildCreateEvent : public CEventBase {
public:
    CNoNeedGoldOnGuildCreateEvent();
    virtual ~CNoNeedGoldOnGuildCreateEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/CReformingDanjinEvent.
class CReformingDanjinEvent : public CEventBase {
public:
    CReformingDanjinEvent();
    virtual ~CReformingDanjinEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/CStabToDeathEvent.
class CStabToDeathEvent : public CEventBase {
public:
    CStabToDeathEvent();
    virtual ~CStabToDeathEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/CStopOverlabExpEvent.
class CStopOverlabExpEvent : public CEventBase {
public:
    CStopOverlabExpEvent();
    virtual ~CStopOverlabExpEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/CUXGameLogEvent.
class CUXGameLogEvent : public CEventBase {
public:
    CUXGameLogEvent();
    virtual ~CUXGameLogEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/CVendingMachineBonusEvent.
class CVendingMachineBonusEvent : public CEventBase {
public:
    CVendingMachineBonusEvent();
    virtual ~CVendingMachineBonusEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/CWeekendBonusEvent.
class CWeekendBonusEvent : public CEventBase {
public:
    CWeekendBonusEvent();
    virtual ~CWeekendBonusEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/CPowerWarVictoriousEvent.
// IDA_EA: ctor/dtor/start/end
// @0x826891C/@0x8268938/@0x8268968/@0x8268986/@0x8268A68/@0x8268B4A.
class CPowerWarVictoriousEvent : public CEventBase {
public:
    CPowerWarVictoriousEvent();
    virtual ~CPowerWarVictoriousEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/CGuildWarEvent.
// IDA_EA: ctor/dtor/start/end @0x811CFA8/@0x811CFD6/@0x811D006/
// @0x811D024/@0x811D152/@0x811D28C.
class CGuildWarEvent : public CEventBase {
public:
    CGuildWarEvent();
    virtual ~CGuildWarEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);

private:
    uint16_t server_no_;
    uint16_t reserved_0e_;
};

// IDA unit: 11_event_runtime/CPowerWarEvent.
// IDA_EA: ctor/dtor/start/end/cleanup @0x8267990/@0x82679AC/
// @0x82679DC/@0x82679FA/@0x8267DEA/@0x82681E2/@0x8268492.
class CPowerWarEvent : public CEventBase {
public:
    CPowerWarEvent();
    virtual ~CPowerWarEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);

    void CleanupPowerWarServerMessage();

private:
    uint32_t raw_param_;
};

// IDA unit: 11_event_runtime/CSchoolMatchEvent.
// IDA_EA: ctor/dtor/start/end/append @0x8273ECC/@0x8273F1A/
// @0x8273F7C/@0x8273F9A/@0x8273FEC/@0x827423C/@0x82742A8.
class CSchoolMatchEvent : public CEventBase {
public:
    CSchoolMatchEvent();
    virtual ~CSchoolMatchEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
    virtual void AppendInfo(PacketGuard& guard);

private:
    std::vector<int> channels_;
};

// IDA unit: 11_event_runtime/CEventCreateDnf.
// IDA_EA: ctor/dtor/start/end/timer/state/reward
// @0x8163AA4/@0x8163AE8/@0x8163B18/@0x8163B36/@0x8163BE0/
// @0x8163CD0/@0x8163D4C/@0x8163E7C/@0x8164092/@0x81640D4/
// @0x8164246/@0x8164252/@0x8164260/@0x816426C.
class CEventCreateDnf : public CEventBase {
public:
    struct State {
        enum T {
            kWaiting = 0,
            kIng = 1,
            kReward = 2,
        };
    };

    CEventCreateDnf();
    virtual ~CEventCreateDnf();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);

    int RegistNextTimer();
    int RegistStartTimer(bool request_now);
    int IsRewardCondition();
    CEventCreateDnf* SetEventInfo(int hour, int rate, int reward_day);
    int GetState();
    CEventCreateDnf* setState(State::T state);
    int GetRate();
    void SendReward(CUser* user);

private:
    int state_;
    uint8_t start_hour_;
    uint8_t end_hour_;
    uint16_t reload_interval_;
    uint8_t reward_day_;
    uint8_t reserved_15_;
    uint16_t reward_rate_;
};


// IDA unit: 11_event_runtime/InterSelectCreateDnfEventInfo.
// IDA_EA: dispatch/makeRequest/Reward @0x8160D5E/@0x8160EF6/
// @0x8160C4A/@0x8160C6E.
struct InterSelectCreateDnfEventInfo : public TaiwanInterfaceInterDispatcher {
    // IDA_EA: 0x816201A
    // IDA_NAME: _ZN29InterSelectCreateDnfEventInfoC2Ev
    // DEMANGLED: InterSelectCreateDnfEventInfo::InterSelectCreateDnfEventInfo(void)
    InterSelectCreateDnfEventInfo();

    // IDA_EA: 0x816203E
    // IDA_NAME: _ZN29InterSelectCreateDnfEventInfoD2Ev
    // DEMANGLED: InterSelectCreateDnfEventInfo::~InterSelectCreateDnfEventInfo()
    // IDA_EA: 0x816206E
    // IDA_NAME: _ZN29InterSelectCreateDnfEventInfoD0Ev
    // DEMANGLED: InterSelectCreateDnfEventInfo::~InterSelectCreateDnfEventInfo()
    virtual ~InterSelectCreateDnfEventInfo();

    struct Msg {
        int rate;
        int hour;
        int event_day;
    };

    struct Reward {
        Reward(int rate, bool rewardable);
        void operator()(CUser* user);

        uint16_t rate;
        uint8_t rewardable;
        uint8_t reserved_03;
    };

    virtual int dispatch_sig(CUser* user, char* msg, int length);
    static void makeRequest(int rate, int hour, int event_day);
};

#if defined(__i386__)
static_assert(sizeof(InterSelectCreateDnfEventInfo) == 0x08u,
              "InterSelectCreateDnfEventInfo must retain the Taiwan ABI");
#endif

// IDA unit: 11_event_runtime/TimerEndPowerWarVictoriousReward.
// IDA_EA: dispatch/ctor/dtor @0x86394CE/@0x863CE56/
// 0x863ED12/@0x863ED42.
class TimerEndPowerWarVictoriousReward {
public:
    TimerEndPowerWarVictoriousReward();
    virtual ~TimerEndPowerWarVictoriousReward();

    virtual int dispatch_sig(int object_id, int param1, unsigned int param2);
};

// IDA unit: 11_event_runtime/CBreakAwayPreventEvent.
// IDA_EA: ctor/dtor/start/end
// @0x8109DBC/@0x8109DD8/@0x8109E08/@0x8109E26/@0x8109E78/@0x8109ED6.
class CBreakAwayPreventEvent : public CEventBase {
public:
    CBreakAwayPreventEvent();
    virtual ~CBreakAwayPreventEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/CBurningFatigueEvent.
// IDA_EA: ctor/dtor/start/end/getters
// @0x8109FA4/@0x8109FF4/@0x810A024/@0x810A042/@0x810A048
// @0x810A09A/@0x810A0EC/@0x810A166.
class CBurningFatigueEvent : public CEventBase {
public:
    CBurningFatigueEvent();
    virtual ~CBurningFatigueEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);

    int getBonusExpRate(uint16_t used_fatigue) const;
    int getRefillFatigue(uint16_t cur_used_fatigue,
                         uint16_t use_fatigue) const;

private:
    const void* burning_fatigue_params_;
};

// IDA unit: 11_event_runtime/CFirstServerOpenEvent.
// IDA_EA: ctor/dtor/start/end/is-eventing
// @0x811CA78/@0x811CA94/@0x811CAC4/@0x811CAE2/@0x811CB34/@0x811CB86/@0x811CBD8.
class CFirstServerOpenEvent : public CEventBase {
public:
    CFirstServerOpenEvent();
    virtual ~CFirstServerOpenEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
    virtual int IsEventing(CUser* user);
};

// IDA unit: 11_event_runtime/CGmRegistEvent.
// IDA_EA: ctor/dtor/start/end
// @0x811CC78/@0x811CC94/@0x811CCC4/@0x811CCE2/@0x811CD2C/@0x811CD76.
class CGmRegistEvent : public CEventBase {
public:
    CGmRegistEvent();
    virtual ~CGmRegistEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/CTrcOnOffEvent.
// IDA_EA: ctor/dtor/start/end/is-eventing
// @0x8285A44/@0x8285A60/@0x8285A90/@0x8285AAE/@0x8285B00/@0x8285B52/@0x8285BA4.
class CTrcOnOffEvent : public CEventBase {
public:
    CTrcOnOffEvent();
    virtual ~CTrcOnOffEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
    virtual int IsEventing(CUser* user) const;
};

// IDA unit: 11_event_runtime/LevelUpBefore70LvEvent.
// IDA_EA: ctor/dtor/start/end/is-eventing
// @0x811FF60/@0x811FF7C/@0x811FFAC/@0x811FFCA/@0x812001C/@0x812006E/@0x812011C.
class LevelUpBefore70LvEvent : public CEventBase {
public:
    LevelUpBefore70LvEvent();
    virtual ~LevelUpBefore70LvEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
    virtual int IsEventing(CUser* user) const;
};

// IDA unit: 11_event_runtime/CNotApplyBalkeunEvent.
// IDA_EA: ctor/dtor/start/end
// @0x81B9D5C/@0x81B9D78/@0x81B9DA8/@0x81B9DC6/@0x81B9E4C/@0x81B9ED2.
class CNotApplyBalkeunEvent : public CEventBase {
public:
    CNotApplyBalkeunEvent();
    virtual ~CNotApplyBalkeunEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/EventGiveMeBox.
// IDA_EA: ctor/dtor/start/end
// @0x81649B0/@0x81649CC/@0x81649FC/@0x8164A1A/@0x8164A6C/@0x8164ABE.
class EventGiveMeBox : public CEventBase {
public:
    // IDA_DATA_EA: 0x8B7CD20
    // IDA_NAME: _ZN14EventGiveMeBox12kRewardItem_E
    // The four read-only bytes at the original address are 0x2920918c.
    // They are consumed by InterInsertGiveBoxEventReward only.
    static const int kRewardItem_;

    EventGiveMeBox();
    virtual ~EventGiveMeBox();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 11_event_runtime/CServerEvent.
class CServerEvent {
public:
    static int GetExpRate();
    static int AddExpRate(int exp_rate);
    static int TurnOnEvent(int event_id, int interval);
    static bool IsEventing(int event_id);
    static int GetEventInterval(int event_id);
    static int SubExpRate(int exp_rate);
    static int TurnOffEvent(int event_id);

private:
    static int m_nExpRate;
    static int m_nCurEvent[2];
    static int m_nCurEventInterval[2];
};

// IDA unit: 11_event_runtime/UsedFatigueRewardItem::Data.
class UsedFatigueRewardItem {
public:
    struct Data {
        Data();
        ~Data();

        uint16_t fatigue;       // IDA +0x000: lower fatigue bound.
        uint16_t item_count;    // IDA +0x002: reward item count.
        char message[0xff];     // IDA +0x004: localized mail body.
        char title[0x11];       // IDA +0x103: 16-byte title plus terminator.
        uint32_t item_id;       // IDA +0x114: reward item template id.
    };

    // The IDA bodies observe a four-byte prefix, flag byte +0x04 and the
    // vector beginning at +0x08.  The prefix remains opaque because neither
    // exported body gives it a semantic role.
    uint32_t opaque_00;
    uint8_t enabled;
    uint8_t reserved_05[3];
    std::vector<Data> entries;

    // IDA_EA: 0x81A25D2
    // IDA_NAME: _ZN21UsedFatigueRewardItem7parsingEPc
    // DEMANGLED: UsedFatigueRewardItem::parsing(char *)
    int parsing(char* script_buffer);

    // IDA_EA: 0x81A28E4
    // IDA_NAME: _ZN21UsedFatigueRewardItem4findEiiRNS_4DataE
    // DEMANGLED: UsedFatigueRewardItem::find(int,int,UsedFatigueRewardItem::Data &)
    int find(int fatigue_begin, int fatigue_end, Data& result);
};

namespace Condition_LevelUp {

// IDA unit: 11_event_runtime/Condition_LevelUp::Reward_Item.
struct Reward_Item {
    Reward_Item();
    Reward_Item(const Reward_Item& rhs);
    ~Reward_Item();

    Reward_Item& operator=(const Reward_Item& rhs);

    int32_t first_dword;
    int32_t second_dword;
    int32_t third_dword;
    std::string first_string;
    std::string second_string;
};

}  // namespace Condition_LevelUp

namespace advancealtar {

// IDA unit: 11_event_runtime/advancealtar/_RewardItem.
struct _RewardItem {
    _RewardItem();
    ~_RewardItem();

    int32_t reward_type;
    int32_t item_id;
    int32_t item_count;
};

// IDA unit: 11_event_runtime/advancealtar/ClearRewardData.
struct ClearRewardData {
    ClearRewardData();
    ~ClearRewardData();

    ClearRewardData& operator=(const ClearRewardData& rhs);
    bool clear();

    std::vector<_RewardItem> reward_items[3];
};

// IDA unit: 11_event_runtime/advancealtar/RoundRewardData.
struct RoundRewardData {
    RoundRewardData();
    RoundRewardData(const RoundRewardData& rhs);
    ~RoundRewardData();

    RoundRewardData& operator=(const RoundRewardData& rhs);

    int begin_round;
    int end_round;
    std::vector<_RewardItem> reward_items;
};

// IDA unit: 11_event_runtime/advancealtar/SurvivalRewardData.
struct SurvivalRewardData {
    SurvivalRewardData();
    ~SurvivalRewardData();

    SurvivalRewardData& operator=(const SurvivalRewardData& rhs);
    const std::vector<_RewardItem>* getRoundRewardData(int round) const;

    std::vector<RoundRewardData> round_reward_data;
};

// IDA unit: 11_event_runtime/advancealtar/AchievementRewardData.
struct AchievementRewardData {
    AchievementRewardData();
    AchievementRewardData(const AchievementRewardData& rhs);
    ~AchievementRewardData();

    AchievementRewardData* reset();

    int achievement_id;
    int achievement_group;
    int clear_count;
    int clear_limit;
    std::vector<_RewardItem> reward_items;
    std::string title;
    std::string text;
    std::string tooltip;
};

// IDA unit: 11_event_runtime/advancealtar/RewardItemType.
struct RewardItemType {
    enum T {
        NORMAL = 0,
        EVENT = 1
    };

    static bool getRewardItemType(const std::string& name, T& type);
};

// IDA unit: 11_event_runtime/advancealtar/RewardParameter.
class RewardParameter {
public:
    RewardParameter();
    ~RewardParameter();

    void reset();
    int importAchievementRewardParameter(const char* filename);
    const AchievementRewardData* getAchievementRewardData(int key) const;
    const AchievementRewardData* getAchievementRewardData(
        StageDifficulty::T difficulty,
        int clear_count) const;

private:
    std::map<int, AchievementRewardData> achievement_rewards;
};

}  // namespace advancealtar

namespace APSystem {

// IDA unit: 11_event_runtime/APSystem/_TodayRewardItem.
struct _TodayRewardItem {
    _TodayRewardItem();
    ~_TodayRewardItem();

    uint8_t raw[0x0e];
};

// IDA unit: 11_event_runtime/APSystem/Inter_LoadRewardMedal.
// IDA_EA: dispatch/SendMail/ctor/dtor
// @0x8124E2A/@0x8124EA0/@0x84ED448/@0x8125620/@0x8125650.
class Inter_LoadRewardMedal : public ISigDispatcher {
public:
    Inter_LoadRewardMedal();
    virtual ~Inter_LoadRewardMedal();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
    int SendMail(CUser& user, unsigned int medal_type);
};

}  // namespace APSystem

#endif  // OLDSRV_11_EVENT_RUNTIME_EVENT_VALUE_TYPES_H_
