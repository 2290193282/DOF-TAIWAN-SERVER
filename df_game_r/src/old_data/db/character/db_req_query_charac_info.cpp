#include "old_data/db/character/db_req_query_charac_info.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <string.h>
#include <time.h>


namespace {

enum {
    kDbSignalReqQueryCharacInfo = 196,
};

}  // namespace

// IDA_EA: 0x844E65A
// IDA_NAME: _ZN21DB_ReqQueryCharacInfoC2Ev
// DEMANGLED: DB_ReqQueryCharacInfo::DB_ReqQueryCharacInfo(void)
DB_ReqQueryCharacInfo::DB_ReqQueryCharacInfo() {}

// IDA_EA: 0x845EB12
// IDA_NAME: _ZN21DB_ReqQueryCharacInfoD2Ev
// DEMANGLED: DB_ReqQueryCharacInfo::~DB_ReqQueryCharacInfo()
// IDA_EA: 0x845EB42
// IDA_NAME: _ZN21DB_ReqQueryCharacInfoD0Ev
// DEMANGLED: DB_ReqQueryCharacInfo::~DB_ReqQueryCharacInfo() deleting destructor
DB_ReqQueryCharacInfo::~DB_ReqQueryCharacInfo() {}

// IDA_EA: 0x842688C
// IDA_NAME: _ZN21DB_ReqQueryCharacInfo8dispatchEiiP6Stream
// DEMANGLED: DB_ReqQueryCharacInfo::dispatch(int,int,Stream *)
bool DB_ReqQueryCharacInfo::dispatch(int signal,
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
        return _sendResult(2, user_idx, 0, 0, 0, 0, 0, 0, 0) != 0;
    }
    if (!db_handle->get_n_rows()) {
        return _sendResult(21, user_idx, 0, 0, 0, 0, 0, 0, 0) != 0;
    }
    if (db_handle->get_n_rows()) {
        int delete_flag = 0;
        if (db_handle->fetch() != true ||
            db_handle->get_int(0, delete_flag) != true) {
            return _sendResult(2, user_idx, 0, 0, 0, 0, 0, 0, 0) != 0;
        }
        if (delete_flag) {
            return _sendResult(21, user_idx, 0, 0, 0, 0, 0, 0, 0) != 0;
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
        const int gift_recv_count =
            _getGiftRecvCount(static_cast<unsigned int>(account_id));
        _sendResult(0,
                    user_idx,
                    payload->charac_name,
                    charac_no,
                    static_cast<char>(job),
                    static_cast<short>(level),
                    static_cast<char>(grow_type),
                    static_cast<char>(sex),
                    static_cast<char>(gift_recv_count));
    } else {
        _sendResult(21, user_idx, 0, 0, 0, 0, 0, 0, 0);
    }
    return true;
}

// IDA_EA: 0x8426D18
// IDA_NAME: _ZN21DB_ReqQueryCharacInfo11makeRequestEiPKc
// DEMANGLED: DB_ReqQueryCharacInfo::makeRequest(int,char const*)
void DB_ReqQueryCharacInfo::makeRequest(int user_idx,
                                        const char* charac_name) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 26347);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalReqQueryCharacInfo;
    *guard << user_idx;
    SIG_QUERY_CHARAC_INFO* payload =
        guard->GetInBuffer<SIG_QUERY_CHARAC_INFO>();
    if (payload && charac_name) {
        strncpy(payload->charac_name, charac_name, 0x1Du);
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x8426E04
// IDA_NAME: _ZN21DB_ReqQueryCharacInfo11_sendResultEiiPKciiiiii
// DEMANGLED: DB_ReqQueryCharacInfo::_sendResult(int,int,char const*,int,int,int,int,int,int)
int DB_ReqQueryCharacInfo::_sendResult(int result,
                                       int user_idx,
                                       const char* charac_name,
                                       int charac_no,
                                       int job,
                                       int level,
                                       int grow_type,
                                       int sex,
                                       int gift_recv_count) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 26390);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalReqQueryCharacInfo;
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
        payload->gift_recv_count = static_cast<int8_t>(gift_recv_count);
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return 1;
}

// IDA_EA: 0x8426F46
// IDA_NAME: _ZN21DB_ReqQueryCharacInfo17_getGiftRecvCountEj
// DEMANGLED: DB_ReqQueryCharacInfo::_getGiftRecvCount(unsigned int)
int DB_ReqQueryCharacInfo::_getGiftRecvCount(unsigned int account_id) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0));
    int last_gift_time = 0;
    int gift_count = -1;
    char table_name[20];
    memset(table_name, 0, sizeof(table_name));
    memcpy(table_name, "member_login", 0xDu);

    db_handle->set_query("seLect last_gift_time,gift_cnt from %s where m_id=%s",
                         table_name,
                         NumberToString(account_id, 0));
    if (db_handle->exec(true) != true) {
        return 0;
    }
    if (db_handle->get_n_rows()) {
        if (db_handle->fetch() != true) {
            return -1;
        }
        db_handle->get_int(0, last_gift_time);
        db_handle->get_int(1, gift_count);
        if (gift_count) {
            CSystemTime* system_time = GlobalData::system_time();
            const time_t cur_sec =
                system_time ? static_cast<time_t>(system_time->getCurSec()) : 0;
            const time_t gift_time = static_cast<time_t>(last_gift_time);
            struct tm cur_tm;
            struct tm gift_tm;
            localtime_r(&gift_time, &gift_tm);
            localtime_r(&cur_sec, &cur_tm);
            if (gift_tm.tm_mon != cur_tm.tm_mon) {
                db_handle->set_query(
                    "upDate %s set last_gift_time=unix_timestamp(now()),gift_cnt=0 where m_id=%s",
                    table_name,
                    NumberToString(account_id, 0));
                db_handle->exec(true);
                return 0;
            }
        }
    }
    return gift_count;
}
