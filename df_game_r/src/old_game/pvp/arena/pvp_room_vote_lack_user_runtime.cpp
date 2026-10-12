#include "old_game/pvp/arena/pvp_room.h"

#include "infra/log/format/trace.h"
#include "infra/concurrent/timer/timer_queue.h"

// IDA_EA: 0x85DCF1A
// IDA_NAME: _ZN8PvP_Room12VoteLackUserEP5CUseri
// DEMANGLED: PvP_Room::VoteLackUser(CUser *,int)
int PvP_Room::VoteLackUser(CUser* voter, int target_seat) {
    if (static_cast<unsigned int>(target_seat) >= 8u) {
        LogManager::logFormat(
            1,
            "pvp.cpp",
            "bool PvP_Room::VoteLackUser(CUser*, int)",
            0x116d,
            "VoteSlotNo(%d) < 0 || MAX_PVP_PLAYER < VoteSlotNo",
            target_seat);
        return 0;
    }

    CUser* const target = users_[target_seat];
    if (voter == target || !target) {
        return 0;
    }

    const int voter_seat = get_user_seat(voter);
    // The old comparison is <= 8, not < 8.  This preserves its exact
    // exceptional-path behavior when the caller is not in the room.
    if (static_cast<unsigned int>(voter_seat) <= 8u) {
        election_.Vote(voter_seat, target_seat);
        const unsigned int timer_key = static_cast<unsigned int>(
            gen_timer_key(TIMER_MESSAGE_PVP_WALK_OUT_LACK_USER));
        G_TimerQueue()->InsertTimerInMilisecond(
            1,
            index_,
            TIMER_MESSAGE_PVP_WALK_OUT_LACK_USER,
            2500,
            timer_key,
            0);
        return 1;
    }

    LogManager::logFormat(
        1,
        "pvp.cpp",
        "bool PvP_Room::VoteLackUser(CUser*, int)",
        0x117b,
        "UserSlotNo(%d) < 0 || MAX_PVP_PLAYER < UserSlotNo",
        voter_seat);
    return 0;
}
