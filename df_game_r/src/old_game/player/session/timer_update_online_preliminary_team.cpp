#include "old_game/player/session/timer_update_online_preliminary_team.h"

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/player/core/cuser.h"
#include "infra/concurrent/timer/timer_queue.h"


// IDA_EA: 0x863CE8E
// IDA_NAME: _ZN33Timer_UpdateOnlinePreliminaryTeamC2Ev
// DEMANGLED: Timer_UpdateOnlinePreliminaryTeam::Timer_UpdateOnlinePreliminaryTeam(void)
Timer_UpdateOnlinePreliminaryTeam::Timer_UpdateOnlinePreliminaryTeam()
    : TimerSig() {}

// IDA_EA: 0x863EBDA / 0x863EC0A
// IDA_NAME: _ZN33Timer_UpdateOnlinePreliminaryTeamD2Ev / D0Ev
// DEMANGLED: Timer_UpdateOnlinePreliminaryTeam::~Timer_UpdateOnlinePreliminaryTeam()
Timer_UpdateOnlinePreliminaryTeam::~Timer_UpdateOnlinePreliminaryTeam() {}

// IDA_EA: 0x8639722
// IDA_NAME: _ZN33Timer_UpdateOnlinePreliminaryTeam12dispatch_sigEiij
// DEMANGLED: Timer_UpdateOnlinePreliminaryTeam::dispatch_sig(int,int,unsigned int)
int Timer_UpdateOnlinePreliminaryTeam::dispatch_sig(int object_id,
                                                    int team_id,
                                                    unsigned int unused_param2) {
    (void)object_id;
    (void)unused_param2;

    online_preliminary::COnlinePreliminaryTeamMgr* const manager =
        GlobalData::s_onlinePreliminaryTeamMgr;
    if (!manager) {
        return 0;
    }

    online_preliminary::COnlinePreliminaryTeam* const team =
        manager->FindTeam(team_id);
    if (!team) {
        return 0;
    }

    team->saveData();
    return RegistNextTimer(team_id) == 1 ? 1 : 0;
}

// IDA_EA: 0x863977E
// IDA_NAME: _ZN33Timer_UpdateOnlinePreliminaryTeam15RegistNextTimerEi
// DEMANGLED: Timer_UpdateOnlinePreliminaryTeam::RegistNextTimer(int)
int Timer_UpdateOnlinePreliminaryTeam::RegistNextTimer(int team_id) {
    // The original ignores InsertTimer's return and reports registration as
    // successful after issuing the queue call.
    G_TimerQueue()->InsertTimer(
        2, 0, 143, 600, static_cast<unsigned int>(team_id), 0);
    return 1;
}
