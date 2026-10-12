#include "old_data/PVF/localization/collect_box_time_policy.h"

namespace collect_box_time {

uint32_t RemainingSeconds(const Window& configured,
                          uint32_t configured_expiry,
                          uint32_t now) {
    if (configured.base == 0u || configured.maximum <= configured.base ||
        configured_expiry < configured.base ||
        configured_expiry > configured.maximum ||
        configured_expiry <= now) {
        return 0u;
    }
    return configured_expiry - now;
}

}  // namespace collect_box_time
