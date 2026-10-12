#include "old_game/economy/inventory/inventory.h"

// IDA_EA: 0x811ED98
// IDA_NAME: _ZNK10CEquipItem13get_enduranceEv
// DEMANGLED: CEquipItem::get_endurance(void)const
int CEquipItem::get_endurance() const {
    return maximum_endurance_;
}
