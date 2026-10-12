#include "old_data/db/account/db_find_factory_hub_user.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>
#include <string.h>



namespace {

enum {
    kDbSignalFindFactoryHubUser = 416,
    kInterSignalFindFactoryHubUserNoConn = 417,
    kDbHandleGame = 2,
};

#pragma pack(push, 1)
struct SIG_FIND_FACTORY_HUB_USER_LOCAL {
    char charac_name[29];      // +0x00
    uint8_t reserved_29;       // +0x1d
    uint16_t value;            // +0x1e
};

struct SIG_FIND_FACTORY_HUB_USER_NO_CONN_LOCAL {
    uint32_t account_id;       // +0x00
    int32_t charac_no;         // +0x04
    uint16_t value;            // +0x08
    uint8_t reserved[2];       // +0x0a
};
#pragma pack(pop)

typedef char SIG_FIND_FACTORY_HUB_USER_LOCAL_Size_Must_Be_32[
    (sizeof(SIG_FIND_FACTORY_HUB_USER_LOCAL) == 32) ? 1 : -1];
typedef char SIG_FIND_FACTORY_HUB_USER_NO_CONN_LOCAL_Size_Must_Be_12[
    (sizeof(SIG_FIND_FACTORY_HUB_USER_NO_CONN_LOCAL) == 12) ? 1 : -1];

static void copy_fixed_name(char* dest, const char* src, unsigned int size) {
    memset(dest, 0, size + 1);
    memcpy(dest, src, size);
}

}  // namespace

// IDA_EA: 0x844F0BE
// IDA_NAME: _ZN24DB_Find_Factory_Hub_UserC2Ev
// DEMANGLED: DB_Find_Factory_Hub_User::DB_Find_Factory_Hub_User(void)
DB_Find_Factory_Hub_User::DB_Find_Factory_Hub_User() {}

// IDA_EA: 0x845D042
// IDA_NAME: _ZN24DB_Find_Factory_Hub_UserD2Ev
// DEMANGLED: DB_Find_Factory_Hub_User::~DB_Find_Factory_Hub_User()
// IDA_EA: 0x845D072
// IDA_NAME: _ZN24DB_Find_Factory_Hub_UserD0Ev
// DEMANGLED: DB_Find_Factory_Hub_User::~DB_Find_Factory_Hub_User() deleting destructor
DB_Find_Factory_Hub_User::~DB_Find_Factory_Hub_User() {}

// IDA_EA: 0x8437A32
// IDA_NAME: _ZN24DB_Find_Factory_Hub_User8dispatchEiiP6Stream
// DEMANGLED: DB_Find_Factory_Hub_User::dispatch(int,int,Stream *)
bool DB_Find_Factory_Hub_User::dispatch(int signal,
                                        int user_idx,
                                        Stream* stream) {
    (void)signal;

    SIG_FIND_FACTORY_HUB_USER_LOCAL payload;
    memset(&payload, 0, sizeof(payload));
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }

    char source_name[30];
    copy_fixed_name(source_name, payload.charac_name,
                    sizeof(payload.charac_name));

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kDbHandleGame),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect charac_no,m_id from charac_info where charac_name='%s'",
        source_name);
    if (db_handle->exec(true) != true) {
        return false;
    }

    int charac_no = 0;
    unsigned int account_id = 0;
    const unsigned int rows = static_cast<unsigned int>(db_handle->get_n_rows());
    for (unsigned int row = 0; row < rows; ++row) {
        if (db_handle->fetch() != true) {
            return false;
        }
        if (db_handle->get_int(0, charac_no) != true) {
            return false;
        }
        if (db_handle->get_uint(1, account_id) != true) {
            return false;
        }
    }

    Stream* out = GlobalData::stream_pool()->Acquire("DBThread.cpp", 34063);
    CStreamGuard guard(out, true);
    *guard << kInterSignalFindFactoryHubUserNoConn;
    *guard << user_idx;
    SIG_FIND_FACTORY_HUB_USER_NO_CONN_LOCAL* result =
        static_cast<SIG_FIND_FACTORY_HUB_USER_NO_CONN_LOCAL*>(
            guard->GetInBufferUntyped(
                sizeof(SIG_FIND_FACTORY_HUB_USER_NO_CONN_LOCAL)));
    memset(result, 0, sizeof(*result));
    result->account_id = account_id;
    result->charac_no = charac_no;
    result->value = payload.value;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x8437C50
// IDA_NAME: _ZN24DB_Find_Factory_Hub_User11makeRequestEjPct
// DEMANGLED: DB_Find_Factory_Hub_User::makeRequest(unsigned int,char *,unsigned short)
void DB_Find_Factory_Hub_User::makeRequest(unsigned int user_idx,
                                           char* charac_name,
                                           unsigned short value) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 34078);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalFindFactoryHubUser;
    *guard << static_cast<int>(user_idx);

    SIG_FIND_FACTORY_HUB_USER_LOCAL* payload =
        static_cast<SIG_FIND_FACTORY_HUB_USER_LOCAL*>(
            guard->GetInBufferUntyped(sizeof(SIG_FIND_FACTORY_HUB_USER_LOCAL)));
    memset(payload, 0, sizeof(*payload));
    if (charac_name) {
        strncpy(payload->charac_name, charac_name,
                sizeof(payload->charac_name));
    }
    payload->value = value;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
