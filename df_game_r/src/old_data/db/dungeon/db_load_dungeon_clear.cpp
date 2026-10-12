#include "old_data/db/dungeon/db_load_dungeon_clear.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <string.h>

namespace {

const unsigned int kDungeonTextCapacity = 0x1800u;
const unsigned int kDungeonRowCapacity = 0x1ACu;

}  // namespace

// IDA_EA: 0x8419852
// IDA_NAME: _ZN19DB_LoadDungeonClearC2Ev
// DEMANGLED: DB_LoadDungeonClear::DB_LoadDungeonClear(void)
DB_LoadDungeonClear::DB_LoadDungeonClear() {}

// IDA_EA: 0x841986E
// IDA_NAME: _ZN19DB_LoadDungeonClearD2Ev
// DEMANGLED: DB_LoadDungeonClear::~DB_LoadDungeonClear()
// IDA_EA: 0x841989E
// IDA_NAME: _ZN19DB_LoadDungeonClearD0Ev
// DEMANGLED: DB_LoadDungeonClear::~DB_LoadDungeonClear() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_LoadDungeonClear::~DB_LoadDungeonClear() {}

// IDA_EA: 0x84198BC
// IDA_NAME: _ZN19DB_LoadDungeonClear8dispatchEiiP6Stream
// DEMANGLED: DB_LoadDungeonClear::dispatch(int,int,Stream *)
bool DB_LoadDungeonClear::dispatch(int signal, int user_idx, Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }

    SIG_LOAD_DUNGEON_CLEAR* request =
        stream->GetOutBuffer<SIG_LOAD_DUNGEON_CLEAR>();
    if (!request) {
        return false;
    }
    if (LoadDungeonClear(request) == true) {
        return SendResult(signal, user_idx, request) == true;
    }
    return false;
}

// IDA_EA: 0x841996E
// IDA_NAME: _ZN19DB_LoadDungeonClear26AradLoadMemberDungeonClearEP22SIG_LOAD_DUNGEON_CLEAR
// DEMANGLED: DB_LoadDungeonClear::AradLoadMemberDungeonClear(SIG_LOAD_DUNGEON_CLEAR *)
bool DB_LoadDungeonClear::AradLoadMemberDungeonClear(SIG_LOAD_DUNGEON_CLEAR* payload_data) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query("seLect dungeon from member_dungeon where m_id=%u",
                         payload_data->account_id);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (!db_handle->get_n_rows()) {
        return false;
    }
    if (db_handle->fetch() != true) {
        return false;
    }
    if (db_handle->get_str(0, payload_data->dungeon, 6143) != true) {
        return false;
    }
    if (!strlen(payload_data->dungeon)) {
        return false;
    }
    payload_data->needs_member_save = 0;
    payload_data->result = 0;
    return true;
}

// IDA_EA: 0x8419A76
// IDA_NAME: _ZN19DB_LoadDungeonClear20AradLoadDungeonClearEP22SIG_LOAD_DUNGEON_CLEAR
// DEMANGLED: DB_LoadDungeonClear::AradLoadDungeonClear(SIG_LOAD_DUNGEON_CLEAR *)
bool DB_LoadDungeonClear::AradLoadDungeonClear(
    SIG_LOAD_DUNGEON_CLEAR* payload) {
    if (AradLoadMemberDungeonClear(payload) == true) {
        return true;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "select B.dungeon from charac_info A Inner Join charac_dungeon B where A.m_id = %d and B.charac_no = A.charac_no and "
        "A.delete_flag = 0",
        payload->account_id);
    if (db_handle->exec(true) != true) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "bool DB_LoadDungeonClear::AradLoadDungeonClear(SIG_LOAD_DUNGEON_CLEAR*)",
            0x4B72,
            "Dungeon share query error : m_id(%d) dungeon count(%d)",
            payload->account_id,
            db_handle->get_n_rows());
        return false;
    }
    if (!db_handle->get_n_rows()) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "bool DB_LoadDungeonClear::AradLoadDungeonClear(SIG_LOAD_DUNGEON_CLEAR*)",
            0x4B79,
            "Dungeon count error : m_id(%d) dungeon count(%d)",
            payload->account_id,
            db_handle->get_n_rows());
        return true;
    }

    char row_dungeon[428];
    memset(row_dungeon, 0, sizeof(row_dungeon));
    char* dest = payload->dungeon;
    for (int row = 0; row < db_handle->get_n_rows(); ++row) {
        if (db_handle->fetch() != true) {
            LogManager::logFormat(
                1,
                "DBThread.cpp",
                "bool DB_LoadDungeonClear::AradLoadDungeonClear(SIG_LOAD_DUNGEON_CLEAR*)",
                0x4B89,
                "Dungeon fetch error : m_id(%d) dungeon count(%d)",
                payload->account_id,
                db_handle->get_n_rows());
            return false;
        }
        if (db_handle->get_str(0, row_dungeon, 427) != true) {
            LogManager::logFormat(
                1,
                "DBThread.cpp",
                "bool DB_LoadDungeonClear::AradLoadDungeonClear(SIG_LOAD_DUNGEON_CLEAR*)",
                0x4B8F,
                "Dungeon Parsing error : m_id(%d) dungeon count(%d)",
                payload->account_id,
                db_handle->get_n_rows());
            return false;
        }
        if (strlen(payload->dungeon) + strlen(row_dungeon) >
            kDungeonTextCapacity) {
            LogManager::logFormat(
                1,
                "DBThread.cpp",
                "bool DB_LoadDungeonClear::AradLoadDungeonClear(SIG_LOAD_DUNGEON_CLEAR*)",
                0x4B95,
                "Dungeon Total Data Error : m_id(%d) dungeon count(%d)",
                payload->account_id,
                db_handle->get_n_rows());
            return false;
        }
        memcpy(dest, row_dungeon, kDungeonRowCapacity);
        dest += strlen(dest);
        if (row != db_handle->get_n_rows() - 1) {
            *dest = ',';
            ++dest;
        }
    }
    payload->needs_member_save = 1;
    payload->result = 0;
    return true;
}

// IDA_EA: 0x8419DB4
// IDA_NAME: _ZN19DB_LoadDungeonClear16LoadDungeonClearEP22SIG_LOAD_DUNGEON_CLEAR
// DEMANGLED: DB_LoadDungeonClear::LoadDungeonClear(SIG_LOAD_DUNGEON_CLEAR *)
bool DB_LoadDungeonClear::LoadDungeonClear(SIG_LOAD_DUNGEON_CLEAR* payload_data) {
    payload_data->result = 1;
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    const char* member_id =
        NumberToString(payload_data->account_id, 0);
    db_handle->set_query("seLect dungeon from member_dungeon where m_id=%s",
                         member_id);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->get_n_rows() == 1) {
        if (db_handle->fetch() != true) {
            return false;
        }
        if (db_handle->get_str(0, payload_data->dungeon, 6143) != true) {
            return false;
        }
        payload_data->result = 0;
        return true;
    }

    LogManager::logFormat(
        1,
        "DBThread.cpp",
        "bool DB_LoadDungeonClear::LoadDungeonClear(SIG_LOAD_DUNGEON_CLEAR*)",
        0x4BBC,
        "Dungeon count error : m_id(%s) dungeon count(%d)",
        NumberToString(payload_data->account_id, 0),
        db_handle->get_n_rows());
    return false;
}

// IDA_EA: 0x8419F0A
// IDA_NAME: _ZN19DB_LoadDungeonClear10SendResultEiiP22SIG_LOAD_DUNGEON_CLEAR
// DEMANGLED: DB_LoadDungeonClear::SendResult(int,int,SIG_LOAD_DUNGEON_CLEAR *)
bool DB_LoadDungeonClear::SendResult(int signal,
                                     int user_idx,
                                     SIG_LOAD_DUNGEON_CLEAR* payload_data) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 19402);
    CStreamGuard guard(stream, true);
    *guard << signal;
    *guard << user_idx;
    guard->put_binary(payload_data, sizeof(*payload_data));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}
