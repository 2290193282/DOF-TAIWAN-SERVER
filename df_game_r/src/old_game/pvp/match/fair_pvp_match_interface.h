#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_FAIR_PVP_MATCH_INTERFACE_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_FAIR_PVP_MATCH_INTERFACE_H_

#include <stdint.h>

#include "old_data/PVF/pvp_mission/datamanager/pvp_mission_system.h"

class CUser;
class PvP_Room;

class IMatch {
public:
    // IDA_EA: 0x8568998
    // IDA_NAME: _ZN6IMatchC2Ev
    // DEMANGLED: IMatch::IMatch(void)
    IMatch();

    // IDA_EA: 0x8568664 (complete destructor)
    // IDA_NAME: _ZN6IMatchD2Ev
    // DEMANGLED: IMatch::~IMatch()
    // IDA_EA: 0x856868A (deleting destructor)
    // IDA_NAME: _ZN6IMatchD0Ev
    // DEMANGLED: IMatch::~IMatch()
    virtual ~IMatch();

    // The order below is the old vtable order at 0x8CA3DE8.  It is shared
    // by CFairMatch and the matching room-list algorithm.
    virtual int GetIndentification();
    virtual float GetGap(IMatch* match);
    virtual CUser* GetUser(uint32_t index);
    // The original mangled ABI is `void *` because match families emit
    // different fixed-size result records.  It is an output buffer contract,
    // not an ownership or generic-layout escape hatch; each override writes
    // its own IDA-evidenced record shape and returns void.
    virtual void GetUserList(void* destination);
    virtual void RequestMap(int16_t& map_no, PVP_BATTLE_MODE& battle_mode);
    virtual int GetExtraWinPoint(int value);
    virtual int OnCreateRoom(CUser* user);
    virtual void OnDestroyRoom(CUser* user);
    virtual void OnJoinRoom(CUser* user);
    virtual void OnLeaveRoom(CUser* user, PvP_Room* room, int team, int seat);
    virtual int OnRegisterMatch(CUser* user, PvP_Room* room);
    virtual int OnUnregisterMatch(CUser* user, PvP_Room* room);
    virtual int OnMatchRoom(PvP_Room* room, int auth_value);
    virtual void OnStartFight();
    virtual void OnEndFight(PvP_Room* room);
    virtual int OnWalkOut(PvP_Room* room, int seat);
    virtual bool OnReady(CUser* user, bool ready);
    virtual void OnReward(PvP_Room* room, int team, bool draw);
    virtual void OnEndPvP(PvP_Room* room);
    virtual void OnCompleteLoad(CUser* user);
    virtual void OnConnectP2PTimeout(PvP_Room* room);
    virtual void OnConnectP2P(CUser* user, PvP_Room* room, char* result, int count);
    virtual int OnGiveup(CUser* user);
    virtual void OnRematchTimeout(PvP_Room* room);
    virtual void OnAskRematch(CUser* user, int answer);
    virtual void OnWaitRematch(CUser* user, PvP_Room* room);
    virtual int CheckJoinable(PvP_Room* room, CUser* user);
    virtual int CheckSeat(int seat);
    virtual bool CheckMatchable(IMatch* match);
    virtual int GetEndPvPResultTimeout();
    virtual void OnEndPvPResult(PvP_Room* room, CUser* user);
    virtual int GetPlayerCount();
    virtual IMatch* GetNextMatch();
    virtual void SetNextMatch(IMatch* match);
};

// IDA unit: 09_dungeon_pvp_party/IRoomList.  The old vtable contains exactly
// these three pure virtual entries; it deliberately has no virtual destructor.
class IRoomList {
public:
    // IDA_EA: 0x8568FA2
    // IDA_NAME: _ZN9IRoomListC2Ev
    // DEMANGLED: IRoomList::IRoomList(void)
    IRoomList();

    virtual bool AddRoom(IMatch* match) = 0;
    virtual bool DelRoom(IMatch* match) = 0;
    virtual bool Match(IMatch* match, IMatch** matched) = 0;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(IMatch) == 0x04u,
              "IMatch IDA ABI size must stay 0x04");
static_assert(sizeof(IRoomList) == 0x04u,
              "IRoomList IDA ABI size must stay 0x04");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_FAIR_PVP_MATCH_INTERFACE_H_
