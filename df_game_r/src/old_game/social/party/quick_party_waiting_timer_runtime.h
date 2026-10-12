#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_QUICK_PARTY_WAITING_TIMER_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_QUICK_PARTY_WAITING_TIMER_RUNTIME_H_

#include "infra/concurrent/timer/timer_sig.h"

namespace QuickParty {
class CQuickParty;
}

// IDA vtable 0x8CE9270.  The timer carries no state beyond TimerSig; the
// queue object id identifies the CParty whose election timer fired.
class TimerPartyWalkOutLackUser : public TimerSig {
public:
    // IDA_EA: 0x863C24A
    // IDA_NAME: _ZN25TimerPartyWalkOutLackUserC2Ev
    TimerPartyWalkOutLackUser();

    // IDA_EA: 0x8640D6A / 0x8640D9A
    // IDA_NAME: _ZN25TimerPartyWalkOutLackUserD2Ev
    //           _ZN25TimerPartyWalkOutLackUserD0Ev
    virtual ~TimerPartyWalkOutLackUser();

    // IDA_EA: 0x8633E14
    // IDA_NAME: _ZN25TimerPartyWalkOutLackUser12dispatch_sigEiij
    virtual int dispatch_sig(int party_index, int timer_key,
                             unsigned int timer_message);
};

// IDA vtable 0x8CE86B8.  It owns the timeout transition from a queued
// matchmaking record to either cancellation or start_game.
class Timer_QuickPartyWaitingPartyMatching : public TimerSig {
public:
    // IDA_EA: 0x863D04E
    // IDA_NAME: _ZN36Timer_QuickPartyWaitingPartyMatchingC2Ev
    Timer_QuickPartyWaitingPartyMatching();

    // IDA_EA: 0x863E6FA / 0x863E72A
    // IDA_NAME: _ZN36Timer_QuickPartyWaitingPartyMatchingD2Ev
    //           _ZN36Timer_QuickPartyWaitingPartyMatchingD0Ev
    virtual ~Timer_QuickPartyWaitingPartyMatching();

    // IDA_EA: 0x863AC18
    // IDA_NAME: _ZN36Timer_QuickPartyWaitingPartyMatching12dispatch_sigEiij
    virtual int dispatch_sig(int quick_party_index, int timer_key,
                             unsigned int timer_message);

    // IDA_EA: 0x863AD96
    // IDA_NAME: _ZN36Timer_QuickPartyWaitingPartyMatching18cancel_quick_partyEPN10QuickParty11CQuickPartyE
    int cancel_quick_party(QuickParty::CQuickParty* quick_party);
};

// IDA vtable 0x8CE86A0.  This fires after the matching party's 90 second
// start window and atomically sends its members back to the world.
class Timer_QuickPartyWaitingStartGame : public TimerSig {
public:
    // IDA_EA: 0x863D06A
    // IDA_NAME: _ZN32Timer_QuickPartyWaitingStartGameC2Ev
    Timer_QuickPartyWaitingStartGame();

    // IDA_EA: 0x863E6AC / 0x863E6DC
    // IDA_NAME: _ZN32Timer_QuickPartyWaitingStartGameD2Ev
    //           _ZN32Timer_QuickPartyWaitingStartGameD0Ev
    virtual ~Timer_QuickPartyWaitingStartGame();

    // IDA_EA: 0x863AE82
    // IDA_NAME: _ZN32Timer_QuickPartyWaitingStartGame12dispatch_sigEiij
    virtual int dispatch_sig(int party_index, int timer_key,
                             unsigned int timer_message);
};

#if defined(__i386__)
static_assert(sizeof(TimerPartyWalkOutLackUser) == 4u,
              "TimerPartyWalkOutLackUser must remain a TimerSig-only object");
static_assert(sizeof(Timer_QuickPartyWaitingPartyMatching) == 4u,
              "Timer_QuickPartyWaitingPartyMatching must remain TimerSig-only");
static_assert(sizeof(Timer_QuickPartyWaitingStartGame) == 4u,
              "Timer_QuickPartyWaitingStartGame must remain TimerSig-only");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_QUICK_PARTY_WAITING_TIMER_RUNTIME_H_
