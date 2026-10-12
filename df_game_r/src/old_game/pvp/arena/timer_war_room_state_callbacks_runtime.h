#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_WAR_ROOM_STATE_CALLBACKS_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_WAR_ROOM_STATE_CALLBACKS_RUNTIME_H_

#include "infra/concurrent/timer/timer_sig.h"

// IDA units: TimerWarRoomWaitCount, TimerWarRoomReadyLockCount, and
// TimerWarRoomBattlePauseCount.  Each is a TimerSig-only object; the queued
// room index and the WarRoom timer key carry all callback state.
class TimerWarRoomWaitCount : public TimerSig {
public:
    // IDA_EA: 0x863C3B6
    // IDA_NAME: _ZN21TimerWarRoomWaitCountC2Ev
    // DEMANGLED: TimerWarRoomWaitCount::TimerWarRoomWaitCount(void)
    TimerWarRoomWaitCount();

    // IDA_EA: 0x864083C (D2), 0x864086C (D0)
    // IDA_NAME: _ZN21TimerWarRoomWaitCountD2Ev
    // IDA_NAME: _ZN21TimerWarRoomWaitCountD0Ev
    // DEMANGLED: TimerWarRoomWaitCount::~TimerWarRoomWaitCount()
    virtual ~TimerWarRoomWaitCount();

    // IDA_EA: 0x8634CFE
    // IDA_NAME: _ZN21TimerWarRoomWaitCount12dispatch_sigEiij
    // DEMANGLED: TimerWarRoomWaitCount::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int war_room_index, int timer_key,
                             unsigned int payload);
};

class TimerWarRoomReadyLockCount : public TimerSig {
public:
    // IDA_EA: 0x863C3D2
    // IDA_NAME: _ZN26TimerWarRoomReadyLockCountC2Ev
    // DEMANGLED: TimerWarRoomReadyLockCount::TimerWarRoomReadyLockCount(void)
    TimerWarRoomReadyLockCount();

    // IDA_EA: 0x86407EE (D2), 0x864081E (D0)
    // IDA_NAME: _ZN26TimerWarRoomReadyLockCountD2Ev
    // IDA_NAME: _ZN26TimerWarRoomReadyLockCountD0Ev
    // DEMANGLED: TimerWarRoomReadyLockCount::~TimerWarRoomReadyLockCount()
    virtual ~TimerWarRoomReadyLockCount();

    // IDA_EA: 0x8634D6A
    // IDA_NAME: _ZN26TimerWarRoomReadyLockCount12dispatch_sigEiij
    // DEMANGLED: TimerWarRoomReadyLockCount::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int war_room_index, int timer_key,
                             unsigned int payload);
};

class TimerWarRoomBattlePauseCount : public TimerSig {
public:
    // IDA_EA: 0x863C3EE
    // IDA_NAME: _ZN28TimerWarRoomBattlePauseCountC2Ev
    // DEMANGLED: TimerWarRoomBattlePauseCount::TimerWarRoomBattlePauseCount(void)
    TimerWarRoomBattlePauseCount();

    // IDA_EA: 0x86407A0 (D2), 0x86407D0 (D0)
    // IDA_NAME: _ZN28TimerWarRoomBattlePauseCountD2Ev
    // IDA_NAME: _ZN28TimerWarRoomBattlePauseCountD0Ev
    // DEMANGLED: TimerWarRoomBattlePauseCount::~TimerWarRoomBattlePauseCount()
    virtual ~TimerWarRoomBattlePauseCount();

    // IDA_EA: 0x8634DD6
    // IDA_NAME: _ZN28TimerWarRoomBattlePauseCount12dispatch_sigEiij
    // DEMANGLED: TimerWarRoomBattlePauseCount::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int war_room_index, int timer_key,
                             unsigned int payload);
};

class TimerWarRoomResultCount : public TimerSig {
public:
    // IDA_EA: 0x863C40A
    // IDA_NAME: _ZN23TimerWarRoomResultCountC2Ev
    // DEMANGLED: TimerWarRoomResultCount::TimerWarRoomResultCount(void)
    TimerWarRoomResultCount();

    // IDA_EA: 0x8640752 (D2), 0x8640782 (D0)
    // IDA_NAME: _ZN23TimerWarRoomResultCountD2Ev
    // IDA_NAME: _ZN23TimerWarRoomResultCountD0Ev
    // DEMANGLED: TimerWarRoomResultCount::~TimerWarRoomResultCount()
    virtual ~TimerWarRoomResultCount();

    // IDA_EA: 0x8634E42
    // IDA_NAME: _ZN23TimerWarRoomResultCount12dispatch_sigEiij
    // DEMANGLED: TimerWarRoomResultCount::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int war_room_index, int timer_key,
                             unsigned int payload);
};

class TImerWarRoomTimeBomb : public TimerSig {
public:
    // IDA_EA: 0x863C426
    // IDA_NAME: _ZN20TImerWarRoomTimeBombC2Ev
    // DEMANGLED: TImerWarRoomTimeBomb::TImerWarRoomTimeBomb(void)
    TImerWarRoomTimeBomb();

    // IDA_EA: 0x8640704 (D2), 0x8640734 (D0)
    // IDA_NAME: _ZN20TImerWarRoomTimeBombD2Ev
    // IDA_NAME: _ZN20TImerWarRoomTimeBombD0Ev
    // DEMANGLED: TImerWarRoomTimeBomb::~TImerWarRoomTimeBomb()
    virtual ~TImerWarRoomTimeBomb();

    // IDA_EA: 0x8634EA6
    // IDA_NAME: _ZN20TImerWarRoomTimeBomb12dispatch_sigEiij
    // DEMANGLED: TImerWarRoomTimeBomb::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int war_room_index, int timer_key,
                             unsigned int payload);
};

class TimerWarRoomCheckFail : public TimerSig {
public:
    // IDA_EA: 0x863C442
    // IDA_NAME: _ZN21TimerWarRoomCheckFailC2Ev
    // DEMANGLED: TimerWarRoomCheckFail::TimerWarRoomCheckFail(void)
    TimerWarRoomCheckFail();

    // IDA_EA: 0x86406B6 (D2), 0x86406E6 (D0)
    // IDA_NAME: _ZN21TimerWarRoomCheckFailD2Ev
    // IDA_NAME: _ZN21TimerWarRoomCheckFailD0Ev
    // DEMANGLED: TimerWarRoomCheckFail::~TimerWarRoomCheckFail()
    virtual ~TimerWarRoomCheckFail();

    // IDA_EA: 0x8634F0A
    // IDA_NAME: _ZN21TimerWarRoomCheckFail12dispatch_sigEiij
    // DEMANGLED: TimerWarRoomCheckFail::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int war_room_index, int timer_key,
                             unsigned int payload);
};

#if defined(__i386__)
static_assert(sizeof(TimerWarRoomWaitCount) == 4u,
              "TimerWarRoomWaitCount must remain a TimerSig-only object");
static_assert(sizeof(TimerWarRoomReadyLockCount) == 4u,
              "TimerWarRoomReadyLockCount must remain a TimerSig-only object");
static_assert(sizeof(TimerWarRoomBattlePauseCount) == 4u,
              "TimerWarRoomBattlePauseCount must remain a TimerSig-only object");
static_assert(sizeof(TimerWarRoomResultCount) == 4u,
              "TimerWarRoomResultCount must remain a TimerSig-only object");
static_assert(sizeof(TImerWarRoomTimeBomb) == 4u,
              "TImerWarRoomTimeBomb must remain a TimerSig-only object");
static_assert(sizeof(TimerWarRoomCheckFail) == 4u,
              "TimerWarRoomCheckFail must remain a TimerSig-only object");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_WAR_ROOM_STATE_CALLBACKS_RUNTIME_H_
