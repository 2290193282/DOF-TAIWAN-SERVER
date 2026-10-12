#ifndef OLDSRV_11_EVENT_RUNTIME_EVENT_MESSAGES_H_
#define OLDSRV_11_EVENT_RUNTIME_EVENT_MESSAGES_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x8260386 in the companion source.


#include <stdint.h>

#include <utility>
#include <vector>

#include "infra/network/packet/message_base.h"


// IDA unit: 11_event_runtime/MSG_BINGO_QUIZ.
#pragma pack(push, 1)
struct MSG_BINGO_QUIZ : public MSG_BASE {
    MSG_BINGO_QUIZ();

    int32_t quiz_value;
};

// IDA unit: 11_event_runtime/MSG_FATIGUE_ATTENDANCE.
struct MSG_FATIGUE_ATTENDANCE : public MSG_BASE {
    MSG_FATIGUE_ATTENDANCE();

    uint16_t request_value;
};

// IDA unit: 11_event_runtime/MSG_BLUE_MARBLE.
struct MSG_BLUE_MARBLE : public MSG_BASE {
    MSG_BLUE_MARBLE();

    uint8_t action;
};

// IDA unit: 99_misc_global/MSG_BLUE_MARBLE_CONFIRM_INFO.
// C2 at 0x822E38E clears +0x0d and initializes the signed index at +0x0e
// to -1.  This request remains packed because MSG_BASE ends at +0x0d.
struct MSG_BLUE_MARBLE_CONFIRM_INFO : public MSG_BASE {
    MSG_BLUE_MARBLE_CONFIRM_INFO();

    uint8_t user_absent;         // +0x0d
    int16_t blue_marble_index;   // +0x0e
};

// IDA unit: 11_event_runtime/MsgTournamentRewardSelect.
struct MsgTournamentRewardSelect : public MSG_BASE {
    MsgTournamentRewardSelect();

    // IDA +0x0d / +0x0e, read separately by CMD459.
    uint8_t card_group;
    uint8_t reward_kind;
};
#pragma pack(pop)

// IDA unit: 11_event_runtime/MSG_EXCHANGE_RANDOM_ITEM_REWARD.
struct MSG_EXCHANGE_RANDOM_ITEM_REWARD : public MSG_BASE {
    MSG_EXCHANGE_RANDOM_ITEM_REWARD();
    ~MSG_EXCHANGE_RANDOM_ITEM_REWARD();

    uint8_t reserved_0d;         // +0x0d
    uint16_t event_id_0e;        // +0x0e, read by CMD449
    std::vector<int> item_indices_10;
};

// IDA unit: 06_userinfo_stat_inventory/MSG_AVATAR_DISJOINT_RANDOM_REWARD.
struct MSG_AVATAR_DISJOINT_RANDOM_REWARD : public MSG_BASE {
    MSG_AVATAR_DISJOINT_RANDOM_REWARD();

    uint8_t reserved_0d;         // +0x0d
    uint16_t avatar_slot_0e;     // +0x0e
    uint32_t avatar_item_id_10;  // +0x10
};

// IDA unit: 11_event_runtime/stBingoQuiz.
struct stBingoQuiz : public ParamBase {
    stBingoQuiz();
    virtual ~stBingoQuiz();

    int32_t error_code;
    int32_t bingo_data_dword;
};

// IDA unit: 11_event_runtime/stBingoReward.
struct stBingoReward : public ParamBase {
    stBingoReward();
    virtual ~stBingoReward();

    int32_t error_code;
    int32_t bingo_data_dword;
};

// IDA unit: 11_event_runtime/stExchangeRandomItemReward.
struct stExchangeRandomItemReward : public ParamBase {
    stExchangeRandomItemReward();
    virtual ~stExchangeRandomItemReward();

    uint32_t dword_04;
    uint8_t byte_08;
    uint8_t byte_09;
    uint8_t reserved_0a[2];
    std::vector<std::pair<int, int> > reward_pairs_0c;
    std::vector<int> item_indices_18;
};

// IDA unit: 06_userinfo_stat_inventory/stAvatarDisjointRandomReward.
struct stAvatarDisjointRandomReward : public ParamBase {
    stAvatarDisjointRandomReward();
    stAvatarDisjointRandomReward(const stAvatarDisjointRandomReward& rhs);
    virtual ~stAvatarDisjointRandomReward();

    int32_t error_code;
    uint16_t avatar_slot;
    uint8_t reserved_0a[2];
    std::vector<std::pair<int, int> > reward_pairs_0c;
};

// IDA unit: 11_event_runtime/stFatigueAttendance.
struct stFatigueAttendance : public ParamBase {
    stFatigueAttendance();
    virtual ~stFatigueAttendance();

    int32_t error_code;
    int32_t request_value;
    uint32_t history_bits;
    int32_t today_time;
};

// IDA unit: 11_event_runtime/stBlueMarble.
struct stBlueMarble : public ParamBase {
    stBlueMarble();
    virtual ~stBlueMarble();

    uint32_t dword_04;
};

// IDA unit: 11_event_runtime/stBlueMarbleConfirmInfo.
struct stBlueMarbleConfirmInfo : public ParamBase {
    stBlueMarbleConfirmInfo();
    virtual ~stBlueMarbleConfirmInfo();

    uint32_t dword_04;
};

#if defined(__i386__)
static_assert(sizeof(MSG_FATIGUE_ATTENDANCE) == 0x0fu,
              "CMD426 request must retain the native 15-byte ABI");
static_assert(sizeof(MSG_BLUE_MARBLE_CONFIRM_INFO) == 0x10u,
              "MSG_BLUE_MARBLE_CONFIRM_INFO IDA ABI size must stay 0x10");
static_assert(sizeof(MSG_EXCHANGE_RANDOM_ITEM_REWARD) == 0x1cu,
              "MSG_EXCHANGE_RANDOM_ITEM_REWARD IDA ABI size must stay 0x1c");
static_assert(sizeof(MSG_AVATAR_DISJOINT_RANDOM_REWARD) == 0x14u,
              "MSG_AVATAR_DISJOINT_RANDOM_REWARD IDA ABI size must stay 0x14");
static_assert(sizeof(stAvatarDisjointRandomReward) == 0x18u,
              "stAvatarDisjointRandomReward IDA ABI size must stay 0x18");
#endif

// IDA unit: 11_event_runtime/stTournamentRewardSelect.
struct stTournamentRewardSelect : public ParamBase {
    stTournamentRewardSelect();
    virtual ~stTournamentRewardSelect();

    int32_t error_code;
};

// IDA unit: 11_event_runtime/stTournamentRewardSelectState.
struct stTournamentRewardSelectState : public ParamBase {
    stTournamentRewardSelectState();
    virtual ~stTournamentRewardSelectState();

    int32_t error_code;
};

// IDA unit: 11_event_runtime/SIG_ATTENDANCE_EVENT.
struct SIG_ATTENDANCE_EVENT {
    SIG_ATTENDANCE_EVENT();

    // IDA 0x8448944 clears this byte and sets it when the account may attend.
    uint8_t can_attend;
};

// IDA unit: 11_event_runtime/SIG_BINGO_EVENT.
struct SIG_BINGO_EVENT {
    SIG_BINGO_EVENT* clear();

    uint8_t raw[0x08];
};

// IDA unit: 11_event_runtime/SIG_BREAK_AWAY_PREVENT_REWARD_MANAGER.
struct SIG_BREAK_AWAY_PREVENT_REWARD_MANAGER {
    SIG_BREAK_AWAY_PREVENT_REWARD_MANAGER();

    uint8_t quest_id;       // +0x00
    uint8_t min_day;        // +0x01
    uint8_t max_day;        // +0x02
    uint8_t reserved_03;    // +0x03
    int32_t min_value;      // +0x04
    int32_t max_value;      // +0x08
    uint32_t item_id;       // +0x0c
    int32_t add_info;       // +0x10
    int32_t luck_point;     // +0x14
};

// IDA unit: 11_event_runtime/SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER.
struct SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER {
    SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER();

    int16_t weekday_var_a;  // +0x00
    int16_t weekday_var_b;  // +0x02
    int16_t weekday_var_c;  // +0x04
    int16_t weekend_var_x;  // +0x06
    int16_t weekend_var_y;  // +0x08
    int16_t weekend_var_z;  // +0x0a
    int16_t next_reward_day;// +0x0c
    uint8_t state_flag;     // +0x0e
    uint8_t reserved_0f;    // +0x0f
    SIG_BREAK_AWAY_PREVENT_REWARD_MANAGER rewards_10[30];
    uint32_t reward_count;  // +0x2e0
};

// IDA unit: 11_event_runtime/SIG_EVENT_STAGE_LOG.
struct SIG_EVENT_STAGE_LOG {
    SIG_EVENT_STAGE_LOG();

    int32_t event_id;         // +0x00
    uint32_t account_id;      // +0x04
    int32_t charac_no;        // +0x08
    int32_t server_info;      // +0x0c
    int32_t stage;            // +0x10
    char stage_result[100];   // +0x14
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(SIG_EVENT_STAGE_LOG) == 0x78u,
              "SIG_EVENT_STAGE_LOG IDA size must stay 0x78");
#endif

namespace Taiwan {

// IDA unit: 11_event_runtime/Taiwan::SigStayTimeEvent.
struct SigStayTimeEvent {
    SigStayTimeEvent* init();
    SigStayTimeEvent* set(unsigned int account_id);

    uint32_t account_id;
};

}  // namespace Taiwan


#endif  // OLDSRV_11_EVENT_RUNTIME_EVENT_MESSAGES_H_
