#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_WAR_ROOM_CHANGE_HOST_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_WAR_ROOM_CHANGE_HOST_H_

#include "infra/concurrent/timer/timer_sig.h"

// IDA unit: 09_dungeon_pvp_party/TimerWarRoomChangeHost.  TimerDispatcher
// owns the one stateless instance at message-table slot 71 (+0x11c).
class TimerWarRoomChangeHost : public TimerSig {
public:
    // IDA_EA: 0x863C45E
    // IDA_NAME: _ZN22TimerWarRoomChangeHostC2Ev
    // DEMANGLED: TimerWarRoomChangeHost::TimerWarRoomChangeHost(void)
    TimerWarRoomChangeHost();

    // IDA_EA: 0x8640668
    // IDA_NAME: _ZN22TimerWarRoomChangeHostD2Ev
    // DEMANGLED: TimerWarRoomChangeHost::~TimerWarRoomChangeHost()
    // IDA_EA: 0x8640698
    // IDA_NAME: _ZN22TimerWarRoomChangeHostD0Ev
    // DEMANGLED: TimerWarRoomChangeHost::~TimerWarRoomChangeHost()
    virtual ~TimerWarRoomChangeHost();

    // IDA_EA: 0x8634F6E
    // IDA_NAME: _ZN22TimerWarRoomChangeHost12dispatch_sigEiij
    // DEMANGLED: TimerWarRoomChangeHost::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int war_room_index,
                             int timer_key,
                             unsigned int unused_param2);
};

#if defined(__i386__)
static_assert(sizeof(TimerWarRoomChangeHost) == 0x04u,
              "TimerWarRoomChangeHost must retain the TimerSig-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_WAR_ROOM_CHANGE_HOST_H_
