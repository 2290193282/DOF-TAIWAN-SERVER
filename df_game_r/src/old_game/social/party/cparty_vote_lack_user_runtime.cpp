#include "old_game/social/party/party.h"

#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/concurrent/timer/timer_queue.h"

// IDA_EA: 0x85B3C90
// IDA_NAME: _ZN6CParty12VoteLackUserEP5CUseri
// DEMANGLED: CParty::VoteLackUser(CUser *,int)
int CParty::VoteLackUser(CUser* voter, int target_seat) {
    if (static_cast<unsigned int>(target_seat) >= 4u) {
        cMyTrace trace("bool CParty::VoteLackUser(CUser*, int)", 13713, 5);
        trace("[%s][%d]", "bool CParty::VoteLackUser(CUser*, int)", 13713);
        return 0;
    }

    CUser* const target = get_user(target_seat);
    if (voter == target || !target) {
        return 0;
    }

    const unsigned int voter_seat =
        static_cast<unsigned int>(GetMemberSlotNo(voter));
    if (voter_seat >= 4u) {
        cMyTrace trace("bool CParty::VoteLackUser(CUser*, int)", 13729, 5);
        trace("[%s][%d]", "bool CParty::VoteLackUser(CUser*, int)", 13729);
        return 0;
    }

    election_.Vote(static_cast<int>(voter_seat), target_seat);
    const unsigned int timer_key = static_cast<unsigned int>(
        gen_timer_key(TIMER_MESSAGE_PARTY_WALK_OUT_LACK_USER));
    G_TimerQueue()->InsertTimerInMilisecond(
        1,
        GetPartyIndex(),
        TIMER_MESSAGE_PARTY_WALK_OUT_LACK_USER,
        2500,
        timer_key,
        0);
    return 1;
}
