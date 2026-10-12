#include "old_data/db/event/db_event_save_give_grow_creature.h"

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

const int kDbSignalEventSaveGiveGrowCreature = 0x317;

}  // namespace

// IDA_EA: 0x844FDFA
// IDA_NAME: _ZN28DB_EventSaveGiveGrowCreatureC2Ev
DB_EventSaveGiveGrowCreature::DB_EventSaveGiveGrowCreature() {}

// IDA_EA: 0x845AD38 / 0x845AD68
// IDA_NAME: _ZN28DB_EventSaveGiveGrowCreatureD2Ev / _ZN28DB_EventSaveGiveGrowCreatureD0Ev
DB_EventSaveGiveGrowCreature::~DB_EventSaveGiveGrowCreature() {}

// IDA_EA: 0x8449A58
// IDA_NAME: _ZN28DB_EventSaveGiveGrowCreature8dispatchEiiP6Stream
bool DB_EventSaveGiveGrowCreature::dispatch(int signal,
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
        "inSert into event_1208_growthcreature(m_id ,server_id, charac_no, job_type, charac_name, occ_date) values(%s, %d, %d, %d, '%s', now())",
        NumberToString(account_id, 0),
        static_cast<int>(event_info->job),
        static_cast<int>(event_info->charac_no),
        static_cast<int>(event_info->creature_id),
        reinterpret_cast<const char*>(event_info->creature_name));
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x8449B32
// IDA_NAME: _ZN28DB_EventSaveGiveGrowCreature11makeRequestEjRN30SIG_GIVE_GROWTH_CREATURE_EVENT15eventCharacInfoE
void DB_EventSaveGiveGrowCreature::makeRequest(
    unsigned int account_id,
    SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo& event_info) {
    CStreamGuard guard(
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 47491), true);
    *guard << kDbSignalEventSaveGiveGrowCreature;
    *guard << -1;
    *guard << account_id;
    SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo* const record =
        guard->GetInBuffer<SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo>();
    std::memset(record, 0, 4u);
    std::memcpy(record, &event_info, sizeof(*record));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
