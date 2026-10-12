#ifndef OLDSRV_DB_ACTION_POINT_PAYLOAD_H_
#define OLDSRV_DB_ACTION_POINT_PAYLOAD_H_

#include <stdint.h>

namespace APSystem {

// The original members preserve this as a named C++ type in their mangled
// names (`NS_17_ActionGroupIndexE`).  A typedef would decay to `unsigned int`
// in a symbol, so retain the four-byte old ABI as a fixed-width enum.
enum _ActionGroupIndex : uint32_t {};

struct __attribute__((packed)) _ActionPoint {
    uint16_t action_index;
    _ActionGroupIndex action_group_index;
    uint8_t clear_state[5];

    _ActionPoint();
};

static_assert(sizeof(_ActionPoint) == 0x0Bu,
              "APSystem::_ActionPoint IDA ABI size must stay 0x0B");

// IDA_EA: 0x848F5D8.
// IDA_NAME: _ZN8APSystem22_SIG_LOAD_ACTION_POINTC2Ev
// DEMANGLED: APSystem::_SIG_LOAD_ACTION_POINT::_SIG_LOAD_ACTION_POINT(void)
struct __attribute__((packed)) _SIG_LOAD_ACTION_POINT {
    enum { kActionPointCount = 300 };

    // IDA_NAME:
    // _ZN8APSystem24CActionPointEtcParameter22IsValidTodayRewardItemENS_22_SIG_LOAD_ACTION_POINT16_TodayRewardItemE
    // This is the four-byte selection enum used by the parameter validator;
    // APSystem::_TodayRewardItem is the distinct 0x0E script-row record.
    enum _TodayRewardItem {
        TODAY_REWARD_ITEM_0 = 0,
        TODAY_REWARD_ITEM_1 = 1,
        TODAY_REWARD_ITEM_2 = 2,
        TODAY_REWARD_ITEM_3 = 3,
    };

    uint32_t charac_no;
    uint32_t ap_sum;
    uint8_t reward_medal;
    uint8_t reward_item_1;
    uint8_t reward_item_2;
    uint8_t reward_item_3;
    uint8_t reward_item_4;
    _ActionPoint action_points[kActionPointCount];

    _SIG_LOAD_ACTION_POINT();
};

static_assert(sizeof(_SIG_LOAD_ACTION_POINT) == 0xCF1u,
              "APSystem::_SIG_LOAD_ACTION_POINT IDA ABI size must stay 0xCF1");
static_assert(sizeof(_SIG_LOAD_ACTION_POINT::_TodayRewardItem) == 0x04u,
              "ActionPoint today-reward selector must stay four bytes");

}  // namespace APSystem

#endif  // OLDSRV_DB_ACTION_POINT_PAYLOAD_H_
