#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/activity/schedule/freely_give_item.h"
#include "infra/network/packet/PacketEnum.h"
#include "common/config.h"
#include "old_game/activity/schedule/event_messages.h"
#include "old_game/activity/schedule/event_shop_attendance.h"
#include "old_game/activity/reward/inter_ontime_reward_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <ctime>

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/common/arad_script.h"
#include "old_data/PVF/event/unlimit_challenge_script.h"
#include "old_data/PVF/event/in_game_event_pvf_scripts.h"
#include "old_data/PVF/stackable/arad_gift_item_set.h"
#include "old_data/db/core/mysql.h"
#include "old_data/db/legacy_dispatch/db_action_point_reward_medal.h"
#include "old_data/db/core/action_point_db_exchange_proc_abi.h"
#include "old_data/db/account/db_load_break_away_prevent_data.h"
#include "old_data/db/character/db_update_gm_data.h"
#include "old_data/db/statistics_log/db_insert_ontime_reward_user_log.h"
#include "core/config/env/environment.h"
#include "old_data/PVF/etc/action_point_etc_parameter.h"
#include "old_data/PVF/data/data_manager.h"
#include "common/manager/cdata_manager_tail_public_abi.h"
#include "common/manager/chatting_emoticon.h"
#include "common/manager/db_mgr.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "old_game/social/guild/guild_power_parameter_script_abi.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/pvp/season/pvp_rating_abi.h"
#include "common/manager/server_parameter_script_abi.h"
#include "common/manager/shared_server_message_abi.h"
#include "common/manager/taiwan_interface_inter_dispatcher.h"
#include "common/manager/user.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"
#include "old_game/player/core/local_china_user.h"
#include "old_game/pvp/match/league_match_static_state_runtime.h"



namespace Taiwan {
// A-owned stream ABI import; C only calls it.
// ABI_IMPORT_IDA_EA: 0x8163950
// ABI_IMPORT_NAME: _ZN6Taiwan15internal_streamER12CStreamGuardN18TaiwanInternalPack1TEi
int internal_stream(CStreamGuard& guard, int code, int src);
}

// IDA_EA: 0x86019D0
// IDA_NAME: _ZN20EVENT_SERVER_MESSAGE5clearEv
// DEMANGLED: EVENT_SERVER_MESSAGE::clear(void)
int EVENT_SERVER_MESSAGE::clear() {
    count = 0;
    messages.clear();
    return count;
}

// IDA_EA: 0x8601A1A
// IDA_NAME: _ZN20EVENT_SERVER_MESSAGEC2Ev
// DEMANGLED: EVENT_SERVER_MESSAGE::EVENT_SERVER_MESSAGE(void)
EVENT_SERVER_MESSAGE::EVENT_SERVER_MESSAGE() {}

// IDA_EA: 0x8601A30
// IDA_NAME: _ZN20EVENT_SERVER_MESSAGED2Ev
// DEMANGLED: EVENT_SERVER_MESSAGE::~EVENT_SERVER_MESSAGE()
EVENT_SERVER_MESSAGE::~EVENT_SERVER_MESSAGE() {}


typedef char STRewardDataKey_Size_Must_Be_0x03[
    (sizeof(STRewardDataKey) == 0x03) ? 1 : -1];
typedef char STRewardData_Size_Must_Be_0x14[
    (sizeof(STRewardData) == 0x14) ? 1 : -1];
typedef char STRewardDataList_Size_Must_Be_0x08[
    (sizeof(STRewardDataList) == 0x08) ? 1 : -1];
typedef char STBreakAwayRewards_Size_Must_Be_0x14[
    (sizeof(STBreakAwayRewards) == 0x14) ? 1 : -1];
typedef char EventRewardItemInfo_Size_Must_Be_0x18[
    (sizeof(EventRewardItemInfo) == 0x18) ? 1 : -1];
typedef char EventRewardSection_Size_Must_Be_0x10[
    (sizeof(EventRewardSection) == 0x10) ? 1 : -1];
typedef char DimensionActivationRewardData_Size_Must_Be_0x3c[
    (sizeof(DimensionActivationRewardData) == 0x3c) ? 1 : -1];
typedef char RewardItemInfo_Size_Must_Be_0x10[
    (sizeof(RewardItemInfo) == 0x10) ? 1 : -1];
typedef char RewardUserData_Size_Must_Be_0x0c[
    (sizeof(RewardUserData) == 0x0c) ? 1 : -1];
typedef char CRewardUserList_Size_Must_Be_0x40[
    (sizeof(CRewardUserList) == 0x40) ? 1 : -1];
typedef char COnTimeEvent_Size_Must_Be_0x60[
    (sizeof(COnTimeEvent) == 0x60) ? 1 : -1];
typedef char PostalReward_Size_Must_Be_0x10[
    (sizeof(PostalReward) == 0x10) ? 1 : -1];
typedef char STExpReward_Size_Must_Be_0x04[
    (sizeof(STExpReward) == 0x04) ? 1 : -1];
typedef char stTempRewardInfo_Size_Must_Be_0x08[
    (sizeof(stTempRewardInfo) == 0x08) ? 1 : -1];
typedef char stClearRewardBonusExp_t_Size_Must_Be_0x48[
    (sizeof(stClearRewardBonusExp_t) == 0x48) ? 1 : -1];
typedef char stClearRewardExpParameter_t_Size_Must_Be_0x60[
    (sizeof(stClearRewardExpParameter_t) == 0x60) ? 1 : -1];
typedef char stFatigueEventParam_t_Size_Must_Be_0x0c[
    (sizeof(stFatigueEventParam_t) == 0x0c) ? 1 : -1];
typedef char stTitleRewardBasis_Size_Must_Be_0x0c[
    (sizeof(stTitleRewardBasis) == 0x0c) ? 1 : -1];
typedef char stRewardChangeStatus_Size_Must_Be_0x18[
    (sizeof(stRewardChangeStatus) == 0x18) ? 1 : -1];
typedef char stRewardGiveEmotion_Size_Must_Be_0x10[
    (sizeof(stRewardGiveEmotion) == 0x10) ? 1 : -1];
typedef char stRewardPostal_Size_Must_Be_0x20[
    (sizeof(stRewardPostal) == 0x20) ? 1 : -1];
typedef char stConditionEventRewardInfo_Size_Must_Be_0x0c[
    (sizeof(stConditionEventRewardInfo) == 0x0c) ? 1 : -1];
typedef char stConditionEventInfo_Size_Must_Be_0x50[
    (sizeof(stConditionEventInfo) == 0x50) ? 1 : -1];
typedef char stEventIdolBringUpParam_t_Size_Must_Be_0x14[
    (sizeof(stEventIdolBringUpParam_t) == 0x14) ? 1 : -1];
typedef char stEVENTADDINFO_Size_Must_Be_0x08[
    (sizeof(stEVENTADDINFO) == 0x08) ? 1 : -1];
typedef char stUnlimitChallengeRewardInfo_Size_Must_Be_0x0c[
    (sizeof(stUnlimitChallengeRewardInfo) == 0x0c) ? 1 : -1];
typedef char stEventMonster_t_Size_Must_Be_0x14[
    (sizeof(stEventMonster_t) == 0x14) ? 1 : -1];
typedef char stSecondEventMonster_t_Size_Must_Be_0x14[
    (sizeof(stSecondEventMonster_t) == 0x14) ? 1 : -1];
typedef char TournamentRewardType_Size_Must_Be_0x0c[
    (sizeof(TournamentRewardType) == 0x0c) ? 1 : -1];
typedef char TournamentRewardRate_Size_Must_Be_0x10[
    (sizeof(TournamentRewardRate) == 0x10) ? 1 : -1];
typedef char TournamentRewardItem_Size_Must_Be_0x0c[
    (sizeof(TournamentRewardItem) == 0x0c) ? 1 : -1];
typedef char TournamentRewardItemRate_Size_Must_Be_0x10[
    (sizeof(TournamentRewardItemRate) == 0x10) ? 1 : -1];
typedef char STUltimateRewardProb_Size_Must_Be_0x08[
    (sizeof(STUltimateRewardProb) == 0x08) ? 1 : -1];
typedef char competition_reward_result_Size_Must_Be_0x0c[
    (sizeof(competition_reward_result) == 0x0c) ? 1 : -1];
typedef char STEventSuper_Size_Must_Be_0x28[
    (sizeof(STEventSuper) == 0x28) ? 1 : -1];
typedef char LotteryInfo_Size_Must_Be_0x20[
    (sizeof(LotteryInfo) == 0x20) ? 1 : -1];
typedef char CLottery_NeedMoney_Size_Must_Be_0x18[
    (sizeof(CLottery_NeedMoney) == 0x18) ? 1 : -1];
typedef char stMercenaryRewardGroup_Size_Must_Be_0x28[
    (sizeof(stMercenaryRewardGroup) == 0x28) ? 1 : -1];
typedef char BingoData_Size_Must_Be_0x08[
    (sizeof(BingoData) == 0x08) ? 1 : -1];
typedef char CLevelRewardTable_Size_Must_Be_0x6088[
    (sizeof(CLevelRewardTable) == 0x6088) ? 1 : -1];
typedef char CLevelRewardTableMgr_Size_Must_Be_0x14[
    (sizeof(CLevelRewardTableMgr) == 0x14) ? 1 : -1];
typedef char InGameEventInfo_Size_Must_Be_0x18[
    (sizeof(InGameEventInfo) == 0x18) ? 1 : -1];
typedef char InGameEventInfoList_Size_Must_Be_0x18[
    (sizeof(InGameEventInfoList) == 0x18) ? 1 : -1];
typedef char InGameEventScript_Size_Must_Be_0x08[
    (sizeof(InGameEventScript) == 0x08) ? 1 : -1];
typedef char AttendanceScript_Size_Must_Be_0x78[
    (sizeof(AttendanceScript) == 0x78) ? 1 : -1];
typedef char ExchangeRandomItemRewardScript_Size_Must_Be_0x68[
    (sizeof(ExchangeRandomItemRewardScript) == 0x68) ? 1 : -1];
typedef char LevelupSupportEventManger_EventInfo_Size_Must_Be_0x04[
    (sizeof(LevelupSupportEventManger::EventInfo) == 0x04) ? 1 : -1];
typedef char LevelupSupportEvent_Size_Must_Be_0x0c[
    (sizeof(LevelupSupportEvent) == 0x0c) ? 1 : -1];
typedef char LevelupSupport2ndEvent_Size_Must_Be_0x0c[
    (sizeof(LevelupSupport2ndEvent) == 0x0c) ? 1 : -1];
typedef char CreateDnfEventSciprt_ItemInfo_Size_Must_Be_0x08[
    (sizeof(CreateDnfEventSciprt::ItemInfo) == 0x08) ? 1 : -1];
typedef char CreateDnfEventSciprt_Size_Must_Be_0x18[
    (sizeof(CreateDnfEventSciprt) == 0x18) ? 1 : -1];
typedef char InterSelectCreateDnfEventInfo_Reward_Size_Must_Be_0x04[
    (sizeof(InterSelectCreateDnfEventInfo::Reward) == 0x04) ? 1 : -1];
typedef char StackablePackage_EventProcessor_Size_Must_Be_0x0c[
    (sizeof(StackablePackage::EventProcessor) == 0x0c) ? 1 : -1];
typedef char StackablePackage_FindControllableEventProcessor_Size_Must_Be_0x04[
    (sizeof(StackablePackage::FindControllableEventProcessor) == 0x04) ? 1 : -1];
typedef char Arad_OnePlusCardEvent_ItemList_Data_Size_Must_Be_0x08[
    (sizeof(Arad_OnePlusCardEvent_ItemList::Data) == 0x08) ? 1 : -1];
typedef char Arad_OnePlusCardEvent_ItemList_Size_Must_Be_0x1c[
    (sizeof(Arad_OnePlusCardEvent_ItemList) == 0x1c) ? 1 : -1];
typedef char CEventStayTime_RewardGoGoFighter_Size_Must_Be_0x08[
    (sizeof(CEventStayTime::RewardGoGoFighter) == 0x08) ? 1 : -1];
typedef char CEventStayTime_Size_Must_Be_0x14[
    (sizeof(CEventStayTime) == 0x14) ? 1 : -1];
typedef char CUnlimitFatigueEvent_Size_Must_Be_0x0c[
    (sizeof(CUnlimitFatigueEvent) == 0x0c) ? 1 : -1];
typedef char CDoubleGoldCardEvent_Size_Must_Be_0x0c[
    (sizeof(CDoubleGoldCardEvent) == 0x0c) ? 1 : -1];
typedef char CDimensionActivationEvent_Size_Must_Be_0x0c[
    (sizeof(CDimensionActivationEvent) == 0x0c) ? 1 : -1];
typedef char CEventAdvanceAltarOpen_Size_Must_Be_0x0c[
    (sizeof(CEventAdvanceAltarOpen) == 0x0c) ? 1 : -1];
typedef char CAssaultOnOffEvent_Size_Must_Be_0x0c[
    (sizeof(CAssaultOnOffEvent) == 0x0c) ? 1 : -1];
typedef char CConditionEvent_Size_Must_Be_0x0c[
    (sizeof(CConditionEvent) == 0x0c) ? 1 : -1];
typedef char CDnFLeaguePromoteFirstEvent_Size_Must_Be_0x0c[
    (sizeof(CDnFLeaguePromoteFirstEvent) == 0x0c) ? 1 : -1];
typedef char CDnFLeaguePromoteSecondEvent_Size_Must_Be_0x0c[
    (sizeof(CDnFLeaguePromoteSecondEvent) == 0x0c) ? 1 : -1];
typedef char CBurnigGoldMonsterEvent_Size_Must_Be_0x0c[
    (sizeof(CBurnigGoldMonsterEvent) == 0x0c) ? 1 : -1];
typedef char Arad_MomijiEvent_Size_Must_Be_0x0c[
    (sizeof(Arad_MomijiEvent) == 0x0c) ? 1 : -1];
typedef char CAradRyosikaEvent_Size_Must_Be_0x14[
    (sizeof(CAradRyosikaEvent) == 0x14) ? 1 : -1];
typedef char AttendanceEvent_Size_Must_Be_0x0c[
    (sizeof(AttendanceEvent) == 0x0c) ? 1 : -1];
typedef char CClearRewardCardEvent_Size_Must_Be_0x0c[
    (sizeof(CClearRewardCardEvent) == 0x0c) ? 1 : -1];
typedef char CCoinEventOnCharCreate_Size_Must_Be_0x10[
    (sizeof(CCoinEventOnCharCreate) == 0x10) ? 1 : -1];
typedef char CCoinEventPerDay_Size_Must_Be_0x10[
    (sizeof(CCoinEventPerDay) == 0x10) ? 1 : -1];
typedef char CCoinRefillEvent_Size_Must_Be_0x20[
    (sizeof(CCoinRefillEvent) == 0x20) ? 1 : -1];
typedef char CCollectArchieveEventLog_Size_Must_Be_0x0c[
    (sizeof(CCollectArchieveEventLog) == 0x0c) ? 1 : -1];
typedef char CExpDoubleEvent_Size_Must_Be_0x10[
    (sizeof(CExpDoubleEvent) == 0x10) ? 1 : -1];
typedef char CItemDropRatioEvent_Size_Must_Be_0x10[
    (sizeof(CItemDropRatioEvent) == 0x10) ? 1 : -1];
typedef char CMaxFatigueFactorEvent_Size_Must_Be_0x10[
    (sizeof(CMaxFatigueFactorEvent) == 0x10) ? 1 : -1];
typedef char CFatigueAttendance_Size_Must_Be_0x0c[
    (sizeof(CFatigueAttendance) == 0x0c) ? 1 : -1];
typedef char CFatigueBuffEvent_Size_Must_Be_0x0c[
    (sizeof(CFatigueBuffEvent) == 0x0c) ? 1 : -1];
typedef char CFatigueGiveItemEvent_Size_Must_Be_0x0c[
    (sizeof(CFatigueGiveItemEvent) == 0x0c) ? 1 : -1];
typedef char CGoldCardBlankItemEvent_Size_Must_Be_0x0c[
    (sizeof(CGoldCardBlankItemEvent) == 0x0c) ? 1 : -1];
typedef char CMemberExpRewardEvent_Size_Must_Be_0x10[
    (sizeof(CMemberExpRewardEvent) == 0x10) ? 1 : -1];
typedef char CNoNeedGoldOnGuildCreateEvent_Size_Must_Be_0x0c[
    (sizeof(CNoNeedGoldOnGuildCreateEvent) == 0x0c) ? 1 : -1];
typedef char CReformingDanjinEvent_Size_Must_Be_0x0c[
    (sizeof(CReformingDanjinEvent) == 0x0c) ? 1 : -1];
typedef char CStabToDeathEvent_Size_Must_Be_0x0c[
    (sizeof(CStabToDeathEvent) == 0x0c) ? 1 : -1];
typedef char CStopOverlabExpEvent_Size_Must_Be_0x0c[
    (sizeof(CStopOverlabExpEvent) == 0x0c) ? 1 : -1];
typedef char CUXGameLogEvent_Size_Must_Be_0x0c[
    (sizeof(CUXGameLogEvent) == 0x0c) ? 1 : -1];
typedef char CVendingMachineBonusEvent_Size_Must_Be_0x0c[
    (sizeof(CVendingMachineBonusEvent) == 0x0c) ? 1 : -1];
typedef char CWeekendBonusEvent_Size_Must_Be_0x0c[
    (sizeof(CWeekendBonusEvent) == 0x0c) ? 1 : -1];
typedef char CPowerWarVictoriousEvent_Size_Must_Be_0x0c[
    (sizeof(CPowerWarVictoriousEvent) == 0x0c) ? 1 : -1];
typedef char CGuildWarEvent_Size_Must_Be_0x10[
    (sizeof(CGuildWarEvent) == 0x10) ? 1 : -1];
typedef char CPowerWarEvent_Size_Must_Be_0x10[
    (sizeof(CPowerWarEvent) == 0x10) ? 1 : -1];
typedef char CSchoolMatchEvent_Size_Must_Be_0x18[
    (sizeof(CSchoolMatchEvent) == 0x18) ? 1 : -1];
typedef char CEventCreateDnf_Size_Must_Be_0x18[
    (sizeof(CEventCreateDnf) == 0x18) ? 1 : -1];
typedef char TimerEndPowerWarVictoriousReward_Size_Must_Be_0x04[
    (sizeof(TimerEndPowerWarVictoriousReward) == 0x04) ? 1 : -1];
typedef char CBreakAwayPreventEvent_Size_Must_Be_0x0c[
    (sizeof(CBreakAwayPreventEvent) == 0x0c) ? 1 : -1];
typedef char CBurningFatigueEvent_Size_Must_Be_0x10[
    (sizeof(CBurningFatigueEvent) == 0x10) ? 1 : -1];
typedef char CFirstServerOpenEvent_Size_Must_Be_0x0c[
    (sizeof(CFirstServerOpenEvent) == 0x0c) ? 1 : -1];
typedef char CGmRegistEvent_Size_Must_Be_0x0c[
    (sizeof(CGmRegistEvent) == 0x0c) ? 1 : -1];
typedef char CTrcOnOffEvent_Size_Must_Be_0x0c[
    (sizeof(CTrcOnOffEvent) == 0x0c) ? 1 : -1];
typedef char LevelUpBefore70LvEvent_Size_Must_Be_0x0c[
    (sizeof(LevelUpBefore70LvEvent) == 0x0c) ? 1 : -1];
typedef char CNotApplyBalkeunEvent_Size_Must_Be_0x0c[
    (sizeof(CNotApplyBalkeunEvent) == 0x0c) ? 1 : -1];
typedef char EventGiveMeBox_Size_Must_Be_0x0c[
    (sizeof(EventGiveMeBox) == 0x0c) ? 1 : -1];
typedef char UsedFatigueRewardItem_Data_Size_Must_Be_0x118[
    (sizeof(UsedFatigueRewardItem::Data) == 0x118) ? 1 : -1];
static_assert(offsetof(UsedFatigueRewardItem::Data, message) == 0x04u,
              "fatigue reward message ABI drift");
static_assert(offsetof(UsedFatigueRewardItem::Data, title) == 0x103u,
              "fatigue reward title ABI drift");
static_assert(offsetof(UsedFatigueRewardItem::Data, item_id) == 0x114u,
              "fatigue reward item id ABI drift");
typedef char Condition_LevelUp_Reward_Item_Size_Must_Be_0x14[
    (sizeof(Condition_LevelUp::Reward_Item) == 0x14) ? 1 : -1];
typedef char CBattle_Field_stEventMonsterStatus_t_Size_Must_Be_0x18[
    (sizeof(CBattle_Field::stEventMonsterStatus_t) == 0x18) ? 1 : -1];
typedef char advancealtar_Reward_Size_Must_Be_0x05[
    (sizeof(advancealtar::_Reward) == 0x05) ? 1 : -1];
typedef char advancealtar_RewardItem_Size_Must_Be_0x0c[
    (sizeof(advancealtar::_RewardItem) == 0x0c) ? 1 : -1];
typedef char advancealtar_ClearRewardData_Size_Must_Be_0x24[
    (sizeof(advancealtar::ClearRewardData) == 0x24) ? 1 : -1];
typedef char advancealtar_RoundRewardData_Size_Must_Be_0x14[
    (sizeof(advancealtar::RoundRewardData) == 0x14) ? 1 : -1];
typedef char advancealtar_SurvivalRewardData_Size_Must_Be_0x0c[
    (sizeof(advancealtar::SurvivalRewardData) == 0x0c) ? 1 : -1];
typedef char advancealtar_AchievementRewardData_Size_Must_Be_0x28[
    (sizeof(advancealtar::AchievementRewardData) == 0x28) ? 1 : -1];
typedef char advancealtar_RewardParameter_Size_Must_Be_0x18[
    (sizeof(advancealtar::RewardParameter) == 0x18) ? 1 : -1];
typedef char APSystem_TodayRewardItem_Size_Must_Be_0x0e[
    (sizeof(APSystem::_TodayRewardItem) == 0x0e) ? 1 : -1];
typedef char APSystem_Inter_LoadRewardMedal_Size_Must_Be_0x04[
    (sizeof(APSystem::Inter_LoadRewardMedal) == 0x04) ? 1 : -1];
typedef char ChattingEmoticon_byEvent_Size_Must_Be_0x20[
    (sizeof(ChattingEmoticon_byEvent) == 0x20) ? 1 : -1];
typedef char MultiBoxLotteryInfo_LotteryDefaultArad_Size_Must_Be_0x0c[
    (sizeof(MultiBoxLotteryInfo::LotteryDefaultArad) == 0x0c) ? 1 : -1];
typedef char MultiBoxLotteryInfo_stDefaultItem_Size_Must_Be_0x10[
    (sizeof(MultiBoxLotteryInfo::stDefaultItem) == 0x10) ? 1 : -1];
typedef char MultiBoxLotteryInfo_stDefaultItemInfo_Size_Must_Be_0x1c[
    (sizeof(MultiBoxLotteryInfo::stDefaultItemInfo) == 0x1c) ? 1 : -1];
typedef char MultiBoxLotteryInfo_LotteryUnit_Size_Must_Be_0x28[
    (sizeof(MultiBoxLotteryInfo::LotteryUnit) == 0x28) ? 1 : -1];
typedef char MultiBoxLotteryInfo_Size_Must_Be_0x38[
    (sizeof(MultiBoxLotteryInfo) == 0x38) ? 1 : -1];

typedef char EVENT_SERVER_MESSAGE_Size_Must_Be_0x10[
    (sizeof(EVENT_SERVER_MESSAGE) == 0x10) ? 1 : -1];

// IDA_EA: 0x831D338
// IDA_NAME: _ZNK15STRewardDataKeyltERKS_
// DEMANGLED: STRewardDataKey::operator<(STRewardDataKey const&)const
bool STRewardDataKey::operator<(const STRewardDataKey& rhs) const {
    if (first < rhs.first) {
        return true;
    }
    if (first != rhs.first) {
        return false;
    }
    if (second < rhs.second) {
        return true;
    }
    return second == rhs.second && third < rhs.third;
}

// IDA_EA: 0x831D3AE
// IDA_NAME: _ZN15STRewardDataKeyC2Ev
// DEMANGLED: STRewardDataKey::STRewardDataKey(void)
STRewardDataKey::STRewardDataKey()
    : first(0),
      second(0),
      third(0) {}

// IDA_EA: 0x831D3C8
// IDA_NAME: _ZN12STRewardDataC2Ev
// DEMANGLED: STRewardData::STRewardData(void)
STRewardData::STRewardData()
    : item_id(0),
      item_count(0),
      item_period(0),
      reward_type(0),
      reward_value(0) {}

// IDA_EA: 0x831D412
// IDA_NAME: _ZN16STRewardDataListC2Ev
// DEMANGLED: STRewardDataList::STRewardDataList(void)
STRewardDataList::STRewardDataList() {
    reward_data.clear();
}

// IDA_EA: 0x831D55C
// IDA_NAME: _ZN16STRewardDataListD2Ev
// DEMANGLED: STRewardDataList::~STRewardDataList()
STRewardDataList::~STRewardDataList() {}

// IDA_EA: 0x831D7A0
// IDA_NAME: _ZN16STRewardDataListC2ERKS_
// DEMANGLED: STRewardDataList::STRewardDataList(STRewardDataList const&)
STRewardDataList::STRewardDataList(const STRewardDataList& rhs)
    : reward_data(rhs.reward_data) {}

// IDA_EA: 0x831D454
// IDA_NAME: _ZN18STBreakAwayRewards5clearEv
// DEMANGLED: STBreakAwayRewards::clear(void)
STBreakAwayRewards* STBreakAwayRewards::clear() {
    item_id = 0;
    item_count = 0;
    luck_point = 0;
    reward_point = 0;
    reward_order = 0;
    return this;
}

// IDA_EA: 0x831D48A
// IDA_NAME: _ZN18STBreakAwayRewardsaSERKS_
// DEMANGLED: STBreakAwayRewards::operator=(STBreakAwayRewards const&)
STBreakAwayRewards& STBreakAwayRewards::operator=(
    const STBreakAwayRewards& rhs) {
    item_id = rhs.item_id;
    item_count = rhs.item_count;
    luck_point = rhs.luck_point;
    reward_point = rhs.reward_point;
    reward_order = rhs.reward_order;
    return *this;
}

// IDA_EA: 0x831D4CA
// IDA_NAME: _ZN18STBreakAwayRewardsC2Ev
// DEMANGLED: STBreakAwayRewards::STBreakAwayRewards(void)
STBreakAwayRewards::STBreakAwayRewards() {
    clear();
}

// IDA_EA: 0x826FAA0
// IDA_NAME: _ZN19EventRewardItemInfoD2Ev
// DEMANGLED: EventRewardItemInfo::~EventRewardItemInfo()
EventRewardItemInfo::~EventRewardItemInfo() {}

// IDA_EA: 0x826FAB6
// IDA_NAME: _ZN19EventRewardItemInfoaSERKS_
// DEMANGLED: EventRewardItemInfo::operator=(EventRewardItemInfo const&)
EventRewardItemInfo& EventRewardItemInfo::operator=(
    const EventRewardItemInfo& rhs) {
    first_dword = rhs.first_dword;
    second_dword = rhs.second_dword;
    word_08 = rhs.word_08;
    values = rhs.values;
    return *this;
}

// IDA_EA: 0x891548A
// IDA_NAME: _ZN19EventRewardItemInfoC2Ev
// DEMANGLED: EventRewardItemInfo::EventRewardItemInfo(void)
EventRewardItemInfo::EventRewardItemInfo() {
    first_dword = 0;
    second_dword = 0;
    word_08 = 0;
}

// IDA_EA: 0x89154BC
// IDA_NAME: _ZN19EventRewardItemInfo5clearEv
// DEMANGLED: EventRewardItemInfo::clear(void)
EventRewardItemInfo* EventRewardItemInfo::clear() {
    first_dword = 0;
    second_dword = 0;
    word_08 = 0;
    values.clear();
    return this;
}

// IDA_EA: 0x89333B0
// IDA_NAME: _ZN19EventRewardItemInfoC2ERKS_
// DEMANGLED: EventRewardItemInfo::EventRewardItemInfo(EventRewardItemInfo const&)
EventRewardItemInfo::EventRewardItemInfo(const EventRewardItemInfo& rhs)
    : first_dword(rhs.first_dword),
      second_dword(rhs.second_dword),
      word_08(rhs.word_08),
      values(rhs.values) {}

// IDA_EA: 0x83DCE92
// IDA_NAME: _ZN18EventRewardSectionD2Ev
// DEMANGLED: EventRewardSection::~EventRewardSection()
EventRewardSection::~EventRewardSection() {}

// IDA_EA: 0x89154EE
// IDA_NAME: _ZN18EventRewardSectionC2Ev
// DEMANGLED: EventRewardSection::EventRewardSection(void)
EventRewardSection::EventRewardSection() {
    byte_00 = 0;
    byte_01 = 0;
    byte_02 = 0;
}

// IDA_EA: 0x8932FAA
// IDA_NAME: _ZN18EventRewardSectionC2ERKS_
// DEMANGLED: EventRewardSection::EventRewardSection(EventRewardSection const&)
EventRewardSection::EventRewardSection(const EventRewardSection& rhs)
    : byte_00(rhs.byte_00),
      byte_01(rhs.byte_01),
      byte_02(rhs.byte_02),
      values(rhs.values) {}

// IDA_EA: 0x893304E
// IDA_NAME: _ZN18EventRewardSectionaSERKS_
// DEMANGLED: EventRewardSection::operator=(EventRewardSection const&)
EventRewardSection& EventRewardSection::operator=(
    const EventRewardSection& rhs) {
    byte_00 = rhs.byte_00;
    byte_01 = rhs.byte_01;
    byte_02 = rhs.byte_02;
    values = rhs.values;
    return *this;
}

// IDA_EA: 0x8371348
// IDA_NAME: _ZN29DimensionActivationRewardDataC2Ev
// DEMANGLED: DimensionActivationRewardData::DimensionActivationRewardData(void)
DimensionActivationRewardData::DimensionActivationRewardData() {}

// IDA_EA: 0x83713B4
// IDA_NAME: _ZN29DimensionActivationRewardDataD2Ev
// DEMANGLED: DimensionActivationRewardData::~DimensionActivationRewardData()
DimensionActivationRewardData::~DimensionActivationRewardData() {}

// IDA_EA: 0x8915F16
// IDA_NAME: _ZN29DimensionActivationRewardData14getSectionDataEi
// DEMANGLED: DimensionActivationRewardData::getSectionData(int)
EventRewardSection* DimensionActivationRewardData::getSectionData(
    int section_value) {
    for (std::vector<EventRewardSection>::reverse_iterator it =
             sections.rbegin();
         it != sections.rend(); ++it) {
        if (it->byte_01 <= section_value) {
            return &(*it);
        }
    }
    return 0;
}

// IDA_EA: 0x8915F9A
// IDA_NAME: _ZN29DimensionActivationRewardData18getRewardGiveItemsEiii
// DEMANGLED: DimensionActivationRewardData::getRewardGiveItems(int,int,int)
std::vector<EventRewardItemInfo>*
DimensionActivationRewardData::getRewardGiveItems(
    int reward_index,
    int charac_job,
    int first_grow_type) {
    std::vector<std::vector<EventRewardItemInfo> >* items =
        getRewardGiveItemList(charac_job, first_grow_type);
    if (items == 0) {
        return 0;
    }
    if (reward_index < 0 ||
        items->size() < static_cast<std::vector<
                            std::vector<EventRewardItemInfo> >::size_type>(
                            reward_index)) {
        return 0;
    }
    return &items->at(reward_index);
}

// IDA_EA: 0x8916004
// IDA_NAME: _ZN29DimensionActivationRewardData21getRewardGiveItemListEii
// DEMANGLED: DimensionActivationRewardData::getRewardGiveItemList(int,int)
std::vector<std::vector<EventRewardItemInfo> >*
DimensionActivationRewardData::getRewardGiveItemList(
    int charac_job,
    int first_grow_type) {
    std::map<std::pair<int, int>,
             std::vector<std::vector<EventRewardItemInfo> > >::iterator it =
        give_groups.find(std::make_pair(charac_job, first_grow_type));
    if (it == give_groups.end()) {
        return 0;
    }
    return &it->second;
}

// IDA_EA: 0x8916086
// IDA_NAME: _ZN29DimensionActivationRewardData23getRewardSelectItemListEi
// DEMANGLED: DimensionActivationRewardData::getRewardSelectItemList(int)
std::vector<std::vector<EventRewardItemInfo> >*
DimensionActivationRewardData::getRewardSelectItemList(int select_group) {
    std::map<int, std::vector<std::vector<EventRewardItemInfo> > >::iterator
        it = select_groups.find(select_group);
    if (it == select_groups.end()) {
        return 0;
    }
    return &it->second;
}

// IDA_EA: 0x836DE56
// IDA_NAME: _ZN14RewardItemInfoC2Ev
// DEMANGLED: RewardItemInfo::RewardItemInfo(void)
RewardItemInfo::RewardItemInfo()
    : item_id(0),
      item_count(0),
      item_type(oldsrv::config::kCharacterJobCommon),
      period(-1) {}

// IDA_EA: 0x83C5056
// IDA_NAME: _ZN14RewardItemInfoD2Ev
// DEMANGLED: RewardItemInfo::~RewardItemInfo()
RewardItemInfo::~RewardItemInfo() {}

// IDA_EA: 0x81BBF82
// IDA_NAME: _ZN14RewardUserDataC2Ev
// DEMANGLED: RewardUserData::RewardUserData(void)
RewardUserData::RewardUserData()
    : first_dword(0),
      second_dword(0),
      byte_08(0) {}

// IDA_EA: 0x81BB690
// IDA_NAME: _ZN15CRewardUserList6insertEjii
// DEMANGLED: CRewardUserList::insert(unsigned int,int,int)
int CRewardUserList::insert(unsigned int account_id,
                            int charac_no,
                            int ontime_id) {
    if (is_exist(account_id)) {
        return 0;
    }

    RewardUserData data;
    data.first_dword = static_cast<uint32_t>(charac_no);
    data.second_dword = static_cast<uint32_t>(ontime_id);
    data.byte_08 = 0;
    reward_users.insert(std::make_pair(account_id, data));
    account_index.push_back(account_id);
    return 1;
}

// IDA_EA: 0x81BB73C
// IDA_NAME: _ZN15CRewardUserList8is_existEj
// DEMANGLED: CRewardUserList::is_exist(unsigned int)
bool CRewardUserList::is_exist(unsigned int account_id) {
    return reward_users.find(account_id) != reward_users.end();
}

// IDA_EA: 0x81BB7BC
// IDA_NAME: _ZN15CRewardUserList11delete_markEj
// DEMANGLED: CRewardUserList::delete_mark(unsigned int)
int CRewardUserList::delete_mark(unsigned int account_id) {
    std::map<unsigned int, RewardUserData>::iterator it =
        reward_users.find(account_id);
    if (it == reward_users.end()) {
        return 0;
    }
    it->second.byte_08 = 1;
    return 1;
}

// IDA_EA: 0x81BBFB6
// IDA_NAME: _ZN15CRewardUserListC2Ev
// DEMANGLED: CRewardUserList::CRewardUserList(void)
CRewardUserList::CRewardUserList() {}

// IDA_EA: 0x81BBFFC
// IDA_NAME: _ZN15CRewardUserListD2Ev
// DEMANGLED: CRewardUserList::~CRewardUserList()
CRewardUserList::~CRewardUserList() {}

// IDA_EA: 0x81BC042
// IDA_NAME: _ZN15CRewardUserList5clearEv
// DEMANGLED: CRewardUserList::clear(void)
int CRewardUserList::clear() {
    reward_users.clear();
    account_index.clear();
    return static_cast<int>(reinterpret_cast<intptr_t>(this));
}

// IDA_EA: 0x81BC064
// IDA_NAME: _ZN15CRewardUserList14get_index_sizeEv
// DEMANGLED: CRewardUserList::get_index_size(void)
int CRewardUserList::get_index_size() {
    return static_cast<int>(account_index.size());
}

// IDA_EA: 0x81BB826
// IDA_NAME: _ZN12COnTimeEventC2Ev
// DEMANGLED: COnTimeEvent::COnTimeEvent(void)
COnTimeEvent::COnTimeEvent()
    : CEventBase(),
      ontime_id_(0),
      reward_state_(1),
      reward_users_() {
    reward_users_.clear();
}

// IDA_EA: 0x81BC07A
// IDA_NAME: _ZN12COnTimeEventD2Ev
// DEMANGLED: COnTimeEvent::~COnTimeEvent()
// IDA_EA: 0x81BC0DC
// IDA_NAME: _ZN12COnTimeEventD0Ev
// DEMANGLED: COnTimeEvent::~COnTimeEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
COnTimeEvent::~COnTimeEvent() {}

// IDA_EA: 0x81BB8AE
// IDA_NAME: _ZN12COnTimeEvent10StartEventEv
// DEMANGLED: COnTimeEvent::StartEvent(void)
void COnTimeEvent::StartEvent() {
    SetEventFlag(true);
}

// IDA_EA: 0x81BB8CA
// IDA_NAME: _ZN12COnTimeEvent10StartEventE10Word_Param
// DEMANGLED: COnTimeEvent::StartEvent(Word_Param)
void COnTimeEvent::StartEvent(Word_Param param) {
    (void)param;
    StartEvent();
    cMyTrace trace("virtual void COnTimeEvent::StartEvent(Word_Param)",
                   71,
                   0);
    trace("COnTimeEvent Event Start!");
}

// IDA_EA: 0x81BB91A
// IDA_NAME: _ZN12COnTimeEvent8EndEventEv
// DEMANGLED: COnTimeEvent::EndEvent(void)
void COnTimeEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void COnTimeEvent::EndEvent()", 77, 0);
    trace("COnTimeEvent Event End!");
}

// IDA_EA: 0x81BB96C
// IDA_NAME: _ZN12COnTimeEvent16get_reward_stateEv
// DEMANGLED: COnTimeEvent::get_reward_state(void)
int COnTimeEvent::get_reward_state() {
    return reward_state_;
}

// IDA_EA: 0x81BB978
// IDA_NAME: _ZN12COnTimeEvent16set_reward_stateEi
// DEMANGLED: COnTimeEvent::set_reward_state(int)
COnTimeEvent* COnTimeEvent::set_reward_state(int state) {
    reward_state_ = state;
    return this;
}

// IDA_EA: 0x81BB986
// IDA_NAME: _ZN12COnTimeEvent13set_ontime_idEi
// DEMANGLED: COnTimeEvent::set_ontime_id(int)
COnTimeEvent* COnTimeEvent::set_ontime_id(int ontime_id) {
    ontime_id_ = ontime_id;
    return this;
}

// IDA_EA: 0x81BB994
// IDA_NAME: _ZN12COnTimeEvent19set_reward_end_timeEi
// DEMANGLED: COnTimeEvent::set_reward_end_time(int)
COnTimeEvent* COnTimeEvent::set_reward_end_time(int seconds_from_now) {
    reward_end_time_ =
        seconds_from_now + static_cast<int>(std::time(0));
    return this;
}

// IDA_EA: 0x81BB9BA
// IDA_NAME: _ZN12COnTimeEvent19get_reward_end_timeEv
// DEMANGLED: COnTimeEvent::get_reward_end_time(void)
int COnTimeEvent::get_reward_end_time() {
    return reward_end_time_;
}

// IDA_EA: 0x81BB9C6
// IDA_NAME: _ZN12COnTimeEvent28get_ontime_reward_user_countEv
// DEMANGLED: COnTimeEvent::get_ontime_reward_user_count(void)
int COnTimeEvent::get_ontime_reward_user_count() {
    return reward_users_.get_index_size();
}

// IDA_EA: 0x81BB9DC
// IDA_NAME: _ZN12COnTimeEvent18insert_reward_userEP5CUser
// DEMANGLED: COnTimeEvent::insert_reward_user(CUser *)
int COnTimeEvent::insert_reward_user(CUser* user) {
    if (!user) {
        return 0;
    }

    const int ontime_id = get_ontime_id();
    if (ontime_id <= user->get_ontime_last_recv_idx()) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_ONTIME_EVENT_REQUEST_REWARD, 18);
        return 0;
    }

    reward_users_.insert(
        static_cast<unsigned int>(user->get_acc_id()),
        user->get_charac_no(-1),
        ontime_id);
    user->set_ontime_last_recv_idx(ontime_id);
    user->set_update_ontime_last_recv_idx();
    user->SendCmdErrorPacket(
        ENUM_CMDPACKET_ONTIME_EVENT_REQUEST_REWARD, 0);
    return 1;
}

// IDA_EA: 0x81BBC64
// IDA_NAME: _ZN12COnTimeEvent18delete_reward_userEP5CUser
// DEMANGLED: COnTimeEvent::delete_reward_user(CUser *)
int COnTimeEvent::delete_reward_user(CUser* user) {
    if (!user) {
        return 0;
    }
    if (get_ontime_id() != user->get_ontime_last_recv_idx()) {
        return 0;
    }
    reward_users_.delete_mark(static_cast<unsigned int>(user->get_acc_id()));
    return 1;
}

// IDA_EA: 0x81BBCC8
// IDA_NAME: _ZN12COnTimeEvent22set_ontime_reward_itemEii
// DEMANGLED: COnTimeEvent::set_ontime_reward_item(int,int)
COnTimeEvent* COnTimeEvent::set_ontime_reward_item(int item_no,
                                                   int item_count) {
    reward_item_no_ = item_no;
    reward_item_count_ = item_count;
    return this;
}

// IDA_EA: 0x81BBCE0
// IDA_NAME: _ZN12COnTimeEvent24process_set_reward_startEiiii
// DEMANGLED: COnTimeEvent::process_set_reward_start(int,int,int,int)
int COnTimeEvent::process_set_reward_start(int ontime_id,
                                           int item_no,
                                           int item_count,
                                           int reward_seconds) {
    set_reward_state(0);
    set_ontime_id(ontime_id);
    set_ontime_reward_item(item_no, item_count);
    set_reward_end_time(reward_seconds);
    return reward_users_.clear();
}

// IDA_EA: 0x81BBD46
// IDA_NAME: _ZN12COnTimeEvent27process_send_reward_to_userEv
// DEMANGLED: COnTimeEvent::process_send_reward_to_user(void)
int COnTimeEvent::process_send_reward_to_user() {
    int processed = 0;
    while (!reward_users_.account_index.empty() && processed <= 50) {
        const unsigned int account_id =
            reward_users_.account_index.front();
        std::map<unsigned int, RewardUserData>::const_iterator reward =
            reward_users_.reward_users.find(account_id);
        if (reward != reward_users_.reward_users.end() &&
            reward->second.byte_08 != 1) {
            CUser* const user =
                G_GameWorld()->find_user_from_world_byaccid(account_id);
            if (user &&
                user->get_charac_no(-1) ==
                    static_cast<int>(reward->second.first_dword) &&
                user->get_state() > 2) {
                if (user->InsertOnTimeEventReward(
                        static_cast<unsigned int>(
                            get_ontime_reward_item_no()),
                        static_cast<unsigned int>(
                            get_ontime_reward_item_count())) == 1) {
                    const int occurrence_id = get_ontime_id();
                    freely_give_item::OnTimedLoginAward(
                        user, static_cast<uint32_t>(occurrence_id));
                    DB_InsertOnTimeRewardUserLog::makeRequest(
                        user->GetUID(), account_id, occurrence_id);
                }
            }
        }

        ++processed;
        reward_users_.account_index.pop_front();
    }
    return 1;
}

// IDA_EA: 0x81BC0FA
// IDA_NAME: _ZN12COnTimeEvent13get_ontime_idEv
// DEMANGLED: COnTimeEvent::get_ontime_id(void)
int COnTimeEvent::get_ontime_id() {
    return ontime_id_;
}

// IDA_EA: 0x81BC106
// IDA_NAME: _ZN12COnTimeEvent25get_ontime_reward_item_noEv
// DEMANGLED: COnTimeEvent::get_ontime_reward_item_no(void)
int COnTimeEvent::get_ontime_reward_item_no() {
    return reward_item_no_;
}

// IDA_EA: 0x81BC112
// IDA_NAME: _ZN12COnTimeEvent28get_ontime_reward_item_countEv
// DEMANGLED: COnTimeEvent::get_ontime_reward_item_count(void)
int COnTimeEvent::get_ontime_reward_item_count() {
    return reward_item_count_;
}

namespace {

// The Inter reply records are consumed at their original byte offsets.  The
// DB dispatcher passes the raw reply buffer; original code did not validate
// its size before loading these fields.
int ReadOnTimeRewardReplyInt(const char* payload, unsigned int offset) {
    int value = 0;
    std::memcpy(&value, payload + offset, sizeof(value));
    return value;
}

COnTimeEvent* GetOnTimeRewardEvent() {
    return static_cast<COnTimeEvent*>(
        GlobalData::s_event_manager->GetRepeatEvent(51));
}

}  // namespace

// IDA_EA: 0x84EE650
// IDA_NAME: _ZN28Inter_OnTimeEventRewardStartC2Ev
// DEMANGLED: Inter_OnTimeEventRewardStart::Inter_OnTimeEventRewardStart(void)
Inter_OnTimeEventRewardStart::Inter_OnTimeEventRewardStart()
    : ISigDispatcher() {}

// IDA_EA: 0x84F70DA
// IDA_NAME: _ZN28Inter_OnTimeEventRewardStartD2Ev
// DEMANGLED: Inter_OnTimeEventRewardStart::~Inter_OnTimeEventRewardStart()
// IDA_EA: 0x84F710A
// IDA_NAME: _ZN28Inter_OnTimeEventRewardStartD0Ev
Inter_OnTimeEventRewardStart::~Inter_OnTimeEventRewardStart() {}

// IDA_EA: 0x84E0DC6
// IDA_NAME: _ZN28Inter_OnTimeEventRewardStart12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_OnTimeEventRewardStart::dispatch_sig(CUser *,char *,int)
int Inter_OnTimeEventRewardStart::dispatch_sig(CUser* user,
                                               char* payload,
                                               int payload_size) {
    (void)user;
    (void)payload_size;

    const int reward_seconds = ReadOnTimeRewardReplyInt(payload, 14);
    COnTimeEvent* const event = GetOnTimeRewardEvent();
    if (event && event->IsEventing(0)) {
        event->process_set_reward_start(
            ReadOnTimeRewardReplyInt(payload, 10),
            ReadOnTimeRewardReplyInt(payload, 18),
            ReadOnTimeRewardReplyInt(payload, 22),
            reward_seconds);

        PacketGuard packet;
        packet.put_header(0, 261);
        packet.put_int(reward_seconds);
        packet.finalize(true);
        G_GameWorld()->send_all_with_state(packet, 3);
    } else {
        // The original error trace formats the on-time ID from +10.
        (void)ReadOnTimeRewardReplyInt(payload, 10);
    }
    return 0;
}

// IDA_EA: 0x84F708C
// IDA_NAME: _ZN35Inter_OnTimeEventRewardStartOnLoginD2Ev
// DEMANGLED: Inter_OnTimeEventRewardStartOnLogin::~Inter_OnTimeEventRewardStartOnLogin()
// IDA_EA: 0x84F70BC
// IDA_NAME: _ZN35Inter_OnTimeEventRewardStartOnLoginD0Ev
// One C++ virtual destructor emits both old ABI entries: D2 resets the
// derived vptr and destroys ISigDispatcher, while D0 invokes D2 then delete.
Inter_OnTimeEventRewardStartOnLogin::~Inter_OnTimeEventRewardStartOnLogin() {}

// IDA_EA: 0x84E0F82
// IDA_NAME: _ZN35Inter_OnTimeEventRewardStartOnLogin12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_OnTimeEventRewardStartOnLogin::dispatch_sig(CUser *,char *,int)
int Inter_OnTimeEventRewardStartOnLogin::dispatch_sig(CUser* user,
                                                      char* payload,
                                                      int payload_size) {
    (void)payload_size;

    if (user) {
        // IDA proves only these two raw reply dwords.  No complete reply
        // layout, prefix type, or semantic name for the +10 value is inferred.
        const int charac_no = ReadOnTimeRewardReplyInt(payload, 18);
        if (charac_no == user->get_charac_no(-1)) {
            PacketGuard packet;
            packet.put_header(0, 261);
            packet.put_int(ReadOnTimeRewardReplyInt(payload, 10));
            packet.finalize(true);
            user->Send(packet);
        }
    }
    return 0;
}

// IDA_EA: 0x84EE66C
// IDA_NAME: _ZN26Inter_OnTimeEventRewardEndC2Ev
// DEMANGLED: Inter_OnTimeEventRewardEnd::Inter_OnTimeEventRewardEnd(void)
Inter_OnTimeEventRewardEnd::Inter_OnTimeEventRewardEnd()
    : ISigDispatcher() {}

// IDA_EA: 0x84F703E
// IDA_NAME: _ZN26Inter_OnTimeEventRewardEndD2Ev
// DEMANGLED: Inter_OnTimeEventRewardEnd::~Inter_OnTimeEventRewardEnd()
// IDA_EA: 0x84F706E
// IDA_NAME: _ZN26Inter_OnTimeEventRewardEndD0Ev
Inter_OnTimeEventRewardEnd::~Inter_OnTimeEventRewardEnd() {}

// IDA_EA: 0x84E1062
// IDA_NAME: _ZN26Inter_OnTimeEventRewardEnd12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_OnTimeEventRewardEnd::dispatch_sig(CUser *,char *,int)
int Inter_OnTimeEventRewardEnd::dispatch_sig(CUser* user,
                                             char* payload,
                                             int payload_size) {
    (void)user;
    (void)payload;
    (void)payload_size;

    COnTimeEvent* const event = GetOnTimeRewardEvent();
    if (event && event->IsEventing(0)) {
        event->set_reward_state(1);
        if (event->get_ontime_reward_user_count()) {
            Timer_Send_Ontime_Reward::RegistNextTimer();
        }

        PacketGuard packet;
        packet.put_header(0, 262);
        packet.finalize(true);
        G_GameWorld()->send_all_with_state(packet, 3);
    }
    return 0;
}

// IDA_EA: 0x863D016
// IDA_NAME: _ZN24Timer_Send_Ontime_RewardC2Ev
// DEMANGLED: Timer_Send_Ontime_Reward::Timer_Send_Ontime_Reward(void)
Timer_Send_Ontime_Reward::Timer_Send_Ontime_Reward()
    : TimerSig() {}

// IDA_EA: 0x863E796
// IDA_NAME: _ZN24Timer_Send_Ontime_RewardD2Ev
// DEMANGLED: Timer_Send_Ontime_Reward::~Timer_Send_Ontime_Reward()
// IDA_EA: 0x863E7C6
// IDA_NAME: _ZN24Timer_Send_Ontime_RewardD0Ev
Timer_Send_Ontime_Reward::~Timer_Send_Ontime_Reward() {}

// IDA_EA: 0x863AB4C
// IDA_NAME: _ZN24Timer_Send_Ontime_Reward15RegistNextTimerEv
// DEMANGLED: Timer_Send_Ontime_Reward::RegistNextTimer(void)
int Timer_Send_Ontime_Reward::RegistNextTimer() {
    return G_TimerQueue()->InsertTimer(2, 0, 155, 10, 0, 0);
}

// IDA_EA: 0x863AA90
// IDA_NAME: _ZN24Timer_Send_Ontime_Reward12dispatch_sigEiij
// DEMANGLED: Timer_Send_Ontime_Reward::dispatch_sig(int,int,unsigned int)
int Timer_Send_Ontime_Reward::dispatch_sig(int object_id,
                                           int param1,
                                           unsigned int param2) {
    (void)object_id;
    (void)param1;
    (void)param2;

    COnTimeEvent* const event = GetOnTimeRewardEvent();
    if (!event || !event->IsEventing(0)) {
        return 0;
    }

    event->process_send_reward_to_user();
    if (event->get_ontime_reward_user_count()) {
        RegistNextTimer();
    }
    return 1;
}

// IDA_EA: 0x836DE84
// IDA_NAME: _ZN12PostalReward5clearEv
// DEMANGLED: PostalReward::clear(void)
PostalReward* PostalReward::clear() {
    first_dword = 0;
    second_dword = 0;
    first_string.clear();
    second_string.clear();
    return this;
}

// IDA_EA: 0x836E456
// IDA_NAME: _ZN12PostalRewardC2Ev
// DEMANGLED: PostalReward::PostalReward(void)
PostalReward::PostalReward() {}

// IDA_EA: 0x836E49C
// IDA_NAME: _ZN12PostalRewardD2Ev
// DEMANGLED: PostalReward::~PostalReward()
PostalReward::~PostalReward() {}

// IDA_EA: 0x837A67E
// IDA_NAME: _ZN12PostalRewardaSERKS_
// DEMANGLED: PostalReward::operator=(PostalReward const&)
PostalReward& PostalReward::operator=(const PostalReward& rhs) {
    first_string = rhs.first_string;
    second_string = rhs.second_string;
    first_dword = rhs.first_dword;
    second_dword = rhs.second_dword;
    return *this;
}

// IDA_EA: 0x837AD5C
// IDA_NAME: _ZN12PostalRewardC2ERKS_
// DEMANGLED: PostalReward::PostalReward(PostalReward const&)
PostalReward::PostalReward(const PostalReward& rhs)
    : first_string(rhs.first_string),
      second_string(rhs.second_string),
      first_dword(rhs.first_dword),
      second_dword(rhs.second_dword) {}

// IDA_EA: 0x85BED52
// IDA_NAME: _ZN11STExpRewardC2Ev
// DEMANGLED: STExpReward::STExpReward(void)
STExpReward::STExpReward()
    : section_index(0),
      gained_exp_bonus_rate(0),
      clear_reward_exp_bonus_rate(0),
      additional_reward_rate(0) {}

// IDA_EA: 0x8941CF6
// IDA_NAME: _ZN11STExpRewardD2Ev
// DEMANGLED: STExpReward::~STExpReward()
STExpReward::~STExpReward() {}

// IDA_EA: 0x891B548
// IDA_NAME: _ZN16stTempRewardInfoC2Ev
// DEMANGLED: stTempRewardInfo::stTempRewardInfo(void)
stTempRewardInfo::stTempRewardInfo()
    : first_dword(3),
      second_dword(1) {}

// IDA_EA: 0x814A800
// IDA_NAME: _ZN23stClearRewardBonusExp_tC2Ev
// DEMANGLED: stClearRewardBonusExp_t::stClearRewardBonusExp_t(void)
stClearRewardBonusExp_t::stClearRewardBonusExp_t()
    : avatar_bonus_exp(0),
      rank_bonus_exp(0),
      event_bonus_type(0),
      event_bonus_exp(0),
      burning_fatigue_bonus_exp(0),
      reserved_bonus_exp(0),
      premium_advantage_bonus_exp(0),
      server_event_bonus_exp(0),
      channel_bonus_exp(0),
      creature_bonus_exp(0),
      upper_member_bonus_exp(0),
      period_exp_item_bonus_exp(0),
      same_pc_room_party_bonus_exp(0),
      return_user_party_bonus_exp(0),
      new_character_party_bonus_exp(0),
      character_manage_bonus_exp(0),
      growth_power_bonus_exp(0),
      premium_exp_bonus_exp(0) {}

// IDA_EA: 0x814A8B8
// IDA_NAME: _ZN27stClearRewardExpParameter_tC2Ev
// DEMANGLED: stClearRewardExpParameter_t::stClearRewardExpParameter_t(void)
stClearRewardExpParameter_t::stClearRewardExpParameter_t()
    : death_penalty_rate(0.0f),
      rank_bonus_rate(0.0f),
      avatar_bonus_rate(0.0f),
      creature_bonus_rate(0.0f),
      pc_room_burning_rate(0.0f),
      double_exp_event_rate(0.0f),
      reserved_bonus_rate(0.0f),
      burning_fatigue_bonus_percent(0),
      channel_bonus_rate(0.0f),
      premium_advantage_rate(0.0f),
      server_event_rate(0.0f),
      period_exp_item_rate(0.0f),
      global_effect_rate(0.0f),
      same_pc_room_party_rate(0.0f),
      return_user_party_rate(0.0f),
      character_manage_rate(0.0f),
      growth_power_rate(0.0f),
      premium_exp_rate(0.0f),
      new_character_party_rate(0.0f),
      reserved_rate_19(0.0f),
      reserved_rate_20(0.0f),
      reserved_flag_54(0),
      reserved_flag_55(0),
      reserved_56(),
      member_slot(-1),
      rank_bonus_index(-1) {}

// IDA_EA: 0x8A8CCC4
// IDA_NAME: _ZN21stFatigueEventParam_tC2Ev
// DEMANGLED: stFatigueEventParam_t::stFatigueEventParam_t(void)
stFatigueEventParam_t::stFatigueEventParam_t() {
    word_00 = 0;
    dword_04 = 0;
    std::memset(bytes_08, 0, 4u);
}

// IDA_EA: 0x8A91DFC
// IDA_NAME: _ZN21stFatigueEventParam_tD2Ev
// DEMANGLED: stFatigueEventParam_t::~stFatigueEventParam_t()
stFatigueEventParam_t::~stFatigueEventParam_t() {}

// IDA_EA: 0x8AA9632
// IDA_NAME: _ZN18stTitleRewardBasisC2Ev
// DEMANGLED: stTitleRewardBasis::stTitleRewardBasis(void)
stTitleRewardBasis::stTitleRewardBasis()
    : reward_kind(4),
      flag_04(0),
      reward_action(0) {}

// IDA_EA: 0x8AA9652
// IDA_NAME: _ZN18stTitleRewardBasisD2Ev
// DEMANGLED: stTitleRewardBasis::~stTitleRewardBasis()
stTitleRewardBasis::~stTitleRewardBasis() {
    if (reward_action != 0) {
        typedef void (*RewardActionDtor)(void*);
        void** vtable = *reinterpret_cast<void***>(reward_action);
        reinterpret_cast<RewardActionDtor>(vtable[3])(reward_action);
    }
    reward_action = 0;
}

// IDA_EA: 0x8AA9690
// IDA_NAME: _ZN20stRewardChangeStatusC2Ev
// DEMANGLED: stRewardChangeStatus::stRewardChangeStatus(void)
stRewardChangeStatus::stRewardChangeStatus() {
    reward_kind = 0;
    status_type = 77;
    value_type = 2;
    status_value = 0;
}

// IDA_EA: 0x8AA96CC
// IDA_NAME: _ZN19stRewardGiveEmotionC2Ev
// DEMANGLED: stRewardGiveEmotion::stRewardGiveEmotion(void)
stRewardGiveEmotion::stRewardGiveEmotion()
    : emotion_id(-1) {
    reward_kind = 1;
}

// IDA_EA: 0x8AA96F2
// IDA_NAME: _ZN14stRewardPostalC2Ev
// DEMANGLED: stRewardPostal::stRewardPostal(void)
stRewardPostal::stRewardPostal()
    : first_dword(0),
      second_dword(0) {
    reward_kind = 3;
}

// IDA_EA: 0x83367BC
// IDA_NAME: _ZN26stConditionEventRewardInfoD2Ev
// DEMANGLED: stConditionEventRewardInfo::~stConditionEventRewardInfo()
stConditionEventRewardInfo::~stConditionEventRewardInfo() {}

// IDA_EA: 0x891B682
// IDA_NAME: _ZN26stConditionEventRewardInfoC2Ev
// DEMANGLED: stConditionEventRewardInfo::stConditionEventRewardInfo(void)
stConditionEventRewardInfo::stConditionEventRewardInfo()
    : first_dword(-1),
      second_dword(0),
      first_flag(0),
      second_flag(0) {}

// IDA_EA: 0x837C8C6
// IDA_NAME: _ZN20stConditionEventInfoC2Ev
// DEMANGLED: stConditionEventInfo::stConditionEventInfo(void)
stConditionEventInfo::stConditionEventInfo() {}

// IDA_EA: 0x837C938
// IDA_NAME: _ZN20stConditionEventInfoD2Ev
// DEMANGLED: stConditionEventInfo::~stConditionEventInfo()
stConditionEventInfo::~stConditionEventInfo() {}

// IDA_EA: 0x836F2A2
// IDA_NAME: _ZN25stEventIdolBringUpParam_tC2Ev
// DEMANGLED: stEventIdolBringUpParam_t::stEventIdolBringUpParam_t(void)
stEventIdolBringUpParam_t::stEventIdolBringUpParam_t()
    : first_dword(0),
      second_dword(0),
      fatigue_items() {}

// IDA_EA: 0x836FA54
// IDA_NAME: _ZN25stEventIdolBringUpParam_tD2Ev
// DEMANGLED: stEventIdolBringUpParam_t::~stEventIdolBringUpParam_t()
stEventIdolBringUpParam_t::~stEventIdolBringUpParam_t() {}

// IDA_EA: 0x8118B56
// IDA_NAME: _ZN14stEVENTADDINFOD2Ev
// DEMANGLED: stEVENTADDINFO::~stEVENTADDINFO()
stEVENTADDINFO::~stEVENTADDINFO() {}

// IDA_EA: 0x893E3B6
// IDA_NAME: _ZN28stUnlimitChallengeRewardInfoD2Ev
// DEMANGLED: stUnlimitChallengeRewardInfo::~stUnlimitChallengeRewardInfo()
stUnlimitChallengeRewardInfo::~stUnlimitChallengeRewardInfo() {}

// IDA_EA: 0x88F90C0
// IDA_NAME: _ZN16stEventMonster_tD2Ev
// DEMANGLED: stEventMonster_t::~stEventMonster_t()
stEventMonster_t::~stEventMonster_t() {}

// IDA_EA: 0x88F8EBE
// IDA_NAME: _ZN22stSecondEventMonster_tD2Ev
// DEMANGLED: stSecondEventMonster_t::~stSecondEventMonster_t()
stSecondEventMonster_t::~stSecondEventMonster_t() {}

// IDA_EA: 0x8284750
// IDA_NAME: _ZN20TournamentRewardType5ClearEv
// DEMANGLED: TournamentRewardType::Clear(void)
TournamentRewardType* TournamentRewardType::Clear() {
    first_dword = -1;
    second_dword = 0;
    reward_card_type = -1;
    return this;
}

// IDA_EA: 0x8285546
// IDA_NAME: _ZN20TournamentRewardTypeC2Ev
// DEMANGLED: TournamentRewardType::TournamentRewardType(void)
TournamentRewardType::TournamentRewardType() {
    Clear();
}

// IDA_EA: 0x88F2DA0
// IDA_NAME: _ZN20TournamentRewardRateC2Ev
// DEMANGLED: TournamentRewardRate::TournamentRewardRate(void)
TournamentRewardRate::TournamentRewardRate() {
    gold_reward_rate = 0;
    item_reward_rate = 0;
    no_reward_rate = 0;
    total_rate = 0;
}

// IDA_EA: 0x88F2DCC
// IDA_NAME: _ZN20TournamentRewardItemC2Ev
// DEMANGLED: TournamentRewardItem::TournamentRewardItem(void)
TournamentRewardItem::TournamentRewardItem() {
    item_id = 0;
    selection_rate = 0;
    item_count = 0;
}

// IDA_EA: 0x88F9EF6
// IDA_NAME: _ZN20TournamentRewardItemD2Ev
// DEMANGLED: TournamentRewardItem::~TournamentRewardItem()
TournamentRewardItem::~TournamentRewardItem() {}

// IDA_EA: 0x836AB66
// IDA_NAME: _ZN24TournamentRewardItemRate5ClearEv
// DEMANGLED: TournamentRewardItemRate::Clear(void)
TournamentRewardItemRate* TournamentRewardItemRate::Clear() {
    reward_items.clear();
    total_rate = 0;
    return this;
}

// IDA_EA: 0x836AB84
// IDA_NAME: _ZN24TournamentRewardItemRateC2Ev
// DEMANGLED: TournamentRewardItemRate::TournamentRewardItemRate(void)
TournamentRewardItemRate::TournamentRewardItemRate() {
    Clear();
}

// IDA_EA: 0x836B5A6
// IDA_NAME: _ZN24TournamentRewardItemRateD2Ev
// DEMANGLED: TournamentRewardItemRate::~TournamentRewardItemRate()
TournamentRewardItemRate::~TournamentRewardItemRate() {}

// IDA_EA: 0x8376D28
// IDA_NAME: _ZN24TournamentRewardItemRateaSERKS_
// DEMANGLED: TournamentRewardItemRate::operator=(TournamentRewardItemRate const&)
TournamentRewardItemRate& TournamentRewardItemRate::operator=(
    const TournamentRewardItemRate& rhs) {
    reward_items = rhs.reward_items;
    total_rate = rhs.total_rate;
    return *this;
}

// IDA_EA: 0x891B1BE
// IDA_NAME: _ZN20STUltimateRewardProbC2Ev
// DEMANGLED: STUltimateRewardProb::STUltimateRewardProb(void)
STUltimateRewardProb::STUltimateRewardProb()
    : first_dword(0),
      second_dword(0) {}

// IDA_EA: 0x81B47A8
// IDA_NAME: _ZN25competition_reward_resultC2Ev
// DEMANGLED: competition_reward_result::competition_reward_result(void)
competition_reward_result::competition_reward_result()
    : first_dword(0),
      second_dword(0),
      success_flag(0) {}

// IDA_EA: 0x8A3D4CC
// IDA_NAME: _ZN12STEventSuper5clearEv
// DEMANGLED: STEventSuper::clear(void)
STEventSuper* STEventSuper::clear() {
    first_dword = -1;
    second_dword = -1;
    third_dword = -1;
    fourth_dword = 0;
    first_flag = 0;
    fifth_dword = -1;
    sixth_dword = -1;
    seventh_dword = -1;
    eighth_dword = -1;
    second_flag = 0;
    return this;
}

// IDA_EA: 0x8A3D52E
// IDA_NAME: _ZN12STEventSuperC2Ev
// DEMANGLED: STEventSuper::STEventSuper(void)
STEventSuper::STEventSuper() {
    clear();
}

// IDA_EA: 0x8A4D310
// IDA_NAME: _ZN12STEventSuperD2Ev
// DEMANGLED: STEventSuper::~STEventSuper()
STEventSuper::~STEventSuper() {}

// IDA_EA: 0x837069C
// IDA_NAME: _ZN11LotteryInfo5clearEv
// DEMANGLED: LotteryInfo::clear(void)
LotteryInfo* LotteryInfo::clear() {
    first_dword = 0;
    pair_values.clear();
    second_dword = 0;
    int_values.clear();
    return this;
}

// IDA_EA: 0x83706D4
// IDA_NAME: _ZN11LotteryInfoC2Ev
// DEMANGLED: LotteryInfo::LotteryInfo(void)
LotteryInfo::LotteryInfo() {
    clear();
}

// IDA_EA: 0x8374CFC
// IDA_NAME: _ZN11LotteryInfoD2Ev
// DEMANGLED: LotteryInfo::~LotteryInfo()
LotteryInfo::~LotteryInfo() {}

// IDA_EA: 0x8365066
// IDA_NAME: _ZN18CLottery_NeedMoney21add_lottery_needMoneyEjj
// DEMANGLED: CLottery_NeedMoney::add_lottery_needMoney(unsigned int,unsigned int)
void CLottery_NeedMoney::add_lottery_needMoney(
    unsigned int item_index,
    unsigned int need_money) {
    std::map<unsigned int, unsigned int>::iterator it =
        need_money_by_item.find(item_index);
    if (it != need_money_by_item.end()) {
        cMyTraceNoop trace;
        trace();
        return;
    }

    need_money_by_item.insert(std::make_pair(item_index, need_money));
}

// IDA_EA: 0x8365162
// IDA_NAME: _ZNK18CLottery_NeedMoney11isNeedMoneyEj
// DEMANGLED: CLottery_NeedMoney::isNeedMoney(unsigned int)const
int CLottery_NeedMoney::isNeedMoney(unsigned int item_index) const {
    std::map<unsigned int, unsigned int>::const_iterator it =
        need_money_by_item.find(item_index);
    if (it != need_money_by_item.end()) {
        return static_cast<int>(it->second);
    }
    return 0;
}

// IDA_EA: 0x837DD28
// IDA_NAME: _ZN18CLottery_NeedMoneyC2Ev
// DEMANGLED: CLottery_NeedMoney::CLottery_NeedMoney(void)
CLottery_NeedMoney::CLottery_NeedMoney()
    : need_money_by_item() {}

// IDA_EA: 0x837DD3C
// IDA_NAME: _ZN18CLottery_NeedMoneyD2Ev
// DEMANGLED: CLottery_NeedMoney::~CLottery_NeedMoney()
CLottery_NeedMoney::~CLottery_NeedMoney() {}

// IDA_EA: 0x83DD67C
// IDA_NAME: _ZN22stMercenaryRewardGroupD2Ev
// DEMANGLED: stMercenaryRewardGroup::~stMercenaryRewardGroup()
stMercenaryRewardGroup::~stMercenaryRewardGroup() {}

// IDA_EA: 0x89F7A9C
// IDA_NAME: _ZN22stMercenaryRewardGroupC2Ev
// DEMANGLED: stMercenaryRewardGroup::stMercenaryRewardGroup(void)
stMercenaryRewardGroup::stMercenaryRewardGroup()
    : first_dword(0),
      second_dword(0),
      third_dword(0) {}

// IDA_EA: 0x89F7F26
// IDA_NAME: _ZN22stMercenaryRewardGroupC2ERKS_
// DEMANGLED: stMercenaryRewardGroup::stMercenaryRewardGroup(stMercenaryRewardGroup const&)
stMercenaryRewardGroup::stMercenaryRewardGroup(
    const stMercenaryRewardGroup& rhs)
    : first_dword(rhs.first_dword),
      group_name(rhs.group_name),
      first_rewards(rhs.first_rewards),
      second_rewards(rhs.second_rewards),
      second_dword(rhs.second_dword),
      third_dword(rhs.third_dword) {}

// IDA_EA: 0x89F805A
// IDA_NAME: _ZN22stMercenaryRewardGroupaSERKS_
// DEMANGLED: stMercenaryRewardGroup::operator=(stMercenaryRewardGroup const&)
stMercenaryRewardGroup& stMercenaryRewardGroup::operator=(
    const stMercenaryRewardGroup& rhs) {
    first_dword = rhs.first_dword;
    group_name = rhs.group_name;
    first_rewards = rhs.first_rewards;
    second_rewards = rhs.second_rewards;
    second_dword = rhs.second_dword;
    third_dword = rhs.third_dword;
    return *this;
}

// IDA_EA: 0x8697128
// IDA_NAME: _ZN9BingoDataC2Ev
// DEMANGLED: BingoData::BingoData(void)
BingoData::BingoData()
    : bingo_count(0),
      line_count(0),
      selected_count(0),
      reward_state(0) {}

// IDA_EA: 0x80CBC6C
// IDA_NAME: _ZN9BingoData5clearEv
// DEMANGLED: BingoData::clear(void)
BingoData* BingoData::clear() {
    bingo_count = 0;
    line_count = 0;
    selected_count = 0;
    reward_state = 0;
    return this;
}

// IDA_EA: 0x86975FE
// IDA_NAME: _ZN17CLevelRewardTableC2Ev
// DEMANGLED: CLevelRewardTable::CLevelRewardTable(void)
CLevelRewardTable::CLevelRewardTable() {
    for (int level_index = 0;
         level_index < oldsrv::config::kLevelRewardTableLevelCapacity;
         ++level_index) {
        proper_levels[level_index] = 0;
        mail_title[level_index][0] = '\0';
        mail_string[level_index][0] = '\0';
    }

    for (int job_index = 0;
         job_index < oldsrv::config::kLevelRewardTableJobIndexCount;
         ++job_index) {
        for (int level_index = 0;
             level_index < oldsrv::config::kLevelRewardTableLevelCapacity;
             ++level_index) {
            reward_item_no[job_index][level_index] = 0;
            item_count[job_index][level_index] = 0;
        }
    }

    job_affection = 0;
}

// IDA_EA: 0x86876C8
// IDA_NAME: _ZN17CLevelRewardTable19GetProperLevelIndexEj
// DEMANGLED: CLevelRewardTable::GetProperLevelIndex(unsigned int)
int CLevelRewardTable::GetProperLevelIndex(unsigned int level) {
    for (int level_index = 0;
         level_index < oldsrv::config::kLevelRewardTableLevelCapacity &&
             proper_levels[level_index] != 0;
         ++level_index) {
        if (proper_levels[level_index] == level) {
            return level_index;
        }
    }
    return -1;
}

// IDA_EA: 0x8687E78
// IDA_NAME: _ZN17CLevelRewardTable17GetProperJobIndexEc
// DEMANGLED: CLevelRewardTable::GetProperJobIndex(char)
int CLevelRewardTable::GetProperJobIndex(char job) {
    if (job < 0) {
        return -1;
    }
    if (GetJobAffection() != 1) {
        return 0;
    }
    switch (job) {
        case oldsrv::config::kCharacterJobSwordman: return 1;
        case oldsrv::config::kCharacterJobFighter: return 2;
        case oldsrv::config::kCharacterJobGunner: return 3;
        case oldsrv::config::kCharacterJobMage: return 4;
        case oldsrv::config::kCharacterJobPriest: return 5;
        case oldsrv::config::kCharacterJobAtGunner: return 6;
        case oldsrv::config::kCharacterJobThief: return 7;
        case oldsrv::config::kCharacterJobAtFighter: return 8;
        default: return -1;
    }
}

// IDA_EA: 0x86976F6
// IDA_NAME: _ZN17CLevelRewardTable15GetRewardItemNoEii
// DEMANGLED: CLevelRewardTable::GetRewardItemNo(int,int)
int CLevelRewardTable::GetRewardItemNo(int job_index, int level_index) {
    return reward_item_no[job_index][level_index];
}

// IDA_EA: 0x8697712
// IDA_NAME: _ZN17CLevelRewardTable12GetItemCountEii
// DEMANGLED: CLevelRewardTable::GetItemCount(int,int)
int CLevelRewardTable::GetItemCount(int job_index, int level_index) {
    return item_count[job_index][level_index];
}

// IDA_EA: 0x8697730
// IDA_NAME: _ZN17CLevelRewardTable12GetMailTitleEi
// DEMANGLED: CLevelRewardTable::GetMailTitle(int)
char* CLevelRewardTable::GetMailTitle(int level_index) {
    return mail_title[level_index];
}

// IDA_EA: 0x8697750
// IDA_NAME: _ZN17CLevelRewardTable13GetMailStringEi
// DEMANGLED: CLevelRewardTable::GetMailString(int)
char* CLevelRewardTable::GetMailString(int level_index) {
    return mail_string[level_index];
}

// IDA_EA: 0x8697766
// IDA_NAME: _ZN17CLevelRewardTable15SetJobAffectionEb
// DEMANGLED: CLevelRewardTable::SetJobAffection(bool)
CLevelRewardTable* CLevelRewardTable::SetJobAffection(bool enabled) {
    job_affection = enabled;
    return this;
}

// IDA_EA: 0x8697782
// IDA_NAME: _ZN17CLevelRewardTable15GetJobAffectionEv
// DEMANGLED: CLevelRewardTable::GetJobAffection(void)
int CLevelRewardTable::GetJobAffection() {
    return job_affection;
}

CLevelRewardTableMgr* CLevelRewardTableMgr::levelRewardTableMgr = 0;

// IDA_EA: 0x837505B
// IDA_NAME: _ZN20CLevelRewardTableMgr11GetInstanceEv
// DEMANGLED: CLevelRewardTableMgr::GetInstance(void)
CLevelRewardTableMgr* CLevelRewardTableMgr::GetInstance() {
    if (levelRewardTableMgr == 0) {
        levelRewardTableMgr = new CLevelRewardTableMgr();
    }
    return levelRewardTableMgr;
}

// IDA_EA: 0x8687710
// IDA_NAME: _ZN20CLevelRewardTableMgrC2Ev
// DEMANGLED: CLevelRewardTableMgr::CLevelRewardTableMgr(void)
CLevelRewardTableMgr::CLevelRewardTableMgr()
    : on(0),
      proper_level_reward_table(new CLevelRewardTable()),
      server_ids() {}

// IDA_EA: 0x868777C
// IDA_NAME: _ZN20CLevelRewardTableMgrD2Ev
// DEMANGLED: CLevelRewardTableMgr::~CLevelRewardTableMgr()
CLevelRewardTableMgr::~CLevelRewardTableMgr() {}

// IDA_EA: 0x8687792
// IDA_NAME: _ZN20CLevelRewardTableMgr16IsChallengingJobEc
// DEMANGLED: CLevelRewardTableMgr::IsChallengingJob(char)
bool CLevelRewardTableMgr::IsChallengingJob(char job) {
    CLevelRewardTable* table = GetProperLevelRewardTableR();
    if (table == 0) {
        return false;
    }
    const int job_index = table->GetProperJobIndex(job);
    if (job_index < 0 ||
        job_index >= oldsrv::config::kLevelRewardTableJobIndexCount) {
        return false;
    }
    const int reward_item_no = table->GetRewardItemNo(job_index, 0);
    const int item_count = table->GetItemCount(job_index, 0);
    return reward_item_no != 0 || item_count != 0;
}

// IDA_EA: 0x8687828
// IDA_NAME: _ZN20CLevelRewardTableMgr16SetChallengeInfoER22stUnlimitChallengeInfo
// DEMANGLED: CLevelRewardTableMgr::SetChallengeInfo(stUnlimitChallengeInfo&)
bool CLevelRewardTableMgr::SetChallengeInfo(
    stUnlimitChallengeInfo& challenge_info) {
    on = challenge_info.enabled;
    server_ids = challenge_info.server_ids;
    proper_level_reward_table->SetJobAffection(true);

    const int level_count = static_cast<int>(challenge_info.levels.size());
    std::vector<int>::const_iterator level_it = challenge_info.levels.begin();
    const std::vector<int>::const_iterator level_end =
        challenge_info.levels.end();
    int level_index = 0;
    while (level_index < level_count && level_it != level_end) {
        proper_level_reward_table->proper_levels[level_index] = *level_it;
        ++level_index;
        ++level_it;
    }

    const int reward_count =
        static_cast<int>(challenge_info.server_rewards.size());
    std::vector<stUnlimitChallengeRewardInfo>::const_iterator reward_it =
        challenge_info.server_rewards.begin();
    const std::vector<stUnlimitChallengeRewardInfo>::const_iterator reward_end =
        challenge_info.server_rewards.end();
    int reward_index = 0;
    while (reward_index < reward_count && reward_it != reward_end) {
        CLevelRewardTable* const table = proper_level_reward_table;
        const int job_index = table->GetProperJobIndex(
            static_cast<char>(reward_it->job));
        const int level_index_for_item =
            proper_level_reward_table->GetProperLevelIndex(
                reward_it->level);
        table->reward_item_no[job_index][level_index_for_item] =
            reward_it->item_id;

        CLevelRewardTable* const table_for_count = proper_level_reward_table;
        const int job_index_for_count = table_for_count->GetProperJobIndex(
            static_cast<char>(reward_it->job));
        const int level_index_for_count =
            proper_level_reward_table->GetProperLevelIndex(
                reward_it->level);
        table_for_count->item_count[job_index_for_count][level_index_for_count] =
            reward_it->item_count;
        ++reward_index;
        ++reward_it;
    }

    const int message_count = static_cast<int>(challenge_info.messages.size());
    std::vector<stUnlimitChallengeMessageInfo>::const_iterator message_it =
        challenge_info.messages.begin();
    const std::vector<stUnlimitChallengeMessageInfo>::const_iterator message_end =
        challenge_info.messages.end();
    int message_index = 0;
    bool result;
    while (true) {
        result = message_index < message_count && message_it != message_end;
        if (!result) {
            break;
        }
        const unsigned int level = message_it->level;
        std::string server_message(message_it->server_message);
        std::string client_message(message_it->client_message);
        const char* const title = g_scriptStringManager_.findString(
            4, server_message.c_str(), 0);
        CLevelRewardTable* const table = proper_level_reward_table;
        const int level_index = table->GetProperLevelIndex(level);
        std::strncpy(table->mail_title[level_index],
                     title,
                     0x14u);
        const char* const message = g_scriptStringManager_.findString(
            4, client_message.c_str(), 0);
        CLevelRewardTable* const table_for_message = proper_level_reward_table;
        const int level_index_for_message =
            table_for_message->GetProperLevelIndex(level);
        std::strncpy(table_for_message->mail_string[level_index_for_message],
                     message,
                     0xffu);
        ++message_index;
        ++message_it;
    }
    return result;
}

// IDA_EA: 0x8687D18
// IDA_NAME: _ZN20CLevelRewardTableMgr22IsHaveStartLevelRewardEv
// DEMANGLED: CLevelRewardTableMgr::IsHaveStartLevelReward(void)
int CLevelRewardTableMgr::IsHaveStartLevelReward() {
    for (int level_index = 0;
         level_index < oldsrv::config::kLevelRewardTableLevelCapacity;
         ++level_index) {
        if (proper_level_reward_table->proper_levels[level_index] == 0) {
            return 0;
        }
        if (proper_level_reward_table->proper_levels[level_index] == 1) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8687D6C
// IDA_NAME: _ZN20CLevelRewardTableMgr10IsOnServerEi
// DEMANGLED: CLevelRewardTableMgr::IsOnServer(int)
int CLevelRewardTableMgr::IsOnServer(int server_id) {
    if (server_ids.size() == 1 || server_ids[0] == 0) {
        return 1;
    }
    if (server_ids.empty()) {
        return 0;
    }

    for (std::vector<int>::iterator it = server_ids.begin();
         it != server_ids.end();
         ++it) {
        if (*it == server_id) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8687E62
// IDA_NAME: _ZNK20CLevelRewardTableMgr26GetProperLevelRewardTableREv
// DEMANGLED: CLevelRewardTableMgr::GetProperLevelRewardTableR(void)const
CLevelRewardTable* CLevelRewardTableMgr::GetProperLevelRewardTableR() const {
    return proper_level_reward_table;
}

// IDA_EA: 0x8687E6E
// IDA_NAME: _ZNK20CLevelRewardTableMgr26GetProperLevelRewardTableREi
// DEMANGLED: CLevelRewardTableMgr::GetProperLevelRewardTableR(int)const
CLevelRewardTable* CLevelRewardTableMgr::GetProperLevelRewardTableR(
    int index) const {
    (void)index;
    return 0;
}

// IDA_EA: 0x8697792
// IDA_NAME: _ZN20CLevelRewardTableMgr4IsOnEv
// DEMANGLED: CLevelRewardTableMgr::IsOn(void)
int CLevelRewardTableMgr::IsOn() {
    return on;
}

// IDA_EA: 0x8986044
// IDA_NAME: _ZN15InGameEventInfoC2Ev
// DEMANGLED: InGameEventInfo::InGameEventInfo(void)
InGameEventInfo::InGameEventInfo() {}

// IDA_EA: 0x8986E7C
// IDA_NAME: _ZN15InGameEventInfoC2ERKS_
// DEMANGLED: InGameEventInfo::InGameEventInfo(InGameEventInfo const&)
InGameEventInfo::InGameEventInfo(const InGameEventInfo& rhs)
    : first_dword(rhs.first_dword),
      second_dword(rhs.second_dword),
      event_name(rhs.event_name),
      third_dword(rhs.third_dword),
      fourth_dword(rhs.fourth_dword),
      fifth_dword(rhs.fifth_dword) {}

// IDA_EA: 0x811407C
// IDA_NAME: _ZN15InGameEventInfoD2Ev
// DEMANGLED: InGameEventInfo::~InGameEventInfo()
InGameEventInfo::~InGameEventInfo() {}

// IDA_EA: 0x8985806
// IDA_NAME: _ZN15InGameEventInfoaSERKS_
// DEMANGLED: InGameEventInfo::operator=(InGameEventInfo const&)
InGameEventInfo& InGameEventInfo::operator=(const InGameEventInfo& rhs) {
    first_dword = rhs.first_dword;
    second_dword = rhs.second_dword;
    event_name = rhs.event_name;
    third_dword = rhs.third_dword;
    fourth_dword = rhs.fourth_dword;
    fifth_dword = rhs.fifth_dword;
    return *this;
}

// IDA_EA: 0x8110F10
// IDA_NAME: _ZN19InGameEventInfoListC2Ev
// DEMANGLED: InGameEventInfoList::InGameEventInfoList(void)
InGameEventInfoList::InGameEventInfoList() {}

// IDA_EA: 0x8110F24
// IDA_NAME: _ZN19InGameEventInfoListD2Ev
// DEMANGLED: InGameEventInfoList::~InGameEventInfoList()
InGameEventInfoList::~InGameEventInfoList() {}

// IDA_EA: 0x897EB30
// IDA_NAME: _ZN19InGameEventInfoList15insertEventInfoEiR15InGameEventInfo
// DEMANGLED: InGameEventInfoList::insertEventInfo(int,InGameEventInfo &)
InGameEventInfo* InGameEventInfoList::insertEventInfo(
    int event_id,
    InGameEventInfo& event_info) {
    InGameEventInfo& stored = events[event_id];
    stored = event_info;
    return &stored;
}

// IDA_EA: 0x8110E96
// IDA_NAME: _ZN17InGameEventScriptD2Ev
// DEMANGLED: InGameEventScript::~InGameEventScript()
InGameEventScript::~InGameEventScript() {}

// IDA_EA: 0x8985864
// IDA_NAME: _ZN17InGameEventScriptC2Ev
// DEMANGLED: InGameEventScript::InGameEventScript(void)
InGameEventScript::InGameEventScript() {
    event_name.clear();
    script_file.clear();
}

// IDA_EA: 0x898597A
// IDA_NAME: _ZN16AttendanceScriptC2Ev
// DEMANGLED: AttendanceScript::AttendanceScript(void)
AttendanceScript::AttendanceScript()
    : InGameEventScript(),
      db_table_name(),
      fatigue_condition(0),
      reward_items_by_day(),
      final_reward_item(),
      final_day_condition(0),
      mail_title(),
      mail_data(),
      attend_mail_text(),
      final_mail_text() {
    db_table_name.clear();
    reward_items_by_day.clear();
    mail_data.reset();
}

// IDA_EA: 0x8985AE0
// IDA_NAME: _ZN30ExchangeRandomItemRewardScript5resetEv
// DEMANGLED: ExchangeRandomItemRewardScript::reset(void)
void ExchangeRandomItemRewardScript::reset() {
    enabled = 0;
    exchange_items.clear();
    reward_items.clear();
    send_mail.reset();
}

// IDA_EA: 0x8985B1A
// IDA_NAME: _ZN30ExchangeRandomItemRewardScriptC2Ev
// DEMANGLED: ExchangeRandomItemRewardScript::ExchangeRandomItemRewardScript(void)
ExchangeRandomItemRewardScript::ExchangeRandomItemRewardScript() {
    reset();
}

// IDA_EA: 0x8147B2A
// IDA_NAME: _ZN25LevelupSupportEventManger9EventInfoC2Ett
// DEMANGLED: LevelupSupportEventManger::EventInfo::EventInfo(unsigned short,unsigned short)
LevelupSupportEventManger::EventInfo::EventInfo(
    uint16_t level_value,
    uint16_t state_value)
    : level(level_value),
      state(state_value) {}

// IDA_EA: 0x8148C54
// IDA_NAME: _ZN25LevelupSupportEventManger9EventInfoD2Ev
// DEMANGLED: LevelupSupportEventManger::EventInfo::~EventInfo()
LevelupSupportEventManger::EventInfo::~EventInfo() {}

// IDA_EA: 0x81169F8
// IDA_NAME: _ZN19LevelupSupportEventC2Ev
// DEMANGLED: LevelupSupportEvent::LevelupSupportEvent(void)
LevelupSupportEvent::LevelupSupportEvent()
    : CEventBase() {}

// IDA_EA: 0x8116A14
// IDA_NAME: _ZN19LevelupSupportEventD2Ev
// DEMANGLED: LevelupSupportEvent::~LevelupSupportEvent()
// IDA_EA: 0x8116A44
// IDA_NAME: _ZN19LevelupSupportEventD0Ev
// DEMANGLED: LevelupSupportEvent::~LevelupSupportEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
LevelupSupportEvent::~LevelupSupportEvent() {}

// IDA_EA: 0x8116A62
// IDA_NAME: _ZN19LevelupSupportEvent10StartEventEv
// DEMANGLED: LevelupSupportEvent::StartEvent(void)
void LevelupSupportEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 63, 0);
    trace("LevelupSupportEvent Event Start!");
}

// IDA_EA: 0x8116AB4
// IDA_NAME: _ZN19LevelupSupportEvent10StartEventE10Word_Param
// DEMANGLED: LevelupSupportEvent::StartEvent(Word_Param)
void LevelupSupportEvent::StartEvent(Word_Param param) {
    (void)param;
    StartEvent();
}

// IDA_EA: 0x8116ACE
// IDA_NAME: _ZN19LevelupSupportEvent8EndEventEv
// DEMANGLED: LevelupSupportEvent::EndEvent(void)
void LevelupSupportEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("EndEvent", 73, 0);
    trace("LevelupSupportEvent Event End!");
}

// IDA_EA: 0x811B988
// IDA_NAME: _ZN22LevelupSupport2ndEventC2Ev
// DEMANGLED: LevelupSupport2ndEvent::LevelupSupport2ndEvent(void)
LevelupSupport2ndEvent::LevelupSupport2ndEvent()
    : CEventBase() {}

// IDA_EA: 0x811B9A4
// IDA_NAME: _ZN22LevelupSupport2ndEventD2Ev
// DEMANGLED: LevelupSupport2ndEvent::~LevelupSupport2ndEvent()
// IDA_EA: 0x811B9D4
// IDA_NAME: _ZN22LevelupSupport2ndEventD0Ev
// DEMANGLED: LevelupSupport2ndEvent::~LevelupSupport2ndEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
LevelupSupport2ndEvent::~LevelupSupport2ndEvent() {}

// IDA_EA: 0x811B9F2
// IDA_NAME: _ZN22LevelupSupport2ndEvent10StartEventEv
// DEMANGLED: LevelupSupport2ndEvent::StartEvent(void)
void LevelupSupport2ndEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 107, 0);
    trace("LevelupSupport2ndEvent Event Start!");
}

// IDA_EA: 0x811BA44
// IDA_NAME: _ZN22LevelupSupport2ndEvent10StartEventE10Word_Param
// DEMANGLED: LevelupSupport2ndEvent::StartEvent(Word_Param)
void LevelupSupport2ndEvent::StartEvent(Word_Param param) {
    (void)param;
    StartEvent();
}

// IDA_EA: 0x811BA5E
// IDA_NAME: _ZN22LevelupSupport2ndEvent8EndEventEv
// DEMANGLED: LevelupSupport2ndEvent::EndEvent(void)
void LevelupSupport2ndEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("EndEvent", 117, 0);
    trace("LevelupSupport2ndEvent Event End!");
}

// IDA_EA: 0x8AC063E
// IDA_NAME: _ZN20CreateDnfEventSciprt8ItemInfoC2Eii
// DEMANGLED: CreateDnfEventSciprt::ItemInfo::ItemInfo(int,int)
CreateDnfEventSciprt::ItemInfo::ItemInfo(
    int32_t item_index_value,
    int item_count_value)
    : item_index(item_index_value),
      item_count(static_cast<int16_t>(item_count_value)) {}

// IDA_EA: 0x8AC1884
// IDA_NAME: _ZN20CreateDnfEventSciprt8ItemInfoD2Ev
// DEMANGLED: CreateDnfEventSciprt::ItemInfo::~ItemInfo()
CreateDnfEventSciprt::ItemInfo::~ItemInfo() {}

// IDA_EA: 0x8AC000C
// IDA_NAME: _ZN20CreateDnfEventSciprtC2Ev
// DEMANGLED: CreateDnfEventSciprt::CreateDnfEventSciprt(void)
CreateDnfEventSciprt::CreateDnfEventSciprt()
    : reward_items_by_day() {}

// IDA_EA: 0x8AC0020
// IDA_NAME: _ZN20CreateDnfEventSciprtD2Ev
// DEMANGLED: CreateDnfEventSciprt::~CreateDnfEventSciprt()
CreateDnfEventSciprt::~CreateDnfEventSciprt() {}

// IDA_EA: 0x8AC0034
// IDA_NAME: _ZN20CreateDnfEventSciprt10loadScriptEv
// DEMANGLED: CreateDnfEventSciprt::loadScript(void)
int CreateDnfEventSciprt::loadScript() {
    if (!loadRDARScriptFile("Event/", "tw_CreateDnf.evt")) {
        return -1;
    }

    std::string token;
    bool is_type = false;
    bool success = false;
    int day = 0;
    std::vector<ItemInfo> item_info;

    while (ScanType(token, true) == 1) {
        if (token == "[step]") {
            if (ScanType(token, true) != 1) {
                break;
            }
            if (token == "[day]") {
                day = ScanInt(static_cast<bool*>(0));
            }
            if (ScanType(token, true) != 1) {
                break;
            }
            if (token == "[reward]") {
                while (1) {
                    const int item_index =
                        ScanTypeOrInt(is_type, token, success, true);
                    if (is_type) {
                        break;
                    }
                    const int item_count = ScanInt(static_cast<bool*>(0));
                    ItemInfo info(item_index, static_cast<int16_t>(item_count));
                    item_info.push_back(info);
                }
            }
            if (ScanType(token, true) != 1) {
                break;
            }
            if (token == "[/step]") {
                if (reward_items_by_day.find(day) !=
                    reward_items_by_day.end()) {
                    std::printf("duplicate [level] %d", day);
                    return -1;
                }
                std::vector<ItemInfo>* stored =
                    new std::vector<ItemInfo>();
                stored->swap(item_info);
                reward_items_by_day.insert(std::make_pair(day, stored));
            }
        }
    }

    unloadRDARScriptFile();
    return 0;
}

// IDA_EA: 0x8AC032A
// IDA_NAME: _ZN20CreateDnfEventSciprt11clearScriptEv
// DEMANGLED: CreateDnfEventSciprt::clearScript(void)
int CreateDnfEventSciprt::clearScript() {
    std::map<int, std::vector<ItemInfo>* >::iterator it =
        reward_items_by_day.begin();
    std::map<int, std::vector<ItemInfo>* >::iterator end =
        reward_items_by_day.end();

    // IDA disasm 0x8AC032A has no iterator increment before the loop test.
    while (it != end) {
        std::vector<ItemInfo>* items = it->second;
        if (items != 0) {
            items->~vector();
            operator delete(items);
        }
    }
    reward_items_by_day.clear();
    return static_cast<int>(
        reinterpret_cast<intptr_t>(&reward_items_by_day));
}

// IDA_EA: 0x8AC03A6
// IDA_NAME: _ZN20CreateDnfEventSciprt11GetItemInfoERSt6vectorISt4pairIiiESaIS2_EEi
// DEMANGLED: CreateDnfEventSciprt::GetItemInfo(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &,int)
char CreateDnfEventSciprt::GetItemInfo(
    std::vector<std::pair<int, int> >& out,
    int day) {
    std::map<int, std::vector<ItemInfo>* >::iterator it =
        reward_items_by_day.find(day);
    std::map<int, std::vector<ItemInfo>* >::iterator end =
        reward_items_by_day.end();
    if (it == end) {
        return 0;
    }

    out.clear();
    std::vector<ItemInfo>* items = it->second;
    unsigned int index = 0;
    unsigned int count = items->size();
    while (index < count) {
        ItemInfo& item = items->at(index);
        out.push_back(
            std::pair<int, int>(item.item_index, item.item_count));
        ++index;
    }
    return 0;
}

// IDA_EA: 0x8AC04A4
// IDA_NAME: _ZN20CreateDnfEventSciprt11GetItemInfoERSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: CreateDnfEventSciprt::GetItemInfo(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &)
int CreateDnfEventSciprt::GetItemInfo(
    std::vector<std::pair<int, int> >& out) {
    std::map<int, std::vector<ItemInfo>* >::iterator end =
        reward_items_by_day.end();
    out.clear();
    std::map<int, std::vector<ItemInfo>* >::iterator it =
        reward_items_by_day.begin();

    while (it != end) {
        unsigned int index = 0;
        unsigned int count = it->second->size();
        while (index < count) {
            std::vector<ItemInfo>* items = it->second;
            ItemInfo& item = items->at(index);
            out.push_back(
                std::pair<int, int>(item.item_index, item.item_count));
            ++index;
        }
        it++;
    }
    return 0;
}

// IDA_EA: 0x816451E
// IDA_NAME: _ZN26CreateDnfEventSciprtServer4LoadEv
// DEMANGLED: CreateDnfEventSciprtServer::Load(void)
bool CreateDnfEventSciprtServer::Load() {
    cMyTrace trace("virtual bool CreateDnfEventSciprtServer::Load()",
                   208,
                   9,
                   true,
                   false);
    trace("\t- Loading EventCreateDnf script - ");
    return script.loadScript() == 0;
}

// IDA_EA: 0x8164590
// IDA_NAME: _ZN26CreateDnfEventSciprtServer7DestroyEv
// DEMANGLED: CreateDnfEventSciprtServer::Destroy(void)
int CreateDnfEventSciprtServer::Destroy() {
    return 1;
}

// IDA_EA: 0x81648AC
// IDA_NAME: _ZN26CreateDnfEventSciprtServerC2Ev
// DEMANGLED: CreateDnfEventSciprtServer::CreateDnfEventSciprtServer(void)
CreateDnfEventSciprtServer::CreateDnfEventSciprtServer()
    : script() {
    ARAD::Singleton<CreateDnfEventSciprtServer>();
}

// IDA_EA: 0x81648E2
// IDA_NAME: _ZN26CreateDnfEventSciprtServerD2Ev
// DEMANGLED: CreateDnfEventSciprtServer::~CreateDnfEventSciprtServer()
CreateDnfEventSciprtServer::~CreateDnfEventSciprtServer() {}

// IDA_EA: 0x8164902
// IDA_NAME: _ZN4ARAD9SingletonI26CreateDnfEventSciprtServerEC2Ev
// DEMANGLED: ARAD::Singleton<CreateDnfEventSciprtServer>::Singleton(void)
ARAD::Singleton<CreateDnfEventSciprtServer>::Singleton() {}

// IDA_EA: 0x8164907
// IDA_NAME: _ZN4ARAD9SingletonI26CreateDnfEventSciprtServerE3GetEv
// DEMANGLED: ARAD::Singleton<CreateDnfEventSciprtServer>::Get(void)
CreateDnfEventSciprtServer*
ARAD::Singleton<CreateDnfEventSciprtServer>::Get() {
    static CreateDnfEventSciprtServer obj;
    return &obj;
}

namespace StackablePackage {

// IDA_EA: 0x8281BFA
// IDA_NAME: _ZNK16StackablePackage14EventProcessor12GetEventTypeEv
// DEMANGLED: StackablePackage::EventProcessor::GetEventType(void)const
int EventProcessor::GetEventType() const {
    return event_type;
}

// IDA_EA: 0x8281C06
// IDA_NAME: _ZNK16StackablePackage14EventProcessor10GetEventIdEv
// DEMANGLED: StackablePackage::EventProcessor::GetEventId(void)const
int EventProcessor::GetEventId() const {
    return event_id;
}

// IDA_EA: 0x8281C3A
// IDA_NAME: _ZN16StackablePackage30FindControllableEventProcessorC2Ei
// DEMANGLED: StackablePackage::FindControllableEventProcessor::FindControllableEventProcessor(int)
FindControllableEventProcessor::FindControllableEventProcessor(
    int item_id_value)
    : item_id(item_id_value) {}

// IDA_EA: 0x8281C48
// IDA_NAME: _ZNK16StackablePackage30FindControllableEventProcessorclERKSt4pairIKiPNS_14EventProcessorEE
// DEMANGLED: StackablePackage::FindControllableEventProcessor::operator()(std::pair<int const,StackablePackage::EventProcessor *> const&)const
bool FindControllableEventProcessor::operator()(
    const std::pair<const int, EventProcessor*>& entry) const {
    EventProcessor* processor = entry.second;
    if (processor == 0) {
        return false;
    }

    typedef bool (*CanProcessSlot)(EventProcessor*, int);
    void** vtable = *reinterpret_cast<void***>(processor);
    CanProcessSlot can_process = reinterpret_cast<CanProcessSlot>(vtable[2]);
    return can_process(processor, item_id);
}

}  // namespace StackablePackage

// C11 migration marker.  The single real OnePlus-card RDAR parser body is
// now src/datamanager/arad_one_plus_card_event_item_list.cpp (B owner).

// IDA_EA: 0x863C002
// IDA_NAME: _ZN14CEventStayTime17RewardGoGoFighterC2Eil
// DEMANGLED: CEventStayTime::RewardGoGoFighter::RewardGoGoFighter(int,long)
CEventStayTime::RewardGoGoFighter::RewardGoGoFighter(
    int reward_stay_time_value,
    long check_time_value)
    : reward_stay_time(reward_stay_time_value),
      check_time(check_time_value) {}

// IDA_EA: 0x863C018
// IDA_NAME: _ZN14CEventStayTime17RewardGoGoFighterclEP5CUser
// DEMANGLED: CEventStayTime::RewardGoGoFighter::operator()(CUser *)
void CEventStayTime::RewardGoGoFighter::operator()(CUser* user) {
    if (!user) {
        return;
    }

    const int elapsed = static_cast<int>(check_time) - user->GetLoginTime();
    if (reward_stay_time > elapsed) {
        return;
    }

    Stream* const stream = GlobalData::s_stream_pool->Acquire(
        "localtaiwan/Event/EventStayTimeCharacter.h", 30);
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, TaiwanInternalPack::CHECK_STAY_TIME_EVENT,
                            user->GetUID());
    Taiwan::SigStayTimeEvent* const payload =
        guard.GetInBuffer<Taiwan::SigStayTimeEvent>();
    if (!payload) {
        return;
    }

    const unsigned int account_id =
        static_cast<unsigned int>(user->get_acc_id());
    cMyTrace trace(
        "void CEventStayTime::RewardGoGoFighter::operator()(CUser*)", 36,
        0);
    trace("[Taiwan, Event] GoGoFighter try reward mid. (%u)", account_id);
    payload->init();
    payload->set(account_id);
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x816BDA0
// IDA_NAME: _ZN14CEventStayTime10StartEventEv
// DEMANGLED: CEventStayTime::StartEvent(void)
void CEventStayTime::StartEvent() {
    SetEventFlag(true);
    const int stay_time = reward_stay_time_;
    cMyTrace trace("virtual void CEventStayTime::StartEvent()", 10, 0);
    trace("[Taiwan, StayEvent] Start! StayTime:%dMin, CheckTime:%dMin",
          stay_time);
    RegistNextTimer();
}

// IDA_EA: 0x816BE18
// IDA_NAME: _ZN14CEventStayTime10StartEventE10Word_Param
// DEMANGLED: CEventStayTime::StartEvent(Word_Param)
void CEventStayTime::StartEvent(Word_Param param) {
    reward_stay_time_ = 60u * static_cast<uint16_t>(param);
    interval_stay_time_ = 60u * static_cast<uint16_t>(param >> 16);
    StartEvent();
}

// IDA_EA: 0x816BE60
// IDA_NAME: _ZN14CEventStayTime8EndEventEv
// DEMANGLED: CEventStayTime::EndEvent(void)
void CEventStayTime::EndEvent() {
    LogManager::logFormat(1,
                          "localtaiwan/Event/EventStayTimeCharacter.cpp",
                          "virtual void CEventStayTime::EndEvent()",
                          27,
                          "[CEventStayTime] End Event.");
    reward_stay_time_ = 0;
    interval_stay_time_ = 0;
    SetEventFlag(false);
}

// IDA_EA: 0x816BEBC
// IDA_NAME: _ZN14CEventStayTime17getRewardStayTimeEv
// DEMANGLED: CEventStayTime::getRewardStayTime(void)
int CEventStayTime::getRewardStayTime() {
    return reward_stay_time_;
}

// IDA_EA: 0x816BEC8
// IDA_NAME: _ZN14CEventStayTime19getIntervalStayTimeEv
// DEMANGLED: CEventStayTime::getIntervalStayTime(void)
int CEventStayTime::getIntervalStayTime() {
    return interval_stay_time_;
}

// IDA_EA: 0x816BED4
// IDA_NAME: _ZN14CEventStayTime12IsFirstResetEv
// DEMANGLED: CEventStayTime::IsFirstReset(void)
int CEventStayTime::IsFirstReset() {
    if (!reserved_09_[0]) {
        return reserved_09_[0];
    }
    reserved_09_[0] = 0;
    return 1;
}

// IDA_EA: 0x816BEFA
// IDA_NAME: _ZN14CEventStayTime15RegistNextTimerEv
// DEMANGLED: CEventStayTime::RegistNextTimer(void)
int CEventStayTime::RegistNextTimer() {
    if (IsEventing(0) != 1) {
        return 1;
    }
    return G_TimerQueue()->InsertTimer(2,
                                       0,
                                       173,
                                       interval_stay_time_,
                                       0,
                                       0);
}

// IDA_EA: 0x816BF6A
// IDA_NAME: _ZN14CEventStayTime14dailyresetDataEv
// DEMANGLED: CEventStayTime::dailyresetData(void)
void CEventStayTime::dailyresetData() {
    if (IsEventing(0) == 1 && !IsFirstReset()) {
        LogManager::logFormat(
            1,
            "localtaiwan/Event/EventStayTimeCharacter.cpp",
            "virtual void CEventStayTime::dailyresetData()",
            74,
            "[CEventStayTime] reset stay time request requires Taiwan stream/msgq ABI");
    }
}

// IDA_EA: 0x816C066
// IDA_NAME: _ZN14CEventStayTimeC2Ev
// DEMANGLED: CEventStayTime::CEventStayTime(void)
CEventStayTime::CEventStayTime()
    : CEventBase(),
      reward_stay_time_(0),
      interval_stay_time_(0) {
    reserved_09_[0] = 1;
}

// IDA_EA: 0x816C08A
// IDA_NAME: _ZN14CEventStayTimeD2Ev
// DEMANGLED: CEventStayTime::~CEventStayTime()
// IDA_EA: 0x816C0BA
// IDA_NAME: _ZN14CEventStayTimeD0Ev
// DEMANGLED: CEventStayTime::~CEventStayTime() deleting destructor
CEventStayTime::~CEventStayTime() {}

// IDA_EA: 0x8285C44
// IDA_NAME: _ZN20CUnlimitFatigueEventC2Ev
// DEMANGLED: CUnlimitFatigueEvent::CUnlimitFatigueEvent(void)
CUnlimitFatigueEvent::CUnlimitFatigueEvent()
    : CEventBase() {}

// IDA_EA: 0x8285C60
// IDA_NAME: _ZN20CUnlimitFatigueEventD2Ev
// DEMANGLED: CUnlimitFatigueEvent::~CUnlimitFatigueEvent()
// IDA_EA: 0x8285C90
// IDA_NAME: _ZN20CUnlimitFatigueEventD0Ev
// DEMANGLED: CUnlimitFatigueEvent::~CUnlimitFatigueEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CUnlimitFatigueEvent::~CUnlimitFatigueEvent() {}

// IDA_EA: 0x8285CAE
// IDA_NAME: _ZN20CUnlimitFatigueEvent10StartEventEv
// DEMANGLED: CUnlimitFatigueEvent::StartEvent(void)
void CUnlimitFatigueEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 18, 0);
    trace("Unlimit Fatigue Event Start!");
}

// IDA_EA: 0x8285D00
// IDA_NAME: _ZN20CUnlimitFatigueEvent10StartEventE10Word_Param
// DEMANGLED: CUnlimitFatigueEvent::StartEvent(Word_Param)
void CUnlimitFatigueEvent::StartEvent(Word_Param param) {
    (void)param;
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 25, 0);
    trace("Unlimit Fatigue Event Start!");
}

// IDA_EA: 0x8285D52
// IDA_NAME: _ZN20CUnlimitFatigueEvent8EndEventEv
// DEMANGLED: CUnlimitFatigueEvent::EndEvent(void)
void CUnlimitFatigueEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("EndEvent", 32, 0);
    trace("Unlimit Fatigue Event End!");
}

// IDA_EA: 0x811C30A
// IDA_NAME: _ZN20CDoubleGoldCardEventC2Ev
// DEMANGLED: CDoubleGoldCardEvent::CDoubleGoldCardEvent(void)
CDoubleGoldCardEvent::CDoubleGoldCardEvent()
    : CEventBase() {}

// IDA_EA: 0x811C326
// IDA_NAME: _ZN20CDoubleGoldCardEventD2Ev
// DEMANGLED: CDoubleGoldCardEvent::~CDoubleGoldCardEvent()
// IDA_EA: 0x811C356
// IDA_NAME: _ZN20CDoubleGoldCardEventD0Ev
// DEMANGLED: CDoubleGoldCardEvent::~CDoubleGoldCardEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CDoubleGoldCardEvent::~CDoubleGoldCardEvent() {}

// IDA_EA: 0x811C374
// IDA_NAME: _ZN20CDoubleGoldCardEvent10StartEventEv
// DEMANGLED: CDoubleGoldCardEvent::StartEvent(void)
void CDoubleGoldCardEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("virtual void CDoubleGoldCardEvent::StartEvent()", 16, 0);
    trace("CDoubleGoldCardEvent Event Start!");
}

// IDA_EA: 0x811C3C6
// IDA_NAME: _ZN20CDoubleGoldCardEvent10StartEventE10Word_Param
// DEMANGLED: CDoubleGoldCardEvent::StartEvent(Word_Param)
void CDoubleGoldCardEvent::StartEvent(Word_Param param) {
    (void)param;
    StartEvent();
}

// IDA_EA: 0x811C3E0
// IDA_NAME: _ZN20CDoubleGoldCardEvent8EndEventEv
// DEMANGLED: CDoubleGoldCardEvent::EndEvent(void)
void CDoubleGoldCardEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void CDoubleGoldCardEvent::EndEvent()", 26, 0);
    trace("CDoubleGoldCardEvent Event End!");
}

// IDA_EA: 0x80EEA84
// IDA_NAME: _ZN25CDimensionActivationEventC2Ev
// DEMANGLED: CDimensionActivationEvent::CDimensionActivationEvent(void)
CDimensionActivationEvent::CDimensionActivationEvent()
    : CEventBase() {}

// IDA_EA: 0x80EEAA0
// IDA_NAME: _ZN25CDimensionActivationEventD2Ev
// DEMANGLED: CDimensionActivationEvent::~CDimensionActivationEvent()
// IDA_EA: 0x80EEAD0
// IDA_NAME: _ZN25CDimensionActivationEventD0Ev
// DEMANGLED: CDimensionActivationEvent::~CDimensionActivationEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CDimensionActivationEvent::~CDimensionActivationEvent() {}

// IDA_EA: 0x80EEAEE
// IDA_NAME: _ZN25CDimensionActivationEvent10StartEventEv
// DEMANGLED: CDimensionActivationEvent::StartEvent(void)
void CDimensionActivationEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("virtual void CDimensionActivationEvent::StartEvent()",
                   18,
                   0);
    trace("CDimensionActivationEvent Event Start!");
    set_dungeon_play_count();
}

// IDA_EA: 0x80EEB4A
// IDA_NAME: _ZN25CDimensionActivationEvent10StartEventE10Word_Param
// DEMANGLED: CDimensionActivationEvent::StartEvent(Word_Param)
void CDimensionActivationEvent::StartEvent(Word_Param param) {
    (void)param;
    StartEvent();
}

// IDA_EA: 0x80EEB64
// IDA_NAME: _ZN25CDimensionActivationEvent8EndEventEv
// DEMANGLED: CDimensionActivationEvent::EndEvent(void)
void CDimensionActivationEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void CDimensionActivationEvent::EndEvent()",
                   30,
                   0);
    trace("CDimensionActivationEvent Event End!");
    reset_dungeon_play_count();
}

// IDA_EA: 0x80EEBC0
// IDA_NAME: _ZN25CDimensionActivationEvent22set_dungeon_play_countEv
// DEMANGLED: CDimensionActivationEvent::set_dungeon_play_count(void)
char CDimensionActivationEvent::set_dungeon_play_count() {
    CDataManager* data_manager = G_CDataManager();
    DimensionActivationData* activation_data =
        data_manager->get_dimension_activation_data();
    const char day_play_count =
        static_cast<char>(activation_data->active_flag);

    cMyTrace trace("void CDimensionActivationEvent::set_dungeon_play_count()",
                   39,
                   0);
    char result = static_cast<char>(
        trace("CDimensionActivationEvent day_play_count : %d",
              day_play_count));

    if (activation_data->active_flag > 0) {
        for (int dimension = 0;; ++dimension) {
            result = static_cast<char>(dimension <= 5);
            if (dimension > 5) {
                break;
            }
            data_manager->set_dimensionInout(dimension, day_play_count);
        }
    }
    return result;
}

// IDA_EA: 0x80EEC58
// IDA_NAME: _ZN25CDimensionActivationEvent24reset_dungeon_play_countEv
// DEMANGLED: CDimensionActivationEvent::reset_dungeon_play_count(void)
int CDimensionActivationEvent::reset_dungeon_play_count() {
    G_CDataManager()->reset_dimensionInout();
    cMyTrace trace(
        "void CDimensionActivationEvent::reset_dungeon_play_count()",
        52,
        0);
    return trace("CDimensionActivationEvent reset_dungeon_play_count");
}

// IDA_EA: 0x80EECA4
// IDA_NAME: _ZN25CDimensionActivationEvent25checkFreeRevivalConditionEi
// DEMANGLED: CDimensionActivationEvent::checkFreeRevivalCondition(int)
bool CDimensionActivationEvent::checkFreeRevivalCondition(int dungeon_index) {
    DimensionActivationData* activation_data =
        G_CDataManager()->get_dimension_activation_data();
    std::map<int, DimensionCoinInfo>::iterator it =
        activation_data->coin_infos.find(dungeon_index);
    return it != activation_data->coin_infos.end() &&
           it->second.by_character != 1;
}

// IDA_EA: 0x8116B2C
// IDA_NAME: _ZN22CEventAdvanceAltarOpenC2Ev
// DEMANGLED: CEventAdvanceAltarOpen::CEventAdvanceAltarOpen(void)
CEventAdvanceAltarOpen::CEventAdvanceAltarOpen()
    : CEventBase() {}

// IDA_EA: 0x80E95E2
// IDA_NAME: _ZN22CEventAdvanceAltarOpenD2Ev
// DEMANGLED: CEventAdvanceAltarOpen::~CEventAdvanceAltarOpen()
// IDA_EA: 0x80E9612
// IDA_NAME: _ZN22CEventAdvanceAltarOpenD0Ev
// DEMANGLED: CEventAdvanceAltarOpen::~CEventAdvanceAltarOpen() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CEventAdvanceAltarOpen::~CEventAdvanceAltarOpen() {}

// IDA_EA: 0x80E94DC
// IDA_NAME: _ZN22CEventAdvanceAltarOpen10StartEventEv
// DEMANGLED: CEventAdvanceAltarOpen::StartEvent(void)
void CEventAdvanceAltarOpen::StartEvent() {}

// IDA_EA: 0x80E94E2
// IDA_NAME: _ZN22CEventAdvanceAltarOpen10StartEventE10Word_Param
// DEMANGLED: CEventAdvanceAltarOpen::StartEvent(Word_Param)
void CEventAdvanceAltarOpen::StartEvent(Word_Param param) {
    (void)param;
    SetEventFlag(true);
    cMyTrace trace(
        "virtual void CEventAdvanceAltarOpen::StartEvent(Word_Param)",
        1166,
        0);
    trace("CEventAdvanceAltarOpen Event Start! ");
}

// IDA_EA: 0x80E9534
// IDA_NAME: _ZN22CEventAdvanceAltarOpen8EndEventEv
// DEMANGLED: CEventAdvanceAltarOpen::EndEvent(void)
void CEventAdvanceAltarOpen::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void CEventAdvanceAltarOpen::EndEvent()",
                   1172,
                   0);
    trace("CEventAdvanceAltarOpen Event End!");
}

// IDA_EA: 0x811BB80
// IDA_NAME: _ZN18CAssaultOnOffEventC2Ev
// DEMANGLED: CAssaultOnOffEvent::CAssaultOnOffEvent(void)
CAssaultOnOffEvent::CAssaultOnOffEvent()
    : CEventBase() {}

// IDA_EA: 0x811BB9C
// IDA_NAME: _ZN18CAssaultOnOffEventD2Ev
// DEMANGLED: CAssaultOnOffEvent::~CAssaultOnOffEvent()
// IDA_EA: 0x811BBCC
// IDA_NAME: _ZN18CAssaultOnOffEventD0Ev
// DEMANGLED: CAssaultOnOffEvent::~CAssaultOnOffEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CAssaultOnOffEvent::~CAssaultOnOffEvent() {}

// IDA_EA: 0x811BAC2
// IDA_NAME: _ZN18CAssaultOnOffEvent10StartEventEv
// DEMANGLED: CAssaultOnOffEvent::StartEvent(void)
void CAssaultOnOffEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("virtual void CAssaultOnOffEvent::StartEvent()", 14, 0);
    trace("Assault On Off Event Start!");
}

// IDA_EA: 0x811BB14
// IDA_NAME: _ZN18CAssaultOnOffEvent10StartEventE10Word_Param
// DEMANGLED: CAssaultOnOffEvent::StartEvent(Word_Param)
void CAssaultOnOffEvent::StartEvent(Word_Param param) {
    (void)param;
    StartEvent();
}

// IDA_EA: 0x811BB2E
// IDA_NAME: _ZN18CAssaultOnOffEvent8EndEventEv
// DEMANGLED: CAssaultOnOffEvent::EndEvent(void)
void CAssaultOnOffEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void CAssaultOnOffEvent::EndEvent()", 24, 0);
    trace("Assault On Off Event End!");
}

// IDA_EA: 0x8184970
// IDA_NAME: _ZN15CConditionEventC2Ev
// DEMANGLED: CConditionEvent::CConditionEvent(void)
CConditionEvent::CConditionEvent()
    : CEventBase() {}

// IDA_EA: 0x818498C
// IDA_NAME: _ZN15CConditionEventD2Ev
// DEMANGLED: CConditionEvent::~CConditionEvent()
// IDA_EA: 0x81849BC
// IDA_NAME: _ZN15CConditionEventD0Ev
// DEMANGLED: CConditionEvent::~CConditionEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CConditionEvent::~CConditionEvent() {}

// IDA_EA: 0x81848BC
// IDA_NAME: _ZN15CConditionEvent10StartEventEv
// DEMANGLED: CConditionEvent::StartEvent(void)
void CConditionEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("virtual void CConditionEvent::StartEvent()", 13, 0);
    trace("[ConditionEvent] Start!");
}

// IDA_EA: 0x818490E
// IDA_NAME: _ZN15CConditionEvent10StartEventE10Word_Param
// DEMANGLED: CConditionEvent::StartEvent(Word_Param)
void CConditionEvent::StartEvent(Word_Param param) {
    (void)param;
    StartEvent();
}

// IDA_EA: 0x8184928
// IDA_NAME: _ZN15CConditionEvent8EndEventEv
// DEMANGLED: CConditionEvent::EndEvent(void)
void CConditionEvent::EndEvent() {
    LogManager::logFormat(1,
                          "localjapan/Arad_ConditionEvent.cpp",
                          "virtual void CConditionEvent::EndEvent()",
                          0x17,
                          "[CConditionEvent] End Event.");
    SetEventFlag(false);
}

// IDA_EA: 0x811C0BA
// IDA_NAME: _ZN27CDnFLeaguePromoteFirstEventC2Ev
// DEMANGLED: CDnFLeaguePromoteFirstEvent::CDnFLeaguePromoteFirstEvent(void)
CDnFLeaguePromoteFirstEvent::CDnFLeaguePromoteFirstEvent()
    : CEventBase() {}

// IDA_EA: 0x811C0D6
// IDA_NAME: _ZN27CDnFLeaguePromoteFirstEventD2Ev
// DEMANGLED: CDnFLeaguePromoteFirstEvent::~CDnFLeaguePromoteFirstEvent()
// IDA_EA: 0x811C106
// IDA_NAME: _ZN27CDnFLeaguePromoteFirstEventD0Ev
// DEMANGLED: CDnFLeaguePromoteFirstEvent::~CDnFLeaguePromoteFirstEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CDnFLeaguePromoteFirstEvent::~CDnFLeaguePromoteFirstEvent() {}

// IDA_EA: 0x811C124
// IDA_NAME: _ZN27CDnFLeaguePromoteFirstEvent10StartEventEv
// DEMANGLED: CDnFLeaguePromoteFirstEvent::StartEvent(void)
void CDnFLeaguePromoteFirstEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace(
        "virtual void CDnFLeaguePromoteFirstEvent::StartEvent()", 19, 0);
    trace("CDnFLeaguePromoteFirstEvent Event Start!");
}

// IDA_EA: 0x811C176
// IDA_NAME: _ZN27CDnFLeaguePromoteFirstEvent10StartEventE10Word_Param
// DEMANGLED: CDnFLeaguePromoteFirstEvent::StartEvent(Word_Param)
void CDnFLeaguePromoteFirstEvent::StartEvent(Word_Param param) {
    (void)param;
    StartEvent();
}

// IDA_EA: 0x811C190
// IDA_NAME: _ZN27CDnFLeaguePromoteFirstEvent8EndEventEv
// DEMANGLED: CDnFLeaguePromoteFirstEvent::EndEvent(void)
void CDnFLeaguePromoteFirstEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void CDnFLeaguePromoteFirstEvent::EndEvent()",
                   29,
                   0);
    trace("CDnFLeaguePromoteFirstEvent Event End!");
}

// IDA_EA: 0x811C1E2
// IDA_NAME: _ZN28CDnFLeaguePromoteSecondEventC2Ev
// DEMANGLED: CDnFLeaguePromoteSecondEvent::CDnFLeaguePromoteSecondEvent(void)
CDnFLeaguePromoteSecondEvent::CDnFLeaguePromoteSecondEvent()
    : CEventBase() {}

// IDA_EA: 0x811C1FE
// IDA_NAME: _ZN28CDnFLeaguePromoteSecondEventD2Ev
// DEMANGLED: CDnFLeaguePromoteSecondEvent::~CDnFLeaguePromoteSecondEvent()
// IDA_EA: 0x811C22E
// IDA_NAME: _ZN28CDnFLeaguePromoteSecondEventD0Ev
// DEMANGLED: CDnFLeaguePromoteSecondEvent::~CDnFLeaguePromoteSecondEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CDnFLeaguePromoteSecondEvent::~CDnFLeaguePromoteSecondEvent() {}

// IDA_EA: 0x811C24C
// IDA_NAME: _ZN28CDnFLeaguePromoteSecondEvent10StartEventEv
// DEMANGLED: CDnFLeaguePromoteSecondEvent::StartEvent(void)
void CDnFLeaguePromoteSecondEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace(
        "virtual void CDnFLeaguePromoteSecondEvent::StartEvent()", 43, 0);
    trace("CDnFLeaguePromoteSecondEvent Event Start!");
}

// IDA_EA: 0x811C29E
// IDA_NAME: _ZN28CDnFLeaguePromoteSecondEvent10StartEventE10Word_Param
// DEMANGLED: CDnFLeaguePromoteSecondEvent::StartEvent(Word_Param)
void CDnFLeaguePromoteSecondEvent::StartEvent(Word_Param param) {
    (void)param;
    StartEvent();
}

// IDA_EA: 0x811C2B8
// IDA_NAME: _ZN28CDnFLeaguePromoteSecondEvent8EndEventEv
// DEMANGLED: CDnFLeaguePromoteSecondEvent::EndEvent(void)
void CDnFLeaguePromoteSecondEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void CDnFLeaguePromoteSecondEvent::EndEvent()",
                   53,
                   0);
    trace("CDnFLeaguePromoteSecondEvent Event End!");
}

// IDA_EA: 0x8116EE6
// IDA_NAME: _ZN23CBurnigGoldMonsterEventC2Ev
// DEMANGLED: CBurnigGoldMonsterEvent::CBurnigGoldMonsterEvent(void)
CBurnigGoldMonsterEvent::CBurnigGoldMonsterEvent()
    : CEventBase() {}

// IDA_EA: 0x8116F02
// IDA_NAME: _ZN23CBurnigGoldMonsterEventD2Ev
// DEMANGLED: CBurnigGoldMonsterEvent::~CBurnigGoldMonsterEvent()
// IDA_EA: 0x8116F32
// IDA_NAME: _ZN23CBurnigGoldMonsterEventD0Ev
// DEMANGLED: CBurnigGoldMonsterEvent::~CBurnigGoldMonsterEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CBurnigGoldMonsterEvent::~CBurnigGoldMonsterEvent() {}

// IDA_EA: 0x8116F50
// IDA_NAME: _ZN23CBurnigGoldMonsterEvent10StartEventEv
// DEMANGLED: CBurnigGoldMonsterEvent::StartEvent(void)
void CBurnigGoldMonsterEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("virtual void CBurnigGoldMonsterEvent::StartEvent()",
                   18,
                   0);
    trace("CBurnigGoldMonsterEvent Event Start!");
}

// IDA_EA: 0x8116FA2
// IDA_NAME: _ZN23CBurnigGoldMonsterEvent10StartEventE10Word_Param
// DEMANGLED: CBurnigGoldMonsterEvent::StartEvent(Word_Param)
void CBurnigGoldMonsterEvent::StartEvent(Word_Param param) {
    (void)param;
    StartEvent();
}

// IDA_EA: 0x8116FBC
// IDA_NAME: _ZN23CBurnigGoldMonsterEvent8EndEventEv
// DEMANGLED: CBurnigGoldMonsterEvent::EndEvent(void)
void CBurnigGoldMonsterEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void CBurnigGoldMonsterEvent::EndEvent()",
                   28,
                   0);
    trace("CBurnigGoldMonsterEvent Event End!");
}

// IDA_EA: 0x819E738
// IDA_NAME: _ZN16Arad_MomijiEventC2Ev
// DEMANGLED: Arad_MomijiEvent::Arad_MomijiEvent(void)
Arad_MomijiEvent::Arad_MomijiEvent()
    : CEventBase() {}

// IDA_EA: 0x819E754
// IDA_NAME: _ZN16Arad_MomijiEventD2Ev
// DEMANGLED: Arad_MomijiEvent::~Arad_MomijiEvent()
// IDA_EA: 0x819E784
// IDA_NAME: _ZN16Arad_MomijiEventD0Ev
// DEMANGLED: Arad_MomijiEvent::~Arad_MomijiEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
Arad_MomijiEvent::~Arad_MomijiEvent() {}

// IDA_EA: 0x819E7A2
// IDA_NAME: _ZN16Arad_MomijiEvent10StartEventEv
// DEMANGLED: Arad_MomijiEvent::StartEvent(void)
void Arad_MomijiEvent::StartEvent() {
    cMyTrace trace("virtual void Arad_MomijiEvent::StartEvent()", 20, 0);
    trace("[MOMIJI_EVENT] Event Start.");
    SetEventFlag(true);
}

// IDA_EA: 0x819E7F4
// IDA_NAME: _ZN16Arad_MomijiEvent10StartEventE10Word_Param
// DEMANGLED: Arad_MomijiEvent::StartEvent(Word_Param)
void Arad_MomijiEvent::StartEvent(Word_Param param) {
    (void)param;
    StartEvent();
}

// IDA_EA: 0x819E80E
// IDA_NAME: _ZN16Arad_MomijiEvent8EndEventEv
// DEMANGLED: Arad_MomijiEvent::EndEvent(void)
void Arad_MomijiEvent::EndEvent() {
    cMyTrace trace("virtual void Arad_MomijiEvent::EndEvent()", 31, 0);
    trace("[MOMIJI_EVENT] Event End.");
    SetEventFlag(false);
}

// IDA_EA: 0x81A0EA0
// IDA_NAME: _ZN17CAradRyosikaEventC2Ev
// DEMANGLED: CAradRyosikaEvent::CAradRyosikaEvent(void)
CAradRyosikaEvent::CAradRyosikaEvent()
    : CEventBase(),
      step_(-1),
      job_(-1) {}

// IDA_EA: 0x81A0ED0
// IDA_NAME: _ZN17CAradRyosikaEventD2Ev
// DEMANGLED: CAradRyosikaEvent::~CAradRyosikaEvent()
// IDA_EA: 0x81A0F00
// IDA_NAME: _ZN17CAradRyosikaEventD0Ev
// DEMANGLED: CAradRyosikaEvent::~CAradRyosikaEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CAradRyosikaEvent::~CAradRyosikaEvent() {}

// IDA_EA: 0x81A0F1E
// IDA_NAME: _ZN17CAradRyosikaEvent10StartEventEv
// DEMANGLED: CAradRyosikaEvent::StartEvent(void)
void CAradRyosikaEvent::StartEvent() {
    cMyTrace trace("StartEvent", 20, 0);
    trace("Arad Ryosika Event Start! [step:%d][job:%d]", step_, job_);
    SetEventFlag(true);
}

// IDA_EA: 0x81A0F8A
// IDA_NAME: _ZN17CAradRyosikaEvent10StartEventE10Word_Param
// DEMANGLED: CAradRyosikaEvent::StartEvent(Word_Param)
void CAradRyosikaEvent::StartEvent(Word_Param param) {
    step_ = static_cast<int32_t>(param & 0xffffu);
    job_ = static_cast<int32_t>((param >> 16) & 0xffffu);
    StartEvent();
}

// IDA_EA: 0x81A0FBE
// IDA_NAME: _ZN17CAradRyosikaEvent8EndEventEv
// DEMANGLED: CAradRyosikaEvent::EndEvent(void)
void CAradRyosikaEvent::EndEvent() {
    SetEventFlag(false);
}

// IDA_EA: 0x81A0FDA
// IDA_NAME: _ZN17CAradRyosikaEvent15ApplyDatePeriodEP5CUser
// DEMANGLED: CAradRyosikaEvent::ApplyDatePeriod(CUser *)
int CAradRyosikaEvent::ApplyDatePeriod(CUser* user) {
    (void)user;
    return 1;
}

// IDA_EA: 0x81A0FE4
// IDA_NAME: _ZN17CAradRyosikaEvent8SendMailEP5CUseriii
// DEMANGLED: CAradRyosikaEvent::SendMail(CUser *,int,int,int)
int CAradRyosikaEvent::SendMail(CUser* user,
                                int receiver_charac_no,
                                int usable_level,
                                int charac_job) {
    if (job_ >= 0) {
        if (charac_job == oldsrv::config::kCharacterJobAtFighter) {
            usable_level = 5;
        }
        if (job_ != charac_job) {
            return job_;
        }
    }

    Arad_GiftItem_Set* const gift_items =
        GlobalInstance<Arad_GiftItem_Set>::inst();
    return gift_items->send_mail(user,
                                 receiver_charac_no,
                                 usable_level,
                                 charac_job,
                                 step_);
}

// IDA_EA: 0x81A107C
// IDA_NAME: _ZN17CAradRyosikaEvent7DoEventEP5CUseriii
// DEMANGLED: CAradRyosikaEvent::DoEvent(CUser *,int,int,int)
void CAradRyosikaEvent::DoEvent(CUser* user,
                                int receiver_charac_no,
                                int unused,
                                int charac_job) {
    (void)unused;
    if (user && IsEventing(0) && ApplyDatePeriod(user)) {
        SendMail(user, receiver_charac_no, 1, charac_job);
    }
}

// IDA_EA: 0x80C828C
// IDA_NAME: _ZN15AttendanceEventC2Ev
// DEMANGLED: AttendanceEvent::AttendanceEvent(void)
AttendanceEvent::AttendanceEvent()
    : CEventBase() {}

// IDA_EA: 0x80C82A8
// IDA_NAME: _ZN15AttendanceEventD2Ev
// DEMANGLED: AttendanceEvent::~AttendanceEvent()
// IDA_EA: 0x80C82D8
// IDA_NAME: _ZN15AttendanceEventD0Ev
// DEMANGLED: AttendanceEvent::~AttendanceEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
AttendanceEvent::~AttendanceEvent() {}

// IDA_EA: 0x80C82F6
// IDA_NAME: _ZN15AttendanceEvent10StartEventEv
// DEMANGLED: AttendanceEvent::StartEvent(void)
void AttendanceEvent::StartEvent() {
    const bool was_active = IsEventing(0) == 1;
    SetEventFlag(true);
    if (!was_active) {
        event_attendance::OnEventStarted();
    }
}

// IDA_EA: 0x80C8312
// IDA_NAME: _ZN15AttendanceEvent10StartEventE10Word_Param
// DEMANGLED: AttendanceEvent::StartEvent(Word_Param)
void AttendanceEvent::StartEvent(Word_Param param) {
    add_info_.word_00 = static_cast<int16_t>(param & 0xffffu);
    add_info_.word_02 = static_cast<int16_t>((param >> 16) & 0xffffu);
    if (add_info_.word_00 <= 0) {
        LogManager::logFormat(1,
                              "AttendanceEvent.cpp",
                              "virtual void AttendanceEvent::StartEvent(Word_Param)",
                              0x19,
                              "Attendance(AddInfo:%d,%d) Event Start Error!",
                              add_info_.word_00,
                              add_info_.word_02);
        return;
    }
    StartEvent();
    cMyTrace trace("virtual void AttendanceEvent::StartEvent(Word_Param)",
                   29,
                   0);
    trace("Attendance(AddInfo:%d,%d) Event Start!",
          add_info_.word_00,
          add_info_.word_02);
}

// IDA_EA: 0x80C83F4
// IDA_NAME: _ZN15AttendanceEvent8EndEventEv
// DEMANGLED: AttendanceEvent::EndEvent(void)
void AttendanceEvent::EndEvent() {
    // Seal every online account at one shared epoch before dropping the event
    // flag; later disconnect/reset paths reuse that exact boundary and cannot
    // accrue post-event time.
    if (IsEventing(0) == 1) {
        event_attendance::OnEventEnded();
    }
    SetEventFlag(false);
    cMyTrace trace("virtual void AttendanceEvent::EndEvent()", 35, 0);
    trace("Attendacne Event End!");
}

// IDA_EA: 0x80C8446
// IDA_NAME: _ZN15AttendanceEvent13getRewordItemEv
// DEMANGLED: AttendanceEvent::getRewordItem(void)
int AttendanceEvent::getRewordItem() {
    stADDINFO add_info = GetAddInfo();
    return add_info.word_00;
}

// IDA_EA: 0x810A720
// IDA_NAME: _ZN21CClearRewardCardEventC2Ev
// DEMANGLED: CClearRewardCardEvent::CClearRewardCardEvent(void)
CClearRewardCardEvent::CClearRewardCardEvent()
    : CEventBase() {}

// IDA_EA: 0x810A73C
// IDA_NAME: _ZN21CClearRewardCardEventD2Ev
// DEMANGLED: CClearRewardCardEvent::~CClearRewardCardEvent()
// IDA_EA: 0x810A76C
// IDA_NAME: _ZN21CClearRewardCardEventD0Ev
// DEMANGLED: CClearRewardCardEvent::~CClearRewardCardEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CClearRewardCardEvent::~CClearRewardCardEvent() {}

// IDA_EA: 0x810A78A
// IDA_NAME: _ZN21CClearRewardCardEvent10StartEventEv
// DEMANGLED: CClearRewardCardEvent::StartEvent(void)
void CClearRewardCardEvent::StartEvent() {}

// IDA_EA: 0x810A790
// IDA_NAME: _ZN21CClearRewardCardEvent10StartEventE10Word_Param
// DEMANGLED: CClearRewardCardEvent::StartEvent(Word_Param)
void CClearRewardCardEvent::StartEvent(Word_Param param) {
    (void)param;
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 24, 0);
    trace("Clear reward card event Start");
}

// IDA_EA: 0x810A7E2
// IDA_NAME: _ZN21CClearRewardCardEvent8EndEventEv
// DEMANGLED: CClearRewardCardEvent::EndEvent(void)
void CClearRewardCardEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("EndEvent", 30, 0);
    trace("Clear reward card event End");
}

// IDA_EA: 0x810A914
// IDA_NAME: _ZN22CCoinEventOnCharCreateC2Ev
// DEMANGLED: CCoinEventOnCharCreate::CCoinEventOnCharCreate(void)
CCoinEventOnCharCreate::CCoinEventOnCharCreate()
    : CEventBase(),
      coin_no_on_create_charac_(5) {}

// IDA_EA: 0x810A93A
// IDA_NAME: _ZN22CCoinEventOnCharCreateD2Ev
// DEMANGLED: CCoinEventOnCharCreate::~CCoinEventOnCharCreate()
// IDA_EA: 0x810A96A
// IDA_NAME: _ZN22CCoinEventOnCharCreateD0Ev
// DEMANGLED: CCoinEventOnCharCreate::~CCoinEventOnCharCreate() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CCoinEventOnCharCreate::~CCoinEventOnCharCreate() {}

// IDA_EA: 0x810A988
// IDA_NAME: _ZN22CCoinEventOnCharCreate10StartEventEv
// DEMANGLED: CCoinEventOnCharCreate::StartEvent(void)
void CCoinEventOnCharCreate::StartEvent() {
    SetEventFlag(true);
    coin_no_on_create_charac_ = 5;
    cMyTrace trace("StartEvent", 21, 0);
    trace("Coin Event On Char Create Start!");
}

// IDA_EA: 0x810A9E4
// IDA_NAME: _ZN22CCoinEventOnCharCreate10StartEventE10Word_Param
// DEMANGLED: CCoinEventOnCharCreate::StartEvent(Word_Param)
void CCoinEventOnCharCreate::StartEvent(Word_Param param) {
    SetEventFlag(true);
    coin_no_on_create_charac_ = static_cast<uint16_t>(param);
    cMyTrace trace("StartEvent", 28, 0);
    trace("Coin Event On Char Create Start!");
}

// IDA_EA: 0x810AA42
// IDA_NAME: _ZN22CCoinEventOnCharCreate8EndEventEv
// DEMANGLED: CCoinEventOnCharCreate::EndEvent(void)
void CCoinEventOnCharCreate::EndEvent() {
    SetEventFlag(false);
    coin_no_on_create_charac_ = 5;
    cMyTrace trace("EndEvent", 36, 0);
    trace("Coin Event On Char Create End!");
}

// IDA_EA: 0x848F200
// IDA_NAME: _ZN22CCoinEventOnCharCreate23GetCoinNoOnCreateCharacEv
// DEMANGLED: CCoinEventOnCharCreate::GetCoinNoOnCreateCharac(void)
int CCoinEventOnCharCreate::GetCoinNoOnCreateCharac() {
    return coin_no_on_create_charac_;
}

// IDA_EA: 0x810AB10
// IDA_NAME: _ZN16CCoinEventPerDayC2Ev
// DEMANGLED: CCoinEventPerDay::CCoinEventPerDay(void)
CCoinEventPerDay::CCoinEventPerDay()
    : CEventBase(),
      coin_no_level_27_plus_(1) {
    reserved_09_[0] = 1;
    reserved_09_[1] = 1;
    reserved_09_[2] = 1;
}

// IDA_EA: 0x810AB48
// IDA_NAME: _ZN16CCoinEventPerDayD2Ev
// DEMANGLED: CCoinEventPerDay::~CCoinEventPerDay()
// IDA_EA: 0x810AB78
// IDA_NAME: _ZN16CCoinEventPerDayD0Ev
// DEMANGLED: CCoinEventPerDay::~CCoinEventPerDay() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CCoinEventPerDay::~CCoinEventPerDay() {}

// IDA_EA: 0x810AB96
// IDA_NAME: _ZN16CCoinEventPerDay10StartEventEv
// DEMANGLED: CCoinEventPerDay::StartEvent(void)
void CCoinEventPerDay::StartEvent() {
    SetEventFlag(true);
    reserved_09_[0] = 1;
    reserved_09_[1] = 1;
    reserved_09_[2] = 1;
    coin_no_level_27_plus_ = 1;
    cMyTrace trace("StartEvent", 56, 0);
    trace("Coin Event Per Day Start!");
}

// IDA_EA: 0x810AC04
// IDA_NAME: _ZN16CCoinEventPerDay10StartEventE10Word_Param
// DEMANGLED: CCoinEventPerDay::StartEvent(Word_Param)
void CCoinEventPerDay::StartEvent(Word_Param param) {
    SetEventFlag(true);
    reserved_09_[0] = static_cast<uint8_t>(param & 0xffu);
    reserved_09_[1] = static_cast<uint8_t>((param >> 8) & 0xffu);
    reserved_09_[2] = static_cast<uint8_t>((param >> 16) & 0xffu);
    coin_no_level_27_plus_ = static_cast<uint8_t>((param >> 24) & 0xffu);
    cMyTrace trace("StartEvent", 65, 0);
    trace("Coin Event Per Day Start! coin(%d) coin(%d) coin(%d) coin(%d) ",
          reserved_09_[0],
          reserved_09_[1],
          reserved_09_[2],
          coin_no_level_27_plus_);
}

// IDA_EA: 0x810ACA8
// IDA_NAME: _ZN16CCoinEventPerDay8EndEventEv
// DEMANGLED: CCoinEventPerDay::EndEvent(void)
void CCoinEventPerDay::EndEvent() {
    SetEventFlag(false);
    reserved_09_[0] = 1;
    reserved_09_[1] = 1;
    reserved_09_[2] = 1;
    coin_no_level_27_plus_ = 1;
    cMyTrace trace("EndEvent", 99, 0);
    trace("Coin Event Per Day End");
}

// IDA_EA: 0x810AD16
// IDA_NAME: _ZN16CCoinEventPerDay15GetCoinNoPerDayEi
// DEMANGLED: CCoinEventPerDay::GetCoinNoPerDay(int)
int CCoinEventPerDay::GetCoinNoPerDay(int level) {
    if (level <= 17) {
        return reserved_09_[0];
    }
    if (level > 26) {
        return reserved_09_[2];
    }
    return reserved_09_[1];
}

// IDA_EA: 0x832BDF4
// IDA_NAME: _ZN16CCoinRefillEventC2Ev
// DEMANGLED: CCoinRefillEvent::CCoinRefillEvent(void)
CCoinRefillEvent::CCoinRefillEvent()
    : CEventBase(),
      event_time_count_(-1),
      event_times_ {-1, -1, -1, -1} {}

// IDA_EA: 0x832BE2E
// IDA_NAME: _ZN16CCoinRefillEventD2Ev
// DEMANGLED: CCoinRefillEvent::~CCoinRefillEvent()
// IDA_EA: 0x832BE5E
// IDA_NAME: _ZN16CCoinRefillEventD0Ev
// DEMANGLED: CCoinRefillEvent::~CCoinRefillEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CCoinRefillEvent::~CCoinRefillEvent() {}

// IDA_EA: 0x832BE7C
// IDA_NAME: _ZN16CCoinRefillEvent10StartEventEv
// DEMANGLED: CCoinRefillEvent::StartEvent(void)
void CCoinRefillEvent::StartEvent() {}

// IDA_EA: 0x832BE82
// IDA_NAME: _ZN16CCoinRefillEvent10StartEventE10Word_Param
// DEMANGLED: CCoinRefillEvent::StartEvent(Word_Param)
void CCoinRefillEvent::StartEvent(Word_Param param) {
    SetEventFlag(true);
    event_time_count_ = -1;
    event_times_[0] = -1;
    event_times_[1] = -1;
    event_times_[2] = -1;
    event_times_[3] = -1;

    event_times_[0] = G_CEnvironment()->get_fatigue_time();
    event_times_[1] = static_cast<int32_t>(param & 0xffffu);
    event_times_[2] = static_cast<int32_t>((param >> 16) & 0xffffu);

    for (event_time_count_ = 0; event_time_count_ <= 3; ++event_time_count_) {
        int32_t& event_time = event_times_[event_time_count_];
        if (event_time > 23) {
            event_time = -1;
        }
        if (event_time == -1) {
            break;
        }
    }

    for (int i = 1; event_time_count_ >= i; ++i) {
        int selected_time = 25;
        int selected_index = 5;
        for (int j = i; event_time_count_ >= j; ++j) {
            if (event_times_[j - 1] < selected_time) {
                selected_time = event_times_[j - 1];
                selected_index = j;
            }
        }
        event_times_[selected_index - 1] = event_times_[i - 1];
        event_times_[i - 1] = selected_time;
    }

    registNextRefillTimer();

    cMyTrace trace("virtual void CCoinRefillEvent::StartEvent(Word_Param)",
                   72,
                   0);
    trace("Coin Refill Event Start! time(%d) time(%d) time(%d)",
          event_times_[0],
          event_times_[1],
          event_times_[2]);
}

// IDA_EA: 0x832C048
// IDA_NAME: _ZN16CCoinRefillEvent8EndEventEv
// DEMANGLED: CCoinRefillEvent::EndEvent(void)
void CCoinRefillEvent::EndEvent() {
    SetEventFlag(false);
    event_time_count_ = -1;
    event_times_[0] = -1;
    event_times_[1] = -1;
    event_times_[2] = -1;
    event_times_[3] = -1;
    cMyTrace trace("virtual void CCoinRefillEvent::EndEvent()", 79, 0);
    trace("Coin Refill Event End");
}

// IDA_EA: 0x832C0B8
// IDA_NAME: _ZNK16CCoinRefillEvent16getLastEventTimeEv
// DEMANGLED: CCoinRefillEvent::getLastEventTime(void)const
int CCoinRefillEvent::getLastEventTime() const {
    std::tm local_time;
    std::memset(&local_time, 0, sizeof(local_time));
    time_t timer = static_cast<time_t>(GlobalData::system_time()->getCurSec());
    localtime_r(&timer, &local_time);

    if (local_time.tm_hour < event_times_[0]) {
        return event_times_[event_time_count_];
    }
    for (int i = 2; event_time_count_ >= i; ++i) {
        if (local_time.tm_hour < event_times_[i - 1]) {
            return event_times_[i - 2];
        }
    }
    return event_times_[event_time_count_];
}

// IDA_EA: 0x832C148
// IDA_NAME: _ZNK16CCoinRefillEvent16getNextEventTimeERK2tm
// DEMANGLED: CCoinRefillEvent::getNextEventTime(tm const&)const
int CCoinRefillEvent::getNextEventTime(const std::tm& local_time) const {
    for (int i = 1; event_time_count_ >= i; ++i) {
        const int event_time = event_times_[i - 1];
        if (event_time != G_CEnvironment()->get_fatigue_time() &&
            local_time.tm_hour < event_time) {
            return event_time;
        }
    }
    for (int i = 1; event_time_count_ >= i; ++i) {
        const int event_time = event_times_[i - 1];
        if (event_time != G_CEnvironment()->get_fatigue_time()) {
            return event_time;
        }
    }
    return event_times_[0];
}

// IDA_EA: 0x832C1FE
// IDA_NAME: _ZNK16CCoinRefillEvent21registNextRefillTimerEv
// DEMANGLED: CCoinRefillEvent::registNextRefillTimer(void)const
int CCoinRefillEvent::registNextRefillTimer() const {
    std::tm local_time;
    std::memset(&local_time, 0, sizeof(local_time));
    time_t timer = static_cast<time_t>(GlobalData::system_time()->getCurSec());
    localtime_r(&timer, &local_time);

    std::tm next_time = local_time;
    next_time.tm_hour = getNextEventTime(local_time);
    if (next_time.tm_hour < local_time.tm_hour) {
        ++next_time.tm_mday;
    }
    next_time.tm_min = 0;
    next_time.tm_sec = 0;

    time_t next_timer = mktime(&next_time);
    const int delay_sec = static_cast<int>(next_timer - timer);
    if (delay_sec < 0) {
        return delay_sec;
    }

    G_TimerQueue()->InsertTimer(2, 0, 101, delay_sec, 0, 0);
    cMyTrace trace("void CCoinRefillEvent::registNextRefillTimer() const",
                   136,
                   0);
    return trace("Next coin refill time is %s", ctime(&next_timer));
}

// IDA_EA: 0x8116B48
// IDA_NAME: _ZN24CCollectArchieveEventLogC2Ev
// DEMANGLED: CCollectArchieveEventLog::CCollectArchieveEventLog(void)
CCollectArchieveEventLog::CCollectArchieveEventLog()
    : CEventBase() {}

// IDA_EA: 0x8116B64
// IDA_NAME: _ZN24CCollectArchieveEventLogD2Ev
// DEMANGLED: CCollectArchieveEventLog::~CCollectArchieveEventLog()
// IDA_EA: 0x8116B94
// IDA_NAME: _ZN24CCollectArchieveEventLogD0Ev
// DEMANGLED: CCollectArchieveEventLog::~CCollectArchieveEventLog() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CCollectArchieveEventLog::~CCollectArchieveEventLog() {}

// IDA_EA: 0x8116BB2
// IDA_NAME: _ZN24CCollectArchieveEventLog10StartEventEv
// DEMANGLED: CCollectArchieveEventLog::StartEvent(void)
void CCollectArchieveEventLog::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("virtual void CCollectArchieveEventLog::StartEvent()",
                   25,
                   0);
    trace("Collect Archieve Event Log Start!");
}

// IDA_EA: 0x8116C04
// IDA_NAME: _ZN24CCollectArchieveEventLog10StartEventE10Word_Param
// DEMANGLED: CCollectArchieveEventLog::StartEvent(Word_Param)
void CCollectArchieveEventLog::StartEvent(Word_Param param) {
    (void)param;
    StartEvent();
}

// IDA_EA: 0x8116C1E
// IDA_NAME: _ZN24CCollectArchieveEventLog8EndEventEv
// DEMANGLED: CCollectArchieveEventLog::EndEvent(void)
void CCollectArchieveEventLog::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void CCollectArchieveEventLog::EndEvent()",
                   35,
                   0);
    trace("Collect Archieve Event Log End!");
}

// IDA_EA: 0x814AAD8
// IDA_NAME: _ZN15CExpDoubleEvent12GetExpFactorEv
// DEMANGLED: CExpDoubleEvent::GetExpFactor(void)
int CExpDoubleEvent::GetExpFactor() {
    return exp_factor_;
}

// IDA_EA: 0x849AC44
// IDA_NAME: _ZN15CExpDoubleEventC2Ev
// DEMANGLED: CExpDoubleEvent::CExpDoubleEvent(void)
CExpDoubleEvent::CExpDoubleEvent()
    : CEventBase(),
      exp_factor_(100) {}

// IDA_EA: 0x849AC6A
// IDA_NAME: _ZN15CExpDoubleEventD2Ev
// DEMANGLED: CExpDoubleEvent::~CExpDoubleEvent()
// IDA_EA: 0x849AC9A
// IDA_NAME: _ZN15CExpDoubleEventD0Ev
// DEMANGLED: CExpDoubleEvent::~CExpDoubleEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CExpDoubleEvent::~CExpDoubleEvent() {}

// IDA_EA: 0x849ACB8
// IDA_NAME: _ZN15CExpDoubleEvent10StartEventEv
// DEMANGLED: CExpDoubleEvent::StartEvent(void)
void CExpDoubleEvent::StartEvent() {
    SetEventFlag(true);
    exp_factor_ = 100;
    cMyTrace trace("StartEvent", 19, 0);
    trace("Exp Double Event Start!");
}

// IDA_EA: 0x849AD14
// IDA_NAME: _ZN15CExpDoubleEvent10StartEventE10Word_Param
// DEMANGLED: CExpDoubleEvent::StartEvent(Word_Param)
void CExpDoubleEvent::StartEvent(Word_Param param) {
    SetEventFlag(true);
    exp_factor_ = static_cast<uint16_t>(param);
    cMyTrace trace("StartEvent", 26, 0);
    trace("Exp Double Event Start!");
}

// IDA_EA: 0x849AD72
// IDA_NAME: _ZN15CExpDoubleEvent8EndEventEv
// DEMANGLED: CExpDoubleEvent::EndEvent(void)
void CExpDoubleEvent::EndEvent() {
    SetEventFlag(false);
    exp_factor_ = 100;
    cMyTrace trace("EndEvent", 33, 0);
    trace("Exp Double Event End!");
}

// IDA_EA: 0x85331D0
// IDA_NAME: _ZN19CItemDropRatioEventC2Ev
// DEMANGLED: CItemDropRatioEvent::CItemDropRatioEvent(void)
CItemDropRatioEvent::CItemDropRatioEvent()
    : CEventBase(),
      item_drop_rate_(1) {}

// IDA_EA: 0x85331F6
// IDA_NAME: _ZN19CItemDropRatioEventD2Ev
// DEMANGLED: CItemDropRatioEvent::~CItemDropRatioEvent()
// IDA_EA: 0x8533226
// IDA_NAME: _ZN19CItemDropRatioEventD0Ev
// DEMANGLED: CItemDropRatioEvent::~CItemDropRatioEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CItemDropRatioEvent::~CItemDropRatioEvent() {}

// IDA_EA: 0x8533244
// IDA_NAME: _ZN19CItemDropRatioEvent10StartEventEv
// DEMANGLED: CItemDropRatioEvent::StartEvent(void)
void CItemDropRatioEvent::StartEvent() {
    SetEventFlag(true);
    item_drop_rate_ = 1;
    G_CDataManager()->SetItemDropRate(1.0f);
    cMyTrace trace("virtual void CItemDropRatioEvent::StartEvent()", 22, 0);
    trace("Item Drop Event Start!");
}

// IDA_EA: 0x85332B6
// IDA_NAME: _ZN19CItemDropRatioEvent10StartEventE10Word_Param
// DEMANGLED: CItemDropRatioEvent::StartEvent(Word_Param)
void CItemDropRatioEvent::StartEvent(Word_Param param) {
    SetEventFlag(true);
    item_drop_rate_ = static_cast<uint16_t>(param);
    G_CDataManager()->SetItemDropRate(static_cast<float>(item_drop_rate_));
    cMyTrace trace("virtual void CItemDropRatioEvent::StartEvent(Word_Param)",
                   31,
                   0);
    trace("Item Drop Event Start!");
}

// IDA_EA: 0x8533340
// IDA_NAME: _ZN19CItemDropRatioEvent8EndEventEv
// DEMANGLED: CItemDropRatioEvent::EndEvent(void)
void CItemDropRatioEvent::EndEvent() {
    SetEventFlag(false);
    item_drop_rate_ = 1;
    G_CDataManager()->SetItemDropRate(1.0f);
    cMyTrace trace("virtual void CItemDropRatioEvent::EndEvent()", 40, 0);
    trace("Item Drop Event End!");
}

// IDA_EA: 0x812CE54
// IDA_NAME: _ZN22CMaxFatigueFactorEvent19GetMaxFatigueFactorEv
// DEMANGLED: CMaxFatigueFactorEvent::GetMaxFatigueFactor(void)
int CMaxFatigueFactorEvent::GetMaxFatigueFactor() {
    return max_fatigue_factor_;
}

// IDA_EA: 0x81B2718
// IDA_NAME: _ZN22CMaxFatigueFactorEventC2Ev
// DEMANGLED: CMaxFatigueFactorEvent::CMaxFatigueFactorEvent(void)
CMaxFatigueFactorEvent::CMaxFatigueFactorEvent()
    : CEventBase(),
      max_fatigue_factor_(100) {}

// IDA_EA: 0x81B273E
// IDA_NAME: _ZN22CMaxFatigueFactorEventD2Ev
// DEMANGLED: CMaxFatigueFactorEvent::~CMaxFatigueFactorEvent()
// IDA_EA: 0x81B276E
// IDA_NAME: _ZN22CMaxFatigueFactorEventD0Ev
// DEMANGLED: CMaxFatigueFactorEvent::~CMaxFatigueFactorEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CMaxFatigueFactorEvent::~CMaxFatigueFactorEvent() {}

// IDA_EA: 0x81B278C
// IDA_NAME: _ZN22CMaxFatigueFactorEvent10StartEventEv
// DEMANGLED: CMaxFatigueFactorEvent::StartEvent(void)
void CMaxFatigueFactorEvent::StartEvent() {
    SetEventFlag(true);
    max_fatigue_factor_ = 100;
    cMyTrace trace("StartEvent", 22, 0);
    trace("Max Fatigue Factor Event Start!");
}

// IDA_EA: 0x81B27E8
// IDA_NAME: _ZN22CMaxFatigueFactorEvent10StartEventE10Word_Param
// DEMANGLED: CMaxFatigueFactorEvent::StartEvent(Word_Param)
void CMaxFatigueFactorEvent::StartEvent(Word_Param param) {
    SetEventFlag(true);
    max_fatigue_factor_ = static_cast<uint16_t>(param);
    cMyTrace trace("StartEvent", 30, 0);
    trace("Max Fatigue Factor Event Start!");
    CLocalChina_User::SendFatigueEvent();
}

// IDA_EA: 0x81B284C
// IDA_NAME: _ZN22CMaxFatigueFactorEvent8EndEventEv
// DEMANGLED: CMaxFatigueFactorEvent::EndEvent(void)
void CMaxFatigueFactorEvent::EndEvent() {
    SetEventFlag(false);
    max_fatigue_factor_ = 100;
    cMyTrace trace("EndEvent", 42, 0);
    trace("Max Fatigue Factor Event End!");
    CLocalChina_User::SendFatigueEvent();
}

// IDA_EA: 0x81173EA
// IDA_NAME: _ZN18CFatigueAttendanceC2Ev
// DEMANGLED: CFatigueAttendance::CFatigueAttendance(void)
CFatigueAttendance::CFatigueAttendance()
    : CEventBase() {}

// IDA_EA: 0x8117406
// IDA_NAME: _ZN18CFatigueAttendanceD2Ev
// DEMANGLED: CFatigueAttendance::~CFatigueAttendance()
// IDA_EA: 0x8117436
// IDA_NAME: _ZN18CFatigueAttendanceD0Ev
// DEMANGLED: CFatigueAttendance::~CFatigueAttendance() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CFatigueAttendance::~CFatigueAttendance() {}

// IDA_EA: 0x8117454
// IDA_NAME: _ZN18CFatigueAttendance10StartEventEv
// DEMANGLED: CFatigueAttendance::StartEvent(void)
void CFatigueAttendance::StartEvent() {
    SetEventFlag(true);
}

// IDA_EA: 0x8117470
// IDA_NAME: _ZN18CFatigueAttendance10StartEventE10Word_Param
// DEMANGLED: CFatigueAttendance::StartEvent(Word_Param)
void CFatigueAttendance::StartEvent(Word_Param param) {
    add_info_.word_00 = static_cast<int16_t>(param & 0xffffu);
    add_info_.word_02 = static_cast<int16_t>((param >> 16) & 0xffffu);
    StartEvent();
    cMyTrace trace("virtual void CFatigueAttendance::StartEvent(Word_Param)",
                   28,
                   0);
    trace("CFatigueAttendance(AddInfo:%d,%d) Event Start!",
          add_info_.word_00,
          add_info_.word_02);
}

// IDA_EA: 0x81174FE
// IDA_NAME: _ZN18CFatigueAttendance8EndEventEv
// DEMANGLED: CFatigueAttendance::EndEvent(void)
void CFatigueAttendance::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void CFatigueAttendance::EndEvent()", 34, 0);
    trace("CFatigueAttendance Event End!");
}

// IDA_EA: 0x811BBEA
// IDA_NAME: _ZN17CFatigueBuffEventC2Ev
// DEMANGLED: CFatigueBuffEvent::CFatigueBuffEvent(void)
CFatigueBuffEvent::CFatigueBuffEvent()
    : CEventBase() {}

// IDA_EA: 0x811BC06
// IDA_NAME: _ZN17CFatigueBuffEventD2Ev
// DEMANGLED: CFatigueBuffEvent::~CFatigueBuffEvent()
// IDA_EA: 0x811BC36
// IDA_NAME: _ZN17CFatigueBuffEventD0Ev
// DEMANGLED: CFatigueBuffEvent::~CFatigueBuffEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CFatigueBuffEvent::~CFatigueBuffEvent() {}

// IDA_EA: 0x811BC54
// IDA_NAME: _ZN17CFatigueBuffEvent10StartEventEv
// DEMANGLED: CFatigueBuffEvent::StartEvent(void)
void CFatigueBuffEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("virtual void CFatigueBuffEvent::StartEvent()", 17, 0);
    trace("CFatigueBuffEvent Event Start!");
}

// IDA_EA: 0x811BCA6
// IDA_NAME: _ZN17CFatigueBuffEvent10StartEventE10Word_Param
// DEMANGLED: CFatigueBuffEvent::StartEvent(Word_Param)
void CFatigueBuffEvent::StartEvent(Word_Param param) {
    (void)param;
    StartEvent();
}

// IDA_EA: 0x811BCC0
// IDA_NAME: _ZN17CFatigueBuffEvent8EndEventEv
// DEMANGLED: CFatigueBuffEvent::EndEvent(void)
void CFatigueBuffEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void CFatigueBuffEvent::EndEvent()", 27, 0);
    trace("CFatigueBuffEvent Event End!");
}

// IDA_EA: 0x811C838
// IDA_NAME: _ZN21CFatigueGiveItemEventC2Ev
// DEMANGLED: CFatigueGiveItemEvent::CFatigueGiveItemEvent(void)
CFatigueGiveItemEvent::CFatigueGiveItemEvent()
    : CEventBase() {}

// IDA_EA: 0x811C854
// IDA_NAME: _ZN21CFatigueGiveItemEventD2Ev
// DEMANGLED: CFatigueGiveItemEvent::~CFatigueGiveItemEvent()
// IDA_EA: 0x811C884
// IDA_NAME: _ZN21CFatigueGiveItemEventD0Ev
// DEMANGLED: CFatigueGiveItemEvent::~CFatigueGiveItemEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CFatigueGiveItemEvent::~CFatigueGiveItemEvent() {}

// IDA_EA: 0x811C8A2
// IDA_NAME: _ZN21CFatigueGiveItemEvent10StartEventEv
// DEMANGLED: CFatigueGiveItemEvent::StartEvent(void)
void CFatigueGiveItemEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("virtual void CFatigueGiveItemEvent::StartEvent()",
                   18,
                   0);
    trace("CFatigueGiveItemEvent Event Start!");
}

// IDA_EA: 0x811C8F4
// IDA_NAME: _ZN21CFatigueGiveItemEvent10StartEventE10Word_Param
// DEMANGLED: CFatigueGiveItemEvent::StartEvent(Word_Param)
void CFatigueGiveItemEvent::StartEvent(Word_Param param) {
    (void)param;
    StartEvent();
}

// IDA_EA: 0x811C90E
// IDA_NAME: _ZN21CFatigueGiveItemEvent8EndEventEv
// DEMANGLED: CFatigueGiveItemEvent::EndEvent(void)
void CFatigueGiveItemEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void CFatigueGiveItemEvent::EndEvent()", 28, 0);
    trace("CFatigueGiveItemEvent Event End!");
}

// IDA_EA: 0x811CE24
// IDA_NAME: _ZN23CGoldCardBlankItemEventC2Ev
// DEMANGLED: CGoldCardBlankItemEvent::CGoldCardBlankItemEvent(void)
CGoldCardBlankItemEvent::CGoldCardBlankItemEvent()
    : CEventBase() {}

// IDA_EA: 0x811CE40
// IDA_NAME: _ZN23CGoldCardBlankItemEventD2Ev
// DEMANGLED: CGoldCardBlankItemEvent::~CGoldCardBlankItemEvent()
// IDA_EA: 0x811CE70
// IDA_NAME: _ZN23CGoldCardBlankItemEventD0Ev
// DEMANGLED: CGoldCardBlankItemEvent::~CGoldCardBlankItemEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CGoldCardBlankItemEvent::~CGoldCardBlankItemEvent() {}

// IDA_EA: 0x811CE8E
// IDA_NAME: _ZN23CGoldCardBlankItemEvent10StartEventEv
// DEMANGLED: CGoldCardBlankItemEvent::StartEvent(void)
void CGoldCardBlankItemEvent::StartEvent() {}

// IDA_EA: 0x811CE94
// IDA_NAME: _ZN23CGoldCardBlankItemEvent10StartEventE10Word_Param
// DEMANGLED: CGoldCardBlankItemEvent::StartEvent(Word_Param)
void CGoldCardBlankItemEvent::StartEvent(Word_Param param) {
    (void)param;
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 24, 0);
    trace("Gold Card Blank Item Event Start");
}

// IDA_EA: 0x811CEE6
// IDA_NAME: _ZN23CGoldCardBlankItemEvent8EndEventEv
// DEMANGLED: CGoldCardBlankItemEvent::EndEvent(void)
void CGoldCardBlankItemEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("EndEvent", 30, 0);
    trace("Gold Card Blank Item Event End");
}

// IDA_EA: 0x81B291C
// IDA_NAME: _ZN21CMemberExpRewardEventC2Ev
// DEMANGLED: CMemberExpRewardEvent::CMemberExpRewardEvent(void)
CMemberExpRewardEvent::CMemberExpRewardEvent()
    : CEventBase(),
      member_exp_reward_ratio_(10) {}

// IDA_EA: 0x81B2942
// IDA_NAME: _ZN21CMemberExpRewardEventD2Ev
// DEMANGLED: CMemberExpRewardEvent::~CMemberExpRewardEvent()
// IDA_EA: 0x81B2972
// IDA_NAME: _ZN21CMemberExpRewardEventD0Ev
// DEMANGLED: CMemberExpRewardEvent::~CMemberExpRewardEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CMemberExpRewardEvent::~CMemberExpRewardEvent() {}

// IDA_EA: 0x81B2990
// IDA_NAME: _ZN21CMemberExpRewardEvent10StartEventEv
// DEMANGLED: CMemberExpRewardEvent::StartEvent(void)
void CMemberExpRewardEvent::StartEvent() {
    SetEventFlag(true);
    member_exp_reward_ratio_ = 10;
    cMyTrace trace("StartEvent", 22, 0);
    trace("Member Exp Reward Event Start!");
}

// IDA_EA: 0x81B29EC
// IDA_NAME: _ZN21CMemberExpRewardEvent10StartEventE10Word_Param
// DEMANGLED: CMemberExpRewardEvent::StartEvent(Word_Param)
void CMemberExpRewardEvent::StartEvent(Word_Param param) {
    const uint16_t ratio = static_cast<uint16_t>(param);
    SetEventFlag(true);
    member_exp_reward_ratio_ = ratio;
    cMyTrace trace("StartEvent", 29, 0);
    trace("Member Exp Reward Event! ratio(%d)", ratio);
}

// IDA_EA: 0x81B2A5A
// IDA_NAME: _ZN21CMemberExpRewardEvent8EndEventEv
// DEMANGLED: CMemberExpRewardEvent::EndEvent(void)
void CMemberExpRewardEvent::EndEvent() {
    SetEventFlag(false);
    member_exp_reward_ratio_ = 10;
    cMyTrace trace("EndEvent", 36, 0);
    trace("Member Exp Reward Event");
}

// IDA_EA: 0x81B2AB6
// IDA_NAME: _ZN21CMemberExpRewardEvent23GetMemberExpRewardRatioEv
// DEMANGLED: CMemberExpRewardEvent::GetMemberExpRewardRatio(void)
int CMemberExpRewardEvent::GetMemberExpRewardRatio() {
    return member_exp_reward_ratio_;
}

// IDA_EA: 0x81B9B8C
// IDA_NAME: _ZN29CNoNeedGoldOnGuildCreateEventC2Ev
// DEMANGLED: CNoNeedGoldOnGuildCreateEvent::CNoNeedGoldOnGuildCreateEvent(void)
CNoNeedGoldOnGuildCreateEvent::CNoNeedGoldOnGuildCreateEvent()
    : CEventBase() {}

// IDA_EA: 0x81B9BA8
// IDA_NAME: _ZN29CNoNeedGoldOnGuildCreateEventD2Ev
// DEMANGLED: CNoNeedGoldOnGuildCreateEvent::~CNoNeedGoldOnGuildCreateEvent()
// IDA_EA: 0x81B9BD8
// IDA_NAME: _ZN29CNoNeedGoldOnGuildCreateEventD0Ev
// DEMANGLED: CNoNeedGoldOnGuildCreateEvent::~CNoNeedGoldOnGuildCreateEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CNoNeedGoldOnGuildCreateEvent::~CNoNeedGoldOnGuildCreateEvent() {}

// IDA_EA: 0x81B9BF6
// IDA_NAME: _ZN29CNoNeedGoldOnGuildCreateEvent10StartEventEv
// DEMANGLED: CNoNeedGoldOnGuildCreateEvent::StartEvent(void)
void CNoNeedGoldOnGuildCreateEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 18, 0);
    trace("No Need Money On Guild Create Event Start!");
}

// IDA_EA: 0x81B9C48
// IDA_NAME: _ZN29CNoNeedGoldOnGuildCreateEvent10StartEventE10Word_Param
// DEMANGLED: CNoNeedGoldOnGuildCreateEvent::StartEvent(Word_Param)
void CNoNeedGoldOnGuildCreateEvent::StartEvent(Word_Param param) {
    (void)param;
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 24, 0);
    trace("No Need Money On Guild Create Event Start!");
}

// IDA_EA: 0x81B9C9A
// IDA_NAME: _ZN29CNoNeedGoldOnGuildCreateEvent8EndEventEv
// DEMANGLED: CNoNeedGoldOnGuildCreateEvent::EndEvent(void)
void CNoNeedGoldOnGuildCreateEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("EndEvent", 30, 0);
    trace("No Need Money On Guild Create Event End!");
}

// IDA_EA: 0x826E92C
// IDA_NAME: _ZN21CReformingDanjinEventC2Ev
// DEMANGLED: CReformingDanjinEvent::CReformingDanjinEvent(void)
CReformingDanjinEvent::CReformingDanjinEvent()
    : CEventBase() {}

// IDA_EA: 0x826E948
// IDA_NAME: _ZN21CReformingDanjinEventD2Ev
// DEMANGLED: CReformingDanjinEvent::~CReformingDanjinEvent()
// IDA_EA: 0x826E978
// IDA_NAME: _ZN21CReformingDanjinEventD0Ev
// DEMANGLED: CReformingDanjinEvent::~CReformingDanjinEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CReformingDanjinEvent::~CReformingDanjinEvent() {}

// IDA_EA: 0x826E996
// IDA_NAME: _ZN21CReformingDanjinEvent10StartEventEv
// DEMANGLED: CReformingDanjinEvent::StartEvent(void)
void CReformingDanjinEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 19, 0);
    trace("Reforming Danjin Event Start!");
}

// IDA_EA: 0x826E9E8
// IDA_NAME: _ZN21CReformingDanjinEvent10StartEventE10Word_Param
// DEMANGLED: CReformingDanjinEvent::StartEvent(Word_Param)
void CReformingDanjinEvent::StartEvent(Word_Param param) {
    (void)param;
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 25, 0);
    trace("Reforming Danjin Event Start!");
}

// IDA_EA: 0x826EA3A
// IDA_NAME: _ZN21CReformingDanjinEvent8EndEventEv
// DEMANGLED: CReformingDanjinEvent::EndEvent(void)
void CReformingDanjinEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("EndEvent", 31, 0);
    trace("Reforming Danjin Event End!");
}

// IDA_EA: 0x827F508
// IDA_NAME: _ZN17CStabToDeathEventC2Ev
// DEMANGLED: CStabToDeathEvent::CStabToDeathEvent(void)
CStabToDeathEvent::CStabToDeathEvent()
    : CEventBase() {}

// IDA_EA: 0x827F524
// IDA_NAME: _ZN17CStabToDeathEventD2Ev
// DEMANGLED: CStabToDeathEvent::~CStabToDeathEvent()
// IDA_EA: 0x827F554
// IDA_NAME: _ZN17CStabToDeathEventD0Ev
// DEMANGLED: CStabToDeathEvent::~CStabToDeathEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CStabToDeathEvent::~CStabToDeathEvent() {}

// IDA_EA: 0x827F572
// IDA_NAME: _ZN17CStabToDeathEvent10StartEventEv
// DEMANGLED: CStabToDeathEvent::StartEvent(void)
void CStabToDeathEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 17, 0);
    trace("CStabToDeathEvent Start!");
}

// IDA_EA: 0x827F5C4
// IDA_NAME: _ZN17CStabToDeathEvent10StartEventE10Word_Param
// DEMANGLED: CStabToDeathEvent::StartEvent(Word_Param)
void CStabToDeathEvent::StartEvent(Word_Param param) {
    (void)param;
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 23, 0);
    trace("CStabToDeathEvent Start!");
}

// IDA_EA: 0x827F616
// IDA_NAME: _ZN17CStabToDeathEvent8EndEventEv
// DEMANGLED: CStabToDeathEvent::EndEvent(void)
void CStabToDeathEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("EndEvent", 29, 0);
    trace("CStabToDeathEvent End!");
}

// IDA_EA: 0x81177A0
// IDA_NAME: _ZN20CStopOverlabExpEventC2Ev
// DEMANGLED: CStopOverlabExpEvent::CStopOverlabExpEvent(void)
CStopOverlabExpEvent::CStopOverlabExpEvent()
    : CEventBase() {}

// IDA_EA: 0x81177BC
// IDA_NAME: _ZN20CStopOverlabExpEventD2Ev
// DEMANGLED: CStopOverlabExpEvent::~CStopOverlabExpEvent()
// IDA_EA: 0x81177EC
// IDA_NAME: _ZN20CStopOverlabExpEventD0Ev
// DEMANGLED: CStopOverlabExpEvent::~CStopOverlabExpEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CStopOverlabExpEvent::~CStopOverlabExpEvent() {}

// IDA_EA: 0x811780A
// IDA_NAME: _ZN20CStopOverlabExpEvent10StartEventEv
// DEMANGLED: CStopOverlabExpEvent::StartEvent(void)
void CStopOverlabExpEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("virtual void CStopOverlabExpEvent::StartEvent()", 21, 0);
    trace("CStopOverlabExpEvent Event Start!");
}

// IDA_EA: 0x811785C
// IDA_NAME: _ZN20CStopOverlabExpEvent10StartEventE10Word_Param
// DEMANGLED: CStopOverlabExpEvent::StartEvent(Word_Param)
void CStopOverlabExpEvent::StartEvent(Word_Param param) {
    (void)param;
    StartEvent();
}

// IDA_EA: 0x8117876
// IDA_NAME: _ZN20CStopOverlabExpEvent8EndEventEv
// DEMANGLED: CStopOverlabExpEvent::EndEvent(void)
void CStopOverlabExpEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void CStopOverlabExpEvent::EndEvent()", 31, 0);
    trace("CStopOverlabExpEvent Event End!");
}

// IDA_EA: 0x8117678
// IDA_NAME: _ZN15CUXGameLogEventC2Ev
// DEMANGLED: CUXGameLogEvent::CUXGameLogEvent(void)
CUXGameLogEvent::CUXGameLogEvent()
    : CEventBase() {}

// IDA_EA: 0x8117694
// IDA_NAME: _ZN15CUXGameLogEventD2Ev
// DEMANGLED: CUXGameLogEvent::~CUXGameLogEvent()
// IDA_EA: 0x81176C4
// IDA_NAME: _ZN15CUXGameLogEventD0Ev
// DEMANGLED: CUXGameLogEvent::~CUXGameLogEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CUXGameLogEvent::~CUXGameLogEvent() {}

// IDA_EA: 0x81176E2
// IDA_NAME: _ZN15CUXGameLogEvent10StartEventEv
// DEMANGLED: CUXGameLogEvent::StartEvent(void)
void CUXGameLogEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("virtual void CUXGameLogEvent::StartEvent()", 19, 0);
    trace("CUXGameLogEvent Event Start!");
}

// IDA_EA: 0x8117734
// IDA_NAME: _ZN15CUXGameLogEvent10StartEventE10Word_Param
// DEMANGLED: CUXGameLogEvent::StartEvent(Word_Param)
void CUXGameLogEvent::StartEvent(Word_Param param) {
    (void)param;
    StartEvent();
}

// IDA_EA: 0x811774E
// IDA_NAME: _ZN15CUXGameLogEvent8EndEventEv
// DEMANGLED: CUXGameLogEvent::EndEvent(void)
void CUXGameLogEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void CUXGameLogEvent::EndEvent()", 30, 0);
    trace("CUXGameLogEvent Event End!");
}

// IDA_EA: 0x8116DBE
// IDA_NAME: _ZN25CVendingMachineBonusEventC2Ev
// DEMANGLED: CVendingMachineBonusEvent::CVendingMachineBonusEvent(void)
CVendingMachineBonusEvent::CVendingMachineBonusEvent()
    : CEventBase() {}

// IDA_EA: 0x8116DDA
// IDA_NAME: _ZN25CVendingMachineBonusEventD2Ev
// DEMANGLED: CVendingMachineBonusEvent::~CVendingMachineBonusEvent()
// IDA_EA: 0x8116E0A
// IDA_NAME: _ZN25CVendingMachineBonusEventD0Ev
// DEMANGLED: CVendingMachineBonusEvent::~CVendingMachineBonusEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CVendingMachineBonusEvent::~CVendingMachineBonusEvent() {}

// IDA_EA: 0x8116E28
// IDA_NAME: _ZN25CVendingMachineBonusEvent10StartEventEv
// DEMANGLED: CVendingMachineBonusEvent::StartEvent(void)
void CVendingMachineBonusEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("virtual void CVendingMachineBonusEvent::StartEvent()",
                   18,
                   0);
    trace("CVendingMachineBonus Event Start!");
}

// IDA_EA: 0x8116E7A
// IDA_NAME: _ZN25CVendingMachineBonusEvent10StartEventE10Word_Param
// DEMANGLED: CVendingMachineBonusEvent::StartEvent(Word_Param)
void CVendingMachineBonusEvent::StartEvent(Word_Param param) {
    (void)param;
    StartEvent();
}

// IDA_EA: 0x8116E94
// IDA_NAME: _ZN25CVendingMachineBonusEvent8EndEventEv
// DEMANGLED: CVendingMachineBonusEvent::EndEvent(void)
void CVendingMachineBonusEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void CVendingMachineBonusEvent::EndEvent()",
                   28,
                   0);
    trace("CVendingMachineBonus Event End!");
}

// IDA_EA: 0x8117550
// IDA_NAME: _ZN18CWeekendBonusEventC2Ev
// DEMANGLED: CWeekendBonusEvent::CWeekendBonusEvent(void)
CWeekendBonusEvent::CWeekendBonusEvent()
    : CEventBase() {}

// IDA_EA: 0x811756C
// IDA_NAME: _ZN18CWeekendBonusEventD2Ev
// DEMANGLED: CWeekendBonusEvent::~CWeekendBonusEvent()
// IDA_EA: 0x811759C
// IDA_NAME: _ZN18CWeekendBonusEventD0Ev
// DEMANGLED: CWeekendBonusEvent::~CWeekendBonusEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CWeekendBonusEvent::~CWeekendBonusEvent() {}

// IDA_EA: 0x81175BA
// IDA_NAME: _ZN18CWeekendBonusEvent10StartEventEv
// DEMANGLED: CWeekendBonusEvent::StartEvent(void)
void CWeekendBonusEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("virtual void CWeekendBonusEvent::StartEvent()", 18, 0);
    trace("CWeekendBonusEvent Event Start!");
}

// IDA_EA: 0x811760C
// IDA_NAME: _ZN18CWeekendBonusEvent10StartEventE10Word_Param
// DEMANGLED: CWeekendBonusEvent::StartEvent(Word_Param)
void CWeekendBonusEvent::StartEvent(Word_Param param) {
    (void)param;
    StartEvent();
}

// IDA_EA: 0x8117626
// IDA_NAME: _ZN18CWeekendBonusEvent8EndEventEv
// DEMANGLED: CWeekendBonusEvent::EndEvent(void)
void CWeekendBonusEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void CWeekendBonusEvent::EndEvent()", 28, 0);
    trace("CWeekendBonusEvent Event End!");
}

// IDA_EA: 0x811CFA8
// IDA_NAME: _ZN14CGuildWarEventC2Ev
// DEMANGLED: CGuildWarEvent::CGuildWarEvent(void)
CGuildWarEvent::CGuildWarEvent()
    : CEventBase(),
      server_no_(0),
      reserved_0e_(0) {
    reserved_09_[1] = 5;
    reserved_09_[2] = 0;
}

// IDA_EA: 0x811CFD6
// IDA_NAME: _ZN14CGuildWarEventD2Ev
// DEMANGLED: CGuildWarEvent::~CGuildWarEvent()
// IDA_EA: 0x811D006
// IDA_NAME: _ZN14CGuildWarEventD0Ev
// DEMANGLED: CGuildWarEvent::~CGuildWarEvent() deleting destructor
CGuildWarEvent::~CGuildWarEvent() {}

// IDA_EA: 0x811D024
// IDA_NAME: _ZN14CGuildWarEvent10StartEventEv
// DEMANGLED: CGuildWarEvent::StartEvent(void)
void CGuildWarEvent::StartEvent() {
    if (!IsEventing(0)) {
        SetEventFlag(true);
        reserved_09_[1] = 5;
        reserved_09_[2] = 0;
        server_no_ = 0;
        G_TimerQueue()->InsertTimer(2, 0, 98, 3600 * 5 - 600, 10, 0);
        cMyTrace trace("virtual void CGuildWarEvent::StartEvent()", 39, 0);
        trace("Guild War Event Start! time(%d)", 5);
    }
}

// IDA_EA: 0x811D152
// IDA_NAME: _ZN14CGuildWarEvent10StartEventE10Word_Param
// DEMANGLED: CGuildWarEvent::StartEvent(Word_Param)
void CGuildWarEvent::StartEvent(Word_Param param) {
    if (!IsEventing(0)) {
        SetEventFlag(true);
        reserved_09_[1] = static_cast<uint8_t>(param & 0xffu);
        reserved_09_[2] = static_cast<uint8_t>((param >> 8) & 0xffu);
        server_no_ = static_cast<uint16_t>((param >> 16) & 0xffffu);
        const int event_time =
            static_cast<int>(static_cast<uint16_t>(param & 0xffffu));
        G_TimerQueue()->InsertTimer(2,
                                    0,
                                    98,
                                    3600 * event_time - 600,
                                    10,
                                    0);
        cMyTrace trace(
            "virtual void CGuildWarEvent::StartEvent(Word_Param)", 57, 0);
        trace("Guild War Event Start! time(%d), server(%d)",
              static_cast<int>(param),
              G_CEnvironment()->get_server_group());
    }
}

// IDA_EA: 0x811D28C
// IDA_NAME: _ZN14CGuildWarEvent8EndEventEv
// DEMANGLED: CGuildWarEvent::EndEvent(void)
void CGuildWarEvent::EndEvent() {
    if (IsEventing(0)) {
        SetEventFlag(false);
        reserved_09_[1] = 0;
        reserved_09_[2] = 0;
        server_no_ = 0;
        cMyTrace trace("virtual void CGuildWarEvent::EndEvent()", 73, 0);
        trace("Guild War Event End");
    }
}

// IDA_EA: 0x8267990
// IDA_NAME: _ZN14CPowerWarEventC2Ev
// DEMANGLED: CPowerWarEvent::CPowerWarEvent(void)
CPowerWarEvent::CPowerWarEvent()
    : CEventBase(),
      raw_param_(0) {}

// IDA_EA: 0x82679AC
// IDA_NAME: _ZN14CPowerWarEventD2Ev
// DEMANGLED: CPowerWarEvent::~CPowerWarEvent()
// IDA_EA: 0x82679DC
// IDA_NAME: _ZN14CPowerWarEventD0Ev
// DEMANGLED: CPowerWarEvent::~CPowerWarEvent() deleting destructor
CPowerWarEvent::~CPowerWarEvent() {}

// IDA_EA: 0x82679FA
// IDA_NAME: _ZN14CPowerWarEvent10StartEventEv
// DEMANGLED: CPowerWarEvent::StartEvent(void)
void CPowerWarEvent::StartEvent() {
    if (IsEventing(0) != 1) {
        SetEventFlag(true);
        const int server_no = G_CEnvironment()->get_channel_no();
        cMyTrace trace("virtual void CPowerWarEvent::StartEvent()", 112, 0);
        trace("Power War Event Start! server(%d)", server_no);
    }
}

// IDA_EA: 0x8267DEA
// IDA_NAME: _ZN14CPowerWarEvent10StartEventE10Word_Param
// DEMANGLED: CPowerWarEvent::StartEvent(Word_Param)
void CPowerWarEvent::StartEvent(Word_Param param) {
    if (IsEventing(0) != 1) {
        SetEventFlag(true);
        raw_param_ = param;
        const int server_no = G_CEnvironment()->get_channel_no();
        cMyTrace trace(
            "virtual void CPowerWarEvent::StartEvent(Word_Param)", 198, 0);
        trace("Power War Event Start! server(%d)", server_no);
    }
}

// IDA_EA: 0x82681E2
// IDA_NAME: _ZN14CPowerWarEvent8EndEventEv
// DEMANGLED: CPowerWarEvent::EndEvent(void)
void CPowerWarEvent::EndEvent() {
    if (IsEventing(0)) {
        SetEventFlag(false);
        raw_param_ = 0;
        CleanupPowerWarServerMessage();
        cMyTrace trace("virtual void CPowerWarEvent::EndEvent()", 272, 0);
        trace("Power War Event End");
    }
}

// IDA_EA: 0x8268492
// IDA_NAME: _ZN14CPowerWarEvent28CleanupPowerWarServerMessageEv
// DEMANGLED: CPowerWarEvent::CleanupPowerWarServerMessage(void)
void CPowerWarEvent::CleanupPowerWarServerMessage() {}

// IDA_EA: 0x8273ECC
// IDA_NAME: _ZN17CSchoolMatchEventC2Ev
// DEMANGLED: CSchoolMatchEvent::CSchoolMatchEvent(void)
CSchoolMatchEvent::CSchoolMatchEvent()
    : CEventBase(),
      channels_() {}

// IDA_EA: 0x8273F1A
// IDA_NAME: _ZN17CSchoolMatchEventD2Ev
// DEMANGLED: CSchoolMatchEvent::~CSchoolMatchEvent()
// IDA_EA: 0x8273F7C
// IDA_NAME: _ZN17CSchoolMatchEventD0Ev
// DEMANGLED: CSchoolMatchEvent::~CSchoolMatchEvent() deleting destructor
CSchoolMatchEvent::~CSchoolMatchEvent() {}

// IDA_EA: 0x8273F9A
// IDA_NAME: _ZN17CSchoolMatchEvent10StartEventEv
// DEMANGLED: CSchoolMatchEvent::StartEvent(void)
void CSchoolMatchEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("virtual void CSchoolMatchEvent::StartEvent()", 21, 0);
    trace("CSchoolMatchEvent Start!");
}

// IDA_EA: 0x8273FEC
// IDA_NAME: _ZN17CSchoolMatchEvent10StartEventE10Word_Param
// DEMANGLED: CSchoolMatchEvent::StartEvent(Word_Param)
void CSchoolMatchEvent::StartEvent(Word_Param param) {
    SetEventFlag(true);
    const uint32_t raw_param = param.value;
    const int channel_no = static_cast<int>(raw_param & 0xffu);
    const int player_count = static_cast<int>((raw_param >> 8) & 0xffu);
    const int school_rating = static_cast<int>((raw_param >> 16) & 0xffu);
    const int player_rating = static_cast<int>((raw_param >> 24) & 0xffu);
    cMyTrace trace(
        "virtual void CSchoolMatchEvent::StartEvent(Word_Param)", 30, 0);
    trace("CSchoolMatchEvent Start! channel_no(%d), player_count(%d), school_rating(%d), player_rating(%d)",
          channel_no,
          player_count,
          school_rating,
          player_rating);

    GameWorld* world = G_GameWorld();
    if (world->GetChannelType() == 9 ||
        G_GameWorld()->GetChannelType() == 10) {
        CEnvironment* environment = G_CEnvironment();
        if (environment->get_channel_no() == channel_no) {
            CLeagueMatch::SetMatchAble(true);
            CLeagueMatch::SetPlayCountPerOneTeam(
                static_cast<uint32_t>(player_count));
            PvPGlobalEnvironmentVariable::school_match_elo_k2_ = school_rating;
            PvPGlobalEnvironmentVariable::school_match_elo_k_ = player_rating;
            if (G_CEnvironment()->get_channel_no() == 71) {
                CLeagueMatch::SetBattleMode(2);
            }
            cMyTrace match_trace(
                "virtual void CSchoolMatchEvent::StartEvent(Word_Param)",
                48,
                0);
            match_trace("This is School Matching Server!!");
        }
    }

    if (std::find(channels_.begin(), channels_.end(), channel_no) ==
        channels_.end()) {
        channels_.push_back(channel_no);
    }
}

// IDA_EA: 0x827423C
// IDA_NAME: _ZN17CSchoolMatchEvent8EndEventEv
// DEMANGLED: CSchoolMatchEvent::EndEvent(void)
void CSchoolMatchEvent::EndEvent() {
    SetEventFlag(false);
    CLeagueMatch::SetMatchAble(false);
    cMyTrace trace("virtual void CSchoolMatchEvent::EndEvent()", 63, 0);
    trace("CSchoolMatchEvent End!");
    channels_.clear();
}

// IDA_EA: 0x82742A8
// IDA_NAME: _ZN17CSchoolMatchEvent10AppendInfoER11PacketGuard
// DEMANGLED: CSchoolMatchEvent::AppendInfo(PacketGuard &)
void CSchoolMatchEvent::AppendInfo(PacketGuard& guard) {
    guard.put_byte(static_cast<int>(channels_.size()));
    for (std::vector<int>::const_iterator it = channels_.begin();
         it != channels_.end();
         ++it) {
        guard.put_short(*it);
    }
}

// IDA_EA: 0x8163AA4
// IDA_NAME: _ZN15CEventCreateDnfC2Ev
// DEMANGLED: CEventCreateDnf::CEventCreateDnf(void)
CEventCreateDnf::CEventCreateDnf()
    : CEventBase(),
      state_(State::kWaiting),
      start_hour_(0),
      end_hour_(0),
      reload_interval_(0),
      reward_day_(0),
      reserved_15_(0),
      reward_rate_(0) {}

// IDA_EA: 0x8163AE8
// IDA_NAME: _ZN15CEventCreateDnfD2Ev
// DEMANGLED: CEventCreateDnf::~CEventCreateDnf()
// IDA_EA: 0x8163B18
// IDA_NAME: _ZN15CEventCreateDnfD0Ev
// DEMANGLED: CEventCreateDnf::~CEventCreateDnf() deleting destructor
CEventCreateDnf::~CEventCreateDnf() {}

// IDA_EA: 0x8163B36
// IDA_NAME: _ZN15CEventCreateDnf10StartEventEv
// DEMANGLED: CEventCreateDnf::StartEvent(void)
void CEventCreateDnf::StartEvent() {
    SetEventFlag(true);
    setState(State::kWaiting);
    RegistStartTimer(true);
    cMyTrace trace("virtual void CEventCreateDnf::StartEvent()", 29, 0);
    trace("[Taiwan, CreateDnfEvent] Start! startTime:%2dh, endTime:%2dh, reLoadInterval:%d",
          start_hour_,
          end_hour_,
          reload_interval_);
}

// IDA_EA: 0x8163BE0
// IDA_NAME: _ZN15CEventCreateDnf10StartEventE10Word_Param
// DEMANGLED: CEventCreateDnf::StartEvent(Word_Param)
void CEventCreateDnf::StartEvent(Word_Param param) {
    start_hour_ = static_cast<uint8_t>(param & 0xffu);
    end_hour_ = static_cast<uint8_t>((param >> 8) & 0xffu);
    reload_interval_ =
        static_cast<uint16_t>(60u * static_cast<uint8_t>((param >> 16) & 0xffu));

    if (start_hour_ <= 0x17u && end_hour_ <= 0x17u &&
        reload_interval_ > 0x3bu && reload_interval_ <= 0x0e10u) {
        StartEvent();
        return;
    }

    LogManager::logFormat(
        1,
        "localtaiwan/Event/EventCreateDnf.cpp",
        "virtual void CEventCreateDnf::StartEvent(Word_Param)",
        0x31,
        "[Taiwan, CreateDnfEvent] Error! startTime:%2dh, endTime:%2dh, reLoadInterval:%d",
        start_hour_,
        end_hour_,
        reload_interval_);
    EndEvent();
}

// IDA_EA: 0x8163CD0
// IDA_NAME: _ZN15CEventCreateDnf8EndEventEv
// DEMANGLED: CEventCreateDnf::EndEvent(void)
void CEventCreateDnf::EndEvent() {
    cMyTrace trace("virtual void CEventCreateDnf::EndEvent()", 56, 0);
    trace("[Taiwan, CreateDnfEvent] End Event.");
    start_hour_ = 0;
    end_hour_ = 0;
    reload_interval_ = 0;
    setState(State::kWaiting);
    SetEventFlag(false);
}

// IDA_EA: 0x8163D4C
// IDA_NAME: _ZN15CEventCreateDnf15RegistNextTimerEv
// DEMANGLED: CEventCreateDnf::RegistNextTimer(void)
int CEventCreateDnf::RegistNextTimer() {
    if (!IsEventing(0)) {
        return 1;
    }

    time_t now = static_cast<time_t>(GlobalData::system_time()->getCurSec());
    struct tm local_time;
    localtime_r(&now, &local_time);
    local_time.tm_hour = end_hour_;
    local_time.tm_min = 0;
    local_time.tm_sec = 0;
    time_t end_time = mktime(&local_time);
    if (end_time >= now) {
        setState(State::kReward);
    }

    G_TimerQueue()->InsertTimer(2, 0, 174, reload_interval_, 0, 0);
    const int next_time =
        GlobalData::system_time()->getCurSec() + reload_interval_;
    cMyTrace trace("void CEventCreateDnf::RegistNextTimer()", 85, 0);
    return trace("[Taiwan, CreateDnfEvent] next db load time:%d", next_time);
}

// IDA_EA: 0x8163E7C
// IDA_NAME: _ZN15CEventCreateDnf16RegistStartTimerEb
// DEMANGLED: CEventCreateDnf::RegistStartTimer(bool)
int CEventCreateDnf::RegistStartTimer(bool request_now) {
    (void)request_now;
    if (!IsEventing(0)) {
        return 1;
    }

    SetEventInfo(0, 0, 0);
    time_t now = static_cast<time_t>(GlobalData::system_time()->getCurSec());
    struct tm local_time;
    localtime_r(&now, &local_time);
    local_time.tm_hour = start_hour_;
    local_time.tm_min = 0;
    local_time.tm_sec = 0;
    time_t start_time = mktime(&local_time);
    if (start_time < now) {
        DBSelectCreateDnfEventInfo::makeRequest();
        setState(State::kIng);
        time_t next_time = start_time + reload_interval_;
        while (next_time < now) {
            next_time += reload_interval_;
        }
        const int delay = static_cast<int>(next_time - now);
        G_TimerQueue()->InsertTimer(2, 0, 174, delay, 0, 0);
        cMyTrace trace("void CEventCreateDnf::RegistStartTimer(bool)",
                       126,
                       0);
        return trace("[Taiwan, CreateDnfEvent] next db load time:%d",
                     static_cast<int>(next_time));
    }

    setState(State::kWaiting);
    const int delay = static_cast<int>(start_time - now);
    G_TimerQueue()->InsertTimer(2, 0, 174, delay, 0, 0);
    cMyTrace trace("void CEventCreateDnf::RegistStartTimer(bool)", 110, 0);
    return trace("[Taiwan, CreateDnfEvent] next db load time:%d",
                 static_cast<int>(start_time));
}

// IDA_EA: 0x8164092
// IDA_NAME: _ZN15CEventCreateDnf17IsRewardConditionEv
// DEMANGLED: CEventCreateDnf::IsRewardCondition(void)
int CEventCreateDnf::IsRewardCondition() {
    return GetState() == State::kReward && reward_rate_ == 100;
}

// IDA_EA: 0x81640D4
// IDA_NAME: _ZN15CEventCreateDnf12SetEventInfoEiii
// DEMANGLED: CEventCreateDnf::SetEventInfo(int,int,int)
CEventCreateDnf* CEventCreateDnf::SetEventInfo(int hour,
                                               int rate,
                                               int reward_day) {
    if (GetState() == State::kReward && start_hour_ > hour && end_hour_ <= hour) {
        cMyTrace trace("void CEventCreateDnf::SetEventInfo(int, int, int)",
                       142,
                       0);
        trace("[Taiwan, CreateDnfEvent] State:kReward rate:%d",
              reward_rate_);
        return this;
    }

    if (start_hour_ > hour || end_hour_ <= hour) {
        setState(State::kReward);
        cMyTrace trace("void CEventCreateDnf::SetEventInfo(int, int, int)",
                       154,
                       0);
        trace("[Taiwan, CreateDnfEvent] State:kReward rate:%d", rate);
    } else {
        setState(State::kIng);
        cMyTrace trace("void CEventCreateDnf::SetEventInfo(int, int, int)",
                       149,
                       0);
        trace("[Taiwan, CreateDnfEvent] State:kIng rate:%d", rate);
    }

    reward_rate_ = static_cast<uint16_t>(rate);
    if (reward_day != 0) {
        reward_day_ = static_cast<uint8_t>(reward_day);
    }
    return this;
}

// IDA_EA: 0x8164246
// IDA_NAME: _ZN15CEventCreateDnf8GetStateEv
// DEMANGLED: CEventCreateDnf::GetState(void)
int CEventCreateDnf::GetState() {
    return state_;
}

// IDA_EA: 0x8164252
// IDA_NAME: _ZN15CEventCreateDnf8setStateENS_5State1TE
// DEMANGLED: CEventCreateDnf::setState(CEventCreateDnf::State::T)
CEventCreateDnf* CEventCreateDnf::setState(State::T state) {
    state_ = state;
    return this;
}

// IDA_EA: 0x8164260
// IDA_NAME: _ZN15CEventCreateDnf7GetRateEv
// DEMANGLED: CEventCreateDnf::GetRate(void)
int CEventCreateDnf::GetRate() {
    return reward_rate_;
}

// IDA_EA: 0x816426C
// IDA_NAME: _ZN15CEventCreateDnf10SendRewardEP5CUser
// DEMANGLED: CEventCreateDnf::SendReward(CUser *)
void CEventCreateDnf::SendReward(CUser* user) {
    std::vector<std::pair<int, int> > rewards;
    CreateDnfEventSciprtServer* server =
        ARAD::Singleton<CreateDnfEventSciprtServer>::Get();
    server->script.GetItemInfo(rewards, reward_day_);

    const char* title = g_scriptStringManager_.findString(
        4,
        "tw_cb_launching_title_01",
        0);
    char title_buffer[30] = {};
    std::strncpy(title_buffer, title, 0x1du);
    const char* text = g_scriptStringManager_.findString(
        4,
        "game_server_msg_81",
        0);
    char text_buffer[256] = {};
    std::strncpy(text_buffer, text, 0xffu);

    Inven_Item mail_items[10];
    WongWork::CMailBoxHelper::MakeSystemMultiMailPostal(
        rewards,
        mail_items,
        10);
    const int server_group = user->GetServerGroup();
    const int charac_no = user->getCurCharacNo();
    WongWork::CMailBoxHelper::ReqDBSendNewSystemMultiMail(
        title_buffer,
        mail_items,
        static_cast<unsigned int>(rewards.size()),
        0,
        static_cast<unsigned int>(charac_no),
        text_buffer,
        static_cast<int>(std::strlen(text_buffer)),
        0,
        static_cast<ENUM_SERVER_GROUP>(server_group),
        false);

    PacketGuard guard;
    guard.put_header(1, ENUM_CMDPACKET_EVENT_CREATE_DNF_REQUEST);
    guard.put_byte(1);
    guard.put_byte(static_cast<int>(rewards.size()));
    for (std::vector<std::pair<int, int> >::const_iterator it =
             rewards.begin();
         it != rewards.end();
         ++it) {
        guard.put_int(it->first);
        guard.put_byte(it->second);
    }
    guard.finalize(true);
    user->Send(guard);
}



// IDA_EA: 0x8160C4A
// IDA_NAME: _ZN29InterSelectCreateDnfEventInfo6RewardC2Eib
// DEMANGLED: InterSelectCreateDnfEventInfo::Reward::Reward(int,bool)
InterSelectCreateDnfEventInfo::Reward::Reward(int reward_rate,
                                              bool can_reward)
    : rate(static_cast<uint16_t>(reward_rate)),
      rewardable(can_reward ? 1 : 0),
      reserved_03(0) {}

// IDA_EA: 0x8160C6E
// IDA_NAME: _ZN29InterSelectCreateDnfEventInfo6RewardclEP5CUser
// DEMANGLED: InterSelectCreateDnfEventInfo::Reward::operator()(CUser *)
void InterSelectCreateDnfEventInfo::Reward::operator()(CUser* user) {
    PacketGuard guard;
    guard.put_header(0, ENUM_NOTIPACKET_EVENT_CREATE_DNF);
    guard.put_short(rate);
    if (user->GetEventCreateDnfReward() != 1 && rewardable) {
        guard.put_byte(1);
    } else {
        guard.put_byte(0);
    }
    guard.finalize(true);
    user->Send(guard);
}

// IDA_EA: 0x8160D5E
// IDA_NAME: _ZN29InterSelectCreateDnfEventInfo12dispatch_sigEP5CUserPci
// DEMANGLED: InterSelectCreateDnfEventInfo::dispatch_sig(CUser *,char *,int)
int InterSelectCreateDnfEventInfo::dispatch_sig(CUser* user,
                                                char* msg,
                                                int length) {
    (void)user;
    (void)length;
    if (!msg) {
        LogManager::logFormat(
            1,
            "localtaiwan/Dispatcher/Taiwan_InterDispatcher.cpp",
            "virtual int InterSelectCreateDnfEventInfo::dispatch_sig(CUser*, char*, int)",
            253,
            "[Taiwan, Event] interMsg is null.");
        return 254;
    }

    CEventCreateDnf* repeat_event =
        GlobalData::s_event_manager
            ? static_cast<CEventCreateDnf*>(
                  GlobalData::s_event_manager->GetRepeatEvent(160))
            : 0;
    const int eventing = repeat_event ? repeat_event->IsEventing(0) : 0;
    if (!repeat_event || eventing != 1) {
        LogManager::logFormat(
            1,
            "localtaiwan/Dispatcher/Taiwan_InterDispatcher.cpp",
            "virtual int InterSelectCreateDnfEventInfo::dispatch_sig(CUser*, char*, int)",
            263,
            "[Taiwan, Event] stayEvent=%s eventing:%d",
            repeat_event ? "ok" : "nullptr",
            eventing);
        return 264;
    }

    Msg* inter_msg = reinterpret_cast<Msg*>(msg);
    if (inter_msg->rate < 0) {
        inter_msg->rate = repeat_event->GetRate();
    }
    repeat_event->SetEventInfo(inter_msg->hour,
                               inter_msg->rate,
                               inter_msg->event_day);
    const bool rewardable = repeat_event->IsRewardCondition() != 0;
    Reward reward(repeat_event->GetRate(), rewardable);
    G_GameWorld()->ProcessAllUsers(reward);
    return 0;
}

// IDA_EA: 0x8160EF6
// IDA_NAME: _ZN29InterSelectCreateDnfEventInfo11makeRequestEiii
// DEMANGLED: InterSelectCreateDnfEventInfo::makeRequest(int,int,int)
void InterSelectCreateDnfEventInfo::makeRequest(int rate,
                                                int hour,
                                                int event_day) {
    Stream* stream = GlobalData::stream_pool()->Acquire(
        "localtaiwan/Dispatcher/Taiwan_InterDispatcher.cpp",
        283);
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, 9, -1);
    Msg* msg = guard.GetInBuffer<Msg>();
    if (msg) {
        msg->rate = rate;
        msg->hour = hour;
        msg->event_day = event_day;
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    } else {
        LogManager::logFormat(
            1,
            "localtaiwan/Dispatcher/Taiwan_InterDispatcher.cpp",
            "static void InterSelectCreateDnfEventInfo::makeRequest(int, int, int)",
            288,
            "[Taiwan, Event] hour:%d rate:%d event_day:%d",
            hour,
            rate,
            event_day);
    }
}

// IDA_EA: 0x816201A
// IDA_NAME: _ZN29InterSelectCreateDnfEventInfoC2Ev
// DEMANGLED: InterSelectCreateDnfEventInfo::InterSelectCreateDnfEventInfo(void)
InterSelectCreateDnfEventInfo::InterSelectCreateDnfEventInfo()
    : TaiwanInterfaceInterDispatcher(
          TaiwanInternalPack::SELECT_CREATE_DNF_EVENT_INFO) {}

// IDA_EA: 0x816203E
// IDA_NAME: _ZN29InterSelectCreateDnfEventInfoD2Ev
// DEMANGLED: InterSelectCreateDnfEventInfo::~InterSelectCreateDnfEventInfo()
// IDA_EA: 0x816206E
// IDA_NAME: _ZN29InterSelectCreateDnfEventInfoD0Ev
// DEMANGLED: InterSelectCreateDnfEventInfo::~InterSelectCreateDnfEventInfo()
InterSelectCreateDnfEventInfo::~InterSelectCreateDnfEventInfo() {}

// IDA_EA: 0x826891C
// IDA_NAME: _ZN24CPowerWarVictoriousEventC2Ev
// DEMANGLED: CPowerWarVictoriousEvent::CPowerWarVictoriousEvent(void)
CPowerWarVictoriousEvent::CPowerWarVictoriousEvent()
    : CEventBase() {}

// IDA_EA: 0x8268938
// IDA_NAME: _ZN24CPowerWarVictoriousEventD2Ev
// DEMANGLED: CPowerWarVictoriousEvent::~CPowerWarVictoriousEvent()
// IDA_EA: 0x8268968
// IDA_NAME: _ZN24CPowerWarVictoriousEventD0Ev
// DEMANGLED: CPowerWarVictoriousEvent::~CPowerWarVictoriousEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CPowerWarVictoriousEvent::~CPowerWarVictoriousEvent() {}

// IDA_EA: 0x8268986
// IDA_NAME: _ZN24CPowerWarVictoriousEvent10StartEventEv
// DEMANGLED: CPowerWarVictoriousEvent::StartEvent(void)
void CPowerWarVictoriousEvent::StartEvent() {
    if (!IsEventing(0)) {
        SetEventFlag(true);
        const int delay_sec =
            60 * G_CDataManager()->get_power_war_victorious_reward_minutes();
        G_TimerQueue()->InsertTimer(2, 0, 139, delay_sec, 0, 0);
        const int channel_no = G_CEnvironment()->get_channel_no();
        cMyTrace trace(
            "virtual void CPowerWarVictoriousEvent::StartEvent()",
            34,
            0);
        trace("Power War Victorious Event Start! server(%d)", channel_no);
    }
}

// IDA_EA: 0x8268A68
// IDA_NAME: _ZN24CPowerWarVictoriousEvent10StartEventE10Word_Param
// DEMANGLED: CPowerWarVictoriousEvent::StartEvent(Word_Param)
void CPowerWarVictoriousEvent::StartEvent(Word_Param param) {
    (void)param;
    if (!IsEventing(0)) {
        SetEventFlag(true);
        const int delay_sec =
            60 * G_CDataManager()->get_power_war_victorious_reward_minutes();
        G_TimerQueue()->InsertTimer(2, 0, 139, delay_sec, 0, 0);
        const int channel_no = G_CEnvironment()->get_channel_no();
        cMyTrace trace(
            "virtual void CPowerWarVictoriousEvent::StartEvent(Word_Param)",
            52,
            0);
        trace("Power War Victorious Event Start! server(%d)", channel_no);
    }
}

// IDA_EA: 0x8268B4A
// IDA_NAME: _ZN24CPowerWarVictoriousEvent8EndEventEv
// DEMANGLED: CPowerWarVictoriousEvent::EndEvent(void)
void CPowerWarVictoriousEvent::EndEvent() {
    if (IsEventing(0)) {
        SetEventFlag(false);
        add_info_.word_00 = 0;
        add_info_.word_02 = 0;
        cMyTrace trace("virtual void CPowerWarVictoriousEvent::EndEvent()",
                       65,
                       0);
        trace("Power War Victorious Event End");
    }
}

// IDA_EA: 0x863CE56
// IDA_NAME: _ZN32TimerEndPowerWarVictoriousRewardC2Ev
// DEMANGLED: TimerEndPowerWarVictoriousReward::TimerEndPowerWarVictoriousReward(void)
TimerEndPowerWarVictoriousReward::TimerEndPowerWarVictoriousReward() {}

// IDA_EA: 0x863ED12
// IDA_NAME: _ZN32TimerEndPowerWarVictoriousRewardD2Ev
// DEMANGLED: TimerEndPowerWarVictoriousReward::~TimerEndPowerWarVictoriousReward()
// IDA_EA: 0x863ED42
// IDA_NAME: _ZN32TimerEndPowerWarVictoriousRewardD0Ev
// DEMANGLED: TimerEndPowerWarVictoriousReward::~TimerEndPowerWarVictoriousReward() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
TimerEndPowerWarVictoriousReward::~TimerEndPowerWarVictoriousReward() {}

// IDA_EA: 0x86394CE
// IDA_NAME: _ZN32TimerEndPowerWarVictoriousReward12dispatch_sigEiij
// DEMANGLED: TimerEndPowerWarVictoriousReward::dispatch_sig(int,int,unsigned int)
int TimerEndPowerWarVictoriousReward::dispatch_sig(int object_id,
                                                   int param1,
                                                   unsigned int param2) {
    (void)object_id;
    (void)param1;
    (void)param2;
    GlobalData::s_event_manager->TriggerEventEnd(53);
    return 1;
}

// IDA_EA: 0x8109DBC
// IDA_NAME: _ZN22CBreakAwayPreventEventC2Ev
// DEMANGLED: CBreakAwayPreventEvent::CBreakAwayPreventEvent(void)
CBreakAwayPreventEvent::CBreakAwayPreventEvent()
    : CEventBase() {}

// IDA_EA: 0x8109DD8
// IDA_NAME: _ZN22CBreakAwayPreventEventD2Ev
// DEMANGLED: CBreakAwayPreventEvent::~CBreakAwayPreventEvent()
// IDA_EA: 0x8109E08
// IDA_NAME: _ZN22CBreakAwayPreventEventD0Ev
// DEMANGLED: CBreakAwayPreventEvent::~CBreakAwayPreventEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CBreakAwayPreventEvent::~CBreakAwayPreventEvent() {}

// IDA_EA: 0x8109E26
// IDA_NAME: _ZN22CBreakAwayPreventEvent10StartEventEv
// DEMANGLED: CBreakAwayPreventEvent::StartEvent(void)
void CBreakAwayPreventEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 20, 0);
    trace("CBreakAwayPreventEvent Start!");
}

// IDA_EA: 0x8109E78
// IDA_NAME: _ZN22CBreakAwayPreventEvent10StartEventE10Word_Param
// DEMANGLED: CBreakAwayPreventEvent::StartEvent(Word_Param)
void CBreakAwayPreventEvent::StartEvent(Word_Param param) {
    (void)param;
    SetEventFlag(true);
    DB_LoadBreakAwayPreventData::makeRequest(true);
    cMyTrace trace("StartEvent", 27, 0);
    trace("CBreakAwayPreventEvent Start!");
}

// IDA_EA: 0x8109ED6
// IDA_NAME: _ZN22CBreakAwayPreventEvent8EndEventEv
// DEMANGLED: CBreakAwayPreventEvent::EndEvent(void)
void CBreakAwayPreventEvent::EndEvent() {
    SetEventFlag(false);
    DB_LoadBreakAwayPreventData::makeRequest(false);
    cMyTrace trace("EndEvent", 35, 0);
    trace("CBreakAwayPreventEvent End!");
}

// IDA_EA: 0x8109FA4
// IDA_NAME: _ZN20CBurningFatigueEventC2Ev
// DEMANGLED: CBurningFatigueEvent::CBurningFatigueEvent(void)
CBurningFatigueEvent::CBurningFatigueEvent()
    : CEventBase(),
      burning_fatigue_params_(0) {
    burning_fatigue_params_ = &G_CDataManager()
                                   ->get_server_parameter_script()
                                   ->get_burning_fatigue_params();
}

// IDA_EA: 0x8109FF4
// IDA_NAME: _ZN20CBurningFatigueEventD2Ev
// DEMANGLED: CBurningFatigueEvent::~CBurningFatigueEvent()
// IDA_EA: 0x810A024
// IDA_NAME: _ZN20CBurningFatigueEventD0Ev
// DEMANGLED: CBurningFatigueEvent::~CBurningFatigueEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CBurningFatigueEvent::~CBurningFatigueEvent() {}

// IDA_EA: 0x810A042
// IDA_NAME: _ZN20CBurningFatigueEvent10StartEventEv
// DEMANGLED: CBurningFatigueEvent::StartEvent(void)
void CBurningFatigueEvent::StartEvent() {}

// IDA_EA: 0x810A048
// IDA_NAME: _ZN20CBurningFatigueEvent10StartEventE10Word_Param
// DEMANGLED: CBurningFatigueEvent::StartEvent(Word_Param)
void CBurningFatigueEvent::StartEvent(Word_Param param) {
    (void)param;
    SetEventFlag(true);
    cMyTrace trace("virtual void CBurningFatigueEvent::StartEvent(Word_Param)",
                   27,
                   0);
    trace("Burning Fatigue Event Start");
}

// IDA_EA: 0x810A09A
// IDA_NAME: _ZN20CBurningFatigueEvent8EndEventEv
// DEMANGLED: CBurningFatigueEvent::EndEvent(void)
void CBurningFatigueEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void CBurningFatigueEvent::EndEvent()", 33, 0);
    trace("Burning Fatigue Event End");
}

// IDA_EA: 0x810A0EC
// IDA_NAME: _ZNK20CBurningFatigueEvent15getBonusExpRateEt
// DEMANGLED: CBurningFatigueEvent::getBonusExpRate(unsigned short)const
int CBurningFatigueEvent::getBonusExpRate(uint16_t used_fatigue) const {
    const BurningFatigueParamVector* params =
        reinterpret_cast<const BurningFatigueParamVector*>(
            burning_fatigue_params_);
    if (!params) {
        return 0;
    }

    for (unsigned int i = 1; params->size() > i; ++i) {
        if ((*params)[i].fatigue > used_fatigue) {
            return (*params)[i - 1].bonus_exp_rate;
        }
    }
    return 0;
}

// IDA_EA: 0x810A166
// IDA_NAME: _ZNK20CBurningFatigueEvent16getRefillFatigueEtt
// DEMANGLED: CBurningFatigueEvent::getRefillFatigue(unsigned short,unsigned short)const
int CBurningFatigueEvent::getRefillFatigue(uint16_t cur_used_fatigue,
                                           uint16_t use_fatigue) const {
    const BurningFatigueParamVector* params =
        reinterpret_cast<const BurningFatigueParamVector*>(
            burning_fatigue_params_);
    if (!params) {
        return 0;
    }

    const int lower_bound =
        static_cast<int>(cur_used_fatigue) - static_cast<int>(use_fatigue);
    for (unsigned int i = 1; params->size() > i; ++i) {
        const uint16_t fatigue = (*params)[i].fatigue;
        if (fatigue <= cur_used_fatigue && fatigue > lower_bound) {
            return (*params)[i].refill_fatigue;
        }
    }
    return 0;
}

// IDA_EA: 0x811CA78
// IDA_NAME: _ZN21CFirstServerOpenEventC2Ev
// DEMANGLED: CFirstServerOpenEvent::CFirstServerOpenEvent(void)
CFirstServerOpenEvent::CFirstServerOpenEvent()
    : CEventBase() {}

// IDA_EA: 0x811CA94
// IDA_NAME: _ZN21CFirstServerOpenEventD2Ev
// DEMANGLED: CFirstServerOpenEvent::~CFirstServerOpenEvent()
// IDA_EA: 0x811CAC4
// IDA_NAME: _ZN21CFirstServerOpenEventD0Ev
// DEMANGLED: CFirstServerOpenEvent::~CFirstServerOpenEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CFirstServerOpenEvent::~CFirstServerOpenEvent() {}

// IDA_EA: 0x811CAE2
// IDA_NAME: _ZN21CFirstServerOpenEvent10StartEventEv
// DEMANGLED: CFirstServerOpenEvent::StartEvent(void)
void CFirstServerOpenEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 32, 0);
    trace("First Server Open Event Start!");
}

// IDA_EA: 0x811CB34
// IDA_NAME: _ZN21CFirstServerOpenEvent10StartEventE10Word_Param
// DEMANGLED: CFirstServerOpenEvent::StartEvent(Word_Param)
void CFirstServerOpenEvent::StartEvent(Word_Param param) {
    (void)param;
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 46, 0);
    trace("First Server Open Event Start!");
}

// IDA_EA: 0x811CB86
// IDA_NAME: _ZN21CFirstServerOpenEvent8EndEventEv
// DEMANGLED: CFirstServerOpenEvent::EndEvent(void)
void CFirstServerOpenEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("EndEvent", 52, 0);
    trace("First Server Open Event End!");
}

// IDA_EA: 0x811CBD8
// IDA_NAME: _ZN21CFirstServerOpenEvent10IsEventingEP5CUser
// DEMANGLED: CFirstServerOpenEvent::IsEventing(CUser *)const
int CFirstServerOpenEvent::IsEventing(CUser* user) {
    return CEventBase::IsEventing(user) == 1;
}

// IDA_EA: 0x811CC78
// IDA_NAME: _ZN14CGmRegistEventC2Ev
// DEMANGLED: CGmRegistEvent::CGmRegistEvent(void)
CGmRegistEvent::CGmRegistEvent()
    : CEventBase() {}

// IDA_EA: 0x811CC94
// IDA_NAME: _ZN14CGmRegistEventD2Ev
// DEMANGLED: CGmRegistEvent::~CGmRegistEvent()
// IDA_EA: 0x811CCC4
// IDA_NAME: _ZN14CGmRegistEventD0Ev
// DEMANGLED: CGmRegistEvent::~CGmRegistEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CGmRegistEvent::~CGmRegistEvent() {}

// IDA_EA: 0x811CCE2
// IDA_NAME: _ZN14CGmRegistEvent10StartEventEv
// DEMANGLED: CGmRegistEvent::StartEvent(void)
void CGmRegistEvent::StartEvent() {
    DB_UpdateGMData::makeRequest(true);
    cMyTrace trace("virtual void CGmRegistEvent::StartEvent()", 22, 0);
    trace("CGmRegistEvent Start!");
}

// IDA_EA: 0x811CD2C
// IDA_NAME: _ZN14CGmRegistEvent10StartEventE10Word_Param
// DEMANGLED: CGmRegistEvent::StartEvent(Word_Param)
void CGmRegistEvent::StartEvent(Word_Param param) {
    (void)param;
    DB_UpdateGMData::makeRequest(true);
    cMyTrace trace("virtual void CGmRegistEvent::StartEvent(Word_Param)",
                   29,
                   0);
    trace("CGmRegistEvent Start!");
}

// IDA_EA: 0x811CD76
// IDA_NAME: _ZN14CGmRegistEvent8EndEventEv
// DEMANGLED: CGmRegistEvent::EndEvent(void)
void CGmRegistEvent::EndEvent() {
    cMyTrace trace("virtual void CGmRegistEvent::EndEvent()", 36, 0);
    trace("CGmRegistEvent End!");
}

// IDA_EA: 0x8285A44
// IDA_NAME: _ZN14CTrcOnOffEventC2Ev
// DEMANGLED: CTrcOnOffEvent::CTrcOnOffEvent(void)
CTrcOnOffEvent::CTrcOnOffEvent()
    : CEventBase() {}

// IDA_EA: 0x8285A60
// IDA_NAME: _ZN14CTrcOnOffEventD2Ev
// DEMANGLED: CTrcOnOffEvent::~CTrcOnOffEvent()
// IDA_EA: 0x8285A90
// IDA_NAME: _ZN14CTrcOnOffEventD0Ev
// DEMANGLED: CTrcOnOffEvent::~CTrcOnOffEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CTrcOnOffEvent::~CTrcOnOffEvent() {}

// IDA_EA: 0x8285AAE
// IDA_NAME: _ZN14CTrcOnOffEvent10StartEventEv
// DEMANGLED: CTrcOnOffEvent::StartEvent(void)
void CTrcOnOffEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 19, 0);
    trace("Trc On Off Event Start!");
}

// IDA_EA: 0x8285B00
// IDA_NAME: _ZN14CTrcOnOffEvent10StartEventE10Word_Param
// DEMANGLED: CTrcOnOffEvent::StartEvent(Word_Param)
void CTrcOnOffEvent::StartEvent(Word_Param param) {
    (void)param;
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 25, 0);
    trace("Trc On Off Event Event Start!");
}

// IDA_EA: 0x8285B52
// IDA_NAME: _ZN14CTrcOnOffEvent8EndEventEv
// DEMANGLED: CTrcOnOffEvent::EndEvent(void)
void CTrcOnOffEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("EndEvent", 31, 0);
    trace("Trc On Off Event Event End!");
}

// IDA_EA: 0x8285BA4
// IDA_NAME: _ZNK14CTrcOnOffEvent10IsEventingEP5CUser
// DEMANGLED: CTrcOnOffEvent::IsEventing(CUser *)const
int CTrcOnOffEvent::IsEventing(CUser* user) const {
    return CEventBase::IsEventing(user) == 1;
}

// IDA_EA: 0x811FF60
// IDA_NAME: _ZN22LevelUpBefore70LvEventC2Ev
// DEMANGLED: LevelUpBefore70LvEvent::LevelUpBefore70LvEvent(void)
LevelUpBefore70LvEvent::LevelUpBefore70LvEvent()
    : CEventBase() {}

// IDA_EA: 0x811FF7C
// IDA_NAME: _ZN22LevelUpBefore70LvEventD2Ev
// DEMANGLED: LevelUpBefore70LvEvent::~LevelUpBefore70LvEvent()
// IDA_EA: 0x811FFAC
// IDA_NAME: _ZN22LevelUpBefore70LvEventD0Ev
// DEMANGLED: LevelUpBefore70LvEvent::~LevelUpBefore70LvEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
LevelUpBefore70LvEvent::~LevelUpBefore70LvEvent() {}

// IDA_EA: 0x811FFCA
// IDA_NAME: _ZN22LevelUpBefore70LvEvent10StartEventEv
// DEMANGLED: LevelUpBefore70LvEvent::StartEvent(void)
void LevelUpBefore70LvEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 18, 0);
    trace("LevelUpBefore70Lv Event start!");
}

// IDA_EA: 0x812001C
// IDA_NAME: _ZN22LevelUpBefore70LvEvent10StartEventE10Word_Param
// DEMANGLED: LevelUpBefore70LvEvent::StartEvent(Word_Param)
void LevelUpBefore70LvEvent::StartEvent(Word_Param param) {
    (void)param;
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 24, 0);
    trace("LevelUpBefore70Lv Event start! [monitor request]");
}

// IDA_EA: 0x812006E
// IDA_NAME: _ZN22LevelUpBefore70LvEvent8EndEventEv
// DEMANGLED: LevelUpBefore70LvEvent::EndEvent(void)
void LevelUpBefore70LvEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("EndEvent", 30, 0);
    trace("LevelUpBefore70Lv Event end!");
}

// IDA_EA: 0x812011C
// IDA_NAME: _ZNK22LevelUpBefore70LvEvent10IsEventingEP5CUser
// DEMANGLED: LevelUpBefore70LvEvent::IsEventing(CUser *)const
int LevelUpBefore70LvEvent::IsEventing(CUser* user) const {
    (void)user;
    return 1;
}

// IDA_EA: 0x81B9D5C
// IDA_NAME: _ZN21CNotApplyBalkeunEventC2Ev
// DEMANGLED: CNotApplyBalkeunEvent::CNotApplyBalkeunEvent(void)
CNotApplyBalkeunEvent::CNotApplyBalkeunEvent()
    : CEventBase() {}

// IDA_EA: 0x81B9D78
// IDA_NAME: _ZN21CNotApplyBalkeunEventD2Ev
// DEMANGLED: CNotApplyBalkeunEvent::~CNotApplyBalkeunEvent()
// IDA_EA: 0x81B9DA8
// IDA_NAME: _ZN21CNotApplyBalkeunEventD0Ev
// DEMANGLED: CNotApplyBalkeunEvent::~CNotApplyBalkeunEvent() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CNotApplyBalkeunEvent::~CNotApplyBalkeunEvent() {}

// IDA_EA: 0x81B9DC6
// IDA_NAME: _ZN21CNotApplyBalkeunEvent10StartEventEv
// DEMANGLED: CNotApplyBalkeunEvent::StartEvent(void)
void CNotApplyBalkeunEvent::StartEvent() {
    if (!IsEventing(0)) {
        SetEventFlag(true);
        const int channel_no = G_CEnvironment()->get_channel_no();
        cMyTrace trace("virtual void CNotApplyBalkeunEvent::StartEvent()",
                       22,
                       0);
        trace("Not Apply Balkeun Event Start! server(%d)", channel_no);
    }
}

// IDA_EA: 0x81B9E4C
// IDA_NAME: _ZN21CNotApplyBalkeunEvent10StartEventE10Word_Param
// DEMANGLED: CNotApplyBalkeunEvent::StartEvent(Word_Param)
void CNotApplyBalkeunEvent::StartEvent(Word_Param param) {
    (void)param;
    if (!IsEventing(0)) {
        SetEventFlag(true);
        const int channel_no = G_CEnvironment()->get_channel_no();
        cMyTrace trace(
            "virtual void CNotApplyBalkeunEvent::StartEvent(Word_Param)",
            31,
            0);
        trace("Not Apply Balkeun Event Start! server(%d)", channel_no);
    }
}

// IDA_EA: 0x81B9ED2
// IDA_NAME: _ZN21CNotApplyBalkeunEvent8EndEventEv
// DEMANGLED: CNotApplyBalkeunEvent::EndEvent(void)
void CNotApplyBalkeunEvent::EndEvent() {
    if (IsEventing(0)) {
        SetEventFlag(false);
        cMyTrace trace("virtual void CNotApplyBalkeunEvent::EndEvent()",
                       40,
                       0);
        trace("Not Apply Balkeun Event End");
    }
}

// IDA_EA: 0x81649B0
// IDA_NAME: _ZN14EventGiveMeBoxC2Ev
// DEMANGLED: EventGiveMeBox::EventGiveMeBox(void)
EventGiveMeBox::EventGiveMeBox()
    : CEventBase() {}

// IDA_EA: 0x81649CC
// IDA_NAME: _ZN14EventGiveMeBoxD2Ev
// DEMANGLED: EventGiveMeBox::~EventGiveMeBox()
// IDA_EA: 0x81649FC
// IDA_NAME: _ZN14EventGiveMeBoxD0Ev
// DEMANGLED: EventGiveMeBox::~EventGiveMeBox() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
EventGiveMeBox::~EventGiveMeBox() {}

// IDA_EA: 0x8164A1A
// IDA_NAME: _ZN14EventGiveMeBox10StartEventEv
// DEMANGLED: EventGiveMeBox::StartEvent(void)
void EventGiveMeBox::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("virtual void EventGiveMeBox::StartEvent()", 18, 0);
    trace("[Taiwan, EventGiveMeBox] Event start!");
}

// IDA_EA: 0x8164A6C
// IDA_NAME: _ZN14EventGiveMeBox10StartEventE10Word_Param
// DEMANGLED: EventGiveMeBox::StartEvent(Word_Param)
void EventGiveMeBox::StartEvent(Word_Param param) {
    (void)param;
    SetEventFlag(true);
    cMyTrace trace("virtual void EventGiveMeBox::StartEvent(Word_Param)",
                   23,
                   0);
    trace("[Taiwan, EventGiveMeBox] Event start!");
}

// IDA_EA: 0x8164ABE
// IDA_NAME: _ZN14EventGiveMeBox8EndEventEv
// DEMANGLED: EventGiveMeBox::EndEvent(void)
void EventGiveMeBox::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void EventGiveMeBox::EndEvent()", 29, 0);
    trace("[Taiwan, EventGiveMeBox] Event stop!");
}

int CServerEvent::m_nExpRate = 0;
int CServerEvent::m_nCurEvent[2] = {0, 0};
int CServerEvent::m_nCurEventInterval[2] = {0, 0};

// IDA_EA: 0x814A6E0
// IDA_NAME: _ZN12CServerEvent10GetExpRateEv
// DEMANGLED: CServerEvent::GetExpRate(void)
int CServerEvent::GetExpRate() {
    return m_nExpRate;
}

// IDA_EA: 0x84E9248
// IDA_NAME: _ZN12CServerEvent10AddExpRateEi
// DEMANGLED: CServerEvent::AddExpRate(int)
int CServerEvent::AddExpRate(int exp_rate) {
    m_nExpRate += exp_rate;
    return m_nExpRate;
}

// IDA_EA: 0x84E925F
// IDA_NAME: _ZN12CServerEvent11TurnOnEventEii
// DEMANGLED: CServerEvent::TurnOnEvent(int,int)
int CServerEvent::TurnOnEvent(int event_id, int interval) {
    m_nCurEvent[event_id] = 1;
    m_nCurEventInterval[event_id] = interval;
    return event_id;
}

// IDA_EA: 0x84E927F
// IDA_NAME: _ZN12CServerEvent10IsEventingEi
// DEMANGLED: CServerEvent::IsEventing(int)
bool CServerEvent::IsEventing(int event_id) {
    return m_nCurEvent[event_id] == 1;
}

// IDA_EA: 0x84E9294
// IDA_NAME: _ZN12CServerEvent16GetEventIntervalEi
// DEMANGLED: CServerEvent::GetEventInterval(int)
int CServerEvent::GetEventInterval(int event_id) {
    return m_nCurEventInterval[event_id];
}

// IDA_EA: 0x863BC70
// IDA_NAME: _ZN12CServerEvent10SubExpRateEi
// DEMANGLED: CServerEvent::SubExpRate(int)
int CServerEvent::SubExpRate(int exp_rate) {
    m_nExpRate -= exp_rate;
    return m_nExpRate;
}

// IDA_EA: 0x863BC87
// IDA_NAME: _ZN12CServerEvent12TurnOffEventEi
// DEMANGLED: CServerEvent::TurnOffEvent(int)
int CServerEvent::TurnOffEvent(int event_id) {
    m_nCurEvent[event_id] = 0;
    m_nCurEventInterval[event_id] = 0;
    return event_id;
}

// IDA_EA: 0x81A40A0
// IDA_NAME: _ZN21UsedFatigueRewardItem4DataC2Ev
// DEMANGLED: UsedFatigueRewardItem::Data::Data(void)
UsedFatigueRewardItem::Data::Data() {
    fatigue = 0;
    item_count = 0;
    item_id = 0;
    std::memset(message, 0, sizeof(message));
    std::memset(title, 0, sizeof(title));
}

// IDA_EA: 0x81A6BF4
// IDA_NAME: _ZN21UsedFatigueRewardItem4DataD2Ev
// DEMANGLED: UsedFatigueRewardItem::Data::~Data()
UsedFatigueRewardItem::Data::~Data() {}

// IDA_EA: 0x81A25D2
// IDA_NAME: _ZN21UsedFatigueRewardItem7parsingEPc
// DEMANGLED: UsedFatigueRewardItem::parsing(char *)
int UsedFatigueRewardItem::parsing(char* script_buffer) {
    Arad_Script script;
    if (script.Load(script_buffer, "on_off") != 1) {
        return 161;
    }
    if (script.GetRowNum() == 0) {
        return 164;
    }

    enabled = 0;
    const std::string flag(
        script.GetString(0, const_cast<char*>("flag")));
    if (flag == "true") {
        enabled = 1;
    }

    if (script.Load(script_buffer, "item_reward") != 1) {
        return 172;
    }
    if (script.GetRowNum() == 0) {
        return 175;
    }

    for (int row = 0; row < script.GetRowNum(); ++row) {
        Data entry;
        entry.fatigue = static_cast<uint16_t>(
            script.GetInt(row, const_cast<char*>("fatigue")));
        entry.item_count = static_cast<uint16_t>(
            script.GetInt(row, const_cast<char*>("item_count")));
        entry.item_id = static_cast<uint32_t>(
            script.GetInt(row, const_cast<char*>("item_id")));
        std::strncpy(entry.message,
                     script.GetString(row, const_cast<char*>("message")),
                     0xffu);
        std::strncpy(entry.title,
                     script.GetString(row, const_cast<char*>("title")),
                     0x10u);
        entries.push_back(entry);
    }
    return 0;
}

// IDA_EA: 0x81A28E4
// IDA_NAME: _ZN21UsedFatigueRewardItem4findEiiRNS_4DataE
// DEMANGLED: UsedFatigueRewardItem::find(int,int,UsedFatigueRewardItem::Data &)
int UsedFatigueRewardItem::find(int fatigue_begin,
                                int fatigue_end,
                                Data& result) {
    for (std::vector<Data>::const_iterator it = entries.begin();
         it != entries.end(); ++it) {
        if (it->fatigue < fatigue_begin || it->fatigue >= fatigue_end) {
            continue;
        }

        result.fatigue = it->fatigue;
        result.item_id = it->item_id;
        result.item_count = it->item_count;

        // The old body copies twenty bytes from the interned title although
        // the physical title member is seventeen bytes.  Consequently the
        // low three item-id bytes are replaced by the last three transfer
        // bytes.  Preserve that observed 0627 behavior through named members,
        // without manufacturing a raw Data+0x103 facade.
        const char* const title = g_scriptStringManager_.findString(
            4, it->title, 0);
        char title_transfer[0x14];
        std::strncpy(title_transfer, title, sizeof(title_transfer));
        std::memcpy(result.title, title_transfer, sizeof(result.title));
        unsigned char item_id_bytes[sizeof(result.item_id)];
        std::memcpy(item_id_bytes, &result.item_id, sizeof(item_id_bytes));
        std::memcpy(item_id_bytes,
                    title_transfer + sizeof(result.title),
                    sizeof(item_id_bytes) - 1u);
        std::memcpy(&result.item_id, item_id_bytes, sizeof(result.item_id));
        const char* const message = g_scriptStringManager_.findString(
            4, it->message, 0);
        std::strncpy(result.message, message, sizeof(result.message));
        return 1;
    }
    return 0;
}

namespace Condition_LevelUp {

// IDA_EA: 0x8986086
// IDA_NAME: _ZN17Condition_LevelUp11Reward_ItemC2Ev
// DEMANGLED: Condition_LevelUp::Reward_Item::Reward_Item(void)
Reward_Item::Reward_Item() {}

// IDA_EA: 0x89860D2
// IDA_NAME: _ZN17Condition_LevelUp11Reward_ItemD2Ev
// DEMANGLED: Condition_LevelUp::Reward_Item::~Reward_Item()
Reward_Item::~Reward_Item() {}

// IDA_EA: 0x898700C
// IDA_NAME: _ZN17Condition_LevelUp11Reward_ItemC2ERKS0_
// DEMANGLED: Condition_LevelUp::Reward_Item::Reward_Item(Condition_LevelUp::Reward_Item const&)
Reward_Item::Reward_Item(const Reward_Item& rhs)
    : first_dword(rhs.first_dword),
      second_dword(rhs.second_dword),
      third_dword(rhs.third_dword),
      first_string(rhs.first_string),
      second_string(rhs.second_string) {}

// IDA_EA: 0x8987112
// IDA_NAME: _ZN17Condition_LevelUp11Reward_ItemaSERKS0_
// DEMANGLED: Condition_LevelUp::Reward_Item::operator=(Condition_LevelUp::Reward_Item const&)
Reward_Item& Reward_Item::operator=(const Reward_Item& rhs) {
    first_dword = rhs.first_dword;
    second_dword = rhs.second_dword;
    third_dword = rhs.third_dword;
    first_string = rhs.first_string;
    second_string = rhs.second_string;
    return *this;
}

}  // namespace Condition_LevelUp

// IDA_EA: 0x830EE4C
// IDA_NAME: _ZN13CBattle_Field22stEventMonsterStatus_tC2Ev
// DEMANGLED: CBattle_Field::stEventMonsterStatus_t::stEventMonsterStatus_t(void)
CBattle_Field::stEventMonsterStatus_t::stEventMonsterStatus_t()
    : map_position() {}

// IDA_EA: 0x8316ADC
// IDA_NAME: _ZN13CBattle_Field22stEventMonsterStatus_tD2Ev
// DEMANGLED: CBattle_Field::stEventMonsterStatus_t::~stEventMonsterStatus_t()
CBattle_Field::stEventMonsterStatus_t::~stEventMonsterStatus_t() {}

namespace advancealtar {

// IDA_EA: 0x81344BE
// IDA_NAME: _ZN12advancealtar7_RewardC2Ev
// DEMANGLED: advancealtar::_Reward::_Reward(void)
_Reward::_Reward() {
    std::memset(this, 0, 5u);
}

// IDA_EA: 0x889B9C0
// IDA_NAME: _ZN12advancealtar11_RewardItemC2Ev
// DEMANGLED: advancealtar::_RewardItem::_RewardItem(void)
_RewardItem::_RewardItem() {
    std::memset(this, 0, 0x0cu);
}

// IDA_EA: 0x813AA10
// IDA_NAME: _ZN12advancealtar11_RewardItemD2Ev
// DEMANGLED: advancealtar::_RewardItem::~_RewardItem()
_RewardItem::~_RewardItem() {}

// IDA_EA: 0x836A072
// IDA_NAME: _ZN12advancealtar15ClearRewardDataC2Ev
// DEMANGLED: advancealtar::ClearRewardData::ClearRewardData(void)
ClearRewardData::ClearRewardData() {}

// IDA_EA: 0x836A0E6
// IDA_NAME: _ZN12advancealtar15ClearRewardDataD2Ev
// DEMANGLED: advancealtar::ClearRewardData::~ClearRewardData()
ClearRewardData::~ClearRewardData() {}

// IDA_EA: 0x836A114
// IDA_NAME: _ZN12advancealtar15ClearRewardData5clearEv
// DEMANGLED: advancealtar::ClearRewardData::clear(void)
bool ClearRewardData::clear() {
    for (int index = 0; index <= 2; ++index) {
        reward_items[index].clear();
    }
    return false;
}

// IDA_EA: 0x8376C9C
// IDA_NAME: _ZN12advancealtar15ClearRewardDataaSERKS0_
// DEMANGLED: advancealtar::ClearRewardData::operator=(advancealtar::ClearRewardData const&)
ClearRewardData& ClearRewardData::operator=(const ClearRewardData& rhs) {
    for (int index = 0; index <= 2; ++index) {
        reward_items[index] = rhs.reward_items[index];
    }
    return *this;
}

// IDA_EA: 0x88F28EC
// IDA_NAME: _ZN12advancealtar15RoundRewardDataC2Ev
// DEMANGLED: advancealtar::RoundRewardData::RoundRewardData(void)
RoundRewardData::RoundRewardData() {}

// IDA_EA: 0x836A14C
// IDA_NAME: _ZN12advancealtar15RoundRewardDataD2Ev
// DEMANGLED: advancealtar::RoundRewardData::~RoundRewardData()
RoundRewardData::~RoundRewardData() {}

// IDA_EA: 0x83D1D72
// IDA_NAME: _ZN12advancealtar15RoundRewardDataC2ERKS0_
// DEMANGLED: advancealtar::RoundRewardData::RoundRewardData(advancealtar::RoundRewardData const&)
RoundRewardData::RoundRewardData(const RoundRewardData& rhs)
    : begin_round(rhs.begin_round),
      end_round(rhs.end_round),
      reward_items(rhs.reward_items) {}

// IDA_EA: 0x83DF710
// IDA_NAME: _ZN12advancealtar15RoundRewardDataaSERKS0_
// DEMANGLED: advancealtar::RoundRewardData::operator=(advancealtar::RoundRewardData const&)
RoundRewardData& RoundRewardData::operator=(const RoundRewardData& rhs) {
    begin_round = rhs.begin_round;
    end_round = rhs.end_round;
    reward_items = rhs.reward_items;
    return *this;
}

// IDA_EA: 0x836A162
// IDA_NAME: _ZN12advancealtar18SurvivalRewardDataC2Ev
// DEMANGLED: advancealtar::SurvivalRewardData::SurvivalRewardData(void)
SurvivalRewardData::SurvivalRewardData() {}

// IDA_EA: 0x836A176
// IDA_NAME: _ZN12advancealtar18SurvivalRewardDataD2Ev
// DEMANGLED: advancealtar::SurvivalRewardData::~SurvivalRewardData()
SurvivalRewardData::~SurvivalRewardData() {}

// IDA_EA: 0x8376CDE
// IDA_NAME: _ZN12advancealtar18SurvivalRewardDataaSERKS0_
// DEMANGLED: advancealtar::SurvivalRewardData::operator=(advancealtar::SurvivalRewardData const&)
SurvivalRewardData& SurvivalRewardData::operator=(
    const SurvivalRewardData& rhs) {
    round_reward_data = rhs.round_reward_data;
    return *this;
}

// IDA_EA: 0x889AEBE
// IDA_NAME: _ZNK12advancealtar18SurvivalRewardData18getRoundRewardDataEi
// DEMANGLED: advancealtar::SurvivalRewardData::getRoundRewardData(int)const
const std::vector<_RewardItem>* SurvivalRewardData::getRoundRewardData(
    int round) const {
    for (std::vector<RoundRewardData>::const_iterator it =
             round_reward_data.begin();
         it != round_reward_data.end(); ++it) {
        if (it->begin_round <= round && it->end_round >= round) {
            return &it->reward_items;
        }
    }
    return 0;
}

// IDA_EA: 0x889AD06
// IDA_NAME: _ZN12advancealtar21AchievementRewardDataC2Ev
// DEMANGLED: advancealtar::AchievementRewardData::AchievementRewardData(void)
AchievementRewardData::AchievementRewardData()
    : achievement_id(0),
      achievement_group(-1),
      clear_count(0) {}

// IDA_EA: 0x889ADBA
// IDA_NAME: _ZN12advancealtar21AchievementRewardDataD2Ev
// DEMANGLED: advancealtar::AchievementRewardData::~AchievementRewardData()
AchievementRewardData::~AchievementRewardData() {}

// IDA_EA: 0x889AE56
// IDA_NAME: _ZN12advancealtar21AchievementRewardData5resetEv
// DEMANGLED: advancealtar::AchievementRewardData::reset(void)
AchievementRewardData* AchievementRewardData::reset() {
    achievement_id = 0;
    achievement_group = -1;
    clear_count = 0;
    clear_limit = -1;
    reward_items.clear();
    title.clear();
    text.clear();
    tooltip.clear();
    return this;
}

// IDA_EA: 0x889C4EC
// IDA_NAME: _ZN12advancealtar21AchievementRewardDataC2ERKS0_
// DEMANGLED: advancealtar::AchievementRewardData::AchievementRewardData(advancealtar::AchievementRewardData const&)
AchievementRewardData::AchievementRewardData(
    const AchievementRewardData& rhs)
    : achievement_id(rhs.achievement_id),
      achievement_group(rhs.achievement_group),
      clear_count(rhs.clear_count),
      clear_limit(rhs.clear_limit),
      reward_items(rhs.reward_items),
      title(rhs.title),
      text(rhs.text),
      tooltip(rhs.tooltip) {}

// IDA_EA: 0x889ACAA
// IDA_NAME: _ZN12advancealtar14RewardItemType17getRewardItemTypeERKSsRNS0_1TE
// DEMANGLED: advancealtar::RewardItemType::getRewardItemType(std::string const&,advancealtar::RewardItemType::T &)
bool RewardItemType::getRewardItemType(const std::string& name, T& type) {
    if (name == "normal") {
        type = NORMAL;
        return true;
    }
    if (name == "event") {
        type = EVENT;
        return true;
    }
    return false;
}

// IDA_EA: 0x889AF74
// IDA_NAME: _ZN12advancealtar15RewardParameterC2Ev
// DEMANGLED: advancealtar::RewardParameter::RewardParameter(void)
RewardParameter::RewardParameter() {}

// IDA_EA: 0x889AF88
// IDA_NAME: _ZN12advancealtar15RewardParameterD2Ev
// DEMANGLED: advancealtar::RewardParameter::~RewardParameter()
RewardParameter::~RewardParameter() {}

// IDA_EA: 0x889AF9C
// IDA_NAME: _ZN12advancealtar15RewardParameter5resetEv
// DEMANGLED: advancealtar::RewardParameter::reset(void)
void RewardParameter::reset() {
    achievement_rewards.clear();
}

// IDA_EA: 0x889AFB0
// IDA_NAME: _ZN12advancealtar15RewardParameter32importAchievementRewardParameterEPKc
// DEMANGLED: advancealtar::RewardParameter::importAchievementRewardParameter(char const*)
int RewardParameter::importAchievementRewardParameter(const char* filename) {
    if (loadRDARScriptFile("", filename) != 1) {
        return 865;
    }

    int result = 0;
    std::map<int, AchievementRewardData> parsed_rewards;
    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag != "[reward data]") {
            result = 933;
            break;
        }

        AchievementRewardData reward_data;
        bool saw_type = false;
        bool saw_stage_difficulty = false;
        bool saw_stage_clear_count = false;
        bool saw_icon_image = false;
        bool saw_reward_item = false;
        bool saw_reward_title = false;
        bool saw_postal_title = false;
        bool saw_postal_letter = false;
        bool closed_reward_data = false;

        while (ScanType(tag, true) == 1) {
            if (tag == "[/reward data]") {
                closed_reward_data = true;
                break;
            }

            if (tag == "[type]") {
                if (saw_type) {
                    result = 933;
                    break;
                }
                saw_type = true;
                bool success = false;
                reward_data.achievement_id = ScanInt(&success);
                if (!success) {
                    result = 866;
                    break;
                }
                if (reward_data.achievement_id <= 0) {
                    result = 868;
                    break;
                }
            } else if (tag == "[stage difficulty]") {
                if (saw_stage_difficulty) {
                    result = 933;
                    break;
                }
                saw_stage_difficulty = true;
                std::string difficulty_text;
                if (ScanStr(&difficulty_text) != 1) {
                    result = 874;
                    break;
                }
                StageDifficulty::T difficulty = StageDifficulty::NONE;
                if (!StageDifficulty::getDifficulty(difficulty_text,
                                                    difficulty)) {
                    result = 877;
                    break;
                }
                reward_data.achievement_group = difficulty;
            } else if (tag == "[stage clear count]") {
                if (saw_stage_clear_count) {
                    result = 933;
                    break;
                }
                saw_stage_clear_count = true;
                bool success = false;
                reward_data.clear_count = ScanInt(&success);
                if (!success || reward_data.clear_count < 0) {
                    result = 883;
                    break;
                }
            } else if (tag == "[icon image]") {
                if (saw_icon_image) {
                    result = 933;
                    break;
                }
                saw_icon_image = true;
                bool success = false;
                // The 0627 value type calls this slot clear_limit; the
                // official 0828 field stored at the same ABI slot is the
                // achievement icon index.
                reward_data.clear_limit = ScanInt(&success);
                if (!success || reward_data.clear_limit < 0) {
                    result = 890;
                    break;
                }
            } else if (tag == "[reward item]") {
                if (saw_reward_item) {
                    result = 933;
                    break;
                }
                saw_reward_item = true;
                bool closed_reward_items = false;
                for (;;) {
                    bool is_type = false;
                    std::string reward_type_or_tag;
                    if (ScanTypeOrStr(is_type, reward_type_or_tag, true) !=
                        1) {
                        result = 913;
                        break;
                    }
                    if (is_type) {
                        if (reward_type_or_tag != "[/reward item]") {
                            result = 933;
                        } else {
                            closed_reward_items = true;
                        }
                        break;
                    }

                    RewardItemType::T reward_type = RewardItemType::NORMAL;
                    if (!RewardItemType::getRewardItemType(
                            reward_type_or_tag, reward_type)) {
                        result = 913;
                        break;
                    }

                    _RewardItem reward_item;
                    reward_item.reward_type = reward_type;
                    bool success = false;
                    reward_item.item_id = ScanInt(&success);
                    if (!success || reward_item.item_id <= 0) {
                        result = 913;
                        break;
                    }
                    reward_item.item_count = ScanInt(&success);
                    if (!success || reward_item.item_count <= 0) {
                        result = 913;
                        break;
                    }
                    reward_data.reward_items.push_back(reward_item);
                }
                if (result != 0 || !closed_reward_items ||
                    reward_data.reward_items.empty()) {
                    if (result == 0) {
                        result = 913;
                    }
                    break;
                }
            } else if (tag == "[reward title]") {
                if (saw_reward_title) {
                    result = 933;
                    break;
                }
                saw_reward_title = true;
                if (ScanStr(&reward_data.title) != 1 ||
                    reward_data.title.empty()) {
                    result = 921;
                    break;
                }
            } else if (tag == "[postal title]") {
                if (saw_postal_title) {
                    result = 933;
                    break;
                }
                saw_postal_title = true;
                if (ScanStr(&reward_data.text) != 1 ||
                    reward_data.text.empty()) {
                    result = 926;
                    break;
                }
            } else if (tag == "[postal letter]") {
                if (saw_postal_letter) {
                    result = 933;
                    break;
                }
                saw_postal_letter = true;
                if (ScanStr(&reward_data.tooltip) != 1 ||
                    reward_data.tooltip.empty()) {
                    result = 931;
                    break;
                }
            } else {
                result = 933;
                break;
            }
        }

        if (result != 0) {
            break;
        }
        if (!closed_reward_data || !saw_type ||
            !saw_stage_difficulty || !saw_stage_clear_count ||
            !saw_icon_image || !saw_reward_item ||
            !saw_reward_title || !saw_postal_title ||
            !saw_postal_letter) {
            result = 932;
            break;
        }
        if (!parsed_rewards.insert(
                 std::make_pair(reward_data.achievement_id,
                                reward_data)).second) {
            result = 933;
            break;
        }
    }

    unloadRDARScriptFile();
    if (result == 0 && parsed_rewards.empty()) {
        result = 932;
    }
    if (result == 0) {
        // Keep the previous live table intact on every parser failure.
        achievement_rewards.swap(parsed_rewards);
    }
    return result;
}

// IDA_EA: 0x889B544
// IDA_NAME: _ZNK12advancealtar15RewardParameter24getAchievementRewardDataEi
// DEMANGLED: advancealtar::RewardParameter::getAchievementRewardData(int)const
const AchievementRewardData* RewardParameter::getAchievementRewardData(
    int key) const {
    std::map<int, AchievementRewardData>::const_iterator it =
        achievement_rewards.find(key);
    if (it == achievement_rewards.end()) {
        return 0;
    }
    return &it->second;
}

// IDA_EA: 0x889B5A8
// IDA_NAME: _ZNK12advancealtar15RewardParameter24getAchievementRewardDataENS_15StageDifficulty1TEi
// DEMANGLED: advancealtar::RewardParameter::getAchievementRewardData(advancealtar::StageDifficulty::T,int)const
const AchievementRewardData* RewardParameter::getAchievementRewardData(
    StageDifficulty::T difficulty,
    int clear_count_value) const {
    for (std::map<int, AchievementRewardData>::const_iterator it =
             achievement_rewards.begin();
         it != achievement_rewards.end(); ++it) {
        if (it->second.achievement_group == difficulty &&
            it->second.clear_count == clear_count_value) {
            return &it->second;
        }
    }
    return 0;
}

}  // namespace advancealtar

namespace APSystem {

// IDA_EA: 0x8895736
// IDA_NAME: _ZN8APSystem16_TodayRewardItemC2Ev
// DEMANGLED: APSystem::_TodayRewardItem::_TodayRewardItem(void)
_TodayRewardItem::_TodayRewardItem() {
    std::memset(this, 0, 0x0eu);
}

// IDA_EA: 0x8897D76
// IDA_NAME: _ZN8APSystem16_TodayRewardItemD2Ev
// DEMANGLED: APSystem::_TodayRewardItem::~_TodayRewardItem()
_TodayRewardItem::~_TodayRewardItem() {}

// IDA_EA: 0x84ED448
// IDA_NAME: _ZN8APSystem21Inter_LoadRewardMedalC2Ev
// DEMANGLED: APSystem::Inter_LoadRewardMedal::Inter_LoadRewardMedal(void)
Inter_LoadRewardMedal::Inter_LoadRewardMedal()
    : ISigDispatcher() {}

// IDA_EA: 0x8125620
// IDA_NAME: _ZN8APSystem21Inter_LoadRewardMedalD2Ev
// DEMANGLED: APSystem::Inter_LoadRewardMedal::~Inter_LoadRewardMedal()
// IDA deleting destructor: 0x8125650.
Inter_LoadRewardMedal::~Inter_LoadRewardMedal() {}

// IDA_EA: 0x8124E2A
// IDA_NAME: _ZN8APSystem21Inter_LoadRewardMedal12dispatch_sigEP5CUserPci
// DEMANGLED: APSystem::Inter_LoadRewardMedal::dispatch_sig(CUser *,char *,int)
int Inter_LoadRewardMedal::dispatch_sig(CUser* user,
                                        char* payload,
                                        int payload_size) {
    (void)payload_size;

    if (!user || !user->get_state() ||
        !user->getCurCharacR()) {
        return 550;
    }
    if (!payload) {
        return 555;
    }

    DB_UpdateRewardMedal::makeRequest(
        user->GetUID(),
        *reinterpret_cast<const _SIG_LOAD_REWARD_MEDAL*>(payload));
    return 0;
}

// IDA_EA: 0x8124EA0
// IDA_NAME: _ZN8APSystem21Inter_LoadRewardMedal8SendMailER5CUserj
// DEMANGLED: APSystem::Inter_LoadRewardMedal::SendMail(CUser &,unsigned int)
int Inter_LoadRewardMedal::SendMail(CUser& user, unsigned int medal_type) {
    CDataManager* data_manager = G_CDataManager();
    CActionPointEtcParameter* parameter =
        data_manager ? data_manager->GetActionPointEtcParameter(
                           APSystem::CActionPointManager::_IsGMOn)
                     : 0;
    if (!parameter) {
        return 0;
    }

    const std::vector<_MedalRewardItem>& rewards =
        parameter->medal_rewards();
    const _MedalRewardItem* reward = 0;
    for (std::vector<_MedalRewardItem>::const_iterator it =
             rewards.begin();
         it != rewards.end();
         ++it) {
        if (it->medal_type <= medal_type &&
            (it->grade >= medal_type || it->medal_type == 100)) {
            reward = &*it;
            break;
        }
    }
    if (!reward) {
        return 0;
    }

    CItem* item_proto =
        data_manager ? data_manager->find_item(reward->item_id) : 0;
    Inven_Item item;
    if (!item_proto) {
        return 0;
    }

    typedef void (*CItemMakeInvenItemFn)(CItem*, Inven_Item*);
    void** vtable = *reinterpret_cast<void***>(item_proto);
    reinterpret_cast<CItemMakeInvenItemFn>(vtable[2])(item_proto, &item);
    item.set_add_info(static_cast<int>(reward->item_count));
    item.item_id_ = static_cast<int>(reward->item_id);

    char title[30] = {};
    const char* title_src = g_scriptStringManager_.findString(
        4,
        "chn_game_server_msg_82",
        0);
    if (!title_src) {
        title_src = "";
    }
    std::strncpy(title, title_src, 0x1du);

    char text[256] = {};
    const char* text_src = g_scriptStringManager_.findString(
        4,
        "chn_game_server_msg_83",
        0);
    if (!text_src) {
        text_src = "";
    }
    std::strncpy(text, text_src, 0xffu);

    const int server_group = user.GetServerGroup();
    const int charac_no = user.get_charac_no(-1);
    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        item,
        0,
        static_cast<unsigned int>(charac_no),
        text,
        static_cast<int>(std::strlen(text)),
        8,
        static_cast<ENUM_SERVER_GROUP>(server_group),
        false,
        false);
    return 1;
}

}  // namespace APSystem

// IDA_EA: 0x80E5CA6
// IDA_NAME: _ZNK24ChattingEmoticon_byEvent8isUsableERK5CUser
// DEMANGLED: ChattingEmoticon_byEvent::isUsable(CUser const&)const
int ChattingEmoticon_byEvent::isUsable(const CUser& user) const {
    for (std::set<int>::const_iterator it = premium_indices_.begin();
         it != premium_indices_.end(); ++it) {
        if (user.isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(*it))) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x80E5D2E
// IDA_NAME: _ZNK24ChattingEmoticon_byEvent18GetStartArrayIndexEv
// DEMANGLED: ChattingEmoticon_byEvent::GetStartArrayIndex(void)const
int ChattingEmoticon_byEvent::GetStartArrayIndex() const { return 0; }

// IDA_EA: 0x80E5D38
// IDA_NAME: _ZNK24ChattingEmoticon_byEvent16GetEndArrayIndexEv
// DEMANGLED: ChattingEmoticon_byEvent::GetEndArrayIndex(void)const
int ChattingEmoticon_byEvent::GetEndArrayIndex() const { return 5; }

// IDA_EA: 0x80E5F82
// IDA_NAME: _ZN24ChattingEmoticon_byEventC2Ev
// DEMANGLED: ChattingEmoticon_byEvent::ChattingEmoticon_byEvent(void)
ChattingEmoticon_byEvent::ChattingEmoticon_byEvent()
    : ChattingEmoticonBase() {}

// IDA_EA: 0x80E9478
// IDA_NAME: _ZN24ChattingEmoticon_byEventD2Ev
// DEMANGLED: ChattingEmoticon_byEvent::~ChattingEmoticon_byEvent()
// IDA deleting destructor: 0x80E94A8.
ChattingEmoticon_byEvent::~ChattingEmoticon_byEvent() {}

// IDA_EA: 0x8513E30
// IDA_NAME: _ZN19MultiBoxLotteryInfo18LotteryDefaultAradC2Ejjj
// DEMANGLED: MultiBoxLotteryInfo::LotteryDefaultArad::LotteryDefaultArad(unsigned int,unsigned int,unsigned int)
MultiBoxLotteryInfo::LotteryDefaultArad::LotteryDefaultArad(
    unsigned int item_id_value,
    unsigned int count_value,
    unsigned int rate_value)
    : item_id(item_id_value),
      count(count_value),
      rate(rate_value) {}

// IDA_EA: 0x8522962
// IDA_NAME: _ZN19MultiBoxLotteryInfo18LotteryDefaultAradD2Ev
// DEMANGLED: MultiBoxLotteryInfo::LotteryDefaultArad::~LotteryDefaultArad()
MultiBoxLotteryInfo::LotteryDefaultArad::~LotteryDefaultArad() {}

// IDA_EA: 0x89C35D4
// IDA_NAME: _ZN19MultiBoxLotteryInfo13stDefaultItemC2Ev
// DEMANGLED: MultiBoxLotteryInfo::stDefaultItem::stDefaultItem(void)
MultiBoxLotteryInfo::stDefaultItem::stDefaultItem()
    : first_dword(0),
      second_dword(0),
      third_dword(0),
      flag(0) {}

// IDA_EA: 0x89D48B2
// IDA_NAME: _ZN19MultiBoxLotteryInfo13stDefaultItemD2Ev
// DEMANGLED: MultiBoxLotteryInfo::stDefaultItem::~stDefaultItem()
MultiBoxLotteryInfo::stDefaultItem::~stDefaultItem() {}

// IDA_EA: 0x8513E50
// IDA_NAME: _ZN19MultiBoxLotteryInfo17stDefaultItemInfoC2Ev
// DEMANGLED: MultiBoxLotteryInfo::stDefaultItemInfo::stDefaultItemInfo(void)
MultiBoxLotteryInfo::stDefaultItemInfo::stDefaultItemInfo()
    : first_dword(0),
      second_dword(0),
      third_dword(0),
      flag(0) {
    default_items.clear();
}

// IDA_EA: 0x8513EC0
// IDA_NAME: _ZN19MultiBoxLotteryInfo17stDefaultItemInfoD2Ev
// DEMANGLED: MultiBoxLotteryInfo::stDefaultItemInfo::~stDefaultItemInfo()
MultiBoxLotteryInfo::stDefaultItemInfo::~stDefaultItemInfo() {}

// IDA_EA: 0x85151A8
// IDA_NAME: _ZN19MultiBoxLotteryInfo11LotteryUnitC2ERKS0_
// DEMANGLED: MultiBoxLotteryInfo::LotteryUnit::LotteryUnit(MultiBoxLotteryInfo::LotteryUnit const&)
MultiBoxLotteryInfo::LotteryUnit::LotteryUnit(const LotteryUnit& rhs)
    : first_dword(rhs.first_dword),
      second_dword(rhs.second_dword),
      third_dword(rhs.third_dword),
      flag(rhs.flag),
      first_pairs(rhs.first_pairs),
      second_pairs(rhs.second_pairs) {}

// IDA_EA: 0x8515236
// IDA_NAME: _ZN19MultiBoxLotteryInfo11LotteryUnitD2Ev
// DEMANGLED: MultiBoxLotteryInfo::LotteryUnit::~LotteryUnit()
MultiBoxLotteryInfo::LotteryUnit::~LotteryUnit() {}

// IDA_EA: 0x89C351E
// IDA_NAME: _ZN19MultiBoxLotteryInfo11LotteryUnitC2Ev
// DEMANGLED: MultiBoxLotteryInfo::LotteryUnit::LotteryUnit(void)
MultiBoxLotteryInfo::LotteryUnit::LotteryUnit() {
    clear();
}

// IDA_EA: 0x89C358C
// IDA_NAME: _ZN19MultiBoxLotteryInfo11LotteryUnit5clearEv
// DEMANGLED: MultiBoxLotteryInfo::LotteryUnit::clear(void)
MultiBoxLotteryInfo::LotteryUnit* MultiBoxLotteryInfo::LotteryUnit::clear() {
    second_dword = 0;
    third_dword = 0;
    flag = 0;
    first_pairs.clear();
    second_pairs.clear();
    first_dword = 0;
    return this;
}

// IDA_EA: 0x89CDF46
// IDA_NAME: _ZN19MultiBoxLotteryInfo11LotteryUnitaSERKS0_
// DEMANGLED: MultiBoxLotteryInfo::LotteryUnit::operator=(MultiBoxLotteryInfo::LotteryUnit const&)
MultiBoxLotteryInfo::LotteryUnit& MultiBoxLotteryInfo::LotteryUnit::operator=(
    const LotteryUnit& rhs) {
    first_dword = rhs.first_dword;
    second_dword = rhs.second_dword;
    third_dword = rhs.third_dword;
    flag = rhs.flag;
    first_pairs = rhs.first_pairs;
    second_pairs = rhs.second_pairs;
    return *this;
}

// IDA_EA: 0x8513ED6
// IDA_NAME: _ZN19MultiBoxLotteryInfoC2Ev
// DEMANGLED: MultiBoxLotteryInfo::MultiBoxLotteryInfo(void)
MultiBoxLotteryInfo::MultiBoxLotteryInfo() {
    clear();
}

// IDA_EA: 0x8513F44
// IDA_NAME: _ZN19MultiBoxLotteryInfo5clearEv
// DEMANGLED: MultiBoxLotteryInfo::clear(void)
MultiBoxLotteryInfo* MultiBoxLotteryInfo::clear() {
    lottery_units.clear();
    item_id = -1;
    second_dword = 0;
    third_dword = -1;
    fourth_dword = 0;
    return this;
}

// IDA_EA: 0x85145B2
// IDA_NAME: _ZN19MultiBoxLotteryInfoD2Ev
// DEMANGLED: MultiBoxLotteryInfo::~MultiBoxLotteryInfo()
MultiBoxLotteryInfo::~MultiBoxLotteryInfo() {}
