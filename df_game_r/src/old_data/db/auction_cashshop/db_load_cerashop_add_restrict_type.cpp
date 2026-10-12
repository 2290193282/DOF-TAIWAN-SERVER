#include "old_data/db/auction_cashshop/db_load_cerashop_add_restrict_type.h"

#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "old_data/db/core/mysql.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


namespace {

enum {
    kDbSignalLoadCerashopAddRestrictType = 833,
    kAccountDbHandle = 1,
    kCharacterDbHandle = 2,
};

bool SendLoadResult(MySQL* database,
                    int user_idx,
                    unsigned int restrict_scope) {
    const int row_count = database->get_n_rows();
    if (row_count <= 0) {
        return true;
    }

    Stream* const result_stream = GlobalData::stream_pool()->Acquire(
        "CerashopAddRestrictType.cpp", restrict_scope ? 474 : 444);
    CStreamGuard guard(result_stream, true);
    *guard << kDbSignalLoadCerashopAddRestrictType;
    *guard << user_idx;
    SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPE* const result =
        guard->GetInBuffer<SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPE>();
    result->row_count = static_cast<uint32_t>(row_count);
    result->restrict_scope = restrict_scope;
    for (int index = 0; index < row_count; ++index) {
        SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPE::RestrictEntry& entry =
            result->entries[index];
        if (!database->fetch() ||
            !database->get_int(0, entry.ipg_no) ||
            !database->get_int(1, entry.count) ||
            !database->get_int(2, entry.next_date)) {
            return false;
        }
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

}  // namespace

// IDA_EA: 0x844FF82
// IDA_NAME: _ZN30DB_LoadCerashopAddRestrictTypeC2Ev
DB_LoadCerashopAddRestrictType::DB_LoadCerashopAddRestrictType() {}

// IDA_EA: 0x80E46DE
// IDA_NAME: _ZN30DB_LoadCerashopAddRestrictTypeD2Ev
// IDA_EA: 0x80E470E
// IDA_NAME: _ZN30DB_LoadCerashopAddRestrictTypeD0Ev
DB_LoadCerashopAddRestrictType::~DB_LoadCerashopAddRestrictType() {}

// IDA_EA: 0x80E1ACA
// IDA_NAME: _ZN30DB_LoadCerashopAddRestrictType8dispatchEiiP6Stream
bool DB_LoadCerashopAddRestrictType::dispatch(int /*signal*/,
                                              int user_idx,
                                              Stream* stream) {
    unsigned int account_id = 0;
    unsigned int charac_no = 0;
    *stream >> account_id;
    *stream >> charac_no;

    MySQL* const account_database = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(kAccountDbHandle),
        static_cast<ENUM_SERVER_GROUP>(0));
    if (!account_database) {
        return false;
    }
    account_database->set_query(
        "seLect ipg_no, count, next_date from account_cerashop_restrict "
        "where m_id = %u and end_date > UNIX_TIMESTAMP() limit 10",
        account_id);
    if (!account_database->exec(true)) {
        return false;
    }
    if (!SendLoadResult(account_database, user_idx, 0)) {
        return false;
    }

    MySQL* const character_database = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(kCharacterDbHandle),
        static_cast<ENUM_SERVER_GROUP>(0));
    if (!character_database) {
        return false;
    }
    character_database->set_query(
        "seLect ipg_no, count, next_date from charac_cerashop_restrict "
        "where charac_no = %u and end_date > UNIX_TIMESTAMP() limit 10",
        charac_no);
    if (!character_database->exec(true)) {
        return false;
    }
    return SendLoadResult(character_database, user_idx, 1);
}

// IDA_EA: 0x80E1FF6
// IDA_NAME: _ZN30DB_LoadCerashopAddRestrictType11makeRequestEP5CUser
void DB_LoadCerashopAddRestrictType::makeRequest(CUser* user) {
    Stream* const stream = GlobalData::stream_pool()->Acquire(
        "CerashopAddRestrictType.cpp", 500);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalLoadCerashopAddRestrictType;
    *guard << user->GetUID();
    *guard << static_cast<unsigned int>(user->get_acc_id());
    *guard << static_cast<unsigned int>(user->getCurCharacNo());
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
