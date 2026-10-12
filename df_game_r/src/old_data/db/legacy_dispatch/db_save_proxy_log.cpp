#include "old_data/db/auth_security/db_save_proxy_log.h"
#include "old_data/db/auth_security/save_proxy_log_payload.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"

// IDA_EA: 0x844E47E
// IDA_NAME: _ZN15DB_SaveProxyLogC2Ev
// DEMANGLED: DB_SaveProxyLog::DB_SaveProxyLog(void)
DB_SaveProxyLog::DB_SaveProxyLog() {}

// IDA_EA: 0x845F08E
// IDA_NAME: _ZN15DB_SaveProxyLogD2Ev
// DEMANGLED: DB_SaveProxyLog::~DB_SaveProxyLog()
// IDA_EA: 0x845F0BE
// IDA_NAME: _ZN15DB_SaveProxyLogD0Ev
// DEMANGLED: DB_SaveProxyLog::~DB_SaveProxyLog() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_SaveProxyLog::~DB_SaveProxyLog() {}

// IDA_EA: 0x8422742
// IDA_NAME: _ZN15DB_SaveProxyLog8dispatchEiiP6Stream
// DEMANGLED: DB_SaveProxyLog::dispatch(int,int,Stream *)
bool DB_SaveProxyLog::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_SAVE_PROXY_LOG log;
    if (stream->get_binary(&log, sizeof(log)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(4), static_cast<ENUM_SERVER_GROUP>(0));
    const char* m_id = NumberToString(log.m_id, 0);
    db_handle->set_query(
        "inSert into dnf_proxy_log values(now(),%s,'%s','%s',%d)",
        m_id,
        log.first_ip,
        log.second_ip,
        static_cast<int>(log.proxy_flag));
    return db_handle->exec(true) == true;
}
