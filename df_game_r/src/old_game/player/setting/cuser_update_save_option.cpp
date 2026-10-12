#include "old_game/player/core/cuser.h"

#include "old_game/player/setting/character_option.h"
#include "old_data/db/character/character_option_payload.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>
#include <stdint.h>

// IDA_EA: 0x868F03C
// IDA_NAME: _ZNK5CUser18GetCurCharacOptionER15CharacterOption
// DEMANGLED: CUser::GetCurCharacOption(CharacterOption &)const
bool CUser::GetCurCharacOption(CharacterOption& out) const {
    const _Charac_info* const selected = getCurCharacR();
    if (!selected) {
        return true;
    }
    out = selected->fields_.character_option_;
    return false;
}

// IDA_EA: 0x8652DDE
// IDA_NAME: _ZN5CUser17_updateSaveOptionEv
// DEMANGLED: CUser::_updateSaveOption(void)
void CUser::_updateSaveOption() {
    Stream* const stream =
        GlobalData::stream_pool()->Acquire("user.cpp", 7437);
    CStreamGuard guard(stream, true);
    *guard << 693;
    *guard << this->GetUID();

    SIG_CHARACTER_OPTION* const payload =
        guard.GetInBuffer<SIG_CHARACTER_OPTION>();
    std::memset(payload, 0, sizeof(*payload));
    payload->charac_no = static_cast<unsigned int>(this->getCurCharacNo());
    GetCurCharacOption(payload->option);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    this->setSaveCharacOption(false);
}
