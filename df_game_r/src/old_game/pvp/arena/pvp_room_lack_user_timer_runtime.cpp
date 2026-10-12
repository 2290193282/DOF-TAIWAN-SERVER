#include "old_game/pvp/arena/pvp_room.h"

// IDA_EA: 0x85DCE28
// IDA_NAME: _ZN8PvP_Room22OnTimerWalkOutLackUserEv
// DEMANGLED: PvP_Room::OnTimerWalkOutLackUser(void)
int PvP_Room::OnTimerWalkOutLackUser() {
    for (;;) {
        const int elected_seat = election_.GetElected();
        if (elected_seat < 0) {
            return 1;
        }

        election_.CancelCandidate(elected_seat);
        election_.CancelVoter(elected_seat);

        // IDA 0x85DCEAA calls IMatch vtable slot +0x44.  The recovered
        // IMatch order identifies that slot as OnWalkOut(PvP_Room*, int).
        current_match_->OnWalkOut(this, elected_seat);
        if (OnSetPVPSeatState(
                users_[elected_seat], elected_seat,
                static_cast<ENUM_SEAT_STATE>(254),
                static_cast<ENUM_WALKOUT_TYPE>(3)) != 1) {
            return 0;
        }
    }
}
