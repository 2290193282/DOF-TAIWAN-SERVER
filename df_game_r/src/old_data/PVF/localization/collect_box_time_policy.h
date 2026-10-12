#ifndef OLDSRV_OLD_DATA_PVF_LOCALIZATION_COLLECT_BOX_TIME_POLICY_H_
#define OLDSRV_OLD_DATA_PVF_LOCALIZATION_COLLECT_BOX_TIME_POLICY_H_

#include <stdint.h>

namespace collect_box_time {

struct Window {
    Window() : base(0u), maximum(0u) {}
    Window(uint32_t base_value, uint32_t maximum_value)
        : base(base_value), maximum(maximum_value) {}

    uint32_t base;
    uint32_t maximum;
};

// Both timestamps are absolute Unix seconds from collectboxinfo.etc.  A
// durable expiry is accepted only inside that exact [base, maximum] interval;
// historical windows are never shifted or rolled to another calendar year.
uint32_t RemainingSeconds(const Window& configured,
                          uint32_t configured_expiry,
                          uint32_t now);

}  // namespace collect_box_time

#endif  // OLDSRV_OLD_DATA_PVF_LOCALIZATION_COLLECT_BOX_TIME_POLICY_H_
