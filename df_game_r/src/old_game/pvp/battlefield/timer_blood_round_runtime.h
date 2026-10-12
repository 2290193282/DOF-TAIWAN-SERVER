#ifndef OLDSRV_OLD_GAME_PVP_BATTLEFIELD_TIMER_BLOOD_ROUND_RUNTIME_H_
#define OLDSRV_OLD_GAME_PVP_BATTLEFIELD_TIMER_BLOOD_ROUND_RUNTIME_H_

#include "infra/concurrent/timer/timer_sig.h"

// The 0627 TimerDispatcher owns one vptr-only callback for each blood-round
// timer message.  Keep the concrete classes separate: CParty maintains an
// independent timer key for every message in the 22..33 interval.
class TimerBloodSmallRoundRegister : public TimerSig {
public:
    TimerBloodSmallRoundRegister();
    virtual ~TimerBloodSmallRoundRegister();
    virtual int dispatch_sig(int party_index, int timer_key,
                             unsigned int param2);
};

class TimerBloodPhaseRegister0 : public TimerSig {
public:
    TimerBloodPhaseRegister0();
    virtual ~TimerBloodPhaseRegister0();
    virtual int dispatch_sig(int party_index, int timer_key,
                             unsigned int param2);
};

class TimerBloodPhaseRegister1 : public TimerSig {
public:
    TimerBloodPhaseRegister1();
    virtual ~TimerBloodPhaseRegister1();
    virtual int dispatch_sig(int party_index, int timer_key,
                             unsigned int param2);
};

class TimerBloodPhaseRegister2 : public TimerSig {
public:
    TimerBloodPhaseRegister2();
    virtual ~TimerBloodPhaseRegister2();
    virtual int dispatch_sig(int party_index, int timer_key,
                             unsigned int param2);
};

class TimerBloodPhaseRegister3 : public TimerSig {
public:
    TimerBloodPhaseRegister3();
    virtual ~TimerBloodPhaseRegister3();
    virtual int dispatch_sig(int party_index, int timer_key,
                             unsigned int param2);
};

class TimerBloodPhaseRegister4 : public TimerSig {
public:
    TimerBloodPhaseRegister4();
    virtual ~TimerBloodPhaseRegister4();
    virtual int dispatch_sig(int party_index, int timer_key,
                             unsigned int param2);
};

class TimerBloodPhaseRegister5 : public TimerSig {
public:
    TimerBloodPhaseRegister5();
    virtual ~TimerBloodPhaseRegister5();
    virtual int dispatch_sig(int party_index, int timer_key,
                             unsigned int param2);
};

class TimerBloodPhaseRegister6 : public TimerSig {
public:
    TimerBloodPhaseRegister6();
    virtual ~TimerBloodPhaseRegister6();
    virtual int dispatch_sig(int party_index, int timer_key,
                             unsigned int param2);
};

class TimerBloodPhaseRegister7 : public TimerSig {
public:
    TimerBloodPhaseRegister7();
    virtual ~TimerBloodPhaseRegister7();
    virtual int dispatch_sig(int party_index, int timer_key,
                             unsigned int param2);
};

class TimerBloodPhaseRegister8 : public TimerSig {
public:
    TimerBloodPhaseRegister8();
    virtual ~TimerBloodPhaseRegister8();
    virtual int dispatch_sig(int party_index, int timer_key,
                             unsigned int param2);
};

class TimerBloodPhaseRegister9 : public TimerSig {
public:
    TimerBloodPhaseRegister9();
    virtual ~TimerBloodPhaseRegister9();
    virtual int dispatch_sig(int party_index, int timer_key,
                             unsigned int param2);
};

class TimerBloodCrazyMonster : public TimerSig {
public:
    TimerBloodCrazyMonster();
    virtual ~TimerBloodCrazyMonster();
    virtual int dispatch_sig(int party_index, int timer_key,
                             unsigned int param2);
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(TimerBloodSmallRoundRegister) == 0x04u &&
                  sizeof(TimerBloodPhaseRegister0) == 0x04u &&
                  sizeof(TimerBloodPhaseRegister1) == 0x04u &&
                  sizeof(TimerBloodPhaseRegister2) == 0x04u &&
                  sizeof(TimerBloodPhaseRegister3) == 0x04u &&
                  sizeof(TimerBloodPhaseRegister4) == 0x04u &&
                  sizeof(TimerBloodPhaseRegister5) == 0x04u &&
                  sizeof(TimerBloodPhaseRegister6) == 0x04u &&
                  sizeof(TimerBloodPhaseRegister7) == 0x04u &&
                  sizeof(TimerBloodPhaseRegister8) == 0x04u &&
                  sizeof(TimerBloodPhaseRegister9) == 0x04u &&
                  sizeof(TimerBloodCrazyMonster) == 0x04u,
              "0627 blood-round timer callbacks must stay vptr-only");
#endif

#endif  // OLDSRV_OLD_GAME_PVP_BATTLEFIELD_TIMER_BLOOD_ROUND_RUNTIME_H_
