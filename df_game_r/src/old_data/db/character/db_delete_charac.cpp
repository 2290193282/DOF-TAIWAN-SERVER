#include "old_data/db/character/db_delete_charac.h"

#include "old_data/db/inventory/db_load_inventory.h"
#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <stdint.h>
#include <string.h>



namespace {

enum {
    kItemLockInfoCount = 100,
    kItemLockExpireSeconds = 259200,
};

}  // namespace

// IDA_EA: 0x8402BCE
// IDA_NAME: _ZN15DB_DeleteCharacC2Ev
// DEMANGLED: DB_DeleteCharac::DB_DeleteCharac(void)
DB_DeleteCharac::DB_DeleteCharac() {}

// IDA_EA: 0x8402BEA
// IDA_NAME: _ZN15DB_DeleteCharacD2Ev
// DEMANGLED: DB_DeleteCharac::~DB_DeleteCharac()
// IDA_EA: 0x8402C1A
// IDA_NAME: _ZN15DB_DeleteCharacD0Ev
// DEMANGLED: DB_DeleteCharac::~DB_DeleteCharac() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_DeleteCharac::~DB_DeleteCharac() {}

// IDA_EA: 0x8402C38
// IDA_NAME: _ZN15DB_DeleteCharac8dispatchEiiP6Stream
// DEMANGLED: DB_DeleteCharac::dispatch(int,int,Stream *)
bool DB_DeleteCharac::dispatch(int signal, int user_idx, Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }

    SIG_DELETE_CHARAC* request = stream->GetOutBuffer<SIG_DELETE_CHARAC>();
    if (!request) {
        return false;
    }
    request->result = 0;

    const int cur_sec = GlobalData::system_time()->getCurSec();
    stItemLockInfo item_locks[kItemLockInfoCount];
    memset(item_locks, 0, sizeof(item_locks));

    if (DB_LoadInventory::GetItemLock(request->charac_no,
                                      item_locks) == 0) {
        for (int index = 0; index < kItemLockInfoCount; ++index) {
            const stItemLockInfo& lock = item_locks[index];
            if (lock.lock_type == 1 ||
                (lock.lock_type == 2 &&
                 static_cast<int>(lock.lock_time + kItemLockExpireSeconds) >
                     cur_sec)) {
                request->result = 3;
                return SendResult(signal, user_idx, request) == true;
            }
        }
    }

    if (DeleteGuildInfo(request) == true) {
        request->event_character_deleted = 0;
        if (CheckEventCharacter40(request) == true) {
            DeleteEventCharacFlag(request);
            request->event_character_deleted = 1;
        }
        if (DeleteCharac(request) != true) {
            request->result = 1;
        }
        DeleteCharacOption(request);
        const bool sent = SendResult(signal, user_idx, request);
        if (sent == true) {
            cMyTrace trace(
                "virtual bool DB_DeleteCharac::dispatch(int, int, Stream*)",
                4814,
                0);
            trace("[%s][%d] deLete charac(%d),guild_id(%d)",
                  "virtual bool DB_DeleteCharac::dispatch(int, int, Stream*)",
                  4814,
                  request->charac_no,
                  request->guild_id);
            return true;
        }
        return false;
    }

    cMyTrace trace("virtual bool DB_DeleteCharac::dispatch(int, int, Stream*)",
                   4775,
                   0);
    trace("[%s][%d] deLete charac(%d),guild_id(%d) FAIL",
          "virtual bool DB_DeleteCharac::dispatch(int, int, Stream*)",
          4775,
          request->charac_no,
          request->guild_id);
    return SendResult(signal, user_idx, request) == true;
}

// IDA_EA: 0x8402F4C
// IDA_NAME: _ZN15DB_DeleteCharac15DeleteGuildInfoEP17SIG_DELETE_CHARAC
// DEMANGLED: DB_DeleteCharac::DeleteGuildInfo(SIG_DELETE_CHARAC *)
bool DB_DeleteCharac::DeleteGuildInfo(SIG_DELETE_CHARAC* payload_data) {
    if (!payload_data->guild_id) {
        MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
        db_handle->set_query(
            "seLect guild_id from charac_info where charac_no = %d",
            payload_data->charac_no);
        if (db_handle->exec(true) != true) {
            payload_data->result = 1;
            return false;
        }
        if (db_handle->fetch() != true ||
            db_handle->get_uint(0, payload_data->guild_id) != true) {
            payload_data->result = 1;
            return false;
        }
    }

    if (!payload_data->guild_id) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "bool DB_DeleteCharac::DeleteGuildInfo(SIG_DELETE_CHARAC*)",
            0x1312,
            "[Guild Del Error] guild_id=0, charac_no:%d",
            payload_data->charac_no);
        return true;
    }

    MySQL* guild_db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(8),static_cast<ENUM_SERVER_GROUP>( 0));
    guild_db_handle->set_query("seLect master_no from guild_info where guild_id = %d",
                               payload_data->guild_id);
    if (guild_db_handle->exec(true) != true) {
        payload_data->result = 1;
        return false;
    }

    unsigned int master_no = 0;
    if (guild_db_handle->fetch() != true ||
        guild_db_handle->get_uint(0, master_no) != true) {
        payload_data->result = 1;
        return false;
    }
    if (payload_data->charac_no == master_no) {
        payload_data->result = 2;
        return false;
    }

    guild_db_handle->set_query(
        "upDate guild_member set member_flag = 2, secede_time = now(), secede_type = 2 where guild_id = %d and charac_no = %d",
        payload_data->guild_id,
        payload_data->charac_no);
    if (guild_db_handle->exec(true) != true) {
        payload_data->result = 1;
        return false;
    }

    guild_db_handle->set_query(
        "seLect count(*) from guild_member where guild_id = %d and member_flag = 1",
        payload_data->guild_id);
    if (guild_db_handle->exec(true) != true) {
        payload_data->result = 1;
        return false;
    }

    int member_count = 0;
    if (guild_db_handle->fetch() != true ||
        guild_db_handle->get_int(0, member_count) != true) {
        payload_data->result = 1;
        return false;
    }

    if (!member_count) {
        return true;
    }

    guild_db_handle->set_query(
        "upDate guild_info set member_count = %d where guild_id = %d",
        member_count,
        payload_data->guild_id);
    if (guild_db_handle->exec(true) != true) {
        payload_data->result = 1;
        return false;
    }
    return true;
}

// IDA_EA: 0x84032AA
// IDA_NAME: _ZN15DB_DeleteCharac21DeleteEventCharacFlagEP17SIG_DELETE_CHARAC
// DEMANGLED: DB_DeleteCharac::DeleteEventCharacFlag(SIG_DELETE_CHARAC *)
bool DB_DeleteCharac::DeleteEventCharacFlag(SIG_DELETE_CHARAC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0));
    const char* account_id = NumberToString(payload->account_id, 0);
    db_handle->set_query(
        "upDate member_login set event_charac_flag = 0 where m_id=%s",
        account_id);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x8403324
// IDA_NAME: _ZN15DB_DeleteCharac21CheckEventCharacter40EP17SIG_DELETE_CHARAC
// DEMANGLED: DB_DeleteCharac::CheckEventCharacter40(SIG_DELETE_CHARAC *)
bool DB_DeleteCharac::CheckEventCharacter40(SIG_DELETE_CHARAC* payload_data) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    const char* account_id = NumberToString(payload_data->account_id, 0);
    db_handle->set_query(
        "seLect event_charac_level from charac_info where m_id=%s and charac_no=%d and delete_flag=0",
        account_id,
        payload_data->charac_no);
    if (db_handle->exec(true) != true) {
        return false;
    }

    int event_charac_level = 0;
    return db_handle->fetch() == true &&
           db_handle->get_int(0, event_charac_level) == true &&
           event_charac_level != 0;
}

// IDA_EA: 0x840340C
// IDA_NAME: _ZN15DB_DeleteCharac12DeleteCharacEP17SIG_DELETE_CHARAC
// DEMANGLED: DB_DeleteCharac::DeleteCharac(SIG_DELETE_CHARAC *)
bool DB_DeleteCharac::DeleteCharac(SIG_DELETE_CHARAC* payload_data) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    const char* account_id = NumberToString(payload_data->account_id, 0);
    db_handle->set_query(
        "upDate charac_info set delete_time=now(),delete_flag=1 where charac_no=%u and m_id=%s",
        payload_data->charac_no,
        account_id);
    if (db_handle->exec(true) != true) {
        return false;
    }

    MySQL* master_db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1),static_cast<ENUM_SERVER_GROUP>( 0));
    // DeleteCharac @ 0x840340C uses CEnvironment+888 as the
    // dnf_master_charac.server_id key.
    const unsigned int server_id =
        static_cast<unsigned int>(G_CEnvironment()->get_server_group());
    master_db_handle->set_query(
        "deLete from dnf_master_charac where m_id='%u' and server_id='%u' and charac_no='%u'",
        payload_data->account_id,
        server_id,
        payload_data->charac_no);
    master_db_handle->exec(true);
    return true;
}

// IDA_EA: 0x84034FE
// IDA_NAME: _ZN15DB_DeleteCharac10SendResultEiiP17SIG_DELETE_CHARAC
// DEMANGLED: DB_DeleteCharac::SendResult(int,int,SIG_DELETE_CHARAC *)
bool DB_DeleteCharac::SendResult(int signal,
                                 int user_idx,
                                 SIG_DELETE_CHARAC* payload_data) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 5040);
    CStreamGuard guard(stream, true);
    *guard << signal;
    *guard << user_idx;
    guard->put_binary(payload_data, sizeof(*payload_data));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x84035E0
// IDA_NAME: _ZN15DB_DeleteCharac18DeleteCharacOptionEP17SIG_DELETE_CHARAC
// DEMANGLED: DB_DeleteCharac::DeleteCharacOption(SIG_DELETE_CHARAC *)
bool DB_DeleteCharac::DeleteCharacOption(SIG_DELETE_CHARAC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query("deLete from charac_option where charac_no = %u",
                         payload->charac_no);
    return db_handle->exec(true) == true;
}
