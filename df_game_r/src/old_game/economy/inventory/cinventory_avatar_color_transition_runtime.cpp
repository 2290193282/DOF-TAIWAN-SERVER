#include "old_game/economy/inventory/inventory.h"
#include "common/config.h"

#include "common/manager/data_manager.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/character/avatar_color_variation.h"
#include "old_game/appearance/pet/creature.h"

namespace {

typedef int (*IsClearAvatarSlot)(const CItem* item);

bool IsClearAvatarPrototype(const CItem* item) {
    if (!item || !item->vtable_) return false;

    // IDA_EVIDENCE: CInventory::MoveClearAvatar @0x8508FD0 dispatches
    // vtable +0x18 (slot 6), not IsAvatarItem at +0x10.  On CEquipItem this
    // is the [clear avatar]/transparent classification.  Keeping the virtual
    // call also preserves the base implementation for non-equipment items.
    const CItemVTable::Entry slot =
        item->vtable_->slots_6_through_9[0];
    return slot &&
           reinterpret_cast<IsClearAvatarSlot>(slot)(item) != 0;
}

}  // namespace

// IDA_EA: 0x8508FD0
// IDA_NAME: _ZN10CInventory15MoveClearAvatarEiiiiii
// DEMANGLED: CInventory::MoveClearAvatar(int,int,int,int,int,int)
void CInventory::MoveClearAvatar(int target_avatar_slot,
                                 int source_avatar_key,
                                 int source_avatar_item_id,
                                 int equipment_slot,
                                 int clear_avatar_key,
                                 int clear_avatar_item_id) {
    // The old body receives both persistent avatar keys from move_item, but
    // only consumes the prototype IDs and slot positions in this transition.
    (void)source_avatar_key;
    (void)clear_avatar_key;

    CItem* source_avatar_item = 0;
    CItem* clear_avatar_item = 0;
    bool is_valid_transition = false;

    // Preserve the original short-circuit order: the outgoing prototype is
    // queried only after the incoming prototype proved to be transparent.
    if (source_avatar_item_id != 0) {
        source_avatar_item = G_CDataManager()->find_item(source_avatar_item_id);
        if (IsClearAvatarPrototype(source_avatar_item) &&
            clear_avatar_item_id != 0) {
            clear_avatar_item =
                G_CDataManager()->find_item(clear_avatar_item_id);
            is_valid_transition =
                clear_avatar_item &&
                !IsClearAvatarPrototype(clear_avatar_item);
        }
    }

    if (!is_valid_transition) {
        // LABEL_23 clears a stale projection when the outgoing equipped
        // prototype is itself transparent (normal-over-transparent or
        // transparent-over-transparent transitions).
        if (clear_avatar_item_id != 0) {
            if (!clear_avatar_item) {
                clear_avatar_item =
                    G_CDataManager()->find_item(clear_avatar_item_id);
            }
            if (IsClearAvatarPrototype(clear_avatar_item)) {
                SetClearAvatar(equipment_slot, 0);
            }
        }
        return;
    }

    if (static_cast<unsigned int>(equipment_slot) >=
        oldsrv::config::kPersistedClearAvatarCount) {
        return;
    }
    SetClearAvatar(equipment_slot, clear_avatar_item_id);

    const Inven_Item& equipment_item = equipment_items_[equipment_slot];
    const stAvatarExpansionInfo_t* const expansion =
        GetAvatarItemMgrW()->GetExpansionInfo(equipment_item.add_info_);
    if (!expansion) {
        return;
    }

    const int avatar_slot =
        target_avatar_slot - oldsrv::config::kAvatarInventorySlotBase;
    if (avatar_slot < 0 ||
        avatar_slot >= oldsrv::config::kAvatarInventoryItemCount) {
        return;
    }

    const AvatarVariation::AvatarColorInfo* const color_info =
        G_CDataManager()->get_avatar_color_info();
    const short first_color =
        static_cast<short>(expansion->avatar_expansion[0]);
    if (color_info->colors.find(first_color) == color_info->colors.end()) {
        LogManager::logFormat(
            1,
            "inventory.cpp",
            "void CInventory::MoveClearAvatar(int, int, int, int, int, int)",
            0x21DB,
            "Avatar color1 error. ItemID(%d), colorId(%d)",
            equipment_item.add_info_,
            first_color);
        return;
    }

    const short second_color =
        static_cast<short>(expansion->avatar_expansion[1]);
    if (color_info->colors.find(second_color) == color_info->colors.end()) {
        LogManager::logFormat(
            1,
            "inventory.cpp",
            "void CInventory::MoveClearAvatar(int, int, int, int, int, int)",
            0x21E2,
            "Avatar color2 error. ItemID(%d), colorId(%d)",
            equipment_item.add_info_,
            first_color);
        return;
    }

    const Inven_Item& target_avatar_item = avatar_items_[avatar_slot];
    if (GetAvatarItemMgrW()->SetAvatarColor(
            target_avatar_item.add_info_,
            first_color,
            second_color) == 1) {
        SendAvatarEvent(794,
                        target_avatar_item.add_info_,
                        first_color,
                        second_color,
                        0);
    }
}
