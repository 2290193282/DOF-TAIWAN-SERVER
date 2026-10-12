#include "old_data/db/auction_cashshop/db_load_auto_market_conditions_control.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <string.h>


namespace {

enum {
    kDbSignalLoadAutoMarketConditionsControl = 346,
    kAutoMarketDbHandle = 2,
};

}  // namespace

// IDA_EA: 0x844EE56
// IDA_NAME: _ZN34DB_LoadAutoMarketConditionsControlC2Ev
// DEMANGLED: DB_LoadAutoMarketConditionsControl::DB_LoadAutoMarketConditionsControl(void)
DB_LoadAutoMarketConditionsControl::DB_LoadAutoMarketConditionsControl() {}

// IDA_EA: 0x845D744
// IDA_NAME: _ZN34DB_LoadAutoMarketConditionsControlD2Ev
// DEMANGLED: DB_LoadAutoMarketConditionsControl::~DB_LoadAutoMarketConditionsControl()
// IDA_EA: 0x845D774
// IDA_NAME: _ZN34DB_LoadAutoMarketConditionsControlD0Ev
// DEMANGLED: DB_LoadAutoMarketConditionsControl::~DB_LoadAutoMarketConditionsControl() deleting destructor
DB_LoadAutoMarketConditionsControl::~DB_LoadAutoMarketConditionsControl() {}

// IDA_EA: 0x84335DC
// IDA_NAME: _ZN34DB_LoadAutoMarketConditionsControl8dispatchEiiP6Stream
// DEMANGLED: DB_LoadAutoMarketConditionsControl::dispatch(int,int,Stream *)
bool DB_LoadAutoMarketConditionsControl::dispatch(int signal,
                                                  int user_idx,
                                                  Stream* stream) {
    (void)signal;
    (void)user_idx;

    char load_from_event = 0;
    *stream >> load_from_event;

    SIG_AUTO_MARKET_CONDITIONS_CONTROL result;
    memset(&result, 0, sizeof(result));

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kAutoMarketDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect optimum_gold_supply, over_gold from auto_market_condition_ctrl limit 1");
    if (db_handle->exec(true) != true) {
        return false;
    }

    int row_count = db_handle->get_n_rows();
    if (row_count) {
        if (db_handle->fetch() != true ||
            db_handle->get_ulonglong(
                0, result.control_optimum_gold_supply) != true ||
            db_handle->get_ulonglong(
                1, result.control_over_gold) != true) {
            return false;
        }
    }

    db_handle->set_query(
        "seLect total_gold, auction_gold, optimum_gold_supply, over_gold, gold_phase, item_phase, durability_phase from auto_market_condition_ctrl_daily where occ_time >= DATE_SUB(CURDATE(), INTERVAL 1 DAY) order by occ_time limit 2");
    if (db_handle->exec(true) != true) {
        return false;
    }

    row_count = db_handle->get_n_rows();
    if (row_count != 2 && row_count != 1) {
        cMyTrace trace(
            "virtual bool DB_LoadAutoMarketConditionsControl::dispatch(int, int, Stream*)",
            31756,
            0);
        trace("DB_LoadAutoMarketConditionsControl error %d", row_count);
        return false;
    }

    SIG_AUTO_MARKET_CONDITIONS_CONTROL_DAILY* records[2] = {
        &result.previous_day,
        &result.current_day,
    };
    for (int index = 0; index < row_count && index <= 1; ++index) {
        if (db_handle->fetch() != true ||
            db_handle->get_ulonglong(0, records[index]->total_gold) != true ||
            db_handle->get_ulonglong(1, records[index]->auction_gold) != true ||
            db_handle->get_ulonglong(
                2, records[index]->optimum_gold_supply) != true ||
            db_handle->get_ulonglong(3, records[index]->over_gold) != true ||
            db_handle->get_int(4, records[index]->gold_phase) != true ||
            db_handle->get_int(5, records[index]->item_phase) != true ||
            db_handle->get_int(6, records[index]->durability_phase) != true) {
            return false;
        }
    }
    if (row_count == 1) {
        result.current_day = result.previous_day;
    }

    Stream* out_stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 31792);
    CStreamGuard guard(out_stream, true);
    *guard << kDbSignalLoadAutoMarketConditionsControl;
    *guard << -1;
    SIG_AUTO_MARKET_CONDITIONS_CONTROL* out_payload =
        guard->GetInBuffer<SIG_AUTO_MARKET_CONDITIONS_CONTROL>();
    memcpy(out_payload, &result, sizeof(result));
    out_payload->request_flag = static_cast<unsigned char>(load_from_event);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x8433ADA
// IDA_NAME: _ZN34DB_LoadAutoMarketConditionsControl11makeRequestEb
// DEMANGLED: DB_LoadAutoMarketConditionsControl::makeRequest(bool)
void DB_LoadAutoMarketConditionsControl::makeRequest(bool load_from_event) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 31811);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalLoadAutoMarketConditionsControl;
    *guard << -1;
    *guard << static_cast<unsigned char>(load_from_event);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
