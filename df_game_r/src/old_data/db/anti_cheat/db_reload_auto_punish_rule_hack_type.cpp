#include "old_data/db/anti_cheat/db_reload_auto_punish_rule_hack_type.h"

#include "old_data/db/core/mysql.h"
#include "old_data/Login/security/auto_punish_hack_type_abi.h"
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
    kDbTaskSignalReloadAutoPunishRuleHackType = 265,
    kDispatchSignalAutoPunishRuleHackType = 265,
    kAutoPunishDbHandle = 6,
    kAutoPunishHackTypeLimit = 500,
};

bool IsOldHackTypeRule(unsigned int hack_type, unsigned int hack_sub_type) {
    return hack_type > 100u && hack_type <= 4000u &&
           (!hack_sub_type ||
            (hack_sub_type > 100u && hack_sub_type <= 4000u));
}

}  // namespace

// IDA_EA: 0x844EA2E
// IDA_NAME: _ZN31DB_ReloadAutoPunishRuleHackTypeC2Ev
// DEMANGLED: DB_ReloadAutoPunishRuleHackType::DB_ReloadAutoPunishRuleHackType(void)
DB_ReloadAutoPunishRuleHackType::DB_ReloadAutoPunishRuleHackType() {}

// IDA_EA: 0x845E0B6
// IDA_NAME: _ZN31DB_ReloadAutoPunishRuleHackTypeD2Ev
// DEMANGLED: DB_ReloadAutoPunishRuleHackType::~DB_ReloadAutoPunishRuleHackType()
// IDA_EA: 0x845E0E6
// IDA_NAME: _ZN31DB_ReloadAutoPunishRuleHackTypeD0Ev
// DEMANGLED: DB_ReloadAutoPunishRuleHackType::~DB_ReloadAutoPunishRuleHackType() deleting destructor
DB_ReloadAutoPunishRuleHackType::~DB_ReloadAutoPunishRuleHackType() {}

// IDA_EA: 0x842D16C
// IDA_NAME: _ZN31DB_ReloadAutoPunishRuleHackType8dispatchEiiP6Stream
// DEMANGLED: DB_ReloadAutoPunishRuleHackType::dispatch(int,int,Stream *)
bool DB_ReloadAutoPunishRuleHackType::dispatch(int signal,
                                               int user_idx,
                                               Stream* stream) {
    (void)signal;
    (void)user_idx;
    (void)stream;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(kAutoPunishDbHandle),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect hack_type,cnt,etc,hack_sub_type,hack_sub_cnt,apply_flag, ip_cnt from auto_punish_hack_info where apply_flag > 0");
    if (db_handle->exec(true) != true) {
        return false;
    }

    Stream* out_stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 28820);
    CStreamGuard guard(out_stream, true);
    *guard << kDispatchSignalAutoPunishRuleHackType;
    *guard << -1;

    SIG_AUTO_PUNISH_RULE_HACK_TYPE* payload =
        static_cast<SIG_AUTO_PUNISH_RULE_HACK_TYPE*>(
            guard->GetInBufferUntyped(sizeof(SIG_AUTO_PUNISH_RULE_HACK_TYPE)));
    if (!payload) {
        return false;
    }
    memset(payload, 0, sizeof(SIG_AUTO_PUNISH_RULE_HACK_TYPE));

    const unsigned int row_count =
        static_cast<unsigned int>(db_handle->get_n_rows());
    const unsigned int limit =
        row_count < kAutoPunishHackTypeLimit ? row_count
                                             : kAutoPunishHackTypeLimit;
    for (unsigned int row = 0; row < limit; ++row) {
        if (db_handle->fetch() != true) {
            return false;
        }

        unsigned int hack_type = 0;
        unsigned int count = 0;
        unsigned long long etc = 0;
        unsigned int hack_sub_type = 0;
        unsigned int hack_sub_count = 0;
        unsigned int apply_flag = 0;
        unsigned int ip_count = 0;
        if (db_handle->get_uint(0, hack_type) != true ||
            db_handle->get_uint(1, count) != true ||
            db_handle->get_ulonglong(2, etc) != true ||
            db_handle->get_uint(3, hack_sub_type) != true ||
            db_handle->get_uint(4, hack_sub_count) != true ||
            db_handle->get_uint(5, apply_flag) != true ||
            db_handle->get_uint(6, ip_count) != true) {
            return false;
        }

        if (!hack_type) {
            payload->total_count = count;
            payload->total_etc = etc;
            continue;
        }
        if (!IsOldHackTypeRule(hack_type, hack_sub_type)) {
            continue;
        }
        if (!hack_sub_type) {
            hack_sub_count = 0;
        }

        const uint16_t entry_index = payload->rule_count;
        SIG_AUTO_PUNISH_RULE_HACK_TYPE_ENTRY& entry =
            payload->entries[entry_index];
        entry.hack_type = static_cast<uint16_t>(hack_type);
        entry.count = static_cast<uint16_t>(count);
        entry.hack_sub_type = static_cast<uint16_t>(hack_sub_type);
        entry.hack_sub_count = static_cast<uint16_t>(hack_sub_count);
        entry.apply_flag = static_cast<uint16_t>(apply_flag);
        entry.ip_count = ip_count;
        entry.etc_low = static_cast<uint32_t>(etc);
        payload->rule_count = entry_index + 1;
    }

    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x842D610
// IDA_NAME: _ZN31DB_ReloadAutoPunishRuleHackType11makeRequestEv
// DEMANGLED: DB_ReloadAutoPunishRuleHackType::makeRequest(void)
void DB_ReloadAutoPunishRuleHackType::makeRequest() {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 28898);
    CStreamGuard guard(stream, true);

    // Old DBTask stores this dispatcher at this+266; active DBTask has a
    // leading queue_idx_ field, so the matching active DB_Q slot is 265.
    // The DB result sent to DISPATCH_Q still uses the old signal 9.
    *guard << kDbTaskSignalReloadAutoPunishRuleHackType;
    *guard << -1;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
