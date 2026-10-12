#include "old_game/economy/mall/auction_inter_return_dispatchers.h"
#include "infra/network/packet/PacketEnum.h"
#include "common/config.h"

#include <cstring>
#include <stdint.h>

#include "old_game/player/core/action_point_packet_dispatchers.h"
#include "old_game/economy/mall/auction_server_proxy_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/ipg_runtime.h"
#include "old_game/economy/equipment/item_option.h"
#include "common/manager/prepay_info_abi.h"
#include "old_game/economy/trade/trade_limit_item_abi.h"
#include "common/manager/user_history_log_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mall/auction_inter_return_payloads.h"
#include "old_game/economy/mall/auction_regist_item_return_abi.h"
#include "old_game/economy/mail/mail_trade_packets.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/economy/inventory/taiwan_billing_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

namespace {

// IDA_DATA: _ZL6gmList_0.  These are the five literal character numbers read
// by every original auction acknowledgement trace branch.  They are not a
// configuration list and must remain in this exact order.
const int kAuctionDelayTraceCharacters[5] = {
    0x004c2dc8,
    0x005cf723,
    0x00a9c742,
    0x00a2ad85,
    0x009ae1ae,
};

char kBillingConfirmStage[] = "confirm";
char kBillingRollbackStage[] = "rollback";

template <typename Payload>
Payload CopyAuctionPayload(const char* source) {
    Payload value;
    std::memcpy(&value, source, sizeof(value));
    return value;
}

// CIPGMgr deliberately stays private to the IPG runtime because callers must
// not depend on its 0xf0-byte implementation layout.  The old non-virtual
// provider is nevertheless an exported ABI function; this declaration calls
// it by its original name rather than inventing an offset access.
}  // namespace

// IDA_EA: 0x8685838
// IDA_NAME: _ZN15cUserHistoryLog11BillingFailE17ENUM_BILLING_TYPEyiiPc
// DEMANGLED: cUserHistoryLog::BillingFail(ENUM_BILLING_TYPE,unsigned long long,int,int,char *)
int cUserHistoryLog::BillingFail(ENUM_BILLING_TYPE billing_type,
                                 unsigned long long transaction_id,
                                 int billing_item_id,
                                 int error_code,
                                 char* stage) {
    return owner_->LogHistory("BILLING, Fail,%d,%s,%d,%d,%s",
                              static_cast<int>(billing_type),
                              NumberToString(transaction_id, 0),
                              billing_item_id,
                              error_code,
                              stage);
}

// IDA_EA: 0x86858A0
// IDA_NAME: _ZN15cUserHistoryLog14BillingSuccessE17ENUM_BILLING_TYPEyi
// DEMANGLED: cUserHistoryLog::BillingSuccess(ENUM_BILLING_TYPE,unsigned long long,int)
int cUserHistoryLog::BillingSuccess(ENUM_BILLING_TYPE billing_type,
                                    unsigned long long transaction_id,
                                    int billing_item_id) {
    const char* const transaction_text = NumberToString(transaction_id, 0);
    if (static_cast<int>(billing_type) == 3) {
        return owner_->LogHistory("BILLING, Success,%d,%s",
                                  static_cast<int>(billing_type),
                                  transaction_text);
    }
    // The old format intentionally uses %d for the transaction text in the
    // non-type-3 branch.  Preserve the original audit record byte-for-byte.
    return owner_->LogHistory("BILLING, Success,%d,%d,%d",
                              static_cast<int>(billing_type),
                              transaction_text,
                              billing_item_id);
}

// IDA_EA: 0x84EDD20
// IDA_NAME: _ZN23Inter_NotifyAuctionMailC2Ev
Inter_NotifyAuctionMail::Inter_NotifyAuctionMail() : ISigDispatcher() {}

// IDA_EA: 0x84F8A24 / 0x84F8A54
// IDA_NAME: _ZN23Inter_NotifyAuctionMailD2Ev / D0Ev
Inter_NotifyAuctionMail::~Inter_NotifyAuctionMail() {}

// IDA_EA: 0x84D55A4
// IDA_NAME: _ZN23Inter_NotifyAuctionMail12dispatch_sigEP5CUserPci
int Inter_NotifyAuctionMail::dispatch_sig(CUser* user,
                                          char* payload,
                                          int payload_size) {
    LogManager::logFormat(
        1,
        "InterDispatcher.cpp",
        "Inter_NotifyAuctionMail::dispatch_sig",
        0,
        "[AUCTION_MAIL_DIAG][INTER239_ENTER] payload_size=%d "
        "expected_size=%u user_present=%d",
        payload_size,
        static_cast<unsigned int>(sizeof(AuctionNotifyMailPayload)),
        user ? 1 : 0);
    if (!user || !payload ||
        payload_size !=
            static_cast<int>(sizeof(AuctionNotifyMailPayload))) {
        LogManager::logFormat(
            1,
            "InterDispatcher.cpp",
            "Inter_NotifyAuctionMail::dispatch_sig",
            0,
            "[AUCTION_MAIL_DIAG][INTER239_REJECT_FRAME] "
            "payload_present=%d payload_size=%d expected_size=%u "
            "user_present=%d",
            payload ? 1 : 0,
            payload_size,
            static_cast<unsigned int>(sizeof(AuctionNotifyMailPayload)),
            user ? 1 : 0);
        return 0;
    }
    // The validated Inter body is the complete packed monitor frame.  Bind
    // it in place: PacketHeader intentionally has no default constructor, so
    // manufacturing a temporary here would invent sender-side state before
    // immediately overwriting it with the wire bytes.
    const AuctionNotifyMailPayload& result =
        *reinterpret_cast<const AuctionNotifyMailPayload*>(payload);
    if (!user->getCurCharacR() ||
        user->getCurCharacNo() != result.receiver_charac_no) {
        LogManager::logFormat(
            1,
            "InterDispatcher.cpp",
            "Inter_NotifyAuctionMail::dispatch_sig",
            0,
            "[AUCTION_MAIL_DIAG][INTER239_REJECT_RECEIVER] "
            "character_loaded=%d receiver_match=%d",
            user->getCurCharacR() ? 1 : 0,
            user->getCurCharacR() &&
                    user->getCurCharacNo() == result.receiver_charac_no
                ? 1
                : 0);
        return 14152;
    }

    if (G_GameWorld()->GetChannelType() == 7) {
        LogManager::logFormat(
            1,
            "InterDispatcher.cpp",
            "Inter_NotifyAuctionMail::dispatch_sig",
            0,
            "[AUCTION_MAIL_DIAG][INTER239_REJECT_CHANNEL] channel_type=7");
        return 0;
    }
    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        LogManager::logFormat(
            1,
            "InterDispatcher.cpp",
            "Inter_NotifyAuctionMail::dispatch_sig",
            0,
            "[AUCTION_MAIL_DIAG][INTER239_REJECT_CHANNEL] "
            "integrated_pvp=1");
        return 0;
    }

    WongWork::CMailBox* const mailbox = user->GetMailBox();
    if (!mailbox) {
        LogManager::logFormat(
            1,
            "InterDispatcher.cpp",
            "Inter_NotifyAuctionMail::dispatch_sig",
            0,
            "[AUCTION_MAIL_DIAG][INTER239_REJECT_MAILBOX] mailbox_present=0");
        return 0;
    }
    mailbox->IncNotLoadedMailCount();

    PacketGuard packet;
    packet.put_header(0, 169);
    packet.put_byte(result.notification_type);
    packet.put_byte(result.mail_item_type);
    packet.put_int(static_cast<int>(result.mail_item_id));
    packet.put_short(mailbox->GetNotLoadedMailCount());
    result.random_option.put_packet_random_option(packet);
    packet.finalize(true);
    const int send_result = user->Send(packet);
    LogManager::logFormat(
        1,
        "InterDispatcher.cpp",
        "Inter_NotifyAuctionMail::dispatch_sig",
        0,
        "[AUCTION_MAIL_DIAG][NOTI169_SEND] result=%d not_loaded=%d "
        "notification_type=%u mail_item_type=%u",
        send_result,
        mailbox->GetNotLoadedMailCount(),
        static_cast<unsigned int>(result.notification_type),
        static_cast<unsigned int>(result.mail_item_type));
    return 0;
}

// IDA_EA: 0x84EDD3C
// IDA_NAME: _ZN27Inter_AuctionResultAskPriceC2Ev
Inter_AuctionResultAskPrice::Inter_AuctionResultAskPrice()
    : ISigDispatcher() {}

// IDA_EA: 0x84F89D6 / 0x84F8A06
// IDA_NAME: _ZN27Inter_AuctionResultAskPriceD2Ev / D0Ev
Inter_AuctionResultAskPrice::~Inter_AuctionResultAskPrice() {}

// IDA_EA: 0x84D5740
// IDA_NAME: _ZN27Inter_AuctionResultAskPrice12dispatch_sigEP5CUserPci
int Inter_AuctionResultAskPrice::dispatch_sig(CUser* user,
                                              char* payload,
                                              int) {
    if (!user) {
        return 0;
    }
    const AuctionResultAskPricePayload result =
        CopyAuctionPayload<AuctionResultAskPricePayload>(payload);
    if (result.charac_no != user->getCurCharacNo()) {
        return 0;
    }

    for (int n4 = 0; n4 <= 4; ++n4) {
        if (kAuctionDelayTraceCharacters[n4] == user->getCurCharacNo()) {
            cMyTrace trace(
                "virtual int Inter_AuctionResultAskPrice::dispatch_sig(CUser*, char*, int)",
                14204,
                0);
            trace("Trace Auction Delay, %s(%d)",
                  "ACK AuctionResultAskPrice",
                  user->getCurCharacNo());
            break;
        }
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_AUCTION_ASK_AVERAGE_PRICE);
    packet.put_byte(1);
    packet.put_byte(static_cast<unsigned char>(result.payment_type));
    packet.put_int(result.representative_price);
    for (int n2 = 0; n2 <= 2; ++n2) {
        packet.put_int(result.first_price_values[n2]);
    }
    for (int n2_1 = 0; n2_1 <= 2; ++n2_1) {
        packet.put_int(result.second_price_values[n2_1]);
    }
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EDD58
// IDA_NAME: _ZN37Inter_AuctionResultAskRegistedItemNumC2Ev
Inter_AuctionResultAskRegistedItemNum::Inter_AuctionResultAskRegistedItemNum()
    : ISigDispatcher() {}

// IDA_EA: 0x84F8988 / 0x84F89B8
// IDA_NAME: _ZN37Inter_AuctionResultAskRegistedItemNumD2Ev / D0Ev
Inter_AuctionResultAskRegistedItemNum::~Inter_AuctionResultAskRegistedItemNum() {}

// IDA_EA: 0x84D5930
// IDA_NAME: _ZN37Inter_AuctionResultAskRegistedItemNum12dispatch_sigEP5CUserPci
int Inter_AuctionResultAskRegistedItemNum::dispatch_sig(CUser* user,
                                                        char* payload,
                                                        int) {
    if (!user) {
        return 0;
    }

    // The original entry reads the fixed 0x2d-byte Inter result directly.
    // It deliberately does not add a payload-size gate here.
    AuctionResultAskRegistedItemNumPayload result;
    std::memcpy(&result, payload, sizeof(result));
    if (result.charac_no != user->getCurCharacNo()) {
        return 0;
    }

    LogManager::logFormat(1, "auction_inter_return_dispatchers.cpp",
        "Inter_AuctionResultAskRegistedItemNum::dispatch_sig", 0,
        "[AUCTION][REGIST_PREFLIGHT] charac=%d payment=%u listings=%d space=%u slot=%u item=%u count=%d",
        result.charac_no, result.payment_type, static_cast<int>(result.registed_item_count),
        static_cast<unsigned int>(result.item_space), static_cast<unsigned int>(result.item_slot),
        result.item_index, result.item_count);

    for (int n4 = 0; n4 <= 4; ++n4) {
        if (kAuctionDelayTraceCharacters[n4] == user->getCurCharacNo()) {
            cMyTrace trace(
                "virtual int Inter_AuctionResultAskRegistedItemNum::dispatch_sig(CUser*, char*, int)",
                14243,
                0);
            trace("Trace Auction Delay, %s(%d)",
                  "ACK AuctionResultAskRegistedItemNum",
                  user->getCurCharacNo());
            break;
        }
    }

    const PAY_TYPE payment_type = static_cast<PAY_TYPE>(result.payment_type);
    if (result.registed_item_count == -1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                 158,
                                 payment_type);
        return 0;
    }
    if (result.registed_item_count == -2) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                 145,
                                 payment_type);
        return 0;
    }

    if (user->isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(29))) {
        const int max_registed_item_count =
            result.payment_type == 1 ? 10 : 30;
        if (result.registed_item_count >= max_registed_item_count) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                     151,
                                     payment_type);
            return 0;
        }
    } else {
        const int max_registed_item_count =
            result.payment_type == 1 ? 3 : 10;
        if (result.registed_item_count >= max_registed_item_count) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                     151,
                                     payment_type);
            return 0;
        }
    }

    const int auction_fee = result.payment_type == 1 ? 0 : 10000;
    if (user->getCurCharacMoney() < auction_fee) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                 144,
                                 payment_type);
        return 0;
    }

    int cera_avatar_price = 0;
    if (result.item_index > 0x28d287u &&
        result.item_index <= 0x28d299u &&
        result.payment_type == 1) {
        if (result.item_index > 0x28d291u) {
            cera_avatar_price = static_cast<int>(
                result.item_index * 10000000u - 88713216u);
        } else {
            cera_avatar_price = static_cast<int>(
                result.item_index * 1000000u + 429625408u);
        }
    }

    PCK_AUCTION_REGIST_ITEM_GA request;
    AuctionRegistItemGaBody& request_body = AuctionRegistItemGaBodyOf(request);
    request_body.user_uid = user->GetUID();
    request_body.charac_no = user->getCurCharacNo();
    std::strncpy(request_body.charac_name,
                 user->getCurCharacR()->fields_.character_name_,
                 sizeof(request_body.charac_name));
    request_body.is_affected_premium =
        static_cast<uint8_t>(user->isAffectedPremium(
            static_cast<ENUM_PREMIUM_TYPE>(29)));
    request_body.field_28 = result.field_15;
    request_body.field_2c = result.field_19;
    request_body.curr_charac_black_count = user->getCurrCharacBlackCount();
    request_body.field_95 = result.field_1d;
    request_body.category = result.category;

    CInventory* const inventory = user->getCurCharacInvenW();
    int inventory_type = 6;
    Inven_Item inventory_item;

    if (result.item_index != 0) {
        if (result.item_space == 1) {
            inventory_type = 2;
        } else if (result.item_space == 7) {
            inventory_type = 3;
        } else if (result.item_space == 0) {
            inventory_type = 1;
        } else {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                     145,
                                     payment_type);
            return 0;
        }

        if (result.item_index > 0x28d287u &&
            result.item_index <= 0x28d299u) {
            if (result.payment_type == 1 &&
                inventory->use_money(cera_avatar_price,
                                     static_cast<eMoneySubReason>(38),
                                     true) == 1) {
                user->SaveInventory();
                user->SendUpdateItemList(
                    static_cast<CUser::eSendTarget>(1),
                    static_cast<ENUM_ITEMSPACE>(0),
                    0);
                request.setCategory(18);
                request_body.item.add_info_ = result.item_count;
                request_body.emblem.init();
                request_body.item.seal_or_state_ = 0;
                request_body.item.item_id_ = result.item_index;
                request_body.item.avatar_select_ = 0;
                request_body.item.extend_info_ = 0;
                const size_t account_length = std::strlen(user->get_acc_name());
                std::strncpy(request_body.account_name,
                             user->get_acc_name(),
                             account_length);
                GlobalData::s_cera_auction_proxy->SendPacket(
                    reinterpret_cast<nsl::PACKET_HEADER*>(&request));
                return 0;
            }

            user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                     147,
                                     payment_type);
            return 0;
        }

        inventory_item = inventory->GetInvenSlot(inventory_type,
                                                 result.item_slot);
        if (static_cast<uint32_t>(inventory_item.item_id_) !=
                result.item_index ||
            inventory_item.item_id_ == 4332) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                     145,
                                     payment_type);
            return 0;
        }

        if (IsTradeLimitAttachTypeItemByUser(*user,
                                             inventory_type,
                                             result.item_slot)) {
            bool can_move = true;
            CheckItemTradeLimitCountToMoveByUser(*user,
                                                 inventory_type,
                                                 result.item_slot,
                                                 can_move);
            if (!can_move) {
                user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                         114);
                return 0;
            }
        }

        if (!inventory_item.isEquipableItemType() &&
            inventory_item.add_info_ < result.item_count) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                     146,
                                     payment_type);
            return 0;
        }

        CItem* const item = G_CDataManager()->find_item(
            static_cast<int>(inventory_item.item_id_));
        if (!item || !item->isPackagable()) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                     145,
                                     payment_type);
            return 0;
        }

        if (item->vtable_->is_avatar_item(item)) {
            const int expire_date = inventory->GetAvatarItemMgrR()->GetExpireDate(
                inventory_item.add_info_);
            if (expire_date != 0 &&
                GlobalData::s_systemTime_.getCurSec() > expire_date) {
                user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                         145,
                                         payment_type);
                return 0;
            }
        } else if (item->getUsablePeriod() || item->getExpirationDate()) {
            // Preserve the original second test even though the outer
            // condition makes its success path redundant.
            if (!item->getUsablePeriod() && !item->getExpirationDate()) {
                user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                         145,
                                         payment_type);
                return 0;
            }
            const int expiration_date =
                86400 * static_cast<int>(inventory_item.avatar_select_) +
                1151683200;
            if (expiration_date <= GlobalData::s_systemTime_.getCurSec()) {
                user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                         145,
                                         payment_type);
                return 0;
            }
        }

        if (inventory_item.item_category_ == 5 &&
            !inventory->GetCreatureMgrR()->FindCreatureItem(
                inventory_item.add_info_)) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                     145,
                                     payment_type);
            return 0;
        }

        if (!request_body.is_affected_premium && item->GetCashPrice() &&
            !G_CDataManager()->is_AuctionRegFreeCeraItem(result.item_index)) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                     149,
                                     payment_type);
            return 0;
        }

        switch (item->GetAttachType()) {
        case 1:
        case 2:
        case 5:
            user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                     145,
                                     payment_type);
            return 0;
        case 3:
            if (inventory_item.seal_or_state_ != 1) {
                user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                         145,
                                         payment_type);
                return 0;
            }
            break;
        default:
            break;
        }

        if (inventory_item.upgrade_separate_.IsTradeRestriction()) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM, 145);
            return 0;
        }

        if (item->vtable_->is_avatar_item(item)) {
            const WongWork::CAvatarItemMgr* const avatar_items =
                inventory->GetAvatarItemMgrR();
            if (WongWork::CAvatarItemMgr::IsTempKey(
                    inventory_item.add_info_) ||
                avatar_items->GetExpireDate(inventory_item.add_info_)) {
                user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                         145,
                                         payment_type);
                return 0;
            }
        }

        request_body.item.add_info_ = inventory_item.isEquipableItemType()
            ? inventory_item.add_info_
            : result.item_count;
        request_body.item.item_attr_ = static_cast<uint8_t>(
            inventory_item.GetItemAttr());
        request_body.emblem.init();

        if (item->vtable_->is_avatar_item(item)) {
            const WongWork::CAvatarItemMgr* const avatar_items =
                inventory->GetAvatarItemMgrR();
            std::memcpy(&request_body.emblem,
                        avatar_items->getJewelSocketData(
                            inventory_item.add_info_),
                        sizeof(request_body.emblem));
            const stAvatarExpansionInfo_t* const expansion =
                avatar_items->GetExpansionInfo(inventory_item.add_info_);
            if (expansion) {
                std::memcpy(&request_body.expansion,
                            expansion,
                            sizeof(request_body.expansion));
            } else {
                request_body.expansion.init();
            }
            if (request_body.emblem.sockets[0].socket_type) {
                request_body.item.item_attr_ =
                    static_cast<uint8_t>(
                        (request_body.item.item_attr_ & 0xe0u) | 1u);
            }
        }

        request_body.item.seal_or_state_ = inventory_item.seal_or_state_;
        request_body.item.item_id_ = inventory_item.item_id_;
        request_body.item.avatar_select_ = inventory_item.avatar_select_;
        request_body.item.extend_info_ = inventory_item.extend_info_;
        inventory_item.amplify_option_.getValues(
            request_body.item.amplify_type_,
            request_body.item.amplify_value_);
        std::memcpy(&request_body.item.globally_unique_id_,
                    &inventory_item.globally_unique_id_,
                    sizeof(request_body.item.globally_unique_id_));
        std::memcpy(&request_body.item.random_option_,
                    &inventory_item.chronicle_random_option_.random_option(),
                    sizeof(request_body.item.random_option_));
        request_body.item.upgrade_separate_.SetUpgradeSeparate(
            static_cast<uint8_t>(
                inventory_item.upgrade_separate_.GetUpgradeSeparate()));

        if (item->vtable_->is_random_option(item)) {
            if (request_body.category.category_0 == 0) {
                request_body.category.category_0 = 30000;
            }
            if (request_body.category.category_1 == 0) {
                request_body.category.category_1 = 30000;
            }
            if (request_body.category.category_2 == 0) {
                request_body.category.category_2 = 30000;
            }
        }
        request_body.category._sort();
    }

    if (inventory->use_money(auction_fee,
                              static_cast<eMoneySubReason>(25),
                              true) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                 147,
                                 payment_type);
        return 0;
    }

    if (inventory_item.item_id_ == 0) {
        user->SaveInventory();
        const size_t account_length = std::strlen(user->get_acc_name());
        std::strncpy(request_body.account_name,
                     user->get_acc_name(),
                     account_length);
        GlobalData::s_auction_proxy->SendPacket(
            reinterpret_cast<nsl::PACKET_HEADER*>(&request));
        return 0;
    }

    if (inventory_item.item_category_ == 8) {
        if (inventory->delete_item(static_cast<INVEN_TYPE>(2),
                                   result.item_slot,
                                   1,
                                   static_cast<eItemDelReason>(5),
                                   true) != 1) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                     148,
                                     payment_type);
            const int gained_money = inventory->gain_money(
                10000,
                static_cast<eMoneyAddReason>(19),
                true,
                0);
            if (gained_money <= 9999) {
                user->SendMoneyFullReason(
                    static_cast<ENUM_MONEY_FULL_REASON>(0),
                    10000,
                    gained_money);
            }
            return 0;
        }
        inventory->SendAvatarEvent(54,
                                   inventory_item.add_info_,
                                   4,
                                   static_cast<int>(result.item_slot) +
                                       oldsrv::config::kAvatarInventorySlotBase,
                                   0);
        inventory->GetAvatarItemMgrW()->UnRegistItem(
            inventory_item.add_info_);
    } else if (inventory->delete_item(
                   static_cast<INVEN_TYPE>(inventory_type),
                   result.item_slot,
                   request_body.item.add_info_,
                   static_cast<eItemDelReason>(25),
                   true) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                 148,
                                 payment_type);
        const int gained_money = inventory->gain_money(
            10000,
            static_cast<eMoneyAddReason>(19),
            true,
            0);
        if (gained_money <= 9999) {
            user->SendMoneyFullReason(
                static_cast<ENUM_MONEY_FULL_REASON>(0),
                10000,
                gained_money);
        }
        return 0;
    }

    if (inventory_item.IsCreatureItemType()) {
        inventory->GetCreatureMgrW()->AuctionRegist(
            &inventory_item,
            result.item_slot,
            25);
    }

    user->SaveInventory();
    const size_t account_length = std::strlen(user->get_acc_name());
    std::strncpy(request_body.account_name,
                 user->get_acc_name(),
                 account_length);
    GlobalData::s_auction_proxy->SendPacket(
        reinterpret_cast<nsl::PACKET_HEADER*>(&request));
    return 0;
}

// IDA_EA: 0x84EDD74
// IDA_NAME: _ZN25Inter_AuctionResultRegistC2Ev
Inter_AuctionResultRegist::Inter_AuctionResultRegist() : ISigDispatcher() {}

// IDA_EA: 0x84F893A / 0x84F896A
// IDA_NAME: _ZN25Inter_AuctionResultRegistD2Ev / D0Ev
Inter_AuctionResultRegist::~Inter_AuctionResultRegist() {}

// IDA_EA: 0x84D6AD6
// IDA_NAME: _ZN25Inter_AuctionResultRegist12dispatch_sigEP5CUserPci
int Inter_AuctionResultRegist::dispatch_sig(CUser* user, char* payload, int) {
    if (!user) {
        return 0;
    }
    const AuctionSimpleResultPayload response =
        CopyAuctionPayload<AuctionSimpleResultPayload>(payload);
    if (response.charac_no != user->getCurCharacNo()) {
        return 0;
    }

    LogManager::logFormat(1, "auction_inter_return_dispatchers.cpp",
        "Inter_AuctionResultRegist::dispatch_sig", 0,
        "[AUCTION][REGIST_RESULT] charac=%d payment=%u result=%u error=%u",
        response.charac_no, response.payment_type,
        static_cast<unsigned int>(response.result),
        static_cast<unsigned int>(response.error_code));

    for (int n4 = 0; n4 <= 4; ++n4) {
        if (kAuctionDelayTraceCharacters[n4] == user->getCurCharacNo()) {
            cMyTrace trace(
                "virtual int Inter_AuctionResultRegist::dispatch_sig(CUser*, char*, int)",
                15028,
                0);
            trace("Trace Auction Delay, %s(%d)",
                  "ACK AuctionResultRegist",
                  user->getCurCharacNo());
            break;
        }
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_AUCTION_REGIST_ITEM);
    const unsigned char result = response.result;
    packet.put_byte(result);
    if (!result) {
        packet.put_byte(response.error_code);
    }
    packet.put_byte(static_cast<unsigned char>(response.payment_type));
    packet.finalize(true);
    user->Send(packet);

    if (result == 1u) {
        APSystem::CUserProc::ClearActionAndSendtoUser(
            user,
            static_cast<APSystem::_ActionGroupIndex>(9),
            0,
            static_cast<ENUM_PACKETCLASS>(0));
    }
    return 0;
}

// IDA_EA: 0x84EDD90
// IDA_NAME: _ZN26Inter_AuctionResultBiddingC2Ev
Inter_AuctionResultBidding::Inter_AuctionResultBidding()
    : ISigDispatcher() {}

// IDA_EA: 0x84F88EC / 0x84F891C
// IDA_NAME: _ZN26Inter_AuctionResultBiddingD2Ev / D0Ev
Inter_AuctionResultBidding::~Inter_AuctionResultBidding() {}

// IDA_EA: 0x84D6CA4
// IDA_NAME: _ZN26Inter_AuctionResultBidding12dispatch_sigEP5CUserPci
int Inter_AuctionResultBidding::dispatch_sig(CUser* user,
                                             char* payload,
                                             int payload_size) {
    if (user) {
        return dispatch_sig_taiwan(user, payload, payload_size);
    }
    return 0;
}

// IDA_EA: 0x84D6CDA
// IDA_NAME: _ZN26Inter_AuctionResultBidding19dispatch_sig_taiwanEP5CUserPci
int Inter_AuctionResultBidding::dispatch_sig_taiwan(CUser* user,
                                                    char* payload,
                                                    int) {
    const AuctionResultBiddingPayload response =
        CopyAuctionPayload<AuctionResultBiddingPayload>(payload);
    if (response.charac_no != user->getCurCharacNo()) {
        return 0;
    }

    for (int n4 = 0; n4 <= 4; ++n4) {
        if (kAuctionDelayTraceCharacters[n4] == user->getCurCharacNo()) {
            cMyTrace trace(
                "int Inter_AuctionResultBidding::dispatch_sig_taiwan(CUser*, char*, int)",
                15378,
                0);
            trace("Trace Auction Delay, %s(%d)",
                  "ACK AuctionResultBidding",
                  user->getCurCharacNo());
            break;
        }
    }

    const int payment_type = static_cast<int>(response.payment_type);
    const bool auction_result = response.result != 0;

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_AUCTION_BIDDING);
    packet.put_byte(response.result);
    if (!auction_result) {
        packet.put_byte(response.error_code);
    }
    packet.put_byte(static_cast<unsigned char>(response.payment_type));

    PrePayInfo* prepay_info = 0;
    WongWork::IPG::CIPGMgr* ipg_manager = 0;
    int primary_item_id = 0;
    int secondary_item_id = 0;
    Taiwan::BillTransaction primary_transaction;
    Taiwan::BillTransaction secondary_transaction;

    if (payment_type == 1) {
        user->SetBuyingGold(false);
        const int charac_no = user->getCurCharacNo();
        prepay_info = G_CPrePayInfoMgr()->FindPrePayInfo(charac_no);
        if (!prepay_info) {
            LogManager::logFormat(
                1,
                "InterDispatcher.cpp",
                "int Inter_AuctionResultBidding::dispatch_sig_taiwan(CUser*, char*, int)",
                15400,
                "PrePayInfo is null. character:%u",
                user->getCurCharacNo());
            return 0;
        }

        ipg_manager = GlobalData::s_pIPGHelper->GetIPGMgr();
        primary_transaction = prepay_info->primary_transaction;
        secondary_transaction = prepay_info->secondary_transaction;
        primary_item_id = prepay_info->billing_item_id;
        secondary_item_id = prepay_info->secondary_billing_item_id;
        G_CPrePayInfoMgr()->DeletePrePayInfo(user->getCurCharacNo());
    }

    if (auction_result) {
        if (payment_type != 1) {
            packet.finalize(true);
            user->Send(packet);
            return 0;
        }

        int billing_result =
            ipg_manager->getBillingCash()->confirm(primary_transaction);
        if (billing_result == 0) {
            user->history_log().BillingSuccess(
                static_cast<ENUM_BILLING_TYPE>(2),
                primary_transaction.transaction_id,
                primary_item_id);
        } else {
            LogManager::logFormat(
                1,
                "InterDispatcher.cpp",
                "int Inter_AuctionResultBidding::dispatch_sig_taiwan(CUser*, char*, int)",
                15455,
                "IPG_CONFIRM user: %d, charac: %s reason: %d",
                user->getCurCharacNo(),
                user->getCurCharacName(),
                billing_result);
            user->history_log().BillingFail(
                static_cast<ENUM_BILLING_TYPE>(2),
                primary_transaction.transaction_id,
                primary_item_id,
                billing_result,
                kBillingConfirmStage);
        }

        billing_result =
            ipg_manager->getBillingCash()->confirm(secondary_transaction);
        if (billing_result != 0) {
            LogManager::logFormat(
                1,
                "InterDispatcher.cpp",
                "int Inter_AuctionResultBidding::dispatch_sig_taiwan(CUser*, char*, int)",
                15466,
                "IPG_CONFIRM user: %d, charac: %s reason: %d",
                user->getCurCharacNo(),
                user->getCurCharacName(),
                billing_result);
            user->history_log().BillingFail(
                static_cast<ENUM_BILLING_TYPE>(2),
                secondary_transaction.transaction_id,
                secondary_item_id,
                billing_result,
                kBillingConfirmStage);
        } else {
            user->history_log().BillingSuccess(
                static_cast<ENUM_BILLING_TYPE>(2),
                secondary_transaction.transaction_id,
                secondary_item_id);
        }
    } else if (payment_type != 1) {
        const int requested_money = response.requested_money;
        const int gained_money = user->getCurCharacInvenW()->gain_money(
            requested_money,
            static_cast<eMoneyAddReason>(20),
            true,
            0);
        if (requested_money > gained_money) {
            user->SendMoneyFullReason(static_cast<ENUM_MONEY_FULL_REASON>(0),
                                      requested_money,
                                      gained_money);
        }
    } else {
        int billing_result =
            ipg_manager->getBillingCash()->cancel(primary_transaction);
        if (billing_result == 0) {
            user->history_log().BillingSuccess(
                static_cast<ENUM_BILLING_TYPE>(2),
                primary_transaction.transaction_id,
                primary_item_id);
        } else {
            LogManager::logFormat(
                1,
                "InterDispatcher.cpp",
                "int Inter_AuctionResultBidding::dispatch_sig_taiwan(CUser*, char*, int)",
                15420,
                "IPG_CANCEL user: %d, charac: %s reason: %d",
                user->getCurCharacNo(),
                user->getCurCharacName(),
                billing_result);
            user->history_log().BillingFail(
                static_cast<ENUM_BILLING_TYPE>(2),
                primary_transaction.transaction_id,
                primary_item_id,
                billing_result,
                kBillingRollbackStage);
        }

        billing_result =
            ipg_manager->getBillingCash()->cancel(secondary_transaction);
        if (billing_result != 0) {
            LogManager::logFormat(
                1,
                "InterDispatcher.cpp",
                "int Inter_AuctionResultBidding::dispatch_sig_taiwan(CUser*, char*, int)",
                15431,
                "IPG_CANCEL user: %d, charac: %s reason: %d",
                user->getCurCharacNo(),
                user->getCurCharacName(),
                billing_result);
            user->history_log().BillingFail(
                static_cast<ENUM_BILLING_TYPE>(2),
                secondary_transaction.transaction_id,
                secondary_item_id,
                billing_result,
                kBillingRollbackStage);
        } else {
            user->history_log().BillingSuccess(
                static_cast<ENUM_BILLING_TYPE>(2),
                secondary_transaction.transaction_id,
                secondary_item_id);
        }
    }

    packet.finalize(true);
    user->Send(packet);
    if (payment_type == 1) {
        GlobalData::s_pIPGHelper->IPGQuery(user);
    }
    return 0;
}

// IDA_EA: 0x84EDDAC
// IDA_NAME: _ZN31Inter_AuctionResultRegistCancelC2Ev
Inter_AuctionResultRegistCancel::Inter_AuctionResultRegistCancel()
    : ISigDispatcher() {}

// IDA_EA: 0x84F889E / 0x84F88CE
// IDA_NAME: _ZN31Inter_AuctionResultRegistCancelD2Ev / D0Ev
Inter_AuctionResultRegistCancel::~Inter_AuctionResultRegistCancel() {}

// IDA_EA: 0x84D741C
// IDA_NAME: _ZN31Inter_AuctionResultRegistCancel12dispatch_sigEP5CUserPci
int Inter_AuctionResultRegistCancel::dispatch_sig(CUser* user,
                                                  char* payload,
                                                  int) {
    if (!user) {
        return 0;
    }
    const AuctionSimpleResultPayload response =
        CopyAuctionPayload<AuctionSimpleResultPayload>(payload);
    if (response.charac_no != user->getCurCharacNo()) {
        return 0;
    }

    for (int n4 = 0; n4 <= 4; ++n4) {
        if (kAuctionDelayTraceCharacters[n4] == user->getCurCharacNo()) {
            cMyTrace trace(
                "virtual int Inter_AuctionResultRegistCancel::dispatch_sig(CUser*, char*, int)",
                15496,
                0);
            trace("Trace Auction Delay, %s(%d)",
                  "ACK AuctionResultRegistCancel",
                  user->getCurCharacNo());
            break;
        }
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_AUCTION_REGIST_CANCEL);
    const unsigned char result = response.result;
    packet.put_byte(result);
    if (!result) {
        packet.put_byte(response.error_code);
    }
    packet.put_byte(static_cast<unsigned char>(response.payment_type));
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EDDC8
// IDA_NAME: _ZN27Inter_AuctionResultItemListC2Ev
Inter_AuctionResultItemList::Inter_AuctionResultItemList()
    : ISigDispatcher() {}

// IDA_EA: 0x84F8850 / 0x84F8880
// IDA_NAME: _ZN27Inter_AuctionResultItemListD2Ev / D0Ev
Inter_AuctionResultItemList::~Inter_AuctionResultItemList() {}

// IDA_EA: 0x84D75BC
// IDA_NAME: _ZN27Inter_AuctionResultItemList12dispatch_sigEP5CUserPci
int Inter_AuctionResultItemList::dispatch_sig(CUser* user,
                                              char* payload,
                                              int payload_size) {
    if (!user) {
        return 0;
    }
    if (payload_size >= static_cast<int>(sizeof(AuctionVariableListPrefix))) {
        const AuctionVariableListPrefix response =
            CopyAuctionPayload<AuctionVariableListPrefix>(payload);
        if (response.charac_no != user->getCurCharacNo()) {
            return 0;
        }

        for (int n4 = 0; n4 <= 4; ++n4) {
            if (kAuctionDelayTraceCharacters[n4] == user->getCurCharacNo()) {
                cMyTrace trace(
                    "virtual int Inter_AuctionResultItemList::dispatch_sig(CUser*, char*, int)",
                    15531,
                    0);
                trace("Trace Auction Delay, %s(%d)",
                      "ACK AuctionResultItemList",
                      user->getCurCharacNo());
                break;
            }
        }

        PacketGuard packet;
        packet.put_header(1, ENUM_CMDPACKET_AUCTION_SEARCH_BY_ITEMKEY);
        packet.put_byte(1);
        packet.put_binary(payload + sizeof(response),
                          payload_size - sizeof(response));
        packet.finalize(true);
        user->Send(packet);
    }
    return 0;
}

// IDA_EA: 0x84EDDE4
// IDA_NAME: _ZN34Inter_AuctionResultMyRegistedItemsC2Ev
Inter_AuctionResultMyRegistedItems::Inter_AuctionResultMyRegistedItems()
    : ISigDispatcher() {}

// IDA_EA: 0x84F8802 / 0x84F8832
// IDA_NAME: _ZN34Inter_AuctionResultMyRegistedItemsD2Ev / D0Ev
Inter_AuctionResultMyRegistedItems::~Inter_AuctionResultMyRegistedItems() {}

// IDA_EA: 0x84D7758
// IDA_NAME: _ZN34Inter_AuctionResultMyRegistedItems12dispatch_sigEP5CUserPci
int Inter_AuctionResultMyRegistedItems::dispatch_sig(CUser* user,
                                                      char* payload,
                                                      int payload_size) {
    if (!user) {
        return 0;
    }
    if (payload_size >= static_cast<int>(sizeof(AuctionVariableListPrefix))) {
        const AuctionVariableListPrefix response =
            CopyAuctionPayload<AuctionVariableListPrefix>(payload);
        if (response.charac_no != user->getCurCharacNo()) {
            return 0;
        }

        for (int n4 = 0; n4 <= 4; ++n4) {
            if (kAuctionDelayTraceCharacters[n4] == user->getCurCharacNo()) {
                cMyTrace trace(
                    "virtual int Inter_AuctionResultMyRegistedItems::dispatch_sig(CUser*, char*, int)",
                    15597,
                    0);
                trace("Trace Auction Delay, %s(%d)",
                      "ACK AuctionResultMyRegistedItems",
                      user->getCurCharacNo());
                break;
            }
        }

        PacketGuard packet;
        packet.put_header(1, ENUM_CMDPACKET_AUCTION_MY_REGISTED_ITEM_INFO);
        packet.put_byte(1);
        packet.put_binary(payload + sizeof(response),
                          payload_size - sizeof(response));
        packet.finalize(true);
        user->Send(packet);
    }
    return 0;
}

// IDA_EA: 0x84EDE00
// IDA_NAME: _ZN28Inter_AuctionResultMyBiddingC2Ev
Inter_AuctionResultMyBidding::Inter_AuctionResultMyBidding()
    : ISigDispatcher() {}

// IDA_EA: 0x84F87B4 / 0x84F87E4
// IDA_NAME: _ZN28Inter_AuctionResultMyBiddingD2Ev / D0Ev
Inter_AuctionResultMyBidding::~Inter_AuctionResultMyBidding() {}

// IDA_EA: 0x84D78F4
// IDA_NAME: _ZN28Inter_AuctionResultMyBidding12dispatch_sigEP5CUserPci
int Inter_AuctionResultMyBidding::dispatch_sig(CUser* user,
                                               char* payload,
                                               int payload_size) {
    if (!user) {
        return 0;
    }
    if (payload_size >= static_cast<int>(sizeof(AuctionVariableListPrefix))) {
        const AuctionVariableListPrefix response =
            CopyAuctionPayload<AuctionVariableListPrefix>(payload);
        if (response.charac_no != user->getCurCharacNo()) {
            return 0;
        }

        for (int n4 = 0; n4 <= 4; ++n4) {
            if (kAuctionDelayTraceCharacters[n4] == user->getCurCharacNo()) {
                cMyTrace trace(
                    "virtual int Inter_AuctionResultMyBidding::dispatch_sig(CUser*, char*, int)",
                    15665,
                    0);
                trace("Trace Auction Delay, %s(%d)",
                      "ACK AuctionResultMyBidding",
                      user->getCurCharacNo());
                break;
            }
        }

        PacketGuard packet;
        packet.put_header(1, ENUM_CMDPACKET_AUCTION_MY_BIDDING_INFO);
        packet.put_byte(1);
        packet.put_binary(payload + sizeof(response),
                          payload_size - sizeof(response));
        packet.finalize(true);
        user->Send(packet);
    }
    return 0;
}
