#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_LEAGUE_MATCH_ROOM_LIST_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_LEAGUE_MATCH_ROOM_LIST_RUNTIME_H_

#include <map>

#include "old_game/pvp/match/fair_pvp_match_interface.h"

// IDA unit: 09_dungeon_pvp_party/CLeagueMatchRoomList.
//
// The old object is an IRoomList vptr at +0x00 followed by the only owned
// state, std::map<IMatch *, int>, at +0x04.  Its owners manually delete the
// allocation after their own refcount gates, and the IDB has no derived D2/D0
// identity, so this declaration deliberately exposes only the four proven
// member identities.
class CLeagueMatchRoomList : public IRoomList {
public:
    // IDA_EA: 0x8568FB0
    // IDA_NAME: _ZN20CLeagueMatchRoomListC2Ev
    // DEMANGLED: CLeagueMatchRoomList::CLeagueMatchRoomList(void)
    CLeagueMatchRoomList();

    // IDA_EA: 0x8567E9E
    // IDA_NAME: _ZN20CLeagueMatchRoomList7AddRoomEP6IMatch
    // DEMANGLED: CLeagueMatchRoomList::AddRoom(IMatch *)
    virtual bool AddRoom(IMatch* match) override;

    // IDA_EA: 0x8567F72
    // IDA_NAME: _ZN20CLeagueMatchRoomList7DelRoomEP6IMatch
    // DEMANGLED: CLeagueMatchRoomList::DelRoom(IMatch *)
    virtual bool DelRoom(IMatch* match) override;

    // IDA_EA: 0x8567D00
    // IDA_NAME: _ZN20CLeagueMatchRoomList5MatchEP6IMatchPS1_
    // DEMANGLED: CLeagueMatchRoomList::Match(IMatch *,IMatch **)
    virtual bool Match(IMatch* match, IMatch** matched) override;

private:
    std::map<IMatch*, int> waiting_rooms_;  // +0x04
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(std::map<IMatch*, int>) == 0x18u,
              "CLeagueMatchRoomList map ABI size must stay 0x18");
static_assert(sizeof(CLeagueMatchRoomList) == 0x1cu,
              "CLeagueMatchRoomList IDA ABI size must stay 0x1c");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_LEAGUE_MATCH_ROOM_LIST_RUNTIME_H_
