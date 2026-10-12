#include "old_game/gm/core/game_master_mail_trade_commands.h"
#include "common/config.h"

#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/legacy_dispatch/db_sr08_leaf_dispatchers.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"

#include <cstdlib>
#include <cstring>

namespace {

// 0x84B2640 writes bit 5 of UpgradeSeparateInfo for the exact item ranges
// below.  Keeping the mutation at the named typed subobject avoids rebuilding
// any CInventory or Inven_Item byte projection in this command provider.
void SetTradeRestriction(Inven_Item* item) {
    item->upgrade_separate_.bits_ |= 0x20u;
}

bool IsRestrictableGeneralItem(const Inven_Item* item) {
    return item->item_id_ != 0 && item->item_category_ == 1;
}

}  // namespace

namespace game_master {

// IDA_EA: 0x82A8060
// IDA_NAME: _ZN11game_master31CAutoMarketConditionsControlCmdC2Ev
// DEMANGLED: game_master::CAutoMarketConditionsControlCmd::CAutoMarketConditionsControlCmd(void)
CAutoMarketConditionsControlCmd::CAutoMarketConditionsControlCmd()
    : CCommand() {}

// IDA_EA: 0x84A8FFC
// IDA_NAME: _ZN11game_master31CAutoMarketConditionsControlCmd7executeEv
// DEMANGLED: game_master::CAutoMarketConditionsControlCmd::execute(void)
void CAutoMarketConditionsControlCmd::execute() {
    // The old body is a real zero-instruction command body.  Registration and
    // command routing intentionally remain outside this recovered provider.
}

// IDA_EA: 0x82A853E
// IDA_NAME: _ZN11game_master14CAddItemByMailC2Ev
// DEMANGLED: game_master::CAddItemByMail::CAddItemByMail(void)
CAddItemByMail::CAddItemByMail()
    : CCommand() {}

// IDA_EA: 0x84AC2AA
// IDA_NAME: _ZN11game_master14CAddItemByMail5parseEv
// DEMANGLED: game_master::CAddItemByMail::parse(void)
void CAddItemByMail::parse() {
    std::string& command_tail = parser_->command_tail();
    trim(command_tail);
    item_id_ = std::atoi(command_tail.c_str());
}

// IDA_EA: 0x84AC2E2
// IDA_NAME: _ZN11game_master14CAddItemByMail7executeEv
// DEMANGLED: game_master::CAddItemByMail::execute(void)
void CAddItemByMail::execute() {
    CUser* const user = GetUser();
    CItem* const requested_item = G_CDataManager()->find_item(item_id_);
    if (!requested_item) {
        return;
    }

    // 0x84AC330..0x84AC3F8 initializes the same two stack buffers before
    // loading their original RDAR strings.  The wire call uses msg_143 as the
    // title and msg_144 as the letter text.
    char letter_text[256] = {};
    char title[21] = {};
    std::strncpy(letter_text,
                 g_scriptStringManager_.findString(4, "game_server_msg_144", 0),
                 0xffu);
    std::strncpy(title,
                 g_scriptStringManager_.findString(4, "game_server_msg_143", 0),
                 0x14u);

    const int selected_item_id = item_id_;
    const unsigned int one = 1;
    Inven_Item mail_item;
    unsigned int money = 0;

    if (selected_item_id != 0) {
        if (selected_item_id == 1) {
            // Exact special test item construction at 0x84AC438..0x84AC470.
            mail_item.item_id_ = 1;
            mail_item.item_category_ = 2;
            mail_item.add_info_ = static_cast<int32_t>(one);
            mail_item.avatar_select_ = 1;
            mail_item.ResetItemAttr();
        } else {
            // The second lookup and virtual slot-2 make_item call are both
            // present in the old body; do not substitute a new item factory.
            CItem* const prototype =
                G_CDataManager()->find_item(selected_item_id);
            if (!prototype) {
                return;
            }
            mail_item.item_id_ = selected_item_id;
            prototype->vtable_->make_item(prototype, mail_item);
            mail_item.set_add_info(static_cast<int>(one));
        }
    } else {
        money = one;
    }

    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        mail_item,
        money,
        static_cast<unsigned int>(user->getCurCharacNo()),
        letter_text,
        static_cast<int>(std::strlen(letter_text)),
        0,
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
        false,
        false);
}

// IDA_EA: 0x82A88CA
// IDA_NAME: _ZN11game_master18CTestTradeRestrictC2Ev
// DEMANGLED: game_master::CTestTradeRestrict::CTestTradeRestrict(void)
CTestTradeRestrict::CTestTradeRestrict()
    : CCommand() {}

// IDA_EA: 0x84B263A
// IDA_NAME: _ZN11game_master18CTestTradeRestrict5parseEv
// DEMANGLED: game_master::CTestTradeRestrict::parse(void)
void CTestTradeRestrict::parse() {
    // The old parser is deliberately empty.
}

// IDA_EA: 0x84B2640
// IDA_NAME: _ZN11game_master18CTestTradeRestrict7executeEv
// DEMANGLED: game_master::CTestTradeRestrict::execute(void)
void CTestTradeRestrict::execute() {
    CUser* const user = GetUser();
    if (!user) {
        return;
    }

    // Stackable inventory through the expanded equipment page.
    for (int slot = 0;
         slot <= oldsrv::config::kMainInventoryEquipmentSlotLast;
         ++slot) {
        const Inven_Item* const item =
            user->getCurCharacInvenR()->GetInvenRef(1, slot);
        if (IsRestrictableGeneralItem(item)) {
            SetTradeRestriction(
                user->getCurCharacInvenW()->GetInvenRef(1, slot));
        }
    }

    // Equipment inventory includes the native weapon-avatar record.
    for (int slot = 0; slot < oldsrv::config::kEquipmentTypeCount; ++slot) {
        const Inven_Item* const item =
            user->getCurCharacInvenR()->GetInvenRef(0, slot);
        if (IsRestrictableGeneralItem(item)) {
            SetTradeRestriction(
                user->getCurCharacInvenW()->GetInvenRef(0, slot));
        }
    }

    // Avatar inventory: raw CInventory +0x654, slots 0..104.  The original
    // code restricts every occupied avatar record, regardless of category.
    for (int slot = 0;
         slot <= oldsrv::config::kAvatarInventoryLastSlot;
         ++slot) {
        const Inven_Item* const item =
            user->getCurCharacInvenR()->GetInvenRef(2, slot);
        if (item->item_id_ != 0) {
            SetTradeRestriction(
                user->getCurCharacInvenW()->GetInvenRef(2, slot));
        }
    }

    // Character cargo: 0x84B2640 deliberately obtains the writable cargo
    // and its backing start for every slot, before it tests the item.  The
    // writable accessor marks cargo for saving even when no slot is
    // restrictable; do not replace this with the read accessor plus a
    // conditional write accessor.
    for (int slot = 0; slot <= 151; ++slot) {
        CCargo* const cargo = user->getCurCharacCargoW();
        Inven_Item* const item = cargo->get_cargo_start() + slot;
        if (IsRestrictableGeneralItem(item)) {
            SetTradeRestriction(item);
        }
    }

    // The old body obtains the typed first account-cargo slot once, then
    // walks the fixed contiguous 56-Inven_Item backing store.  Retain that
    // one GetSlotRef side effect without recreating a byte-offset bridge.
    CAccountCargo* const account_cargo = user->GetAccountCargo();
    Inven_Item* const first_account_slot =
        account_cargo ? account_cargo->GetSlotRef(0) : 0;
    if (first_account_slot) {
        for (int slot = 0; slot <= 55; ++slot) {
            Inven_Item* const item = first_account_slot + slot;
            if (IsRestrictableGeneralItem(item)) {
                SetTradeRestriction(item);
            }
        }
    }

    // The existing DB leaf owns signal 28 and its DBTask slot.  This command
    // only preserves the original request; it creates no registration.
    DB_TestTradeRestrict::makeRequest(
        static_cast<unsigned int>(user->getCurCharacNo()));
}

}  // namespace game_master
