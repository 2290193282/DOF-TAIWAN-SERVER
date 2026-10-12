#include "old_game/pvp/match/league_match_room_list_runtime.h"

#include <utility>

// IDA_EA: 0x8568FB0
// IDA_NAME: _ZN20CLeagueMatchRoomListC2Ev
// DEMANGLED: CLeagueMatchRoomList::CLeagueMatchRoomList(void)
CLeagueMatchRoomList::CLeagueMatchRoomList()
    : IRoomList(),
      waiting_rooms_() {
}

// IDA_EA: 0x8567D00
// IDA_NAME: _ZN20CLeagueMatchRoomList5MatchEP6IMatchPS1_
// DEMANGLED: CLeagueMatchRoomList::Match(IMatch *,IMatch **)
bool CLeagueMatchRoomList::Match(IMatch* match, IMatch** matched) {
    if (waiting_rooms_.empty()) {
        return false;
    }

    bool found = false;
    float selected_gap = 0.0f;
    std::map<IMatch*, int>::iterator selected = waiting_rooms_.end();
    for (std::map<IMatch*, int>::iterator it = waiting_rooms_.begin();
         it != waiting_rooms_.end(); ++it) {
        IMatch* candidate = it->first;
        if (candidate != match && match->CheckMatchable(candidate)) {
            const float gap = match->GetGap(candidate);
            if (!found) {
                selected_gap = gap;
                selected = it;
                found = true;
            }
            if (selected_gap > gap) {
                selected_gap = gap;
                selected = it;
            }
        }
    }

    if (selected == waiting_rooms_.end()) {
        return false;
    }
    *matched = selected->first;
    waiting_rooms_.erase(selected);
    return true;
}

// IDA_EA: 0x8567E9E
// IDA_NAME: _ZN20CLeagueMatchRoomList7AddRoomEP6IMatch
// DEMANGLED: CLeagueMatchRoomList::AddRoom(IMatch *)
bool CLeagueMatchRoomList::AddRoom(IMatch* match) {
    // Both inserts are present in the old body.  The first result is unused;
    // only the second tells the original code whether to increment the entry.
    waiting_rooms_.insert(std::make_pair(match, 1));
    const std::pair<std::map<IMatch*, int>::iterator, bool> result =
        waiting_rooms_.insert(std::make_pair(match, 1));
    if (!result.second) {
        ++result.first->second;
    }
    return true;
}

// IDA_EA: 0x8567F72
// IDA_NAME: _ZN20CLeagueMatchRoomList7DelRoomEP6IMatch
// DEMANGLED: CLeagueMatchRoomList::DelRoom(IMatch *)
bool CLeagueMatchRoomList::DelRoom(IMatch* match) {
    const std::map<IMatch*, int>::iterator found = waiting_rooms_.find(match);
    if (found == waiting_rooms_.end()) {
        return false;
    }
    waiting_rooms_.erase(found);
    return true;
}
