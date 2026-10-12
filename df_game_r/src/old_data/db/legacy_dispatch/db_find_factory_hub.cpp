#include "old_data/db/account/db_find_factory_hub.h"
#include "old_data/db/core/db_mgr.h"

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
    kDbHandleGame = 2,
    kInterSignalFindFactoryHubFatigueCheck = 394,
};

#pragma pack(push, 1)
struct SIG_FIND_FACTORY_HUB_LOCAL {
    uint32_t account_id;       // +0x00
    char charac_name[29];      // +0x04
    uint8_t fatigue_check;     // +0x21
    uint8_t reserved[2];       // +0x22
};

struct SIG_FIND_FACTORY_HUB_FATIGUE_CHECK_LOCAL {
    uint32_t account_id;       // +0x00
    uint8_t fatigue_check;     // +0x04
    char charac_name[29];      // +0x05
    uint8_t reserved[2];       // +0x22
};
#pragma pack(pop)

typedef char SIG_FIND_FACTORY_HUB_LOCAL_Size_Must_Be_36[
    (sizeof(SIG_FIND_FACTORY_HUB_LOCAL) == 36) ? 1 : -1];
typedef char SIG_FIND_FACTORY_HUB_FATIGUE_CHECK_LOCAL_Size_Must_Be_36[
    (sizeof(SIG_FIND_FACTORY_HUB_FATIGUE_CHECK_LOCAL) == 36) ? 1 : -1];

static void copy_fixed_name(char* dest, const char* src, unsigned int size) {
    memset(dest, 0, size + 1);
    memcpy(dest, src, size);
}

}  // namespace

// IDA_EA: 0x844F016
// IDA_NAME: _ZN17DB_FindFactoryHubC2Ev
// DEMANGLED: DB_FindFactoryHub::DB_FindFactoryHub(void)
DB_FindFactoryHub::DB_FindFactoryHub() {}

// IDA_EA: 0x845D264
// IDA_NAME: _ZN17DB_FindFactoryHubD2Ev
// DEMANGLED: DB_FindFactoryHub::~DB_FindFactoryHub()
// IDA_EA: 0x845D294
// IDA_NAME: _ZN17DB_FindFactoryHubD0Ev
// DEMANGLED: DB_FindFactoryHub::~DB_FindFactoryHub() deleting destructor
DB_FindFactoryHub::~DB_FindFactoryHub() {}

// IDA_EA: 0x8436BA0
// IDA_NAME: _ZN17DB_FindFactoryHub8dispatchEiiP6Stream
// DEMANGLED: DB_FindFactoryHub::dispatch(int,int,Stream *)
bool DB_FindFactoryHub::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;

    SIG_FIND_FACTORY_HUB_LOCAL payload;
    memset(&payload, 0, sizeof(payload));
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(kDbHandleGame),
        static_cast<ENUM_SERVER_GROUP>(0));
    char source_name[30];
    char escaped_name[59];
    copy_fixed_name(source_name, payload.charac_name, sizeof(payload.charac_name));
    memset(escaped_name, 0, sizeof(escaped_name));
    db_handle->escape_string(escaped_name, source_name);

    db_handle->set_query(
        "seLect m_id from charac_info where charac_name='%s' limit 1",
        escaped_name);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->fetch() != true) {
        return false;
    }

    int found_account_id_signed = 0;
    if (db_handle->get_int(0, found_account_id_signed) != true) {
        return false;
    }
    const uint32_t found_account_id =
        static_cast<uint32_t>(found_account_id_signed);

    if (payload.account_id != found_account_id) {
        Stream* out = GlobalData::stream_pool()->Acquire("DBThread.cpp", 33363);
        CStreamGuard guard(out, true);
        *guard << kInterSignalFindFactoryHubFatigueCheck;
        *guard << user_idx;
        SIG_FIND_FACTORY_HUB_FATIGUE_CHECK_LOCAL* result =
            static_cast<SIG_FIND_FACTORY_HUB_FATIGUE_CHECK_LOCAL*>(
                guard->GetInBufferUntyped(
                    sizeof(SIG_FIND_FACTORY_HUB_FATIGUE_CHECK_LOCAL)));
        memset(result, 0, sizeof(*result));
        result->account_id = found_account_id;
        result->fatigue_check = payload.fatigue_check;
        strncpy(result->charac_name, payload.charac_name,
                sizeof(result->charac_name));
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    }

    return true;
}
