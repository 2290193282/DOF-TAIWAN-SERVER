#include "old_game/world/region/stage_map_data.h"

namespace stagemap {

typedef char SlotType_Size_Must_Be_0x14[
    (sizeof(SlotType) == 0x14) ? 1 : -1];

// IDA_EA: 0x88AA6C6
// IDA_NAME: _ZN8stagemap8SlotType5clearEv
// DEMANGLED: stagemap::SlotType::clear(void)
SlotType* SlotType::clear() {
    slots[0] = -1;
    slots[1] = -1;
    slots[2] = -1;
    slots[3] = -1;
    slots[4] = -1;
    return this;
}

// IDA_EA: 0x88AA6FC
// IDA_NAME: _ZN8stagemap8SlotTypeC2Ev
// DEMANGLED: stagemap::SlotType::SlotType(void)
SlotType::SlotType() {
    clear();
}

}  // namespace stagemap
