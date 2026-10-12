#ifndef OLDSRV_OLD_GAME_WORLD_WORLD_TIMER_RUNTIME_H_
#define OLDSRV_OLD_GAME_WORLD_WORLD_TIMER_RUNTIME_H_

#include "infra/concurrent/timer/timer_sig.h"

// IDA units: 05_world_enter_village/Timer*.  These are vptr-only timer
// handlers; their state lives in the referenced party, world manager, or
// event manager rather than in the callback object.
class TimerMapLoading : public TimerSig {
public:
    // IDA_EA: 0x8633B84
    // IDA_NAME: _ZN15TimerMapLoading12dispatch_sigEiij
    virtual int dispatch_sig(int party_index,
                             int timer_key,
                             unsigned int unused_param);
};

class TimerEPLPReturnVillage : public TimerSig {
public:
    // IDA_EA: 0x8634C06
    // IDA_NAME: _ZN22TimerEPLPReturnVillage12dispatch_sigEiij
    virtual int dispatch_sig(int party_index,
                             int timer_key,
                             unsigned int unused_param);
};

class TimerVillageMonsterUpdateRate : public TimerSig {
public:
    // IDA_EA: 0x8639000
    // IDA_NAME: _ZN29TimerVillageMonsterUpdateRate12dispatch_sigEiij
    virtual int dispatch_sig(int object_id,
                             int param1,
                             unsigned int param2);
};

class TimerVillageMonsterRegen : public TimerSig {
public:
    // IDA_EA: 0x863901A
    // IDA_NAME: _ZN24TimerVillageMonsterRegen12dispatch_sigEiij
    virtual int dispatch_sig(int object_id,
                             int village,
                             unsigned int area);
};

class TimerStartVillageAttacked : public TimerSig {
public:
    // IDA_EA: 0x8639042
    // IDA_NAME: _ZN25TimerStartVillageAttacked12dispatch_sigEiij
    virtual int dispatch_sig(int object_id,
                             int param1,
                             unsigned int param2);
};

class TimerVillageAttackedPlayTimeout : public TimerSig {
public:
    // IDA_EA: 0x863905C
    // IDA_NAME: _ZN31TimerVillageAttackedPlayTimeout12dispatch_sigEiij
    virtual int dispatch_sig(int party_index,
                             int timer_key,
                             unsigned int monster_token);
};

class TimerStartExpEventAfterVillageAttack : public TimerSig {
public:
    // IDA_EA: 0x863949E
    // IDA_NAME: _ZN36TimerStartExpEventAfterVillageAttack12dispatch_sigEiij
    virtual int dispatch_sig(int object_id,
                             int param1,
                             unsigned int param2);
};

#endif  // OLDSRV_OLD_GAME_WORLD_WORLD_TIMER_RUNTIME_H_
