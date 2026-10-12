#include "old_game/player/core/cuser.h"

#include "old_game/economy/equipment/expand_equipslot_runtime.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/title_book_runtime.h"

namespace {

const Inven_Item* GetReadInventoryRef(const CUser* user,
                                      unsigned short inventory_type,
                                      unsigned short slot) {
    switch (inventory_type) {
        case 0:
            return user->getCurCharacInvenR()->GetInvenRef(1, slot);
        case 1:
            return user->getCurCharacInvenR()->GetInvenRef(2, slot);
        case 2:
            return user->getCurCharacCargoR()->GetCargoRef(slot);
        case 3:
            return user->getCurCharacInvenR()->GetInvenRef(0, slot);
        case 7:
            return user->getCurCharacInvenR()->GetInvenRef(3, slot);
        case 18: {
            charac_expand::CData* data = user->GetCharacExpandDataR(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(9));
            return data ? static_cast<CExpandEquipslot*>(data)
                              ->GetInvenRefR(static_cast<INVEN_TYPE>(5), slot)
                        : 0;
        }
        case 19:
        case 20:
        case 21:
        case 22:
        case 23: {
            charac_expand::CData* data = user->GetCharacExpandDataR(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(14));
            return data ? static_cast<CTitleBook*>(data)->getTitleSlot(
                              static_cast<ENUM_TITLE_BOOK_CATEGORY>(
                                  inventory_type - 19u),
                              slot)
                        : 0;
        }
        default:
            return 0;
    }
}

Inven_Item* GetWriteInventoryRef(CUser* user,
                                 unsigned short inventory_type,
                                 unsigned short slot) {
    switch (inventory_type) {
        case 0:
            return user->getCurCharacInvenW()->GetInvenRef(1, slot);
        case 1:
            return user->getCurCharacInvenW()->GetInvenRef(2, slot);
        case 2:
            return reinterpret_cast<CCargo*>(user->getCurCharacCargoW())
                ->GetCargoRef(slot);
        case 3:
            return user->getCurCharacInvenW()->GetInvenRef(0, slot);
        case 7:
            return user->getCurCharacInvenW()->GetInvenRef(3, slot);
        case 18: {
            charac_expand::CData* data = user->GetCharacExpandData(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(9));
            return data ? static_cast<CExpandEquipslot*>(data)->GetInvenRefW(
                              static_cast<INVEN_TYPE>(5), slot)
                        : 0;
        }
        case 19:
        case 20:
        case 21:
        case 22:
        case 23: {
            charac_expand::CData* data = user->GetCharacExpandData(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(14));
            return data ? static_cast<CTitleBook*>(data)->getTitleSlot(
                              static_cast<ENUM_TITLE_BOOK_CATEGORY>(
                                  inventory_type - 19u),
                              slot)
                        : 0;
        }
        default:
            return 0;
    }
}

}  // namespace

// IDA_EA: 0x8680C8A
// IDA_NAME: _ZNK5CUser24GetCurCharacInventoryRefEtt
// DEMANGLED: CUser::GetCurCharacInventoryRef(unsigned short,unsigned short)const
const Inven_Item* CUser::GetCurCharacInventoryRef(
    unsigned short inventory_type,
    unsigned short slot) const {
    return getCurCharacR()
               ? GetReadInventoryRef(this, inventory_type, slot)
               : 0;
}

// IDA_EA: 0x8680F2E
// IDA_NAME: _ZN5CUser24GetCurCharacInventoryRefEtt
// DEMANGLED: CUser::GetCurCharacInventoryRef(unsigned short,unsigned short)
Inven_Item* CUser::GetCurCharacInventoryRef(unsigned short inventory_type,
                                             unsigned short slot) {
    return getCurCharacR()
               ? GetWriteInventoryRef(this, inventory_type, slot)
               : 0;
}
