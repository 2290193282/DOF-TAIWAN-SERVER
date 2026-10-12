#include "old_game/world/region/game_world.h"

#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/system_time.h"

#include <stdint.h>
#include <time.h>

// IDA_EA: 0x86D12B2
// IDA_NAME: _ZN9GameWorld20SetWeekendBounsEventEv
// DEMANGLED: GameWorld::SetWeekendBounsEvent(void)
int GameWorld::SetWeekendBounsEvent() {
    time_t timer = static_cast<time_t>(GlobalData::s_systemTime_.getCurSec());
    struct tm local_time;
    localtime_r(&timer, &local_time);

    if ((local_time.tm_wday == 6 && local_time.tm_hour > 5) ||
        local_time.tm_wday == 0 ||
        (local_time.tm_wday == 1 && local_time.tm_hour <= 5)) {
        CEventBase* const repeat_event =
            GlobalData::s_event_manager->GetRepeatEvent(87);
        const uint8_t is_eventing =
            static_cast<uint8_t>(repeat_event->IsEventing(0));
        if (static_cast<uint8_t>(is_eventing ^ 1u) != 0u) {
            return GlobalData::s_event_manager->TriggerEventStart(87,
                                                                   Word_Param(0));
        }
        return 0;
    }

    if (local_time.tm_wday == 1) {
        if (local_time.tm_hour > 5) {
            CEventBase* const repeat_event =
                GlobalData::s_event_manager->GetRepeatEvent(87);
            if (static_cast<uint8_t>(repeat_event->IsEventing(0)) != 0u) {
                return GlobalData::s_event_manager->TriggerEventEnd(87);
            }
            return 0;
        }
        return local_time.tm_hour;
    }

    return local_time.tm_wday;
}
