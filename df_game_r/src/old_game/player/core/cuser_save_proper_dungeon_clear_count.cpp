#include "common/manager/global_data.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


struct SIG_SAVE_BLOOD_CLEAR_COUNT;

// IDA_EA: 0x8681B3A
// IDA_NAME: _ZN5CUser28_saveProperDungeonClearCountEjs
// DEMANGLED: CUser::_saveProperDungeonClearCount(unsigned int,short)
void CUser::_saveProperDungeonClearCount(unsigned int charac_no,
                                       short clear_count) {
    Stream* const stream =
        GlobalData::stream_pool()->Acquire("user.cpp", 34253);
    CStreamGuard guard(stream, true);
    guard << 468;
    guard << this->GetUID();
    unsigned char* const payload = reinterpret_cast<unsigned char*>(
        guard.GetInBuffer<SIG_SAVE_BLOOD_CLEAR_COUNT>());
    *reinterpret_cast<unsigned int*>(payload) = charac_no;
    *reinterpret_cast<short*>(payload + 4) = clear_count;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
