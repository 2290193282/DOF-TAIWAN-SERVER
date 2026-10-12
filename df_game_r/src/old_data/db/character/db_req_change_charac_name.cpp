#include "old_data/db/character/db_req_change_charac_name.h"

#include "old_data/db/core/db_conn_info.h"
#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <map>
#include <string.h>


namespace {

enum {
    kDbSignalReqChangeCharacName = 195,
};

}  // namespace

// IDA_EA: 0x844E63E
// IDA_NAME: _ZN22DB_ReqChangeCharacNameC2Ev
// DEMANGLED: DB_ReqChangeCharacName::DB_ReqChangeCharacName(void)
DB_ReqChangeCharacName::DB_ReqChangeCharacName() {}

// IDA_EA: 0x845EB60
// IDA_NAME: _ZN22DB_ReqChangeCharacNameD2Ev
// DEMANGLED: DB_ReqChangeCharacName::~DB_ReqChangeCharacName()
// IDA_EA: 0x845EB90
// IDA_NAME: _ZN22DB_ReqChangeCharacNameD0Ev
// DEMANGLED: DB_ReqChangeCharacName::~DB_ReqChangeCharacName() deleting destructor
DB_ReqChangeCharacName::~DB_ReqChangeCharacName() {}

// IDA_EA: 0x84262EA
// IDA_NAME: _ZN22DB_ReqChangeCharacName8dispatchEiiP6Stream
// DEMANGLED: DB_ReqChangeCharacName::dispatch(int,int,Stream *)
bool DB_ReqChangeCharacName::dispatch(int signal,
                                      int user_idx,
                                      Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    SIG_CHANGE_CHARAC_NAME* payload =
        stream->GetOutBuffer<SIG_CHANGE_CHARAC_NAME>();
    if (!payload) {
        return false;
    }

    char escaped_name[60];
    memset(escaped_name, 0, sizeof(escaped_name));
    db_handle->escape_string(escaped_name, payload->charac_name);

    int game_db_index = 2;
    CEnvironment* environment = G_CEnvironment();
    DBConnInfoMap& db_map = environment->db_conn_info_map();
    STDBConnInfo& game_db_info =
        db_map[static_cast<ENUM_DB_HANDLE_IDX>(game_db_index)];
    int same_name_result =
        _hasSameCharacName(db_handle, game_db_info.db_name, escaped_name);
    if (same_name_result) {
        _sendResult(payload, user_idx, same_name_result);
        return true;
    }

    db_handle->set_query(
        "upDate charac_info set charac_name='%s' where charac_no=%d and delete_flag=0",
        escaped_name,
        payload->charac_no);
    if (db_handle->exec(true) != true || !db_handle->getAffectedRowCount()) {
        _sendResult(payload, user_idx, 2);
        return true;
    }

    MySQL* guild_db = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(8),static_cast<ENUM_SERVER_GROUP>( 0));
    guild_db->set_query(
        "seLect guild_id from guild_member where charac_no=%d and member_flag=1 and server_id=%d",
        payload->charac_no,
        payload->server_or_guild.server_id);
    payload->guild_context.guild_id = 0;
    if (guild_db->exec(true) != true || guild_db->fetch() != true) {
        payload->guild_context.guild_id = 0;
    }
    if (guild_db->get_n_rows()) {
        if (guild_db->get_uint(0, payload->guild_context.guild_id) != true) {
            payload->guild_context.guild_id = 0;
        }
    } else {
        payload->guild_context.guild_id = 0;
    }
    _sendResult(payload, user_idx, 0);
    return true;
}

// IDA_EA: 0x8426590
// IDA_NAME: _ZN22DB_ReqChangeCharacName11makeRequestEiiPKci
// DEMANGLED: DB_ReqChangeCharacName::makeRequest(int,int,char const*,int)
void DB_ReqChangeCharacName::makeRequest(int user_idx,
                                         int charac_no,
                                         const char* charac_name,
                                         int request_flag) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 25822);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalReqChangeCharacName;
    *guard << user_idx;
    SIG_CHANGE_CHARAC_NAME* payload =
        guard->GetInBuffer<SIG_CHANGE_CHARAC_NAME>();
    if (!payload) {
        return;
    }
    payload->charac_no = static_cast<uint32_t>(charac_no);
    strncpy(payload->charac_name, charac_name, 0x1Du);
    payload->status.request_flag = request_flag;
    payload->server_or_guild.server_id =
        G_CEnvironment()->get_accept_handshake_flag_888();
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x84266A4
// IDA_NAME: _ZN22DB_ReqChangeCharacName11_sendResultEPK22SIG_CHANGE_CHARAC_NAMEii
// DEMANGLED: DB_ReqChangeCharacName::_sendResult(SIG_CHANGE_CHARAC_NAME const*,int,int)
void DB_ReqChangeCharacName::_sendResult(const SIG_CHANGE_CHARAC_NAME* payload_data,
                                         int user_idx,
                                         int result_code) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 25848);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalReqChangeCharacName;
    *guard << user_idx;
    SIG_CHANGE_CHARAC_NAME* result =
        guard->GetInBuffer<SIG_CHANGE_CHARAC_NAME>();
    if (!result) {
        return;
    }
    result->guild_context.result_request_flag =
        payload_data->status.request_flag;
    result->status.result_code = result_code;
    result->charac_no = payload_data->charac_no;
    result->server_or_guild.result_guild_id =
        payload_data->guild_context.guild_id;
    strncpy(result->charac_name, payload_data->charac_name, 0x1Du);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
}

// IDA_EA: 0x84267C2
// IDA_NAME: _ZN22DB_ReqChangeCharacName18_hasSameCharacNameEP5MySQLPKcS3_
// DEMANGLED: DB_ReqChangeCharacName::_hasSameCharacName(MySQL *,char const*,char const*)
int DB_ReqChangeCharacName::_hasSameCharacName(MySQL* db_handle,
                                               const char* database_name,
                                               const char* escaped_name) {
    db_handle->set_query(
        "seLect delete_flag from %s.charac_info where charac_name='%s'",
        database_name,
        escaped_name);
    if (db_handle->exec(true) != true) {
        return 2;
    }
    int delete_flag = 0;
    if (!db_handle->get_n_rows()) {
        return 0;
    }
    if (db_handle->fetch() != true ||
        db_handle->get_int(0, delete_flag) != true) {
        return 2;
    }
    if (delete_flag) {
        return 24;
    }
    return 20;
}
