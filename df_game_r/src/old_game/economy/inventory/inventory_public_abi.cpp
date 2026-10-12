#include "old_game/economy/inventory/inventory_public_abi.h"

// IDA_NAME: g_emptySlot
// DEMANGLED: g_emptySlot (Inven_Item)
// Preserve the old process-global object lifetime; callers must not substitute
// a function-local empty record for this shared inventory slot.
Inven_Item g_emptySlot;
