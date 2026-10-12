#include "old_data/db/anti_cheat/db_save_client_hack_log.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>



namespace {

#pragma pack(push, 1)
struct SIG_SAVE_CLIENT_HACK_LOG_LOCAL {
    uint32_t raw_00;           // +0x00: request context / result m_id
    uint32_t charac_no;        // +0x04
    uint16_t hack_type;        // +0x08
    int32_t etc;               // +0x0A
    int32_t hack_count;        // +0x0E
};
#pragma pack(pop)

typedef char SIG_SAVE_CLIENT_HACK_LOG_LOCAL_Size_Must_Be_18[
    (sizeof(SIG_SAVE_CLIENT_HACK_LOG_LOCAL) == 18) ? 1 : -1];

}  // namespace

// IDA_EA: 0x8421D50
// IDA_NAME: _ZN20DB_SaveClientHackLog19_getTodaysHackCountEP5MySQLjt
// DEMANGLED: DB_SaveClientHackLog::_getTodaysHackCount(MySQL *,unsigned int,unsigned short)
int DB_SaveClientHackLog::_getTodaysHackCount(MySQL* db_handle,
                                              unsigned int account_id,
                                              unsigned short hack_type) {
    int count = 0;
    db_handle->set_query(
        "seLect cnt from dnf_hack_log where m_id=%s and hack_type=%d and occ_date=CAST(DATE_FORMAT(now(), '%%Y-%%m-%%d') AS CHAR)",
        NumberToString(account_id, 0),
        hack_type);
    if (db_handle->exec(true) != true || db_handle->get_n_rows() != 1) {
        return 0;
    }
    if (db_handle->fetch() != true || db_handle->get_int(0, count) != true ||
        count == 0) {
        return 0;
    }
    return count;
}

// IDA_EA: 0x8421E3A
// IDA_NAME: _ZN20DB_SaveClientHackLog11_sendResultEj
// DEMANGLED: DB_SaveClientHackLog::_sendResult(unsigned int)
void DB_SaveClientHackLog::_sendResult(unsigned int account_id) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 23060);
    CStreamGuard guard(stream, true);
    *guard << 154;
    *guard << -1;
    SIG_SAVE_CLIENT_HACK_LOG_LOCAL* payload =
        static_cast<SIG_SAVE_CLIENT_HACK_LOG_LOCAL*>(
            guard->GetInBufferUntyped(sizeof(SIG_SAVE_CLIENT_HACK_LOG_LOCAL)));
    payload->raw_00 = account_id;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
}

// IDA_EA: 0x8421F16
// IDA_NAME: _ZN20DB_SaveClientHackLog8dispatchEiiP6Stream
// DEMANGLED: DB_SaveClientHackLog::dispatch(int,int,Stream *)
bool DB_SaveClientHackLog::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_SAVE_CLIENT_HACK_LOG_LOCAL payload;
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(4),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "upDate dnf_hack_log set charac_no=%d,last_time=now(),cnt=cnt+%d,etc=%d where m_id=%s and hack_type=%d and occ_date=CAST(DATE_FORMAT(now(), '%%Y-%%m-%%d') AS CHAR)",
        payload.charac_no,
        payload.hack_count,
        payload.etc,
        NumberToString(payload.raw_00, 0),
        payload.hack_type);
    if (db_handle->exec(true) != true || db_handle->getAffectedRowCount() == 0) {
        db_handle->set_query(
            "inSert into dnf_hack_log(m_id,charac_no,hack_type,occ_date,last_time,etc,cnt) values(%s,%d,%d,cast(now() as date),now(),%d,%d)",
            NumberToString(payload.raw_00, 0),
            payload.charac_no,
            payload.hack_type,
            payload.etc,
            payload.hack_count);
        db_handle->exec(true);
        return true;
    }

    if (payload.hack_type == 403 || payload.hack_type == 503 ||
        payload.hack_type == 806 || payload.hack_type == 954) {
        const int count =
            _getTodaysHackCount(db_handle, payload.raw_00, payload.hack_type);
        _sendHackTypeResult(payload.raw_00, payload.hack_type, count);
    }
    return true;
}

// IDA_EA: 0x8422152
// IDA_NAME: _ZN20DB_SaveClientHackLog19_sendHackTypeResultEjii
// DEMANGLED: DB_SaveClientHackLog::_sendHackTypeResult(unsigned int,int,int)
void DB_SaveClientHackLog::_sendHackTypeResult(unsigned int account_id,
                                               int hack_type,
                                               int todays_hack_count) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 23148);
    CStreamGuard guard(stream, true);
    *guard << 154;
    *guard << -1;
    SIG_SAVE_CLIENT_HACK_LOG_LOCAL* payload =
        static_cast<SIG_SAVE_CLIENT_HACK_LOG_LOCAL*>(
            guard->GetInBufferUntyped(sizeof(SIG_SAVE_CLIENT_HACK_LOG_LOCAL)));
    payload->raw_00 = account_id;
    payload->hack_type = static_cast<uint16_t>(hack_type);
    payload->hack_count = todays_hack_count;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
}

// IDA_EA: 0x844E42A
// IDA_NAME: _ZN20DB_SaveClientHackLogC2Ev
// DEMANGLED: DB_SaveClientHackLog::DB_SaveClientHackLog(void)
DB_SaveClientHackLog::DB_SaveClientHackLog() {}

// IDA_EA: 0x845F1C6
// IDA_NAME: _ZN20DB_SaveClientHackLogD2Ev
// DEMANGLED: DB_SaveClientHackLog::~DB_SaveClientHackLog()
// IDA_EA: 0x845F1F6
// IDA_NAME: _ZN20DB_SaveClientHackLogD0Ev
// DEMANGLED: DB_SaveClientHackLog::~DB_SaveClientHackLog() deleting destructor
DB_SaveClientHackLog::~DB_SaveClientHackLog() {}
