#include "old_game/player/core/cuser.h"

#include "common/manager/global_data.h"
#include "old_data/db/dungeon/blood_best_record_payload.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>


// IDA_EA: 0x8651DE6
// IDA_NAME: _ZN5CUser17UpdateBloodRecordEv
// DEMANGLED: CUser::UpdateBloodRecord(void)
int CUser::UpdateBloodRecord() {
    const _Charac_info* const selected = this->getCurCharacR();
    if (!selected) {
        return 0;
    }

    const CUserBloodBestRecordMap* const records =
        &selected->fields_.blood_best_records_;
    for (CUserBloodBestRecordMap::const_iterator it = records->begin();
         it != records->end();
         ++it) {
        Stream* const stream =
            GlobalData::stream_pool()->Acquire("user.cpp", 7099);
        CStreamGuard guard(stream, true);
        *guard << 320;
        *guard << this->GetUID();

        SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD* const payload =
            guard.GetInBuffer<SIG_UPDATE_BLOOD_DUNGEON_BEST_RECORD>();
        payload->charac_no = static_cast<unsigned int>(this->getCurCharacNo());
        payload->best_time = static_cast<uint32_t>(it->second.time_record);
        payload->best_round = it->second.round_record;
        payload->dungeon_index = it->first;
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    }
    return setBloodBestRecordUpdate(false);
}
