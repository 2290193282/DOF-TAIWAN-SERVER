#include "old_game/appearance/avatar/inter_avatar_db_returns.h"

#include "common/config.h"
#include "old_data/db/avatar_aura/avatar_item_payload.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"

namespace {

const int kAvatarInventoryType = 2;
}  // namespace

// IDA_EA: 0x84ED5CC
Inter_AvatarInsert::Inter_AvatarInsert() : ISigDispatcher() {}

// IDA_EA: 0x84F9DA4 / 0x84F9DD4
Inter_AvatarInsert::~Inter_AvatarInsert() {}

// IDA_EA: 0x84CF5A4
// IDA_NAME: _ZN18Inter_AvatarInsert12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_AvatarInsert::dispatch_sig(CUser *,char *,int)
int Inter_AvatarInsert::dispatch_sig(CUser* user,
                                     char* payload,
                                     int payload_size) {
    if (!user || !payload ||
        payload_size != static_cast<int>(sizeof(SIG_AVATAR_INSERT)) ||
        user->get_state() <= 2) {
        return 0;
    }

    const SIG_AVATAR_INSERT& signal =
        *reinterpret_cast<const SIG_AVATAR_INSERT*>(payload);
    const int inventory_slot = static_cast<int>(signal.slot) -
        oldsrv::config::kAvatarInventorySlotBase;
    if (inventory_slot < 0 ||
        inventory_slot >= oldsrv::config::kAvatarInventoryItemCount) {
        return 0;
    }
    const Inven_Item existing = user->getCurCharacInvenR()->GetInvenSlot(
        kAvatarInventoryType, inventory_slot);

    user->getCurCharacInvenW()->GetAvatarItemMgrW()->AssignAvatarItemKey(
        existing.add_info_, static_cast<int>(signal.ui_id));

    Inven_Item updated = existing;
    updated.set_add_info(static_cast<int>(signal.ui_id));
    user->getCurCharacInvenW()->update_item(
        static_cast<INVEN_TYPE>(kAvatarInventoryType), inventory_slot, updated);
    return 0;
}

// IDA_EA: 0x84ED5E8
Inter_AvatarChangeStat::Inter_AvatarChangeStat() : ISigDispatcher() {}

// IDA_EA: 0x84F9D56 / 0x84F9D86
Inter_AvatarChangeStat::~Inter_AvatarChangeStat() {}

// IDA_EA: 0x84CF6EC
// IDA_NAME: _ZN22Inter_AvatarChangeStat12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_AvatarChangeStat::dispatch_sig(CUser *,char *,int)
int Inter_AvatarChangeStat::dispatch_sig(CUser* user,
                                         char* payload,
                                         int payload_size) {
    if (!user || !payload ||
        payload_size != static_cast<int>(sizeof(SIG_AVATAR_CHANGE_STAT)) ||
        user->get_state() <= 2) {
        return 0;
    }

    const SIG_AVATAR_CHANGE_STAT& signal =
        *reinterpret_cast<const SIG_AVATAR_CHANGE_STAT*>(payload);
    const int inventory_slot = static_cast<int>(signal.slot) -
        oldsrv::config::kAvatarInventorySlotBase;
    if (inventory_slot < 0 ||
        inventory_slot >= oldsrv::config::kAvatarInventoryItemCount) {
        return 0;
    }
    const int key = static_cast<int>(signal.ui_id);
    WongWork::CAvatarItemMgr* const avatars =
        user->getCurCharacInvenW()->GetAvatarItemMgrW();
    if (avatars->GetExpireDate(key) == -1) {
        const SIG_AVATAR_CHANGE_STAT::LoadedAvatar& loaded =
            signal.charac_or_loaded.loaded;
        avatars->RegistItem(key,
                            static_cast<int>(loaded.expire_date),
                            static_cast<int>(loaded.reg_date),
                            loaded.ipg_agency_no,
                            loaded.jewel_socket,
                            loaded.expansion_or_db_row.expansion_info,
                            false);
    }
    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(1),
                             inventory_slot);
    return 0;
}
