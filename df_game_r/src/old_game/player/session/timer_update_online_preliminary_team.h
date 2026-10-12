#ifndef OLDSRV_OLD_GAME_USERINFO_TIMER_UPDATE_ONLINE_PRELIMINARY_TEAM_H_
#define OLDSRV_OLD_GAME_USERINFO_TIMER_UPDATE_ONLINE_PRELIMINARY_TEAM_H_

#include "infra/concurrent/timer/timer_sig.h"

// IDA unit: 99_misc_global/Timer_UpdateOnlinePreliminaryTeam.  A team id is
// kept in TimerEntry::param1; object_id remains the original zero value.
class Timer_UpdateOnlinePreliminaryTeam : public TimerSig {
public:
    // IDA_EA: 0x863CE8E
    // IDA_NAME: _ZN33Timer_UpdateOnlinePreliminaryTeamC2Ev
    Timer_UpdateOnlinePreliminaryTeam();

    // IDA_EA: 0x863EBDA / 0x863EC0A
    // IDA_NAME: _ZN33Timer_UpdateOnlinePreliminaryTeamD2Ev / D0Ev
    virtual ~Timer_UpdateOnlinePreliminaryTeam();

    // IDA_EA: 0x8639722
    // IDA_NAME: _ZN33Timer_UpdateOnlinePreliminaryTeam12dispatch_sigEiij
    virtual int dispatch_sig(int object_id,
                             int team_id,
                             unsigned int unused_param2);

    // IDA_EA: 0x863977E
    // IDA_NAME: _ZN33Timer_UpdateOnlinePreliminaryTeam15RegistNextTimerEi
    static int RegistNextTimer(int team_id);
};

#if defined(__i386__)
static_assert(sizeof(Timer_UpdateOnlinePreliminaryTeam) == 0x04u,
              "Timer_UpdateOnlinePreliminaryTeam must remain vptr-only");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_TIMER_UPDATE_ONLINE_PRELIMINARY_TEAM_H_
