#include "old_data/PVF/data/data_manager.h"

#include "old_game/economy/inventory/item_list.h"

// IDA_EA: 0x835FA32
// IDA_NAME: _ZNK12CDataManager9find_itemEi
// DEMANGLED: CDataManager::find_item(int) const
//
// The old i386 body explicitly rejects only the sentinel IDs 0, 1, and 2,
// then loads the named CItemList pointer from CDataManager+0x0C.  Keeping the
// comparison unsigned preserves the original negative-ID behaviour too: a
// negative int is forwarded to CItemList::find_item rather than treated as a
// sentinel.
CItem* CDataManager::find_item(int item_id) const {
    if (static_cast<unsigned int>(item_id) <= 2u) {
        return 0;
    }
    return item_list_->find_item(item_id);
}
