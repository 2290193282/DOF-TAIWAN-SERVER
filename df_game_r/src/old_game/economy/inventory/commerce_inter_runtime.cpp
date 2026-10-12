#include "old_game/economy/mall/auction_runtime.h"

#include "old_data/db/auction_cashshop/purchase_cash_item_bonus_payload.h"
#include "old_game/economy/mall/auction_server_proxy_abi.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_data.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "common/manager/user_history_log_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream.h"

#include <cstring>

// IDA_EA: 0x84EDFDC
// IDA_NAME: _ZN37Inter_LoadAutoMarketConditionsControlC2Ev
// DEMANGLED: Inter_LoadAutoMarketConditionsControl::Inter_LoadAutoMarketConditionsControl(void)
Inter_LoadAutoMarketConditionsControl::Inter_LoadAutoMarketConditionsControl() {}

// IDA_EA: 0x84F8322
// IDA_NAME: _ZN37Inter_LoadAutoMarketConditionsControlD2Ev
// DEMANGLED: Inter_LoadAutoMarketConditionsControl::~Inter_LoadAutoMarketConditionsControl()
// IDA_EA: 0x84F8352
// IDA_NAME: _ZN37Inter_LoadAutoMarketConditionsControlD0Ev
// DEMANGLED: Inter_LoadAutoMarketConditionsControl::~Inter_LoadAutoMarketConditionsControl()
Inter_LoadAutoMarketConditionsControl::~Inter_LoadAutoMarketConditionsControl() {}

// IDA_EA: 0x84D891C
// IDA_NAME: _ZN37Inter_LoadAutoMarketConditionsControl12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadAutoMarketConditionsControl::dispatch_sig(CUser *,char *,int)
int Inter_LoadAutoMarketConditionsControl::dispatch_sig(CUser* user,
                                                        char* payload,
                                                        int payload_size) {
    (void)user;
    (void)payload_size;

    // The dispatcher ABI supplies a byte pointer, but this signal's old
    // Stream specialization is the exact typed 0x6C database record.  A local
    // typed copy preserves the old read-only business flow without retaining
    // a raw payload cast in the canonical provider.
    SIG_AUTO_MARKET_CONDITIONS_CONTROL record;
    std::memcpy(&record, payload, sizeof(record));

    CDataManager* data_manager = G_CDataManager();
    CAutoMarketConditionsControl* control =
        data_manager->GetAutoMarketContitionsControl();
    control->LoadDatabase(&record);
    return 0;
}

// IDA_EA: 0x84EDE38
// IDA_NAME: _ZN33Inter_AuctionNotifyAuctionServiceC2Ev
// DEMANGLED: Inter_AuctionNotifyAuctionService::Inter_AuctionNotifyAuctionService(void)
Inter_AuctionNotifyAuctionService::Inter_AuctionNotifyAuctionService() {}

// IDA_EA: 0x84F8718
// IDA_NAME: _ZN33Inter_AuctionNotifyAuctionServiceD2Ev
// DEMANGLED: Inter_AuctionNotifyAuctionService::~Inter_AuctionNotifyAuctionService()
// IDA_EA: 0x84F8748
// IDA_NAME: _ZN33Inter_AuctionNotifyAuctionServiceD0Ev
// DEMANGLED: Inter_AuctionNotifyAuctionService::~Inter_AuctionNotifyAuctionService()
Inter_AuctionNotifyAuctionService::~Inter_AuctionNotifyAuctionService() {}

// IDA_EA: 0x84D7AC8
// IDA_NAME: _ZN33Inter_AuctionNotifyAuctionService12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_AuctionNotifyAuctionService::dispatch_sig(CUser *,char *,int)
int Inter_AuctionNotifyAuctionService::dispatch_sig(CUser* user,
                                                    char* payload,
                                                    int payload_size) {
    (void)user;
    (void)payload_size;

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_AUCTION_NOTIFY_AUCTION_SERVICE);

    SIG_AUCTION_NOTIFY_AUCTION_SERVICE record;
    std::memcpy(&record, payload, sizeof(record));
    packet.put_byte(record.service_type);

    const bool running = record.running != 0;
    if (record.service_type == 1) {
        ::GlobalData::s_cera_auction_proxy->SetRunning(running);
    } else {
        ::GlobalData::s_auction_proxy->SetRunning(running);
    }

    packet.put_byte(running ? 1 : 0);
    packet.finalize(true);
    G_GameWorld()->send_all(packet);

    const char* service_name = record.service_type ? "Point" : "Auction";
    const char* running_text = running ? "opened" : "closed";
    cMyTrace trace(
        "virtual int Inter_AuctionNotifyAuctionService::dispatch_sig(CUser*, char*, int)",
        15850,
        0);
    trace("%s server %s! SEND SIG_AUCTION_NOTIFY_AUCTION_SERVICE TO ALL USERS",
          service_name,
          running_text);
    return 0;
}

// IDA_EA: 0x84EEAB0
// IDA_NAME: _ZN33Inter_UpdatePurchaseCashItemBonusC2Ev
// DEMANGLED: Inter_UpdatePurchaseCashItemBonus::Inter_UpdatePurchaseCashItemBonus(void)
Inter_UpdatePurchaseCashItemBonus::Inter_UpdatePurchaseCashItemBonus() {}

// IDA_EA: 0x84F6546
// IDA_NAME: _ZN33Inter_UpdatePurchaseCashItemBonusD2Ev
// DEMANGLED: Inter_UpdatePurchaseCashItemBonus::~Inter_UpdatePurchaseCashItemBonus()
// IDA_EA: 0x84F6576
// IDA_NAME: _ZN33Inter_UpdatePurchaseCashItemBonusD0Ev
// DEMANGLED: Inter_UpdatePurchaseCashItemBonus::~Inter_UpdatePurchaseCashItemBonus()
Inter_UpdatePurchaseCashItemBonus::~Inter_UpdatePurchaseCashItemBonus() {}

// IDA_EA: 0x84E42E0
// IDA_NAME: _ZN33Inter_UpdatePurchaseCashItemBonus12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_UpdatePurchaseCashItemBonus::dispatch_sig(CUser *,char *,int)
int Inter_UpdatePurchaseCashItemBonus::dispatch_sig(CUser* user,
                                                    char* payload,
                                                    int payload_size) {
    (void)payload_size;

    if (user->get_state() <= 2) {
        return 0;
    }

    SIG_PURCHASE_CASHITEM_BONUS record;
    std::memcpy(&record, payload, sizeof(record));
    if (record.purchase_count != 0) {
        CDataManager* data_manager = G_CDataManager();
        EventClassify::CEventScriptMng* event_script_mng =
            data_manager->get_event_script_mng();
        event_script_mng->send_purchase_bonus_mail(user, record.purchase_count);
    }

    return 0;
}

// IDA_EA: 0x84EDE1C
// IDA_NAME: _ZN23Inter_AuctionLogMessageC2Ev
// DEMANGLED: Inter_AuctionLogMessage::Inter_AuctionLogMessage(void)
Inter_AuctionLogMessage::Inter_AuctionLogMessage() {}

// IDA_EA: 0x84F8766
// IDA_NAME: _ZN23Inter_AuctionLogMessageD2Ev
// DEMANGLED: Inter_AuctionLogMessage::~Inter_AuctionLogMessage()
// IDA_EA: 0x84F8796
// IDA_NAME: _ZN23Inter_AuctionLogMessageD0Ev
// DEMANGLED: Inter_AuctionLogMessage::~Inter_AuctionLogMessage()
Inter_AuctionLogMessage::~Inter_AuctionLogMessage() {}

// IDA_EA: 0x84D7A90
// IDA_NAME: _ZN23Inter_AuctionLogMessage12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_AuctionLogMessage::dispatch_sig(CUser *,char *,int)
int Inter_AuctionLogMessage::dispatch_sig(CUser* user,
                                          char* payload,
                                          int payload_size) {
    (void)payload_size;

    if (user) {
        SIG_AUCTION_LOG_MESSAGE_AG record;
        std::memcpy(&record, payload, sizeof(record));
        CUser_history_log(user)->Auction(&record);
    }

    return 0;
}

// IDA_EA: 0x84EDE54
// IDA_NAME: _ZN28Inter_PointAuctionLogMessageC2Ev
// DEMANGLED: Inter_PointAuctionLogMessage::Inter_PointAuctionLogMessage(void)
Inter_PointAuctionLogMessage::Inter_PointAuctionLogMessage() {}

// IDA_EA: 0x84F86CA
// IDA_NAME: _ZN28Inter_PointAuctionLogMessageD2Ev
// DEMANGLED: Inter_PointAuctionLogMessage::~Inter_PointAuctionLogMessage()
// IDA_EA: 0x84F86FA
// IDA_NAME: _ZN28Inter_PointAuctionLogMessageD0Ev
// DEMANGLED: Inter_PointAuctionLogMessage::~Inter_PointAuctionLogMessage()
Inter_PointAuctionLogMessage::~Inter_PointAuctionLogMessage() {}

// IDA_EA: 0x84D7C4C
// IDA_NAME: _ZN28Inter_PointAuctionLogMessage12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_PointAuctionLogMessage::dispatch_sig(CUser *,char *,int)
int Inter_PointAuctionLogMessage::dispatch_sig(CUser* user,
                                               char* payload,
                                               int payload_size) {
    (void)payload_size;

    if (user) {
        SIG_AUCTION_LOG_MESSAGE_PG record;
        std::memcpy(&record, payload, sizeof(record));
        CUser_history_log(user)->AuctionPoint(&record);
    }

    return 0;
}

// IDA_EA: 0x84ED4D0
// IDA_NAME: _ZN19Inter_GenCeraTicketC2Ev
// DEMANGLED: Inter_GenCeraTicket::Inter_GenCeraTicket(void)
Inter_GenCeraTicket::Inter_GenCeraTicket() {}

// IDA_EA: 0x84F9EDC
// IDA_NAME: _ZN19Inter_GenCeraTicketD2Ev
// DEMANGLED: Inter_GenCeraTicket::~Inter_GenCeraTicket()
// IDA_EA: 0x84F9F0C
// IDA_NAME: _ZN19Inter_GenCeraTicketD0Ev
// DEMANGLED: Inter_GenCeraTicket::~Inter_GenCeraTicket() deleting destructor
Inter_GenCeraTicket::~Inter_GenCeraTicket() {}

// IDA_EA: 0x84CEEA2
// IDA_NAME: _ZN19Inter_GenCeraTicket12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_GenCeraTicket::dispatch_sig(CUser *,char *,int)
int Inter_GenCeraTicket::dispatch_sig(CUser* user,
                                      char* payload,
                                      int payload_size) {
    (void)payload_size;

    if (user) {
        SIG_GEN_CERA_TICKET response;
        std::memcpy(&response, payload, sizeof(response));

        // The separate local fixed field intentionally preserves the old
        // strncpy/strlen behavior, including its non-normalizing 32-byte
        // boundary behavior.
        CeraTicketResponseText ticket_text;
        std::strncpy(ticket_text.data(),
                     response.response_text.data(),
                     sizeof(ticket_text));

        PacketGuard packet;
        packet.put_header(1, ENUM_CMDPACKET_GEN_CERATICKET);
        packet.put_byte(1);
        const size_t first_length = std::strlen(ticket_text.data());
        packet.put_int(first_length);
        const size_t second_length = std::strlen(ticket_text.data());
        packet.put_str(ticket_text.data(), second_length);
        packet.put_int(response.response_value);
        packet.finalize(true);
        user->Send(packet);
    }

    return 0;
}

// IDA_EA: 0x84EEE4C
// IDA_NAME: _ZN21Inter_AuctionSubMoneyC2Ev
// DEMANGLED: Inter_AuctionSubMoney::Inter_AuctionSubMoney(void)
Inter_AuctionSubMoney::Inter_AuctionSubMoney() {}

// IDA_EA: 0x84F5980
// IDA_NAME: _ZN21Inter_AuctionSubMoneyD2Ev
// DEMANGLED: Inter_AuctionSubMoney::~Inter_AuctionSubMoney()
// IDA_EA: 0x84F59B0
// IDA_NAME: _ZN21Inter_AuctionSubMoneyD0Ev
// DEMANGLED: Inter_AuctionSubMoney::~Inter_AuctionSubMoney() deleting destructor
Inter_AuctionSubMoney::~Inter_AuctionSubMoney() {}

// IDA_EA: 0x84E8004
// IDA_NAME: _ZN21Inter_AuctionSubMoney12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_AuctionSubMoney::dispatch_sig(CUser *,char *,int)
int Inter_AuctionSubMoney::dispatch_sig(CUser* user,
                                        char* payload,
                                        int payload_size) {
    (void)payload_size;

    if (user) {
        SIG_AUCTION_SUB_MONEY signal;
        std::memcpy(&signal, payload, sizeof(signal));
        user->getHackAnalyzer()->addServerHackCnt(
            user,
            static_cast<WongWork::ENUM_HACKTYPE>(0x2D3u),
            1u,
            signal.hack_argument_0,
            signal.hack_argument_1);
    }
    return 0;
}

// IDA_EA: 0x84EDE70
// IDA_NAME: _ZN33Inter_PointAuctionChargeCeraPointC2Ev
// DEMANGLED: Inter_PointAuctionChargeCeraPoint::Inter_PointAuctionChargeCeraPoint(void)
Inter_PointAuctionChargeCeraPoint::Inter_PointAuctionChargeCeraPoint() {}

// IDA_EA: 0x84F867C
// IDA_NAME: _ZN33Inter_PointAuctionChargeCeraPointD2Ev
// DEMANGLED: Inter_PointAuctionChargeCeraPoint::~Inter_PointAuctionChargeCeraPoint()
// IDA_EA: 0x84F86AC
// IDA_NAME: _ZN33Inter_PointAuctionChargeCeraPointD0Ev
// DEMANGLED: Inter_PointAuctionChargeCeraPoint::~Inter_PointAuctionChargeCeraPoint() deleting destructor
Inter_PointAuctionChargeCeraPoint::~Inter_PointAuctionChargeCeraPoint() {}

// IDA_EA: 0x84D7C84
// IDA_NAME: _ZN33Inter_PointAuctionChargeCeraPoint12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_PointAuctionChargeCeraPoint::dispatch_sig(CUser *,char *,int)
int Inter_PointAuctionChargeCeraPoint::dispatch_sig(CUser*, char*, int) {
    return 0;
}

// IDA_EA: 0x84EDE8C
// IDA_NAME: _ZN32Inter_AuctionResultBuyItemApieceC2Ev
// DEMANGLED: Inter_AuctionResultBuyItemApiece::Inter_AuctionResultBuyItemApiece(void)
Inter_AuctionResultBuyItemApiece::Inter_AuctionResultBuyItemApiece() {}

// IDA_EA: 0x84F862E/0x84F865E
// IDA_NAME: _ZN32Inter_AuctionResultBuyItemApieceD2Ev
// DEMANGLED: Inter_AuctionResultBuyItemApiece::~Inter_AuctionResultBuyItemApiece()
Inter_AuctionResultBuyItemApiece::~Inter_AuctionResultBuyItemApiece() {}

// IDA_EA: 0x84D7C8E
// IDA_NAME: _ZN32Inter_AuctionResultBuyItemApiece12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_AuctionResultBuyItemApiece::dispatch_sig(CUser *,char *,int)
int Inter_AuctionResultBuyItemApiece::dispatch_sig(CUser* user,
                                                   char* payload,
                                                   int payload_size) {
    if (!user || !payload ||
        payload_size < static_cast<int>(sizeof(SIG_AUCTION_RESULT_BUY_ITEM_APIECE))) {
        return 0;
    }

    SIG_AUCTION_RESULT_BUY_ITEM_APIECE response;
    std::memcpy(&response, payload, sizeof(response));
    if (response.charac_no != user->getCurCharacNo()) {
        return 0;
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_AUCTION_BUY_ITEM_APIECE);
    packet.put_byte(response.result);

    if (response.result == 0) {
        const int gained_money = user->getCurCharacInvenW()->gain_money(
            static_cast<int>(response.refund_money),
            static_cast<eMoneyAddReason>(20),
            true,
            0);
        if (response.refund_money > static_cast<unsigned int>(gained_money)) {
            user->SendMoneyFullReason(static_cast<ENUM_MONEY_FULL_REASON>(0),
                                      response.refund_money,
                                      static_cast<unsigned long>(gained_money));
        }
        packet.put_byte(response.response_detail);
    }

    packet.finalize(true);
    user->Send(packet);
    return 0;
}
