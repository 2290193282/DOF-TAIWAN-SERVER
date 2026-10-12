#ifndef OLDSRV_DATAMANAGER_ARAD_EVENT_REWARD_H_
#define OLDSRV_DATAMANAGER_ARAD_EVENT_REWARD_H_

#include "old_data/PVF/common/arad_script_data.h"
#include "old_data/PVF/event/datamanager/repeat_event_types.h"
#include "old_data/db/event/reward_event_request.h"

#include <stdint.h>

namespace ARAD {

// Event-period selector used only with B-owned parsed ARAD script rows.
class SearchEventPeriod {
public:
    explicit SearchEventPeriod(ENUM_REPEAT_EVENT_CODE event_code_value);

    bool operator()(const SCRIPT::SEventPeriodData& data) const;
    bool operator()(const SCRIPT::SRewardPeriodData& data) const;

private:
    ENUM_REPEAT_EVENT_CODE event_code;
};

// Per-account/character reward-de-duplication key. It is data-manager state,
// not a C event-state-machine record.
struct EventRewardItemArad {
    EventRewardItemArad(int event_code_value,
                        uint32_t account_id_value,
                        uint32_t charac_no_value);

    bool operator==(const EventRewardItemArad& rhs) const;

    int32_t event_code;
    uint32_t account_id;
    uint32_t charac_no;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(SearchEventPeriod) == 0x04u,
              "SearchEventPeriod IDA ABI size must stay 0x04");
static_assert(sizeof(EventRewardItemArad) == 0x0cu,
              "EventRewardItemArad IDA ABI size must stay 0x0c");
#endif

}  // namespace ARAD

#endif  // OLDSRV_DATAMANAGER_ARAD_EVENT_REWARD_H_
