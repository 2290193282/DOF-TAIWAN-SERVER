#include "old_data/db/avatar_aura/db_update_avatar_history.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>


namespace {

enum {
    kDbSignalUpdateAvatarHistory = 688,
    kAvatarHistoryDbHandle = 3,
    kIpgAgencyNoBytes = 0x18,
};

}  // namespace

// IDA_EA: 0x844FA5E
// IDA_NAME: _ZN22DB_UpdateAvatarHistoryC2Ev
// DEMANGLED: DB_UpdateAvatarHistory::DB_UpdateAvatarHistory(void)
DB_UpdateAvatarHistory::DB_UpdateAvatarHistory() {}

// IDA_EA: 0x845B524
// IDA_NAME: _ZN22DB_UpdateAvatarHistoryD2Ev
// DEMANGLED: DB_UpdateAvatarHistory::~DB_UpdateAvatarHistory()
// IDA_EA: 0x845B554
// IDA_NAME: _ZN22DB_UpdateAvatarHistoryD0Ev
// DEMANGLED: DB_UpdateAvatarHistory::~DB_UpdateAvatarHistory() deleting destructor
DB_UpdateAvatarHistory::~DB_UpdateAvatarHistory() {}

// IDA_EA: 0x8445246
// IDA_NAME: _ZN22DB_UpdateAvatarHistory8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateAvatarHistory::dispatch(int,int,Stream *)
bool DB_UpdateAvatarHistory::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    unsigned int charac_no = 0;
    int avatar_ui_id = 0;
    char ipg_agency_no[kIpgAgencyNoBytes];
    std::memset(ipg_agency_no, 0, sizeof(ipg_agency_no));
    *stream >> charac_no;
    *stream >> avatar_ui_id;
    stream->get_binary(ipg_agency_no, sizeof(ipg_agency_no));

    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kAvatarHistoryDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "upDate user_items set to_ipg_agency_no='%s', m_time=now() where ui_id=%d and charac_no=%d",
        ipg_agency_no,
        avatar_ui_id,
        charac_no);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x844536A
// IDA_NAME: _ZN22DB_UpdateAvatarHistory11makeRequestEjiPc
// DEMANGLED: DB_UpdateAvatarHistory::makeRequest(unsigned int,int,char *)
void DB_UpdateAvatarHistory::makeRequest(unsigned int charac_no,
                                         int avatar_ui_id,
                                         char* ipg_agency_no) {
    Stream* stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 44091);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalUpdateAvatarHistory;
    *guard << -1;
    *guard << charac_no;
    *guard << avatar_ui_id;
    guard->put_binary(ipg_agency_no, kIpgAgencyNoBytes);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
