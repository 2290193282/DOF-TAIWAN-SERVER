#include "old_game/pvp/battlefield/fair_pvp_team_info_runtime.h"

#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

namespace fair_pvp {

// IDA_EA: 0x8568FDA
// IDA_NAME: _ZN8fair_pvp15PlayerInfoThirdC2Ev
// DEMANGLED: fair_pvp::PlayerInfoThird::PlayerInfoThird(void)
PlayerInfoThird::PlayerInfoThird()
    : character_kind() {
}

// IDA_EA: 0x8568900
// IDA_NAME: _ZN8fair_pvp15PlayerInfoThird5resetEv
// DEMANGLED: fair_pvp::PlayerInfoThird::reset(void)
PlayerInfoThird* PlayerInfoThird::reset() {
    user = 0;
    present = false;
    pvp_grade = 0;
    rating = 0.0f;
    return this;
}

// IDA_EA: 0x8568FF0
// IDA_NAME: _ZN8fair_pvp13TeamInfoThirdC2Ev
// DEMANGLED: fair_pvp::TeamInfoThird::TeamInfoThird(void)
TeamInfoThird::TeamInfoThird() {
}

// IDA_EA: 0x85624DE
// IDA_NAME: _ZN8fair_pvp13TeamInfoThird7refreshEv
// DEMANGLED: fair_pvp::TeamInfoThird::refresh(void)
TeamInfoThird* TeamInfoThird::refresh() {
    for (int index = 0; index <= 3; ++index) {
        if (players[index].present && players[index].user) {
            players[index].rating = players[index].user->GetRating();
        } else {
            players[index].rating = 0.0f;
        }
    }
    rematch_requested = false;
    rematch_state = 2;
    return this;
}

// IDA_EA: 0x8562582
// IDA_NAME: _ZN8fair_pvp13TeamInfoThird5resetEv
// DEMANGLED: fair_pvp::TeamInfoThird::reset(void)
TeamInfoThird* TeamInfoThird::reset() {
    for (int index = 0; index <= 3; ++index) {
        players[index].reset();
    }
    rematch_requested = false;
    rematch_state = 2;
    return this;
}

// IDA_EA: 0x85625C6
// IDA_NAME: _ZN8fair_pvp13TeamInfoThird3addEP5CUser
// DEMANGLED: fair_pvp::TeamInfoThird::add(CUser *)
int TeamInfoThird::add(CUser* user) {
    for (int index = 0; index <= 3; ++index) {
        PlayerInfoThird& player = players[index];
        if (!player.present) {
            player.present = true;
            player.user = user;
            player.character_kind.charac_job =
                static_cast<uint8_t>(user->get_charac_job());
            player.character_kind.first_grow_type =
                static_cast<uint8_t>(user->getCurCharacGrowType());
            player.pvp_grade = user->get_pvp_grade();
            player.rating = user->GetRating();
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x85626AA
// IDA_NAME: _ZN8fair_pvp13TeamInfoThird3delEP5CUser
// DEMANGLED: fair_pvp::TeamInfoThird::del(CUser *)
int TeamInfoThird::del(CUser* user) {
    for (int index = 0; index <= 3; ++index) {
        PlayerInfoThird& player = players[index];
        if (player.present && player.user == user) {
            player.present = false;
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x856270C
// IDA_NAME: _ZNK8fair_pvp13TeamInfoThird4sizeEv
// DEMANGLED: fair_pvp::TeamInfoThird::size(void)const
int TeamInfoThird::size() const {
    int count = 0;
    for (int index = 0; index <= 3; ++index) {
        if (players[index].present) {
            ++count;
        }
    }
    return count;
}

// IDA_EA: 0x856274C
// IDA_NAME: _ZN8fair_pvp13TeamInfoThird10SendToTeamER11PacketGuard
// DEMANGLED: fair_pvp::TeamInfoThird::SendToTeam(PacketGuard &)
bool TeamInfoThird::SendToTeam(PacketGuard& packet) {
    bool result = false;
    for (int index = 0;; ++index) {
        result = index <= 3;
        if (index > 3) {
            break;
        }
        if (players[index].present && players[index].user) {
            players[index].user->Send(packet);
        }
    }
    return result;
}

// IDA_EA: 0x85627B2
// IDA_NAME: _ZN8fair_pvp13TeamInfoThird13GetTeamRatingEv
// DEMANGLED: fair_pvp::TeamInfoThird::GetTeamRating(void)
float TeamInfoThird::GetTeamRating() {
    float rating_sum = 0.0f;
    int rating_count = 0;
    for (int index = 0; index <= 3; ++index) {
        rating_sum += players[index].rating;
        if (players[index].rating != 0.0f) {
            ++rating_count;
        }
    }
    if (rating_count) {
        return rating_sum / static_cast<float>(rating_count);
    }
    return rating_sum;
}

}  // namespace fair_pvp
