#include "old_data/db/auction_cashshop/db_auction_sub_money.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>
#include <string.h>



namespace {

enum {
    kDispatchAuctionMoneySubSignal = 777,
};

}  // namespace

// IDA_EA: 0x844FD1A
// IDA_NAME: _ZN18DB_AuctionSubMoneyC2Ev
// DEMANGLED: DB_AuctionSubMoney::DB_AuctionSubMoney(void)
DB_AuctionSubMoney::DB_AuctionSubMoney() {}

// IDA_EA: 0x845AFF6
// IDA_NAME: _ZN18DB_AuctionSubMoneyD2Ev
// DEMANGLED: DB_AuctionSubMoney::~DB_AuctionSubMoney()
// IDA_EA: 0x845B026
// IDA_NAME: _ZN18DB_AuctionSubMoneyD0Ev
// DEMANGLED: DB_AuctionSubMoney::~DB_AuctionSubMoney()
// deleting destructor
DB_AuctionSubMoney::~DB_AuctionSubMoney() {}

// IDA_EA: 0x8448530
// IDA_NAME: _ZN18DB_AuctionSubMoney8dispatchEiiP6Stream
// DEMANGLED: DB_AuctionSubMoney::dispatch(int,int,Stream *)
bool DB_AuctionSubMoney::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;

    if (!stream->enable_out(sizeof(SIG_AUCTION_MONEY_SUB))) {
        return false;
    }
    SIG_AUCTION_MONEY_SUB* request =
        reinterpret_cast<SIG_AUCTION_MONEY_SUB*>(stream->get_buffer());
    stream->out_ptr(sizeof(SIG_AUCTION_MONEY_SUB));

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, SERVER_GROUP_DEFAULT);
    char escaped_name[60];
    memset(escaped_name, 0, sizeof(escaped_name));
    db_handle->escape_string(escaped_name, request->charac_name);
    db_handle->set_query(
        "seLect m_id from charac_info where charac_name='%s' limit 1",
        escaped_name);
    if (db_handle->exec(true) != true || db_handle->fetch() != true) {
        return false;
    }

    unsigned int account_id = 0;
    if (db_handle->get_uint(0, account_id) != true) {
        return false;
    }

    Stream* result_stream = GlobalData::stream_pool()->Acquire(
        "DBThread.cpp",
        46417);
    CStreamGuard guard(result_stream, true);
    *guard << kDispatchAuctionMoneySubSignal;
    *guard << user_idx;
    SIG_AUCTION_MONEY_SUB* result =
        reinterpret_cast<SIG_AUCTION_MONEY_SUB*>(
            guard->GetInBufferUntyped(sizeof(SIG_AUCTION_MONEY_SUB)));
    if (!result) {
        return false;
    }
    result->account_id = account_id;
    result->money = request->money;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}
