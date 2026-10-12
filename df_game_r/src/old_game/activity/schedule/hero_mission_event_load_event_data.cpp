#include "old_game/activity/schedule/hero_mission_runtime.h"

#include "common/manager/global_data.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/db/taiwan/taiwan_db_payloads.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

// IDA_EA: 0x816709E
// IDA_NAME: _ZN16HeroMissionEvent13loadEventDataEP5CUser
// DEMANGLED: HeroMissionEvent::loadEventData(CUser *)
void HeroMissionEvent::loadEventData(CUser* user) {
    if (IsEventing(0) != 1) {
        return;
    }

    if (!user) {
        LogManager::logFormat(
            1,
            "localtaiwan/Event/EventHeroMission.cpp",
            "void HeroMissionEvent::loadEventData(CUser*)",
            652,
            "[HeroMission] User is null.");
        return;
    }

    Stream* const stream = GlobalData::s_stream_pool->Acquire(
        "localtaiwan/Event/EventHeroMission.cpp", 657);
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard,
                            TaiwanInternalPack::LOAD_HERO_MISSION_EVENT_DATA,
                            user->GetUID());
    Taiwan::SigLoadHeroMissionData* const payload =
        guard.GetInBuffer<Taiwan::SigLoadHeroMissionData>();
    if (!payload) {
        LogManager::logFormat(
            1,
            "localtaiwan/Event/EventHeroMission.cpp",
            "void HeroMissionEvent::loadEventData(CUser*)",
            662,
            "[HeroMission] SigLoadHeroMissionData is null.");
        return;
    }

    payload->init();
    payload->charac_no = static_cast<unsigned int>(user->get_charac_no(-1));
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}
