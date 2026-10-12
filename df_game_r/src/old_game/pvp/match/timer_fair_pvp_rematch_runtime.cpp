#include "old_game/pvp/match/timer_fair_pvp_rematch_runtime.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/pvp/match/fair_pvp_match_interface.h"
#include "old_game/pvp/arena/pvp_room.h"

// IDA_EA: 0x863CF6E
// IDA_NAME: _ZN20Timer_FairPvPRematchC2Ev
// DEMANGLED: Timer_FairPvPRematch::Timer_FairPvPRematch(void)
Timer_FairPvPRematch::Timer_FairPvPRematch() : TimerSig() {}

// IDA_EA: 0x863E9B8 (D2), 0x863E9E8 (D0)
// IDA_NAME: _ZN20Timer_FairPvPRematchD2Ev
// IDA_NAME: _ZN20Timer_FairPvPRematchD0Ev
// DEMANGLED: Timer_FairPvPRematch::~Timer_FairPvPRematch()
Timer_FairPvPRematch::~Timer_FairPvPRematch() {}

// IDA_EA: 0x863A430
// IDA_NAME: _ZN20Timer_FairPvPRematch12dispatch_sigEiij
// DEMANGLED: Timer_FairPvPRematch::dispatch_sig(int,int,unsigned int)
int Timer_FairPvPRematch::dispatch_sig(int pvp_index, int timer_key,
                                       unsigned int) {
    PvP_Room* const room =
        G_CGameManager()->GetPvp(pvp_index, 0, 0);
    if (!room || room->get_timer_key(static_cast<TIMER_MESSAGE>(62)) !=
                     timer_key) {
        return 0;
    }

    if (room->current_match_) {
        room->current_match_->OnRematchTimeout(room);
    }
    return 1;
}
