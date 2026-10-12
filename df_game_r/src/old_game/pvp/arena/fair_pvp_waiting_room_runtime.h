#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_FAIR_PVP_WAITING_ROOM_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_FAIR_PVP_WAITING_ROOM_RUNTIME_H_

#include <map>
#include <set>
#include <vector>

#include "old_game/pvp/match/fair_pvp_match_interface.h"

typedef std::multimap<float, IMatch*> MatchGapMultimap;

// IDA unit: 09_dungeon_pvp_party/CWaitingRoomList.
class CWaitingRoomList {
public:
    // IDA_EA: 0x856904E
    // IDA_NAME: _ZN16CWaitingRoomListC2Ev
    // DEMANGLED: CWaitingRoomList::CWaitingRoomList(void)
    CWaitingRoomList();

    // IDA_EA: 0x8569062
    // IDA_NAME: _ZN16CWaitingRoomListD2Ev
    // DEMANGLED: CWaitingRoomList::~CWaitingRoomList()
    ~CWaitingRoomList();

    // IDA_EA: 0x85696E8
    // IDA_NAME: _ZN16CWaitingRoomListC2ERKS_
    // DEMANGLED: CWaitingRoomList::CWaitingRoomList(CWaitingRoomList const&)
    CWaitingRoomList(const CWaitingRoomList& other);

    // IDA_EA: 0x8566D9C
    // IDA_NAME: _ZN16CWaitingRoomList7doMatchEP6IMatchRSt8multimapIfS1_St4lessIfESaISt4pairIKfS1_EEE
    // DEMANGLED: CWaitingRoomList::doMatch(IMatch *,std::multimap<float,IMatch *,std::less<float>,std::allocator<std::pair<float const,IMatch *>>> &)
    bool doMatch(IMatch* match, MatchGapMultimap& candidates);

    // IDA_EA: 0x8568872
    // IDA_NAME: _ZN16CWaitingRoomList4findEP6IMatch
    // DEMANGLED: CWaitingRoomList::find(IMatch *)
    bool find(IMatch* match);

    // IDA_EA: 0x85688BE
    // IDA_NAME: _ZN16CWaitingRoomList5eraseEP6IMatch
    // DEMANGLED: CWaitingRoomList::erase(IMatch *)
    int erase(IMatch* match);

    // IDA_EA: 0x85688D8
    // IDA_NAME: _ZN16CWaitingRoomList6insertEP6IMatch
    // DEMANGLED: CWaitingRoomList::insert(IMatch *)
    int insert(IMatch* match);

    std::set<IMatch*> waiting_rooms;
};

// IDA unit: 09_dungeon_pvp_party/CMultiMatchRoomList.
class CMultiMatchRoomList : public IRoomList {
public:
    // IDA_EA: 0x8566EFC
    // IDA_NAME: _ZN19CMultiMatchRoomListC2Ev
    // DEMANGLED: CMultiMatchRoomList::CMultiMatchRoomList(void)
    CMultiMatchRoomList();

    // IDA_EA: 0x856703C
    // IDA_NAME: _ZN19CMultiMatchRoomList25GetWaitingRoomCombinationEP6IMatchiRSt6vectorIS1_SaIS1_EE
    // DEMANGLED: CMultiMatchRoomList::GetWaitingRoomCombination(IMatch *,int,std::vector<IMatch *,std::allocator<IMatch *>> &)
    bool GetWaitingRoomCombination(IMatch* match, int needed_count,
                                   std::vector<IMatch*>& combination);

    // IDA_EA: 0x85675D0
    // IDA_NAME: _ZN19CMultiMatchRoomList5MatchEP6IMatchPS1_
    // DEMANGLED: CMultiMatchRoomList::Match(IMatch *,IMatch **)
    virtual bool Match(IMatch* match, IMatch** matched);

    // IDA_EA: 0x8567954
    // IDA_NAME: _ZN19CMultiMatchRoomList7AddRoomEP6IMatch
    // DEMANGLED: CMultiMatchRoomList::AddRoom(IMatch *)
    virtual bool AddRoom(IMatch* match);

    // IDA_EA: 0x8567BBA
    // IDA_NAME: _ZN19CMultiMatchRoomList7DelRoomEP6IMatch
    // DEMANGLED: CMultiMatchRoomList::DelRoom(IMatch *)
    virtual bool DelRoom(IMatch* match);

    std::map<int, CWaitingRoomList> waiting_room_lists;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CWaitingRoomList) == 0x18u,
              "CWaitingRoomList IDA ABI size must stay 0x18");
static_assert(sizeof(CMultiMatchRoomList) == 0x1cu,
              "CMultiMatchRoomList IDA ABI size must stay 0x1c");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_FAIR_PVP_WAITING_ROOM_RUNTIME_H_
