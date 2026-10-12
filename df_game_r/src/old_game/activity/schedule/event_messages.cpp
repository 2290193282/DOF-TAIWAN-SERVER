#include "old_game/activity/schedule/event_messages.h"

#include <cstring>


typedef char MSG_BINGO_QUIZ_Size_Must_Be_0x11[
    (sizeof(MSG_BINGO_QUIZ) == 0x11) ? 1 : -1];
typedef char MSG_FATIGUE_ATTENDANCE_Size_Must_Be_0x0f[
    (sizeof(MSG_FATIGUE_ATTENDANCE) == 0x0f) ? 1 : -1];
typedef char MSG_BLUE_MARBLE_Size_Must_Be_0x0e[
    (sizeof(MSG_BLUE_MARBLE) == 0x0e) ? 1 : -1];
typedef char MsgTournamentRewardSelect_Size_Must_Be_0x0f[
    (sizeof(MsgTournamentRewardSelect) == 0x0f) ? 1 : -1];
typedef char MSG_EXCHANGE_RANDOM_ITEM_REWARD_Size_Must_Be_0x1c[
    (sizeof(MSG_EXCHANGE_RANDOM_ITEM_REWARD) == 0x1c) ? 1 : -1];
typedef char MSG_AVATAR_DISJOINT_RANDOM_REWARD_Size_Must_Be_0x14[
    (sizeof(MSG_AVATAR_DISJOINT_RANDOM_REWARD) == 0x14) ? 1 : -1];
typedef char stBingoQuiz_Size_Must_Be_0x0c[
    (sizeof(stBingoQuiz) == 0x0c) ? 1 : -1];
typedef char stBingoReward_Size_Must_Be_0x0c[
    (sizeof(stBingoReward) == 0x0c) ? 1 : -1];
typedef char stExchangeRandomItemReward_Size_Must_Be_0x24[
    (sizeof(stExchangeRandomItemReward) == 0x24) ? 1 : -1];
typedef char stAvatarDisjointRandomReward_Size_Must_Be_0x18[
    (sizeof(stAvatarDisjointRandomReward) == 0x18) ? 1 : -1];
typedef char stFatigueAttendance_Size_Must_Be_0x14[
    (sizeof(stFatigueAttendance) == 0x14) ? 1 : -1];
typedef char stBlueMarble_Size_Must_Be_0x08[
    (sizeof(stBlueMarble) == 0x08) ? 1 : -1];
typedef char stBlueMarbleConfirmInfo_Size_Must_Be_0x08[
    (sizeof(stBlueMarbleConfirmInfo) == 0x08) ? 1 : -1];
typedef char stTournamentRewardSelect_Size_Must_Be_0x08[
    (sizeof(stTournamentRewardSelect) == 0x08) ? 1 : -1];
typedef char stTournamentRewardSelectState_Size_Must_Be_0x08[
    (sizeof(stTournamentRewardSelectState) == 0x08) ? 1 : -1];
typedef char SIG_ATTENDANCE_EVENT_Size_Must_Be_0x01[
    (sizeof(SIG_ATTENDANCE_EVENT) == 0x01) ? 1 : -1];
typedef char SIG_BINGO_EVENT_Size_Must_Be_0x08[
    (sizeof(SIG_BINGO_EVENT) == 0x08) ? 1 : -1];
typedef char SIG_BREAK_AWAY_PREVENT_REWARD_MANAGER_Size_Must_Be_0x18[
    (sizeof(SIG_BREAK_AWAY_PREVENT_REWARD_MANAGER) == 0x18) ? 1 : -1];
typedef char SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER_Size_Must_Be_0x2e4[
    (sizeof(SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER) == 0x2e4) ? 1 : -1];
typedef char SIG_EVENT_STAGE_LOG_Size_Must_Be_0x78[
    (sizeof(SIG_EVENT_STAGE_LOG) == 0x78) ? 1 : -1];
typedef char Taiwan_SigStayTimeEvent_Size_Must_Be_0x04[
    (sizeof(Taiwan::SigStayTimeEvent) == 0x04) ? 1 : -1];

// IDA_EA: 0x8260386
// IDA_NAME: _ZN14MSG_BINGO_QUIZC2Ev
// DEMANGLED: MSG_BINGO_QUIZ::MSG_BINGO_QUIZ(void)
MSG_BINGO_QUIZ::MSG_BINGO_QUIZ()
    : MSG_BASE(), quiz_value(0) {}

// IDA_EA: 0x822DF4E
// IDA_NAME: _ZN22MSG_FATIGUE_ATTENDANCEC2Ev
// DEMANGLED: MSG_FATIGUE_ATTENDANCE::MSG_FATIGUE_ATTENDANCE(void)
MSG_FATIGUE_ATTENDANCE::MSG_FATIGUE_ATTENDANCE()
    : MSG_BASE(), request_value(0) {}

// IDA_EA: 0x822E374
// IDA_NAME: _ZN15MSG_BLUE_MARBLEC2Ev
// DEMANGLED: MSG_BLUE_MARBLE::MSG_BLUE_MARBLE(void)
MSG_BLUE_MARBLE::MSG_BLUE_MARBLE()
    : MSG_BASE(), action(0) {}

// IDA_EA: 0x822E38E
// IDA_NAME: _ZN28MSG_BLUE_MARBLE_CONFIRM_INFOC2Ev
// DEMANGLED: MSG_BLUE_MARBLE_CONFIRM_INFO::MSG_BLUE_MARBLE_CONFIRM_INFO(void)
MSG_BLUE_MARBLE_CONFIRM_INFO::MSG_BLUE_MARBLE_CONFIRM_INFO()
    : MSG_BASE(), user_absent(0), blue_marble_index(-1) {}

// IDA_EA: 0x822E0AA
// IDA_NAME: _ZN25MsgTournamentRewardSelectC2Ev
// DEMANGLED: MsgTournamentRewardSelect::MsgTournamentRewardSelect(void)
MsgTournamentRewardSelect::MsgTournamentRewardSelect()
    : MSG_BASE(), card_group(0xffu), reward_kind(0xffu) {}

// IDA_EA: 0x824CBA6
// IDA_NAME: _ZN31MSG_EXCHANGE_RANDOM_ITEM_REWARDC2Ev
// DEMANGLED: MSG_EXCHANGE_RANDOM_ITEM_REWARD::MSG_EXCHANGE_RANDOM_ITEM_REWARD(void)
MSG_EXCHANGE_RANDOM_ITEM_REWARD::MSG_EXCHANGE_RANDOM_ITEM_REWARD()
    : MSG_BASE(),
      reserved_0d(0),
      event_id_0e(0),
      item_indices_10() {}

// IDA_EA: 0x824CBC8
// IDA_NAME: _ZN31MSG_EXCHANGE_RANDOM_ITEM_REWARDD2Ev
// DEMANGLED: MSG_EXCHANGE_RANDOM_ITEM_REWARD::~MSG_EXCHANGE_RANDOM_ITEM_REWARD()
MSG_EXCHANGE_RANDOM_ITEM_REWARD::~MSG_EXCHANGE_RANDOM_ITEM_REWARD() {}

// The request object is constructed by the CMD450 dispatcher template.  Its
// public source export was not split out by the IDA unit catalog; the +0x0e
// and +0x10 fields are instead proven by Dispatcher_Avatar_Disjoint_...
// process @0x81DEA06.
MSG_AVATAR_DISJOINT_RANDOM_REWARD::MSG_AVATAR_DISJOINT_RANDOM_REWARD()
    : MSG_BASE(),
      reserved_0d(0),
      avatar_slot_0e(0),
      avatar_item_id_10(0) {}

// IDA_EA: 0x82619C0
// IDA_NAME: _ZN11stBingoQuizC2Ev
// DEMANGLED: stBingoQuiz::stBingoQuiz(void)
stBingoQuiz::stBingoQuiz()
    : ParamBase() {}

// IDA_EA: 0x82619DC
// IDA_NAME: _ZN11stBingoQuizD2Ev
// DEMANGLED: stBingoQuiz::~stBingoQuiz()
// IDA_EA: 0x8261A0C
// IDA_NAME: _ZN11stBingoQuizD0Ev
// DEMANGLED: stBingoQuiz::~stBingoQuiz() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
stBingoQuiz::~stBingoQuiz() {}

// IDA_EA: 0x8261C24
// IDA_NAME: _ZN13stBingoRewardC2Ev
// DEMANGLED: stBingoReward::stBingoReward(void)
stBingoReward::stBingoReward()
    : ParamBase() {}

// IDA_EA: 0x8261C40
// IDA_NAME: _ZN13stBingoRewardD2Ev
// DEMANGLED: stBingoReward::~stBingoReward()
// IDA_EA: 0x8261C70
// IDA_NAME: _ZN13stBingoRewardD0Ev
// DEMANGLED: stBingoReward::~stBingoReward() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
stBingoReward::~stBingoReward() {}

// IDA_EA: 0x823191E
// IDA_NAME: _ZN26stExchangeRandomItemRewardC2Ev
// DEMANGLED: stExchangeRandomItemReward::stExchangeRandomItemReward(void)
stExchangeRandomItemReward::stExchangeRandomItemReward()
    : ParamBase(),
      dword_04(0),
      byte_08(0),
      byte_09(0),
      reward_pairs_0c(),
      item_indices_18() {}

// IDA_EA: 0x8246A9E
// IDA_NAME: _ZN26stExchangeRandomItemRewardD2Ev
// DEMANGLED: stExchangeRandomItemReward::~stExchangeRandomItemReward()
// IDA_EA: 0x8246B28
// IDA_NAME: _ZN26stExchangeRandomItemRewardD0Ev
// DEMANGLED: stExchangeRandomItemReward::~stExchangeRandomItemReward() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
stExchangeRandomItemReward::~stExchangeRandomItemReward() {}

// IDA_EA: 0x82319AA
// IDA_NAME: _ZN28stAvatarDisjointRandomRewardC2Ev
// DEMANGLED: stAvatarDisjointRandomReward::stAvatarDisjointRandomReward(void)
stAvatarDisjointRandomReward::stAvatarDisjointRandomReward()
    : ParamBase(),
      error_code(0),
      avatar_slot(0),
      reserved_0a(),
      reward_pairs_0c() {}

// IDA_EA: 0x823516E
// IDA_NAME: _ZN28stAvatarDisjointRandomRewardC1ERKS_
// DEMANGLED: stAvatarDisjointRandomReward::stAvatarDisjointRandomReward(stAvatarDisjointRandomReward const&)
stAvatarDisjointRandomReward::stAvatarDisjointRandomReward(
    const stAvatarDisjointRandomReward& rhs)
    : ParamBase(),
      error_code(rhs.error_code),
      avatar_slot(rhs.avatar_slot),
      reserved_0a(),
      reward_pairs_0c(rhs.reward_pairs_0c) {
    reserved_0a[0] = rhs.reserved_0a[0];
    reserved_0a[1] = rhs.reserved_0a[1];
}

// IDA_EA: 0x82351E6
// IDA_NAME: _ZN28stAvatarDisjointRandomRewardD2Ev
// DEMANGLED: stAvatarDisjointRandomReward::~stAvatarDisjointRandomReward()
// IDA_EA: 0x8235248
// IDA_NAME: _ZN28stAvatarDisjointRandomRewardD0Ev
// DEMANGLED: stAvatarDisjointRandomReward::~stAvatarDisjointRandomReward() deleting destructor
stAvatarDisjointRandomReward::~stAvatarDisjointRandomReward() {}

// IDA_EA: 0x824E71C
// IDA_NAME: _ZN19stFatigueAttendanceC2Ev
// DEMANGLED: stFatigueAttendance::stFatigueAttendance(void)
stFatigueAttendance::stFatigueAttendance()
    : ParamBase() {}

// IDA_EA: 0x824E738
// IDA_NAME: _ZN19stFatigueAttendanceD2Ev
// DEMANGLED: stFatigueAttendance::~stFatigueAttendance()
// IDA_EA: 0x824E768
// IDA_NAME: _ZN19stFatigueAttendanceD0Ev
// DEMANGLED: stFatigueAttendance::~stFatigueAttendance() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
stFatigueAttendance::~stFatigueAttendance() {}

// IDA_EA: 0x8231D98
// IDA_NAME: _ZN12stBlueMarbleC2Ev
// DEMANGLED: stBlueMarble::stBlueMarble(void)
stBlueMarble::stBlueMarble()
    : ParamBase(),
      dword_04(0) {}

// IDA_EA: 0x8246612
// IDA_NAME: _ZN12stBlueMarbleD2Ev
// DEMANGLED: stBlueMarble::~stBlueMarble()
// IDA_EA: 0x8246642
// IDA_NAME: _ZN12stBlueMarbleD0Ev
// DEMANGLED: stBlueMarble::~stBlueMarble() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
stBlueMarble::~stBlueMarble() {}

// IDA_EA: 0x8231DBE
// IDA_NAME: _ZN23stBlueMarbleConfirmInfoC2Ev
// DEMANGLED: stBlueMarbleConfirmInfo::stBlueMarbleConfirmInfo(void)
stBlueMarbleConfirmInfo::stBlueMarbleConfirmInfo()
    : ParamBase(),
      dword_04(0) {}

// IDA_EA: 0x82465C4
// IDA_NAME: _ZN23stBlueMarbleConfirmInfoD2Ev
// DEMANGLED: stBlueMarbleConfirmInfo::~stBlueMarbleConfirmInfo()
// IDA_EA: 0x82465F4
// IDA_NAME: _ZN23stBlueMarbleConfirmInfoD0Ev
// DEMANGLED: stBlueMarbleConfirmInfo::~stBlueMarbleConfirmInfo() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
stBlueMarbleConfirmInfo::~stBlueMarbleConfirmInfo() {}

// IDA_EA: 0x824BE96
// IDA_NAME: _ZN24stTournamentRewardSelectC2Ev
// DEMANGLED: stTournamentRewardSelect::stTournamentRewardSelect(void)
stTournamentRewardSelect::stTournamentRewardSelect()
    : ParamBase() {}

// IDA_EA: 0x824BEB2
// IDA_NAME: _ZN24stTournamentRewardSelectD2Ev
// DEMANGLED: stTournamentRewardSelect::~stTournamentRewardSelect()
// IDA_EA: 0x824BEE2
// IDA_NAME: _ZN24stTournamentRewardSelectD0Ev
// DEMANGLED: stTournamentRewardSelect::~stTournamentRewardSelect() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
stTournamentRewardSelect::~stTournamentRewardSelect() {}

// IDA_EA: 0x824C0F8
// IDA_NAME: _ZN29stTournamentRewardSelectStateC2Ev
// DEMANGLED: stTournamentRewardSelectState::stTournamentRewardSelectState(void)
stTournamentRewardSelectState::stTournamentRewardSelectState()
    : ParamBase() {}

// IDA_EA: 0x824C114
// IDA_NAME: _ZN29stTournamentRewardSelectStateD2Ev
// DEMANGLED: stTournamentRewardSelectState::~stTournamentRewardSelectState()
// IDA_EA: 0x824C144
// IDA_NAME: _ZN29stTournamentRewardSelectStateD0Ev
// DEMANGLED: stTournamentRewardSelectState::~stTournamentRewardSelectState() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
stTournamentRewardSelectState::~stTournamentRewardSelectState() {}

// IDA_EA: 0x848FB84
// IDA_NAME: _ZN20SIG_ATTENDANCE_EVENTC2Ev
// DEMANGLED: SIG_ATTENDANCE_EVENT::SIG_ATTENDANCE_EVENT(void)
SIG_ATTENDANCE_EVENT::SIG_ATTENDANCE_EVENT() {
    can_attend = 0;
}

// IDA_EA: 0x844D600
// IDA_NAME: _ZN15SIG_BINGO_EVENT5clearEv
// DEMANGLED: SIG_BINGO_EVENT::clear(void)
SIG_BINGO_EVENT* SIG_BINGO_EVENT::clear() {
    raw[0] = 0;
    *reinterpret_cast<uint32_t*>(raw + 4) = 0;
    return this;
}

// IDA_EA: 0x831D264
// IDA_NAME: _ZN37SIG_BREAK_AWAY_PREVENT_REWARD_MANAGERC2Ev
// DEMANGLED: SIG_BREAK_AWAY_PREVENT_REWARD_MANAGER::SIG_BREAK_AWAY_PREVENT_REWARD_MANAGER(void)
SIG_BREAK_AWAY_PREVENT_REWARD_MANAGER::
    SIG_BREAK_AWAY_PREVENT_REWARD_MANAGER() {
    quest_id = 0;
    min_day = 0;
    max_day = 0;
    reserved_03 = 0;
    min_value = 0;
    max_value = 0;
    item_id = 0;
    add_info = 0;
    luck_point = 0;
}

// IDA_EA: 0x831D2B0
// IDA_NAME: _ZN37SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGERC2Ev
// DEMANGLED: SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER::SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER(void)
SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER::
    SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER() {
    weekday_var_a = 0;
    weekday_var_b = 0;
    weekday_var_c = 0;
    weekend_var_x = 0;
    weekend_var_y = 0;
    weekend_var_z = 0;
    next_reward_day = 0;
    state_flag = 0;
    reserved_0f = 0;
    reward_count = 0;
}

// IDA_EA: 0x81BA886
// IDA_NAME: _ZN19SIG_EVENT_STAGE_LOGC2Ev
// DEMANGLED: SIG_EVENT_STAGE_LOG::SIG_EVENT_STAGE_LOG(void)
SIG_EVENT_STAGE_LOG::SIG_EVENT_STAGE_LOG() {
    event_id = 0;
    account_id = 0;
    charac_no = 0;
    server_info = 0;
    stage = 0;
    std::memset(stage_result, 0, sizeof(stage_result));
}

namespace Taiwan {

// IDA_EA: 0x863BD76
// IDA_NAME: _ZN6Taiwan16SigStayTimeEvent4initEv
// DEMANGLED: Taiwan::SigStayTimeEvent::init(void)
SigStayTimeEvent* SigStayTimeEvent::init() {
    account_id = 0;
    return this;
}

// IDA_EA: 0x863BD84
// IDA_NAME: _ZN6Taiwan16SigStayTimeEvent3setEj
// DEMANGLED: Taiwan::SigStayTimeEvent::set(unsigned int)
SigStayTimeEvent* SigStayTimeEvent::set(unsigned int account_id_value) {
    account_id = account_id_value;
    return this;
}

}  // namespace Taiwan
