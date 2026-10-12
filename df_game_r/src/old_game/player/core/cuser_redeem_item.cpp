#include "old_game/player/core/cuser.h"

#include <stdint.h>

#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/redeem_item_runtime.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const eItemAddReason kRedeemItemAddReason =
    static_cast<eItemAddReason>(41);
const eMoneySubReason kRedeemMoneySubReason =
    static_cast<eMoneySubReason>(37);

void SendRedeemTradeError(CUser* user, int trace_line) {
    cMyTrace trace("bool CUser::Redeem_Item(int)", trace_line, 5);
    trace("redeem error : trade : m_id(%s)charac_no(%u)",
          NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0),
          static_cast<unsigned int>(user->get_charac_no(-1)));
    user->SendCmdErrorPacket(ENUM_CMDPACKET_REDEEM, 3);
}

}  // namespace

// IDA_EA: 0x8646DF2
// IDA_NAME: _ZN5CUser11Redeem_ItemEi
// DEMANGLED: CUser::Redeem_Item(int)
int CUser::Redeem_Item(int index) {
    Redeem_Item::CRedeemItem* const redeem =
        static_cast<Redeem_Item::CRedeemItem*>(GetCharacExpandData(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(6)));
    if (!redeem) {
        SendCmdErrorPacket(ENUM_CMDPACKET_REDEEM, 3);
        return 0;
    }

    if (private_store::GetInstancePrivateStoreMgr()->IsBusyPrivateStore(this)) {
        SendRedeemTradeError(this, 1321);
        return 0;
    }
    if (CheckInTrade()) {
        SendRedeemTradeError(this, 1327);
        return 0;
    }

    const int price = redeem->GetItemPrice(index);
    if (price < 0) {
        SendCmdErrorPacket(ENUM_CMDPACKET_REDEEM, 3);
        return 0;
    }
    if (getCurCharacMoney() < price) {
        SendCmdErrorPacket(ENUM_CMDPACKET_REDEEM, 70);
        return 0;
    }

    const Redeem_Item::Inven_Item_Expand_RedeemInfo redeem_info =
        redeem->GetRedeemInfoR(index);
    const Inven_Item& item = Redeem_Item::GetInvenItem(redeem_info);
    if (item.isEmpty()) {
        SendCmdErrorPacket(ENUM_CMDPACKET_REDEEM, 3);
        return 0;
    }

    CInventory* const inventory = getCurCharacInvenW();
    const int slot = inventory->insertItemIntoInventory(
        item, kRedeemItemAddReason, true, true);
    if (slot < 0) {
        SendCmdErrorPacket(ENUM_CMDPACKET_REDEEM, 4);
        return 0;
    }

    inventory->use_money(price, kRedeemMoneySubReason, true);
    CItem* const item_data = G_CDataManager()->find_item(item.item_id_);
    history_log().RedeemItemDel(
        static_cast<int>(item.item_id_),
        item_data->is_stackable() ? static_cast<int>(item.add_info_) : 1);
    redeem->RedeemItem(index);

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_REDEEM);
    packet.put_byte(1);
    // Native 140828 response handler reads the inserted inventory slot as
    // u16; the old dword writer shifted the next packet cursor by two bytes.
    packet.put_short(static_cast<uint16_t>(slot));
    packet.finalize(true);
    Send(packet);
    return 1;
}
