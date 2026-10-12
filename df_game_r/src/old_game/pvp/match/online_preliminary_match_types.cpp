#include "old_game/pvp/match/online_preliminary_match_types.h"

#include "old_game/player/core/cuser.h"
#include "old_game/player/session/online_preliminary_member_runtime.h"
#include "old_game/player/session/online_preliminary_runtime.h"

// IDA_EA: 0x8568850
// IDA_NAME: _ZN16PlayerInfoSecond5resetEv
// DEMANGLED: PlayerInfoSecond::reset(void)
PlayerInfoSecond* PlayerInfoSecond::reset() {
    user = 0;
    occupied = 0;
    return this;
}

// IDA_EA: 0x855FCF6
// IDA_NAME: _ZN14TeamInfoSecond7refreshEv
// DEMANGLED: TeamInfoSecond::refresh(void)
void TeamInfoSecond::refresh() {
    CUser* first_user = 0;
    for (int index = 0; index <= 3; ++index) {
        if (players[index].occupied && players[index].user) {
            first_user = players[index].user;
            break;
        }
    }

    if (!first_user) {
        return;
    }

    online_preliminary_team_id =
        first_user->GetCurCharacOnlinePreliminaryTeamId();
    if (static_cast<uint8_t>(
            online_preliminary::COnlinePreliminary::GetPlayCountPerOneTeam()) ==
        1u) {
        online_preliminary::COnlinePreliminaryMember* const member =
            static_cast<online_preliminary::COnlinePreliminaryMember*>(
                first_user->GetCharacExpandData(
                    static_cast<ENUM_CHARAC_EXPAND_TYPE>(4)));
        rating = member->GetRating();
        return;
    }

    online_preliminary::COnlinePreliminaryTeam* const team =
        first_user->GetCurCharacOnlinePreliminaryTeam();
    if (team) {
        rating = team->GetRating();
    }
}

// IDA_EA: 0x855FDC0
// IDA_NAME: _ZN14TeamInfoSecond5resetEv
// DEMANGLED: TeamInfoSecond::reset(void)
bool TeamInfoSecond::reset() {
    online_preliminary_team_id = 0;
    for (int index = 0; index <= 3; ++index) {
        players[index].reset();
    }
    return false;
}

// IDA_EA: 0x855FDFE
// IDA_NAME: _ZN14TeamInfoSecond3addEP5CUser
// DEMANGLED: TeamInfoSecond::add(CUser *)
int TeamInfoSecond::add(CUser* user) {
    for (int index = 0; index <= 3; ++index) {
        if (players[index].occupied != 1u) {
            players[index].occupied = 1;
            players[index].user = user;
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x855FE54
// IDA_NAME: _ZN14TeamInfoSecond3delEP5CUser
// DEMANGLED: TeamInfoSecond::del(CUser *)
int TeamInfoSecond::del(CUser* user) {
    for (int index = 0; index <= 3; ++index) {
        if (players[index].occupied && players[index].user == user) {
            players[index].occupied = 0;
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x855FEAA
// IDA_NAME: _ZNK14TeamInfoSecond4sizeEv
// DEMANGLED: TeamInfoSecond::size(void)const
int TeamInfoSecond::size() const {
    int count = 0;
    for (int index = 0; index <= 3; ++index) {
        if (players[index].occupied) {
            ++count;
        }
    }
    return count;
}
