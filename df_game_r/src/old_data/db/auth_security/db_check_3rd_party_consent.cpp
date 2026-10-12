#include "old_data/db/auth_security/db_check_3rd_party_consent.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


namespace {

enum {
    kDbSignalCheck3rdPartyConcent = 695,
    kCheck3rdPartyConcentDbHandle = 1,
};

}  // namespace

// IDA_EA: 0x844FACE
// IDA_NAME: _ZN23DB_Check3rdPartyConcentC2Ev
// DEMANGLED: DB_Check3rdPartyConcent::DB_Check3rdPartyConcent(void)
DB_Check3rdPartyConcent::DB_Check3rdPartyConcent() {}

// IDA_EA: 0x845B3EC
// IDA_NAME: _ZN23DB_Check3rdPartyConcentD2Ev
// DEMANGLED: DB_Check3rdPartyConcent::~DB_Check3rdPartyConcent()
// IDA_EA: 0x845B41C
// IDA_NAME: _ZN23DB_Check3rdPartyConcentD0Ev
// DEMANGLED: DB_Check3rdPartyConcent::~DB_Check3rdPartyConcent() deleting destructor
DB_Check3rdPartyConcent::~DB_Check3rdPartyConcent() {}

// IDA_EA: 0x8445EEE
// IDA_NAME: _ZN23DB_Check3rdPartyConcent8dispatchEiiP6Stream
// DEMANGLED: DB_Check3rdPartyConcent::dispatch(int,int,Stream *)
bool DB_Check3rdPartyConcent::dispatch(int signal,
                                       int user_idx,
                                       Stream* stream) {
    (void)signal;

    unsigned int account_id = 0;
    *stream >> account_id;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(kCheck3rdPartyConcentDbHandle),
        static_cast<ENUM_SERVER_GROUP>(0));
    db_handle->set_query(
        "seLect 3dparty_consent from member_join_info where m_id = %s",
        NumberToString(account_id, 0));
    if (db_handle->exec(true) != true) {
        return false;
    }

    const int row_count = db_handle->get_n_rows();
    unsigned char consent = 0;
    if (row_count > 0) {
        if (db_handle->fetch() != true) {
            return false;
        }
        if (db_handle->get_ubyte(0, consent) != true) {
            return false;
        }
    }

    Stream* const out_stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 44433);
    CStreamGuard guard(out_stream, true);
    *guard << kDbSignalCheck3rdPartyConcent;
    *guard << user_idx;
    *guard << consent;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x84460C0
// IDA_NAME: _ZN23DB_Check3rdPartyConcent11makeRequestEij
// DEMANGLED: DB_Check3rdPartyConcent::makeRequest(int,unsigned int)
void DB_Check3rdPartyConcent::makeRequest(int user_idx,
                                          unsigned int account_id) {
    Stream* const stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 44449);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalCheck3rdPartyConcent;
    *guard << user_idx;
    *guard << account_id;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
