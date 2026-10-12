#include "old_game/player/core/character_load_mercenary.h"
#include "common/manager/global_data.h"
#include "old_data/db/core/db_runtime_payloads.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>
#include <vector>


// IDA_EA: 0x868E36A
// IDA_NAME: _ZN5CUser17SaveMercenaryDataEv
// DEMANGLED: CUser::SaveMercenaryData(void)
int CUser::SaveMercenaryData() {
    std::vector<CHARAC_LOAD_MERCENARY>* const mercenaries =
        &mercenary_info_.mercenaries_;
    if (!mercenaries->empty()) {
        Stream* const stream =
            GlobalData::stream_pool()->Acquire("user.cpp", 43835);
        CStreamGuard guard(stream, true);
        guard << 691;
        guard << this->GetUID();

        unsigned char* const payload = reinterpret_cast<unsigned char*>(
            guard.GetInBuffer<SIG_MERCENARY_DB_DATA>());
        std::memset(payload, 0, 0xBD4u);

        for (unsigned int i = 0;
             i < static_cast<unsigned int>(mercenaries->size());
             ++i) {
            CHARAC_LOAD_MERCENARY& record = (*mercenaries)[i];
            if (record.save_flag != 0) {
                unsigned int& count = *reinterpret_cast<unsigned int*>(payload);
                std::memcpy(payload + 4u + 0x54u * count,
                            &record,
                            sizeof(record));
                ++count;
                record.save_complete();
            }
        }
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    }
    return 1;
}
