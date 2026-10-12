#include "old_data/db/auth_security/db_count_of_human_certify.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


namespace {

enum {
    kDbSignalCountOfHumanCertify = 273,
    kHumanCertifyDbHandle = 6,
};

}  // namespace

// IDA_EA: 0x844E73A
// IDA_NAME: _ZN22DB_CountOfHumanCertifyC2Ev
// DEMANGLED: DB_CountOfHumanCertify::DB_CountOfHumanCertify(void)
DB_CountOfHumanCertify::DB_CountOfHumanCertify() {}

// IDA_EA: 0x845E854
// IDA_NAME: _ZN22DB_CountOfHumanCertifyD2Ev
// DEMANGLED: DB_CountOfHumanCertify::~DB_CountOfHumanCertify()
// IDA_EA: 0x845E884
// IDA_NAME: _ZN22DB_CountOfHumanCertifyD0Ev
// DEMANGLED: DB_CountOfHumanCertify::~DB_CountOfHumanCertify() deleting destructor
DB_CountOfHumanCertify::~DB_CountOfHumanCertify() {}

// IDA_EA: 0x84282A0
// IDA_NAME: _ZN22DB_CountOfHumanCertify8dispatchEiiP6Stream
// DEMANGLED: DB_CountOfHumanCertify::dispatch(int,int,Stream *)
bool DB_CountOfHumanCertify::dispatch(int signal,
                                      int user_idx,
                                      Stream* stream) {
    (void)signal;
    (void)user_idx;

    unsigned int account_id = 0;
    *stream >> account_id;

    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kHumanCertifyDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "upDate human_certify_try_count set count = count + 1 where m_id = %s",
        NumberToString(account_id, 0));
    if (db_handle->exec(true) == true && db_handle->getAffectedRowCount()) {
        return true;
    }

    db_handle->set_query(
        "inSert into human_certify_try_count(m_id, count) values(%s, 1)",
        NumberToString(account_id, 0));
    if (db_handle->exec(true) == true) {
        return true;
    }

    cMyTrace trace(
        "virtual bool DB_CountOfHumanCertify::dispatch(int, int, Stream*)",
        27085,
        5);
    trace("Update and Insert into HumanCertifyCount failed");
    return false;
}

// IDA_EA: 0x84283CA
// IDA_NAME: _ZN22DB_CountOfHumanCertify11makeRequestEj
// DEMANGLED: DB_CountOfHumanCertify::makeRequest(unsigned int)
void DB_CountOfHumanCertify::makeRequest(unsigned int account_id) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 27094);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalCountOfHumanCertify;
    *guard << -1;
    *guard << account_id;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
