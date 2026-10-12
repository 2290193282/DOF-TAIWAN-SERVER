#include "old_game/player/core/cuser.h"

#include "common/config.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/equipment/item_option.h"
#include "old_game/economy/equipment/expand_equipslot_runtime.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_lock_runtime.h"
#include "old_game/player/core/title_book_runtime.h"


// IDA_EA: 0x8680E04
// IDA_NAME: _ZNK15CUserCharacInfo12FindItemLockEhR14ENUM_ITEMSPACERiP5CUser
// DEMANGLED: CUserCharacInfo::FindItemLock(unsigned char,ENUM_ITEMSPACE &,int &,CUser *)const
bool CUserCharacInfo::FindItemLock(unsigned char lock,
                                   ENUM_ITEMSPACE& item_space,
                                   int& slot,
                                   CUser* user) const {
    if (!user || !getCurCharacR()) {
        return false;
    }
    if (getCurCharacCargoR()->FindItemLock(lock, item_space, slot)) {
        return true;
    }
    if (getCurCharacInvenR()->FindItemLock(lock, item_space, slot)) {
        return true;
    }

    const charac_expand::CData* const expand_equipslot =
        user->GetCharacExpandData(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(9));
    if (reinterpret_cast<const CExpandEquipslot*>(expand_equipslot)
            ->FindItemLock(lock, item_space, slot)) {
        return true;
    }

    const charac_expand::CData* const title_book =
        user->GetCharacExpandData(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(14));
    return reinterpret_cast<const CTitleBook*>(title_book)
               ->findItemLock(lock, item_space, slot) != 0;
}

// IDA_EA: 0x8689808
// IDA_NAME: _ZN15cUserHistoryLog16ItemLockUsingLogEhiiii
// DEMANGLED: cUserHistoryLog::ItemLockUsingLog(unsigned char,int,int,int,int)
int cUserHistoryLog::ItemLockUsingLog(unsigned char action,
                                      int item_space,
                                      int slot,
                                      int item_id,
                                      int item_key) {
    return owner_->LogHistory("ItemLock,%d,%d,%d,%d,%d",
                              action,
                              item_space,
                              slot,
                              item_id,
                              item_key);
}

// IDA_EA: 0x8646A9C
// IDA_NAME: _ZNK5CUser14IsAbleItemLockEii
// DEMANGLED: CUser::IsAbleItemLock(int,int)const
int CUser::IsAbleItemLock(int inventory_type, int slot) const {
    const Inven_Item* const inventory_item = GetCurCharacInventoryRef(
        static_cast<unsigned short>(inventory_type),
        static_cast<unsigned short>(slot));
    if (!inventory_item) {
        return 21;
    }

    CItem* const item = G_CDataManager()->find_item(inventory_item->item_id_);
    if (!item) {
        return 21;
    }
    if (item->is_stackable()) {
        return 19;
    }

    const CItemVTable* const item_vtable =
        *reinterpret_cast<const CItemVTable* const*>(item);
    const int item_type = item_vtable->get_item_type(item);
    if (item_type == oldsrv::config::kEquipmentTypeCreature &&
        inventory_item->seal_or_state_ != 1u) {
        return 19;
    }
    if (item_type == oldsrv::config::kEquipmentTypeTitle &&
        item->GetAttachType() == 2) {
        return 17;
    }
    if (inventory_type == 12) {
        return 19;
    }

    const stAmplifyOption_t* const amplify = &inventory_item->amplify_option_;
    const unsigned char lock =
        static_cast<unsigned char>(amplify->GetLock());
    if (lock) {
        return 19;
    }

    const item_lock::CItemLock* const lock_data =
        reinterpret_cast<const item_lock::CItemLock*>(
            GetCharacExpandDataR(static_cast<ENUM_CHARAC_EXPAND_TYPE>(2)));
    const int result = lock_data->IsAbleItemLock(lock);
    return result ? result : 0;
}

// IDA_EA: 0x8646BFE
// IDA_NAME: _ZNK5CUser16IsAbleItemUnlockEiiRi
// DEMANGLED: CUser::IsAbleItemUnlock(int,int,int &)const
int CUser::IsAbleItemUnlock(int inventory_type,
                            int slot,
                            int& unlock_time) const {
    const Inven_Item* const inventory_item = GetCurCharacInventoryRef(
        static_cast<unsigned short>(inventory_type),
        static_cast<unsigned short>(slot));
    if (!inventory_item) {
        return 21;
    }
    if (inventory_type == 12) {
        return 19;
    }
    const stAmplifyOption_t* const amplify = &inventory_item->amplify_option_;
    const unsigned char lock = static_cast<unsigned char>(amplify->GetLock());
    if (!lock) {
        return 19;
    }

    const item_lock::CItemLock* const lock_data =
        reinterpret_cast<const item_lock::CItemLock*>(
            GetCharacExpandDataR(static_cast<ENUM_CHARAC_EXPAND_TYPE>(2)));
    const int result = lock_data->IsAbleItemUnlock(lock, unlock_time);
    return result ? result : 0;
}

// IDA_EA: 0x8646CAE
// IDA_NAME: _ZNK5CUser22IsAbleItemUnlockCancelEii
// DEMANGLED: CUser::IsAbleItemUnlockCancel(int,int)const
int CUser::IsAbleItemUnlockCancel(int inventory_type, int slot) const {
    const Inven_Item* const inventory_item = GetCurCharacInventoryRef(
        static_cast<unsigned short>(inventory_type),
        static_cast<unsigned short>(slot));
    if (!inventory_item) {
        return 21;
    }
    const stAmplifyOption_t* const amplify = &inventory_item->amplify_option_;
    const unsigned char lock = static_cast<unsigned char>(amplify->GetLock());
    if (!lock) {
        return 19;
    }

    const item_lock::CItemLock* const lock_data =
        reinterpret_cast<const item_lock::CItemLock*>(
            GetCharacExpandDataR(static_cast<ENUM_CHARAC_EXPAND_TYPE>(2)));
    const int result = lock_data->IsAbleItemUnlockCancel(lock);
    return result ? result : 0;
}
