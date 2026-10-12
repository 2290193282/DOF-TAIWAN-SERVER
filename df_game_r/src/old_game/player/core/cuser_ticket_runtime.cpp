#include "old_game/player/core/cuser.h"

#include "common/manager/global_data.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>

// IDA_EA: 0x8678282
// IDA_NAME: _ZN5CUser9AddTicketEib
// DEMANGLED: CUser::AddTicket(int,bool)
void CUser::AddTicket(int village, bool persist) {
    if (!persist) {
        reset_list_.push_back(village);
        return;
    }

    for (std::list<int>::const_iterator it = reset_list_.begin();
         it != reset_list_.end();
         ++it) {
        if (*it == village) {
            return;
        }
    }

    reset_list_.push_back(village);
    Stream* stream = GlobalData::stream_pool()->Acquire("user.cpp", 28733);
    CStreamGuard guard(stream, true);
    *guard << 40;

    *guard << GetUID();

    uint32_t request[2] = {
        static_cast<uint32_t>(getCurCharacNo()),
        static_cast<uint32_t>(village),
    };
    guard->put_binary(request, sizeof(request));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x86785F4
// IDA_NAME: _ZN5CUser11CheckTicketEi
// DEMANGLED: CUser::CheckTicket(int)
int CUser::CheckTicket(int village) {
    if (IsGameMasterMode()) {
        return 1;
    }
    if (G_GameWorld()->IsPVPChannel()) {
        return 1;
    }

    for (std::list<int>::const_iterator ticket = reset_list_.begin();
         ticket != reset_list_.end();
         ++ticket) {
        if (*ticket == village) {
            return 1;
        }
    }
    return 0;
}
