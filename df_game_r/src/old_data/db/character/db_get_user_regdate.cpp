#include "old_data/db/character/db_get_user_regdate.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


// IDA_EA: 0x844F92A
// IDA_NAME: _ZN17DB_GetUserRegdateC2Ev
// DEMANGLED: DB_GetUserRegdate::DB_GetUserRegdate(void)
DB_GetUserRegdate::DB_GetUserRegdate() {}

// IDA_EA: 0x845B8CC
// IDA_NAME: _ZN17DB_GetUserRegdateD2Ev
// DEMANGLED: DB_GetUserRegdate::~DB_GetUserRegdate()
// IDA_EA: 0x845B8FC
// IDA_NAME: _ZN17DB_GetUserRegdateD0Ev
// DEMANGLED: DB_GetUserRegdate::~DB_GetUserRegdate() deleting destructor
DB_GetUserRegdate::~DB_GetUserRegdate() {}

// IDA_EA: 0x8442E68
// IDA_NAME: _ZN17DB_GetUserRegdate8dispatchEiiP6Stream
// DEMANGLED: DB_GetUserRegdate::dispatch(int,int,Stream *)
bool DB_GetUserRegdate::dispatch(int signal, int user_idx, Stream* stream) {
    SIG_GET_USER_REGDATE* payload =
        stream->GetOutBuffer<SIG_GET_USER_REGDATE>();

    unsigned int regdate_hour = 0;
    unsigned int regdate = 0;
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect DATE_FORMAT(from_unixtime(reg_date),'%%Y%%m%%d%%H') ,reg_date from member_info where m_id=%d",
        payload->account_id);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->get_n_rows()) {
        if (db_handle->fetch() != true) return false;
        if (db_handle->get_uint(0, regdate_hour) != true) return false;
        if (db_handle->get_uint(1, regdate) != true) return false;
    }

    payload->regdate_hour = regdate_hour / 100;
    payload->regdate_seconds = regdate;
    return SendResult(signal, user_idx, payload);
}

// IDA_EA: 0x8442FAE
// IDA_NAME: _ZN17DB_GetUserRegdate11makeRequestEij
// DEMANGLED: DB_GetUserRegdate::makeRequest(int,unsigned int)
void DB_GetUserRegdate::makeRequest(int user_idx, unsigned int account_id) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 42162);
    CStreamGuard guard(stream, true);
    *guard << 673;
    *guard << user_idx;
    SIG_GET_USER_REGDATE* payload =
        guard->GetInBuffer<SIG_GET_USER_REGDATE>();
    if (payload) {
        payload->account_id = account_id;
        payload->regdate_seconds = 0;
        payload->regdate_hour = 0;
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x844309C
// IDA_NAME: _ZN17DB_GetUserRegdate10SendResultEiiP20SIG_GET_USER_REGDATE
// DEMANGLED: DB_GetUserRegdate::SendResult(int,int,SIG_GET_USER_REGDATE *)
bool DB_GetUserRegdate::SendResult(int signal,
                                   int user_idx,
                                   SIG_GET_USER_REGDATE* payload_data) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 42177);
    CStreamGuard guard(stream, true);
    *guard << signal;
    *guard << user_idx;
    guard->put_binary(payload_data, sizeof(*payload_data));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}
