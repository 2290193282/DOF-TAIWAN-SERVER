#include "old_game/pvp/match/timer_start_matching.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "infra/concurrent/timer/timer_queue.h"

// IDA_EA: 0x863CB5E
// IDA_NAME: _ZN18TimerStartMatchingC2Ev
// DEMANGLED: TimerStartMatching::TimerStartMatching(void)
TimerStartMatching::TimerStartMatching()
    : TimerSig() {}

// IDA_EA: 0x863F4BC (D2), 0x863F4EC (D0)
// IDA_NAME: _ZN18TimerStartMatchingD2Ev / _ZN18TimerStartMatchingD0Ev
// DEMANGLED: TimerStartMatching::~TimerStartMatching()
TimerStartMatching::~TimerStartMatching() {}

// IDA_EA: 0x8637D92
// IDA_NAME: _ZN18TimerStartMatching12dispatch_sigEiij
// DEMANGLED: TimerStartMatching::dispatch_sig(int,int,unsigned int)
int TimerStartMatching::dispatch_sig(int room_index,
                                     int auth_value,
                                     unsigned int) {
    CGameManager* const manager = G_CGameManager();
    PvP_Room* const room =
        manager ? manager->GetPvp(room_index, 0, 0) : 0;
    if (!room || !room->current_match_) {
        return 0;
    }
    return room->current_match_->OnMatchRoom(room, auth_value);
}

// IDA_EA: 0x8637E14
// IDA_NAME: _ZN18TimerStartMatching15registNextTimerElii
// DEMANGLED: TimerStartMatching::registNextTimer(long,int,int)
int TimerStartMatching::registNextTimer(long delay_seconds,
                                        int room_index,
                                        int auth_value) {
    TimerQueue* const timer_queue = G_TimerQueue();
    return timer_queue->InsertTimer(2, room_index, 121,
                                    static_cast<int>(delay_seconds),
                                    static_cast<unsigned int>(auth_value), 0);
}
