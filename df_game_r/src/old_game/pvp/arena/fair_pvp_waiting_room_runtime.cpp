#include "old_game/pvp/arena/fair_pvp_waiting_room_runtime.h"

#include <array>

#include "infra/log/format/trace.h"
#include "old_game/pvp/match/fair_pvp_match_runtime.h"

namespace {

// Old read-only table at 0x8CA3200.  Each nonzero part gives a waiting-room
// key and the number of matches selected from that key.  Its original span is
// 7 * 10 * 7 * 2 = 0x3d4 bytes; zero-initialized trailing parts are literal
// zero bytes from that span, not a runtime fallback.
struct WaitingRoomCombinationPart {
    uint8_t room_size;
    uint8_t take_count;
};

const WaitingRoomCombinationPart kWaitingRoomCombinations[7][10][7] = {
    {},
    {
        { {1, 1} }
    },
    {
        { {1, 2} },
        { {2, 1} }
    },
    {
        { {1, 3} },
        { {2, 1}, {1, 1} },
        { {3, 1} }
    },
    {
        { {1, 4} },
        { {2, 1}, {1, 2} },
        { {2, 1}, {2, 1} },
        { {3, 1}, {1, 1} },
        { {4, 1} }
    },
    {
        { {1, 5} },
        { {2, 1}, {1, 3} },
        { {2, 2}, {1, 1} },
        { {3, 1}, {2, 1} },
        { {3, 1}, {1, 2} },
        { {4, 1}, {1, 1} }
    },
    {
        { {1, 6} },
        { {2, 1}, {1, 4} },
        { {2, 2}, {1, 2} },
        { {2, 3} },
        { {3, 1}, {1, 3} },
        { {3, 1}, {2, 1}, {1, 1} },
        { {4, 1}, {1, 2} },
        { {4, 1}, {2, 1} },
        { {5, 1}, {1, 1} }
    }
};

}  // namespace

// IDA_EA: 0x856904E
// IDA_NAME: _ZN16CWaitingRoomListC2Ev
// DEMANGLED: CWaitingRoomList::CWaitingRoomList(void)
CWaitingRoomList::CWaitingRoomList() {
}

// IDA_EA: 0x8569062
// IDA_NAME: _ZN16CWaitingRoomListD2Ev
// DEMANGLED: CWaitingRoomList::~CWaitingRoomList()
CWaitingRoomList::~CWaitingRoomList() {
}

// IDA_EA: 0x85696E8
// IDA_NAME: _ZN16CWaitingRoomListC2ERKS_
// DEMANGLED: CWaitingRoomList::CWaitingRoomList(CWaitingRoomList const&)
CWaitingRoomList::CWaitingRoomList(const CWaitingRoomList& other)
    : waiting_rooms(other.waiting_rooms) {
}

// IDA_EA: 0x8566D9C
// IDA_NAME: _ZN16CWaitingRoomList7doMatchEP6IMatchRSt8multimapIfS1_St4lessIfESaISt4pairIKfS1_EEE
// DEMANGLED: CWaitingRoomList::doMatch(IMatch *,std::multimap<float,IMatch *,std::less<float>,std::allocator<std::pair<float const,IMatch *>>> &)
bool CWaitingRoomList::doMatch(IMatch* match, MatchGapMultimap& candidates) {
    if (waiting_rooms.empty()) {
        return false;
    }

    for (std::set<IMatch*>::const_iterator it = waiting_rooms.begin();
         it != waiting_rooms.end(); ++it) {
        IMatch* candidate = *it;
        if (candidate != match && match->CheckMatchable(candidate)) {
            candidates.insert(std::make_pair(match->GetGap(candidate), candidate));
        }
    }
    return !candidates.empty();
}

// IDA_EA: 0x8568872
// IDA_NAME: _ZN16CWaitingRoomList4findEP6IMatch
// DEMANGLED: CWaitingRoomList::find(IMatch *)
bool CWaitingRoomList::find(IMatch* match) {
    return waiting_rooms.find(match) != waiting_rooms.end();
}

// IDA_EA: 0x85688BE
// IDA_NAME: _ZN16CWaitingRoomList5eraseEP6IMatch
// DEMANGLED: CWaitingRoomList::erase(IMatch *)
int CWaitingRoomList::erase(IMatch* match) {
    return static_cast<int>(waiting_rooms.erase(match));
}

// IDA_EA: 0x85688D8
// IDA_NAME: _ZN16CWaitingRoomList6insertEP6IMatch
// DEMANGLED: CWaitingRoomList::insert(IMatch *)
int CWaitingRoomList::insert(IMatch* match) {
    return waiting_rooms.insert(match).second;
}

// IDA_EA: 0x8566EFC
// IDA_NAME: _ZN19CMultiMatchRoomListC2Ev
// DEMANGLED: CMultiMatchRoomList::CMultiMatchRoomList(void)
CMultiMatchRoomList::CMultiMatchRoomList() {
    for (int index = 0;
         static_cast<uint8_t>(fair_pvp::CFairMatch::GetPlayCountPerOneTeam()) > index;
         ++index) {
        waiting_room_lists.insert(
            std::make_pair(index + 1, CWaitingRoomList()));
    }
}

// IDA_EA: 0x856703C
// IDA_NAME: _ZN19CMultiMatchRoomList25GetWaitingRoomCombinationEP6IMatchiRSt6vectorIS1_SaIS1_EE
// DEMANGLED: CMultiMatchRoomList::GetWaitingRoomCombination(IMatch *,int,std::vector<IMatch *,std::allocator<IMatch *>> &)
bool CMultiMatchRoomList::GetWaitingRoomCombination(
    IMatch* match, int needed_count, std::vector<IMatch*>& combination) {
    std::array<std::vector<IMatch*>, 10> matched_rooms;
    std::array<float, 10> average_gaps = {{0.0f}};
    MatchGapMultimap candidates;
    int valid_combination_count = 0;

    for (int combination_index = 0;
         combination_index <= 9 &&
         kWaitingRoomCombinations[needed_count][combination_index][0].room_size;
         ++combination_index) {
        int matched_player_count = 0;
        for (int part_index = 0; part_index <= 6; ++part_index) {
            const WaitingRoomCombinationPart& part =
                kWaitingRoomCombinations[needed_count][combination_index][part_index];
            if (!part.room_size) {
                break;
            }

            candidates.clear();
            std::map<int, CWaitingRoomList>::iterator room_list =
                waiting_room_lists.find(part.room_size);
            if (!room_list->second.doMatch(match, candidates)) {
                break;
            }
            if (candidates.size() < part.take_count) {
                break;
            }

            int selected_count = 0;
            for (MatchGapMultimap::iterator candidate = candidates.begin();
                 candidate != candidates.end() && selected_count < part.take_count;
                 ++candidate, ++selected_count) {
                IMatch* selected_match = candidate->second;
                const int selected_player_count = selected_match->GetPlayerCount();
                average_gaps[combination_index] =
                    (average_gaps[combination_index] * matched_player_count +
                     selected_player_count * candidate->first) /
                    static_cast<float>(matched_player_count + selected_player_count);
                matched_player_count += selected_player_count;
                matched_rooms[combination_index].push_back(selected_match);
            }
        }

        if (!matched_rooms[combination_index].empty()) {
            if (matched_player_count == needed_count) {
                ++valid_combination_count;
            } else {
                matched_rooms[combination_index].clear();
            }
        }
    }

    if (!valid_combination_count) {
        return false;
    }

    bool selected = false;
    int selected_index = 0;
    float selected_gap = 0.0f;
    for (int combination_index = 0; combination_index <= 9;
         ++combination_index) {
        if (!matched_rooms[combination_index].empty()) {
            if (!selected) {
                selected_index = combination_index;
                selected_gap = average_gaps[combination_index];
                selected = true;
            }
            if (selected_gap > average_gaps[combination_index]) {
                selected_gap = average_gaps[combination_index];
                selected_index = combination_index;
            }
        }
    }
    combination.insert(combination.end(), matched_rooms[selected_index].begin(),
                       matched_rooms[selected_index].end());
    return true;
}

// IDA_EA: 0x85675D0
// IDA_NAME: _ZN19CMultiMatchRoomList5MatchEP6IMatchPS1_
// DEMANGLED: CMultiMatchRoomList::Match(IMatch *,IMatch **)
bool CMultiMatchRoomList::Match(IMatch* match, IMatch** matched) {
    if (waiting_room_lists.empty()) {
        return false;
    }

    std::vector<IMatch*> selected_matches;
    const int player_count = match->GetPlayerCount();
    if (player_count <
        static_cast<uint8_t>(fair_pvp::CFairMatch::GetPlayCountPerOneTeam())) {
        const int needed_count =
            static_cast<uint8_t>(fair_pvp::CFairMatch::GetPlayCountPerOneTeam()) -
            player_count;
        if (!GetWaitingRoomCombination(match, needed_count, selected_matches)) {
            return false;
        }
        for (std::vector<IMatch*>::iterator it = selected_matches.begin();
             it != selected_matches.end(); ++it) {
            DelRoom(*it);
        }
    }

    if (!GetWaitingRoomCombination(
            match,
            static_cast<uint8_t>(fair_pvp::CFairMatch::GetPlayCountPerOneTeam()),
            selected_matches)) {
        for (std::vector<IMatch*>::iterator it = selected_matches.begin();
             it != selected_matches.end(); ++it) {
            AddRoom(*it);
        }
        return false;
    }

    if (selected_matches.empty()) {
        return false;
    }
    if (selected_matches.size() == 1) {
        DelRoom(selected_matches.front());
        *matched = selected_matches.front();
        return true;
    }

    IMatch* previous = selected_matches.front();
    *matched = previous;
    for (std::vector<IMatch*>::iterator it = selected_matches.begin() + 1;
         it != selected_matches.end(); ++it) {
        DelRoom(*it);
        previous->SetNextMatch(*it);
        previous = *it;
    }
    return true;
}

// IDA_EA: 0x8567954
// IDA_NAME: _ZN19CMultiMatchRoomList7AddRoomEP6IMatch
// DEMANGLED: CMultiMatchRoomList::AddRoom(IMatch *)
bool CMultiMatchRoomList::AddRoom(IMatch* match) {
    for (std::map<int, CWaitingRoomList>::iterator it =
             waiting_room_lists.begin();
         it != waiting_room_lists.end(); ++it) {
        if (it->second.find(match)) {
            LogManager::logFormat(
                1, "MatchingSystem.cpp",
                "virtual bool CMultiMatchRoomList::AddRoom(IMatch*)", 0x11F3,
                "error find line(%d)", 0x11F3);
            return false;
        }
    }

    std::map<int, CWaitingRoomList>::iterator room_list =
        waiting_room_lists.find(match->GetPlayerCount());
    if (room_list != waiting_room_lists.end()) {
        room_list->second.insert(match);
    } else {
        LogManager::logFormat(
            1, "MatchingSystem.cpp",
            "virtual bool CMultiMatchRoomList::AddRoom(IMatch*)", 0x11FF,
            "error find line(%d), playCount(%d)", 0x11FF,
            match->GetPlayerCount());
    }
    return true;
}

// IDA_EA: 0x8567BBA
// IDA_NAME: _ZN19CMultiMatchRoomList7DelRoomEP6IMatch
// DEMANGLED: CMultiMatchRoomList::DelRoom(IMatch *)
bool CMultiMatchRoomList::DelRoom(IMatch* match) {
    for (std::map<int, CWaitingRoomList>::iterator it =
             waiting_room_lists.begin();
         it != waiting_room_lists.end(); ++it) {
        it->second.erase(match);
    }
    return true;
}
