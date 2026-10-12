#include "old_game/pvp/arena/timer_war_room_change_host.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/pvp/battlefield/war_room.h"

// IDA_EA: 0x863C45E
// IDA_NAME: _ZN22TimerWarRoomChangeHostC2Ev
// DEMANGLED: TimerWarRoomChangeHost::TimerWarRoomChangeHost(void)
TimerWarRoomChangeHost::TimerWarRoomChangeHost() : TimerSig() {}

// IDA_EA: 0x8640668
// IDA_NAME: _ZN22TimerWarRoomChangeHostD2Ev
// DEMANGLED: TimerWarRoomChangeHost::~TimerWarRoomChangeHost()
// IDA_EA: 0x8640698
// IDA_NAME: _ZN22TimerWarRoomChangeHostD0Ev
// DEMANGLED: TimerWarRoomChangeHost::~TimerWarRoomChangeHost()
TimerWarRoomChangeHost::~TimerWarRoomChangeHost() {}

// IDA_EA: 0x8634F6E
// IDA_NAME: _ZN22TimerWarRoomChangeHost12dispatch_sigEiij
// DEMANGLED: TimerWarRoomChangeHost::dispatch_sig(int,int,unsigned int)
int TimerWarRoomChangeHost::dispatch_sig(int war_room_index,
                                         int timer_key,
                                         unsigned int /* unused_param2 */) {
    WarRoom* const room = G_CGameManager()->GetWarRoom(war_room_index);
    if (!room) {
        return 0;
    }
    if (!room->CheckTimerKey(static_cast<TIMER_MESSAGE>(71), timer_key)) {
        return 0;
    }
    room->CheckHostChange();
    return 1;
}
