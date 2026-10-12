#include "old_data/PVF/pvp_mission/datamanager/pvp_mission_system.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/datamanager/script_file_list.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "common/manager/user_history_log_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/db/character/load_etc_payload.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/progression/quest/character_mission_list.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/mt_rand.h"

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <new>
#include <string>
#include <utility>

extern char g_PvpMissionScriptBaseDirectory[256];
extern STScriptFileList g_PvpMissionScriptFileList;

bool loadRDARScriptFile(const char* dir, const char* filename);
typedef char MissionScript_Size_Must_Be_0x88[
    (sizeof(MissionScript) == 0x88) ? 1 : -1];
typedef char MissionParameterScript_Size_Must_Be_0x28[
    (sizeof(MissionParameterScript) == 0x28) ? 1 : -1];
typedef char CMission_Size_Must_Be_0x40[(sizeof(CMission) == 0x40) ? 1 : -1];
typedef char MissionClearCondition_Parameter_Size_Must_Be_0x0f[
    (sizeof(MissionClearCondition_Parameter) == 0x0f) ? 1 : -1];
typedef char CMission_winning_count_Size_Must_Be_0x44[
    (sizeof(CMission_winning_count) == 0x44) ? 1 : -1];
typedef char CMission_winning_streak_Size_Must_Be_0x48[
    (sizeof(CMission_winning_streak) == 0x48) ? 1 : -1];
typedef char CMission_within_Size_Must_Be_0x4c[
    (sizeof(CMission_within) == 0x4c) ? 1 : -1];
typedef char CMission_rank_Size_Must_Be_0x44[
    (sizeof(CMission_rank) == 0x44) ? 1 : -1];
typedef char CMission_combo_clear_Size_Must_Be_0x58[
    (sizeof(CMission_combo_clear) == 0x58) ? 1 : -1];
typedef char PvP_MissionSystem_Size_Must_Be_0x54[
    (sizeof(PvP_MissionSystem) == 0x54) ? 1 : -1];

namespace {

enum MissionCondition {
    kWinningCount = 0,
    kWinningStreak = 1,
    kJoin = 2,
    kKillCount = 3,
    kHereafterJoin = 4,
    kHereafterWinningCount = 5,
    kRemainHp = 6,
    kRevenge = 7,
    kHereafterKillCount = 8,
    kKill = 9,
    kWithin = 11,
    kRemainSHp = 12,
    kMoveChannel = 13,
    kMoveChannelTotal = 14,
    kComboClear = 15,
    kNoCondition = 16
};

}  // namespace

// IDA_EA: 0x85E1A44
// IDA_NAME: _ZN31MissionClearCondition_ParameterC2E15PVP_BATTLE_MODE
// DEMANGLED: MissionClearCondition_Parameter::MissionClearCondition_Parameter(PVP_BATTLE_MODE)
MissionClearCondition_Parameter::MissionClearCondition_Parameter(
    PVP_BATTLE_MODE battle_mode) {
    battle_mode_category = 10;
    if (battle_mode == PVP_BATTLE_MODE_2) {
        battle_mode_category = 1;
    } else if (battle_mode == PVP_BATTLE_MODE_3) {
        battle_mode_category = 2;
    }
}

// IDA_EA: 0x8A5AA4E
// IDA_NAME: _Z19is_Victory_in_a_rowSt6bitsetILj32EEs
// DEMANGLED: is_Victory_in_a_row(std::bitset<32u>,short)
bool is_Victory_in_a_row(std::bitset<32> issue_history,
                         short required_count) {
    std::bitset<32> mask;
    for (short i = 0; i < required_count; ++i) {
        mask.set(static_cast<size_t>(i));
    }
    return (issue_history & mask) == mask;
}

// IDA_EA: 0x8A5AB91
// IDA_NAME: _Z13_ReadThreeIntP13MissionScript
// DEMANGLED: _ReadThreeInt(MissionScript *)
int _ReadThreeInt(MissionScript* script) {
    script->pvp_battle_mode = ScanInt(static_cast<bool*>(0));
    script->condition_param1 = ScanInt(static_cast<bool*>(0));
    script->condition_param2 = ScanInt(static_cast<bool*>(0));
    return script->condition_param2;
}

// IDA_EA: 0x8A5ABD2
// IDA_NAME: _Z12_ReadFourIntP13MissionScript
// DEMANGLED: _ReadFourInt(MissionScript *)
int _ReadFourInt(MissionScript* script) {
    _ReadThreeInt(script);
    script->condition_param3 = ScanInt(static_cast<bool*>(0));
    return script->condition_param3;
}

// IDA_EA: 0x85E6ED8
// IDA_NAME: _ZN13MissionScriptD2Ev
// DEMANGLED: MissionScript::~MissionScript()
MissionScript::~MissionScript() {}

// IDA_EA: 0x8A5BA8C
// IDA_NAME: _ZN13MissionScriptC2Ev
// DEMANGLED: MissionScript::MissionScript(void)
MissionScript::MissionScript() {
    clear();
}

// IDA_EA: 0x8A5BBB2
// IDA_NAME: _ZN13MissionScript5clearEv
// DEMANGLED: MissionScript::clear(void)
MissionScript* MissionScript::clear() {
    rank_range = std::make_pair(0, 0);
    mission_grade = 4;
    target_character = 11;
    pre_required_missions.clear();
    condition_type = kNoCondition;
    next_mission_index = -1;
    prev_mission_index = -1;
    name_text.clear();
    condition_text.clear();
    reward_items.clear();
    difficulty_ratio = 0.0f;
    stealing_skill = false;
    reward_skill = false;
    reward_sp = false;
    pad_5f = 0;
    combo_clear.clear();
    mission_kind = 0;
    script_index = 0;
    return this;
}

// IDA_EA: 0x85E6FC4
// IDA_NAME: _ZN22MissionParameterScriptC2Ev
// DEMANGLED: MissionParameterScript::MissionParameterScript(void)
MissionParameterScript::MissionParameterScript() {}

// IDA_EA: 0x85E7030
// IDA_NAME: _ZN22MissionParameterScriptD2Ev
// DEMANGLED: MissionParameterScript::~MissionParameterScript()
MissionParameterScript::~MissionParameterScript() {}

// IDA_EA: 0x8A5BCA2
// IDA_NAME: _ZN22MissionParameterScript5clearEv
// DEMANGLED: MissionParameterScript::clear(void)
int MissionParameterScript::clear() {
    frequent_daily_missions.clear();
    casual_daily_count = 0;
    casual_daily_missions.clear();
    casual_daily_total_weight = 0;
    base_mission_exp_by_rank.clear();
    return 0;
}

// IDA_EA: 0x85E2FFC
// IDA_NAME: _ZN8CMissionC2ERK13MissionScript
// DEMANGLED: CMission::CMission(MissionScript const&)
CMission::CMission(const MissionScript& script)
    : pvp_battle_mode_(static_cast<unsigned char>(script.pvp_battle_mode)),
      mission_kind_(script.mission_kind),
      mission_index_(script.script_index),
      next_mission_index_(script.next_mission_index),
      prev_mission_index_(script.prev_mission_index),
      rank_range_(script.rank_range),
      rate_range_(script.rate_range),
      mission_grade_(script.mission_grade),
      difficulty_ratio_(script.difficulty_ratio),
      reward_items_(script.reward_items),
      target_character_(script.target_character) {}

// IDA_EA: 0x85E1A6A
// IDA_NAME: _ZN8CMissionD2Ev
// DEMANGLED: CMission::~CMission()
// IDA_EA: 0x85E1A9E
// IDA_NAME: _ZN8CMissionD0Ev
// DEMANGLED: CMission::~CMission() deleting destructor
CMission::~CMission() {}

// IDA_EA: 0x85E3188
// IDA_NAME: _ZNK8CMission21_makeTotalMissionInfoER11MissionInfoRK5CUser
// DEMANGLED: CMission::_makeTotalMissionInfo(MissionInfo &,CUser const&)const
void CMission::_makeTotalMissionInfo(MissionInfo&, const CUser&) const {}

// IDA_EA: 0x85E68F2
// IDA_NAME: _ZNK8CMission10_canUpdateERK31MissionClearCondition_Parameter
// DEMANGLED: CMission::_canUpdate(MissionClearCondition_Parameter const&)const
bool CMission::_canUpdate(
    const MissionClearCondition_Parameter&) const {
    return true;
}

// IDA_EA: 0x85E68FC
// IDA_NAME: _ZNK8CMission18_isAcceptConditionERK5CUser
// DEMANGLED: CMission::_isAcceptCondition(CUser const&)const
bool CMission::_isAcceptCondition(const CUser&) const {
    return true;
}

// IDA_EA: 0x85E6950
// IDA_NAME: _ZNK8CMission7_RewardER5CUser
// DEMANGLED: CMission::_Reward(CUser &)const
int CMission::_Reward(CUser&) const {
    return 1;
}

// IDA_EA: 0x85E3714
// IDA_NAME: _ZNK13CMission_rank11isClearableER11MissionInfoRb
// DEMANGLED: CMission_rank::isClearable(MissionInfo &,bool &)const
bool CMission_rank::isClearable(MissionInfo& info, bool&) const {
    return condition_ <= info.current_value;
}

// IDA_EA: 0x85E372C
// IDA_NAME: _ZNK13CMission_rank17isAcceptConditionERK5CUser
// DEMANGLED: CMission_rank::isAcceptCondition(CUser const&)const
bool CMission_rank::isAcceptCondition(const CUser& user) const {
    const int grade = user.get_pvp_grade();
    return rank_range_.first <= grade && grade < rank_range_.second;
}

// IDA_EA: 0x85E3780
// IDA_NAME: _ZNK13CMission_rank6UpdateER11MissionInfoRK31MissionClearCondition_Parameter
// DEMANGLED: CMission_rank::Update(MissionInfo &,MissionClearCondition_Parameter const&)const
int CMission_rank::Update(MissionInfo&,
                          const MissionClearCondition_Parameter&) const {
    return 1;
}

// IDA_EA: 0x85E6A84
// IDA_NAME: _ZN13CMission_rankD2Ev
// DEMANGLED: CMission_rank::~CMission_rank()
// IDA_EA: 0x85E6AB4
// IDA_NAME: _ZN13CMission_rankD0Ev
// DEMANGLED: CMission_rank::~CMission_rank() deleting destructor
CMission_rank::~CMission_rank() {}

// IDA_EA: 0x85E6AD2
// IDA_NAME: _ZNK13CMission_rank15isSeriesMissionEv
// DEMANGLED: CMission_rank::isSeriesMission(void)const
bool CMission_rank::isSeriesMission() const {
    return false;
}

// IDA_EA: 0x85E6ADC
// IDA_NAME: _ZNK13CMission_rank17getClearConditionEv
// DEMANGLED: CMission_rank::getClearCondition(void)const
int CMission_rank::getClearCondition() const {
    return condition_;
}

// IDA_EA: 0x85E30AE
// IDA_NAME: _ZN8CMissionltERKS_
// DEMANGLED: CMission::operator<(CMission const&)
bool CMission::operator<(const CMission& rhs) {
    if (mission_kind_ == rhs.GetMissionKind()) {
        if (mission_index_ == rhs.GetMissionIndex()) {
            return false;
        }
        return mission_index_ < rhs.GetMissionIndex();
    }
    return mission_kind_ < rhs.GetMissionKind();
}

// IDA_EA: 0x85E3124
// IDA_NAME: _ZNK8CMission15makeMissionInfoER11MissionInfoRK5CUser
// DEMANGLED: CMission::makeMissionInfo(MissionInfo &,CUser const&)const
int CMission::makeMissionInfo(MissionInfo& info, const CUser& user) const {
    info.mission_kind = static_cast<uint16_t>(GetMissionKind());
    info.mission_index = static_cast<uint16_t>(GetMissionIndex());
    info.current_value = 0;
    info.target_value = 0;
    _makeTotalMissionInfo(info, user);
    return 1;
}

// IDA_EA: 0x85E318E
// IDA_NAME: _ZNK8CMission17isAcceptConditionERK5CUser
// DEMANGLED: CMission::isAcceptCondition(CUser const&)const
int CMission::isAcceptCondition(const CUser& user) const {
    const CMissionList_Charac* mission_list =
        static_cast<const CMissionList_Charac*>(user.GetCharacExpandDataR(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
    if (mission_list->getIndex_byKind(GetMissionKind()) != 0) {
        return 0;
    }
    if (rank_range_.first != -1 &&
        (rank_range_.first > user.get_pvp_grade() ||
         user.get_pvp_grade() >= rank_range_.second)) {
        return 0;
    }
    const int rate = user.get_pvp_WinningRate_relateMission();
    if (rate_range_.first != -1 &&
        (rate_range_.first > rate || rate >= rate_range_.second)) {
        return 0;
    }
    return _isAcceptCondition(user);
}

// IDA_EA: 0x85E327A
// IDA_NAME: _ZNK8CMission9canUpdateERK31MissionClearCondition_Parameter
// DEMANGLED: CMission::canUpdate(MissionClearCondition_Parameter const&)const
int CMission::canUpdate(
    const MissionClearCondition_Parameter& parameter) const {
    if (pvp_battle_mode_ == 0xffu) {
        if (parameter.battle_mode_category != 1 &&
            parameter.battle_mode_category != 2) {
            return 0;
        }
    } else if (pvp_battle_mode_ != parameter.battle_mode_category) {
        return 0;
    }
    return _canUpdate(parameter);
}

// IDA_EA: 0x85E32DA
// IDA_NAME: _ZNK8CMission6RewardER5CUser
// DEMANGLED: CMission::Reward(CUser &)const
int CMission::Reward(CUser& user) const {
    const unsigned int base_exp =
        G_CDataManager()->get_BaseMissionExp_byRank(
            static_cast<unsigned short>(rank_range_.first));
    const int mission_exp =
        static_cast<int>(static_cast<long double>(base_exp) *
                         difficulty_ratio_);
    const int mission_index = GetMissionIndex();
    const int mission_kind = GetMissionKind();
    CUser_history_log(&user)->pvpMissionClearReward(
        user.getPVPResultRefW()->values[2],
        mission_exp,
        mission_kind,
        mission_index);
    user.getPVPResultRefW()->values[2] += mission_exp;

    std::vector<int> inserted_slots;
    const int inserted = user.getCurCharacInvenW()->insert_event_items(
        reward_items_,
        inserted_slots,
        static_cast<eMoneyAddReason>(30),
        static_cast<eItemAddReason>(51),
        true,
        "game_server_msg_173",
        "game_server_msg_174");
    if (inserted > 0 && inserted_slots.size() > 2u) {
        user.SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                            static_cast<ENUM_ITEMSPACE>(0),
                            inserted_slots[0]);
    }
    _Reward(user);
    return 1;
}

// IDA_EA: 0x85E3490
// IDA_NAME: _ZNK8CMission25_isAcceptCondition_SeriesERK5CUser
// DEMANGLED: CMission::_isAcceptCondition_Series(CUser const&)const
bool CMission::_isAcceptCondition_Series(const CUser& user) const {
    const CMissionList_Charac* mission_list =
        static_cast<const CMissionList_Charac*>(user.GetCharacExpandDataR(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
    if (mission_list->isClearMission(GetMissionIndex())) {
        return false;
    }
    if (GetPrevMissionIndex() == -1) {
        return true;
    }
    if (!mission_list->isClearMission(GetPrevMissionIndex())) {
        return false;
    }
    return mission_list->getIndex_byKind(GetMissionKind()) == 0;
}

// IDA_EA: 0x85E6906
// IDA_NAME: _ZNK8CMission15GetMissionIndexEv
// DEMANGLED: CMission::GetMissionIndex(void)const
int CMission::GetMissionIndex() const {
    return mission_index_;
}

// IDA_EA: 0x85E6912
// IDA_NAME: _ZNK8CMission14GetMissionKindEv
// DEMANGLED: CMission::GetMissionKind(void)const
int CMission::GetMissionKind() const {
    return mission_kind_;
}

// IDA_EA: 0x85E691E
// IDA_NAME: _ZNK8CMission19GetPrevMissionIndexEv
// DEMANGLED: CMission::GetPrevMissionIndex(void)const
int CMission::GetPrevMissionIndex() const {
    return prev_mission_index_;
}

// IDA_EA: 0x85E692A
// IDA_NAME: _ZNK8CMission12GetRankRangeEv
// DEMANGLED: CMission::GetRankRange(void)const
std::pair<int, int> CMission::GetRankRange() const {
    return rank_range_;
}

// IDA_EA: 0x85E6944
// IDA_NAME: _ZNK8CMission15GetMissionGradeEv
// DEMANGLED: CMission::GetMissionGrade(void)const
int CMission::GetMissionGrade() const {
    return mission_grade_;
}

// IDA_EA: 0x85E3548
// IDA_NAME: _ZNK22CMission_winning_count21_makeTotalMissionInfoER11MissionInfoRK5CUser
// DEMANGLED: CMission_winning_count::_makeTotalMissionInfo(MissionInfo &,CUser const&)const
void CMission_winning_count::_makeTotalMissionInfo(
    MissionInfo& info, const CUser&) const {
    if (prev_mission_index_ != -1) {
        info.current_value = static_cast<uint16_t>(
            G_CDataManager()->find_mission(prev_mission_index_)
                ->getClearCondition());
    }
}

// IDA_EA: 0x85E3596
// IDA_NAME: _ZNK22CMission_winning_count18_isAcceptConditionERK5CUser
// DEMANGLED: CMission_winning_count::_isAcceptCondition(CUser const&)const
bool CMission_winning_count::_isAcceptCondition(const CUser& user) const {
    return _isAcceptCondition_Series(user);
}

// IDA_EA: 0x85E35B0
// IDA_NAME: _ZNK22CMission_winning_count13verifyMissionERK11MissionInfo
// DEMANGLED: CMission_winning_count::verifyMission(MissionInfo const&)const
bool CMission_winning_count::verifyMission(const MissionInfo& info) const {
    return condition_ + 10 > static_cast<int16_t>(info.current_value);
}

// IDA_EA: 0x85E35D6
// IDA_NAME: _ZNK22CMission_winning_count10_canUpdateERK31MissionClearCondition_Parameter
// DEMANGLED: CMission_winning_count::_canUpdate(MissionClearCondition_Parameter const&)const
bool CMission_winning_count::_canUpdate(
    const MissionClearCondition_Parameter& parameter) const {
    return is_Victory_in_a_row(std::bitset<32>(parameter.issue_history), 1);
}

// IDA_EA: 0x85E361A
// IDA_NAME: _ZNK22CMission_winning_count6UpdateER11MissionInfoRK31MissionClearCondition_Parameter
// DEMANGLED: CMission_winning_count::Update(MissionInfo &,MissionClearCondition_Parameter const&)const
int CMission_winning_count::Update(
    MissionInfo& info, const MissionClearCondition_Parameter&) const {
    ++info.current_value;
    return 1;
}

// IDA_EA: 0x85E3636
// IDA_NAME: _ZNK22CMission_winning_count11isClearableER11MissionInfoRb
// DEMANGLED: CMission_winning_count::isClearable(MissionInfo &,bool &)const
bool CMission_winning_count::isClearable(MissionInfo& info, bool&) const {
    return condition_ <= static_cast<int16_t>(info.current_value);
}

// IDA_EA: 0x85E69A8
// IDA_NAME: _ZN22CMission_winning_countC2ERK13MissionScripti
// DEMANGLED: CMission_winning_count::CMission_winning_count(MissionScript const&,int)
CMission_winning_count::CMission_winning_count(const MissionScript& script,
                                               int condition)
    : CMission(script), condition_(condition) {}

// IDA_EA: 0x85E695A
// IDA_NAME: _ZN22CMission_winning_countD2Ev
// DEMANGLED: CMission_winning_count::~CMission_winning_count()
// IDA_EA: 0x85E698A
// IDA_NAME: _ZN22CMission_winning_countD0Ev
// DEMANGLED: CMission_winning_count::~CMission_winning_count() deleting destructor
CMission_winning_count::~CMission_winning_count() {}

// IDA_EA: 0x85E69D4
// IDA_NAME: _ZNK22CMission_winning_count15isSeriesMissionEv
// DEMANGLED: CMission_winning_count::isSeriesMission(void)const
bool CMission_winning_count::isSeriesMission() const { return true; }

// IDA_EA: 0x85E69DE
// IDA_NAME: _ZNK22CMission_winning_count17getClearConditionEv
// DEMANGLED: CMission_winning_count::getClearCondition(void)const
int CMission_winning_count::getClearCondition() const { return condition_; }

// IDA_EA: 0x85E364E
// IDA_NAME: _ZNK23CMission_winning_streak10_canUpdateERK31MissionClearCondition_Parameter
// DEMANGLED: CMission_winning_streak::_canUpdate(MissionClearCondition_Parameter const&)const
bool CMission_winning_streak::_canUpdate(
    const MissionClearCondition_Parameter&) const { return true; }

// IDA_EA: 0x85E3658
// IDA_NAME: _ZNK23CMission_winning_streak21_makeTotalMissionInfoER11MissionInfoRK5CUser
// DEMANGLED: CMission_winning_streak::_makeTotalMissionInfo(MissionInfo &,CUser const&)const
void CMission_winning_streak::_makeTotalMissionInfo(
    MissionInfo& info, const CUser&) const {
    info.target_value = static_cast<uint16_t>(streak_length_);
}

// IDA_EA: 0x85E366C
// IDA_NAME: _ZNK23CMission_winning_streak13verifyMissionERK11MissionInfo
// DEMANGLED: CMission_winning_streak::verifyMission(MissionInfo const&)const
bool CMission_winning_streak::verifyMission(const MissionInfo& info) const {
    return clear_count_ + 10 > static_cast<int16_t>(info.current_value);
}

// IDA_EA: 0x85E3692
// IDA_NAME: _ZNK23CMission_winning_streak6UpdateER11MissionInfoRK31MissionClearCondition_Parameter
// DEMANGLED: CMission_winning_streak::Update(MissionInfo &,MissionClearCondition_Parameter const&)const
int CMission_winning_streak::Update(
    MissionInfo& info,
    const MissionClearCondition_Parameter& parameter) const {
    if ((parameter.issue_history & 1u) != 0) {
        --info.target_value;
        if (static_cast<int16_t>(info.target_value) <= 0) {
            ++info.current_value;
            info.target_value = static_cast<uint16_t>(streak_length_);
        }
    } else {
        info.target_value = static_cast<uint16_t>(streak_length_);
    }
    return 1;
}

// IDA_EA: 0x85E36FC
// IDA_NAME: _ZNK23CMission_winning_streak11isClearableER11MissionInfoRb
// DEMANGLED: CMission_winning_streak::isClearable(MissionInfo &,bool &)const
bool CMission_winning_streak::isClearable(MissionInfo& info, bool&) const {
    return clear_count_ <= static_cast<int16_t>(info.current_value);
}

// IDA_EA: 0x85E6A38
// IDA_NAME: _ZN23CMission_winning_streakC2ERK13MissionScriptii
// DEMANGLED: CMission_winning_streak::CMission_winning_streak(MissionScript const&,int,int)
CMission_winning_streak::CMission_winning_streak(
    const MissionScript& script, int streak_length, int clear_count)
    : CMission(script),
      streak_length_(streak_length),
      clear_count_(clear_count) {}

// IDA_EA: 0x85E69EA
// IDA_NAME: _ZN23CMission_winning_streakD2Ev
// DEMANGLED: CMission_winning_streak::~CMission_winning_streak()
// IDA_EA: 0x85E6A1A
// IDA_NAME: _ZN23CMission_winning_streakD0Ev
// DEMANGLED: CMission_winning_streak::~CMission_winning_streak() deleting destructor
CMission_winning_streak::~CMission_winning_streak() {}

// IDA_EA: 0x85E6A6E
// IDA_NAME: _ZNK23CMission_winning_streak15isSeriesMissionEv
// DEMANGLED: CMission_winning_streak::isSeriesMission(void)const
bool CMission_winning_streak::isSeriesMission() const { return false; }

// IDA_EA: 0x85E6A78
// IDA_NAME: _ZNK23CMission_winning_streak17getClearConditionEv
// DEMANGLED: CMission_winning_streak::getClearCondition(void)const
int CMission_winning_streak::getClearCondition() const { return clear_count_; }

// IDA_EA: 0x85E378A
// IDA_NAME: _ZNK13CMission_join21_makeTotalMissionInfoER11MissionInfoRK5CUser
// DEMANGLED: CMission_join::_makeTotalMissionInfo(MissionInfo &,CUser const&)const
void CMission_join::_makeTotalMissionInfo(MissionInfo& info,
                                          const CUser&) const {
    if (prev_mission_index_ != -1) {
        info.current_value = static_cast<uint16_t>(
            G_CDataManager()->find_mission(prev_mission_index_)
                ->getClearCondition());
    }
}

// IDA_EA: 0x85E37FE
// IDA_NAME: _ZNK13CMission_join18_isAcceptConditionERK5CUser
// DEMANGLED: CMission_join::_isAcceptCondition(CUser const&)const
bool CMission_join::_isAcceptCondition(const CUser& user) const {
    return _isAcceptCondition_Series(user);
}

// IDA_EA: 0x85E37D8
// IDA_NAME: _ZNK13CMission_join13verifyMissionERK11MissionInfo
// DEMANGLED: CMission_join::verifyMission(MissionInfo const&)const
bool CMission_join::verifyMission(const MissionInfo& info) const {
    return condition_ + 10 > static_cast<int16_t>(info.current_value);
}

// IDA_EA: 0x85E3818
// IDA_NAME: _ZNK13CMission_join6UpdateER11MissionInfoRK31MissionClearCondition_Parameter
// DEMANGLED: CMission_join::Update(MissionInfo &,MissionClearCondition_Parameter const&)const
int CMission_join::Update(MissionInfo& info,
                          const MissionClearCondition_Parameter&) const {
    ++info.current_value;
    return 1;
}

// IDA_EA: 0x85E3834
// IDA_NAME: _ZNK13CMission_join11isClearableER11MissionInfoRb
// DEMANGLED: CMission_join::isClearable(MissionInfo &,bool &)const
bool CMission_join::isClearable(MissionInfo& info, bool&) const {
    return condition_ <= static_cast<int16_t>(info.current_value);
}

// IDA_EA: 0x85E6AE8
// IDA_NAME: _ZN13CMission_joinC2ERK13MissionScripti
// DEMANGLED: CMission_join::CMission_join(MissionScript const&,int)
CMission_join::CMission_join(const MissionScript& script, int condition)
    : CMission(script), condition_(condition) {}

// IDA_EA: 0x85ECA4E
// IDA_NAME: _ZN13CMission_joinD2Ev
// DEMANGLED: CMission_join::~CMission_join()
// IDA_EA: 0x85ECA7E
// IDA_NAME: _ZN13CMission_joinD0Ev
// DEMANGLED: CMission_join::~CMission_join() deleting destructor
CMission_join::~CMission_join() {}

// IDA_EA: 0x85E6B14
// IDA_NAME: _ZNK13CMission_join15isSeriesMissionEv
// DEMANGLED: CMission_join::isSeriesMission(void)const
bool CMission_join::isSeriesMission() const { return true; }

// IDA_EA: 0x85E6B1E
// IDA_NAME: _ZNK13CMission_join17getClearConditionEv
// DEMANGLED: CMission_join::getClearCondition(void)const
int CMission_join::getClearCondition() const { return condition_; }

// IDA_EA: 0x85E384C
// IDA_NAME: _ZNK19CMission_kill_count21_makeTotalMissionInfoER11MissionInfoRK5CUser
// DEMANGLED: CMission_kill_count::_makeTotalMissionInfo(MissionInfo &,CUser const&)const
void CMission_kill_count::_makeTotalMissionInfo(
    MissionInfo& info, const CUser&) const {
    if (prev_mission_index_ != -1) {
        info.current_value = static_cast<uint16_t>(
            G_CDataManager()->find_mission(prev_mission_index_)
                ->getClearCondition());
    }
}

// IDA_EA: 0x85E389A
// IDA_NAME: _ZNK19CMission_kill_count18_isAcceptConditionERK5CUser
// DEMANGLED: CMission_kill_count::_isAcceptCondition(CUser const&)const
bool CMission_kill_count::_isAcceptCondition(const CUser& user) const {
    return _isAcceptCondition_Series(user);
}

// IDA_EA: 0x85E38B4
// IDA_NAME: _ZNK19CMission_kill_count13verifyMissionERK11MissionInfo
// DEMANGLED: CMission_kill_count::verifyMission(MissionInfo const&)const
bool CMission_kill_count::verifyMission(const MissionInfo& info) const {
    return condition_ + 10 > static_cast<int16_t>(info.current_value);
}

// IDA_EA: 0x85E38DA
// IDA_NAME: _ZNK19CMission_kill_count6UpdateER11MissionInfoRK31MissionClearCondition_Parameter
// DEMANGLED: CMission_kill_count::Update(MissionInfo &,MissionClearCondition_Parameter const&)const
int CMission_kill_count::Update(
    MissionInfo& info, const MissionClearCondition_Parameter&) const {
    ++info.current_value;
    return 1;
}

// IDA_EA: 0x85E38F6
// IDA_NAME: _ZNK19CMission_kill_count11isClearableER11MissionInfoRb
// DEMANGLED: CMission_kill_count::isClearable(MissionInfo &,bool &)const
bool CMission_kill_count::isClearable(MissionInfo& info, bool&) const {
    return condition_ <= static_cast<int16_t>(info.current_value);
}

// IDA_EA: 0x85E6B2A
// IDA_NAME: _ZN19CMission_kill_countC2ERK13MissionScripti
// DEMANGLED: CMission_kill_count::CMission_kill_count(MissionScript const&,int)
CMission_kill_count::CMission_kill_count(const MissionScript& script,
                                         int condition)
    : CMission(script), condition_(condition) {}

// IDA_EA: 0x85ECA00
// IDA_NAME: _ZN19CMission_kill_countD2Ev
// DEMANGLED: CMission_kill_count::~CMission_kill_count()
// IDA_EA: 0x85ECA30
// IDA_NAME: _ZN19CMission_kill_countD0Ev
// DEMANGLED: CMission_kill_count::~CMission_kill_count() deleting destructor
CMission_kill_count::~CMission_kill_count() {}

// IDA_EA: 0x85E6B56
// IDA_NAME: _ZNK19CMission_kill_count15isSeriesMissionEv
// DEMANGLED: CMission_kill_count::isSeriesMission(void)const
bool CMission_kill_count::isSeriesMission() const { return true; }

// IDA_EA: 0x85E6B60
// IDA_NAME: _ZNK19CMission_kill_count17getClearConditionEv
// DEMANGLED: CMission_kill_count::getClearCondition(void)const
int CMission_kill_count::getClearCondition() const { return condition_; }

// IDA_EA: 0x85E390E
// IDA_NAME: _ZNK23CMission_hereafter_join13verifyMissionERK11MissionInfo
// DEMANGLED: CMission_hereafter_join::verifyMission(MissionInfo const&)const
bool CMission_hereafter_join::verifyMission(const MissionInfo& info) const {
    return condition_ + 10 > static_cast<int16_t>(info.current_value);
}

// IDA_EA: 0x85E3934
// IDA_NAME: _ZNK23CMission_hereafter_join6UpdateER11MissionInfoRK31MissionClearCondition_Parameter
// DEMANGLED: CMission_hereafter_join::Update(MissionInfo &,MissionClearCondition_Parameter const&)const
int CMission_hereafter_join::Update(
    MissionInfo& info, const MissionClearCondition_Parameter&) const {
    ++info.current_value;
    return 1;
}

// IDA_EA: 0x85E3950
// IDA_NAME: _ZNK23CMission_hereafter_join11isClearableER11MissionInfoRb
// DEMANGLED: CMission_hereafter_join::isClearable(MissionInfo &,bool &)const
bool CMission_hereafter_join::isClearable(MissionInfo& info, bool&) const {
    return condition_ <= static_cast<int16_t>(info.current_value);
}

// IDA_EA: 0x85E6B6C
// IDA_NAME: _ZN23CMission_hereafter_joinC2ERK13MissionScripti
// DEMANGLED: CMission_hereafter_join::CMission_hereafter_join(MissionScript const&,int)
CMission_hereafter_join::CMission_hereafter_join(
    const MissionScript& script, int condition)
    : CMission(script), condition_(condition) {}

// IDA_EA: 0x85EC9B2
// IDA_NAME: _ZN23CMission_hereafter_joinD2Ev
// DEMANGLED: CMission_hereafter_join::~CMission_hereafter_join()
// IDA_EA: 0x85EC9E2
// IDA_NAME: _ZN23CMission_hereafter_joinD0Ev
// DEMANGLED: CMission_hereafter_join::~CMission_hereafter_join() deleting destructor
CMission_hereafter_join::~CMission_hereafter_join() {}

// IDA_EA: 0x85E6B98
// IDA_NAME: _ZNK23CMission_hereafter_join15isSeriesMissionEv
// DEMANGLED: CMission_hereafter_join::isSeriesMission(void)const
bool CMission_hereafter_join::isSeriesMission() const { return false; }

// IDA_EA: 0x85E6BA2
// IDA_NAME: _ZNK23CMission_hereafter_join17getClearConditionEv
// DEMANGLED: CMission_hereafter_join::getClearCondition(void)const
int CMission_hereafter_join::getClearCondition() const { return condition_; }

// IDA_EA: 0x85E3968
// IDA_NAME: _ZNK32CMission_hereafter_winning_count13verifyMissionERK11MissionInfo
// DEMANGLED: CMission_hereafter_winning_count::verifyMission(MissionInfo const&)const
bool CMission_hereafter_winning_count::verifyMission(
    const MissionInfo& info) const {
    return condition_ + 10 > static_cast<int16_t>(info.current_value);
}

// IDA_EA: 0x85E398E
// IDA_NAME: _ZNK32CMission_hereafter_winning_count10_canUpdateERK31MissionClearCondition_Parameter
// DEMANGLED: CMission_hereafter_winning_count::_canUpdate(MissionClearCondition_Parameter const&)const
bool CMission_hereafter_winning_count::_canUpdate(
    const MissionClearCondition_Parameter& parameter) const {
    return is_Victory_in_a_row(std::bitset<32>(parameter.issue_history), 1);
}

// IDA_EA: 0x85E39D2
// IDA_NAME: _ZNK32CMission_hereafter_winning_count6UpdateER11MissionInfoRK31MissionClearCondition_Parameter
// DEMANGLED: CMission_hereafter_winning_count::Update(MissionInfo &,MissionClearCondition_Parameter const&)const
int CMission_hereafter_winning_count::Update(
    MissionInfo& info, const MissionClearCondition_Parameter&) const {
    ++info.current_value;
    return 1;
}

// IDA_EA: 0x85E39EE
// IDA_NAME: _ZNK32CMission_hereafter_winning_count11isClearableER11MissionInfoRb
// DEMANGLED: CMission_hereafter_winning_count::isClearable(MissionInfo &,bool &)const
bool CMission_hereafter_winning_count::isClearable(
    MissionInfo& info, bool&) const {
    return condition_ <= static_cast<int16_t>(info.current_value);
}

// IDA_EA: 0x85E6BAE
// IDA_NAME: _ZN32CMission_hereafter_winning_countC2ERK13MissionScripti
// DEMANGLED: CMission_hereafter_winning_count::CMission_hereafter_winning_count(MissionScript const&,int)
CMission_hereafter_winning_count::CMission_hereafter_winning_count(
    const MissionScript& script, int condition)
    : CMission(script), condition_(condition) {}

// IDA_EA: 0x85EC964
// IDA_NAME: _ZN32CMission_hereafter_winning_countD2Ev
// DEMANGLED: CMission_hereafter_winning_count::~CMission_hereafter_winning_count()
// IDA_EA: 0x85EC994
// IDA_NAME: _ZN32CMission_hereafter_winning_countD0Ev
// DEMANGLED: CMission_hereafter_winning_count::~CMission_hereafter_winning_count() deleting destructor
CMission_hereafter_winning_count::~CMission_hereafter_winning_count() {}

// IDA_EA: 0x85E6BDA
// IDA_NAME: _ZNK32CMission_hereafter_winning_count15isSeriesMissionEv
// DEMANGLED: CMission_hereafter_winning_count::isSeriesMission(void)const
bool CMission_hereafter_winning_count::isSeriesMission() const {
    return false;
}

// IDA_EA: 0x85E6BE4
// IDA_NAME: _ZNK32CMission_hereafter_winning_count17getClearConditionEv
// DEMANGLED: CMission_hereafter_winning_count::getClearCondition(void)const
int CMission_hereafter_winning_count::getClearCondition() const {
    return condition_;
}

// IDA_EA: 0x85E3A06
// IDA_NAME: _ZNK18CMission_remain_hp13verifyMissionERK11MissionInfo
// DEMANGLED: CMission_remain_hp::verifyMission(MissionInfo const&)const
bool CMission_remain_hp::verifyMission(const MissionInfo& info) const {
    return clear_count_ + 10 > static_cast<int16_t>(info.current_value);
}

// IDA_EA: 0x85E3A2C
// IDA_NAME: _ZNK18CMission_remain_hp11isClearableER11MissionInfoRb
// DEMANGLED: CMission_remain_hp::isClearable(MissionInfo &,bool &)const
bool CMission_remain_hp::isClearable(MissionInfo& info, bool&) const {
    return clear_count_ <= static_cast<int16_t>(info.current_value);
}

// IDA_EA: 0x85E3A44
// IDA_NAME: _ZNK18CMission_remain_hp6UpdateER11MissionInfoRK31MissionClearCondition_Parameter
// DEMANGLED: CMission_remain_hp::Update(MissionInfo &,MissionClearCondition_Parameter const&)const
int CMission_remain_hp::Update(
    MissionInfo& info, const MissionClearCondition_Parameter&) const {
    ++info.current_value;
    return 1;
}

// IDA_EA: 0x85E6BF0
// IDA_NAME: _ZN18CMission_remain_hpC2ERK13MissionScriptii
// DEMANGLED: CMission_remain_hp::CMission_remain_hp(MissionScript const&,int,int)
CMission_remain_hp::CMission_remain_hp(const MissionScript& script,
                                       int remain_percent,
                                       int clear_count)
    : CMission(script),
      remain_percent_(remain_percent),
      clear_count_(clear_count) {}

// IDA_EA: 0x85EC916
// IDA_NAME: _ZN18CMission_remain_hpD2Ev
// DEMANGLED: CMission_remain_hp::~CMission_remain_hp()
// IDA_EA: 0x85EC946
// IDA_NAME: _ZN18CMission_remain_hpD0Ev
// DEMANGLED: CMission_remain_hp::~CMission_remain_hp() deleting destructor
CMission_remain_hp::~CMission_remain_hp() {}

// IDA_EA: 0x85E6C26
// IDA_NAME: _ZNK18CMission_remain_hp15isSeriesMissionEv
// DEMANGLED: CMission_remain_hp::isSeriesMission(void)const
bool CMission_remain_hp::isSeriesMission() const { return false; }

// IDA_EA: 0x85E6C30
// IDA_NAME: _ZNK18CMission_remain_hp17getClearConditionEv
// DEMANGLED: CMission_remain_hp::getClearCondition(void)const
int CMission_remain_hp::getClearCondition() const { return clear_count_; }

// IDA_EA: 0x85E3A60
// IDA_NAME: _ZNK16CMission_revenge6UpdateER11MissionInfoRK31MissionClearCondition_Parameter
// DEMANGLED: CMission_revenge::Update(MissionInfo &,MissionClearCondition_Parameter const&)const
int CMission_revenge::Update(
    MissionInfo& info,
    const MissionClearCondition_Parameter& parameter) const {
    const int state = static_cast<int16_t>(info.target_value);
    if (state == 1) {
        info.target_value = 2;
        return 0;
    }
    if (state == 2) {
        if ((parameter.issue_history & 1u) != 0) {
            info.target_value = 0;
            ++info.current_value;
            return 1;
        }
        return 0;
    }
    if (state != 0) {
        cMyTraceNoop trace;
        trace();
        return 0;
    }
    if ((parameter.issue_history & 1u) == 0) {
        info.target_value = 1;
    }
    return 0;
}

// IDA_EA: 0x85E3B1C
// IDA_NAME: _ZNK16CMission_revenge11isClearableER11MissionInfoRb
// DEMANGLED: CMission_revenge::isClearable(MissionInfo &,bool &)const
bool CMission_revenge::isClearable(MissionInfo& info, bool&) const {
    return condition_ <= static_cast<int16_t>(info.current_value);
}

// IDA_EA: 0x85E3B34
// IDA_NAME: _ZNK16CMission_revenge13verifyMissionERK11MissionInfo
// DEMANGLED: CMission_revenge::verifyMission(MissionInfo const&)const
bool CMission_revenge::verifyMission(const MissionInfo& info) const {
    return condition_ + 10 > static_cast<int16_t>(info.current_value);
}

// IDA_EA: 0x85E6C3C
// IDA_NAME: _ZN16CMission_revengeC2ERK13MissionScripti
// DEMANGLED: CMission_revenge::CMission_revenge(MissionScript const&,int)
CMission_revenge::CMission_revenge(const MissionScript& script,
                                   int condition)
    : CMission(script), condition_(condition) {}

// IDA_EA: 0x85EC8C8
// IDA_NAME: _ZN16CMission_revengeD2Ev
// DEMANGLED: CMission_revenge::~CMission_revenge()
// IDA_EA: 0x85EC8F8
// IDA_NAME: _ZN16CMission_revengeD0Ev
// DEMANGLED: CMission_revenge::~CMission_revenge() deleting destructor
CMission_revenge::~CMission_revenge() {}

// IDA_EA: 0x85E6C68
// IDA_NAME: _ZNK16CMission_revenge15isSeriesMissionEv
// DEMANGLED: CMission_revenge::isSeriesMission(void)const
bool CMission_revenge::isSeriesMission() const { return false; }

// IDA_EA: 0x85E6C72
// IDA_NAME: _ZNK16CMission_revenge17getClearConditionEv
// DEMANGLED: CMission_revenge::getClearCondition(void)const
int CMission_revenge::getClearCondition() const { return condition_; }

// IDA_EA: 0x85E3B5A
// IDA_NAME: _ZNK29CMission_hereafter_kill_count13verifyMissionERK11MissionInfo
// DEMANGLED: CMission_hereafter_kill_count::verifyMission(MissionInfo const&)const
bool CMission_hereafter_kill_count::verifyMission(
    const MissionInfo& info) const {
    return condition_ + 10 > static_cast<int16_t>(info.current_value);
}

// IDA_EA: 0x85E3B80
// IDA_NAME: _ZNK29CMission_hereafter_kill_count6UpdateER11MissionInfoRK31MissionClearCondition_Parameter
// DEMANGLED: CMission_hereafter_kill_count::Update(MissionInfo &,MissionClearCondition_Parameter const&)const
int CMission_hereafter_kill_count::Update(
    MissionInfo& info, const MissionClearCondition_Parameter&) const {
    ++info.current_value;
    return 1;
}

// IDA_EA: 0x85E3B9C
// IDA_NAME: _ZNK29CMission_hereafter_kill_count11isClearableER11MissionInfoRb
// DEMANGLED: CMission_hereafter_kill_count::isClearable(MissionInfo &,bool &)const
bool CMission_hereafter_kill_count::isClearable(
    MissionInfo& info, bool&) const {
    return condition_ <= static_cast<int16_t>(info.current_value);
}

// IDA_EA: 0x85E6C7E
// IDA_NAME: _ZN29CMission_hereafter_kill_countC2ERK13MissionScripti
// DEMANGLED: CMission_hereafter_kill_count::CMission_hereafter_kill_count(MissionScript const&,int)
CMission_hereafter_kill_count::CMission_hereafter_kill_count(
    const MissionScript& script, int condition)
    : CMission(script), condition_(condition) {}

// IDA_EA: 0x85EC87A
// IDA_NAME: _ZN29CMission_hereafter_kill_countD2Ev
// DEMANGLED: CMission_hereafter_kill_count::~CMission_hereafter_kill_count()
// IDA_EA: 0x85EC8AA
// IDA_NAME: _ZN29CMission_hereafter_kill_countD0Ev
// DEMANGLED: CMission_hereafter_kill_count::~CMission_hereafter_kill_count() deleting destructor
CMission_hereafter_kill_count::~CMission_hereafter_kill_count() {}

// IDA_EA: 0x85E6CAA
// IDA_NAME: _ZNK29CMission_hereafter_kill_count15isSeriesMissionEv
// DEMANGLED: CMission_hereafter_kill_count::isSeriesMission(void)const
bool CMission_hereafter_kill_count::isSeriesMission() const { return false; }

// IDA_EA: 0x85E6CB4
// IDA_NAME: _ZNK29CMission_hereafter_kill_count17getClearConditionEv
// DEMANGLED: CMission_hereafter_kill_count::getClearCondition(void)const
int CMission_hereafter_kill_count::getClearCondition() const {
    return condition_;
}

// IDA_EA: 0x85E3BB4
// IDA_NAME: _ZNK13CMission_kill11isClearableER11MissionInfoRb
// DEMANGLED: CMission_kill::isClearable(MissionInfo &,bool &)const
bool CMission_kill::isClearable(MissionInfo& info, bool&) const {
    return clear_count_ <= static_cast<int16_t>(info.current_value);
}

// IDA_EA: 0x85E3BCC
// IDA_NAME: _ZNK13CMission_kill10_canUpdateERK31MissionClearCondition_Parameter
// DEMANGLED: CMission_kill::_canUpdate(MissionClearCondition_Parameter const&)const
bool CMission_kill::_canUpdate(
    const MissionClearCondition_Parameter& parameter) const {
    return parameter.kill_count >= required_kills_;
}

// IDA_EA: 0x85E3BEE
// IDA_NAME: _ZNK13CMission_kill13verifyMissionERK11MissionInfo
// DEMANGLED: CMission_kill::verifyMission(MissionInfo const&)const
bool CMission_kill::verifyMission(const MissionInfo& info) const {
    return clear_count_ + 10 > static_cast<int16_t>(info.current_value);
}

// IDA_EA: 0x85E3C14
// IDA_NAME: _ZNK13CMission_kill6UpdateER11MissionInfoRK31MissionClearCondition_Parameter
// DEMANGLED: CMission_kill::Update(MissionInfo &,MissionClearCondition_Parameter const&)const
int CMission_kill::Update(MissionInfo& info,
                          const MissionClearCondition_Parameter&) const {
    ++info.current_value;
    return 1;
}

// IDA_EA: 0x85E6CC0
// IDA_NAME: _ZN13CMission_killC2ERK13MissionScriptii
// DEMANGLED: CMission_kill::CMission_kill(MissionScript const&,int,int)
CMission_kill::CMission_kill(const MissionScript& script,
                             int required_kills,
                             int clear_count)
    : CMission(script),
      required_kills_(required_kills),
      clear_count_(clear_count) {}

// IDA_EA: 0x85EC82C
// IDA_NAME: _ZN13CMission_killD2Ev
// DEMANGLED: CMission_kill::~CMission_kill()
// IDA_EA: 0x85EC85C
// IDA_NAME: _ZN13CMission_killD0Ev
// DEMANGLED: CMission_kill::~CMission_kill() deleting destructor
CMission_kill::~CMission_kill() {}

// IDA_EA: 0x85E6CF6
// IDA_NAME: _ZNK13CMission_kill15isSeriesMissionEv
// DEMANGLED: CMission_kill::isSeriesMission(void)const
bool CMission_kill::isSeriesMission() const { return false; }

// IDA_EA: 0x85E6D00
// IDA_NAME: _ZNK13CMission_kill17getClearConditionEv
// DEMANGLED: CMission_kill::getClearCondition(void)const
int CMission_kill::getClearCondition() const { return clear_count_; }

// IDA_EA: 0x85E3C30
// IDA_NAME: _ZNK15CMission_within18_isAcceptConditionERK5CUser
// DEMANGLED: CMission_within::_isAcceptCondition(CUser const&)const
bool CMission_within::_isAcceptCondition(const CUser& user) const {
    const CMissionList_Charac* mission_list =
        static_cast<const CMissionList_Charac*>(user.GetCharacExpandDataR(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
    if (mission_list->isClearMission(GetMissionIndex())) {
        return false;
    }
    if (user.getPVPResultRefR()->values[5] != rank_range_.first) {
        return false;
    }
    const int pvp_point =
        GlobalData::s_ref_pvp_grade->GetPvpPoint(rank_range_.first);
    (void)pvp_point;
    const int next_point =
        GlobalData::s_ref_pvp_grade->GetPvpNextRankPoint(rank_range_.first);
    if (user.getPVPResultRefR()->values[2] < next_point) {
        return false;
    }
    const_cast<CUser&>(user).getPVPResultRefW()->values[2] = next_point;
    return true;
}

// IDA_EA: 0x85E3D4E
// IDA_NAME: _ZNK15CMission_within13verifyMissionERK11MissionInfo
// DEMANGLED: CMission_within::verifyMission(MissionInfo const&)const
bool CMission_within::verifyMission(const MissionInfo&) const { return true; }

// IDA_EA: 0x85E3D58
// IDA_NAME: _ZNK15CMission_within6UpdateER11MissionInfoRK31MissionClearCondition_Parameter
// DEMANGLED: CMission_within::Update(MissionInfo &,MissionClearCondition_Parameter const&)const
int CMission_within::Update(
    MissionInfo& info,
    const MissionClearCondition_Parameter& parameter) const {
    info.current_value = static_cast<uint16_t>(info.current_value * 2);
    info.current_value = static_cast<uint16_t>(
        info.current_value + ((parameter.issue_history & 1u) != 0));
    ++info.target_value;
    return 1;
}

// IDA_EA: 0x85E3DB8
// IDA_NAME: _ZNK15CMission_within11isClearableER11MissionInfoRb
// DEMANGLED: CMission_within::isClearable(MissionInfo &,bool &)const
bool CMission_within::isClearable(MissionInfo& info, bool& failed) const {
    int wins = 0;
    for (int i = 0; i < attempts_; ++i) {
        if (((static_cast<int16_t>(info.current_value) >> i) & 1) != 0) {
            ++wins;
        }
    }
    if (wins >= required_wins_) {
        return true;
    }
    if (wins + attempts_ - static_cast<int16_t>(info.target_value) <
        required_wins_) {
        info.current_value = 0;
        info.target_value = 0;
        failed = true;
    }
    return false;
}

// IDA_EA: 0x85E6D0C
// IDA_NAME: _ZN15CMission_withinC2ERK13MissionScriptiii
// DEMANGLED: CMission_within::CMission_within(MissionScript const&,int,int,int)
CMission_within::CMission_within(const MissionScript& script,
                                 int attempts,
                                 int required_wins,
                                 int unused)
    : CMission(script),
      attempts_(attempts),
      required_wins_(required_wins),
      unused_(unused) {}

// IDA_EA: 0x85EC7DE
// IDA_NAME: _ZN15CMission_withinD2Ev
// DEMANGLED: CMission_within::~CMission_within()
// IDA_EA: 0x85EC80E
// IDA_NAME: _ZN15CMission_withinD0Ev
// DEMANGLED: CMission_within::~CMission_within() deleting destructor
CMission_within::~CMission_within() {}

// IDA_EA: 0x85E6D4A
// IDA_NAME: _ZNK15CMission_within15isSeriesMissionEv
// DEMANGLED: CMission_within::isSeriesMission(void)const
bool CMission_within::isSeriesMission() const { return false; }

// IDA_EA: 0x85E6D54
// IDA_NAME: _ZNK15CMission_within17getClearConditionEv
// DEMANGLED: CMission_within::getClearCondition(void)const
int CMission_within::getClearCondition() const { return required_wins_; }

// IDA_EA: 0x85E3E5C
// IDA_NAME: _ZNK19CMission_remains_hp18_isAcceptConditionERK5CUser
// DEMANGLED: CMission_remains_hp::_isAcceptCondition(CUser const&)const
bool CMission_remains_hp::_isAcceptCondition(const CUser& user) const {
    const CMissionList_Charac* mission_list =
        static_cast<const CMissionList_Charac*>(user.GetCharacExpandDataR(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
    return !mission_list->isClearMission(GetMissionIndex()) &&
           user.get_charac_job() == target_character_;
}

// IDA_EA: 0x85E3EC6
// IDA_NAME: _ZNK19CMission_remains_hp13verifyMissionERK11MissionInfo
// DEMANGLED: CMission_remains_hp::verifyMission(MissionInfo const&)const
bool CMission_remains_hp::verifyMission(const MissionInfo& info) const {
    return clear_count_ + 10 > static_cast<int16_t>(info.current_value);
}

// IDA_EA: 0x85E3EEC
// IDA_NAME: _ZNK19CMission_remains_hp6UpdateER11MissionInfoRK31MissionClearCondition_Parameter
// DEMANGLED: CMission_remains_hp::Update(MissionInfo &,MissionClearCondition_Parameter const&)const
int CMission_remains_hp::Update(
    MissionInfo& info, const MissionClearCondition_Parameter&) const {
    ++info.current_value;
    return 1;
}

// IDA_EA: 0x85E3F08
// IDA_NAME: _ZNK19CMission_remains_hp11isClearableER11MissionInfoRb
// DEMANGLED: CMission_remains_hp::isClearable(MissionInfo &,bool &)const
bool CMission_remains_hp::isClearable(MissionInfo& info, bool&) const {
    return clear_count_ <= static_cast<int16_t>(info.current_value);
}

// IDA_EA: 0x85E6D60
// IDA_NAME: _ZN19CMission_remains_hpC2ERK13MissionScriptii
// DEMANGLED: CMission_remains_hp::CMission_remains_hp(MissionScript const&,int,int)
CMission_remains_hp::CMission_remains_hp(const MissionScript& script,
                                         int remain_percent,
                                         int clear_count)
    : CMission(script),
      remain_percent_(remain_percent),
      clear_count_(clear_count) {}

// IDA_EA: 0x85EC790
// IDA_NAME: _ZN19CMission_remains_hpD2Ev
// DEMANGLED: CMission_remains_hp::~CMission_remains_hp()
// IDA_EA: 0x85EC7C0
// IDA_NAME: _ZN19CMission_remains_hpD0Ev
// DEMANGLED: CMission_remains_hp::~CMission_remains_hp() deleting destructor
CMission_remains_hp::~CMission_remains_hp() {}

// IDA_EA: 0x85E6D96
// IDA_NAME: _ZNK19CMission_remains_hp15isSeriesMissionEv
// DEMANGLED: CMission_remains_hp::isSeriesMission(void)const
bool CMission_remains_hp::isSeriesMission() const { return false; }

// IDA_EA: 0x85E6DA0
// IDA_NAME: _ZNK19CMission_remains_hp17getClearConditionEv
// DEMANGLED: CMission_remains_hp::getClearCondition(void)const
int CMission_remains_hp::getClearCondition() const { return clear_count_; }

// IDA_EA: 0x85E3F2C
// IDA_NAME: _ZNK21CMission_move_channel18_isAcceptConditionERK5CUser
// DEMANGLED: CMission_move_channel::_isAcceptCondition(CUser const&)const
bool CMission_move_channel::_isAcceptCondition(const CUser& user) const {
    return _isAcceptCondition_Series(user);
}

// IDA_EA: 0x85E3F46
// IDA_NAME: _ZNK21CMission_move_channel13verifyMissionERK11MissionInfo
// DEMANGLED: CMission_move_channel::verifyMission(MissionInfo const&)const
bool CMission_move_channel::verifyMission(const MissionInfo&) const {
    return true;
}

// IDA_EA: 0x85E3F50
// IDA_NAME: _ZNK21CMission_move_channel6UpdateER11MissionInfoRK31MissionClearCondition_Parameter
// DEMANGLED: CMission_move_channel::Update(MissionInfo &,MissionClearCondition_Parameter const&)const
int CMission_move_channel::Update(
    MissionInfo& info,
    const MissionClearCondition_Parameter& parameter) const {
    info.current_value = static_cast<uint16_t>(parameter.pvp_channel_type);
    info.target_value = static_cast<uint16_t>(parameter.channel_type);
    return 1;
}

// IDA_EA: 0x85E3F78
// IDA_NAME: _ZNK21CMission_move_channel11isClearableER11MissionInfoRb
// DEMANGLED: CMission_move_channel::isClearable(MissionInfo &,bool &)const
bool CMission_move_channel::isClearable(MissionInfo& info, bool&) const {
    if (info.target_value == 16) {
        return true;
    }
    if (channel_type_ == static_cast<int16_t>(info.current_value)) {
        return true;
    }
    return channel_type_ == 10 && info.current_value == 3;
}

// IDA_EA: 0x85E6DAC
// IDA_NAME: _ZN21CMission_move_channelC2ERK13MissionScripti
// DEMANGLED: CMission_move_channel::CMission_move_channel(MissionScript const&,int)
CMission_move_channel::CMission_move_channel(const MissionScript& script,
                                             int channel_type)
    : CMission(script), channel_type_(channel_type) {}

// IDA_EA: 0x85EC742
// IDA_NAME: _ZN21CMission_move_channelD2Ev
// DEMANGLED: CMission_move_channel::~CMission_move_channel()
// IDA_EA: 0x85EC772
// IDA_NAME: _ZN21CMission_move_channelD0Ev
// DEMANGLED: CMission_move_channel::~CMission_move_channel() deleting destructor
CMission_move_channel::~CMission_move_channel() {}

// IDA_EA: 0x85E6DD8
// IDA_NAME: _ZNK21CMission_move_channel15isSeriesMissionEv
// DEMANGLED: CMission_move_channel::isSeriesMission(void)const
bool CMission_move_channel::isSeriesMission() const { return true; }

// IDA_EA: 0x85E6DE2
// IDA_NAME: _ZNK21CMission_move_channel17getClearConditionEv
// DEMANGLED: CMission_move_channel::getClearCondition(void)const
int CMission_move_channel::getClearCondition() const { return 1; }

// IDA_EA: 0x85E3FCE
// IDA_NAME: _ZNK27CMission_move_channel_total18_isAcceptConditionERK5CUser
// DEMANGLED: CMission_move_channel_total::_isAcceptCondition(CUser const&)const
bool CMission_move_channel_total::_isAcceptCondition(
    const CUser& user) const {
    const CMissionList_Charac* mission_list =
        static_cast<const CMissionList_Charac*>(user.GetCharacExpandDataR(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
    if (mission_list->getIndex_byKind(27) != 0) {
        return false;
    }
    if (mission_list->isClearMission(GetMissionIndex())) {
        return false;
    }
    if (GetPrevMissionIndex() == -1) {
        return false;
    }
    return !mission_list->isClearMission(GetPrevMissionIndex()) &&
           mission_list->getIndex_byKind(7) == 0;
}

// IDA_EA: 0x85E40A2
// IDA_NAME: _ZNK27CMission_move_channel_total13verifyMissionERK11MissionInfo
// DEMANGLED: CMission_move_channel_total::verifyMission(MissionInfo const&)const
bool CMission_move_channel_total::verifyMission(const MissionInfo&) const {
    return true;
}

// IDA_EA: 0x85E40AC
// IDA_NAME: _ZNK27CMission_move_channel_total6UpdateER11MissionInfoRK31MissionClearCondition_Parameter
// DEMANGLED: CMission_move_channel_total::Update(MissionInfo &,MissionClearCondition_Parameter const&)const
int CMission_move_channel_total::Update(
    MissionInfo& info,
    const MissionClearCondition_Parameter& parameter) const {
    info.current_value = static_cast<uint16_t>(parameter.pvp_channel_type);
    info.target_value = static_cast<uint16_t>(parameter.channel_type);
    return 1;
}

// IDA_EA: 0x85E40D4
// IDA_NAME: _ZNK27CMission_move_channel_total11isClearableER11MissionInfoRb
// DEMANGLED: CMission_move_channel_total::isClearable(MissionInfo &,bool &)const
bool CMission_move_channel_total::isClearable(
    MissionInfo& info, bool&) const {
    if (info.target_value == 16) {
        return true;
    }
    if (channel_type_ == static_cast<int16_t>(info.current_value)) {
        return true;
    }
    return channel_type_ == 10 && info.current_value == 3;
}

// IDA_EA: 0x85E6DEC
// IDA_NAME: _ZN27CMission_move_channel_totalC2ERK13MissionScripti
// DEMANGLED: CMission_move_channel_total::CMission_move_channel_total(MissionScript const&,int)
CMission_move_channel_total::CMission_move_channel_total(
    const MissionScript& script, int channel_type)
    : CMission(script), channel_type_(channel_type) {}

// IDA_EA: 0x85EC6F4
// IDA_NAME: _ZN27CMission_move_channel_totalD2Ev
// DEMANGLED: CMission_move_channel_total::~CMission_move_channel_total()
// IDA_EA: 0x85EC724
// IDA_NAME: _ZN27CMission_move_channel_totalD0Ev
// DEMANGLED: CMission_move_channel_total::~CMission_move_channel_total() deleting destructor
CMission_move_channel_total::~CMission_move_channel_total() {}

// IDA_EA: 0x85E6E18
// IDA_NAME: _ZNK27CMission_move_channel_total15isSeriesMissionEv
// DEMANGLED: CMission_move_channel_total::isSeriesMission(void)const
bool CMission_move_channel_total::isSeriesMission() const { return true; }

// IDA_EA: 0x85E6E22
// IDA_NAME: _ZNK27CMission_move_channel_total17getClearConditionEv
// DEMANGLED: CMission_move_channel_total::getClearCondition(void)const
int CMission_move_channel_total::getClearCondition() const { return 1; }

// IDA_EA: 0x85E412A
// IDA_NAME: _ZNK20CMission_combo_clear13verifyMissionERK11MissionInfo
// DEMANGLED: CMission_combo_clear::verifyMission(MissionInfo const&)const
bool CMission_combo_clear::verifyMission(const MissionInfo&) const {
    return true;
}

// IDA_EA: 0x85E4134
// IDA_NAME: _ZNK20CMission_combo_clear7_RewardER5CUser
// DEMANGLED: CMission_combo_clear::_Reward(CUser &)const
int CMission_combo_clear::_Reward(CUser& user) const {
    combo_script_key key;
    key.job = static_cast<uint8_t>(user.get_charac_job());
    key.skill_index = static_cast<uint16_t>(user.getCurCharFirstGrowType());
    std::map<combo_script_key, combo_script_data>::const_iterator it =
        combo_clear_.find(key);
    if (it != combo_clear_.end()) {
        std::vector<std::pair<int, int> > items;
        items.push_back(it->second.value_range);
        std::vector<int> inserted_slots;
        const int inserted = user.getCurCharacInvenW()->insert_event_items(
            items,
            inserted_slots,
            static_cast<eMoneyAddReason>(30),
            static_cast<eItemAddReason>(51),
            true,
            "game_server_msg_173",
            "game_server_msg_174");
        if (inserted > 0 && inserted_slots.size() > 2u) {
            user.SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                                static_cast<ENUM_ITEMSPACE>(0),
                                inserted_slots[0]);
        }
    }
    return 1;
}

// IDA_EA: 0x85E42F0
// IDA_NAME: _ZNK20CMission_combo_clear18_isAcceptConditionERK5CUser
// DEMANGLED: CMission_combo_clear::_isAcceptCondition(CUser const&)const
bool CMission_combo_clear::_isAcceptCondition(const CUser& user) const {
    if (!_isAcceptCondition_Series(user)) {
        return false;
    }
    combo_script_key key;
    key.job = static_cast<uint8_t>(user.get_charac_job());
    key.skill_index = static_cast<uint16_t>(user.getCurCharFirstGrowType());
    return combo_clear_.find(key) != combo_clear_.end();
}

// IDA_EA: 0x85E439E
// IDA_NAME: _ZNK20CMission_combo_clear21_makeTotalMissionInfoER11MissionInfoRK5CUser
// DEMANGLED: CMission_combo_clear::_makeTotalMissionInfo(MissionInfo &,CUser const&)const
void CMission_combo_clear::_makeTotalMissionInfo(
    MissionInfo& info, const CUser& user) const {
    combo_script_key key;
    key.job = static_cast<uint8_t>(user.get_charac_job());
    key.skill_index = static_cast<uint16_t>(user.getCurCharFirstGrowType());
    std::map<combo_script_key, combo_script_data>::const_iterator it =
        combo_clear_.find(key);
    if (it == combo_clear_.end()) {
        return;
    }
    const int count = static_cast<int>(it->second.size());
    for (int i = 0; i <= 3 && i < count; ++i) {
        info.current_value = static_cast<uint16_t>(info.current_value * 16);
        info.current_value = static_cast<uint16_t>(
            info.current_value + (it->second[i].second & 0x0f));
    }
    if (count > 4) {
        for (int i = 4; i <= 7; ++i) {
            info.target_value = static_cast<uint16_t>(info.target_value * 16);
            if (i < count) {
                info.target_value = static_cast<uint16_t>(
                    info.target_value + (it->second[i].second & 0x0f));
            }
        }
    }
}

// IDA_EA: 0x85E452E
// IDA_NAME: _ZNK20CMission_combo_clear11isClearableER11MissionInfoRb
// DEMANGLED: CMission_combo_clear::isClearable(MissionInfo &,bool &)const
bool CMission_combo_clear::isClearable(MissionInfo& info, bool&) const {
    return info.current_value == 0 && info.target_value == 0;
}

// IDA_EA: 0x85E4558
// IDA_NAME: _ZNK20CMission_combo_clear15isSeriesMissionEv
// DEMANGLED: CMission_combo_clear::isSeriesMission(void)const
bool CMission_combo_clear::isSeriesMission() const { return true; }

// IDA_EA: 0x85E4562
// IDA_NAME: _ZNK20CMission_combo_clear6UpdateER11MissionInfoRK31MissionClearCondition_Parameter
// DEMANGLED: CMission_combo_clear::Update(MissionInfo &,MissionClearCondition_Parameter const&)const
int CMission_combo_clear::Update(
    MissionInfo&, const MissionClearCondition_Parameter&) const {
    return 1;
}

// IDA_EA: 0x85E456C
// IDA_NAME: _ZNK20CMission_combo_clear17getClearConditionEv
// DEMANGLED: CMission_combo_clear::getClearCondition(void)const
int CMission_combo_clear::getClearCondition() const { return 0; }

// IDA_EA: 0x85E6E40
// IDA_NAME: _ZN20CMission_combo_clearC2ERK13MissionScriptRKSt3mapI16combo_script_key17combo_script_dataSt4lessIS4_ESaISt4pairIKS4_S5_EEE
// DEMANGLED: CMission_combo_clear::CMission_combo_clear(MissionScript const&,std::map<combo_script_key,combo_script_data> const&)
CMission_combo_clear::CMission_combo_clear(
    const MissionScript& script,
    const std::map<combo_script_key, combo_script_data>& combo_clear)
    : CMission(script), combo_clear_(combo_clear) {}

// IDA_EA: 0x85EC674
// IDA_NAME: _ZN20CMission_combo_clearD2Ev
// DEMANGLED: CMission_combo_clear::~CMission_combo_clear()
// IDA_EA: 0x85EC6D6
// IDA_NAME: _ZN20CMission_combo_clearD0Ev
// DEMANGLED: CMission_combo_clear::~CMission_combo_clear() deleting destructor
CMission_combo_clear::~CMission_combo_clear() {}

// IDA_EA: 0x85E709E
// IDA_NAME: _ZN15MissionLessThanclEPK8CMissionS2_
// DEMANGLED: MissionLessThan::operator()(CMission const *,CMission const *)
bool MissionLessThan::operator()(const CMission* lhs, const CMission* rhs) {
    if (lhs->GetMissionKind() == rhs->GetMissionKind()) {
        if (lhs->GetMissionIndex() == rhs->GetMissionIndex()) {
            return false;
        }
        return lhs->GetMissionIndex() < rhs->GetMissionIndex();
    }
    return lhs->GetMissionKind() < rhs->GetMissionKind();
}

// IDA_EA: 0x85E1ABC
// IDA_NAME: _ZN17PvP_MissionSystemC2Ev
// DEMANGLED: PvP_MissionSystem::PvP_MissionSystem(void)
PvP_MissionSystem::PvP_MissionSystem() {
    unsigned int seed = static_cast<unsigned int>(std::time(0));
    random_ = new CMTRand(seed);
    casual_daily_total_weight_ = 0;
    daily_mission_kind_.reset();
    kind_missions_.resize(36);
}

// IDA_EA: 0x85E1C48
// IDA_NAME: _ZN17PvP_MissionSystemD2Ev
// DEMANGLED: PvP_MissionSystem::~PvP_MissionSystem()
PvP_MissionSystem::~PvP_MissionSystem() {
    delete random_;
    random_ = 0;
    std::puts("Mission System Delete");
    for (MissionMap::iterator it = missions_.begin(); it != missions_.end();
         ++it) {
        delete it->second;
    }
    missions_.clear();
}

// IDA_EA: 0x85E1DC6
// IDA_NAME: _ZN17PvP_MissionSystem9Load_listEv
// DEMANGLED: PvP_MissionSystem::Load_list(void)
int PvP_MissionSystem::Load_list() {
    MissionScript script;

    for (STScriptFileList::iterator it =
             g_PvpMissionScriptFileList.begin();
         it != g_PvpMissionScriptFileList.end();
         ++it) {
        const char* filename = it->second.c_str();
        if (filename == 0) {
            break;
        }
        if (importPvpMissionScript(&script, filename) != 1) {
            std::printf("Mission Script Error: %s\n", filename);
            return 0;
        }
        CMission* mission = Factory(script);
        if (mission == 0) {
            return 0;
        }

        const int mission_index = mission->GetMissionIndex();
        missions_.insert(std::make_pair(mission_index, mission));
        const int mission_kind = mission->GetMissionKind();
        kind_missions_[mission_kind].push_back(mission);
        if (mission->GetMissionGrade() == 2) {
            daily_mission_kind_[mission_kind] = true;
        }
    }

    sort_list();
    return 1;
}

// IDA_EA: 0x85E1FEE
// IDA_NAME: _ZN17PvP_MissionSystem17Load_EtcParameterEv
// DEMANGLED: PvP_MissionSystem::Load_EtcParameter(void)
int PvP_MissionSystem::Load_EtcParameter() {
    MissionParameterScript script;
    if (importPvpMissionParameterScript(script) == 1) {
        copy_missionParameter(script);
        return 1;
    }
    std::puts("Mission Parameter Error!");
    return 0;
}

// IDA_EA: 0x85E206E
// IDA_NAME: _ZN17PvP_MissionSystem21copy_missionParameterERK22MissionParameterScript
// DEMANGLED: PvP_MissionSystem::copy_missionParameter(MissionParameterScript const&)
int PvP_MissionSystem::copy_missionParameter(
    const MissionParameterScript& script) {
    frequent_daily_missions_ = script.frequent_daily_missions;
    casual_daily_count_ = script.casual_daily_count;
    casual_daily_missions_ = script.casual_daily_missions;
    casual_daily_total_weight_ = script.casual_daily_total_weight;
    base_mission_exp_by_rank_ = script.base_mission_exp_by_rank;
    return 0;
}

// IDA_EA: 0x85E20D4
// IDA_NAME: _ZN17PvP_MissionSystem9sort_listEv
// DEMANGLED: PvP_MissionSystem::sort_list(void)
int PvP_MissionSystem::sort_list() {
    for (unsigned int i = 0; i < kind_missions_.size(); ++i) {
        kind_missions_[i].sort(MissionLessThan());
    }
    return 0;
}

// IDA_EA: 0x85E2132
// IDA_NAME: _ZN17PvP_MissionSystem7FactoryERK13MissionScript
// DEMANGLED: PvP_MissionSystem::Factory(MissionScript const&)
CMission* PvP_MissionSystem::Factory(const MissionScript& script) {
    switch (script.condition_type) {
        case kWinningCount:
            return new CMission_winning_count(script,
                                              script.condition_param1);
        case kWinningStreak:
            return new CMission_winning_streak(script,
                                               script.condition_param1,
                                               script.condition_param2);
        case kJoin:
            return new CMission_join(script, script.condition_param1);
        case kKillCount:
            return new CMission_kill_count(script, script.condition_param1);
        case kHereafterJoin:
            return new CMission_hereafter_join(script,
                                               script.condition_param1);
        case kHereafterWinningCount:
            return new CMission_hereafter_winning_count(
                script, script.condition_param1);
        case kRemainHp:
            return new CMission_remain_hp(script,
                                          script.condition_param1,
                                          script.condition_param2);
        case kRevenge:
            return new CMission_revenge(script, script.condition_param1);
        case kHereafterKillCount:
            return new CMission_hereafter_kill_count(
                script, script.condition_param1);
        case kKill:
            return new CMission_kill(script,
                                     script.condition_param1,
                                     script.condition_param2);
        case kWithin:
            return new CMission_within(script,
                                       script.condition_param1,
                                       script.condition_param2,
                                       script.condition_param3);
        case kRemainSHp:
            return new CMission_remains_hp(script,
                                           script.condition_param1,
                                           script.condition_param2);
        case kMoveChannel:
            return new CMission_move_channel(script,
                                             script.condition_param1);
        case kMoveChannelTotal:
            return new CMission_move_channel_total(
                script, script.condition_param1);
        case kComboClear:
            return new CMission_combo_clear(script, script.combo_clear);
        default:
            return 0;
    }
}

// IDA_EA: 0x85E262E
// IDA_NAME: _ZN17PvP_MissionSystem12find_missionEi
// DEMANGLED: PvP_MissionSystem::find_mission(int)
CMission* PvP_MissionSystem::find_mission(int mission_index) {
    MissionMap::iterator it = missions_.find(mission_index);
    if (it == missions_.end()) {
        return 0;
    }
    return it->second;
}

// IDA_EA: 0x85E2692
// IDA_NAME: _ZN17PvP_MissionSystem21get_kind_mission_listEj
// DEMANGLED: PvP_MissionSystem::get_kind_mission_list(unsigned int)
std::list<CMission*>& PvP_MissionSystem::get_kind_mission_list(
    unsigned int mission_kind) {
    return kind_missions_[mission_kind];
}

// IDA_EA: 0x85E26E0
// IDA_NAME: _ZN17PvP_MissionSystem20get_New_DailyMissionERSt6vectorISt4pairIjjESaIS2_EE
// DEMANGLED: PvP_MissionSystem::get_New_DailyMission(std::vector<std::pair<unsigned int,unsigned int> > &)
void PvP_MissionSystem::get_New_DailyMission(
    std::vector<std::pair<unsigned int, unsigned int> >& missions) {
    for (std::vector<unsigned int>::const_iterator it =
             frequent_daily_missions_.begin();
         it != frequent_daily_missions_.end();
         ++it) {
        MissionMap::iterator mission_it = missions_.find(*it);
        if (mission_it != missions_.end() && mission_it->second != 0) {
            missions.push_back(std::make_pair(
                static_cast<unsigned int>(mission_it->second->GetMissionKind()),
                static_cast<unsigned int>(
                    mission_it->second->GetMissionIndex())));
        }
    }

    std::list<std::pair<unsigned int, unsigned int> > casual =
        casual_daily_missions_;
    unsigned int max_dice = casual_daily_total_weight_;
    for (unsigned int i = 0; i < casual_daily_count_; ++i) {
        const unsigned int dice = random_->randInt() % max_dice;
        unsigned int current_weight = 0;
        for (std::list<std::pair<unsigned int, unsigned int> >::iterator it =
                 casual.begin();
             it != casual.end();
             ++it) {
            current_weight += it->first;
            if (dice < current_weight) {
                MissionMap::iterator mission_it = missions_.find(it->second);
                if (mission_it != missions_.end()) {
                    if (mission_it->second != 0) {
                        missions.push_back(std::make_pair(
                            static_cast<unsigned int>(
                                mission_it->second->GetMissionKind()),
                            static_cast<unsigned int>(
                                mission_it->second->GetMissionIndex())));
                    }
                    max_dice -= it->first;
                    casual.erase(it);
                    break;
                }
            }
        }
        if (static_cast<int>(max_dice) <= 0) {
            cMyTrace trace(
                "void PvP_MissionSystem::get_New_DailyMission(std::vector<std::pair<unsigned int, unsigned int>, std::allocator<std::pair<unsigned int, unsigned int> > >&)",
                291,
                5);
            trace("[@missionSystem] InvalidMaxDice %d", max_dice);
            return;
        }
    }

    std::bitset<32> created_kind;
    for (unsigned int i = 0; i < missions.size(); ++i) {
        const unsigned int kind = missions[i].first;
        if (kind == 0 || kind > 32) {
            char buffer[64];
            std::string list;
            for (unsigned int index = 0; index < missions.size(); ++index) {
                std::sprintf(buffer,
                             "<K:%d, I:%d>",
                             missions[index].first,
                             missions[index].second);
                list += buffer;
            }
            cMyTrace trace(
                "void PvP_MissionSystem::get_New_DailyMission(std::vector<std::pair<unsigned int, unsigned int>, std::allocator<std::pair<unsigned int, unsigned int> > >&)",
                313,
                5);
            trace("[@missionSystem] OverKind: <Invalid mission kind : %d> <mission index : %d> <Created DailyMission Size : %d> <List : %s >",
                  kind,
                  missions[i].second,
                  static_cast<int>(missions.size()),
                  list.c_str());
            missions.clear();
            return;
        }
        if (created_kind[kind]) {
            char buffer[64];
            std::string list;
            for (unsigned int index = 0; index < missions.size(); ++index) {
                std::sprintf(buffer,
                             "<K:%d, I:%d>",
                             missions[index].first,
                             missions[index].second);
                list += buffer;
            }
            cMyTrace trace(
                "void PvP_MissionSystem::get_New_DailyMission(std::vector<std::pair<unsigned int, unsigned int>, std::allocator<std::pair<unsigned int, unsigned int> > >&)",
                333,
                5);
            trace("[@missionSystem] OverlappedKind: <Invalid mission kind : %d> <mission index : %d> <Created DailyMission Size : %d> <List : %s >",
                  kind,
                  missions[i].second,
                  static_cast<int>(missions.size()),
                  list.c_str());
            missions.clear();
            return;
        }
        created_kind[kind] = true;
    }
}

// IDA_EA: 0x8376312
// IDA_NAME: _ZNK17PvP_MissionSystem22get_daily_mission_kindEv
// DEMANGLED: PvP_MissionSystem::get_daily_mission_kind(void)const
std::bitset<32> PvP_MissionSystem::get_daily_mission_kind() const {
    return daily_mission_kind_;
}

// IDA_EA: 0x85E2E74
// IDA_NAME: _ZNK17PvP_MissionSystem25get_BaseMissionExp_byRankEt
// DEMANGLED: PvP_MissionSystem::get_BaseMissionExp_byRank(unsigned short)const
unsigned int PvP_MissionSystem::get_BaseMissionExp_byRank(
    unsigned short rank) const {
    if (rank < base_mission_exp_by_rank_.size()) {
        return base_mission_exp_by_rank_[rank];
    }
    cMyTrace trace(
        "unsigned int PvP_MissionSystem::get_BaseMissionExp_byRank(short unsigned int) const",
        354,
        5);
    trace("[@missionSystem] <InvalidRank : %d>", rank);
    return 0;
}

// IDA_EA: 0x85E2F00
// IDA_NAME: _ZNK17PvP_MissionSystem22get_WithinMissionIndexEi
// DEMANGLED: PvP_MissionSystem::get_WithinMissionIndex(int)const
int PvP_MissionSystem::get_WithinMissionIndex(int pvp_grade) const {
    const std::list<CMission*>& list = kind_missions_[27];
    for (std::list<CMission*>::const_iterator it = list.begin();
         it != list.end();
         ++it) {
        const std::pair<int, int> range = (*it)->GetRankRange();
        if (range.first == pvp_grade) {
            return (*it)->GetMissionIndex();
        }
    }
    return 0;
}

// IDA_EA: 0x8A5AC28
// IDA_NAME: _Z22importPvpMissionScriptP13MissionScriptPKc
// DEMANGLED: importPvpMissionScript(MissionScript *,char const*)
int importPvpMissionScript(MissionScript* script, const char* filename) {
    if (loadRDARScriptFile(g_PvpMissionScriptBaseDirectory,
                                   filename) != 1) {
        return 0;
    }

    script->clear();
    script->script_index =
        g_PvpMissionScriptFileList.FindIndexByFullScan(filename);

    std::string tag;
    std::string text;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[rank range]") {
            script->rank_range.first =
                ScanInt(static_cast<bool*>(0));
            script->rank_range.second =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[kind]") {
            script->mission_kind = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[rate range]") {
            script->rate_range.first =
                ScanInt(static_cast<bool*>(0));
            script->rate_range.second =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[grade]") {
            ScanStr(&text);
            if (text == "[normal]") {
                script->mission_grade = 0;
            } else if (text == "[repeatable]") {
                script->mission_grade = 1;
            } else if (text == "[daily]") {
                script->mission_grade = 2;
            } else if (text == "[rank up]") {
                script->mission_grade = 3;
            }
        } else if (tag == "[target character]") {
            if (ScanStr(&text) != 1) {
                break;
            }
            if (::checkValiedJobName(text.c_str()) != 1) {
                std::printf("parsing error : %s\n", text.c_str());
                return 0;
            }
            script->target_character = ::getCharacterJob(text.c_str());
        } else if (tag == "[pre required mission]") {
            while (1) {
                const int mission = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->pre_required_missions.push_back(mission);
            }
        } else if (tag == "[condition]") {
            if (ScanStr(&text) != 1) {
                break;
            }
            if (text == "[winning count]") {
                script->condition_type = kWinningCount;
                _ReadThreeInt(script);
            } else if (text == "[winning streak]") {
                script->condition_type = kWinningStreak;
                _ReadThreeInt(script);
            } else if (text == "[join]") {
                script->condition_type = kJoin;
                _ReadThreeInt(script);
            } else if (text == "[revenge]") {
                script->condition_type = kRevenge;
                _ReadThreeInt(script);
            } else if (text == "[kill]") {
                script->condition_type = kKill;
                _ReadThreeInt(script);
            } else if (text == "[kill count]") {
                script->condition_type = kKillCount;
                _ReadThreeInt(script);
            } else if (text == "[hereafter join]") {
                script->condition_type = kHereafterJoin;
                _ReadThreeInt(script);
            } else if (text == "[hereafter winning count]") {
                script->condition_type = kHereafterWinningCount;
                _ReadThreeInt(script);
            } else if (text == "[remain HP]") {
                script->condition_type = kRemainHp;
                _ReadThreeInt(script);
            } else if (text == "[hereafter kill count]") {
                script->condition_type = kHereafterKillCount;
                _ReadThreeInt(script);
            } else if (text == "[within]") {
                script->condition_type = kWithin;
                _ReadFourInt(script);
            } else if (text == "[remainS HP]") {
                script->condition_type = kRemainSHp;
                _ReadThreeInt(script);
            } else if (text == "[move_channel]") {
                script->condition_type = kMoveChannel;
                _ReadThreeInt(script);
            } else if (text == "[move_channel_total]") {
                script->condition_type = kMoveChannelTotal;
                _ReadThreeInt(script);
            } else if (text == "[combo clear]") {
                script->condition_type = kComboClear;
                std::string combo_tag;
                while (ScanType(combo_tag, true) == 1) {
                    combo_script_key key;
                    combo_script_data data;
                    if (combo_tag != "[job]") {
                        continue;
                    }
                    if (ScanStr(&text) != 1) {
                        std::printf("parsing error %d, %s\n",
                                    297,
                                    text.c_str());
                        return 0;
                    }
                    if (::checkValiedJobName(text.c_str()) != 1) {
                        std::printf("parsing error : %s\n", text.c_str());
                        return 0;
                    }

                    key.job = static_cast<unsigned char>(
                        ::getCharacterJob(text.c_str()));
                    key.skill_index = static_cast<unsigned short>(
                        ScanInt(static_cast<bool*>(0)));
                    const int count = ScanInt(static_cast<bool*>(0));
                    data.resize(count);
                    for (int index = 0; index < count; ++index) {
                        data[index].first =
                            ScanInt(static_cast<bool*>(0));
                        data[index].second =
                            ScanInt(static_cast<bool*>(0));
                    }

                    if (ScanType(combo_tag, true) == 1) {
                        if (combo_tag == "[reward item]") {
                            data.value_range.first =
                                ScanInt(static_cast<bool*>(0));
                            data.value_range.second =
                                ScanInt(static_cast<bool*>(0));
                        }
                        ScanType(combo_tag, true);
                        if (script->combo_clear.find(key) ==
                            script->combo_clear.end()) {
                            script->combo_clear.insert(
                                std::make_pair(key, data));
                        }
                    }
                }
            }
        } else if (tag == "[prev mission]") {
            script->prev_mission_index =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[next mission]") {
            script->next_mission_index =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[name_text]") {
            ScanStr(&script->name_text);
        } else if (tag == "[cond_text]") {
            ScanStr(&script->condition_text);
        } else if (tag == "[reward item]") {
            while (1) {
                const int item = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int count = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                script->reward_items.push_back(std::make_pair(item, count));
            }
        } else if (tag == "[reward skill]") {
            script->reward_skill = true;
        } else if (tag == "[reward sp]") {
            script->reward_sp = true;
        } else if (tag == "[difficulty_ratio]") {
            script->difficulty_ratio =
                ScanFloat(static_cast<bool*>(0));
        } else if (tag == "[stealing skill]") {
            script->stealing_skill = true;
        }
    }

    return 1;
}

// IDA_EA: 0x8A5B7EC
// IDA_NAME: _Z31importPvpMissionParameterScriptR22MissionParameterScript
// DEMANGLED: importPvpMissionParameterScript(MissionParameterScript &)
int importPvpMissionParameterScript(MissionParameterScript& script) {
    script.clear();
    if (loadRDARScriptFile("Etc/", "MissionSystem.etc") != 1) {
        return 0;
    }

    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[frequent daily mission]") {
            const int count = ScanInt(&success);
            if (!success) {
                return 0;
            }
            for (int i = 0; i < count; ++i) {
                const unsigned int mission =
                    static_cast<unsigned int>(ScanInt(&success));
                if (!success) {
                    return 0;
                }
                script.frequent_daily_missions.push_back(mission);
            }
        } else if (tag == "[casual daily mission]") {
            script.casual_daily_count =
                static_cast<unsigned int>(ScanInt(&success));
            if (!success) {
                return 0;
            }
            while (1) {
                const unsigned int mission =
                    static_cast<unsigned int>(ScanInt(&success));
                if (!success) {
                    break;
                }
                const unsigned int weight =
                    static_cast<unsigned int>(ScanInt(&success));
                if (!success) {
                    return 0;
                }
                script.casual_daily_missions.push_back(
                    std::make_pair(weight, mission));
                script.casual_daily_total_weight += weight;
            }
        } else if (tag == "[mission reward exp table]") {
            script.base_mission_exp_by_rank.resize(30, 0);
            for (int i = 0; i <= 29; ++i) {
                script.base_mission_exp_by_rank[i] =
                    static_cast<unsigned int>(ScanInt(&success));
                if (!success) {
                    return 0;
                }
            }
        }
    }
    return 1;
}
