#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"

#include "common/config.h"
#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/system_time.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <new>

#include "infra/log/format/trace.h"


typedef char CPCRoomBurningEvent_Size_Must_Be_0x10[
    (sizeof(CPCRoomBurningEvent) == 0x10) ? 1 : -1];
typedef char CPCRoomFatigueEvent_Size_Must_Be_0x0c[
    (sizeof(CPCRoomFatigueEvent) == 0x0c) ? 1 : -1];
typedef char BlueMarbleDungeonEvent_Size_Must_Be_0x0c[
    (sizeof(BlueMarbleDungeonEvent) == 0x0c) ? 1 : -1];
typedef char CBloodDungeonRewardFirstEvent_Size_Must_Be_0x10[
    (sizeof(CBloodDungeonRewardFirstEvent) == 0x10) ? 1 : -1];
typedef char CBloodDungeonRewardSecondEvent_Size_Must_Be_0x10[
    (sizeof(CBloodDungeonRewardSecondEvent) == 0x10) ? 1 : -1];
typedef char CDeathTowerWinPointEvent_Size_Must_Be_0x10[
    (sizeof(CDeathTowerWinPointEvent) == 0x10) ? 1 : -1];
typedef char CPartyExpBonusEvent_Size_Must_Be_0x10[
    (sizeof(CPartyExpBonusEvent) == 0x10) ? 1 : -1];
typedef char CPvPExpPenaltyEvent_Size_Must_Be_0x0c[
    (sizeof(CPvPExpPenaltyEvent) == 0x0c) ? 1 : -1];
typedef char CPcRoomCardBlankItemEvent_Size_Must_Be_0x0c[
    (sizeof(CPcRoomCardBlankItemEvent) == 0x0c) ? 1 : -1];
typedef char CIntegratedPvPServerMatchEvent_Size_Must_Be_0x0c[
    (sizeof(CIntegratedPvPServerMatchEvent) == 0x0c) ? 1 : -1];
typedef char CTournamentPvPEvent_Size_Must_Be_0x0c[
    (sizeof(CTournamentPvPEvent) == 0x0c) ? 1 : -1];
typedef char CPvPLiveEvent_Size_Must_Be_0x0c[
    (sizeof(CPvPLiveEvent) == 0x0c) ? 1 : -1];
typedef char CPartyResultRecvFlag_Size_Must_Be_0x04[
    (sizeof(CPartyResultRecvFlag) == 0x04) ? 1 : -1];
typedef char PvpResultRecvFlag_Size_Must_Be_0x08[
    (sizeof(PvpResultRecvFlag) == 0x08) ? 1 : -1];
typedef char PvpResultType_Size_Must_Be_0x84[
    (sizeof(PvpResultType) == 0x84) ? 1 : -1];
typedef char PvpResultSet_Size_Must_Be_0x84[
    (sizeof(PvpResultSet) == 0x84) ? 1 : -1];
typedef char MSG_RES_PVP_RANK_Size_Must_Be_0x91[
    (sizeof(MSG_RES_PVP_RANK) == 0x91) ? 1 : -1];
typedef char CParty_cMember_Size_Must_Be_0x18[
    (sizeof(CParty::cMember) == 0x18) ? 1 : -1];
typedef char DimensionPartyCount_Size_Must_Be_0x03[
    (sizeof(DimensionPartyCount) == 0x03) ? 1 : -1];
typedef char CDungeonGainedGold_Size_Must_Be_0x04[
    (sizeof(CDungeonGainedGold) == 0x04) ? 1 : -1];
typedef char DungeonUserShotCount_Size_Must_Be_0x08[
    (sizeof(DungeonUserShotCount) == 0x08) ? 1 : -1];
typedef char DungeonHoldBalance_Size_Must_Be_0x08[
    (sizeof(DungeonHoldBalance) == 0x08) ? 1 : -1];
typedef char DungeonPartyBalance_Size_Must_Be_0xd8[
    (sizeof(DungeonPartyBalance) == 0xd8) ? 1 : -1];
typedef char CDungeonClearTracer_Size_Must_Be_0x18[
    (sizeof(CDungeonClearTracer) == 0x18) ? 1 : -1];
typedef char TowerOfDespairStatistic_Key_Size_Must_Be_0x08[
    (sizeof(TowerOfDespairStatistic_Key) == 0x08) ? 1 : -1];
typedef char STPvPGuild_Size_Must_Be_0x0c[
    (sizeof(STPvPGuild) == 0x0c) ? 1 : -1];
typedef char PvP_GuildWar_Log_Size_Must_Be_0x68[
    (sizeof(PvP_GuildWar_Log) == 0x68) ? 1 : -1];
typedef char CLevelDungeonPlayStatistic_stDungeonPlayInfo_Size_Must_Be_0x08[
    (sizeof(CLevelDungeonPlayStatistic::stDungeonPlayInfo) == 0x08) ? 1 : -1];
typedef char CLevelDungeonPlayStatistic_Size_Must_Be_0x18[
    (sizeof(CLevelDungeonPlayStatistic) == 0x18) ? 1 : -1];
typedef char CDungeonExpStatistic_stDungeonExp_Size_Must_Be_0x0c[
    (sizeof(CDungeonExpStatistic::stDungeonExp) == 0x0c) ? 1 : -1];
typedef char CDungeonExpStatistic_Size_Must_Be_0x18[
    (sizeof(CDungeonExpStatistic) == 0x18) ? 1 : -1];
typedef char UseCoinDungeonDiff_Size_Must_Be_0x08[
    (sizeof(UseCoinDungeonDiff) == 0x08) ? 1 : -1];
typedef char STDungeonInoutCnt_Size_Must_Be_0x10[
    (sizeof(STDungeonInoutCnt) == 0x10) ? 1 : -1];
typedef char STDungeonInoutTime_Size_Must_Be_0x10[
    (sizeof(STDungeonInoutTime) == 0x10) ? 1 : -1];
typedef char STPvPBuddyDBInfo_Size_Must_Be_0x2b[
    (sizeof(STPvPBuddyDBInfo) == 0x2b) ? 1 : -1];
typedef char STDeadTowerData_Size_Must_Be_0x0c[
    (sizeof(STDeadTowerData) == 0x0c) ? 1 : -1];
typedef char STDeadTowerDialog_Size_Must_Be_0x0c[
    (sizeof(STDeadTowerDialog) == 0x0c) ? 1 : -1];
typedef char STTowerDialog_Size_Must_Be_0x0c[
    (sizeof(STTowerDialog) == 0x0c) ? 1 : -1];
typedef char STTowerFullDialog_Size_Must_Be_0x24[
    (sizeof(STTowerFullDialog) == 0x24) ? 1 : -1];
typedef char stDungeonAssignItem_t_Size_Must_Be_0x0c[
    (sizeof(stDungeonAssignItem_t) == 0x0c) ? 1 : -1];
typedef char stPvpRefTable_stPvPGradeExp_t_Size_Must_Be_0x08[
    (sizeof(stPvpRefTable_t::stPvPGradeExp_t) == 0x08) ? 1 : -1];
typedef char stPvpRefTable_t_Size_Must_Be_0x50[
    (sizeof(stPvpRefTable_t) == 0x50) ? 1 : -1];
typedef char RefPvpGradeData_Size_Must_Be_0x08[
    (sizeof(RefPvpGrade::RefPvpGradeData) == 0x08) ? 1 : -1];
typedef char RefPvpGrade_Size_Must_Be_0x168[
    (sizeof(RefPvpGrade) == 0x168) ? 1 : -1];
typedef char StatisticsPvP_PvP_Key_Size_Must_Be_0x08[
    (sizeof(StatisticsPvP::PvP_Key) == 0x08) ? 1 : -1];
typedef char DungeonKey_Size_Must_Be_0x08[
    (sizeof(DungeonKey) == 0x08) ? 1 : -1];
typedef char ConditionLevelChkDungeon_Size_Must_Be_0x10[
    (sizeof(ConditionLevelChkDungeon) == 0x10) ? 1 : -1];
typedef char DungeonClearItem_Size_Must_Be_0x18[
    (sizeof(DungeonClearItem) == 0x18) ? 1 : -1];
typedef char DungeonClearCondition_Size_Must_Be_0x0c[
    (sizeof(DungeonClearCondition) == 0x0c) ? 1 : -1];
typedef char stDunGeonLagInfo_Size_Must_Be_0x18[
    (sizeof(stDunGeonLagInfo) == 0x18) ? 1 : -1];
typedef char dungeonEventHanaseStruct_Size_Must_Be_0x58[
    (sizeof(dungeonEventHanaseStruct) == 0x58) ? 1 : -1];
typedef char stPvPTempTable_t_Size_Must_Be_0x10[
    (sizeof(stPvPTempTable_t) == 0x10) ? 1 : -1];
typedef char SIG_SAVE_PC_ROOM_PLAY_TIME_Size_Must_Be_0x09[
    (sizeof(SIG_SAVE_PC_ROOM_PLAY_TIME) == 0x09) ? 1 : -1];
typedef char SIG_SAVE_PC_ROOM_RENT_INFO_Size_Must_Be_0x0c[
    (sizeof(SIG_SAVE_PC_ROOM_RENT_INFO) == 0x0c) ? 1 : -1];
typedef char SIG_SAVE_PC_ROOM_RENT_ITEM_Size_Must_Be_0x0c[
    (sizeof(SIG_SAVE_PC_ROOM_RENT_ITEM) == 0x0c) ? 1 : -1];
typedef char ExtreamDungeon_OUTPUT_ITEM_STR_Size_Must_Be_0x08[
    (sizeof(ExtreamDungeon::OUTPUT_ITEM_STR) == 0x08) ? 1 : -1];
typedef char ExtreamDungeon_forceRule_ResultItem_Size_Must_Be_0x08[
    (sizeof(ExtreamDungeon::forceRule_ResultItem) == 0x08) ? 1 : -1];
typedef char stLeavePartyParam_Size_Must_Be_0x08[
    (sizeof(stLeavePartyParam) == 0x08) ? 1 : -1];
typedef char stPartyTeleport_Size_Must_Be_0x10[
    (sizeof(stPartyTeleport) == 0x10) ? 1 : -1];
typedef char stPartyTeleportConfirm_Size_Must_Be_0x0c[
    (sizeof(stPartyTeleportConfirm) == 0x0c) ? 1 : -1];
typedef char stQuickPartyParam_Size_Must_Be_0x08[
    (sizeof(stQuickPartyParam) == 0x08) ? 1 : -1];
typedef char stOnPvPTimeOut_Size_Must_Be_0x08[
    (sizeof(stOnPvPTimeOut) == 0x08) ? 1 : -1];
typedef char stOnSetPvPReadyParam_Size_Must_Be_0x08[
    (sizeof(stOnSetPvPReadyParam) == 0x08) ? 1 : -1];
typedef char stSetPartyInfo_Size_Must_Be_0x08[
    (sizeof(stSetPartyInfo) == 0x08) ? 1 : -1];
typedef char stSelectDungeonParam_Size_Must_Be_0x18[
    (sizeof(stSelectDungeonParam) == 0x18) ? 1 : -1];
typedef char stChangeHostWarRoom_Size_Must_Be_0x08[
    (sizeof(stChangeHostWarRoom) == 0x08) ? 1 : -1];
typedef char stWalkoutPartyParam_Size_Must_Be_0x08[
    (sizeof(stWalkoutPartyParam) == 0x08) ? 1 : -1];
typedef char stWarRoomWp_Size_Must_Be_0x08[
    (sizeof(stWarRoomWp) == 0x08) ? 1 : -1];
typedef char PcRoomPlayTimeRewardParam_Size_Must_Be_0x08[
    (sizeof(PcRoomPlayTimeRewardParam) == 0x08) ? 1 : -1];
typedef char stSeriaRoomDecoEvent_Size_Must_Be_0x0c[
    (sizeof(stSeriaRoomDecoEvent) == 0x0c) ? 1 : -1];
typedef char seriaRoom_AniDeco_Size_Must_Be_0x04[
    (sizeof(seriaRoom_AniDeco) == 0x04) ? 1 : -1];
typedef char MSG_PARTY_TELEPORT_Size_Must_Be_0x14[
    (sizeof(MSG_PARTY_TELEPORT) == 0x14) ? 1 : -1];
typedef char MSG_PARTY_TELEPORT_CONFIRM_Size_Must_Be_0x0e[
    (sizeof(MSG_PARTY_TELEPORT_CONFIRM) == 0x0e) ? 1 : -1];
typedef char MSG_QUICK_PARTY_Size_Must_Be_0x11[
    (sizeof(MSG_QUICK_PARTY) == 0x11) ? 1 : -1];
typedef char MSG_MAKE_PVP_ROOM_Size_Must_Be_0x3d[
    (sizeof(MSG_MAKE_PVP_ROOM) == 0x3d) ? 1 : -1];
typedef char PVP_ROOM_INFO_Size_Must_Be_0x10[
    (sizeof(PVP_ROOM_INFO) == 0x10) ? 1 : -1];
typedef char stPartyMatchingInfo_Size_Must_Be_0x14[
    (sizeof(stPartyMatchingInfo) == 0x14) ? 1 : -1];
typedef char stConditionEnterBossRoom_t_Size_Must_Be_0x10[
    (sizeof(stConditionEnterBossRoom_t) == 0x10) ? 1 : -1];
typedef char CBattle_Field_CTowerOfDespairRewardData_Size_Must_Be_0x14[
    (sizeof(CBattle_Field::CTowerOfDespairRewardData) == 0x14) ? 1 : -1];
typedef char CBattle_Field_stConditionEnterBossRoomStatus_t_Size_Must_Be_0x18[
    (sizeof(CBattle_Field::stConditionEnterBossRoomStatus_t) == 0x18) ? 1 : -1];
typedef char pvp_assault_Point2d_Size_Must_Be_0x08[
    (sizeof(pvp_assault::Point2d) == 0x08) ? 1 : -1];
typedef char pvp_assault_CandidatePoint_Size_Must_Be_0x0C[
    (sizeof(pvp_assault::CandidatePoint) == 0x0c) ? 1 : -1];
typedef char pvp_assault_PartyInfo_Size_Must_Be_0x04[
    (sizeof(pvp_assault::PartyInfo) == 0x04) ? 1 : -1];
typedef char fair_pvp_JobKey_Size_Must_Be_0x08[
    (sizeof(fair_pvp::JobKey) == 0x08) ? 1 : -1];
typedef char QuickParty_RewardData_Param_Size_Must_Be_0x08[
    (sizeof(QuickParty::RewardData_Param) == 0x08) ? 1 : -1];
typedef char QuickParty_RewardData_Result_Size_Must_Be_0x1c[
    (sizeof(QuickParty::RewardData_Result) == 0x1c) ? 1 : -1];
typedef char QuickParty_IRewardType_Size_Must_Be_0x08[
    (sizeof(QuickParty::IRewardType) == 0x08) ? 1 : -1];
typedef char QuickParty_IBasicReward_Size_Must_Be_0x08[
    (sizeof(QuickParty::IBasicReward) == 0x08) ? 1 : -1];
typedef char QuickParty_IRandomwReward_Size_Must_Be_0x08[
    (sizeof(QuickParty::IRandomwReward) == 0x08) ? 1 : -1];
typedef char QuickParty_LevelData_Size_Must_Be_0x08[
    (sizeof(QuickParty::LevelData) == 0x08) ? 1 : -1];
typedef char QuickParty_LevelDataHell_Size_Must_Be_0x08[
    (sizeof(QuickParty::LevelDataHell) == 0x08) ? 1 : -1];
typedef char QuickParty_BasicReward_Size_Must_Be_0x20[
    (sizeof(QuickParty::BasicReward) == 0x20) ? 1 : -1];
typedef char QuickParty_LevelDataRandomReward_Size_Must_Be_0x14[
    (sizeof(QuickParty::LevelDataRandomReward) == 0x14) ? 1 : -1];
typedef char QuickParty_QuickPartyData_Size_Must_Be_0x228[
    (sizeof(QuickParty::QuickPartyData) == 0x228) ? 1 : -1];
typedef char QuickParty_CQuickPartyRewardManager_Size_Must_Be_0x228[
    (sizeof(QuickParty::CQuickPartyRewardManager) == 0x228) ? 1 : -1];

// IDA_EA: 0x814AAE4
// IDA_NAME: _ZN19CPCRoomBurningEvent12GetExpFactorEv
// DEMANGLED: CPCRoomBurningEvent::GetExpFactor(void)
int CPCRoomBurningEvent::GetExpFactor() {
    return exp_factor_;
}

// IDA_EA: 0x82671DC
// IDA_NAME: _ZN19CPCRoomBurningEventC2Ev
// DEMANGLED: CPCRoomBurningEvent::CPCRoomBurningEvent(void)
CPCRoomBurningEvent::CPCRoomBurningEvent()
    : CEventBase(),
      exp_factor_(100) {}

// IDA_EA: 0x8267202
// IDA_NAME: _ZN19CPCRoomBurningEventD2Ev
// DEMANGLED: CPCRoomBurningEvent::~CPCRoomBurningEvent()
// IDA_EA: 0x8267232
// IDA_NAME: _ZN19CPCRoomBurningEventD0Ev
// DEMANGLED: CPCRoomBurningEvent::~CPCRoomBurningEvent() deleting destructor
CPCRoomBurningEvent::~CPCRoomBurningEvent() {}

// IDA_EA: 0x8267250
// IDA_NAME: _ZN19CPCRoomBurningEvent10StartEventEv
// DEMANGLED: CPCRoomBurningEvent::StartEvent(void)
void CPCRoomBurningEvent::StartEvent() {
    SetEventFlag(true);
    exp_factor_ = 100;
    cMyTrace trace("StartEvent", 20, 0);
    trace("PC Room Burning Event Start!");
}

// IDA_EA: 0x82672AC
// IDA_NAME: _ZN19CPCRoomBurningEvent10StartEventE10Word_Param
// DEMANGLED: CPCRoomBurningEvent::StartEvent(Word_Param)
void CPCRoomBurningEvent::StartEvent(Word_Param param) {
    SetEventFlag(true);
    exp_factor_ = static_cast<uint16_t>(param);
    cMyTrace trace("StartEvent", 28, 0);
    trace("PC Room Burning Event Start!");
}

// IDA_EA: 0x826730A
// IDA_NAME: _ZN19CPCRoomBurningEvent8EndEventEv
// DEMANGLED: CPCRoomBurningEvent::EndEvent(void)
void CPCRoomBurningEvent::EndEvent() {
    SetEventFlag(false);
    exp_factor_ = 100;
    cMyTrace trace("EndEvent", 36, 0);
    trace("PC Room Burning Event End!");
}

// IDA_EA: 0x8267366
// IDA_NAME: _ZNK19CPCRoomBurningEvent10IsEventingEP5CUser
// DEMANGLED: CPCRoomBurningEvent::IsEventing(CUser *)const
int CPCRoomBurningEvent::IsEventing(CUser* user) const {
    return CEventBase::IsEventing(user) == 1 && user &&
           user->isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(12));
}

// IDA_EA: 0x82675C0
// IDA_NAME: _ZN19CPCRoomFatigueEventC2Ev
// DEMANGLED: CPCRoomFatigueEvent::CPCRoomFatigueEvent(void)
CPCRoomFatigueEvent::CPCRoomFatigueEvent()
    : CEventBase() {}

// IDA_EA: 0x82675DC
// IDA_NAME: _ZN19CPCRoomFatigueEventD2Ev
// DEMANGLED: CPCRoomFatigueEvent::~CPCRoomFatigueEvent()
// IDA_EA: 0x826760C
// IDA_NAME: _ZN19CPCRoomFatigueEventD0Ev
// DEMANGLED: CPCRoomFatigueEvent::~CPCRoomFatigueEvent() deleting destructor
CPCRoomFatigueEvent::~CPCRoomFatigueEvent() {}

// IDA_EA: 0x826762A
// IDA_NAME: _ZN19CPCRoomFatigueEvent10StartEventEv
// DEMANGLED: CPCRoomFatigueEvent::StartEvent(void)
void CPCRoomFatigueEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 19, 0);
    trace("PC Room Fatigue Event Start!");
}

// IDA_EA: 0x826767C
// IDA_NAME: _ZN19CPCRoomFatigueEvent10StartEventE10Word_Param
// DEMANGLED: CPCRoomFatigueEvent::StartEvent(Word_Param)
void CPCRoomFatigueEvent::StartEvent(Word_Param param) {
    (void)param;
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 26, 0);
    trace("PC Room Fatigue Event Start!");
}

// IDA_EA: 0x82676CE
// IDA_NAME: _ZN19CPCRoomFatigueEvent8EndEventEv
// DEMANGLED: CPCRoomFatigueEvent::EndEvent(void)
void CPCRoomFatigueEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("EndEvent", 33, 0);
    trace("PC Room Fatigue Event End!");
}

// IDA_EA: 0x8267720
// IDA_NAME: _ZNK19CPCRoomFatigueEvent10IsEventingEP5CUser
// DEMANGLED: CPCRoomFatigueEvent::IsEventing(CUser *)const
int CPCRoomFatigueEvent::IsEventing(CUser* user) const {
    return CEventBase::IsEventing(user) == 1 && user &&
           user->isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(12));
}

// IDA_EA: 0x80DC470
// IDA_NAME: _ZN22BlueMarbleDungeonEventC2Ev
// DEMANGLED: BlueMarbleDungeonEvent::BlueMarbleDungeonEvent(void)
BlueMarbleDungeonEvent::BlueMarbleDungeonEvent()
    : CEventBase() {}

// IDA_EA: 0x80DC48C
// IDA_NAME: _ZN22BlueMarbleDungeonEventD2Ev
// DEMANGLED: BlueMarbleDungeonEvent::~BlueMarbleDungeonEvent()
// IDA_EA: 0x80DC4BC
// IDA_NAME: _ZN22BlueMarbleDungeonEventD0Ev
// DEMANGLED: BlueMarbleDungeonEvent::~BlueMarbleDungeonEvent() deleting destructor
BlueMarbleDungeonEvent::~BlueMarbleDungeonEvent() {}

// IDA_EA: 0x80DC4DA
// IDA_NAME: _ZN22BlueMarbleDungeonEvent10StartEventEv
// DEMANGLED: BlueMarbleDungeonEvent::StartEvent(void)
void BlueMarbleDungeonEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 22, 0);
    trace("BlueMarbleDungeonEvent Start!");
}

// IDA_EA: 0x80DC52C
// IDA_NAME: _ZN22BlueMarbleDungeonEvent10StartEventE10Word_Param
// DEMANGLED: BlueMarbleDungeonEvent::StartEvent(Word_Param)
void BlueMarbleDungeonEvent::StartEvent(Word_Param param) {
    (void)param;
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 28, 0);
    trace("BlueMarbleDungeonEvent Start!");
}

// IDA_EA: 0x80DC57E
// IDA_NAME: _ZN22BlueMarbleDungeonEvent8EndEventEv
// DEMANGLED: BlueMarbleDungeonEvent::EndEvent(void)
void BlueMarbleDungeonEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("EndEvent", 34, 0);
    trace("BlueMarbleDungeonEvent End!");
}

// IDA_EA: 0x811BDDE
// IDA_NAME: _ZN29CBloodDungeonRewardFirstEventC2Ev
// DEMANGLED: CBloodDungeonRewardFirstEvent::CBloodDungeonRewardFirstEvent(void)
CBloodDungeonRewardFirstEvent::CBloodDungeonRewardFirstEvent()
    : CEventBase(),
      gold_(0) {}

// IDA_EA: 0x811BE04
// IDA_NAME: _ZN29CBloodDungeonRewardFirstEventD2Ev
// DEMANGLED: CBloodDungeonRewardFirstEvent::~CBloodDungeonRewardFirstEvent()
// IDA_EA: 0x811BE34
// IDA_NAME: _ZN29CBloodDungeonRewardFirstEventD0Ev
// DEMANGLED: CBloodDungeonRewardFirstEvent::~CBloodDungeonRewardFirstEvent() deleting destructor
CBloodDungeonRewardFirstEvent::~CBloodDungeonRewardFirstEvent() {}

// IDA_EA: 0x811BD12
// IDA_NAME: _ZN29CBloodDungeonRewardFirstEvent10StartEventEv
// DEMANGLED: CBloodDungeonRewardFirstEvent::StartEvent(void)
void CBloodDungeonRewardFirstEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace(
        "virtual void CBloodDungeonRewardFirstEvent::StartEvent()", 16, 0);
    trace("CBloodDungeonRewardFirstEvent Start!");
}

// IDA_EA: 0x811BD64
// IDA_NAME: _ZN29CBloodDungeonRewardFirstEvent10StartEventE10Word_Param
// DEMANGLED: CBloodDungeonRewardFirstEvent::StartEvent(Word_Param)
void CBloodDungeonRewardFirstEvent::StartEvent(Word_Param param) {
    gold_ = static_cast<int>(static_cast<uint16_t>(param));
    StartEvent();
}

// IDA_EA: 0x811BD8C
// IDA_NAME: _ZN29CBloodDungeonRewardFirstEvent8EndEventEv
// DEMANGLED: CBloodDungeonRewardFirstEvent::EndEvent(void)
void CBloodDungeonRewardFirstEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace(
        "virtual void CBloodDungeonRewardFirstEvent::EndEvent()", 45, 0);
    trace("CBloodDungeonRewardFirstEvent End!");
}

// IDA_EA: 0x85BFF1A
// IDA_NAME: _ZN29CBloodDungeonRewardFirstEvent7GetGoldEv
// DEMANGLED: CBloodDungeonRewardFirstEvent::GetGold(void)
int CBloodDungeonRewardFirstEvent::GetGold() {
    return gold_;
}

// IDA_EA: 0x811BF1E
// IDA_NAME: _ZN30CBloodDungeonRewardSecondEventC2Ev
// DEMANGLED: CBloodDungeonRewardSecondEvent::CBloodDungeonRewardSecondEvent(void)
CBloodDungeonRewardSecondEvent::CBloodDungeonRewardSecondEvent()
    : CEventBase(),
      gold_(0) {}

// IDA_EA: 0x811BF44
// IDA_NAME: _ZN30CBloodDungeonRewardSecondEventD2Ev
// DEMANGLED: CBloodDungeonRewardSecondEvent::~CBloodDungeonRewardSecondEvent()
// IDA_EA: 0x811BF74
// IDA_NAME: _ZN30CBloodDungeonRewardSecondEventD0Ev
// DEMANGLED: CBloodDungeonRewardSecondEvent::~CBloodDungeonRewardSecondEvent() deleting destructor
CBloodDungeonRewardSecondEvent::~CBloodDungeonRewardSecondEvent() {}

// IDA_EA: 0x811BE52
// IDA_NAME: _ZN30CBloodDungeonRewardSecondEvent10StartEventEv
// DEMANGLED: CBloodDungeonRewardSecondEvent::StartEvent(void)
void CBloodDungeonRewardSecondEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace(
        "virtual void CBloodDungeonRewardSecondEvent::StartEvent()", 16, 0);
    trace("CBloodDungeonRewardSecondEvent Start!");
}

// IDA_EA: 0x811BEA4
// IDA_NAME: _ZN30CBloodDungeonRewardSecondEvent10StartEventE10Word_Param
// DEMANGLED: CBloodDungeonRewardSecondEvent::StartEvent(Word_Param)
void CBloodDungeonRewardSecondEvent::StartEvent(Word_Param param) {
    gold_ = static_cast<int>(static_cast<uint16_t>(param));
    StartEvent();
}

// IDA_EA: 0x811BECC
// IDA_NAME: _ZN30CBloodDungeonRewardSecondEvent8EndEventEv
// DEMANGLED: CBloodDungeonRewardSecondEvent::EndEvent(void)
void CBloodDungeonRewardSecondEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace(
        "virtual void CBloodDungeonRewardSecondEvent::EndEvent()", 27, 0);
    trace("CBloodDungeonRewardSecondEvent End!");
}

// IDA_EA: 0x85BFF26
// IDA_NAME: _ZN30CBloodDungeonRewardSecondEvent7GetGoldEv
// DEMANGLED: CBloodDungeonRewardSecondEvent::GetGold(void)
int CBloodDungeonRewardSecondEvent::GetGold() {
    return gold_;
}

// IDA_EA: 0x81170E8
// IDA_NAME: _ZN24CDeathTowerWinPointEventC2Ev
// DEMANGLED: CDeathTowerWinPointEvent::CDeathTowerWinPointEvent(void)
CDeathTowerWinPointEvent::CDeathTowerWinPointEvent()
    : CEventBase(),
      bonus_rate_(0) {}

// IDA_EA: 0x811710E
// IDA_NAME: _ZN24CDeathTowerWinPointEventD2Ev
// DEMANGLED: CDeathTowerWinPointEvent::~CDeathTowerWinPointEvent()
// IDA_EA: 0x811713E
// IDA_NAME: _ZN24CDeathTowerWinPointEventD0Ev
// DEMANGLED: CDeathTowerWinPointEvent::~CDeathTowerWinPointEvent() deleting destructor
CDeathTowerWinPointEvent::~CDeathTowerWinPointEvent() {}

// IDA_EA: 0x811700E
// IDA_NAME: _ZN24CDeathTowerWinPointEvent10StartEventEv
// DEMANGLED: CDeathTowerWinPointEvent::StartEvent(void)
void CDeathTowerWinPointEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace(
        "virtual void CDeathTowerWinPointEvent::StartEvent()", 16, 0);
    trace("EVENT_REFORM : Death Tower Win Point Event Start! (%x%%)",
          bonus_rate_);
}

// IDA_EA: 0x811706E
// IDA_NAME: _ZN24CDeathTowerWinPointEvent10StartEventE10Word_Param
// DEMANGLED: CDeathTowerWinPointEvent::StartEvent(Word_Param)
void CDeathTowerWinPointEvent::StartEvent(Word_Param param) {
    bonus_rate_ = static_cast<int>(static_cast<uint16_t>(param));
    StartEvent();
}

// IDA_EA: 0x8117096
// IDA_NAME: _ZN24CDeathTowerWinPointEvent8EndEventEv
// DEMANGLED: CDeathTowerWinPointEvent::EndEvent(void)
void CDeathTowerWinPointEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void CDeathTowerWinPointEvent::EndEvent()", 34, 0);
    trace("Death Tower Win Point Event End!");
}

// IDA_EA: 0x8469DCC
// IDA_NAME: _ZN24CDeathTowerWinPointEvent12getBonusRateEv
// DEMANGLED: CDeathTowerWinPointEvent::getBonusRate(void)
long double CDeathTowerWinPointEvent::getBonusRate() {
    if (IsEventing(0)) {
        return static_cast<long double>(
                   static_cast<unsigned int>(bonus_rate_)) /
               100.0L;
    }
    return 0.0L;
}

// C13 migration marker: CPartyExpBonusEvent bodies moved to the unique
// old_game/group/cparty_exp_bonus_event_runtime.cpp owner.

// IDA_EA: 0x8268D1C
// IDA_NAME: _ZN19CPvPExpPenaltyEventC2Ev
// DEMANGLED: CPvPExpPenaltyEvent::CPvPExpPenaltyEvent(void)
CPvPExpPenaltyEvent::CPvPExpPenaltyEvent()
    : CEventBase() {}

// IDA_EA: 0x8268D38
// IDA_NAME: _ZN19CPvPExpPenaltyEventD2Ev
// DEMANGLED: CPvPExpPenaltyEvent::~CPvPExpPenaltyEvent()
// IDA_EA: 0x8268D68
// IDA_NAME: _ZN19CPvPExpPenaltyEventD0Ev
// DEMANGLED: CPvPExpPenaltyEvent::~CPvPExpPenaltyEvent() deleting destructor
CPvPExpPenaltyEvent::~CPvPExpPenaltyEvent() {}

// IDA_EA: 0x8268D86
// IDA_NAME: _ZN19CPvPExpPenaltyEvent10StartEventEv
// DEMANGLED: CPvPExpPenaltyEvent::StartEvent(void)
void CPvPExpPenaltyEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 18, 0);
    trace("CPvPExpPenaltyEvent Start!");
}

// IDA_EA: 0x8268DD8
// IDA_NAME: _ZN19CPvPExpPenaltyEvent10StartEventE10Word_Param
// DEMANGLED: CPvPExpPenaltyEvent::StartEvent(Word_Param)
void CPvPExpPenaltyEvent::StartEvent(Word_Param param) {
    (void)param;
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 24, 0);
    trace("CPvPExpPenaltyEvent Start!");
}

// IDA_EA: 0x8268E2A
// IDA_NAME: _ZN19CPvPExpPenaltyEvent8EndEventEv
// DEMANGLED: CPvPExpPenaltyEvent::EndEvent(void)
void CPvPExpPenaltyEvent::EndEvent() {
    SetEventFlag(false);
}

// IDA_EA: 0x826743C
// IDA_NAME: _ZN25CPcRoomCardBlankItemEventC2Ev
// DEMANGLED: CPcRoomCardBlankItemEvent::CPcRoomCardBlankItemEvent(void)
CPcRoomCardBlankItemEvent::CPcRoomCardBlankItemEvent()
    : CEventBase() {}

// IDA_EA: 0x8267458
// IDA_NAME: _ZN25CPcRoomCardBlankItemEventD2Ev
// DEMANGLED: CPcRoomCardBlankItemEvent::~CPcRoomCardBlankItemEvent()
// IDA_EA: 0x8267488
// IDA_NAME: _ZN25CPcRoomCardBlankItemEventD0Ev
// DEMANGLED: CPcRoomCardBlankItemEvent::~CPcRoomCardBlankItemEvent() deleting destructor
CPcRoomCardBlankItemEvent::~CPcRoomCardBlankItemEvent() {}

// IDA_EA: 0x82674A6
// IDA_NAME: _ZN25CPcRoomCardBlankItemEvent10StartEventEv
// DEMANGLED: CPcRoomCardBlankItemEvent::StartEvent(void)
void CPcRoomCardBlankItemEvent::StartEvent() {}

// IDA_EA: 0x82674AC
// IDA_NAME: _ZN25CPcRoomCardBlankItemEvent10StartEventE10Word_Param
// DEMANGLED: CPcRoomCardBlankItemEvent::StartEvent(Word_Param)
void CPcRoomCardBlankItemEvent::StartEvent(Word_Param param) {
    (void)param;
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 24, 0);
    trace("Pc Room Card Blank Item Event Start");
}

// IDA_EA: 0x82674FE
// IDA_NAME: _ZN25CPcRoomCardBlankItemEvent8EndEventEv
// DEMANGLED: CPcRoomCardBlankItemEvent::EndEvent(void)
void CPcRoomCardBlankItemEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("EndEvent", 30, 0);
    trace("Pc Room Card Blank Item Event End");
}

// IDA_EA: 0x811D438
// IDA_NAME: _ZN30CIntegratedPvPServerMatchEventC2Ev
// DEMANGLED: CIntegratedPvPServerMatchEvent::CIntegratedPvPServerMatchEvent(void)
CIntegratedPvPServerMatchEvent::CIntegratedPvPServerMatchEvent()
    : CEventBase() {}

// IDA_EA: 0x811D454
// IDA_NAME: _ZN30CIntegratedPvPServerMatchEventD2Ev
// DEMANGLED: CIntegratedPvPServerMatchEvent::~CIntegratedPvPServerMatchEvent()
// IDA_EA: 0x811D484
// IDA_NAME: _ZN30CIntegratedPvPServerMatchEventD0Ev
// DEMANGLED: CIntegratedPvPServerMatchEvent::~CIntegratedPvPServerMatchEvent() deleting destructor
CIntegratedPvPServerMatchEvent::~CIntegratedPvPServerMatchEvent() {}

// IDA_EA: 0x811D4A2
// IDA_NAME: _ZN30CIntegratedPvPServerMatchEvent10StartEventEv
// DEMANGLED: CIntegratedPvPServerMatchEvent::StartEvent(void)
void CIntegratedPvPServerMatchEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 18, 0);
    trace("CIntegratedPvPServerMatchEvent Start!");
}

// IDA_EA: 0x811D4F4
// IDA_NAME: _ZN30CIntegratedPvPServerMatchEvent10StartEventE10Word_Param
// DEMANGLED: CIntegratedPvPServerMatchEvent::StartEvent(Word_Param)
void CIntegratedPvPServerMatchEvent::StartEvent(Word_Param param) {
    (void)param;
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 24, 0);
    trace("CIntegratedPvPServerMatchEvent Start!");
}

// IDA_EA: 0x811D546
// IDA_NAME: _ZN30CIntegratedPvPServerMatchEvent8EndEventEv
// DEMANGLED: CIntegratedPvPServerMatchEvent::EndEvent(void)
void CIntegratedPvPServerMatchEvent::EndEvent() {
    SetEventFlag(false);
}

// IDA_EA: 0x82845CC
// IDA_NAME: _ZN19CTournamentPvPEventC2Ev
// DEMANGLED: CTournamentPvPEvent::CTournamentPvPEvent(void)
CTournamentPvPEvent::CTournamentPvPEvent()
    : CEventBase() {}

// IDA_EA: 0x82845E8
// IDA_NAME: _ZN19CTournamentPvPEventD2Ev
// DEMANGLED: CTournamentPvPEvent::~CTournamentPvPEvent()
// IDA_EA: 0x8284618
// IDA_NAME: _ZN19CTournamentPvPEventD0Ev
// DEMANGLED: CTournamentPvPEvent::~CTournamentPvPEvent() deleting destructor
CTournamentPvPEvent::~CTournamentPvPEvent() {}

// IDA_EA: 0x8284636
// IDA_NAME: _ZN19CTournamentPvPEvent10StartEventEv
// DEMANGLED: CTournamentPvPEvent::StartEvent(void)
void CTournamentPvPEvent::StartEvent() {}

// IDA_EA: 0x828463C
// IDA_NAME: _ZN19CTournamentPvPEvent10StartEventE10Word_Param
// DEMANGLED: CTournamentPvPEvent::StartEvent(Word_Param)
void CTournamentPvPEvent::StartEvent(Word_Param param) {
    (void)param;
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 24, 0);
    trace("Tournament PvP Channel Open Start");
}

// IDA_EA: 0x828468E
// IDA_NAME: _ZN19CTournamentPvPEvent8EndEventEv
// DEMANGLED: CTournamentPvPEvent::EndEvent(void)
void CTournamentPvPEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("EndEvent", 30, 0);
    trace("Tournament PvP Channel Open End");
}

// IDA_EA: 0x8268EB8
// IDA_NAME: _ZN13CPvPLiveEventC2Ev
// DEMANGLED: CPvPLiveEvent::CPvPLiveEvent(void)
CPvPLiveEvent::CPvPLiveEvent()
    : CEventBase() {}

// IDA_EA: 0x8268ED4
// IDA_NAME: _ZN13CPvPLiveEventD2Ev
// DEMANGLED: CPvPLiveEvent::~CPvPLiveEvent()
// IDA_EA: 0x8268F04
// IDA_NAME: _ZN13CPvPLiveEventD0Ev
// DEMANGLED: CPvPLiveEvent::~CPvPLiveEvent() deleting destructor
CPvPLiveEvent::~CPvPLiveEvent() {}

// IDA_EA: 0x8268F22
// IDA_NAME: _ZN13CPvPLiveEvent10StartEventEv
// DEMANGLED: CPvPLiveEvent::StartEvent(void)
void CPvPLiveEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 19, 0);
    trace("CPvPLiveEvent Start!");
}

// IDA_EA: 0x8268F74
// IDA_NAME: _ZN13CPvPLiveEvent10StartEventE10Word_Param
// DEMANGLED: CPvPLiveEvent::StartEvent(Word_Param)
void CPvPLiveEvent::StartEvent(Word_Param param) {
    (void)param;
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 25, 0);
    trace("CPvPLiveEvent Start!");
}

// IDA_EA: 0x8268FC6
// IDA_NAME: _ZN13CPvPLiveEvent8EndEventEv
// DEMANGLED: CPvPLiveEvent::EndEvent(void)
void CPvPLiveEvent::EndEvent() {
    SetEventFlag(false);
}

// C-owned PvP result flags, result records, and CParty::cMember bodies moved to
// old_game/dungeon_pvp/pvp_result_runtime.cpp and
// old_game/group/cparty_result_flag_member_runtime.cpp.

// IDA_EA: 0x836AB32
// IDA_NAME: _ZNK19DimensionPartyCounteqERKS_
// DEMANGLED: DimensionPartyCount::operator==(DimensionPartyCount const&)const
bool DimensionPartyCount::operator==(const DimensionPartyCount& rhs) const {
    return first_count == rhs.first_count && second_count == rhs.second_count;
}

// IDA_EA: 0x85BEABE
// IDA_NAME: _ZN19DimensionPartyCountC2Eccc
// DEMANGLED: DimensionPartyCount::DimensionPartyCount(char,char,char)
DimensionPartyCount::DimensionPartyCount(
    char first_count_value,
    char second_count_value,
    char third_count_value)
    : first_count(first_count_value),
      second_count(second_count_value),
      third_count(third_count_value) {}

// IDA_EA: 0x88F2D86
// IDA_NAME: _ZN19DimensionPartyCountC2Ev
// DEMANGLED: DimensionPartyCount::DimensionPartyCount(void)
DimensionPartyCount::DimensionPartyCount()
    : first_count(-1),
      second_count(-1),
      third_count(-1) {}

// IDA_EA: 0x88F8912
// IDA_NAME: _ZN19DimensionPartyCountD2Ev
// DEMANGLED: DimensionPartyCount::~DimensionPartyCount()
DimensionPartyCount::~DimensionPartyCount() {}

// IDA_EA: 0x8A3DED6
// IDA_NAME: _ZN18DungeonHoldBalance5clearEv
// DEMANGLED: DungeonHoldBalance::clear(void)
DungeonHoldBalance* DungeonHoldBalance::clear() {
    raw_00 = 0;
    raw_04 = 0;
    return this;
}

// IDA_EA: 0x8A3DEEE
// IDA_NAME: _ZN18DungeonHoldBalanceC2Ev
// DEMANGLED: DungeonHoldBalance::DungeonHoldBalance(void)
DungeonHoldBalance::DungeonHoldBalance() {
    clear();
}

// IDA_EA: 0x8A4B89A
// IDA_NAME: _ZN18DungeonHoldBalanceD2Ev
// DEMANGLED: DungeonHoldBalance::~DungeonHoldBalance()
DungeonHoldBalance::~DungeonHoldBalance() {}

// IDA_EA: 0x8A4B618
// IDA_NAME: _ZN19DungeonPartyBalanceD2Ev
// DEMANGLED: DungeonPartyBalance::~DungeonPartyBalance()
DungeonPartyBalance::~DungeonPartyBalance() {}

// IDA_EA: 0x8374CA2
// IDA_NAME: _ZN27TowerOfDespairStatistic_KeyC2Ev
// DEMANGLED: TowerOfDespairStatistic_Key::TowerOfDespairStatistic_Key(void)
TowerOfDespairStatistic_Key::TowerOfDespairStatistic_Key()
    : raw_00(0),
      raw_04(0) {}

// IDA_EA: 0x85DF7D0
// IDA_NAME: _ZN10STPvPGuildC2Ev
// DEMANGLED: STPvPGuild::STPvPGuild(void)
STPvPGuild::STPvPGuild()
    : guild_no(0),
      point(0),
      grade(0) {}

// IDA_EA: 0x85DDA46
// IDA_NAME: _ZN16PvP_GuildWar_LogC2Ev
// DEMANGLED: PvP_GuildWar_Log::PvP_GuildWar_Log(void)
PvP_GuildWar_Log::PvP_GuildWar_Log() {
    pvp_start_time = 0;
}

// IDA_EA: 0x85DDA86
// IDA_NAME: _ZN16PvP_GuildWar_LogD2Ev
// DEMANGLED: PvP_GuildWar_Log::~PvP_GuildWar_Log()
PvP_GuildWar_Log::~PvP_GuildWar_Log() {
    Clear();
}

// IDA_EA: 0x85DDA9A
// IDA_NAME: _ZN16PvP_GuildWar_Log5ClearEv
// DEMANGLED: PvP_GuildWar_Log::Clear(void)
void* PvP_GuildWar_Log::Clear() {
    guild_count = 0;
    pvp_start_time = 0;
    return std::memset(
        static_cast<void*>(guilds), 0, sizeof(guilds));
}

// IDA_EA: 0x85DDD80
// IDA_NAME: _ZN16PvP_GuildWar_Log14InsertPvPGuildEji
// DEMANGLED: PvP_GuildWar_Log::InsertPvPGuild(unsigned int,int)
int PvP_GuildWar_Log::InsertPvPGuild(
    uint32_t guild_key,
    int guildwar_point) {
    for (int index = 0; index <= 7; ++index) {
        if (!guilds[index].guild_no) {
            guilds[index].guild_no = guild_key;
            guilds[index].point = guildwar_point;
            guilds[index].grade = 1;
            ++guild_count;
            return 1;
        }
        if (static_cast<uint32_t>(guilds[index].guild_no) == guild_key) {
            guilds[index].point += guildwar_point;
            ++guilds[index].grade;
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x85DF7F0
// IDA_NAME: _ZN16PvP_GuildWar_Log15SetpvpStartTimeEl
// DEMANGLED: PvP_GuildWar_Log::SetpvpStartTime(long)
PvP_GuildWar_Log* PvP_GuildWar_Log::SetpvpStartTime(long cur_sec) {
    pvp_start_time = cur_sec;
    return this;
}

// IDA_EA: 0x85DDAD0
// IDA_NAME: _ZN16PvP_GuildWar_Log19WriteGuildWarPvPLogEicPP5CUser
// DEMANGLED: PvP_GuildWar_Log::WriteGuildWarPvPLog(int,char,CUser **)
void PvP_GuildWar_Log::WriteGuildWarPvPLog(int room_index,
                                           char battle_mode,
                                           CUser** users) {
    for (int seat = 0; seat <= 7; ++seat) {
        if (users[seat]) {
            InsertPvPGuild(
                static_cast<uint32_t>(
                    users[seat]->get_charac_guildkey()),
                users[seat]->get_guildwar_point_per_pvpplay());
        }
    }

    char message[2048] = {};
    std::snprintf(
        message,
        sizeof(message),
        "Room_Idx:%d\tPvP_Mode:%d\tElasp_Time:%d\tG_Cnt:%d\n",
        room_index,
        static_cast<int>(battle_mode),
        GlobalData::s_systemTime_.getCurSec() - pvp_start_time,
        static_cast<int>(guild_count));

    if (guild_count > 8u) {
        LogManager::logFormat(
            1,
            "pvp.cpp",
            "void PvP_GuildWar_Log::WriteGuildWarPvPLog(int, char, CUser**)",
            0x12cc,
            "PvP_GuildWar_Log::WriteGuildWarPvPLog() "
            "m_guildCnt(%d) > MAX_PVP_PLAYER(8) ",
            static_cast<int>(guild_count));
        return;
    }

    for (int index = 0; index < static_cast<int>(guild_count); ++index) {
        if (!guilds[index].guild_no) {
            continue;
        }
        const size_t used = std::strlen(message);
        if (used >= sizeof(message)) {
            break;
        }
        std::snprintf(message + used,
                      sizeof(message) - used,
                      "->\t\tG_Key:%d\tPoint:%d\tM_Cnt:%d\n",
                      guilds[index].guild_no,
                      guilds[index].point,
                      static_cast<int>(guilds[index].grade));
    }

    char log_path[256] = {};
    std::snprintf(log_path,
                  sizeof(log_path),
                  "./log/%s/GuildWarResult",
                  G_CEnvironment()->get_file_name());
    CMyFileLog log(
        "void PvP_GuildWar_Log::WriteGuildWarPvPLog(int, char, CUser**)",
        0x12dc);
    log(log_path, "%s", message);

    Clear();
    for (int seat = 0; seat <= 7; ++seat) {
        if (users[seat]) {
            users[seat]->set_guildwar_point_per_pvpplay(0);
        }
    }
}

// IDA_EA: 0x82AA862
// IDA_NAME: _ZN26CLevelDungeonPlayStatisticC2Ev
// DEMANGLED: CLevelDungeonPlayStatistic::CLevelDungeonPlayStatistic(void)
CLevelDungeonPlayStatistic::CLevelDungeonPlayStatistic()
    : entries() {}

// IDA_EA: 0x82AA876
// IDA_NAME: _ZN26CLevelDungeonPlayStatisticD2Ev
// DEMANGLED: CLevelDungeonPlayStatistic::~CLevelDungeonPlayStatistic()
CLevelDungeonPlayStatistic::~CLevelDungeonPlayStatistic() {}

// IDA_EA: 0x860ECC6
// IDA_NAME: _ZN26CLevelDungeonPlayStatistic24IncreaseLevelDungeonPlayEii
// DEMANGLED: CLevelDungeonPlayStatistic::IncreaseLevelDungeonPlay(int,int)
int CLevelDungeonPlayStatistic::IncreaseLevelDungeonPlay(
    int charac_level,
    int play_value) {
    if (charac_level <= 0 || charac_level > 200) {
        cMyTrace trace(
            "void CLevelDungeonPlayStatistic::IncreaseLevelDungeonPlay(int, int)",
            1360,
            0);
        return trace("LEVEL_DUNGEON_PLAY_STATISTIC wrong level(%d)",
                     charac_level);
    }

    std::map<int, stDungeonPlayInfo>::iterator it =
        entries.find(charac_level);
    if (it == entries.end()) {
        stDungeonPlayInfo info;
        info.play_count = 1;
        info.play_value = play_value;
        entries.insert(std::make_pair(charac_level, info));
        return 1;
    }

    it->second.play_value += play_value;
    ++it->second.play_count;
    return it->second.play_count;
}

// IDA_EA: 0x82AA88A
// IDA_NAME: _ZN20CDungeonExpStatisticC2Ev
// DEMANGLED: CDungeonExpStatistic::CDungeonExpStatistic(void)
CDungeonExpStatistic::CDungeonExpStatistic()
    : entries() {}

// IDA_EA: 0x860EFCA
// IDA_NAME: _ZN20CDungeonExpStatistic18IncreaseDungeonExpEiii
// DEMANGLED: CDungeonExpStatistic::IncreaseDungeonExp(int,int,int)
int CDungeonExpStatistic::IncreaseDungeonExp(
    int charac_level,
    int exp_value,
    int bonus_value) {
    if (charac_level <= 0 || charac_level > 200) {
        cMyTrace trace(
            "void CDungeonExpStatistic::IncreaseDungeonExp(int, int, int)",
            1409,
            0);
        return trace("DUNGEON_EXP_STATISTIC wrong level(%d)", charac_level);
    }

    std::map<int, stDungeonExp>::iterator it = entries.find(charac_level);
    if (it == entries.end()) {
        stDungeonExp info;
        info.exp_value = exp_value;
        info.bonus_value = bonus_value;
        info.play_count = 1;
        entries.insert(std::make_pair(charac_level, info));
        return 1;
    }

    it->second.exp_value += exp_value;
    it->second.bonus_value += bonus_value;
    ++it->second.play_count;
    return it->second.play_count;
}

// IDA_EA: 0x88F2DEE
// IDA_NAME: _ZN18UseCoinDungeonDiffC2Ev
// DEMANGLED: UseCoinDungeonDiff::UseCoinDungeonDiff(void)
UseCoinDungeonDiff::UseCoinDungeonDiff()
    : dungeon_index(-1),
      difficulty(1) {}

// IDA_EA: 0x86103BE
// IDA_NAME: _ZN17STDungeonInoutCntC2Ev
// DEMANGLED: STDungeonInoutCnt::STDungeonInoutCnt(void)
STDungeonInoutCnt::STDungeonInoutCnt()
    : normal_entrance_total(0),
      tower_entrance_total(0),
      normal_entrance_hour(0),
      normal_active_count(0) {}

// IDA_EA: 0x8A8CCFC
// IDA_NAME: _ZN18STDungeonInoutTimeC2Ev
// DEMANGLED: STDungeonInoutTime::STDungeonInoutTime(void)
STDungeonInoutTime::STDungeonInoutTime()
    : start_hour(0),
      end_hour(0),
      start_minute(0),
      end_minute(0) {}

// IDA_EA: 0x846CD4A
// IDA_NAME: _ZN16STPvPBuddyDBInfoC2Ev
// DEMANGLED: STPvPBuddyDBInfo::STPvPBuddyDBInfo(void)
STPvPBuddyDBInfo::STPvPBuddyDBInfo() {
    raw[0] = 0;
    std::memset(raw + 1, 0, 4u);
    std::memset(raw + 5, 0, 4u);
    raw[39] = 0;
    raw[40] = 0;
    raw[41] = 0;
    raw[42] = 0;
    std::memset(raw + 9, 0, 0x1eu);
}

// IDA_EA: 0x836A1A0
// IDA_NAME: _ZN15STDeadTowerDataC2Ev
// DEMANGLED: STDeadTowerData::STDeadTowerData(void)
STDeadTowerData::STDeadTowerData()
    : values() {
    clear();
}

// IDA_EA: 0x836A1E2
// IDA_NAME: _ZN15STDeadTowerData5clearEv
// DEMANGLED: STDeadTowerData::clear(void)
int STDeadTowerData::clear() {
    values.clear();
    return 0;
}

// IDA_EA: 0x836B41C
// IDA_NAME: _ZN15STDeadTowerDataD2Ev
// DEMANGLED: STDeadTowerData::~STDeadTowerData()
STDeadTowerData::~STDeadTowerData() {}

// IDA_EA: 0x891B12C
// IDA_NAME: _ZN17STDeadTowerDialogC2Ev
// DEMANGLED: STDeadTowerDialog::STDeadTowerDialog(void)
STDeadTowerDialog::STDeadTowerDialog()
    : dialog() {
    clear();
}

// IDA_EA: 0x891B174
// IDA_NAME: _ZN17STDeadTowerDialogD2Ev
// DEMANGLED: STDeadTowerDialog::~STDeadTowerDialog()
STDeadTowerDialog::~STDeadTowerDialog() {}

// IDA_EA: 0x891B18A
// IDA_NAME: _ZN17STDeadTowerDialog5clearEv
// DEMANGLED: STDeadTowerDialog::clear(void)
STDeadTowerDialog* STDeadTowerDialog::clear() {
    raw_00 = 0;
    dialog.clear();
    raw_08 = 0;
    raw_09 = 0;
    raw_0a = 0;
    return this;
}

// IDA_EA: 0x8929428
// IDA_NAME: _ZN17STDeadTowerDialogC2ERKS_
// DEMANGLED: STDeadTowerDialog::STDeadTowerDialog(STDeadTowerDialog const&)
STDeadTowerDialog::STDeadTowerDialog(const STDeadTowerDialog& rhs)
    : raw_00(rhs.raw_00),
      dialog(rhs.dialog),
      raw_08(rhs.raw_08),
      raw_09(rhs.raw_09),
      raw_0a(rhs.raw_0a) {}

// IDA_EA: 0x89294D8
// IDA_NAME: _ZN17STDeadTowerDialogaSERKS_
// DEMANGLED: STDeadTowerDialog::operator=(STDeadTowerDialog const&)
STDeadTowerDialog& STDeadTowerDialog::operator=(
    const STDeadTowerDialog& rhs) {
    raw_00 = rhs.raw_00;
    dialog = rhs.dialog;
    raw_08 = rhs.raw_08;
    raw_09 = rhs.raw_09;
    raw_0a = rhs.raw_0a;
    return *this;
}

// IDA_EA: 0x836A9F0
// IDA_NAME: _ZN13STTowerDialogD2Ev
// DEMANGLED: STTowerDialog::~STTowerDialog()
STTowerDialog::~STTowerDialog() {}

// IDA_EA: 0x88F2CF0
// IDA_NAME: _ZN13STTowerDialogC2Ev
// DEMANGLED: STTowerDialog::STTowerDialog(void)
STTowerDialog::STTowerDialog()
    : dialog() {
    clear();
}

// IDA_EA: 0x88F2D38
// IDA_NAME: _ZN13STTowerDialog5clearEv
// DEMANGLED: STTowerDialog::clear(void)
STTowerDialog* STTowerDialog::clear() {
    raw_00 = 0;
    dialog.clear();
    raw_08 = 0;
    raw_09 = 0;
    raw_0a = 0;
    return this;
}

// IDA_EA: 0x88F6698
// IDA_NAME: _ZN13STTowerDialogC2ERKS_
// DEMANGLED: STTowerDialog::STTowerDialog(STTowerDialog const&)
STTowerDialog::STTowerDialog(const STTowerDialog& rhs)
    : raw_00(rhs.raw_00),
      dialog(rhs.dialog),
      raw_08(rhs.raw_08),
      raw_09(rhs.raw_09),
      raw_0a(rhs.raw_0a) {}

// IDA_EA: 0x88F6748
// IDA_NAME: _ZN13STTowerDialogaSERKS_
// DEMANGLED: STTowerDialog::operator=(STTowerDialog const&)
STTowerDialog& STTowerDialog::operator=(const STTowerDialog& rhs) {
    raw_00 = rhs.raw_00;
    dialog = rhs.dialog;
    raw_08 = rhs.raw_08;
    raw_09 = rhs.raw_09;
    raw_0a = rhs.raw_0a;
    return *this;
}

// IDA_EA: 0x836AA06
// IDA_NAME: _ZN17STTowerFullDialogC2Ev
// DEMANGLED: STTowerFullDialog::STTowerFullDialog(void)
STTowerFullDialog::STTowerFullDialog()
    : start_dialogs(),
      end_dialogs(),
      clear_dialogs() {
    clear();
}

// IDA_EA: 0x836AA94
// IDA_NAME: _ZN17STTowerFullDialogD2Ev
// DEMANGLED: STTowerFullDialog::~STTowerFullDialog()
STTowerFullDialog::~STTowerFullDialog() {}

// IDA_EA: 0x836AB02
// IDA_NAME: _ZN17STTowerFullDialog5clearEv
// DEMANGLED: STTowerFullDialog::clear(void)
int STTowerFullDialog::clear() {
    start_dialogs.clear();
    end_dialogs.clear();
    clear_dialogs.clear();
    return 0;
}

// IDA_EA: 0x83D1A94
// IDA_NAME: _ZN21stDungeonAssignItem_tC2ERKS_
// DEMANGLED: stDungeonAssignItem_t::stDungeonAssignItem_t(stDungeonAssignItem_t const&)
stDungeonAssignItem_t::stDungeonAssignItem_t(
    const stDungeonAssignItem_t& rhs)
    : dungeon_index(rhs.dungeon_index),
      items(rhs.items) {}

// IDA_EA: 0x83DC750
// IDA_NAME: _ZN21stDungeonAssignItem_tD2Ev
// DEMANGLED: stDungeonAssignItem_t::~stDungeonAssignItem_t()
stDungeonAssignItem_t::~stDungeonAssignItem_t() {}

// IDA_EA: 0x83DF334
// IDA_NAME: _ZN21stDungeonAssignItem_taSERKS_
// DEMANGLED: stDungeonAssignItem_t::operator=(stDungeonAssignItem_t const&)
stDungeonAssignItem_t& stDungeonAssignItem_t::operator=(
    const stDungeonAssignItem_t& rhs) {
    dungeon_index = rhs.dungeon_index;
    items = rhs.items;
    return *this;
}

// IDA_EA: 0x88F2C88
// IDA_NAME: _ZN21stDungeonAssignItem_t5clearEv
// DEMANGLED: stDungeonAssignItem_t::clear(void)
int stDungeonAssignItem_t::clear() {
    dungeon_index = -1;
    items.clear();
    return 0;
}

// IDA_EA: 0x88F2CA8
// IDA_NAME: _ZN21stDungeonAssignItem_tC2Ev
// DEMANGLED: stDungeonAssignItem_t::stDungeonAssignItem_t(void)
stDungeonAssignItem_t::stDungeonAssignItem_t()
    : items() {
    clear();
}

// IDA_EA: 0x891B0FE
// IDA_NAME: _ZN15stPvpRefTable_t15stPvPGradeExp_tC2Eij
// DEMANGLED: stPvpRefTable_t::stPvPGradeExp_t::stPvPGradeExp_t(int,unsigned int)
stPvpRefTable_t::stPvPGradeExp_t::stPvPGradeExp_t(
    int grade_value,
    unsigned int exp_value)
    : grade(grade_value),
      exp(exp_value) {}

// IDA_EA: 0x891B114
// IDA_NAME: _ZN15stPvpRefTable_t15stPvPGradeExp_tC2Ev
// DEMANGLED: stPvpRefTable_t::stPvPGradeExp_t::stPvPGradeExp_t(void)
stPvpRefTable_t::stPvPGradeExp_t::stPvPGradeExp_t()
    : grade(0),
      exp(0) {}

// IDA_EA: 0x837F45E
// IDA_NAME: _ZN15stPvpRefTable_tC2Ev
// DEMANGLED: stPvpRefTable_t::stPvpRefTable_t(void)
stPvpRefTable_t::stPvpRefTable_t()
    : max_grade_plus_one(0),
      grade_exp_table(),
      pvp_exp_loss_grade(0),
      pvp_exp_loss_exp(0),
      pvp_rank_rate_grade(0),
      pvp_rank_rate(),
      raw_28_values(),
      pvp_rank_total_count(0),
      pvp_direct_wait(0),
      pvp_timeout(0),
      pvp_rematch(0),
      pvp_reward_item_index(0),
      pvp_reward_item_count(0),
      pvp_reward_item_limit_per_day(0) {}

// IDA_EA: 0x837F4D0
// IDA_NAME: _ZN15stPvpRefTable_tD2Ev
// DEMANGLED: stPvpRefTable_t::~stPvpRefTable_t()
stPvpRefTable_t::~stPvpRefTable_t() {}

void stPvpRefTable_t::swap(stPvpRefTable_t& other) {
    std::swap(max_grade_plus_one, other.max_grade_plus_one);
    grade_exp_table.swap(other.grade_exp_table);
    std::swap(pvp_exp_loss_grade, other.pvp_exp_loss_grade);
    std::swap(pvp_exp_loss_exp, other.pvp_exp_loss_exp);
    std::swap(pvp_rank_rate_grade, other.pvp_rank_rate_grade);
    pvp_rank_rate.swap(other.pvp_rank_rate);
    raw_28_values.swap(other.raw_28_values);
    std::swap(pvp_rank_total_count, other.pvp_rank_total_count);
    std::swap(pvp_direct_wait, other.pvp_direct_wait);
    std::swap(pvp_timeout, other.pvp_timeout);
    std::swap(pvp_rematch, other.pvp_rematch);
    std::swap(pvp_reward_item_index, other.pvp_reward_item_index);
    std::swap(pvp_reward_item_count, other.pvp_reward_item_count);
    std::swap(pvp_reward_item_limit_per_day,
              other.pvp_reward_item_limit_per_day);
}

// IDA_EA: 0x844D54C
// IDA_NAME: _ZN11RefPvpGrade18getPvPExpLossGradeEv
// DEMANGLED: RefPvpGrade::getPvPExpLossGrade(void)
int RefPvpGrade::getPvPExpLossGrade() {
    return pvp_exp_loss_grade;
}

// IDA_EA: 0x844D55A
// IDA_NAME: _ZN11RefPvpGrade16getPvPExpLossExpEv
// DEMANGLED: RefPvpGrade::getPvPExpLossExp(void)
int RefPvpGrade::getPvPExpLossExp() {
    return pvp_exp_loss_exp;
}

// IDA_EA: 0x844D568
// IDA_NAME: _ZN11RefPvpGrade19getPvPRankRateGradeEv
// DEMANGLED: RefPvpGrade::getPvPRankRateGrade(void)
int RefPvpGrade::getPvPRankRateGrade() {
    return pvp_rank_rate_grade;
}

// IDA_EA: 0x844D576
// IDA_NAME: _ZN11RefPvpGrade14getPvPRankRateEv
// DEMANGLED: RefPvpGrade::getPvPRankRate(void)
std::vector<int>* RefPvpGrade::getPvPRankRate() {
    return &pvp_rank_rate;
}

int RefPvpGrade::getPvPRankTotalCount() const {
    return pvp_rank_total_count;
}

int RefPvpGrade::getPvPDirectWait() const {
    return pvp_direct_wait;
}

int RefPvpGrade::getPvPTimeout() const {
    return pvp_timeout;
}

int RefPvpGrade::getPvPRematch() const {
    return pvp_rematch;
}

int RefPvpGrade::getPvPRewardItemIndex() const {
    return pvp_reward_item_index;
}

int RefPvpGrade::getPvPRewardItemCount() const {
    return pvp_reward_item_count;
}

int RefPvpGrade::getPvPRewardItemLimitPerDay() const {
    return pvp_reward_item_limit_per_day;
}

// IDA_EA: 0x85D46E6
// IDA_NAME: _ZN11RefPvpGradeC2Ev
// DEMANGLED: RefPvpGrade::RefPvpGrade(void)
RefPvpGrade::RefPvpGrade()
    : pvp_rank_rate() {
    Clear();
}

// IDA_EA: 0x85D470A
// IDA_NAME: _ZN11RefPvpGradeD2Ev
// DEMANGLED: RefPvpGrade::~RefPvpGrade()
RefPvpGrade::~RefPvpGrade() {}

void RefPvpGrade::swap(RefPvpGrade& other) {
    std::swap(last_grade, other.last_grade);
    for (int index = 0; index <= 37; ++index) {
        std::swap(grade_ranges[index].curr_rank_point,
                  other.grade_ranges[index].curr_rank_point);
        std::swap(grade_ranges[index].next_rank_point,
                  other.grade_ranges[index].next_rank_point);
    }
    std::swap(pvp_exp_loss_grade, other.pvp_exp_loss_grade);
    std::swap(pvp_exp_loss_exp, other.pvp_exp_loss_exp);
    std::swap(pvp_rank_rate_grade, other.pvp_rank_rate_grade);
    pvp_rank_rate.swap(other.pvp_rank_rate);
    std::swap(pvp_rank_total_count, other.pvp_rank_total_count);
    std::swap(pvp_direct_wait, other.pvp_direct_wait);
    std::swap(pvp_timeout, other.pvp_timeout);
    std::swap(pvp_rematch, other.pvp_rematch);
    std::swap(pvp_reward_item_index, other.pvp_reward_item_index);
    std::swap(pvp_reward_item_count, other.pvp_reward_item_count);
    std::swap(pvp_reward_item_limit_per_day,
              other.pvp_reward_item_limit_per_day);
}

// IDA_EA: 0x85D4722
// IDA_NAME: _ZN11RefPvpGrade5ClearEv
// DEMANGLED: RefPvpGrade::Clear(void)
bool RefPvpGrade::Clear() {
    bool result = false;
    last_grade = 0;
    pvp_exp_loss_grade = 0;
    pvp_exp_loss_exp = 0;
    pvp_rank_rate_grade = 0;
    pvp_rank_rate.clear();
    pvp_rank_total_count = 0;
    pvp_direct_wait = 0;
    pvp_timeout = 0;
    pvp_rematch = 0;
    pvp_reward_item_index = 0;
    pvp_reward_item_count = 0;
    pvp_reward_item_limit_per_day = 0;
    for (int index = 0; ; ++index) {
        result = index <= 37;
        if (index > 37) {
            break;
        }
        grade_ranges[index].curr_rank_point = 0;
        grade_ranges[index].next_rank_point = 0;
    }
    return result;
}

// IDA_EA: 0x85D4768
// IDA_NAME: _ZN11RefPvpGrade12SetLastGradeEi
// DEMANGLED: RefPvpGrade::SetLastGrade(int)
int RefPvpGrade::SetLastGrade(int grade) {
    if (static_cast<unsigned int>(grade) > 0x26u) {
        return 0;
    }
    last_grade = grade;
    return 1;
}

// IDA_EA: 0x85D478E
// IDA_NAME: _ZNK11RefPvpGrade12GetLastGradeEv
// DEMANGLED: RefPvpGrade::GetLastGrade(void)const
int RefPvpGrade::GetLastGrade() const {
    return last_grade;
}

// IDA_EA: 0x85D4798
// IDA_NAME: _ZN11RefPvpGrade11SetPvpGradeEiRKNS_15RefPvpGradeDataE
// DEMANGLED: RefPvpGrade::SetPvpGrade(int,RefPvpGrade::RefPvpGradeData const&)
int RefPvpGrade::SetPvpGrade(int grade, const RefPvpGradeData& data) {
    if (static_cast<unsigned int>(grade) >= 0x26u) {
        return 0;
    }
    grade_ranges[grade].curr_rank_point = data.curr_rank_point;
    grade_ranges[grade].next_rank_point = data.next_rank_point;
    return 1;
}

// IDA_EA: 0x85D47CE
// IDA_NAME: _ZNK11RefPvpGrade11GetPvpGradeEi
// DEMANGLED: RefPvpGrade::GetPvpGrade(int)const
int RefPvpGrade::GetPvpGrade(int pvp_point) const {
    if (grade_ranges[1].curr_rank_point > pvp_point) {
        return 0;
    }
    for (int index = 1; GetLastGrade() > index; ++index) {
        if (grade_ranges[index].curr_rank_point < pvp_point &&
            grade_ranges[index].next_rank_point >= pvp_point) {
            return index;
        }
    }
    return -1;
}

// IDA_EA: 0x85D4832
// IDA_NAME: _ZNK11RefPvpGrade11GetPvpPointEi
// DEMANGLED: RefPvpGrade::GetPvpPoint(int)const
int RefPvpGrade::GetPvpPoint(int grade) const {
    if (grade <= 0) {
        return 0;
    }
    if (grade <= 37) {
        return grade_ranges[grade].curr_rank_point;
    }
    return grade_ranges[37].curr_rank_point;
}

// IDA_EA: 0x85D4860
// IDA_NAME: _ZNK11RefPvpGrade19GetPvpNextRankPointEi
// DEMANGLED: RefPvpGrade::GetPvpNextRankPoint(int)const
int RefPvpGrade::GetPvpNextRankPoint(int grade) const {
    if (grade <= 37) {
        return grade_ranges[grade].next_rank_point;
    }
    return grade_ranges[37].next_rank_point;
}

// IDA_EA: 0x85D4880
// IDA_NAME: _ZNK11RefPvpGrade19GetPvpCurrRankPointEi
// DEMANGLED: RefPvpGrade::GetPvpCurrRankPoint(int)const
int RefPvpGrade::GetPvpCurrRankPoint(int grade) const {
    if (grade <= 37) {
        return grade_ranges[grade].curr_rank_point;
    }
    return grade_ranges[37].curr_rank_point;
}

// IDA_EA: 0x85D48A0
// IDA_NAME: _ZN11RefPvpGrade3setERK15stPvpRefTable_t
// DEMANGLED: RefPvpGrade::set(stPvpRefTable_t const&)
std::vector<int>* RefPvpGrade::set(const stPvpRefTable_t& table) {
    pvp_exp_loss_grade = table.pvp_exp_loss_grade;
    pvp_exp_loss_exp = table.pvp_exp_loss_exp;
    pvp_rank_rate_grade = table.pvp_rank_rate_grade;
    pvp_rank_rate = table.pvp_rank_rate;
    pvp_rank_total_count = table.pvp_rank_total_count;
    pvp_direct_wait = table.pvp_direct_wait;
    pvp_timeout = table.pvp_timeout;
    pvp_rematch = table.pvp_rematch;
    pvp_reward_item_index = table.pvp_reward_item_index;
    pvp_reward_item_count = table.pvp_reward_item_count;
    pvp_reward_item_limit_per_day =
        table.pvp_reward_item_limit_per_day;
    return &pvp_rank_rate;
}

// IDA_EA: 0x86101D2
// IDA_NAME: _ZNK13StatisticsPvP7PvP_KeyltERKS0_
// DEMANGLED: StatisticsPvP::PvP_Key::operator<(StatisticsPvP::PvP_Key const&)const
bool StatisticsPvP::PvP_Key::operator<(const PvP_Key& rhs) const {
    if (room_number < rhs.room_number) {
        return true;
    }
    if (room_number == rhs.room_number) {
        return play_type < rhs.play_type;
    }
    return false;
}

// IDA_EA: 0x848686C
// IDA_NAME: _ZNK10DungeonKeyltERKS_
// DEMANGLED: DungeonKey::operator<(DungeonKey const&)const
bool DungeonKey::operator<(const DungeonKey& rhs) const {
    if (dungeon_type < rhs.dungeon_type) {
        return true;
    }
    return dungeon_type == rhs.dungeon_type && dungeon_no < rhs.dungeon_no;
}

// IDA_EA: 0x84868B2
// IDA_NAME: _ZN10DungeonKeyD2Ev
// DEMANGLED: DungeonKey::~DungeonKey()
DungeonKey::~DungeonKey() {}

// IDA_EA: 0x8A6887C
// IDA_NAME: _ZN21AdditionalDungeonInfoD2Ev
// DEMANGLED: AdditionalDungeonInfo::~AdditionalDungeonInfo()
AdditionalDungeonInfo::~AdditionalDungeonInfo() {}

// Migrated to old_game/dungeon_pvp/condition_level_chk_dungeon_runtime.cpp.

// IDA_EA: 0x836DDC2
// IDA_NAME: _ZN16DungeonClearItem5clearEv
// DEMANGLED: DungeonClearItem::clear(void)
DungeonClearItem* DungeonClearItem::clear() {
    reward_type = 0;
    dungeon_index = -1;
    item_id = 0;
    item_count = 0;
    difficulty = 0;
    raw_14 = -1;
    return this;
}

// IDA_EA: 0x836DE02
// IDA_NAME: _ZN16DungeonClearItemC2Ev
// DEMANGLED: DungeonClearItem::DungeonClearItem(void)
DungeonClearItem::DungeonClearItem() {
    clear();
}

// IDA_EA: 0x8A6866C
// IDA_NAME: _ZN16DungeonClearItemD2Ev
// DEMANGLED: DungeonClearItem::~DungeonClearItem()
DungeonClearItem::~DungeonClearItem() {}

// IDA_EA: 0x822C0E8
// IDA_NAME: _ZN16stDunGeonLagInfoC2Ev
// DEMANGLED: stDunGeonLagInfo::stDunGeonLagInfo(void)
stDunGeonLagInfo::stDunGeonLagInfo() {
    std::memset(raw, 0, sizeof(raw));
}

// IDA_EA: 0x822C13E
// IDA_NAME: _ZN16stDunGeonLagInfoD2Ev
// DEMANGLED: stDunGeonLagInfo::~stDunGeonLagInfo()
stDunGeonLagInfo::~stDunGeonLagInfo() {}

// IDA_EA: 0x83E53C4
// IDA_NAME: _ZN24dungeonEventHanaseStructaSERKS_
// DEMANGLED: dungeonEventHanaseStruct::operator=(dungeonEventHanaseStruct const&)
dungeonEventHanaseStruct& dungeonEventHanaseStruct::operator=(
    const dungeonEventHanaseStruct& rhs) {
    raw_00 = rhs.raw_00;
    raw_04 = rhs.raw_04;
    raw_08 = rhs.raw_08;
    raw_0c = rhs.raw_0c;
    raw_10 = rhs.raw_10;
    raw_14 = rhs.raw_14;
    raw_18 = rhs.raw_18;
    raw_1c = rhs.raw_1c;
    default_dialog = rhs.default_dialog;
    raw_24 = rhs.raw_24;
    for (int index = 0; index < oldsrv::config::kCharacterJobCount; ++index) {
        job_dialogs[index] = rhs.job_dialogs[index];
    }
    return *this;
}

// IDA_EA: 0x8A60F78
// IDA_NAME: _ZN24dungeonEventHanaseStructC2Ev
// DEMANGLED: dungeonEventHanaseStruct::dungeonEventHanaseStruct(void)
dungeonEventHanaseStruct::dungeonEventHanaseStruct()
    : raw_00(0),
      raw_04(0),
      raw_08(0),
      raw_0c(0),
      raw_10(0),
      raw_14(0),
      raw_18(0),
      raw_1c(0),
      default_dialog(),
      raw_24(0) {
    for (int index = 0; index < oldsrv::config::kCharacterJobCount; ++index) {
        job_dialogs[index] = 0;
    }
}

// IDA_EA: 0x8A6100E
// IDA_NAME: _ZN24dungeonEventHanaseStructC2ERKS_
// DEMANGLED: dungeonEventHanaseStruct::dungeonEventHanaseStruct(dungeonEventHanaseStruct const&)
dungeonEventHanaseStruct::dungeonEventHanaseStruct(
    const dungeonEventHanaseStruct& rhs)
    : raw_00(rhs.raw_00),
      raw_04(rhs.raw_04),
      raw_08(rhs.raw_08),
      raw_0c(rhs.raw_0c),
      raw_10(rhs.raw_10),
      raw_14(rhs.raw_14),
      raw_18(rhs.raw_18),
      raw_1c(rhs.raw_1c),
      default_dialog(rhs.default_dialog),
      raw_24(rhs.raw_24) {
    for (int index = 0; index < oldsrv::config::kCharacterJobCount; ++index) {
        job_dialogs[index] = 0;
        if (rhs.job_dialogs[index]) {
            job_dialogs[index] = new std::string();
            *job_dialogs[index] = *rhs.job_dialogs[index];
        }
    }
}

// IDA_EA: 0x8A61164
// IDA_NAME: _ZN24dungeonEventHanaseStructD2Ev
// DEMANGLED: dungeonEventHanaseStruct::~dungeonEventHanaseStruct()
dungeonEventHanaseStruct::~dungeonEventHanaseStruct() {
    for (int index = 0; index < oldsrv::config::kCharacterJobCount; ++index) {
        if (job_dialogs[index]) {
            delete job_dialogs[index];
        }
        job_dialogs[index] = 0;
    }
}

// IDA_EA: 0x8A615FE
// IDA_NAME: _ZN24dungeonEventHanaseStruct12getNewDialogE17ENUM_CHARACTERJOB
// DEMANGLED: dungeonEventHanaseStruct::getNewDialog(ENUM_CHARACTERJOB)
std::string* dungeonEventHanaseStruct::getNewDialog(
    ENUM_CHARACTERJOB character_job) {
    const int job_index = static_cast<int>(character_job);
    if (raw_24 && job_index >= 0 &&
        job_index < oldsrv::config::kCharacterJobCount &&
        job_dialogs[job_index]) {
        return job_dialogs[job_index];
    }
    return &default_dialog;
}

// IDA_EA: 0x8458E92
// IDA_NAME: _ZN16stPvPTempTable_tD2Ev
// DEMANGLED: stPvPTempTable_t::~stPvPTempTable_t()
stPvPTempTable_t::~stPvPTempTable_t() {}

// IDA_EA: 0x848FB64
// IDA_NAME: _ZN26SIG_SAVE_PC_ROOM_PLAY_TIMEC2Ev
// DEMANGLED: SIG_SAVE_PC_ROOM_PLAY_TIME::SIG_SAVE_PC_ROOM_PLAY_TIME(void)
SIG_SAVE_PC_ROOM_PLAY_TIME::SIG_SAVE_PC_ROOM_PLAY_TIME()
    : account_id(0),
      play_time(0),
      reward_index(0) {}

// IDA_EA: 0x848F98E
// IDA_NAME: _ZN26SIG_SAVE_PC_ROOM_RENT_INFOC2Ev
// DEMANGLED: SIG_SAVE_PC_ROOM_RENT_INFO::SIG_SAVE_PC_ROOM_RENT_INFO(void)
SIG_SAVE_PC_ROOM_RENT_INFO::SIG_SAVE_PC_ROOM_RENT_INFO() {
    clear();
}

// IDA_EA: 0x848F9A2
// IDA_NAME: _ZN26SIG_SAVE_PC_ROOM_RENT_INFO5clearEv
// DEMANGLED: SIG_SAVE_PC_ROOM_RENT_INFO::clear(void)
SIG_SAVE_PC_ROOM_RENT_INFO* SIG_SAVE_PC_ROOM_RENT_INFO::clear() {
    item_id = 0;
    slot = 0;
    rent_type = 0;
    state = 0;
    expire_time = -1;
    return this;
}

// IDA_EA: 0x8491CBE
// IDA_NAME: _ZN26SIG_SAVE_PC_ROOM_RENT_ITEMC2Ev
// DEMANGLED: SIG_SAVE_PC_ROOM_RENT_ITEM::SIG_SAVE_PC_ROOM_RENT_ITEM(void)
SIG_SAVE_PC_ROOM_RENT_ITEM::SIG_SAVE_PC_ROOM_RENT_ITEM() {
    // items[0] is constructed before this body.  The raw body at 0x8491CBE
    // initializes that single 0x0c-byte child record and has no other state.
}

// IDA_EA: 0x8694C9A
// IDA_NAME: _ZN26SIG_SAVE_PC_ROOM_RENT_ITEM5clearEv
// DEMANGLED: SIG_SAVE_PC_ROOM_RENT_ITEM::clear(void)
bool SIG_SAVE_PC_ROOM_RENT_ITEM::clear() {
    items[0].clear();
    return true;
}

namespace ExtreamDungeon {

// IDA_EA: 0x832DFBE
// IDA_NAME: _ZN14ExtreamDungeon15OUTPUT_ITEM_STRC2Eii
// DEMANGLED: ExtreamDungeon::OUTPUT_ITEM_STR::OUTPUT_ITEM_STR(int,int)
OUTPUT_ITEM_STR::OUTPUT_ITEM_STR(int first_value, int second_value)
    : first(first_value),
      second(second_value) {}

// IDA_EA: 0x832DFD4
// IDA_NAME: _ZNK14ExtreamDungeon15OUTPUT_ITEM_STRltERKS0_
// DEMANGLED: ExtreamDungeon::OUTPUT_ITEM_STR::operator<(ExtreamDungeon::OUTPUT_ITEM_STR const&)const
bool OUTPUT_ITEM_STR::operator<(const OUTPUT_ITEM_STR& rhs) const {
    if (first == rhs.first) {
        return second < rhs.second;
    }
    return first < rhs.first;
}

// IDA_EA: 0x8330CEC
// IDA_NAME: _ZN14ExtreamDungeon15OUTPUT_ITEM_STRD2Ev
// DEMANGLED: ExtreamDungeon::OUTPUT_ITEM_STR::~OUTPUT_ITEM_STR()
OUTPUT_ITEM_STR::~OUTPUT_ITEM_STR() {}

// IDA_EA: 0x832E00A
// IDA_NAME: _ZN14ExtreamDungeon20forceRule_ResultItemC2Ebi
// DEMANGLED: ExtreamDungeon::forceRule_ResultItem::forceRule_ResultItem(bool,int)
forceRule_ResultItem::forceRule_ResultItem(
    bool force_value,
    int item_index_value)
    : force(force_value),
      item_index(item_index_value) {}

// IDA_EA: 0x832C3CC
// IDA_NAME: _ZN14ExtreamDungeon19BindTable_PredicateERKNS_15OUTPUT_ITEM_STRES2_
// DEMANGLED: ExtreamDungeon::BindTable_Predicate(ExtreamDungeon::OUTPUT_ITEM_STR const&,ExtreamDungeon::OUTPUT_ITEM_STR const&)
bool BindTable_Predicate(
    const OUTPUT_ITEM_STR& lhs,
    const OUTPUT_ITEM_STR& rhs) {
    if (lhs.first == rhs.first) {
        return lhs.second < rhs.second;
    }
    return lhs.first < rhs.first;
}

}  // namespace ExtreamDungeon

// IDA_EA: 0x8230C88
// IDA_NAME: _ZN17stLeavePartyParamC2Ev
// DEMANGLED: stLeavePartyParam::stLeavePartyParam(void)
stLeavePartyParam::stLeavePartyParam()
    : ParamBase(),
      raw_04(0) {}

// IDA_EA: 0x82480FE
// IDA_NAME: _ZN17stLeavePartyParamD2Ev
// DEMANGLED: stLeavePartyParam::~stLeavePartyParam()
stLeavePartyParam::~stLeavePartyParam() {}

// IDA_EA: 0x824812E
// IDA_NAME: _ZN17stLeavePartyParamD0Ev
// DEMANGLED: stLeavePartyParam::~stLeavePartyParam() deleting destructor
void stLeavePartyParam_deleting_destructor(stLeavePartyParam* self) {
    if (self) {
        self->~stLeavePartyParam();
        operator delete(self);
    }
}

// IDA_EA: 0x824E0D4
// IDA_NAME: _ZN15stPartyTeleportC2Ev
// DEMANGLED: stPartyTeleport::stPartyTeleport(void)
stPartyTeleport::stPartyTeleport()
    : ParamBase() {}

// IDA_EA: 0x824E0F0
// IDA_NAME: _ZN15stPartyTeleportD2Ev
// DEMANGLED: stPartyTeleport::~stPartyTeleport()
stPartyTeleport::~stPartyTeleport() {}

// IDA_EA: 0x824E120
// IDA_NAME: _ZN15stPartyTeleportD0Ev
// DEMANGLED: stPartyTeleport::~stPartyTeleport() deleting destructor
void stPartyTeleport_deleting_destructor(stPartyTeleport* self) {
    if (self) {
        self->~stPartyTeleport();
        operator delete(self);
    }
}

// IDA_EA: 0x824DE72
// IDA_NAME: _ZN22stPartyTeleportConfirmC2Ev
// DEMANGLED: stPartyTeleportConfirm::stPartyTeleportConfirm(void)
stPartyTeleportConfirm::stPartyTeleportConfirm()
    : ParamBase() {}

// IDA_EA: 0x824DE8E
// IDA_NAME: _ZN22stPartyTeleportConfirmD2Ev
// DEMANGLED: stPartyTeleportConfirm::~stPartyTeleportConfirm()
stPartyTeleportConfirm::~stPartyTeleportConfirm() {}

// IDA_EA: 0x824DEBE
// IDA_NAME: _ZN22stPartyTeleportConfirmD0Ev
// DEMANGLED: stPartyTeleportConfirm::~stPartyTeleportConfirm() deleting destructor
void stPartyTeleportConfirm_deleting_destructor(
    stPartyTeleportConfirm* self) {
    if (self) {
        self->~stPartyTeleportConfirm();
        operator delete(self);
    }
}

// IDA_EA: 0x82318F8
// IDA_NAME: _ZN17stQuickPartyParamC2Ev
// DEMANGLED: stQuickPartyParam::stQuickPartyParam(void)
stQuickPartyParam::stQuickPartyParam()
    : ParamBase(),
      result_code(0) {}

// IDA_EA: 0x8246B46
// IDA_NAME: _ZN17stQuickPartyParamD2Ev
// DEMANGLED: stQuickPartyParam::~stQuickPartyParam()
stQuickPartyParam::~stQuickPartyParam() {}

// IDA_EA: 0x8231140
// IDA_NAME: _ZN19stBossDieCheckParamC2Ev
// DEMANGLED: stBossDieCheckParam::stBossDieCheckParam(void)
stBossDieCheckParam::stBossDieCheckParam()
    : ParamBase(),
      result_code(0),
      map_order_depleted(0),
      acknowledged_monster_unique_id(0),
      cmd183_status(0) {}

// IDA_EA: 0x82478C4
// IDA_NAME: _ZN19stBossDieCheckParamD2Ev
// DEMANGLED: stBossDieCheckParam::~stBossDieCheckParam()
// IDA_EA: 0x82478F4
// IDA_NAME: _ZN19stBossDieCheckParamD0Ev
// DEMANGLED: stBossDieCheckParam::~stBossDieCheckParam() deleting destructor
stBossDieCheckParam::~stBossDieCheckParam() {}

// IDA_EA: 0x8246B76
// IDA_NAME: _ZN17stQuickPartyParamD0Ev
// DEMANGLED: stQuickPartyParam::~stQuickPartyParam() deleting destructor
void stQuickPartyParam_deleting_destructor(stQuickPartyParam* self) {
    if (self) {
        self->~stQuickPartyParam();
        operator delete(self);
    }
}

// IDA_EA: 0x8230D4C
// IDA_NAME: _ZN14stOnPvPTimeOutC2Ev
// DEMANGLED: stOnPvPTimeOut::stOnPvPTimeOut(void)
stOnPvPTimeOut::stOnPvPTimeOut()
    : ParamBase(),
      room_id(-1) {}

// IDA_EA: 0x8247FC6
// IDA_NAME: _ZN14stOnPvPTimeOutD2Ev
// DEMANGLED: stOnPvPTimeOut::~stOnPvPTimeOut()
stOnPvPTimeOut::~stOnPvPTimeOut() {}

// IDA_EA: 0x8247FF6
// IDA_NAME: _ZN14stOnPvPTimeOutD0Ev
// DEMANGLED: stOnPvPTimeOut::~stOnPvPTimeOut() deleting destructor
void stOnPvPTimeOut_deleting_destructor(stOnPvPTimeOut* self) {
    if (self) {
        self->~stOnPvPTimeOut();
        operator delete(self);
    }
}

// IDA_EA: 0x8230E5A
// IDA_NAME: _ZN20stOnSetPvPReadyParamC2Ev
// DEMANGLED: stOnSetPvPReadyParam::stOnSetPvPReadyParam(void)
stOnSetPvPReadyParam::stOnSetPvPReadyParam()
    : ParamBase(),
      raw_04(0) {}

// IDA_EA: 0x8247E40
// IDA_NAME: _ZN20stOnSetPvPReadyParamD2Ev
// DEMANGLED: stOnSetPvPReadyParam::~stOnSetPvPReadyParam()
stOnSetPvPReadyParam::~stOnSetPvPReadyParam() {}

// IDA_EA: 0x8247E70
// IDA_NAME: _ZN20stOnSetPvPReadyParamD0Ev
// DEMANGLED: stOnSetPvPReadyParam::~stOnSetPvPReadyParam() deleting destructor
void stOnSetPvPReadyParam_deleting_destructor(
    stOnSetPvPReadyParam* self) {
    if (self) {
        self->~stOnSetPvPReadyParam();
        operator delete(self);
    }
}

// IDA_EA: 0x8230CAE
// IDA_NAME: _ZN14stSetPartyInfoC2Ev
// DEMANGLED: stSetPartyInfo::stSetPartyInfo(void)
stSetPartyInfo::stSetPartyInfo()
    : ParamBase(),
      raw_04(0) {}

// IDA_EA: 0x82480B0
// IDA_NAME: _ZN14stSetPartyInfoD2Ev
// DEMANGLED: stSetPartyInfo::~stSetPartyInfo()
stSetPartyInfo::~stSetPartyInfo() {}

// IDA_EA: 0x82480E0
// IDA_NAME: _ZN14stSetPartyInfoD0Ev
// DEMANGLED: stSetPartyInfo::~stSetPartyInfo() deleting destructor
void stSetPartyInfo_deleting_destructor(stSetPartyInfo* self) {
    if (self) {
        self->~stSetPartyInfo();
        operator delete(self);
    }
}

// IDA_EA: 0x8230DD8
// IDA_NAME: _ZN20stSelectDungeonParamC2Ev
// DEMANGLED: stSelectDungeonParam::stSelectDungeonParam(void)
stSelectDungeonParam::stSelectDungeonParam()
    : ParamBase(),
      raw_04(0),
      raw_08(0),
      raw_0c(0),
      raw_10(2),
      raw_14(0) {}

// IDA_EA: 0x8247EDC
// IDA_NAME: _ZN20stSelectDungeonParamD2Ev
// DEMANGLED: stSelectDungeonParam::~stSelectDungeonParam()
stSelectDungeonParam::~stSelectDungeonParam() {}

// IDA_EA: 0x8247F0C
// IDA_NAME: _ZN20stSelectDungeonParamD0Ev
// DEMANGLED: stSelectDungeonParam::~stSelectDungeonParam() deleting destructor
void stSelectDungeonParam_deleting_destructor(
    stSelectDungeonParam* self) {
    if (self) {
        self->~stSelectDungeonParam();
        operator delete(self);
    }
}

// IDA_EA: 0x8231700
// IDA_NAME: _ZN19stChangeHostWarRoomC2Ev
// DEMANGLED: stChangeHostWarRoom::stChangeHostWarRoom(void)
stChangeHostWarRoom::stChangeHostWarRoom()
    : ParamBase(),
      result_code(0) {}

// IDA_EA: 0x8246DCC
// IDA_NAME: _ZN19stChangeHostWarRoomD2Ev
// DEMANGLED: stChangeHostWarRoom::~stChangeHostWarRoom()
stChangeHostWarRoom::~stChangeHostWarRoom() {}

// IDA_EA: 0x8246DFC
// IDA_NAME: _ZN19stChangeHostWarRoomD0Ev
// DEMANGLED: stChangeHostWarRoom::~stChangeHostWarRoom() deleting destructor
void stChangeHostWarRoom_deleting_destructor(
    stChangeHostWarRoom* self) {
    if (self) {
        self->~stChangeHostWarRoom();
        operator delete(self);
    }
}

// IDA_EA: 0x8230F3E
// IDA_NAME: _ZN19stWalkoutPartyParamC2Ev
// DEMANGLED: stWalkoutPartyParam::stWalkoutPartyParam(void)
stWalkoutPartyParam::stWalkoutPartyParam()
    : ParamBase(),
      raw_04(0) {}

// IDA_EA: 0x8247CBA
// IDA_NAME: _ZN19stWalkoutPartyParamD2Ev
// DEMANGLED: stWalkoutPartyParam::~stWalkoutPartyParam()
stWalkoutPartyParam::~stWalkoutPartyParam() {}

// IDA_EA: 0x8247CEA
// IDA_NAME: _ZN19stWalkoutPartyParamD0Ev
// DEMANGLED: stWalkoutPartyParam::~stWalkoutPartyParam() deleting destructor
void stWalkoutPartyParam_deleting_destructor(
    stWalkoutPartyParam* self) {
    if (self) {
        self->~stWalkoutPartyParam();
        operator delete(self);
    }
}

// IDA_EA: 0x8231606
// IDA_NAME: _ZN11stWarRoomWpC2Ev
// DEMANGLED: stWarRoomWp::stWarRoomWp(void)
stWarRoomWp::stWarRoomWp()
    : ParamBase(),
      raw_04(0) {}

// IDA_EA: 0x8246FA0
// IDA_NAME: _ZN11stWarRoomWpD2Ev
// DEMANGLED: stWarRoomWp::~stWarRoomWp()
stWarRoomWp::~stWarRoomWp() {}

// IDA_EA: 0x8246FD0
// IDA_NAME: _ZN11stWarRoomWpD0Ev
// DEMANGLED: stWarRoomWp::~stWarRoomWp() deleting destructor
void stWarRoomWp_deleting_destructor(stWarRoomWp* self) {
    if (self) {
        self->~stWarRoomWp();
        operator delete(self);
    }
}

// IDA_EA: 0x8249CA0
// IDA_NAME: _ZN25PcRoomPlayTimeRewardParamC2Ev
// DEMANGLED: PcRoomPlayTimeRewardParam::PcRoomPlayTimeRewardParam(void)
PcRoomPlayTimeRewardParam::PcRoomPlayTimeRewardParam()
    : ParamBase() {}

// IDA_EA: 0x8249CBC
// IDA_NAME: _ZN25PcRoomPlayTimeRewardParamD2Ev
// DEMANGLED: PcRoomPlayTimeRewardParam::~PcRoomPlayTimeRewardParam()
PcRoomPlayTimeRewardParam::~PcRoomPlayTimeRewardParam() {}

// IDA_EA: 0x8249CEC
// IDA_NAME: _ZN25PcRoomPlayTimeRewardParamD0Ev
// DEMANGLED: PcRoomPlayTimeRewardParam::~PcRoomPlayTimeRewardParam() deleting destructor
void PcRoomPlayTimeRewardParam_deleting_destructor(
    PcRoomPlayTimeRewardParam* self) {
    if (self) {
        self->~PcRoomPlayTimeRewardParam();
        operator delete(self);
    }
}

// IDA_EA: 0x8231D60
// IDA_NAME: _ZN20stSeriaRoomDecoEventC2Ev
// DEMANGLED: stSeriaRoomDecoEvent::stSeriaRoomDecoEvent(void)
stSeriaRoomDecoEvent::stSeriaRoomDecoEvent()
    : ParamBase(),
      error_code(0),
      selected_inventory_slot(0),
      deco_flow(0) {}

// IDA_EA: 0x8246660
// IDA_NAME: _ZN20stSeriaRoomDecoEventD2Ev
// DEMANGLED: stSeriaRoomDecoEvent::~stSeriaRoomDecoEvent()
stSeriaRoomDecoEvent::~stSeriaRoomDecoEvent() {}

// IDA_EA: 0x8246690
// IDA_NAME: _ZN20stSeriaRoomDecoEventD0Ev
// DEMANGLED: stSeriaRoomDecoEvent::~stSeriaRoomDecoEvent() deleting destructor
void stSeriaRoomDecoEvent_deleting_destructor(
    stSeriaRoomDecoEvent* self) {
    if (self) {
        self->~stSeriaRoomDecoEvent();
        operator delete(self);
    }
}

// IDA_EA: 0x822EE18
// IDA_NAME: _ZN17seriaRoom_AniDecoC2Ev
// DEMANGLED: seriaRoom_AniDeco::seriaRoom_AniDeco(void)
seriaRoom_AniDeco::seriaRoom_AniDeco()
    : receive_flow(0),
      deco_flow(0) {}

// IDA_EA: 0x822DFA4
// IDA_NAME: _ZN18MSG_PARTY_TELEPORTC2Ev
// DEMANGLED: MSG_PARTY_TELEPORT::MSG_PARTY_TELEPORT(void)
MSG_PARTY_TELEPORT::MSG_PARTY_TELEPORT()
    : MSG_BASE(), village_index(0), area_index(0), position_x(0),
      position_y(0), direction(0) {}

// IDA_EA: 0x822DFDE
// IDA_NAME: _ZN26MSG_PARTY_TELEPORT_CONFIRMC2Ev
// DEMANGLED: MSG_PARTY_TELEPORT_CONFIRM::MSG_PARTY_TELEPORT_CONFIRM(void)
MSG_PARTY_TELEPORT_CONFIRM::MSG_PARTY_TELEPORT_CONFIRM()
    : MSG_BASE(), byte_0d(0) {}

// IDA_EA: 0x822E06A
// IDA_NAME: _ZN15MSG_QUICK_PARTYC2Ev
// DEMANGLED: MSG_QUICK_PARTY::MSG_QUICK_PARTY(void)
MSG_QUICK_PARTY::MSG_QUICK_PARTY()
    : MSG_BASE(), pool_id(0), channel_type(0xffu) {}

// IDA_EA: 0x8568F16
// IDA_NAME: _ZN13PVP_ROOM_INFOC2Ev
// DEMANGLED: PVP_ROOM_INFO::PVP_ROOM_INFO(void)
PVP_ROOM_INFO::PVP_ROOM_INFO()
    : room_title(),
      password() {}

// IDA_EA: 0x8568F5C
// IDA_NAME: _ZN13PVP_ROOM_INFOD2Ev
// DEMANGLED: PVP_ROOM_INFO::~PVP_ROOM_INFO()
PVP_ROOM_INFO::~PVP_ROOM_INFO() {}

// IDA_EA: 0x891B78E
// IDA_NAME: _ZN19stPartyMatchingInfoC2Ev
// DEMANGLED: stPartyMatchingInfo::stPartyMatchingInfo(void)
stPartyMatchingInfo::stPartyMatchingInfo()
    : channel_name(),
      region_name(),
      dungeon_indices() {
    clear();
}

// IDA_EA: 0x891B81C
// IDA_NAME: _ZN19stPartyMatchingInfo5clearEv
// DEMANGLED: stPartyMatchingInfo::clear(void)
int stPartyMatchingInfo::clear() {
    channel_name.clear();
    region_name.clear();
    dungeon_indices.clear();
    return 0;
}

// IDA_EA: 0x891CA12
// IDA_NAME: _ZN19stPartyMatchingInfoD2Ev
// DEMANGLED: stPartyMatchingInfo::~stPartyMatchingInfo()
stPartyMatchingInfo::~stPartyMatchingInfo() {}

// IDA_EA: 0x892EAD6
// IDA_NAME: _ZN19stPartyMatchingInfoC2ERKS_
// DEMANGLED: stPartyMatchingInfo::stPartyMatchingInfo(stPartyMatchingInfo const&)
stPartyMatchingInfo::stPartyMatchingInfo(
    const stPartyMatchingInfo& rhs)
    : channel_name(rhs.channel_name),
      region_name(rhs.region_name),
      dungeon_indices(rhs.dungeon_indices) {}

// IDA_EA: 0x892EBE0
// IDA_NAME: _ZN19stPartyMatchingInfoaSERKS_
// DEMANGLED: stPartyMatchingInfo::operator=(stPartyMatchingInfo const&)
stPartyMatchingInfo& stPartyMatchingInfo::operator=(
    const stPartyMatchingInfo& rhs) {
    channel_name = rhs.channel_name;
    region_name = rhs.region_name;
    dungeon_indices = rhs.dungeon_indices;
    return *this;
}

// IDA_EA: 0x836A9BC
// IDA_NAME: _ZN26stConditionEnterBossRoom_t5clearEv
// DEMANGLED: stConditionEnterBossRoom_t::clear(void)
int stConditionEnterBossRoom_t::clear() {
    enabled = 0;
    conditions.clear();
    return 0;
}

// IDA_EA: 0x836B552
// IDA_NAME: _ZN26stConditionEnterBossRoom_tC2Ev
// DEMANGLED: stConditionEnterBossRoom_t::stConditionEnterBossRoom_t(void)
stConditionEnterBossRoom_t::stConditionEnterBossRoom_t()
    : conditions() {}

// IDA_EA: 0x836B568
// IDA_NAME: _ZN26stConditionEnterBossRoom_tD2Ev
// DEMANGLED: stConditionEnterBossRoom_t::~stConditionEnterBossRoom_t()
stConditionEnterBossRoom_t::~stConditionEnterBossRoom_t() {}

// IDA_EA: 0x82A44C8
// IDA_NAME: _ZN13CBattle_Field25CTowerOfDespairRewardDataC2Ev
// DEMANGLED: CBattle_Field::CTowerOfDespairRewardData::CTowerOfDespairRewardData(void)
CBattle_Field::CTowerOfDespairRewardData::CTowerOfDespairRewardData() {}

// IDA_EA: 0x82A44EA
// IDA_NAME: _ZN13CBattle_Field25CTowerOfDespairRewardDataD2Ev
// DEMANGLED: CBattle_Field::CTowerOfDespairRewardData::~CTowerOfDespairRewardData()
CBattle_Field::CTowerOfDespairRewardData::~CTowerOfDespairRewardData() {}

// IDA_EA: 0x83071A6
// IDA_NAME: _ZN13CBattle_Field25CTowerOfDespairRewardData9reset_todEv
// DEMANGLED: CBattle_Field::CTowerOfDespairRewardData::reset_tod(void)
void CBattle_Field::CTowerOfDespairRewardData::reset_tod() {
    today_reward_items.clear();
}

// IDA_EA: 0x830EE22
// IDA_NAME: _ZN13CBattle_Field32stConditionEnterBossRoomStatus_tC2Ev
// DEMANGLED: CBattle_Field::stConditionEnterBossRoomStatus_t::stConditionEnterBossRoomStatus_t(void)
CBattle_Field::stConditionEnterBossRoomStatus_t::stConditionEnterBossRoomStatus_t() {}

// IDA_EA: 0x82D2C00
// IDA_NAME: _ZN13CBattle_Field32stConditionEnterBossRoomStatus_tD2Ev
// DEMANGLED: CBattle_Field::stConditionEnterBossRoomStatus_t::~stConditionEnterBossRoomStatus_t()
CBattle_Field::stConditionEnterBossRoomStatus_t::~stConditionEnterBossRoomStatus_t() {}

// IDA_EA: 0x8312EE4
// IDA_NAME: _ZN13CBattle_Field32stConditionEnterBossRoomStatus_tC2ERKS0_
// DEMANGLED: CBattle_Field::stConditionEnterBossRoomStatus_t::stConditionEnterBossRoomStatus_t(CBattle_Field::stConditionEnterBossRoomStatus_t const&)
CBattle_Field::stConditionEnterBossRoomStatus_t::stConditionEnterBossRoomStatus_t(
    const stConditionEnterBossRoomStatus_t& rhs)
    : condition_type(rhs.condition_type),
      condition_values(rhs.condition_values),
      boss_room_id(rhs.boss_room_id),
      status(rhs.status) {}

// IDA_EA: 0x8312F84
// IDA_NAME: _ZN13CBattle_Field32stConditionEnterBossRoomStatus_taSERKS0_
// DEMANGLED: CBattle_Field::stConditionEnterBossRoomStatus_t::operator=(CBattle_Field::stConditionEnterBossRoomStatus_t const&)
CBattle_Field::stConditionEnterBossRoomStatus_t&
CBattle_Field::stConditionEnterBossRoomStatus_t::operator=(
    const stConditionEnterBossRoomStatus_t& rhs) {
    condition_type = rhs.condition_type;
    condition_values = rhs.condition_values;
    boss_room_id = rhs.boss_room_id;
    status = rhs.status;
    return *this;
}

namespace pvp_assault {

// IDA_EA: 0x82E564C
// IDA_NAME: _ZN11pvp_assault18GetDirectionByTeamE17ENUM_ASSAULT_TEAM
// DEMANGLED: pvp_assault::GetDirectionByTeam(ENUM_ASSAULT_TEAM)
int GetDirectionByTeam(ENUM_ASSAULT_TEAM team) {
    int direction = 0;
    if (static_cast<int>(team) == 0) {
        return 6;
    }
    if (static_cast<int>(team) == 1) {
        return 4;
    }
    return direction;
}

// IDA_EA: 0x82F046A
// IDA_NAME: _ZN11pvp_assault7Point2dC2Ev
// DEMANGLED: pvp_assault::Point2d::Point2d(void)
Point2d::Point2d() {}

// IDA_EA: 0x82F0470
// IDA_NAME: _ZN11pvp_assault7Point2dC2Eii
// DEMANGLED: pvp_assault::Point2d::Point2d(int,int)
Point2d::Point2d(int x_value, int y_value)
    : x(x_value),
      y(y_value) {}

// IDA_EA: 0x82F0486
// IDA_NAME: _ZNK11pvp_assault7Point2dmiERKS0_
// DEMANGLED: pvp_assault::Point2d::operator-(pvp_assault::Point2d const&)const
unsigned int Point2d::operator-(const Point2d& rhs) const {
    int delta_x = x - rhs.x;
    int delta_y = y - rhs.y;
    if (delta_x < 0) {
        delta_x = -delta_x;
    }
    if (delta_y < 0) {
        delta_y = -delta_y;
    }
    return static_cast<unsigned int>(delta_x + delta_y);
}

// IDA_EA: 0x82F04C0
// IDA_NAME: _ZN11pvp_assault7Point2dpLERKS0_
// DEMANGLED: pvp_assault::Point2d::operator+=(pvp_assault::Point2d const&)
Point2d& Point2d::operator+=(const Point2d& rhs) {
    x += rhs.x;
    y += rhs.y;
    return *this;
}

// IDA_EA: 0x82F34C0
// IDA_NAME: _ZN11pvp_assault7Point2dD2Ev
// DEMANGLED: pvp_assault::Point2d::~Point2d()
Point2d::~Point2d() {}

// IDA_EA: 0x82F04EA
// IDA_NAME: _ZNK11pvp_assault14CandidatePointltERKS0_
// DEMANGLED: pvp_assault::CandidatePoint::operator<(pvp_assault::CandidatePoint const&)const
bool CandidatePoint::operator<(const CandidatePoint& rhs) const {
    return score > rhs.score;
}

// IDA_EA: 0x82F103C
// IDA_NAME: _ZN11pvp_assault14CandidatePointC2Ev
// DEMANGLED: pvp_assault::CandidatePoint::CandidatePoint(void)
CandidatePoint::CandidatePoint()
    : score(0),
      point() {}

// IDA_EA: 0x82F264A
// IDA_NAME: _ZN11pvp_assault14CandidatePointD2Ev
// DEMANGLED: pvp_assault::CandidatePoint::~CandidatePoint()
CandidatePoint::~CandidatePoint() {}

// IDA_EA: 0x82F05EE
// IDA_NAME: _ZN11pvp_assault9PartyInfoC2Ev
// DEMANGLED: pvp_assault::PartyInfo::PartyInfo(void)
PartyInfo::PartyInfo()
    : party(0) {}

// IDA_EA: 0x82F05FC
// IDA_NAME: _ZN11pvp_assault9PartyInfo3SetEP6CParty
// DEMANGLED: pvp_assault::PartyInfo::Set(CParty *)
PartyInfo* PartyInfo::Set(CParty* party_value) {
    party = party_value;
    return this;
}

// IDA_EA: 0x82F060A
// IDA_NAME: _ZN11pvp_assault9PartyInfo3GetEv
// DEMANGLED: pvp_assault::PartyInfo::Get(void)
CParty* PartyInfo::Get() {
    return party;
}

}  // namespace pvp_assault

namespace fair_pvp {

// IDA_EA: 0x84A63F2
// IDA_NAME: _ZNK8fair_pvp6JobKeyltERKS0_
// DEMANGLED: fair_pvp::JobKey::operator<(fair_pvp::JobKey const&)const
bool JobKey::operator<(const JobKey& rhs) const {
    if (job < rhs.job) {
        return true;
    }
    return job == rhs.job && grow_type < rhs.grow_type;
}

// IDA_EA: 0x84A643A
// IDA_NAME: _ZN8fair_pvp6JobKeyC2E17ENUM_CHARACTERJOBh
// DEMANGLED: fair_pvp::JobKey::JobKey(ENUM_CHARACTERJOB,unsigned char)
JobKey::JobKey(ENUM_CHARACTERJOB job_value, uint8_t grow_type_value)
    : job(static_cast<int>(job_value)),
      grow_type(grow_type_value) {}

}  // namespace fair_pvp

namespace QuickParty {

// IDA_EA: 0x8A6E100
// IDA_NAME: _ZN10QuickParty23getRandomBuffTypeByNameERKSs
// DEMANGLED: QuickParty::getRandomBuffTypeByName(std::string const&)
int getRandomBuffTypeByName(const std::string& name) {
    int result = 11;
    if (name == "[increase exp]") {
        return 0;
    }
    if (name == "[increase stats]") {
        return 1;
    }
    if (name == "[increase speed]") {
        return 2;
    }
    if (name == "[increase dropprob]") {
        return 3;
    }
    if (name == "[increase allattack]") {
        return 4;
    }
    if (name == "[increase allresist]") {
        return 5;
    }
    if (name == "[increase broken]") {
        return 6;
    }
    if (name == "[increase mobhp]") {
        return 7;
    }
    if (name == "[increase antidevil]") {
        return 8;
    }
    if (name == "[increase coinlimit]") {
        return 9;
    }
    if (name == "[return ticket]") {
        return 10;
    }
    return result;
}

// IDA_EA: 0x8469976
// IDA_NAME: _ZN10QuickParty16RewardData_ParamC2EP6CParty
// DEMANGLED: QuickParty::RewardData_Param::RewardData_Param(CParty *)
RewardData_Param::RewardData_Param(CParty* party_value)
    : party(party_value),
      raw_04(0) {}

// IDA_EA: 0x846998E
// IDA_NAME: _ZN10QuickParty17RewardData_Result5clearEv
// DEMANGLED: QuickParty::RewardData_Result::clear(void)
RewardData_Result* RewardData_Result::clear() {
    std::memset(this, 0, 0x14u);
    raw_14 = 1.0f;
    raw_18 = 1;
    return this;
}

// IDA_EA: 0x84699C6
// IDA_NAME: _ZN10QuickParty17RewardData_ResultC2Ev
// DEMANGLED: QuickParty::RewardData_Result::RewardData_Result(void)
RewardData_Result::RewardData_Result() {
    clear();
}

// IDA_EA: 0x826DBEE
// IDA_NAME: _ZN10QuickParty11IRewardTypeC2ERNS_14QuickPartyDataE
// DEMANGLED: QuickParty::IRewardType::IRewardType(QuickParty::QuickPartyData &)
IRewardType::IRewardType(QuickPartyData& data)
    : quick_party_data(&data) {}

// IDA_EA: 0x826DC06
// IDA_NAME: _ZN10QuickParty12IBasicRewardC2ERNS_14QuickPartyDataE
// DEMANGLED: QuickParty::IBasicReward::IBasicReward(QuickParty::QuickPartyData &)
IBasicReward::IBasicReward(QuickPartyData& data)
    : IRewardType(data) {}

// IDA_EA: 0x826DC2A
// IDA_NAME: _ZN10QuickParty12IBasicReward5checkENS_16RewardData_ParamE
// DEMANGLED: QuickParty::IBasicReward::check(QuickParty::RewardData_Param)
void IBasicReward::check(RewardData_Param param) {
    (void)param;
}

// IDA_EA: 0x826DC78
// IDA_NAME: _ZN10QuickParty14IRandomwRewardC2ERNS_14QuickPartyDataE
// DEMANGLED: QuickParty::IRandomwReward::IRandomwReward(QuickParty::QuickPartyData &)
IRandomwReward::IRandomwReward(QuickPartyData& data)
    : IRewardType(data) {}

// IDA_EA: 0x826DC9C
// IDA_NAME: _ZN10QuickParty14IRandomwReward5checkENS_16RewardData_ParamE
// DEMANGLED: QuickParty::IRandomwReward::check(QuickParty::RewardData_Param)
void IRandomwReward::check(RewardData_Param param) {
    (void)param;
}

// IDA_EA: 0x8A6EF72
// IDA_NAME: _ZN10QuickParty9LevelDataC2Ev
// DEMANGLED: QuickParty::LevelData::LevelData(void)
LevelData::LevelData()
    : raw_00(0),
      raw_04(-1) {}

// IDA_EA: 0x8A6FCB8
// IDA_NAME: _ZN10QuickParty9LevelDataD2Ev
// DEMANGLED: QuickParty::LevelData::~LevelData()
LevelData::~LevelData() {}

// IDA_EA: 0x8A6EF8A
// IDA_NAME: _ZN10QuickParty13LevelDataHellC2Ev
// DEMANGLED: QuickParty::LevelDataHell::LevelDataHell(void)
LevelDataHell::LevelDataHell()
    : raw_00(-1),
      raw_04(-1) {}

// IDA_EA: 0x8A6FEFA
// IDA_NAME: _ZN10QuickParty13LevelDataHellD2Ev
// DEMANGLED: QuickParty::LevelDataHell::~LevelDataHell()
LevelDataHell::~LevelDataHell() {}

// IDA_EA: 0x826D9F0
// IDA_NAME: _ZN10QuickParty11BasicRewardC2Ev
// DEMANGLED: QuickParty::BasicReward::BasicReward(void)
BasicReward::BasicReward()
    : level_data(),
      hell_level_data(),
      raw_18(-1),
      raw_1c(-1) {
    level_data.clear();
    hell_level_data.clear();
}

// IDA_EA: 0x826DA7A
// IDA_NAME: _ZN10QuickParty11BasicRewardD2Ev
// DEMANGLED: QuickParty::BasicReward::~BasicReward()
BasicReward::~BasicReward() {}

// IDA_EA: 0x826E77E
// IDA_NAME: _ZN10QuickParty21LevelDataRandomRewardD2Ev
// DEMANGLED: QuickParty::LevelDataRandomReward::~LevelDataRandomReward()
LevelDataRandomReward::~LevelDataRandomReward() {}

// IDA_EA: 0x8A6EFA2
// IDA_NAME: _ZN10QuickParty21LevelDataRandomRewardC2Ev
// DEMANGLED: QuickParty::LevelDataRandomReward::LevelDataRandomReward(void)
LevelDataRandomReward::LevelDataRandomReward()
    : raw_00(11),
      text_04(),
      level_data_08() {
    level_data_08.clear();
    text_04.clear();
}

// IDA_EA: 0x8A6F112
// IDA_NAME: _ZN10QuickParty21LevelDataRandomRewardC2ERKS0_
// DEMANGLED: QuickParty::LevelDataRandomReward::LevelDataRandomReward(QuickParty::LevelDataRandomReward const&)
LevelDataRandomReward::LevelDataRandomReward(
    const LevelDataRandomReward& rhs)
    : raw_00(rhs.raw_00),
      text_04(rhs.text_04),
      level_data_08(rhs.level_data_08) {}

// IDA_EA: 0x826DAD4
// IDA_NAME: _ZN10QuickParty14QuickPartyDataC2Ev
// DEMANGLED: QuickParty::QuickPartyData::QuickPartyData(void)
QuickPartyData::QuickPartyData()
    : unable_to_quick_party(),
      dungeon_idx_list(),
      basic_reward(),
      random_rewards() {
    unable_to_quick_party.clear();
    dungeon_idx_list.clear();
    random_rewards.clear();
    for (int row = 0; ; ++row) {
        if (row > 1) {
            break;
        }
        for (int column = 0; column <= 1; ++column) {
            require_quick_party_min_count[row][column] = -1;
        }
    }
}

// IDA_EA: 0x826DD52
// IDA_NAME: _ZN10QuickParty14QuickPartyDataD2Ev
// DEMANGLED: QuickParty::QuickPartyData::~QuickPartyData()
QuickPartyData::~QuickPartyData() {}

// IDA_EA: 0x826D688
// IDA_NAME: _ZN10QuickParty24CQuickPartyRewardManagerC2Ev
// DEMANGLED: QuickParty::CQuickPartyRewardManager::CQuickPartyRewardManager(void)
CQuickPartyRewardManager::CQuickPartyRewardManager()
    : QuickPartyData() {}

// IDA_EA: 0x826D69C
// IDA_NAME: _ZN10QuickParty24CQuickPartyRewardManagerD2Ev
// DEMANGLED: QuickParty::CQuickPartyRewardManager::~CQuickPartyRewardManager()
CQuickPartyRewardManager::~CQuickPartyRewardManager() {}

// IDA_EA: 0x826D860
// IDA_NAME: _ZN10QuickParty24CQuickPartyRewardManager20get_random_buff_probENS_14RandomBuffTypeENS_17RandomBuffDungeonE
// DEMANGLED: QuickParty::CQuickPartyRewardManager::get_random_buff_prob(QuickParty::RandomBuffType,QuickParty::RandomBuffDungeon)
int CQuickPartyRewardManager::get_random_buff_prob(
    RandomBuffType random_buff_type,
    RandomBuffDungeon random_buff_dungeon) {
    if (random_buff_type <= 11 && random_buff_dungeon <= 6) {
        return random_buff_probs[random_buff_type][random_buff_dungeon];
    }
    return 0;
}

// IDA_EA: 0x826D896
// IDA_NAME: _ZN10QuickParty24CQuickPartyRewardManager20get_dungeon_idx_listEv
// DEMANGLED: QuickParty::CQuickPartyRewardManager::get_dungeon_idx_list(void)
std::vector<int>* CQuickPartyRewardManager::get_dungeon_idx_list() {
    return &dungeon_idx_list;
}

// IDA_EA: 0x826D8A2
// IDA_NAME: _ZN10QuickParty24CQuickPartyRewardManager28getRequireQuickPartyMinCountEii
// DEMANGLED: QuickParty::CQuickPartyRewardManager::getRequireQuickPartyMinCount(int,int)
int CQuickPartyRewardManager::getRequireQuickPartyMinCount(
    int row,
    int column) {
    if (row < 0 || row > 1) {
        return -1;
    }
    if (column < 0 || column > 3) {
        return -1;
    }
    return require_quick_party_min_count[row][column];
}

// IDA_EA: 0x826D8E2
// IDA_NAME: _ZN10QuickParty24CQuickPartyRewardManager22get_reward_type_objectENS_16ENUM_REWARD_TYPEEj
// DEMANGLED: QuickParty::CQuickPartyRewardManager::get_reward_type_object(QuickParty::ENUM_REWARD_TYPE,unsigned int)
void* CQuickPartyRewardManager::get_reward_type_object(
    ENUM_REWARD_TYPE reward_type,
    unsigned int index) {
    const int reward_type_index = static_cast<int>(reward_type);
    if (reward_type_index <= 1 && index <= 12) {
        return reward_type_objects[reward_type_index][index];
    }
    return 0;
}

// IDA_EA: 0x826D91A
// IDA_NAME: _ZN10QuickParty24CQuickPartyRewardManager24set_rand_buff_total_probEv
// DEMANGLED: QuickParty::CQuickPartyRewardManager::set_rand_buff_total_prob(void)
bool CQuickPartyRewardManager::set_rand_buff_total_prob() {
    bool result = false;
    for (int dungeon = 0; ; ++dungeon) {
        result = dungeon <= 6;
        if (dungeon > 6) {
            break;
        }
        int total = 0;
        for (int buff_type = 0; buff_type <= 10; ++buff_type) {
            total += random_buff_probs[buff_type][dungeon];
        }
        random_buff_probs[11][dungeon] = total;
    }
    return result;
}

}  // namespace QuickParty
