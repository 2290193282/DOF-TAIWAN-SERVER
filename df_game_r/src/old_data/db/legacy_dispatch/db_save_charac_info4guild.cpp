#include "old_data/db/character/db_save_charac_info4guild.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"

namespace {

enum {
    kSigSaveCharacInfo4GuildSize = 20,
};

#define DB_GUILD_INFO_I32(payload, offset) \
    (*reinterpret_cast<int*>((payload) + (offset)))

}  // namespace

// IDA_EA: 0x844E152
// IDA_NAME: _ZN23DB_SaveCharacInfo4GuildC2Ev
// DEMANGLED: DB_SaveCharacInfo4Guild::DB_SaveCharacInfo4Guild(void)
DB_SaveCharacInfo4Guild::DB_SaveCharacInfo4Guild() {}

// IDA_EA: 0x845F8C8
// IDA_NAME: _ZN23DB_SaveCharacInfo4GuildD2Ev
// DEMANGLED: DB_SaveCharacInfo4Guild::~DB_SaveCharacInfo4Guild()
// IDA_EA: 0x845F8F8
// IDA_NAME: _ZN23DB_SaveCharacInfo4GuildD0Ev
// DEMANGLED: DB_SaveCharacInfo4Guild::~DB_SaveCharacInfo4Guild() deleting destructor
DB_SaveCharacInfo4Guild::~DB_SaveCharacInfo4Guild() {}

// IDA_EA: 0x841B624
// IDA_NAME: _ZN23DB_SaveCharacInfo4Guild8dispatchEiiP6Stream
// DEMANGLED: DB_SaveCharacInfo4Guild::dispatch(int,int,Stream *)
bool DB_SaveCharacInfo4Guild::dispatch(int signal,
                                       int user_idx,
                                       Stream* stream) {
    (void)signal;
    (void)user_idx;

    if (!stream->enable_out(kSigSaveCharacInfo4GuildSize)) {
        return false;
    }
    char* payload = stream->get_buffer();
    stream->out_ptr(kSigSaveCharacInfo4GuildSize);
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(8), static_cast<ENUM_SERVER_GROUP>(0));
    GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1), static_cast<ENUM_SERVER_GROUP>(0));
    db_handle->set_query(
        "upDate guild_member set grow_type=%d,lev=%d where guild_id=%d and charac_no=%d",
        DB_GUILD_INFO_I32(payload, 12),
        DB_GUILD_INFO_I32(payload, 16),
        DB_GUILD_INFO_I32(payload, 0),
        DB_GUILD_INFO_I32(payload, 8));
    return db_handle->exec(true) == true;
}

#undef DB_GUILD_INFO_I32
