#include "old_game/player/core/cuser.h"

#include <string>

#include "common/manager/global_data.h"
#include "old_data/PVF/arad/arad_singleton.h"
#include "old_data/PVF/data/arad_data_manager.h"
#include "old_data/PVF/data/arad_statistics_event_item_list.h"
#include "old_data/db/event/reward_event_request.h"
#include "old_data/db/legacy_dispatch/arad_dispatcher.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

// IDA_EA: 0x8692B84
// IDA_NAME: _ZN5CUser20SendAradEventItemLogEmi
// DEMANGLED: CUser::SendAradEventItemLog(unsigned long,int)
void CUser::SendAradEventItemLog(unsigned long item_index, int count) {
    const std::string script_name("Arad_StatisticsEventItemList");
    ARAD::Arad_DataManager* const data_manager =
        ARAD::Singleton<ARAD::Arad_DataManager>::Get();
    ARAD::Arad_StatisticsEventItemList* const statistics =
        static_cast<ARAD::Arad_StatisticsEventItemList*>(
            data_manager->findGameScript(script_name));
    if (!statistics->needStatistics(item_index)) {
        return;
    }

    arad::SigAradEventItemLog payload;
    payload.item_id = static_cast<unsigned int>(item_index);
    payload.item_count = count;
    payload.account_id = static_cast<unsigned int>(get_acc_id());
    payload.charac_no = static_cast<unsigned int>(get_charac_no(-1));

    CStreamGuard guard(GlobalData::s_stream_pool->Acquire("user.cpp", 47559),
                       true);
    ARAD::DISPATCHER::make_internal_stream_jpn(
        guard,
        ARAD::kInternalPacketAradEventItemLog,
        GetUID());
    guard->put_binary(&payload, static_cast<int>(sizeof(payload)));
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, guard);
}
