#include "old_data/db/auth_security/db_security_card_update_validity_time.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>



#pragma pack(push, 1)
struct SIG_SECURITY_CARD_UPDATE_VALIDITYTIME {
    uint32_t account_id;  // +0
    uint32_t seconds;     // +4
};
#pragma pack(pop)

typedef char SIG_SECURITY_CARD_UPDATE_VALIDITYTIME_Size_Must_Be_8[
    (sizeof(SIG_SECURITY_CARD_UPDATE_VALIDITYTIME) == 8) ? 1 : -1];

// IDA_EA: 0x844E916
// IDA_NAME: _ZN33DB_SecurityCardUpdateValidityTimeC2Ev
// DEMANGLED: DB_SecurityCardUpdateValidityTime::DB_SecurityCardUpdateValidityTime(void)
DB_SecurityCardUpdateValidityTime::DB_SecurityCardUpdateValidityTime() {}

// IDA_EA: 0x845E374
// IDA_NAME: _ZN33DB_SecurityCardUpdateValidityTimeD2Ev
// DEMANGLED: DB_SecurityCardUpdateValidityTime::~DB_SecurityCardUpdateValidityTime()
// IDA_EA: 0x845E3A4
// IDA_NAME: _ZN33DB_SecurityCardUpdateValidityTimeD0Ev
// DEMANGLED: DB_SecurityCardUpdateValidityTime::~DB_SecurityCardUpdateValidityTime() deleting destructor
DB_SecurityCardUpdateValidityTime::~DB_SecurityCardUpdateValidityTime() {}

// IDA_EA: 0x842BC64
// IDA_NAME: _ZN33DB_SecurityCardUpdateValidityTime8dispatchEiiP6Stream
// DEMANGLED: DB_SecurityCardUpdateValidityTime::dispatch(int,int,Stream *)
bool DB_SecurityCardUpdateValidityTime::dispatch(int signal,
                                                 int user_idx,
                                                 Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_SECURITY_CARD_UPDATE_VALIDITYTIME payload;
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "upDate member_security_card set validity_time=unix_timestamp(now())+%d where m_id=%s and apply_flag=1",
        payload.seconds,
        NumberToString(payload.account_id, 0));
    db_handle->exec(true);
    return true;
}

// IDA_EA: 0x842BCE8
// IDA_NAME: _ZN33DB_SecurityCardUpdateValidityTime11makeRequestEjj
// DEMANGLED: DB_SecurityCardUpdateValidityTime::makeRequest(unsigned int,unsigned int)
void DB_SecurityCardUpdateValidityTime::makeRequest(unsigned int account_id,
                                                    unsigned int seconds) {
    if (!account_id) {
        return;
    }

    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 28347);
    CStreamGuard guard(stream, true);
    *guard << 256;
    *guard << -1;

    SIG_SECURITY_CARD_UPDATE_VALIDITYTIME* payload =
        static_cast<SIG_SECURITY_CARD_UPDATE_VALIDITYTIME*>(
            guard->GetInBufferUntyped(
                sizeof(SIG_SECURITY_CARD_UPDATE_VALIDITYTIME)));
    payload->account_id = account_id;
    payload->seconds = seconds;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
