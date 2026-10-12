#include "old_game/player/core/cuser.h"

#include "infra/log/format/trace.h"

namespace WongWork {

// IDA_EA: 0x80F8BE0
// IDA_NAME: _ZN8WongWork12CMCAPManager19setPointPerHackTypeEPi
// DEMANGLED: WongWork::CMCAPManager::setPointPerHackType(int *)
void CMCAPManager::setPointPerHackType(int* point_per_hack_type) {
    cMyTrace trace(
        "static void WongWork::CMCAPManager::setPointPerHackType(int*)",
        466, 0);
    trace("Load cleanpad point per hack type");

    // The DB producer materializes exactly the 101..4000 (3900 dwords)
    // table; copy all entries to the original static backing array.
    for (int index = 0; index < 3900; ++index) {
        pointPerHackType_[index] = point_per_hack_type[index];
    }
}

}  // namespace WongWork
