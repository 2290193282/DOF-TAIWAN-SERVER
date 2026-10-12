#include "old_data/db/pvp_party/db_guild_exp_book_delete.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

namespace {

enum {
    kDbSignalGuildExpBookDelete = 521,
};

}  // namespace

// IDA_EA: 0x844F4CA
// IDA_NAME: _ZN21DB_GuildExpBookDeleteC2Ev
DB_GuildExpBookDelete::DB_GuildExpBookDelete() {}

// IDA_EA: 0x845C3C4
// IDA_NAME: _ZN21DB_GuildExpBookDeleteD2Ev
// IDA_EA: 0x845C3F4
// IDA_NAME: _ZN21DB_GuildExpBookDeleteD0Ev
DB_GuildExpBookDelete::~DB_GuildExpBookDelete() {}

// IDA_EA: 0x843C35A
// IDA_NAME: _ZN21DB_GuildExpBookDelete8dispatchEiiP6Stream
// DEMANGLED: DB_GuildExpBookDelete::dispatch(int, int, Stream*)
bool DB_GuildExpBookDelete::dispatch(int signal,
                                     int user_idx,
                                     Stream* stream) {
    (void)signal;
    (void)user_idx;

    unsigned int charac_no = 0;
    int guild_secede = 0;
    int server_group = 0;
    *stream >> charac_no;
    *stream >> guild_secede;
    *stream >> server_group;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( server_group));
    db_handle->set_query(
        "upDate charac_info set guild_secede = %d where charac_no = %u",
        guild_secede,
        charac_no);
    db_handle->exec(true);
    return true;
}

// IDA_EA: 0x843C408
// IDA_NAME: _ZN21DB_GuildExpBookDelete11makeRequestEji17ENUM_SERVER_GROUP
// DEMANGLED: DB_GuildExpBookDelete::makeRequest(unsigned int, int, ENUM_SERVER_GROUP)
void DB_GuildExpBookDelete::makeRequest(unsigned int charac_no,
                                         int guild_secede,
                                         ENUM_SERVER_GROUP server_group) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 36906);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalGuildExpBookDelete;
    *guard << -1;
    *guard << charac_no;
    *guard << guild_secede;
    *guard << static_cast<int>(server_group);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
