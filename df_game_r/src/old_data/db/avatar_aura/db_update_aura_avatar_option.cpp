#include "old_data/db/avatar_aura/db_update_aura_avatar_option.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


// IDA_EA: 0x844FA26
// IDA_NAME: _ZN25DB_UpdateAuraAvatarOptionC2Ev
// DEMANGLED: DB_UpdateAuraAvatarOption::DB_UpdateAuraAvatarOption(void)
DB_UpdateAuraAvatarOption::DB_UpdateAuraAvatarOption() {}

// IDA_EA: 0x845B5C0
// IDA_NAME: _ZN25DB_UpdateAuraAvatarOptionD2Ev
// DEMANGLED: DB_UpdateAuraAvatarOption::~DB_UpdateAuraAvatarOption()
// IDA_EA: 0x845B5F0
// IDA_NAME: _ZN25DB_UpdateAuraAvatarOptionD0Ev
// DEMANGLED: DB_UpdateAuraAvatarOption::~DB_UpdateAuraAvatarOption() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_UpdateAuraAvatarOption::~DB_UpdateAuraAvatarOption() {}

// IDA_EA: 0x8444E1E
// IDA_NAME: _ZN25DB_UpdateAuraAvatarOption8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateAuraAvatarOption::dispatch(int,int,Stream *)
bool DB_UpdateAuraAvatarOption::dispatch(int signal,
                                         int user_idx,
                                         Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    int charac_no = 0;
    int option_type = 0;
    int value = 0;
    stream->get_binary(&charac_no, sizeof(charac_no));
    stream->get_binary(&option_type, sizeof(option_type));
    stream->get_binary(&value, sizeof(value));

    db_handle->set_query(
        "upDate aura_avatar_option set value_1=%d where charac_no=%u and option_type = %d",
        value,
        charac_no,
        option_type);
    if (db_handle->exec(true) != true ||
        !db_handle->getAffectedRowCount()) {
        db_handle->set_query(
            "inSert into aura_avatar_option(charac_no,option_type,value_1) values(%u,%d,%d)",
            charac_no,
            option_type,
            value);
        db_handle->exec(true);
    }
    return true;
}

// IDA_EA: 0x8444F26
// IDA_NAME: _ZN25DB_UpdateAuraAvatarOption11makeRequestEijii
// DEMANGLED: DB_UpdateAuraAvatarOption::makeRequest(int,unsigned int,int,int)
void DB_UpdateAuraAvatarOption::makeRequest(int user_idx,
                                            unsigned int charac_no,
                                            int option_type,
                                            int value) {
    (void)user_idx;

    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 44022);
    CStreamGuard guard(stream, true);
    *guard << 687;
    *guard << -1;
    *guard << charac_no;
    *guard << option_type;
    *guard << value;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
