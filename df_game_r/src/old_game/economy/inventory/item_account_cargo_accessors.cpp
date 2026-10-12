#include "old_game/economy/inventory/item.h"

// IDA_EA: 0x828B5B4
// IDA_NAME: _ZNK5CItem12isPackagableEv
// DEMANGLED: CItem::isPackagable(void)const
bool CItem::isPackagable() const {
    return packagable_ == 1;
}
