#include "old_data/db/core/db_runtime_payloads.h"

namespace private_store {

// IDA_EA: 0x823457C
// IDA_NAME: _ZN13private_store16PrivateStoreItemC2Ev
// DEMANGLED: private_store::PrivateStoreItem::PrivateStoreItem(void)
PrivateStoreItem::PrivateStoreItem() : item() {
    Reset();
}

// IDA_EA: 0x823459A
// IDA_NAME: _ZN13private_store16PrivateStoreItem5ResetEv
// DEMANGLED: private_store::PrivateStoreItem::Reset(void)
PrivateStoreItem* PrivateStoreItem::Reset() {
    item.reset();
    state = 0;
    price = 0;
    store_slot = 0;
    inventory_space = 0;
    inventory_slot = 0;
    return this;
}

// IDA_EA: 0x85CB662
// IDA_NAME: _ZN13private_store16PrivateStoreItem8GetStateEv
int PrivateStoreItem::GetState() {
    return state;
}

// IDA_EA: 0x85CB66E
// IDA_NAME: _ZN13private_store16PrivateStoreItemaSERKS0_
PrivateStoreItem* PrivateStoreItem::operator=(const PrivateStoreItem& rhs) {
    item.setCopy(rhs.item);
    state = rhs.state;
    price = rhs.price;
    store_slot = rhs.store_slot;
    inventory_space = rhs.inventory_space;
    inventory_slot = rhs.inventory_slot;
    return this;
}

}  // namespace private_store
