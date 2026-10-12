#include "old_game/economy/trade/item_vending_machine_auction_runtime.h"

#include <stdint.h>

#include "common/manager/global_data.h"
#include "common/manager/ipg_runtime.h"
#include "common/manager/user_history_log_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"

// These two CUser-history leaves are already the canonical restorations in
// auction_inter_return_dispatchers.cpp.  They retain their old symbols here
// instead of creating a second history-log object layout.
// CIPGMgr remains intentionally opaque.  This is its existing non-virtual
// original ABI provider, also used by the auction-result confirmation chain.
namespace {

uint64_t MakeAuctionRequestId(uint32_t low, uint32_t high) {
    return (static_cast<uint64_t>(high) << 32) | low;
}

void LogAuctionPurchaseFailure(CUser* user,
                               const Taiwan::BillTransaction& transaction,
                               int billing_item_id,
                               int billing_result,
                               int source_line) {
    (CUser_history_log(user))->BillingFail(
                            static_cast<ENUM_BILLING_TYPE>(1),
                            transaction.transaction_id,
                            billing_item_id,
                            billing_result,
                            "purchase");
    LogManager::logFormat(
        1,
        "localjapan/AppendVendingMachine.cpp",
        "bool ItemVendingMachine::BuyAuctionItem(CUser*, ItemVendingMachine::AuctionNeedInfo&, int, bool, int)",
        source_line,
        "IPG_OUTPUT user: %u, charac: %s reason: %d",
        user->getCurCharacNo(),
        user->getCurCharacName(),
        billing_result);
}

}  // namespace

// IDA_EA: 0x8175C5C
// IDA_NAME: _ZN18ItemVendingMachine14BuyAuctionItemEP5CUserRNS_15AuctionNeedInfoEibi
// DEMANGLED: ItemVendingMachine::BuyAuctionItem(CUser *,ItemVendingMachine::AuctionNeedInfo &,int,bool,int)
bool ItemVendingMachine::BuyAuctionItem(CUser* user,
                                        AuctionNeedInfo& need_info,
                                        int billing_context,
                                        bool is_vip,
                                        int tax_divisor) {
    // The original fourth argument is carried from SIG_AUCTION_OWNER_IS_VIP
    // but the restored leaf never consumes it.  Preserve its ABI position for
    // the caller and future packet-side auditing.
    (void)billing_context;

    if (!user || !GlobalData::s_pIPGHelper) {
        return false;
    }

    int tax_rate = is_vip ? 2 : 5;
    if (tax_divisor != 0) {
        const long double ratio =
            static_cast<long double>(need_info.bid_cera) /
            static_cast<long double>(tax_divisor);
        if (ratio < 0.5L) {
            return false;
        }
        if (ratio >= 1.5L) {
            // The original has three adjacent 1.6/1.7/1.8 comparisons but
            // only distinguishes the 80% tier at 1.8 and above.
            tax_rate = ratio >= 1.8L ? 80 : 50;
        }
    }

    need_info.auction_tax = static_cast<int>(
        (static_cast<long double>(need_info.bid_cera) / 100.0L) *
        static_cast<long double>(tax_rate));

    // This exact 0x28-byte local stream write is present in the native leaf
    // before either bill is attempted.  It is an auto-freed trace/scratch
    // record, not a queue submission.
    if (GlobalData::s_stream_pool) {
        CStreamGuard auction_record(
            GlobalData::s_stream_pool->Acquire(
                "localjapan/AppendVendingMachine.cpp", 123),
            true);
        auction_record.put_binary(&need_info, sizeof(need_info));
    }

    const uint64_t request_id = MakeAuctionRequestId(
        need_info.auction_key_low, need_info.auction_key_high);

    WongWork::IPG::CIPGMgr* ipg_manager =
        GlobalData::s_pIPGHelper->GetIPGMgr();
    Taiwan::TaiwanCash* billing_cash =
        ipg_manager ? ipg_manager->getBillingCash() : 0;
    if (!billing_cash) {
        return false;
    }

    int billing_result = billing_cash->purchase(
        request_id,
        100000,
        need_info.bid_cera - need_info.auction_tax,
        user->get_acc_name(),
        user->getCurCharacName(),
        need_info.primary_transaction);
    if (!CheckBillingErrorCode(billing_result)) {
        LogAuctionPurchaseFailure(user,
                                  need_info.primary_transaction,
                                  need_info.bid_cera,
                                  billing_result,
                                  165);
        return false;
    }

    (CUser_history_log(user))->BillingSuccess(
                               static_cast<ENUM_BILLING_TYPE>(1),
                               need_info.primary_transaction.transaction_id,
                               need_info.bid_cera);

    // The original re-fetches the opaque manager for the tax bill instead of
    // retaining an implementation-layout pointer across the first purchase.
    ipg_manager = GlobalData::s_pIPGHelper->GetIPGMgr();
    billing_cash = ipg_manager ? ipg_manager->getBillingCash() : 0;
    if (!billing_cash) {
        return false;
    }

    billing_result = billing_cash->purchase(
        request_id,
        99999,
        need_info.auction_tax,
        user->get_acc_name(),
        user->getCurCharacName(),
        need_info.secondary_transaction);
    if (!CheckBillingErrorCode(billing_result)) {
        LogAuctionPurchaseFailure(user,
                                  need_info.secondary_transaction,
                                  need_info.auction_tax,
                                  billing_result,
                                  232);
        return false;
    }

    (CUser_history_log(user))->BillingSuccess(
                               static_cast<ENUM_BILLING_TYPE>(1),
                               need_info.secondary_transaction.transaction_id,
                               need_info.auction_tax);
    return true;
}
