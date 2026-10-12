#include "old_data/Login/inter_returns/statistic_manager_runtime.h"

#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

// IDA_EA: 0x82AA89E
// IDA_NAME: _ZN13CStatisticMgrC2Ev
CStatisticMgr::CStatisticMgr()
    : level_dungeon_play_(), dungeon_exp_() {
}
