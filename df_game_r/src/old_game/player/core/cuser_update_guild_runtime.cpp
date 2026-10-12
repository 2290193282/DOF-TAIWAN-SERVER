#include "old_game/player/core/cuser.h"

#include "common/manager/global_data.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

// IDA_EA: 0x867CD20
// IDA_NAME: _ZN5CUser21_UpdateUserInfo4GuildEv
// DEMANGLED: CUser::_UpdateUserInfo4Guild(void)
void CUser::_UpdateUserInfo4Guild() {
    if (!get_charac_guildkey()) {
        return;
    }

    Stream* const stream =
        GlobalData::s_stream_pool->Acquire("user.cpp", 31039);
    CStreamGuard guard(stream, true);

    (*guard) << 99;
    (*guard) << -1;
    (*guard) << get_charac_guildkey();
    (*guard) << static_cast<unsigned int>(get_acc_id());
    (*guard) << getCurCharacNo();
    (*guard) << static_cast<int>(
        static_cast<signed char>(getCurCharacGrowType()));
    (*guard) << get_charac_level();

    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}
