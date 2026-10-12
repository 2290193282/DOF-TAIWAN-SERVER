#include "old_game/economy/mail/mail_trade_value_types.h"
#include "common/config.h"

#include <cstring>

#include "common/manager/rdar_string_manager_abi.h"
#include "common/manager/user_history_log_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

namespace {

bool has_escrow_item(const TradeSlot& slot) {
    return !slot.item_.isEmpty();
}

}  // namespace

// IDA_EA: 0x852E3A8
// IDA_NAME: _ZN11CTradeSpace24send_lose_item_for_chinaEP5CUserR10Inven_Item
// DEMANGLED: CTradeSpace::send_lose_item_for_china(CUser *,Inven_Item &)
int CTradeSpace::send_lose_item_for_china(CUser* user, Inven_Item& item) {
    if (item.isEmpty()) {
        return 0;
    }

    char title[0x1d] = {};
    char text[0xff] = {};
    std::strncpy(title,
                 g_scriptStringManager_.findString(4,
                                                    "game_server_msg_287",
                                                    0),
                 sizeof(title));
    std::strncpy(text,
                 g_scriptStringManager_.findString(4,
                                                    "game_server_msg_288",
                                                    0),
                 sizeof(text));

    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        item,
        0,
        user->getCurCharacNo(),
        text,
        std::strlen(text),
        0,
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
        false,
        false);
    return 1;
}

// IDA_EA: 0x852E544
// IDA_NAME: _ZN11CTradeSpace22cancel_trade_for_chinaEP5CUser
// DEMANGLED: CTradeSpace::cancel_trade_for_china(CUser *)
int CTradeSpace::cancel_trade_for_china(CUser* disconnected_user) {
    PacketGuard packets[2];

    for (int trader_index = 0; trader_index <= 1; ++trader_index) {
        int entry_count = 0;
        for (int slot_index = 3; slot_index <= 26; ++slot_index) {
            if (has_escrow_item(trade_slots_[trader_index][slot_index])) {
                ++entry_count;
            }
        }

        PacketGuard& packet = packets[trader_index];
        packet.clear();
        packet.put_header(0, 16);

        int applied_count = 0;
        int count_index = 0;
        if (trade_gold_[trader_index] <= 0) {
            count_index = packet.get_index();
            packet.put_short(entry_count);
        } else {
            ++entry_count;
            CUser* const trader = traders_[trader_index];
            if (trader) {
                if (!trader->getCurCharacR()) {
                    cMyTrace trace(
                        "bool CTradeSpace::cancel_trade_for_china(CUser*)",
                        1820,
                        5);
                    trace("CTradeSpace::cancel_trade()\tm_pTraders[user_index]->getCurCharacR()");
                    return 0;
                }

                CInventory* const inventory = static_cast<CInventory*>(
                    trader->getCurCharacInvenW());
                inventory->gain_money(trade_gold_[trader_index],
                                      static_cast<eMoneyAddReason>(2),
                                      true,
                                      0);
                trade_gold_[trader_index] = 0;
                ++applied_count;
            }
            count_index = packet.get_index();
            packet.put_short(entry_count);
            packet.put_short(0);
            packet.put_byte(0);
            packet.put_short(0);
        }

        for (int slot_index = 3; slot_index <= 26; ++slot_index) {
            TradeSlot& trade_slot = trade_slots_[trader_index][slot_index];
            CUser* const trader = traders_[trader_index];
            if (!trader || !trader->getCurCharacR() ||
                !has_escrow_item(trade_slot)) {
                continue;
            }

            packet.put_short(slot_index);
            const uint8_t trade_type = trade_slot.item_space_;
            Inven_Item& item = trade_slot.item_;
            CInventory* const inventory = static_cast<CInventory*>(
                trader->getCurCharacInvenW());
            int empty_slot = -1;

            if (trade_type == 1) {
                empty_slot = inventory->insertAvatarIntoInventory(
                    item,
                    static_cast<eAvatarItemAddReason>(3),
                    true);
                if (empty_slot >= 0 && trade_slot.original_slot_ != empty_slot) {
                    const CInventory* const readable_inventory =
                        static_cast<const CInventory*>(
                            trader->getCurCharacInvenR());
                    readable_inventory->SendAvatarEvent(
                        50,
                        trade_slot.original_slot_ +
                            oldsrv::config::kAvatarInventorySlotBase,
                        empty_slot +
                            oldsrv::config::kAvatarInventorySlotBase,
                        item.get_add_info(),
                        0);
                }
                packet.put_byte(1);
            } else if (trade_type == 7) {
                empty_slot = inventory->insertItemIntoCreature(
                    item,
                    static_cast<eItemAddReason>(28),
                    true,
                    true);
                if (empty_slot >= 0 && trade_slot.original_slot_ != empty_slot) {
                    user_creature::CCreatureMgr* const creature_manager =
                        static_cast<user_creature::CCreatureMgr*>(
                            inventory->GetCreatureMgrW());
                    creature_manager->ChangeSlotNo(&item, empty_slot, 28);
                }
                packet.put_byte(7);
            } else if (trade_type) {
                empty_slot = inventory->insertItemIntoEquipment(
                    item,
                    static_cast<eItemAddReason>(2),
                    true,
                    true);
                packet.put_byte(3);
            } else {
                empty_slot = inventory->insertItemIntoInventory(
                    item,
                    static_cast<eItemAddReason>(2),
                    true,
                    true);
                packet.put_byte(0);
            }

            if (empty_slot < 0) {
                send_lose_item_for_china(trader, item);
            }
            item.reset();
            packet.put_short(empty_slot);
            ++applied_count;
        }

        packet.put_short(count_index, applied_count);
        packet.finalize(true);
    }

    for (int trader_index = 0; trader_index <= 1; ++trader_index) {
        CUser* const trader = traders_[trader_index];
        if (trader && trader != disconnected_user) {
            trader->Send(packets[trader_index]);
            trader->send_equip(0);
        }
    }
    for (int trader_index = 0; trader_index <= 1; ++trader_index) {
        CUser* const trader = traders_[trader_index];
        if (trader) {
            CUser_history_log(trader)->TradeEnd(
                static_cast<eTradeEndReason>(0), 0, 0, 0, 0);
        }
    }
    return 1;
}

// IDA_EA: 0x852EFFA
// IDA_NAME: _ZN11CTradeSpace12cancel_tradeEv
// DEMANGLED: CTradeSpace::cancel_trade(void)
int CTradeSpace::cancel_trade() {
    if ((traders_[0])->getCurCharacR() &&
        (traders_[1])->getCurCharacR()) {
        return cancel_trade_for_china(0);
    }

    cMyTrace trace("bool CTradeSpace::cancel_trade()", 1928, 5);
    trace("CTradeSpace::cancel_trade()\t0 == m_pTraders[0]->getCurCharacR() || m_pTraders[1]->getCurCharacR()");
    return 0;
}

// IDA_EA: 0x852F086
// IDA_NAME: _ZN11CTradeSpace19cancel_trade_by_disEP5CUser
// DEMANGLED: CTradeSpace::cancel_trade_by_dis(CUser *)
int CTradeSpace::cancel_trade_by_dis(CUser* disconnected_user) {
    return cancel_trade_for_china(disconnected_user);
}
