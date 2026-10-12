#include "old_game/economy/shop/npc_runtime.h"

#include "common/manager/data_manager.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "common/manager/history_log_abi.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const int kInventoryItemSpace = 0;
const int kCargoItemSpace = 2;
const int kEquipmentItemSpace = 3;
const int kCreatureItemSpace = 7;

int CopySellItem(CUser* user,
                 char item_space,
                 short slot,
                 Inven_Item& item) {
    if (item_space == kCargoItemSpace) {
        item = user->getCurCharacCargoR()->get_cargo_slot(slot);
        return 0;
    }
    if (item_space != kInventoryItemSpace && item_space != kEquipmentItemSpace &&
        item_space != kCreatureItemSpace) {
        return 17;
    }
    item = user->getCurCharacInvenR()->GetInvenSlot(
        GetInvenTypeFromItemSpace(static_cast<ENUM_ITEMSPACE>(item_space)),
        slot);
    return 0;
}

}  // namespace

// IDA_EA: 0x8619008
// IDA_NAME: _ZN5Store16GetSellItemPriceEP5CUsercssbRiS2_
// DEMANGLED: Store::GetSellItemPrice(CUser *,char,short,short,bool,int &,int &)
int Store::GetSellItemPrice(CUser* user,
                            char item_space,
                            short slot,
                            short count,
                            bool dungeon_sale,
                            int& item_id,
                            int& price) {
    Inven_Item item;
    if (!user || CopySellItem(user, item_space, slot, item) != 0 ||
        item.item_id_ == 0) {
        return 17;
    }

    CItem* const prototype = G_CDataManager()->find_item(item.item_id_);
    if (!prototype) {
        return 17;
    }
    if (prototype->GetFinishPointPrice() || prototype->GetAttachType() == 2) {
        return 23;
    }
    item_id = prototype->get_index();
    return GetSellItemPrice(item, prototype, count, dungeon_sale, price);
}

// IDA_EA: 0x86193F8
// IDA_NAME: _ZN5Store14user_sell_itemEP5CUsercss
// DEMANGLED: Store::user_sell_item(CUser *,char,short,short)
int Store::user_sell_item(CUser* user,
                          char item_space,
                          short slot,
                          short count) {
    if (user->CheckInTrade()) {
        return 19;
    }

    int item_id = 0;
    int price = 0;
    const int price_error = GetSellItemPrice(user,
                                             item_space,
                                             slot,
                                             count,
                                             user->get_state() == 5,
                                             item_id,
                                             price);
    if (price_error) {
        return price_error;
    }
    if (!user->CheckMoney(price)) {
        return 22;
    }

    Inven_Item sold_item;
    if (CopySellItem(user, item_space, slot, sold_item) != 0) {
        return 17;
    }

    int delete_result = 0;
    if (item_space == kCargoItemSpace) {
        delete_result = user->getCurCharacCargoW()->delete_item(
            slot, count, static_cast<eItemDelReason>(0));
    } else if (item_space == kEquipmentItemSpace) {
        delete_result = user->getCurCharacInvenW()->delete_item(
            static_cast<INVEN_TYPE>(0),
            slot,
            1,
            static_cast<eItemDelReason>(0),
            true);
        if (delete_result == 1) {
            user->send_equip(slot);
        }
    } else if (item_space == kCreatureItemSpace) {
        delete_result = user->getCurCharacInvenW()->delete_item(
            static_cast<INVEN_TYPE>(3),
            slot,
            count,
            static_cast<eItemDelReason>(0),
            true);
    } else if (item_space == kInventoryItemSpace) {
        delete_result = user->getCurCharacInvenW()->delete_item(
            static_cast<INVEN_TYPE>(1),
            slot,
            count,
            static_cast<eItemDelReason>(0),
            true);
    } else {
        return 17;
    }
    if (delete_result != 1) {
        return 17;
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    inventory->gain_money(price,
                          static_cast<eMoneyAddReason>(0),
                          true,
                          0);

    if (!sold_item.isEmpty()) {
        CItem* const prototype = G_CDataManager()->find_item(sold_item.item_id_);
        if (prototype && prototype->is_stackable()) {
            sold_item.set_add_info(count);
        }
        user->Add_RedeemInfo(sold_item, price, true);
    }

    PacketGuard response;
    response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                        ENUM_CMDPACKET_SELL_ITEM);
    response.put_byte(1);
    response.put_int(user->getCurCharacInvenR()->get_money());
    response.put_byte(item_space);
    response.put_short(slot);
    response.put_short(count);
    response.finalize(true);
    user->Send(response);

    if (CHackLog_InvitationPaperSellToNPC::IsInvitationPaper(item_id)) {
        user->getHackAnalyzer()->addServerHackCnt(
            user,
            static_cast<WongWork::ENUM_HACKTYPE>(
                CHackLog_InvitationPaperSellToNPC::GetHackType()),
            1u,
            0u,
            0u);
    }
    HistoryLog::WriteSellItem(user->history_stream(), item_id, count);
    return 0;
}
