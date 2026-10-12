#include "old_game/player/core/cuser.h"

#include "old_game/economy/inventory/item_space_runtime.h"

// IDA_EA: 0x866285C
// IDA_NAME: _ZN5CUser11delete_itemEcimi14eItemDelReason
// DEMANGLED: CUser::delete_item(char,int,unsigned long,int,eItemDelReason)
//
// This is the CUser-owned mutation gateway used by legacy packet handlers.
// It verifies the requested item id in the source item-space, applies the
// correct inventory/cargo owner mutation, and carries the original equipment
// refresh side effect.  CMD427 relies on item-space 0 through this member.
int CUser::delete_item(char item_space,
                       int slot,
                       unsigned long item_id,
                       int count,
                       eItemDelReason reason) {
    if (CheckInTrade()) return 19;

    const int inventory_type = GetInvenTypeFromItemSpace(
        static_cast<ENUM_ITEMSPACE>(item_space));
    const Inven_Item requested = getCurCharacInvenR()->GetInvenSlot(
        inventory_type, slot);
    if (requested.item_id_ != static_cast<int>(item_id)) return 17;

    switch (item_space) {
        case 0:
            return getCurCharacInvenW()->delete_item(
                       static_cast<INVEN_TYPE>(1), slot, count, reason, true) == 1
                       ? 0
                       : 17;
        case 1:
            return getCurCharacInvenW()->delete_item(
                       static_cast<INVEN_TYPE>(2), slot, count, reason, true) == 1
                       ? 0
                       : 17;
        case 2:
            return getCurCharacCargoW()->delete_item(slot, count, reason) == 1
                       ? 0
                       : 17;
        case 3:
            if (getCurCharacInvenW()->delete_item(
                    static_cast<INVEN_TYPE>(0), slot, count, reason, true) != 1) {
                return 17;
            }
            send_equip(slot);
            return 0;
        case 7: {
            const Inven_Item* const creature =
                getCurCharacInvenW()->GetInvenRef(3, slot);
            if (!creature || creature->item_category_ == 5) return 17;
            return getCurCharacInvenW()->delete_item(
                       static_cast<INVEN_TYPE>(3), slot, count, reason, true) == 1
                       ? 0
                       : 17;
        }
        default:
            // The original switch reaches its shared success epilogue for
            // unhandled item spaces after the id comparison above.
            return 0;
    }
}
