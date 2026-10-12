#include "old_data/db/statistics_log/db_update_item_drop_statistics.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

namespace {

enum {
    kDbSignalUpdateItemDropStatistics = 210,
};

}  // namespace

// IDA_EA: 0x844E78E
// IDA_NAME: _ZN27DB_UpdateItemDropStatisticsC2Ev
DB_UpdateItemDropStatistics::DB_UpdateItemDropStatistics() {}

// IDA_EA: 0x845E7B8
// IDA_NAME: _ZN27DB_UpdateItemDropStatisticsD2Ev
// IDA_EA: 0x845E7E8
// IDA_NAME: _ZN27DB_UpdateItemDropStatisticsD0Ev
DB_UpdateItemDropStatistics::~DB_UpdateItemDropStatistics() {}

// IDA_EA: 0x84285AE
// IDA_NAME: _ZN27DB_UpdateItemDropStatistics8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateItemDropStatistics::dispatch(int, int, Stream*)
bool DB_UpdateItemDropStatistics::dispatch(int signal,
                                           int user_idx,
                                           Stream* stream) {
    (void)signal;
    (void)user_idx;

    int channel_no = 0;
    int drop_type = 0;
    unsigned int stackable[kGradeCount] = {0};
    unsigned int equipment[kGradeCount] = {0};
    unsigned int recipe[kGradeCount] = {0};
    unsigned int artifact[kGradeCount] = {0};
    *stream >> channel_no;
    *stream >> drop_type;
    for (int index = 0; index < kGradeCount; ++index) {
        *stream >> stackable[index];
    }
    for (int index = 0; index < kGradeCount; ++index) {
        *stream >> equipment[index];
    }
    for (int index = 0; index < kGradeCount; ++index) {
        *stream >> recipe[index];
    }
    for (int index = 0; index < kGradeCount; ++index) {
        *stream >> artifact[index];
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(4),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "inSert into log_item_drop(occ_date,channel_no,drop_type,stackable_common,stackable_uncommon,stackable_rare,stackable_unique,stackable_epic,stackable_chronicle,equip_common,equip_uncommon,equip_rare,equip_unique,equip_epic,equip_chronicle,recipe_common,recipe_uncommon,recipe_rare,recipe_unique,recipe_epic,recipe_chronicle,artifact_common,artifact_uncommon,artifact_rare,artifact_unique,artifact_epic,artifact_chronicle) values(cast(now() as date),%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",
        channel_no,
        drop_type,
        stackable[0], stackable[1], stackable[2],
        stackable[3], stackable[4], stackable[5],
        equipment[0], equipment[1], equipment[2],
        equipment[3], equipment[4], equipment[5],
        recipe[0], recipe[1], recipe[2], recipe[3], recipe[4], recipe[5],
        artifact[0], artifact[1], artifact[2],
        artifact[3], artifact[4], artifact[5]);
    return db_handle->exec(true);
}

// IDA_EA: 0x842894C
// IDA_NAME: _ZN27DB_UpdateItemDropStatistics11makeRequestEiiPjS0_S0_S0_
// DEMANGLED: DB_UpdateItemDropStatistics::makeRequest(int, int, unsigned int*, unsigned int*, unsigned int*, unsigned int*)
void DB_UpdateItemDropStatistics::makeRequest(int channel_no,
                                               int drop_type,
                                               unsigned int* stackable,
                                               unsigned int* equipment,
                                               unsigned int* recipe,
                                               unsigned int* artifact) {
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 27156);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalUpdateItemDropStatistics;
    *guard << -1;
    *guard << channel_no;
    *guard << drop_type;
    for (int index = 0; index < kGradeCount; ++index) {
        *guard << stackable[index];
    }
    for (int index = 0; index < kGradeCount; ++index) {
        *guard << equipment[index];
    }
    for (int index = 0; index < kGradeCount; ++index) {
        *guard << recipe[index];
    }
    for (int index = 0; index < kGradeCount; ++index) {
        *guard << artifact[index];
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::LOGDB_Q, guard);
}
