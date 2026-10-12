#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/db/event/db_save_pc_room_play_time_event.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "old_game/player/core/cuser.h"


// IDA_EA: 0x868F726
// IDA_NAME: _ZN5CUser20updatePcRoomPlayTimeEv
// DEMANGLED: CUser::updatePcRoomPlayTime(void)
void CUser::updatePcRoomPlayTime() {
    EventClassify::CEventScriptMng* const event_script_manager =
        G_CDataManager()->get_event_script_mng();
    EventClassify::CPcRoomPlayTime* const event_entity =
        static_cast<EventClassify::CPcRoomPlayTime*>(
            event_script_manager->get_event_entity(92));
    if (event_entity != 0 &&
        event_entity->verifyUserPcRoomPlayEvent(this) == 0) {
        collectPcRoomPlayTime();
        const unsigned char reward_count =
            getPcRoomPlayTimeRewardCnt();
        const unsigned int total_play_time = getTotalPcRoomPlayTime();
        DB_SavePcRoomPlayTimeEvent::makeRequest(
            static_cast<unsigned int>(this->get_acc_id()),
            total_play_time,
            reward_count);
    }
}
