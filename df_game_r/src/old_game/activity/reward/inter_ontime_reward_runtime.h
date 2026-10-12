#ifndef OLDSRV_OLD_GAME_EVENT_INTER_ONTIME_REWARD_RUNTIME_H_
#define OLDSRV_OLD_GAME_EVENT_INTER_ONTIME_REWARD_RUNTIME_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"
#include "infra/concurrent/timer/timer_sig.h"

class CUser;

class Inter_OnTimeEventRewardStart : public ISigDispatcher {
public:
    Inter_OnTimeEventRewardStart();
    virtual ~Inter_OnTimeEventRewardStart();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_OnTimeEventRewardStartOnLogin : public ISigDispatcher {
public:
    virtual ~Inter_OnTimeEventRewardStartOnLogin();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_OnTimeEventRewardEnd : public ISigDispatcher {
public:
    Inter_OnTimeEventRewardEnd();
    virtual ~Inter_OnTimeEventRewardEnd();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// The slot-155 TimerDispatcher provider drains at most one on-time reward
// batch and schedules the next ten-second tick while users remain queued.
class Timer_Send_Ontime_Reward : public TimerSig {
public:
    Timer_Send_Ontime_Reward();
    virtual ~Timer_Send_Ontime_Reward();
    virtual int dispatch_sig(int object_id, int param1, unsigned int param2);

    // The recovered body has no instance-state access.  The old mangled name
    // is identical for this static entry and the call sites only require the
    // timer insertion behavior.
    static int RegistNextTimer();
};

#if defined(__i386__)
static_assert(sizeof(Inter_OnTimeEventRewardStart) == 0x04u,
              "Inter_OnTimeEventRewardStart must stay vptr-only on i386");
static_assert(sizeof(Inter_OnTimeEventRewardStartOnLogin) == 0x04u,
              "Inter_OnTimeEventRewardStartOnLogin must stay vptr-only on i386");
static_assert(sizeof(Inter_OnTimeEventRewardEnd) == 0x04u,
              "Inter_OnTimeEventRewardEnd must stay vptr-only on i386");
static_assert(sizeof(Timer_Send_Ontime_Reward) == 0x04u,
              "Timer_Send_Ontime_Reward must stay vptr-only on i386");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_INTER_ONTIME_REWARD_RUNTIME_H_
