#include "old_data/db/auth_security/db_cancel_resting_user_restrict.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <stdint.h>
#include <string.h>
#include <time.h>



namespace {

enum {
    kDbSignalCancelRestingUserRestrict = 405,
    kInterSignalCancelRestingUser = 405
};

const char kCancelRestingUserReason[] =
    "\x7c\x2a\x7c\xc8\xde\xb8\xe9\x20\xc1\xa6\xc0\xe7\x20\xc7"
    "\xd8\xc1\xa6\x7c\x2a\x7c\xba\xb8\xbe\xc8\xc6\xc0";

}  // namespace

// IDA_EA: 0x844F04E
// IDA_NAME: _ZN28DB_CancelRestingUserRestrictC2Ev
// DEMANGLED: DB_CancelRestingUserRestrict::DB_CancelRestingUserRestrict(void)
DB_CancelRestingUserRestrict::DB_CancelRestingUserRestrict() {}

// IDA_EA: 0x845D1C8
// IDA_NAME: _ZN28DB_CancelRestingUserRestrictD2Ev
// DEMANGLED: DB_CancelRestingUserRestrict::~DB_CancelRestingUserRestrict()
// IDA_EA: 0x845D1F8
// IDA_NAME: _ZN28DB_CancelRestingUserRestrictD0Ev
// DEMANGLED: DB_CancelRestingUserRestrict::~DB_CancelRestingUserRestrict() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_CancelRestingUserRestrict::~DB_CancelRestingUserRestrict() {}

// IDA_EA: 0x843712E
// IDA_NAME: _ZN28DB_CancelRestingUserRestrict8dispatchEiiP6Stream
// DEMANGLED: DB_CancelRestingUserRestrict::dispatch(int,int,Stream *)
bool DB_CancelRestingUserRestrict::dispatch(int signal,
                                            int user_idx,
                                            Stream* stream) {
    (void)signal;
    if (!stream) {
        return false;
    }

    SIG_RESTING_USER_RESTRICT* payload =
        stream->GetOutBuffer<SIG_RESTING_USER_RESTRICT>();

    MySQL* db_handle =
        GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1),static_cast<ENUM_SERVER_GROUP>( 0)) : 0;
    if (!db_handle) {
        return false;
    }
    db_handle->set_query(
        "upDate member_punish_info set apply_flag = 0 where m_id = %s and punish_type = 10",
        NumberToString(payload->account_id, 0));
    if (db_handle->exec(true) != true) {
        return false;
    }

    Stream* out_stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 33628);
    CStreamGuard guard(out_stream, true);
    *guard << kInterSignalCancelRestingUser;
    *guard << user_idx;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);

    char year[10];
    memset(year, 0, sizeof(year));
    time_t timer = GlobalData::system_time()
                       ? GlobalData::system_time()->getCurSec()
                       : time(0);
    struct tm tp;
    localtime_r(&timer, &tp);
    strftime(year, sizeof(year), "%Y", &tp);

    db_handle = GlobalData::s_db_mgr ? GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1),static_cast<ENUM_SERVER_GROUP>( 0))
                                    : 0;
    if (!db_handle) {
        return false;
    }
    db_handle->set_query(
        "upDate member_punish_info_history_%s set apply_flag = 0 , reason = '%s' where m_id = %s and punish_type = %d and apply_flag = 1",
        year,
        kCancelRestingUserReason,
        NumberToString(payload->account_id, 0),
        10);
    if (db_handle->exec(true) == true) {
        cMyTrace trace(
            "virtual bool DB_CancelRestingUserRestrict::dispatch(int, int, Stream*)",
            33665,
            0);
        trace("CANCEL OK - Resting User Restrict (mid:%s)",
              NumberToString(payload->account_id, 0));
        return true;
    }
    return false;
}

// IDA_EA: 0x843741E
// IDA_NAME: _ZN28DB_CancelRestingUserRestrict11makeRequestEji
// DEMANGLED: DB_CancelRestingUserRestrict::makeRequest(unsigned int,int)
void DB_CancelRestingUserRestrict::makeRequest(unsigned int account_id,
                                               int user_idx) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 33673);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalCancelRestingUserRestrict;
    *guard << user_idx;

    SIG_RESTING_USER_RESTRICT* payload =
        guard->GetInBuffer<SIG_RESTING_USER_RESTRICT>();
    if (!payload) {
        return;
    }
    payload->account_id = account_id;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
