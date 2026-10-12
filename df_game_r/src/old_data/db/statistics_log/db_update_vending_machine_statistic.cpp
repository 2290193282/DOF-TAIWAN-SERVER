#include "old_data/db/statistics_log/db_update_vending_machine_statistic.h"

#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <algorithm>

// IDA_EA: 0x861062C
// IDA_NAME: _ZN24CVendingMachineStatisticC2Ev
// DEMANGLED: CVendingMachineStatistic::CVendingMachineStatistic(void)
CVendingMachineStatistic::CVendingMachineStatistic()
    : excluded_item_indices_(), use_count_(0), output_counts_() {
    _init();
}

// IDA_EA: 0x8610694
// IDA_NAME: _ZN24CVendingMachineStatisticD2Ev
// DEMANGLED: CVendingMachineStatistic::~CVendingMachineStatistic()
CVendingMachineStatistic::~CVendingMachineStatistic() {}

// IDA_EA: 0x822D714
// IDA_NAME: _ZN24CVendingMachineStatistic11incUseCountEv
// DEMANGLED: CVendingMachineStatistic::incUseCount(void)
CVendingMachineStatistic* CVendingMachineStatistic::incUseCount() {
    ++use_count_;
    return this;
}

// IDA_EA: 0x860F5A4
// IDA_NAME: _ZN24CVendingMachineStatistic13addOutputItemEmj
// DEMANGLED: CVendingMachineStatistic::addOutputItem(unsigned long,unsigned int)
int CVendingMachineStatistic::addOutputItem(unsigned long item_index,
                                            unsigned int item_count) {
    if (std::find(excluded_item_indices_.begin(),
                  excluded_item_indices_.end(),
                  item_index) != excluded_item_indices_.end()) {
        return 0;
    }

    std::map<unsigned long, uint32_t>::iterator found =
        output_counts_.find(item_index);
    if (found == output_counts_.end()) {
        output_counts_.insert(std::make_pair(item_index,
                                             static_cast<uint32_t>(item_count)));
    } else {
        found->second += item_count;
    }
    return 1;
}

// IDA_EA: 0x860F6DE
// IDA_NAME: _ZN24CVendingMachineStatistic16updateDataIntoDBEv
// DEMANGLED: CVendingMachineStatistic::updateDataIntoDB(void)
void CVendingMachineStatistic::updateDataIntoDB() {
    CStreamGuard stream(
        GlobalData::s_stream_pool->Acquire("Statistics.cpp", 1572), true);
    stream << 355;
    stream << -1;
    stream << static_cast<int>(use_count_);
    stream << static_cast<char>(output_counts_.size());

    for (std::map<unsigned long, uint32_t>::const_iterator it =
             output_counts_.begin();
         it != output_counts_.end();
         ++it) {
        stream << it->first;
        stream << it->second;
    }

    GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, stream);
    _init();
}

// IDA_EA: 0x86105AC
// IDA_NAME: _ZN24CVendingMachineStatistic5_initEv
// DEMANGLED: CVendingMachineStatistic::_init(void)
int CVendingMachineStatistic::_init() {
    use_count_ = 0;
    excluded_item_indices_.push_back(7451ul);
    excluded_item_indices_.push_back(7452ul);
    excluded_item_indices_.push_back(7453ul);
    output_counts_.clear();
    return 0;
}

// IDA_EA: 0x860F8AD
// IDA_NAME: _Z34GetInstanceVendingMachineStatisticv
// DEMANGLED: GetInstanceVendingMachineStatistic()
CVendingMachineStatistic* GetInstanceVendingMachineStatistic() {
    static CVendingMachineStatistic statistic;
    return &statistic;
}

// IDA_EA: 0x844EEAA
// IDA_NAME: _ZN32DB_UpdateVendingMachineStatisticC2Ev
// DEMANGLED: DB_UpdateVendingMachineStatistic::DB_UpdateVendingMachineStatistic(void)
DB_UpdateVendingMachineStatistic::DB_UpdateVendingMachineStatistic() {}

// IDA_EA: 0x845D65A
// IDA_NAME: _ZN32DB_UpdateVendingMachineStatisticD2Ev
// DEMANGLED: DB_UpdateVendingMachineStatistic::~DB_UpdateVendingMachineStatistic()
// IDA_EA: 0x845D68A
// IDA_NAME: _ZN32DB_UpdateVendingMachineStatisticD0Ev
// DEMANGLED: DB_UpdateVendingMachineStatistic::~DB_UpdateVendingMachineStatistic() deleting destructor
DB_UpdateVendingMachineStatistic::~DB_UpdateVendingMachineStatistic() {}

// IDA_EA: 0x8434718
// IDA_NAME: _ZN32DB_UpdateVendingMachineStatistic8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateVendingMachineStatistic::dispatch(int,int,Stream *)
bool DB_UpdateVendingMachineStatistic::dispatch(int signal,
                                                int user_idx,
                                                Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(DB_HANDLE_LOG),
        static_cast<ENUM_SERVER_GROUP>(SERVER_GROUP_DEFAULT));

    int use_count = 0;
    int output_count = 0;
    *stream >> use_count;
    *stream >> output_count;
    if (!use_count) {
        return true;
    }

    db_handle->set_query(
        "inSert into log_vending_stat(occ_date,channel_no,item_id,cnt) values(cast(now() as date),%d,%d,%d)",
        G_CEnvironment()->get_channel_no(),
        0,
        use_count);
    db_handle->exec(true);

    for (unsigned int index = 0;
         index < static_cast<unsigned int>(output_count);
         ++index) {
        int item_id = 0;
        int count = 0;
        *stream >> item_id;
        *stream >> count;
        if (!item_id) {
            break;
        }

        db_handle->set_query(
            "inSert into log_vending_stat(occ_date,channel_no,item_id,cnt) values(cast(now() as date),%d,%d,%d)",
            G_CEnvironment()->get_channel_no(),
            item_id,
            count);
        db_handle->exec(true);
    }

    // IDA @0x8434870 returns 1 after attempted writes; preserve that result.
    return true;
}
