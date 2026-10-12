#include "old_data/db/mail/db_req_query_charac_info_mailbox.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <string.h>


namespace {

enum {
    kDbSignalReqQueryCharacInfoMailbox = 502,
};

}  // namespace

// IDA_EA: 0x844F4AE
// IDA_NAME: _ZN28DB_ReqQueryCharacInfoMailboxC2Ev
// DEMANGLED: DB_ReqQueryCharacInfoMailbox::DB_ReqQueryCharacInfoMailbox(void)
DB_ReqQueryCharacInfoMailbox::DB_ReqQueryCharacInfoMailbox() {}

// IDA_EA: 0x845C412
// IDA_NAME: _ZN28DB_ReqQueryCharacInfoMailboxD2Ev
// DEMANGLED: DB_ReqQueryCharacInfoMailbox::~DB_ReqQueryCharacInfoMailbox()
// IDA_EA: 0x845C442
// IDA_NAME: _ZN28DB_ReqQueryCharacInfoMailboxD0Ev
// DEMANGLED: DB_ReqQueryCharacInfoMailbox::~DB_ReqQueryCharacInfoMailbox() deleting destructor
DB_ReqQueryCharacInfoMailbox::~DB_ReqQueryCharacInfoMailbox() {}

// IDA_EA: 0x843BD46
// IDA_NAME: _ZN28DB_ReqQueryCharacInfoMailbox8dispatchEiiP6Stream
// DEMANGLED: DB_ReqQueryCharacInfoMailbox::dispatch(int,int,Stream *)
bool DB_ReqQueryCharacInfoMailbox::dispatch(int signal,
                                            int user_idx,
                                            Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    SIG_QUERY_CHARAC_INFO* payload =
        stream->GetOutBuffer<SIG_QUERY_CHARAC_INFO>();
    if (!payload) {
        return false;
    }

    char escaped_name[60];
    memset(escaped_name, 0, sizeof(escaped_name));
    db_handle->escape_string(escaped_name, payload->charac_name);
    db_handle->set_query(
        "seLect delete_flag,m_id,charac_no,job,lev,grow_type,sex from charac_info where charac_name='%s'",
        escaped_name);
    if (db_handle->exec(true) != true) {
        return _sendResult(2, user_idx, 0, 0, 0, 0, 0, 0) != 0;
    }
    if (!db_handle->get_n_rows()) {
        return _sendResult(21, user_idx, 0, 0, 0, 0, 0, 0) != 0;
    }
    if (db_handle->get_n_rows()) {
        int delete_flag = 0;
        if (db_handle->fetch() != true ||
            db_handle->get_int(0, delete_flag) != true) {
            return _sendResult(2, user_idx, 0, 0, 0, 0, 0, 0) != 0;
        }
        if (delete_flag) {
            return _sendResult(21, user_idx, 0, 0, 0, 0, 0, 0) != 0;
        }

        int account_id = 0;
        int charac_no = 0;
        int job = 0;
        int level = 0;
        int grow_type = 0;
        int sex = 0;
        db_handle->get_int(1, account_id);
        db_handle->get_int(2, charac_no);
        db_handle->get_int(3, job);
        db_handle->get_int(4, level);
        db_handle->get_int(5, grow_type);
        db_handle->get_int(6, sex);
        (void)account_id;
        _sendResult(0,
                    user_idx,
                    payload->charac_name,
                    charac_no,
                    static_cast<char>(job),
                    static_cast<short>(level),
                    static_cast<char>(grow_type),
                    static_cast<char>(sex));
    } else {
        _sendResult(21, user_idx, 0, 0, 0, 0, 0, 0);
    }
    return true;
}

// IDA_EA: 0x843C138
// IDA_NAME: _ZN28DB_ReqQueryCharacInfoMailbox11makeRequestEiPKc
// DEMANGLED: DB_ReqQueryCharacInfoMailbox::makeRequest(int,char const*)
void DB_ReqQueryCharacInfoMailbox::makeRequest(int user_idx,
                                               const char* charac_name) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 36549);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalReqQueryCharacInfoMailbox;
    *guard << user_idx;
    SIG_QUERY_CHARAC_INFO* payload =
        guard->GetInBuffer<SIG_QUERY_CHARAC_INFO>();
    if (payload && charac_name) {
        strncpy(payload->charac_name, charac_name, 0x1Du);
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x843C224
// IDA_NAME: _ZN28DB_ReqQueryCharacInfoMailbox11_sendResultEiiPKciiiii
// DEMANGLED: DB_ReqQueryCharacInfoMailbox::_sendResult(int,int,char const*,int,int,int,int,int)
int DB_ReqQueryCharacInfoMailbox::_sendResult(int result,
                                              int user_idx,
                                              const char* charac_name,
                                              int charac_no,
                                              int job,
                                              int level,
                                              int grow_type,
                                              int sex) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 36566);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalReqQueryCharacInfoMailbox;
    *guard << user_idx;
    SIG_QUERY_CHARAC_INFO* payload =
        guard->GetInBuffer<SIG_QUERY_CHARAC_INFO>();
    if (!payload) {
        return 1;
    }
    payload->result = result;
    if (!result) {
        strncpy(payload->charac_name, charac_name, 0x1Du);
        payload->charac_no = static_cast<unsigned int>(charac_no);
        payload->job = static_cast<int8_t>(job);
        payload->level = static_cast<int16_t>(level);
        payload->grow_type = static_cast<int8_t>(grow_type);
        payload->sex = static_cast<int8_t>(sex);
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return 1;
}
