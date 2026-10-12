#include "old_game/player/core/cuser.h"

// IDA_EA: consumed by DisPatcher_GetItem::process @ 0x81C35C0.
// GetItemTimeChecker owns the two signed pickup-span slots.  Keep the
// threshold query with CUser so packet code never depends on its layout.
bool CUser::IsGetItemDistanceOver(short threshold) const {
    return item_time_check_.distance_x > threshold ||
           item_time_check_.distance_y > threshold;
}
