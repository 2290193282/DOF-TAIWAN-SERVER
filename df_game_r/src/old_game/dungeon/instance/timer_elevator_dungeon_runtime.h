#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_ELEVATOR_DUNGEON_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_ELEVATOR_DUNGEON_RUNTIME_H_

#include "infra/concurrent/timer/timer_sig.h"

// IDA unit: 09_dungeon_pvp_party/TimerElevatorDungeon.  The callback is a
// TimerSig-only object; the queued state belongs to CParty/CBattle_Field.
class TimerElevatorDungeon : public TimerSig {
public:
    // IDA_EA: 0x863CC22
    // IDA_NAME: _ZN20TimerElevatorDungeonC2Ev
    // DEMANGLED: TimerElevatorDungeon::TimerElevatorDungeon(void)
    TimerElevatorDungeon();

    // IDA_EA: 0x863F2E8 (D2), 0x863F318 (D0)
    // IDA_NAME: _ZN20TimerElevatorDungeonD2Ev
    // IDA_NAME: _ZN20TimerElevatorDungeonD0Ev
    // DEMANGLED: TimerElevatorDungeon::~TimerElevatorDungeon()
    virtual ~TimerElevatorDungeon();

    // IDA_EA: 0x86381F2
    // IDA_NAME: _ZN20TimerElevatorDungeon12dispatch_sigEiij
    // DEMANGLED: TimerElevatorDungeon::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int party_index,
                             int timer_key,
                             unsigned int unused_payload);
};

#if defined(__i386__)
static_assert(sizeof(TimerElevatorDungeon) == 4u,
              "TimerElevatorDungeon must remain a TimerSig-only object");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_ELEVATOR_DUNGEON_RUNTIME_H_
