#include "old_game/gm/item/game_master_inventory_clear_commands.h"
#include "common/config.h"

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"

namespace game_master {

// IDA_EA: 0x82A8338
// IDA_NAME: _ZN11game_master18CClearInventoryCmdC2Ev
// DEMANGLED: game_master::CClearInventoryCmd::CClearInventoryCmd(void)
CClearInventoryCmd::CClearInventoryCmd() : CCommand() {}

// IDA_EA: 0x84AAEF8
// IDA_NAME: _ZN11game_master18CClearInventoryCmd7executeEv
// DEMANGLED: game_master::CClearInventoryCmd::execute(void)
void CClearInventoryCmd::execute() {
    CUser* const user = GetUser();
    if (!user) {
        return;
    }

    // Preserve the three special slots at 0..2 and clear every target-client
    // main-inventory slot through the jewel page.
    for (int slot = oldsrv::config::kMainInventoryQuickSlotFirst;
         slot <= oldsrv::config::kMainInventoryLastSlot;
         ++slot) {
        user->getCurCharacInvenW()->ResetSlot(1, slot);
    }

    if (static_cast<unsigned char>(user->send_itemspace(0)) != 1u) {
        LogManager::logFormat(
            1,
            "GameMaster.cpp",
            "virtual void game_master::CClearInventoryCmd::execute()",
            0x63B,
            "CClearInventoryCmd::execute, pUser(%s)->send_itemspace( ENUM_ITEMSPACE_INVENTORY ) return FAILED",
            NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0));
    }
}

// IDA_EA: 0x82A8354
// IDA_NAME: _ZN11game_master20CClearAvatarInvenCmdC2Ev
// DEMANGLED: game_master::CClearAvatarInvenCmd::CClearAvatarInvenCmd(void)
CClearAvatarInvenCmd::CClearAvatarInvenCmd() : CCommand() {}

// IDA_EA: 0x84AAFB0
// IDA_NAME: _ZN11game_master20CClearAvatarInvenCmd7executeEv
// DEMANGLED: game_master::CClearAvatarInvenCmd::execute(void)
void CClearAvatarInvenCmd::execute() {
    CUser* const user = GetUser();
    if (!user) {
        return;
    }

    for (int slot = 0;
         slot <= oldsrv::config::kAvatarInventoryLastSlot;
         ++slot) {
        const Inven_Item* const item =
            user->getCurCharacInvenR()->GetInvenRef(2, slot);
        if (!item || item->item_id_ == 0) {
            continue;
        }

        // delete_item resets the item record, so retain the avatar key before
        // entering the existing deletion transaction.
        const int avatar_key = item->add_info_;
        if (user->getCurCharacInvenW()->delete_item(
                static_cast<INVEN_TYPE>(2),
                slot,
                1,
                static_cast<eItemDelReason>(3),
                true) != 1) {
            LogManager::logFormat(
                1,
                "GameMaster.cpp",
                "virtual void game_master::CClearAvatarInvenCmd::execute()",
                0x64D,
                "CClearAvatarInvenCmd::execute, pUser(%s)->getCurCharacInvenW()->delete_item return FAILED",
                NumberToString(static_cast<unsigned int>(user->get_acc_id()),
                               0));
            return;
        }

        user->getCurCharacInvenW()->SendAvatarEvent(
            54,
            avatar_key,
            1,
            slot + oldsrv::config::kAvatarInventorySlotBase,
            0);
    }

    if (static_cast<unsigned char>(user->send_itemspace(1)) != 1u) {
        LogManager::logFormat(
            1,
            "GameMaster.cpp",
            "virtual void game_master::CClearAvatarInvenCmd::execute()",
            0x656,
            "CClearAvatarInvenCmd::execute, pUser(%s)->send_itemspace( ENUM_ITEMSPACE_AVATAR ) return FAILED",
            NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0));
    }
}

}  // namespace game_master
