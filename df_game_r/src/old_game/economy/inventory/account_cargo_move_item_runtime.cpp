#include "old_game/economy/inventory/inventory.h"

#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_lock_runtime.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/system_time.h"

namespace {

bool IsForbiddenAccountCargoCategory(const Inven_Item& item) {
    switch (item.item_category_) {
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            return true;
        default:
            return false;
    }
}

void CopyItem(Inven_Item& destination, const Inven_Item& source) {
    destination.setCopy(source);
}

}  // namespace

// IDA_EA: 0x8289A4A
// IDA_NAME: _ZN13CAccountCargo20CheckInsertConditionER10Inven_Item
// DEMANGLED: CAccountCargo::CheckInsertCondition(Inven_Item &)
bool CAccountCargo::CheckInsertCondition(Inven_Item& item) {
    CItem* const prototype = G_CDataManager()->find_item(item.item_id_);
    if (!prototype || !prototype->isPackagable()) return false;

    const int lock = item.amplify_option_.GetLock();
    if (lock) {
        item_lock::CItemLock* const locks =
            static_cast<item_lock::CItemLock*>(
                user_->GetCharacExpandDataR(
                    static_cast<ENUM_CHARAC_EXPAND_TYPE>(2)));
        if (locks && locks->CheckItemLock(static_cast<unsigned char>(lock))) {
            return false;
        }
    }

    if (IsForbiddenAccountCargoCategory(item) ||
        (item.item_id_ > 0x1963 && item.item_id_ <= 0x1b57)) {
        return false;
    }

    switch (prototype->GetAttachType()) {
        case 1:
        case 2:
            return false;
        case 3:
            if (item.seal_or_state_ != 1) return false;
            break;
        default:
            break;
    }

    if (item.upgrade_separate_.IsTradeRestriction()) return false;
    if (user_->isGMUser() == 1 || prototype->vtable_->is_avatar_item(prototype)) {
        return true;
    }
    if (!prototype->getUsablePeriod() && !prototype->getExpirationDate()) {
        return true;
    }

    const int expiration = 86400 * static_cast<int>(item.avatar_select_) +
                           1151683200;
    return expiration > GlobalData::s_systemTime_.getCurSec();
}

// IDA_EA: 0x8289C82
// IDA_NAME: _ZN13CAccountCargo10InsertItemER10Inven_Itemi
// DEMANGLED: CAccountCargo::InsertItem(Inven_Item &,int)
int CAccountCargo::InsertItem(Inven_Item& item, int slot) {
    if (!CheckValidSlot(slot)) return -1;

    Inven_Item& destination = items_[slot];
    if (item.isEquipableItemType()) {
        if (!destination.isEmpty()) return -1;
        CopyItem(destination, item);
        altered_ = 1;
        return slot;
    }

    if (destination.item_id_ == item.item_id_) {
        destination.set_add_info(destination.get_add_info() + item.get_add_info());
    } else {
        CopyItem(destination, item);
    }
    altered_ = 1;
    return slot;
}

int CAccountCargo::InsertExactEquipmentForCargoTransport(
    const Inven_Item& item,
    int slot) {
    if (!CheckValidSlot(slot) || !items_[slot].isEmpty() || item.isEmpty() ||
        item.item_category_ != 1 || !item.isEquipableItemType()) {
        return -1;
    }

    // Inven_Item::setCopy intentionally omits the ten-byte GUID at +0x15.
    // CMD551 transfers ownership of the same equipment record, so use the
    // canonical complete-record assignment also used by inventory update
    // paths instead of manufacturing a new account-cargo identity.
    items_[slot] = item;
    altered_ = 1;
    return slot;
}

bool CAccountCargo::RestoreSlotSnapshotForTransaction(
    int slot,
    const Inven_Item& snapshot) {
    if (!CheckValidSlot(slot)) return false;
    // setCopy() deliberately omits the ten-byte globally unique identifier.
    // Compensation must restore the complete 0x3d-byte equipment record or
    // a failed CMD551 could silently change identity while rolling back.
    items_[slot] = snapshot;
    altered_ = 1;
    return true;
}

// IDA_EA: 0x8289F26
// IDA_NAME: _ZN13CAccountCargo8MoveItemEii
// DEMANGLED: CAccountCargo::MoveItem(int,int)
int CAccountCargo::MoveItem(int source_slot, int destination_slot) {
    if (!CheckValidSlot(source_slot) || !CheckValidSlot(destination_slot) ||
        source_slot == destination_slot) {
        return 0;
    }

    // Account cargo is shared by every character on the account.  setCopy()
    // deliberately omits the ten-byte GUID, so using it here would detach a
    // transported equipment's persistent identity merely by moving it to a
    // different shared-cargo slot.  Exchange the complete 0x3d-byte records;
    // a later withdrawal by any character then observes exactly the identity
    // that CMD551 committed.
    const Inven_Item source = items_[source_slot];
    items_[source_slot] = items_[destination_slot];
    items_[destination_slot] = source;
    altered_ = 1;
    return 1;
}

// IDA_EA: 0x828A670
// IDA_NAME: _ZN13CAccountCargo15CheckStackLimitEiii
// DEMANGLED: CAccountCargo::CheckStackLimit(int,int,int)
bool CAccountCargo::CheckStackLimit(int slot, int item_id, int count) {
    if (!CheckValidSlot(slot) || items_[slot].item_id_ != item_id) {
        return false;
    }
    CItem* const prototype = G_CDataManager()->find_item(item_id);
    if (!prototype || prototype->is_stackable() != 1) return false;

    const int total = items_[slot].get_add_info() + count;
    const int limit = static_cast<CStackableItem*>(prototype)->getStackableLimit();
    return total >= 0 && total <= limit;
}
