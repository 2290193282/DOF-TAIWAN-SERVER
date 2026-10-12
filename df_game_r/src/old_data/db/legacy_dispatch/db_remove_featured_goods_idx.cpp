#include "old_data/db/auction_cashshop/db_remove_featured_goods_idx.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"

#include <stdint.h>
#include <string.h>

namespace {

enum {
    kFeaturedGoodsDbHandle = 6,
};

#pragma pack(push, 1)
struct SIG_REMOVE_FEATURED_GOODSIDX_LOCAL {
    uint32_t account_id;  // +0x00
    uint32_t charac_no;   // +0x04, present in old payload but unused here.
    uint32_t event_id;    // +0x08
    uint32_t server_id;   // +0x0C
};
#pragma pack(pop)

typedef char SIG_REMOVE_FEATURED_GOODSIDX_LOCAL_Size_Must_Be_16[
    (sizeof(SIG_REMOVE_FEATURED_GOODSIDX_LOCAL) == 16) ? 1 : -1];

}  // namespace

// IDA_EA: 0x844E0FE
// IDA_NAME: _ZN25DB_RemoveFeaturedGoodsIdxC2Ev
// DEMANGLED: DB_RemoveFeaturedGoodsIdx::DB_RemoveFeaturedGoodsIdx(void)
DB_RemoveFeaturedGoodsIdx::DB_RemoveFeaturedGoodsIdx() {}

// IDA_EA: 0x845F964
// IDA_NAME: _ZN25DB_RemoveFeaturedGoodsIdxD2Ev
// DEMANGLED: DB_RemoveFeaturedGoodsIdx::~DB_RemoveFeaturedGoodsIdx()
// IDA_EA: 0x845F994
// IDA_NAME: _ZN25DB_RemoveFeaturedGoodsIdxD0Ev
// DEMANGLED: DB_RemoveFeaturedGoodsIdx::~DB_RemoveFeaturedGoodsIdx() deleting destructor
DB_RemoveFeaturedGoodsIdx::~DB_RemoveFeaturedGoodsIdx() {}

// IDA_EA: 0x841B41A
// IDA_NAME: _ZN25DB_RemoveFeaturedGoodsIdx8dispatchEiiP6Stream
// DEMANGLED: DB_RemoveFeaturedGoodsIdx::dispatch(int,int,Stream *)
bool DB_RemoveFeaturedGoodsIdx::dispatch(int signal,
                                         int user_idx,
                                         Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_REMOVE_FEATURED_GOODSIDX_LOCAL payload;
    memset(&payload, 0, sizeof(payload));
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }

    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(
            static_cast<ENUM_DB_HANDLE_IDX>(kFeaturedGoodsDbHandle),
            static_cast<ENUM_SERVER_GROUP>(0));
    db_handle->set_query(
        "deLete from dnf_event_entry where m_id=%s and event_id=%d and server_id=%d",
        NumberToString(payload.account_id, 0),
        payload.event_id,
        payload.server_id);
    return db_handle->exec(true) == true;
}
