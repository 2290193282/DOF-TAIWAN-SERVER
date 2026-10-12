#include "old_game/player/core/cuser.h"

#include "common/manager/cdata_manager_tail_public_abi.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"

#include <cstring>

// IDA_EA: 0x868E596
// IDA_NAME: _ZN5CUser25AddCurCharacMercenaryInfoEv
// DEMANGLED: CUser::AddCurCharacMercenaryInfo(void)
bool CUser::AddCurCharacMercenaryInfo() {
    if (!getCurCharacR()) {
        return true;
    }

    const short level = static_cast<short>(get_charac_level());
    const short minimum_level = static_cast<short>(
        G_CDataManager()->get_mercenary_system_info()->default_count);
    if (level < minimum_level) {
        return true;
    }

    CHARAC_LOAD_MERCENARY mercenary;
    mercenary.Reset();
    mercenary.level = static_cast<uint16_t>(get_charac_level());
    mercenary.charac_no = static_cast<uint32_t>(getCurCharacNo());
    std::strncpy(mercenary.name, getCurCharacName(), 0x1du);
    mercenary.avatar_bonus_index =
        static_cast<uint8_t>(CalcMercenaryAvatarBonus());
    return mercenary_info_.AddCharac(mercenary) != 0;
}

// IDA_EA: 0x868EA68
// IDA_NAME: _ZNK5CUser24CalcMercenaryAvatarBonusEv
// DEMANGLED: CUser::CalcMercenaryAvatarBonus(void)const
int CUser::CalcMercenaryAvatarBonus() const {
    int minimum_grade = 10;
    if (!getCurCharacR()) {
        return 0;
    }

    const CInventory* const inventory = getCurCharacInvenR();
    if (!inventory) {
        return 0;
    }

    for (int slot = 0; slot <= 7; ++slot) {
        const Inven_Item* const inventory_item =
            inventory->GetInvenRef(0, slot);
        if (!inventory_item) {
            return 0;
        }

        CItem* const item = G_CDataManager()->find_item(
            inventory_item->item_id_);
        // The reconstructed CItem ABI publishes the original virtual slot
        // directly. CStackableItem's slot returns false, so this retains the
        // old CItem-to-CEquipItem RTTI gate before accepting an avatar item.
        if (!item || !item->vtable_ ||
            !item->vtable_->is_avatar_item(item)) {
            return 0;
        }

        const int grade = item->get_grade();
        if (grade < minimum_grade) {
            minimum_grade = grade;
        }
    }

    return minimum_grade <= 3 ? minimum_grade : 0;
}

// IDA_EA: 0x868EBAA
// IDA_NAME: _ZN5CUser18setAvatarTypeBonusEc
// DEMANGLED: CUser::setAvatarTypeBonus(char)
char CUser::setAvatarTypeBonus(char avatar_bonus_index) {
    const uint8_t requested_index =
        static_cast<uint8_t>(avatar_bonus_index);
    char result = static_cast<char>(requested_index);
    if (requested_index >= 4u) {
        return result;
    }

    if (!getCurCharacR()) {
        return 1;
    }

    const int index = mercenary_info_.FindIndexByCharacNo(
        static_cast<uint32_t>(getCurCharacNo()));
    result = static_cast<char>(index);
    if (index == -1) {
        return result;
    }

    const bool invalid_index = !mercenary_info_.CheckCharacIndex(
        static_cast<uint8_t>(index));
    result = static_cast<char>(invalid_index);
    if (invalid_index) {
        return result;
    }

    CHARAC_LOAD_MERCENARY& mercenary =
        mercenary_info_.mercenaries_[static_cast<uint8_t>(index)];
    mercenary.avatar_bonus_index = requested_index;
    return static_cast<char>(reinterpret_cast<uintptr_t>(&mercenary));
}
