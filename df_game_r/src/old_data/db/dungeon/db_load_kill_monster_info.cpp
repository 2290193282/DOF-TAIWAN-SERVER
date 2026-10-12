#include "old_data/db/dungeon/db_load_kill_monster_info.h"

#include "common/types/compressed_blob.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

// IDA_EA: 0x844F812
// IDA_NAME: _ZN22DB_LoadKillMonsterInfoC2Ev
// DEMANGLED: DB_LoadKillMonsterInfo::DB_LoadKillMonsterInfo(void)
DB_LoadKillMonsterInfo::DB_LoadKillMonsterInfo() {}

// IDA_EA: 0x845BB8A
// IDA_NAME: _ZN22DB_LoadKillMonsterInfoD2Ev
// DEMANGLED: DB_LoadKillMonsterInfo::~DB_LoadKillMonsterInfo()
// IDA_EA: 0x845BBBA
// IDA_NAME: _ZN22DB_LoadKillMonsterInfoD0Ev
// DEMANGLED: DB_LoadKillMonsterInfo::~DB_LoadKillMonsterInfo()
DB_LoadKillMonsterInfo::~DB_LoadKillMonsterInfo() {}

// IDA_EA: 0x84414E0
// IDA_NAME: _ZN22DB_LoadKillMonsterInfo8dispatchEiiP6Stream
// DEMANGLED: DB_LoadKillMonsterInfo::dispatch(int,int,Stream *)
bool DB_LoadKillMonsterInfo::dispatch(int signal,
                                      int user_idx,
                                      Stream* stream) {
    SIG_LOAD_KILL_MONSTER_INFO* const request =
        stream->GetOutBuffer<SIG_LOAD_KILL_MONSTER_INFO>();
    if (!request) {
        return false;
    }
    if (LoadMonsterInfo(request) == true) {
        SendResult(signal, user_idx, request);
        return true;
    }

    cMyTrace trace("virtual bool DB_LoadKillMonsterInfo::dispatch(int, int, Stream*)",
                   41035,
                   5);
    trace("DB_LoadKillMonsterInfo::LoadMonsterInfo ERROR charac_no=%u",
          request->charac_no);
    return false;
}

// IDA_EA: 0x844159A
// IDA_NAME: _ZN22DB_LoadKillMonsterInfo15LoadMonsterInfoEP26SIG_LOAD_KILL_MONSTER_INFO
// DEMANGLED: DB_LoadKillMonsterInfo::LoadMonsterInfo(SIG_LOAD_KILL_MONSTER_INFO *)
bool DB_LoadKillMonsterInfo::LoadMonsterInfo(SIG_LOAD_KILL_MONSTER_INFO* payload_data) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(2),
        static_cast<ENUM_SERVER_GROUP>(0));
    db_handle->set_query(
        "seLect boss_info, named_info, apc_boss_info from charac_kill_monster_info where charac_no=%u",
        payload_data->charac_no);
    if (db_handle->exec(true) != true) {
        cMyTrace trace("bool DB_LoadKillMonsterInfo::LoadMonsterInfo(SIG_LOAD_KILL_MONSTER_INFO*)",
                       41059,
                       5);
        trace("DB_LoadKillMonsterInfo::LoadMonsterInfo select, exec() ERROR charac_no=%u",
              payload_data->charac_no);
        return false;
    }

    if (!db_handle->get_n_rows()) {
        db_handle->set_query(
            "inSert into charac_kill_monster_info (charac_no) values(%u)",
            payload_data->charac_no);
        if (db_handle->exec(true) == true) {
            return true;
        }
        cMyTrace trace("bool DB_LoadKillMonsterInfo::LoadMonsterInfo(SIG_LOAD_KILL_MONSTER_INFO*)",
                       41072,
                       5);
        trace("DB_LoadKillMonsterInfo::LoadMonsterInfo insert, exec() ERROR charac_no=%u",
              payload_data->charac_no);
        return false;
    }

    if (db_handle->fetch() != true) {
        cMyTrace trace("bool DB_LoadKillMonsterInfo::LoadMonsterInfo(SIG_LOAD_KILL_MONSTER_INFO*)",
                       41082,
                       5);
        trace("DB_LoadKillMonsterInfo::LoadMonsterInfo select, fetch() ERROR charac_no=%u",
              payload_data->charac_no);
        return false;
    }

    int column = 0;
    if (db_handle->get_binary_length(column)) {
        const int read_column = column++;
        if (get_compressed_blob_data(
                db_handle,
                read_column,
                reinterpret_cast<char*>(payload_data->boss_info),
                sizeof(payload_data->boss_info),
                0) != 1) {
            cMyTrace trace("bool DB_LoadKillMonsterInfo::LoadMonsterInfo(SIG_LOAD_KILL_MONSTER_INFO*)",
                           41095,
                           5);
            trace("DB_LoadKillMonsterInfo::LoadMonsterInfo uncompress_zip(%u) ERROR charac_no=%u",
                  column,
                  payload_data->charac_no);
            return false;
        }
    } else {
        ++column;
    }

    if (db_handle->get_binary_length(column)) {
        const int read_column = column++;
        if (get_compressed_blob_data(
                db_handle,
                read_column,
                reinterpret_cast<char*>(payload_data->named_info),
                sizeof(payload_data->named_info),
                0) != 1) {
            cMyTrace trace("bool DB_LoadKillMonsterInfo::LoadMonsterInfo(SIG_LOAD_KILL_MONSTER_INFO*)",
                           41115,
                           5);
            trace("DB_LoadKillMonsterInfo::LoadMonsterInfo uncompress_zip(%u) ERROR charac_no=%u",
                  column,
                  payload_data->charac_no);
            return false;
        }
    } else {
        ++column;
    }

    if (db_handle->get_binary_length(column)) {
        const int read_column = column++;
        if (get_compressed_blob_data(
                db_handle,
                read_column,
                reinterpret_cast<char*>(payload_data->apc_boss_info),
                sizeof(payload_data->apc_boss_info),
                0) != 1) {
            cMyTrace trace("bool DB_LoadKillMonsterInfo::LoadMonsterInfo(SIG_LOAD_KILL_MONSTER_INFO*)",
                           41135,
                           5);
            trace("DB_LoadKillMonsterInfo::LoadMonsterInfo uncompress_zip(%u) ERROR charac_no=%u",
                  column,
                  payload_data->charac_no);
            return false;
        }
    } else {
        ++column;
    }
    return true;
}

// IDA_EA: 0x84419AC
// IDA_NAME: _ZN22DB_LoadKillMonsterInfo10SendResultEiiP26SIG_LOAD_KILL_MONSTER_INFO
// DEMANGLED: DB_LoadKillMonsterInfo::SendResult(int,int,SIG_LOAD_KILL_MONSTER_INFO *)
bool DB_LoadKillMonsterInfo::SendResult(int signal,
                                        int user_idx,
                                        SIG_LOAD_KILL_MONSTER_INFO* payload_data) {
    Stream* stream = GlobalData::big_stream_pool()->Acquire(
        "DBThread.cpp", 41153);
    CStreamGuard guard(stream, true);
    *guard << signal;
    *guard << user_idx;
    guard->put_binary(payload_data, sizeof(*payload_data));
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}
