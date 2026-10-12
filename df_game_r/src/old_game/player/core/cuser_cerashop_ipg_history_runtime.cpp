#include "old_game/player/core/cuser.h"

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_data/db/legacy_dispatch/db_sr08_leaf_dispatch_payloads.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

// IDA_EA: 0x86848DA
// IDA_NAME: _ZN15cUserHistoryLog23SendMailForCeraShopGiftEjcjj
// DEMANGLED: cUserHistoryLog::SendMailForCeraShopGift(unsigned int,char,unsigned int,unsigned int)
int cUserHistoryLog::SendMailForCeraShopGift(
    unsigned int receiver_charac_no,
    char gift_kind,
    unsigned int item_id,
    unsigned int item_count) {
    return owner_->LogHistory("MailSG,%u,%d,%u,%u",
                              receiver_charac_no,
                              static_cast<int>(gift_kind),
                              item_id,
                              item_count);
}

// IDA_EA: 0x868593C
// IDA_NAME: _ZN15cUserHistoryLog9CeraPointE23ENUM_CERAPOINT_LOG_TYPEj
// DEMANGLED: cUserHistoryLog::CeraPoint(ENUM_CERAPOINT_LOG_TYPE,unsigned int)
int cUserHistoryLog::CeraPoint(ENUM_CERAPOINT_LOG_TYPE log_type,
                               unsigned int amount) {
    return owner_->LogHistory("CeraPoint,%d,%d,%d",
                              static_cast<int>(log_type),
                              amount,
                              owner_->GetCeraPoint());
}

// IDA_EA: 0x8686EA0
// IDA_NAME: _ZN15cUserHistoryLog15BuyCashShopItemEjmjPKccN19CeraShopPaymentType1TEy
// DEMANGLED: cUserHistoryLog::BuyCashShopItem(unsigned int,unsigned long,unsigned int,char const*,char,CeraShopPaymentType::T,unsigned long long)
int cUserHistoryLog::BuyCashShopItem(
    unsigned int goods_no,
    unsigned long item_id,
    unsigned int item_count,
    const char* order_id,
    char gift,
    CeraShopPaymentType::T payment_type,
    unsigned long long transaction_id) {
    return owner_->LogHistory("BuyCashItem, %u, %u, %u, %s, %d, %d, %s",
                              goods_no,
                              item_id,
                              item_count,
                              order_id,
                              static_cast<int>(gift),
                              static_cast<int>(payment_type),
                              NumberToString(transaction_id, 0));
}

// IDA_EA: 0x868A87A
// IDA_NAME: _ZN5CUser28SendCeraShopGiftStatisticLogEjjmj
// DEMANGLED: CUser::SendCeraShopGiftStatisticLog(unsigned int,unsigned int,unsigned long,unsigned int)
void CUser::SendCeraShopGiftStatisticLog(unsigned int receiver_member_id,
                                         unsigned int receiver_charac_no,
                                         unsigned long item_id,
                                         unsigned int item_count) {
    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire("user.cpp", 41664), true);
    guard << 668;
    guard << GetUID();

    SIG_STATISTIC_CERASHOP_GIFT_LOG* const payload =
        guard->GetInBuffer<SIG_STATISTIC_CERASHOP_GIFT_LOG>();
    payload->from_member_id = static_cast<unsigned int>(get_acc_id());
    payload->from_charac_no = static_cast<unsigned int>(getCurCharacNo());
    payload->to_member_id = receiver_member_id;
    payload->to_charac_no = receiver_charac_no;
    payload->item_index = static_cast<unsigned int>(item_id);
    payload->item_count = item_count;

    GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, guard);
}
