#include "old_data/db/inventory/load_inventory_payload.h"


// IDA_EA: 0x844D406
// IDA_NAME: _ZN14stItemLockInfoC2Ev
// DEMANGLED: stItemLockInfo::stItemLockInfo(void)
stItemLockInfo::stItemLockInfo()
    : lock_time(0),
      lock_type(0),
      lock_state(0),
      lock_reason(0),
      lock_owner(0) {}

// IDA_EA: 0x848F976
// IDA_NAME: _ZN31SIG_POSTAL_CREATURE_TEMP_PERIODC2Ev
// DEMANGLED: SIG_POSTAL_CREATURE_TEMP_PERIOD::SIG_POSTAL_CREATURE_TEMP_PERIOD(void)
SIG_POSTAL_CREATURE_TEMP_PERIOD::SIG_POSTAL_CREATURE_TEMP_PERIOD()
    : creature_id(-1), expiration_time(-1) {}

// IDA_EA: 0x848F9D0
// IDA_NAME: _ZN28SIG_SAVE_DIMENSION_RENT_INFOC2Ev
// DEMANGLED: SIG_SAVE_DIMENSION_RENT_INFO::SIG_SAVE_DIMENSION_RENT_INFO(void)
SIG_SAVE_DIMENSION_RENT_INFO::SIG_SAVE_DIMENSION_RENT_INFO() {
    clear();
}

// IDA_EA: 0x848F9E4
// IDA_NAME: _ZN28SIG_SAVE_DIMENSION_RENT_INFO5clearEv
// DEMANGLED: SIG_SAVE_DIMENSION_RENT_INFO::clear(void)
SIG_SAVE_DIMENSION_RENT_INFO* SIG_SAVE_DIMENSION_RENT_INFO::clear() {
    item_id = 0;
    rental_type = 0;
    inventory_slot = 0;
    equipped = 0;
    rental_state = 0;
    return this;
}

// IDA_EA: 0x8491CF2
// IDA_NAME: _ZN28SIG_SAVE_DIMENSION_RENT_ITEMC2Ev
// DEMANGLED: SIG_SAVE_DIMENSION_RENT_ITEM::SIG_SAVE_DIMENSION_RENT_ITEM(void)
SIG_SAVE_DIMENSION_RENT_ITEM::SIG_SAVE_DIMENSION_RENT_ITEM() {}

// IDA_EA: 0x8694CD2
// IDA_NAME: _ZN28SIG_SAVE_DIMENSION_RENT_ITEM5clearEv
// DEMANGLED: SIG_SAVE_DIMENSION_RENT_ITEM::clear(void)
bool SIG_SAVE_DIMENSION_RENT_ITEM::clear() {
    bool result = false;
    for (int index = 0;; ++index) {
        result = index <= 10;
        if (index > 10) {
            break;
        }
        items[index].clear();
    }
    return result;
}

// IDA_EA: 0x8491D26
// IDA_NAME: _ZN18SIG_LOAD_INVENTORYC2Ev
// DEMANGLED: SIG_LOAD_INVENTORY::SIG_LOAD_INVENTORY(void)
SIG_LOAD_INVENTORY::SIG_LOAD_INVENTORY() {}
