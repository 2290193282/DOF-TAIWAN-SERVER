#include "old_game/player/core/cuser.h"

#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/map/datamanager/dungeon_runtime.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>
#include <stdint.h>


struct SIG_UPDATE_DIMENSION;

// IDA_EA: 0x8651FBA
// IDA_NAME: _ZN5CUser20UpdateDimensionInoutEv
// DEMANGLED: CUser::UpdateDimensionInout(void)
void CUser::UpdateDimensionInout() {
    Stream* const stream =
        GlobalData::stream_pool()->Acquire("user.cpp", 7120);
    CStreamGuard guard(stream, true);
    *guard << 335;
    *guard << this->GetUID();

    unsigned char* const payload = reinterpret_cast<unsigned char*>(
        guard.GetInBuffer<SIG_UPDATE_DIMENSION>());
    std::memset(payload, 0, 16);
    *reinterpret_cast<unsigned int*>(payload) =
        static_cast<unsigned int>(getCurCharacNo());

    CDataManager* const data_manager = G_CDataManager();
    for (int index = 0; index <= 5; ++index) {
        const int current_value =
            getDemensionInoutValue(static_cast<char>(index));
        if (current_value < 0 ||
            current_value > data_manager->get_dimensionInout(index) + 10) {
            payload[index + 4] = 0;
            cMyTrace trace("void CUser::UpdateDimensionInout()", 7134, 0);
            trace("DIMENSION DUNGEON INOUT_COUNT ABNORMAL USER!! "
                  "CHARAC_NO(%d)",
                  *reinterpret_cast<unsigned int*>(payload));
        } else {
            payload[index + 4] = static_cast<unsigned char>(current_value);
        }
    }

    // SIG_UPDATE_DIMENSION +0x0a is the schema's dungeon7 byte.  140828
    // assigns it to dungeon 122's remaining-entry count.
    payload[10] = static_cast<unsigned char>(
        getUltimateInoutValue(0));
    uint8_t limited_inout = 0u;
    const bool save_limited = dungeon_runtime::IsInCountDirty(this) &&
        dungeon_runtime::PackInCountForSave(this, &limited_inout);
    payload[11] = limited_inout;
    payload[12] = static_cast<unsigned char>(
        getUltimateInoutValue(2));
    payload[13] = static_cast<unsigned char>(
        IsSaveDemensionInoutValue());
    payload[14] = static_cast<unsigned char>(
        IsSaveBloodInoutValue());
    payload[15] = static_cast<unsigned char>(save_limited);
    SetSaveDemensionInoutValue(false);
    SetSaveBloodInoutValue(false);
    const int queued = GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    if (queued == 1 && save_limited) {
        dungeon_runtime::MarkInCountSaved(this, limited_inout);
    }
}
