#include "old_game/economy/trade/trade_limit_item_abi.h"

#include "common/manager/data_manager.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"

CDataManager* G_CDataManager();

// IDA_EA: 0x812013C
// IDA_NAME: _Z23DecreaseTradeLimitCountR10Inven_Item
// DEMANGLED: DecreaseTradeLimitCount(Inven_Item &)
int DecreaseTradeLimitCount(Inven_Item& item) {
    const char count = item.GetTradeLimitCount();
    if (!count) return 0;
    item.SetTradeLimitCount(static_cast<uint8_t>(count - 1));
    return 1;
}

// IDA_EA: 0x81201DC
// IDA_NAME: _Z26IsTradeLimitAttachTypeItemRK10Inven_Item
// DEMANGLED: IsTradeLimitAttachTypeItem(Inven_Item const&)
bool IsTradeLimitAttachTypeItem(const Inven_Item& item) {
    CItem* const data_item = G_CDataManager()->find_item(
        item.getKey());
    if (!data_item) return false;
    return data_item->is_stackable() == 1 &&
           data_item->GetAttachType() == 7;
}

// IDA_EA: 0x8120242
// IDA_NAME: _Z32IsTradeLimitAttachTypeItemByUserR5CUserii
// DEMANGLED: IsTradeLimitAttachTypeItemByUser(CUser &,int,int)
bool IsTradeLimitAttachTypeItemByUser(CUser& user,
                                      int inventory_type,
                                      int slot) {
    if (!user.getCurCharacR()) {
        return false;
    }

    const Inven_Item item =
        user.getCurCharacInvenR()->GetInvenSlot(inventory_type, slot);
    return item.item_id_ != 0 && IsTradeLimitAttachTypeItem(item);
}

// IDA_EA: 0x81202A9
// IDA_NAME: _Z30CheckTradeLimitItemCountToMoveRK10Inven_ItemRb
// DEMANGLED: CheckTradeLimitItemCountToMove(Inven_Item const&,bool &)
bool CheckTradeLimitItemCountToMove(const Inven_Item& item, bool& can_move) {
    can_move = true;
    CItem* const data_item = G_CDataManager()->find_item(item.getKey());
    if (!data_item) {
        return false;
    }

    // Old 0x81202E3 uses (is_stackable() ^ 1), so the remaining gate is
    // reached only for stackable attach-type-7 items.  When their count is
    // exhausted, the out parameter alone blocks the move.
    if (data_item->is_stackable() == 1 &&
        data_item->GetAttachType() == 7 &&
        item.GetTradeLimitCount() == 0) {
        can_move = false;
    }
    return true;
}

// IDA_EA: 0x812032E
// IDA_NAME: _Z36CheckItemTradeLimitCountToMoveByUserR5CUseriiRb
// DEMANGLED: CheckItemTradeLimitCountToMoveByUser(CUser &,int,int,bool &)
bool CheckItemTradeLimitCountToMoveByUser(CUser& user,
                                          int inventory_type,
                                          int slot,
                                          bool& can_move) {
    can_move = true;
    if (!user.getCurCharacR()) {
        return false;
    }

    Inven_Item item;
    if (user.getCurCharacInvenR()->GetInvenSlotByRef(inventory_type,
                                                     slot,
                                                     item) != 1) {
        return false;
    }
    return CheckTradeLimitItemCountToMove(item, can_move);
}
