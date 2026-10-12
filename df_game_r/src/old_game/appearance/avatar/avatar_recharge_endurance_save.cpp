#include "old_game/appearance/avatar/arad_avatar_runtime.h"

#include "old_data/db/legacy_dispatch/arad_dispatcher.h"
#include "old_data/db/legacy_dispatch/arad_dispatcher_request.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

// IDA_EA: 0x8190DEA
// IDA_NAME: _ZN20AvatarRechargeServer23SaveAvatarEnduranceUnitERN4arad26SigAradAvatarEnduranceSaveE
// DEMANGLED: AvatarRechargeServer::SaveAvatarEnduranceUnit(arad::SigAradAvatarEnduranceSave &)
void AvatarRechargeServer::SaveAvatarEnduranceUnit(
    arad::SigAradAvatarEnduranceSave& save) {
    CStreamGuard stream_guard(
        GlobalData::s_stream_pool->Acquire(
            "localjapan/Arad_EmblemReformServer.cpp", 271),
        true);
    ARAD::DISPATCHER::make_internal_stream_jpn(
        stream_guard,
        ARAD::kInternalPacketAvatarEnduranceSave,
        -1);
    stream_guard->put_binary(&save, static_cast<int>(sizeof(save)));
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, stream_guard);
}

// IDA_EA: 0x8190EAE
// IDA_NAME: _ZN20AvatarRechargeServer23SaveAvatarEnduranceUnitEii
// DEMANGLED: AvatarRechargeServer::SaveAvatarEnduranceUnit(int,int)
void AvatarRechargeServer::SaveAvatarEnduranceUnit(int avatar_key,
                                                    int endurance) {
    arad::SigAradAvatarEnduranceSave save;
    save.ui_id = static_cast<uint32_t>(avatar_key);
    save.endurance = endurance;
    SaveAvatarEnduranceUnit(save);
}
