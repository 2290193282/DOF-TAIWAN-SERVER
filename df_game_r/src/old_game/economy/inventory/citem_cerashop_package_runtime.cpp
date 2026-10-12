#include "old_game/economy/inventory/item.h"

// IDA_EA: 0x8179FB4
// IDA_NAME: _ZNK5CItem21isCerashopPackageItemEv
// DEMANGLED: CItem::isCerashopPackageItem(void)const
//
// Keep the two virtual calls separate.  The original uses the vptr +0x0c
// item-type slot once for the type-14 short-circuit and once for type 15;
// folding that into a cached value would alter the observed virtual-call ABI.
bool CItem::isCerashopPackageItem() const {
    return vtable_->get_item_type(this) == 14 ||
           vtable_->get_item_type(this) == 15;
}
