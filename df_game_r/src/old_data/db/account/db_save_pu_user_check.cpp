#include "old_data/db/account/db_save_pu_user_check.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


namespace {

enum {
    kDbSignalSavePuUserCheck = 738,
    kDbHandlePuUser = 1,
};

}  // namespace

// IDA_EA: 0x844FC1E
// IDA_NAME: _ZN18DB_SavePuUserCheckC2Ev
// DEMANGLED: DB_SavePuUserCheck::DB_SavePuUserCheck(void)
DB_SavePuUserCheck::DB_SavePuUserCheck() {}

// IDA_EA: 0x845A9FA
// IDA_NAME: _ZN18DB_SavePuUserCheckD2Ev
// DEMANGLED: DB_SavePuUserCheck::~DB_SavePuUserCheck()
// IDA_EA: 0x845AA2A
// IDA_NAME: _ZN18DB_SavePuUserCheckD0Ev
// DEMANGLED: DB_SavePuUserCheck::~DB_SavePuUserCheck() deleting destructor
DB_SavePuUserCheck::~DB_SavePuUserCheck() {}

// IDA_EA: 0x844C098
// IDA_NAME: _ZN18DB_SavePuUserCheck11makeRequestEj
// DEMANGLED: DB_SavePuUserCheck::makeRequest(unsigned int)
void DB_SavePuUserCheck::makeRequest(unsigned int m_id) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 48372);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalSavePuUserCheck;
    *guard << -1;
    *guard << m_id;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x844BFB8
// IDA_NAME: _ZN18DB_SavePuUserCheck8dispatchEiiP6Stream
// DEMANGLED: DB_SavePuUserCheck::dispatch(int,int,Stream *)
bool DB_SavePuUserCheck::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kDbHandlePuUser),static_cast<ENUM_SERVER_GROUP>( 0));
    if (db_handle == 0) {
        return false;
    }

    unsigned int m_id = 0;
    *stream >> m_id;

    db_handle->set_query("seLect m_id from pu_user_list where m_id = %u",
                         m_id);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->get_n_rows()) {
        return true;
    }

    db_handle->set_query("inSert into pu_user_list(m_id) values(%u)", m_id);
    return db_handle->exec(true) == true;
}
