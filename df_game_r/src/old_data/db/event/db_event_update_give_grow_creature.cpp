#include "old_data/db/event/db_event_update_give_grow_creature.h"

#include <cstring>

#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_data/db/core/mysql.h"
#include "infra/network/packet/growth_creature_event_stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

namespace {

// The original update producer intentionally queues through signal 0x317 as
// well; DBTask retains the distinct slot-792 receiver ABI.
const int kDbSignalEventUpdateGiveGrowCreature = 0x317;

}  // namespace

// IDA_EA: 0x844FE16
// IDA_NAME: _ZN30DB_EventUpdateGiveGrowCreatureC2Ev
DB_EventUpdateGiveGrowCreature::DB_EventUpdateGiveGrowCreature() {}

// IDA_EA: 0x845ACEA / 0x845AD1A
// IDA_NAME: _ZN30DB_EventUpdateGiveGrowCreatureD2Ev / _ZN30DB_EventUpdateGiveGrowCreatureD0Ev
DB_EventUpdateGiveGrowCreature::~DB_EventUpdateGiveGrowCreature() {}

// IDA_EA: 0x8449C54
// IDA_NAME: _ZN30DB_EventUpdateGiveGrowCreature8dispatchEiiP6Stream
bool DB_EventUpdateGiveGrowCreature::dispatch(int signal,
                                              int user_idx,
                                              Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(9), static_cast<ENUM_SERVER_GROUP>(0));
    unsigned int account_id = 0;
    *stream >> account_id;
    SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo* const event_info =
        stream->GetOutBuffer<SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo>();
    db_handle->set_query(
        "upDate event_1208_growthcreature set charac_name = %s) where m_id = %s and server_id =%d and charac_no = %d",
        reinterpret_cast<const char*>(event_info->creature_name),
        NumberToString(account_id, 0),
        static_cast<int>(event_info->job),
        static_cast<int>(event_info->charac_no));
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x8449D1C
// IDA_NAME: _ZN30DB_EventUpdateGiveGrowCreature11makeRequestEjRN30SIG_GIVE_GROWTH_CREATURE_EVENT15eventCharacInfoE
void DB_EventUpdateGiveGrowCreature::makeRequest(
    unsigned int account_id,
    SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo& event_info) {
    CStreamGuard guard(
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 47530), true);
    *guard << kDbSignalEventUpdateGiveGrowCreature;
    *guard << -1;
    *guard << account_id;
    SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo* const record =
        guard->GetInBuffer<SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo>();
    std::memset(record, 0, 4u);
    std::memcpy(record, &event_info, sizeof(*record));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
