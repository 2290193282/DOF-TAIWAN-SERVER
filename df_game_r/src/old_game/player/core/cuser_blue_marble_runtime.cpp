#include "old_game/player/core/cuser.h"

#include <stdint.h>

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/activity/schedule/blue_marble_runtime.h"


// IDA_EA: 0x86552E6
// IDA_NAME: _ZN5CUser13getBlueMarbleEv
// DEMANGLED: CUser::getBlueMarble(void)
BlueMarble* CUser::getBlueMarble() {
    // The old object slot is a signed short.  The public accessor preserves
    // the raw value in an int, so restore the original signed sentinel before
    // querying the canonical manager lookup.
    const int16_t blue_marble_index =
        static_cast<int16_t>(getBlueMarbleIndex());
    if (blue_marble_index < 0) {
        return 0;
    }
    return G_CGameManager()->getBlueMarble(
        static_cast<int>(blue_marble_index));
}
