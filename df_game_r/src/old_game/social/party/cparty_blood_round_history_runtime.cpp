#include "old_game/social/party/party.h"

#include <cstring>

#include "old_game/player/core/cuser.h"

// IDA_EA: 0x85B7DB6
// IDA_NAME: _ZN6CParty22historyBloodRoundClearEijjPKc
// DEMANGLED: CParty::historyBloodRoundClear(int,unsigned int,unsigned int,char const*)
bool CParty::historyBloodRoundClear(int blood_round,
                                    unsigned int round_seconds,
                                    unsigned int total_play_seconds,
                                    const char* dungeon_name) {
    char member_names[256];
    std::memset(member_names, 0, sizeof(member_names));
    getMemberNames(member_names);
    for (int seat = 0; seat <= 3; ++seat) {
        CUser* const user = get_user(seat);
        if (user && static_cast<unsigned char>(checkValidUser(seat)) == 1u) {
            user->history_log().BloodRoundClear(
                blood_round,
                round_seconds,
                total_play_seconds,
                dungeon_name,
                member_names);
        }
    }
    // The old unoptimized loop leaves AL clear when seat advances to four.
    return false;
}
