#include "old_data/db/auction_cashshop/db_save_featured_goods_idx.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>
#include <string.h>



namespace {

enum {
    kDbSignalSaveFeaturedGoodsIdx = 82,
    kFeaturedGoodsDbHandle = 6,
};

#pragma pack(push, 1)
struct SIG_SAVE_FEATURED_GOODSIDX_LOCAL {
    uint32_t account_id;   // +0x00
    uint32_t charac_no;    // +0x04
    uint32_t event_id;     // +0x08
    uint32_t server_id;    // +0x0C
    uint32_t obtain_date;  // +0x10
};
#pragma pack(pop)

typedef char SIG_SAVE_FEATURED_GOODSIDX_LOCAL_Size_Must_Be_20[
    (sizeof(SIG_SAVE_FEATURED_GOODSIDX_LOCAL) == 20) ? 1 : -1];

}  // namespace

// IDA_EA: 0x844E0E2
// IDA_NAME: _ZN23DB_SaveFeaturedGoodsIdxC2Ev
// DEMANGLED: DB_SaveFeaturedGoodsIdx::DB_SaveFeaturedGoodsIdx(void)
DB_SaveFeaturedGoodsIdx::DB_SaveFeaturedGoodsIdx() {}

// IDA_EA: 0x845F9B2
// IDA_NAME: _ZN23DB_SaveFeaturedGoodsIdxD2Ev
// DEMANGLED: DB_SaveFeaturedGoodsIdx::~DB_SaveFeaturedGoodsIdx()
// IDA_EA: 0x845F9E2
// IDA_NAME: _ZN23DB_SaveFeaturedGoodsIdxD0Ev
// DEMANGLED: DB_SaveFeaturedGoodsIdx::~DB_SaveFeaturedGoodsIdx() deleting destructor
DB_SaveFeaturedGoodsIdx::~DB_SaveFeaturedGoodsIdx() {}

// IDA_EA: 0x841B210
// IDA_NAME: _ZN23DB_SaveFeaturedGoodsIdx8dispatchEiiP6Stream
// DEMANGLED: DB_SaveFeaturedGoodsIdx::dispatch(int,int,Stream *)
bool DB_SaveFeaturedGoodsIdx::dispatch(int signal,
                                       int user_idx,
                                       Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_SAVE_FEATURED_GOODSIDX_LOCAL payload;
    memset(&payload, 0, sizeof(payload));
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }

    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kFeaturedGoodsDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "inSert into dnf_event_entry(event_id,m_id,occ_date,server_id,charac_no,obtain_date) values(%d,%s,from_unixtime(%d),%d,%d,from_unixtime(%d))",
        payload.event_id,
        NumberToString(payload.account_id, 0),
        payload.obtain_date,
        payload.server_id,
        payload.charac_no,
        payload.obtain_date);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x841B2DA
// IDA_NAME: _ZN23DB_SaveFeaturedGoodsIdx11makeRequestEjiiii
// DEMANGLED: DB_SaveFeaturedGoodsIdx::makeRequest(unsigned int,int,int,int,int)
void DB_SaveFeaturedGoodsIdx::makeRequest(unsigned int account_id,
                                          int charac_no,
                                          int event_id,
                                          int obtain_date,
                                          int server_id) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 19963);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalSaveFeaturedGoodsIdx;
    *guard << -1;

    SIG_SAVE_FEATURED_GOODSIDX_LOCAL* payload =
        static_cast<SIG_SAVE_FEATURED_GOODSIDX_LOCAL*>(
            guard->GetInBufferUntyped(sizeof(SIG_SAVE_FEATURED_GOODSIDX_LOCAL)));
    if (payload) {
        payload->account_id = account_id;
        payload->charac_no = static_cast<uint32_t>(charac_no);
        payload->event_id = static_cast<uint32_t>(event_id);
        // Official 0627 makeRequest writes its fifth stack argument at
        // 0x841B3A5 (+0x0C server_id), then its fourth at 0x841B3BF
        // (+0x10 obtain_date).  SaveFeaturedIdx's original call order is
        // therefore (event_id, obtain_date, server_id).
        payload->server_id = static_cast<uint32_t>(server_id);
        payload->obtain_date = static_cast<uint32_t>(obtain_date);
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
