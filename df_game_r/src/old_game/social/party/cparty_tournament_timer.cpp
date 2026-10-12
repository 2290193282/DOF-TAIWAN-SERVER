#include "old_game/social/party/party.h"

#include "infra/concurrent/timer/timer_queue.h"

// IDA_EA: 0x85BD6DE
// IDA_NAME: _ZN6CParty28InsertTournamentDungeonTimerE13TIMER_MESSAGE
// DEMANGLED: CParty::InsertTournamentDungeonTimer(TIMER_MESSAGE)
int CParty::InsertTournamentDungeonTimer(TIMER_MESSAGE message) {
    if (message == TIMER_MESSAGE_TOURNAMENT_DUNGEON_CLEAR_STATE_NOTI_REWARD) {
        const int timer_key = gen_timer_key(message);
        return G_TimerQueue()->InsertTimer(1, party_index_, message, 40,
                                           timer_key, 0);
    }
    if (message == TIMER_MESSAGE_TOURNAMENT_DUNGEON_CLEAR_STATE_SELECT) {
        const int timer_key = gen_timer_key(message);
        return G_TimerQueue()->InsertTimer(1, party_index_, message, 4,
                                           timer_key, 0);
    }
    return static_cast<int>(message);
}
