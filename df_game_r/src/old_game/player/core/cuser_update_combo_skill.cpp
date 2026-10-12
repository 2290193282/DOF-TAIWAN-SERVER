#include "common/manager/global_data.h"
#include "common/config.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


// IDA_EA: 0x8692396
// IDA_NAME: _ZN5CUser16updateComboSkillEv
// DEMANGLED: CUser::updateComboSkill(void)
// Active reconstructed-tree ABI: _ZN5CUser16updateComboSkillEv.
void CUser::updateComboSkill() {
    if (this->get_charac_job() !=
        oldsrv::config::kCharacterJobDemonicSwordman) {
        return;
    }

    Stream* const stream = GlobalData::stream_pool()->Acquire("user.cpp", 46743);
    CStreamGuard guard(stream, true);
    guard << 797;
    guard << -1;

    SIG_COMBO_SKILL* const payload =
        guard.GetInBuffer<SIG_COMBO_SKILL>();
    payload->reset();
    this->getCurCharacSkillR()->getComboSkillSaveData(payload);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
