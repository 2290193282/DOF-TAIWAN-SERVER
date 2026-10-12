#include "old_data/db/account/db_load_pu_user_check.h"

#include <string.h>

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"



namespace {

enum {
    kDbSignalLoadPuUserCheck = 737,
    kDbHandlePuUser = 1,
};

#pragma pack(push, 1)
struct SIG_LOAD_PU_USER_INFO {
    unsigned char found;  // +0x00
};
#pragma pack(pop)

typedef char SIG_LOAD_PU_USER_INFO_Size_Must_Be_1[
    (sizeof(SIG_LOAD_PU_USER_INFO) == 1) ? 1 : -1];

}  // namespace

// IDA_EA: 0x844FC02
// IDA_NAME: _ZN18DB_LoadPuUserCheckC2Ev
// DEMANGLED: DB_LoadPuUserCheck::DB_LoadPuUserCheck(void)
DB_LoadPuUserCheck::DB_LoadPuUserCheck() {}

// IDA_EA: 0x845AA48
// IDA_NAME: _ZN18DB_LoadPuUserCheckD2Ev
// DEMANGLED: DB_LoadPuUserCheck::~DB_LoadPuUserCheck()
// IDA_EA: 0x845AA78
// IDA_NAME: _ZN18DB_LoadPuUserCheckD0Ev
// DEMANGLED: DB_LoadPuUserCheck::~DB_LoadPuUserCheck() deleting destructor
DB_LoadPuUserCheck::~DB_LoadPuUserCheck() {}

// IDA_EA: 0x844BEE2
// IDA_NAME: _ZN18DB_LoadPuUserCheck11makeRequestEji
// DEMANGLED: DB_LoadPuUserCheck::makeRequest(unsigned int,int)
void DB_LoadPuUserCheck::makeRequest(unsigned int m_id, int user_idx) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 48322);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalLoadPuUserCheck;
    *guard << user_idx;
    *guard << m_id;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x844BD5C
// IDA_NAME: _ZN18DB_LoadPuUserCheck8dispatchEiiP6Stream
// DEMANGLED: DB_LoadPuUserCheck::dispatch(int,int,Stream *)
bool DB_LoadPuUserCheck::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;

    unsigned int m_id = 0;
    if (stream->get_binary(&m_id, sizeof(m_id)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kDbHandlePuUser),static_cast<ENUM_SERVER_GROUP>( 0));
    if (db_handle == 0) {
        return false;
    }

    db_handle->set_query("seLect m_id from pu_user_list where m_id = %u",
                         m_id);
    if (db_handle->exec(true) != true) {
        return false;
    }

    SIG_LOAD_PU_USER_INFO response;
    memset(&response, 0, sizeof(response));
    if (db_handle->get_n_rows()) {
        response.found = 1;
    }

    Stream* out_stream = GlobalData::stream_pool()->Acquire("DBThread.cpp",
                                                            48301);
    CStreamGuard guard(out_stream, true);
    *guard << kDbSignalLoadPuUserCheck;
    *guard << user_idx;
    guard->put_binary(&response, sizeof(response));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}
