#ifndef OLDSRV_APSYSTEM_ACTION_POINT_ETC_PARAMETER_H_
#define OLDSRV_APSYSTEM_ACTION_POINT_ETC_PARAMETER_H_

#include <stdint.h>

#include <map>
#include <string>
#include <vector>

#include "old_game/activity/schedule/event_value_types.h"
#include "old_data/db/legacy_dispatch/action_point_payload.h"

namespace APSystem {

// _ActionPoint, _ActionGroupIndex, and _SIG_LOAD_ACTION_POINT are DB payload
// ABI owned by db/action_point_payload.h.

struct CActionPointEx : public _ActionPoint {
    uint16_t rule_value;          // IDA +0x0C: script integer.
    uint8_t group_option;         // IDA +0x0E: group option byte.
    uint8_t start_value;          // IDA +0x0F: lower range byte.
    uint8_t end_value;            // IDA +0x10: upper range byte.
    std::vector<int> values;      // IDA +0x14: comma-separated integer list.
    uint32_t weekday;             // IDA +0x20: 0..6 or 7 for all/default.
    std::string user_message;     // IDA +0x24: user-facing message text.
    std::string system_message;   // IDA +0x28: system-facing message text.
    uint8_t display_flag;         // IDA +0x2C: script display flag.

    CActionPointEx();
    ~CActionPointEx();
    CActionPointEx(const CActionPointEx& other);
    CActionPointEx& operator=(const CActionPointEx& other);
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CActionPointEx) == 0x30u,
              "APSystem::CActionPointEx IDA ABI size must stay 0x30");
#endif

struct __attribute__((packed)) _MedalRewardItem {
    uint16_t medal_type;
    uint16_t grade;
    uint32_t item_id;
    uint16_t item_count;

    _MedalRewardItem();
    ~_MedalRewardItem();
};

static_assert(sizeof(_MedalRewardItem) == 0x0Au,
              "APSystem::_MedalRewardItem IDA ABI size must stay 0x0A");

typedef _TodayRewardItem TodayRewardItem;

static_assert(sizeof(_TodayRewardItem) == 0x0Eu,
              "APSystem::_TodayRewardItem IDA ABI size must stay 0x0E");

class CActionPointEtcParameter {
public:
    CActionPointEtcParameter();
    ~CActionPointEtcParameter();

    void ClearData();
    static bool IsValidActionGroupIndex(_ActionGroupIndex index);
    static bool IsValidTodayRewardItem(
        _SIG_LOAD_ACTION_POINT::_TodayRewardItem item);
    static int importActionPointSystemEtcScript(
        const char* filename,
        CActionPointEtcParameter& parameter);

    const std::map<uint16_t, std::vector<CActionPointEx> >& action_groups()
        const;
    const std::map<uint16_t, const CActionPointEx*>& action_index() const;
    const std::map<uint32_t, std::string>& user_messages() const;
    const std::map<uint32_t, std::string>& system_messages() const;
    const std::vector<_MedalRewardItem>& medal_rewards() const;
    const std::vector<TodayRewardItem>& today_rewards() const;

private:
    // IDA member order:
    // +0x00 action group map, +0x18 direct action index map,
    // +0x30 user messages, +0x48 system messages,
    // +0x60 medal reward vector, +0x6C today reward vector.
    std::map<uint16_t, std::vector<CActionPointEx> > action_groups_;
    std::map<uint16_t, const CActionPointEx*> action_index_;
    std::map<uint32_t, std::string> user_messages_;
    std::map<uint32_t, std::string> system_messages_;
    std::vector<_MedalRewardItem> medal_rewards_;
    std::vector<_TodayRewardItem> today_rewards_;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CActionPointEtcParameter) == 0x78u,
              "APSystem::CActionPointEtcParameter IDA ABI size must stay 0x78");
#endif

}  // namespace APSystem

#endif  // OLDSRV_APSYSTEM_ACTION_POINT_ETC_PARAMETER_H_
