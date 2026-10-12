#include "old_data/db/auction_cashshop/db_save_premium_info.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"

#include <stdint.h>

namespace {

enum {
    kSigSavePremiumInfoSize = 24,
};

#define DB_SAVE_PREMIUM_U32(payload, offset) \
    (*reinterpret_cast<uint32_t*>((payload) + (offset)))

}  // namespace

// IDA_EA: 0x844E136
// IDA_NAME: _ZN18DB_SavePremiumInfoC2Ev
// DEMANGLED: DB_SavePremiumInfo::DB_SavePremiumInfo(void)
DB_SavePremiumInfo::DB_SavePremiumInfo() {}

// IDA_EA: 0x845F916
// IDA_NAME: _ZN18DB_SavePremiumInfoD2Ev
// DEMANGLED: DB_SavePremiumInfo::~DB_SavePremiumInfo()
// IDA_EA: 0x845F946
// IDA_NAME: _ZN18DB_SavePremiumInfoD0Ev
// DEMANGLED: DB_SavePremiumInfo::~DB_SavePremiumInfo() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_SavePremiumInfo::~DB_SavePremiumInfo() {}

// IDA_EA: 0x841B4BE
// IDA_NAME: _ZN18DB_SavePremiumInfo8dispatchEiiP6Stream
// DEMANGLED: DB_SavePremiumInfo::dispatch(int,int,Stream *)
bool DB_SavePremiumInfo::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    unsigned char payload[kSigSavePremiumInfoSize];
    if (stream->get_binary(payload, sizeof(payload)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6), static_cast<ENUM_SERVER_GROUP>(0));
    const uint32_t account_id = DB_SAVE_PREMIUM_U32(payload, 0);
    const uint32_t pre_type = DB_SAVE_PREMIUM_U32(payload, 4);
    const uint32_t service_start = DB_SAVE_PREMIUM_U32(payload, 8);
    const uint32_t service_end = DB_SAVE_PREMIUM_U32(payload, 12);
    const uint32_t event_id = DB_SAVE_PREMIUM_U32(payload, 16);
    const uint32_t server_id = DB_SAVE_PREMIUM_U32(payload, 20);
    const char* account_text = NumberToString(account_id, 0);

    db_handle->set_query(
        "upDate member_premium set service_end=from_unixtime(%d) where event_id=%d and pre_type=%d and m_id=%s and service_end>from_unixtime(%d) and server_id=%d",
        service_end,
        event_id,
        pre_type,
        account_text,
        service_start,
        server_id);

    bool need_insert = true;
    if (db_handle->exec(true) == true && db_handle->getAffectedRowCount()) {
        need_insert = false;
    }
    if (!need_insert) {
        return true;
    }

    account_text = NumberToString(account_id, 0);
    db_handle->set_query(
        "inSert into member_premium(pre_type,m_id,service_start,service_end,event_id,server_id) values(%d,%s,from_unixtime(%d),from_unixtime(%d),%d,%d)",
        pre_type,
        account_text,
        service_start,
        service_end,
        event_id,
        server_id);
    return db_handle->exec(true) == true;
}

#undef DB_SAVE_PREMIUM_U32
