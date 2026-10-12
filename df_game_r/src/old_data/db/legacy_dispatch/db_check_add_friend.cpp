#include "old_data/db/legacy_dispatch/db_check_add_friend.h"

#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/gm_accounts_abi.h"
#include "old_data/db/core/mysql.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>

// IDA_EA: 0x844FC56
// IDA_NAME: _ZN17DB_CheckAddFriendC2Ev
// DEMANGLED: DB_CheckAddFriend::DB_CheckAddFriend(void)
DB_CheckAddFriend::DB_CheckAddFriend() : DBDispatcher() {}

// IDA_EA: 0x845A874
// IDA_NAME: _ZN17DB_CheckAddFriendD2Ev
// DEMANGLED: DB_CheckAddFriend::~DB_CheckAddFriend()
// IDA_EA: 0x845A8A4
// IDA_NAME: _ZN17DB_CheckAddFriendD0Ev
// DEMANGLED: DB_CheckAddFriend::~DB_CheckAddFriend() deleting destructor
DB_CheckAddFriend::~DB_CheckAddFriend() {}

// IDA_EA: 0x844C6E0
// IDA_NAME: _ZN17DB_CheckAddFriend8dispatchEiiP6Stream
// DEMANGLED: DB_CheckAddFriend::dispatch(int,int,Stream *)
bool DB_CheckAddFriend::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;

    uint32_t account_id = 0;
    MySQL* db_handle = 0;
    int result = 1;
    char escaped_name[60] = {};

    if (!stream) {
        return false;
    }

    SIG_CHECK_ADD_FRIEND* request =
        stream->GetOutBuffer<SIG_CHECK_ADD_FRIEND>();
    if (!request) {
        return false;
    }

    if (!request->check_name) {
        result = 0;
    } else {
        db_handle = GlobalData::s_db_mgr->GetDBHandle(
            static_cast<ENUM_DB_HANDLE_IDX>(2),
            static_cast<ENUM_SERVER_GROUP>(0));
        if (db_handle &&
            db_handle->escape_string(escaped_name, request->character_name) &&
            db_handle->set_query(
                "seLect m_id from charac_info where charac_name='%s' and delete_flag=0",
                escaped_name) &&
            db_handle->exec(true) && db_handle->fetch() &&
            db_handle->get_n_rows() == 1 &&
            db_handle->get_int(0, account_id)) {
            result = GlobalData::s_pGMAccounts_->isGM(account_id) ? 90 : 0;
        }
    }

    Stream* reply_stream = GlobalData::s_stream_pool->Acquire(
        "DBThread.cpp", 49246);
    CStreamGuard guard(reply_stream, true);
    *guard << 0x33E;
    *guard << user_idx;
    SIG_CHECK_ADD_FRIEND* reply = guard->GetInBuffer<SIG_CHECK_ADD_FRIEND>();
    if (!reply) {
        return false;
    }

    reply->result = result;
    std::strncpy(reply->character_name, request->character_name, 30);
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}
