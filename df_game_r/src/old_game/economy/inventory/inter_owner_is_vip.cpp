#include "old_game/economy/inventory/inter_owner_is_vip.h"

#include <cstring>

#include "old_game/economy/mall/auction_server_proxy_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/prepay_info_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/mall/auction_runtime.h"
#include "old_game/economy/mail/mail_trade_packets.h"
#include "old_game/economy/trade/item_vending_machine_auction_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"

// IDA_EA: 0x84EB4D4
// IDA_NAME: _ZN16Inter_OwnerisVIPC2Ev
// DEMANGLED: Inter_OwnerisVIP::Inter_OwnerisVIP(void)
Inter_OwnerisVIP::Inter_OwnerisVIP() : ISigDispatcher() {}

// IDA_EA: 0x84EB4F0 / 0x84EB520
// IDA_NAME: _ZN16Inter_OwnerisVIPD2Ev / _ZN16Inter_OwnerisVIPD0Ev
// DEMANGLED: Inter_OwnerisVIP::~Inter_OwnerisVIP()
Inter_OwnerisVIP::~Inter_OwnerisVIP() {}

// IDA_EA: 0x84E7C90
// IDA_NAME: _ZN16Inter_OwnerisVIP12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_OwnerisVIP::dispatch_sig(CUser *,char *,int)
int Inter_OwnerisVIP::dispatch_sig(CUser* user,
                                   char* payload,
                                   int payload_size) {
    if (!payload || payload_size <
                         static_cast<int>(sizeof(SIG_AUCTION_OWNER_IS_VIP))) {
        cMyTrace trace(
            "virtual int Inter_OwnerisVIP::dispatch_sig(CUser*, char*, int)",
            27302,
            5);
        trace("[Check point auction error] internal data is null.");
        return 27303;
    }
    if (!user) {
        return 0;
    }

    SIG_AUCTION_OWNER_IS_VIP result;
    std::memcpy(&result, payload, sizeof(result));

    ItemVendingMachine::AuctionNeedInfo need_info = {};
    need_info.bid_cera = result.bid_cera;
    need_info.auction_key_low = result.auction_key_low;
    need_info.auction_key_high = result.auction_key_high;

    const bool purchased = GlobalData::s_pItemVendingMachine &&
        GlobalData::s_pItemVendingMachine->BuyAuctionItem(
            user,
            need_info,
            result.billing_context,
            result.is_vip != 0,
            result.tax_divisor);
    if (!purchased) {
        user->SetBuyingGold(false);
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_BIDDING,
                                 147,
                                 static_cast<PAY_TYPE>(1));
        return 0;
    }

    PCK_AUCTION_BIDDING_GP request;
    request.user_uid = user->GetUID();
    request.charac_no = user->getCurCharacNo();
    std::strncpy(request.charac_name,
                 user->getCurCharacName(),
                 sizeof(request.charac_name));
    request.bid_cera = need_info.bid_cera;
    std::memcpy(request.key.bytes,
                &need_info.auction_key_low,
                sizeof(need_info.auction_key_low));
    std::memcpy(request.key.bytes + sizeof(need_info.auction_key_low),
                &need_info.auction_key_high,
                sizeof(need_info.auction_key_high));

    // Inter_AuctionResultBidding later owns confirmation/cancellation of
    // these two exact transactions.  Insert before the proxy send, matching
    // the native ordering, so a synchronous acknowledgement cannot race it.
    PrePayInfo prepay_info = {};
    prepay_info.auction_transaction_low = need_info.auction_key_low;
    prepay_info.auction_transaction_high = need_info.auction_key_high;
    prepay_info.billing_item_id = need_info.bid_cera;
    prepay_info.secondary_billing_item_id = need_info.auction_tax;
    prepay_info.primary_transaction = need_info.primary_transaction;
    prepay_info.secondary_transaction = need_info.secondary_transaction;
    G_CPrePayInfoMgr()->InsertPrePayInfo(user->getCurCharacNo(), prepay_info);

    GlobalData::s_cera_auction_proxy->SendPacket(&request);
    return 0;
}
