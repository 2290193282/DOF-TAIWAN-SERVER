#ifndef OLDSRV_OLD_GAME_GROUP_TIMER_DUNGEON_CLEAR_RUNTIME_H_
#define OLDSRV_OLD_GAME_GROUP_TIMER_DUNGEON_CLEAR_RUNTIME_H_

#include "infra/concurrent/timer/timer_sig.h"

class TimerDungeonClear : public TimerSig {
public:
    TimerDungeonClear() {}
    virtual ~TimerDungeonClear() {}
    virtual int dispatch_sig(int party_index, int timer_key, unsigned int param2);
};

// IDA unit: 99_misc_global/TimerBossDieTimeout.  TimerDispatcher slot 100
// forwards the queued party index, boss-death timer key and character number
// to the real CParty timeout body.
class TimerBossDieTimeout : public TimerSig {
public:
    // IDA_EA: 0x863C7FA
    // IDA_NAME: _ZN19TimerBossDieTimeoutC2Ev
    TimerBossDieTimeout();

    // IDA_EA: 0x863FE2E (D2), 0x863FE5E (D0)
    // IDA_NAME: _ZN19TimerBossDieTimeoutD2Ev / D0Ev
    virtual ~TimerBossDieTimeout();

    // IDA_EA: 0x863647E
    // IDA_NAME: _ZN19TimerBossDieTimeout12dispatch_sigEiij
    virtual int dispatch_sig(int party_index,
                             int timer_key,
                             unsigned int charac_no);
};

// IDA unit: 09_dungeon_pvp_party/TimerDungeonFail.
// TimerDispatcher slot 14 validates the party's message-14 key before it
// executes the complete all-members-dead failure path.
class TimerDungeonFail : public TimerSig {
public:
    // IDA_EA: 0x863C22E
    // IDA_NAME: _ZN16TimerDungeonFailC2Ev
    TimerDungeonFail();

    // IDA_EA: 0x8640EF0 (D2), 0x8640F20 (D0)
    // IDA_NAME: _ZN16TimerDungeonFailD2Ev / D0Ev
    virtual ~TimerDungeonFail();

    // IDA_EA: 0x863316C
    // IDA_NAME: _ZN16TimerDungeonFail12dispatch_sigEiij
    virtual int dispatch_sig(int party_index, int timer_key, unsigned int param2);
};

// IDA unit: 99_misc_global/TimerCardSelect.
// TimerDispatcher slot 16 is the normal-card selection timeout.  It owns the
// original four-second completion path queued by CMD073.
class TimerCardSelect : public TimerSig {
public:
    // IDA_EA: 0x863C29E
    // IDA_NAME: _ZN15TimerCardSelectC2Ev
    TimerCardSelect();

    // IDA_EA: 0x8640B48 (D2), 0x8640B78 (D0)
    // IDA_NAME: _ZN15TimerCardSelectD2Ev / D0Ev
    virtual ~TimerCardSelect();

    // IDA_EA: 0x86343BA
    // IDA_NAME: _ZN15TimerCardSelect12dispatch_sigEiij
    virtual int dispatch_sig(int party_index,
                             int timer_key,
                             unsigned int param2);
};

// IDA unit: 09_dungeon_pvp_party/TimerBloodDungeonClearReward.
// The original vptr-only callback is instantiated independently for each of
// the blood-dungeon reward messages 34 through 37. Its dispatch ABI carries
// the timer key in param1 and the TIMER_MESSAGE in param2.
class TimerBloodDungeonClearReward : public TimerSig {
public:
    // IDA_EA: 0x863C5CA
    // IDA_NAME: _ZN28TimerBloodDungeonClearRewardC2Ev
    // DEMANGLED: TimerBloodDungeonClearReward::TimerBloodDungeonClearReward(void)
    TimerBloodDungeonClearReward();

    // IDA_EA: 0x863F06C (D2), 0x863F09C (D0)
    // IDA_NAME: _ZN28TimerBloodDungeonClearRewardD2Ev /
    //           _ZN28TimerBloodDungeonClearRewardD0Ev
    // DEMANGLED: TimerBloodDungeonClearReward::~TimerBloodDungeonClearReward()
    virtual ~TimerBloodDungeonClearReward();

    // IDA_EA: 0x8638CB4
    // IDA_NAME: _ZN28TimerBloodDungeonClearReward12dispatch_sigEiij
    // DEMANGLED: TimerBloodDungeonClearReward::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int party_index,
                             int timer_key,
                             unsigned int timer_message);
};

class TimerClearDungeonReward : public TimerSig {
public:
    TimerClearDungeonReward() {}
    virtual ~TimerClearDungeonReward() {}
    virtual int dispatch_sig(int party_index, int timer_key, unsigned int param2);
};

class TimerClearDungeonScoreScoll : public TimerSig {
public:
    TimerClearDungeonScoreScoll() {}
    virtual ~TimerClearDungeonScoreScoll() {}
    virtual int dispatch_sig(int party_index, int timer_key, unsigned int param2);
};

// IDA unit: 09_dungeon_pvp_party/TimerClearDungeonCardSelectRight.
// Message 19 advances only the state-4 reward-card phase into the final
// dungeon-clear message; the original class has no payload beyond TimerSig.
class TimerClearDungeonCardSelectRight : public TimerSig {
public:
    // IDA_EA: 0x863C2F2
    // IDA_NAME: _ZN32TimerClearDungeonCardSelectRightC2Ev
    TimerClearDungeonCardSelectRight();

    // IDA_EA: 0x8640A5E (D2), 0x8640A8E (D0)
    // IDA_NAME: _ZN32TimerClearDungeonCardSelectRightD2Ev / D0Ev
    virtual ~TimerClearDungeonCardSelectRight();

    // IDA_EA: 0x86347C6
    // IDA_NAME: _ZN32TimerClearDungeonCardSelectRight12dispatch_sigEiij
    virtual int dispatch_sig(int party_index, int timer_key, unsigned int param2);
};

class TimerSetPlayResult : public TimerSig {
public:
    TimerSetPlayResult() {}
    virtual ~TimerSetPlayResult() {}
    virtual int dispatch_sig(int party_index, int timer_key, unsigned int param2);
};

#endif  // OLDSRV_OLD_GAME_GROUP_TIMER_DUNGEON_CLEAR_RUNTIME_H_
