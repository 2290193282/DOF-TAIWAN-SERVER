#include "old_game/pvp/arena/timer_war_room_state_callbacks_runtime.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/pvp/battlefield/war_room.h"

namespace {

WarRoom* GetTimerWarRoom(int war_room_index) {
    return G_CGameManager()->GetWarRoom(war_room_index);
}

}  // namespace

// IDA_EA: 0x863C3B6
// IDA_NAME: _ZN21TimerWarRoomWaitCountC2Ev
// DEMANGLED: TimerWarRoomWaitCount::TimerWarRoomWaitCount(void)
TimerWarRoomWaitCount::TimerWarRoomWaitCount() : TimerSig() {}

// IDA_EA: 0x864083C (D2), 0x864086C (D0)
// IDA_NAME: _ZN21TimerWarRoomWaitCountD2Ev
// IDA_NAME: _ZN21TimerWarRoomWaitCountD0Ev
// DEMANGLED: TimerWarRoomWaitCount::~TimerWarRoomWaitCount()
TimerWarRoomWaitCount::~TimerWarRoomWaitCount() {}

// IDA_EA: 0x8634CFE
// IDA_NAME: _ZN21TimerWarRoomWaitCount12dispatch_sigEiij
// DEMANGLED: TimerWarRoomWaitCount::dispatch_sig(int,int,unsigned int)
int TimerWarRoomWaitCount::dispatch_sig(int war_room_index,
                                        int timer_key,
                                        unsigned int) {
    WarRoom* const room = GetTimerWarRoom(war_room_index);
    if (!room || !room->CheckTimerKey(static_cast<TIMER_MESSAGE>(65),
                                      timer_key)) {
        return 0;
    }

    room->ChangeState(static_cast<WARROOM_STATE>(2));
    return 1;
}

// IDA_EA: 0x863C3D2
// IDA_NAME: _ZN26TimerWarRoomReadyLockCountC2Ev
// DEMANGLED: TimerWarRoomReadyLockCount::TimerWarRoomReadyLockCount(void)
TimerWarRoomReadyLockCount::TimerWarRoomReadyLockCount() : TimerSig() {}

// IDA_EA: 0x86407EE (D2), 0x864081E (D0)
// IDA_NAME: _ZN26TimerWarRoomReadyLockCountD2Ev
// IDA_NAME: _ZN26TimerWarRoomReadyLockCountD0Ev
// DEMANGLED: TimerWarRoomReadyLockCount::~TimerWarRoomReadyLockCount()
TimerWarRoomReadyLockCount::~TimerWarRoomReadyLockCount() {}

// IDA_EA: 0x8634D6A
// IDA_NAME: _ZN26TimerWarRoomReadyLockCount12dispatch_sigEiij
// DEMANGLED: TimerWarRoomReadyLockCount::dispatch_sig(int,int,unsigned int)
int TimerWarRoomReadyLockCount::dispatch_sig(int war_room_index,
                                             int timer_key,
                                             unsigned int) {
    WarRoom* const room = GetTimerWarRoom(war_room_index);
    if (!room || !room->CheckTimerKey(static_cast<TIMER_MESSAGE>(66),
                                      timer_key)) {
        return 0;
    }

    room->ChangeState(static_cast<WARROOM_STATE>(3));
    return 1;
}

// IDA_EA: 0x863C3EE
// IDA_NAME: _ZN28TimerWarRoomBattlePauseCountC2Ev
// DEMANGLED: TimerWarRoomBattlePauseCount::TimerWarRoomBattlePauseCount(void)
TimerWarRoomBattlePauseCount::TimerWarRoomBattlePauseCount() : TimerSig() {}

// IDA_EA: 0x86407A0 (D2), 0x86407D0 (D0)
// IDA_NAME: _ZN28TimerWarRoomBattlePauseCountD2Ev
// IDA_NAME: _ZN28TimerWarRoomBattlePauseCountD0Ev
// DEMANGLED: TimerWarRoomBattlePauseCount::~TimerWarRoomBattlePauseCount()
TimerWarRoomBattlePauseCount::~TimerWarRoomBattlePauseCount() {}

// IDA_EA: 0x8634DD6
// IDA_NAME: _ZN28TimerWarRoomBattlePauseCount12dispatch_sigEiij
// DEMANGLED: TimerWarRoomBattlePauseCount::dispatch_sig(int,int,unsigned int)
int TimerWarRoomBattlePauseCount::dispatch_sig(int war_room_index,
                                               int timer_key,
                                               unsigned int) {
    WarRoom* const room = GetTimerWarRoom(war_room_index);
    if (!room || !room->CheckTimerKey(static_cast<TIMER_MESSAGE>(67),
                                      timer_key)) {
        return 0;
    }

    room->ChangeState(static_cast<WARROOM_STATE>(3));
    return 1;
}

// IDA_EA: 0x863C40A
// IDA_NAME: _ZN23TimerWarRoomResultCountC2Ev
// DEMANGLED: TimerWarRoomResultCount::TimerWarRoomResultCount(void)
TimerWarRoomResultCount::TimerWarRoomResultCount() : TimerSig() {}

// IDA_EA: 0x8640752 (D2), 0x8640782 (D0)
// IDA_NAME: _ZN23TimerWarRoomResultCountD2Ev
// IDA_NAME: _ZN23TimerWarRoomResultCountD0Ev
// DEMANGLED: TimerWarRoomResultCount::~TimerWarRoomResultCount()
TimerWarRoomResultCount::~TimerWarRoomResultCount() {}

// IDA_EA: 0x8634E42
// IDA_NAME: _ZN23TimerWarRoomResultCount12dispatch_sigEiij
// DEMANGLED: TimerWarRoomResultCount::dispatch_sig(int,int,unsigned int)
int TimerWarRoomResultCount::dispatch_sig(int war_room_index,
                                          int timer_key,
                                          unsigned int) {
    WarRoom* const room = GetTimerWarRoom(war_room_index);
    if (!room || !room->CheckTimerKey(static_cast<TIMER_MESSAGE>(68),
                                      timer_key)) {
        return 0;
    }

    room->HandleTimerResultCount();
    return 1;
}

// IDA_EA: 0x863C426
// IDA_NAME: _ZN20TImerWarRoomTimeBombC2Ev
// DEMANGLED: TImerWarRoomTimeBomb::TImerWarRoomTimeBomb(void)
TImerWarRoomTimeBomb::TImerWarRoomTimeBomb() : TimerSig() {}

// IDA_EA: 0x8640704 (D2), 0x8640734 (D0)
// IDA_NAME: _ZN20TImerWarRoomTimeBombD2Ev
// IDA_NAME: _ZN20TImerWarRoomTimeBombD0Ev
// DEMANGLED: TImerWarRoomTimeBomb::~TImerWarRoomTimeBomb()
TImerWarRoomTimeBomb::~TImerWarRoomTimeBomb() {}

// IDA_EA: 0x8634EA6
// IDA_NAME: _ZN20TImerWarRoomTimeBomb12dispatch_sigEiij
// DEMANGLED: TImerWarRoomTimeBomb::dispatch_sig(int,int,unsigned int)
int TImerWarRoomTimeBomb::dispatch_sig(int war_room_index,
                                       int timer_key,
                                       unsigned int) {
    WarRoom* const room = GetTimerWarRoom(war_room_index);
    if (!room || !room->CheckTimerKey(static_cast<TIMER_MESSAGE>(69),
                                      timer_key)) {
        return 0;
    }

    room->HandleTimerTimeBomb();
    return 1;
}

// IDA_EA: 0x863C442
// IDA_NAME: _ZN21TimerWarRoomCheckFailC2Ev
// DEMANGLED: TimerWarRoomCheckFail::TimerWarRoomCheckFail(void)
TimerWarRoomCheckFail::TimerWarRoomCheckFail() : TimerSig() {}

// IDA_EA: 0x86406B6 (D2), 0x86406E6 (D0)
// IDA_NAME: _ZN21TimerWarRoomCheckFailD2Ev
// IDA_NAME: _ZN21TimerWarRoomCheckFailD0Ev
// DEMANGLED: TimerWarRoomCheckFail::~TimerWarRoomCheckFail()
TimerWarRoomCheckFail::~TimerWarRoomCheckFail() {}

// IDA_EA: 0x8634F0A
// IDA_NAME: _ZN21TimerWarRoomCheckFail12dispatch_sigEiij
// DEMANGLED: TimerWarRoomCheckFail::dispatch_sig(int,int,unsigned int)
int TimerWarRoomCheckFail::dispatch_sig(int war_room_index,
                                        int timer_key,
                                        unsigned int) {
    WarRoom* const room = GetTimerWarRoom(war_room_index);
    if (!room || !room->CheckTimerKey(static_cast<TIMER_MESSAGE>(70),
                                      timer_key)) {
        return 0;
    }

    room->WarRoomFail();
    return 1;
}
