#ifndef OLDSRV_OLD_GAME_INVENTORY_INVENTORY_LOAD_VERIFICATION_H_
#define OLDSRV_OLD_GAME_INVENTORY_INVENTORY_LOAD_VERIFICATION_H_

#include "old_data/db/inventory/load_inventory_payload.h"
#include "old_game/economy/inventory/item_lock_runtime.h"

#include <stdint.h>

#include <vector>

class CUser;

// IDA_EA: 0x84EFCDD.  The old export preserves this distinct int-backed
// domain in its mangled signature; only values 0..3 are observed by the
// verified inventory-load call chain, so no unobserved enumerators are named.
enum INVENTORY_TYPE : int {};

int _verifyItem(int slot,
                const Inven_Item* item,
                INVENTORY_TYPE inventory_type,
                bool reject_sealed_item,
                bool reject_stackable_item,
                CUser* user);
int _writeIncorrectItemInventoryForRecovery(uint32_t account_id,
                                            const Inven_Item& item,
                                            const char* inventory_name,
                                            int error_line,
                                            int slot);
int meterial_item_change(int slot,
                         SIG_LOAD_INVENTORY* payload,
                         Inven_Item* item);
void cago_meterial_item_change(int slot,
                               SIG_LOAD_INVENTORY* payload,
                               Inven_Item* item);
int _verifyInventory(SIG_LOAD_INVENTORY* payload,
                     std::vector<item_lock::stItemLockRef>& item_locks,
                     CUser* user);

#endif  // OLDSRV_OLD_GAME_INVENTORY_INVENTORY_LOAD_VERIFICATION_H_
