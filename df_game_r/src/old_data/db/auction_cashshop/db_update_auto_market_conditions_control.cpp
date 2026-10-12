#include "old_data/db/auction_cashshop/db_update_auto_market_conditions_control.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <string.h>


namespace {

enum {
    kDbSignalUpdateAutoMarketConditionsControl = 347,
    kAutoMarketDbHandle = 2,
};

}  // namespace

// IDA_EA: 0x844EE72
// IDA_NAME: _ZN36DB_UpdateAutoMarketConditionsControlC2Ev
// DEMANGLED: DB_UpdateAutoMarketConditionsControl::DB_UpdateAutoMarketConditionsControl(void)
DB_UpdateAutoMarketConditionsControl::DB_UpdateAutoMarketConditionsControl() {}

// IDA_EA: 0x845D6F6
// IDA_NAME: _ZN36DB_UpdateAutoMarketConditionsControlD2Ev
// DEMANGLED: DB_UpdateAutoMarketConditionsControl::~DB_UpdateAutoMarketConditionsControl()
// IDA_EA: 0x845D726
// IDA_NAME: _ZN36DB_UpdateAutoMarketConditionsControlD0Ev
// DEMANGLED: DB_UpdateAutoMarketConditionsControl::~DB_UpdateAutoMarketConditionsControl() deleting destructor
DB_UpdateAutoMarketConditionsControl::~DB_UpdateAutoMarketConditionsControl() {}

// IDA_EA: 0x8433BB8
// IDA_NAME: _ZN36DB_UpdateAutoMarketConditionsControl8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateAutoMarketConditionsControl::dispatch(int,int,Stream *)
bool DB_UpdateAutoMarketConditionsControl::dispatch(int signal,
                                                    int user_idx,
                                                    Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL* request =
        stream->GetOutBuffer<SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL>();
    if (!request) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kAutoMarketDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "upDate  auto_market_condition_ctrl_daily set optimum_gold_supply=%qd, over_gold=%qd, gold_phase=%d, item_phase=%d, durability_phase=%d where occ_time = CURDATE()",
        request->optimum_gold_supply,
        request->over_gold,
        request->gold_phase,
        request->item_phase,
        request->durability_phase);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (!db_handle->getAffectedRowCount()) {
        db_handle->set_query(
            "inSert into auto_market_condition_ctrl_daily(occ_time, total_gold, auction_gold, optimum_gold_supply, over_gold, gold_phase, item_phase, durability_phase) values(CURDATE(),%qd,%qd,%qd,%qd,%d,%d,%d)",
            request->total_gold,
            request->auction_gold,
            request->optimum_gold_supply,
            request->over_gold,
            request->gold_phase,
            request->item_phase,
            request->durability_phase);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x8433D70
// IDA_NAME: _ZN36DB_UpdateAutoMarketConditionsControl11makeRequestEyyyyiii
// DEMANGLED: DB_UpdateAutoMarketConditionsControl::makeRequest(unsigned long long,unsigned long long,unsigned long long,unsigned long long,int,int,int)
void DB_UpdateAutoMarketConditionsControl::makeRequest(
    uint64_t total_gold,
    uint64_t auction_gold,
    uint64_t over_gold,
    uint64_t optimum_gold_supply,
    int gold_phase,
    int item_phase,
    int durability_phase) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 31865);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalUpdateAutoMarketConditionsControl;
    *guard << -1;

    SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL payload;
    memset(&payload, 0, sizeof(payload));
    payload.total_gold = total_gold;
    payload.auction_gold = auction_gold;
    payload.over_gold = over_gold;
    payload.optimum_gold_supply = optimum_gold_supply;
    payload.gold_phase = gold_phase;
    payload.item_phase = item_phase;
    payload.durability_phase = durability_phase;
    guard->put_binary(&payload, sizeof(payload));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
