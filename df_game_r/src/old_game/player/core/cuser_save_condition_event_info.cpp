#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


struct SIG_SAVE_CONDITION_EVENT;

// IDA_EA: 0x86819FA
// IDA_NAME: _ZN5CUser23_saveConditionEventInfoEjss
// DEMANGLED: CUser::_saveConditionEventInfo(unsigned int,short,short)
void CUser::_saveConditionEventInfo(unsigned int charac_no,
                                  short event_step,
                                  short reward_step) {
    CEventBase* const repeat_event =
        GlobalData::s_event_manager->GetRepeatEvent(113);
    if (repeat_event->IsEventing(0) == 1) {
        Stream* const stream =
            GlobalData::stream_pool()->Acquire("user.cpp", 34134);
        CStreamGuard guard(stream, true);
        guard << 415;
        guard << this->GetUID();

        unsigned char* const payload = reinterpret_cast<unsigned char*>(
            guard.GetInBuffer<SIG_SAVE_CONDITION_EVENT>());
        *reinterpret_cast<unsigned int*>(payload) = charac_no;
        *reinterpret_cast<short*>(payload + 4) = event_step;
        *reinterpret_cast<short*>(payload + 6) = reward_step;
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    }
}
