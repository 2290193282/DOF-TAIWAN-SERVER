#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_FAIR_PVP_MATCH_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_FAIR_PVP_MATCH_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>

#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/pvp/match/fair_pvp_match_interface.h"
#include "old_game/pvp/battlefield/fair_pvp_team_info_runtime.h"

class CUser;
class CMultiMatchRoomList;
class Inven_Item;
class PvP_Room;

// These value sets are recovered from the three _UpdateRating branches and
// the two TeamInfoThird array indices in the old IDB.  The fixed underlying
// type is part of the i386 member-call ABI.
enum ENUM_ELO_RATING_SYSTEM : int {
    ENUM_ELO_RATING_SYSTEM_0 = 0,
    ENUM_ELO_RATING_SYSTEM_1 = 1,
    ENUM_ELO_RATING_SYSTEM_2 = 2
};

enum ENUM_SCHOOL_MATCH_TEAM : int {
    ENUM_SCHOOL_MATCH_TEAM_0 = 0,
    ENUM_SCHOOL_MATCH_TEAM_1 = 1,
    ENUM_SCHOOL_MATCH_TEAM_INVALID = 2
};

namespace fair_pvp {

// IDA unit: 09_dungeon_pvp_party/fair_pvp/CFairMatch.
class CFairMatch : public IMatch {
public:
    // IDA_EA: 0x8562834
    // IDA_NAME: _ZN8fair_pvp10CFairMatch22GetPlayCountPerOneTeamEv
    // DEMANGLED: fair_pvp::CFairMatch::GetPlayCountPerOneTeam(void)
    static int GetPlayCountPerOneTeam();

    // IDA_EA: 0x856284A
    // IDA_NAME: _ZN8fair_pvp10CFairMatchC2Ev
    // DEMANGLED: fair_pvp::CFairMatch::CFairMatch(void)
    CFairMatch();

    // IDA_EA: 0x856292E (complete destructor)
    // IDA_NAME: _ZN8fair_pvp10CFairMatchD2Ev
    // DEMANGLED: fair_pvp::CFairMatch::~CFairMatch()
    // IDA_EA: 0x85629CA (deleting destructor)
    // IDA_NAME: _ZN8fair_pvp10CFairMatchD0Ev
    // DEMANGLED: fair_pvp::CFairMatch::~CFairMatch()
    virtual ~CFairMatch();

    int _CheckSameTeam(PvP_Room* room, CUser* user);
    bool _CheckObserver(PvP_Room* room, CUser* user);
    virtual int CheckJoinable(PvP_Room* room, CUser* user);
    virtual int CheckSeat(int seat);
    static void SetPlayCountPerOneTeam(uint32_t count);
    static void SetMatchAble(bool matchable);
    virtual void RequestMap(int16_t& map_no, PVP_BATTLE_MODE& battle_mode);
    // HARD BLOCKER — IDA_EA: 0x8562BDC
    // IDA_NAME: _ZN8fair_pvp10CFairMatch22CheckUnusableEquipmentEPK10Inven_Item
    // Restored through CDataManager::get_server_parameter_script(), the
    // typed owner for the exact embedded ServerParameterScript used by the
    // original equipment gate.
    static bool CheckUnusableEquipment(const Inven_Item* item);
    static int CheckUnusableEquipmentInUser(CUser* user);
    virtual int OnCreateRoom(CUser* user);
    virtual void OnDestroyRoom(CUser* user);
    virtual void OnJoinRoom(CUser* user);
    bool _UpdateRating(int team, ENUM_ELO_RATING_SYSTEM rating_system);
    virtual void OnLeaveRoom(CUser* user, PvP_Room* room, int team, int seat);
    void _ChangeTeamLeader(PvP_Room* room,
                           ENUM_SCHOOL_MATCH_TEAM team,
                           int departed_seat);
    void _PullOutRoom(CUser* user, PvP_Room* room, int seat);
    int _PutInRoom(CUser* user, PvP_Room* room, bool move_room);
    void _BackupRoomInfo(PvP_Room* room);
    PvP_Room* _MakeRoom(CUser* user);
    int _RegisterWaitingRoom(PvP_Room* room);
    int _CheckMatchable(PvP_Room* room) const;
    bool _CheckRematchable(PvP_Room* room) const;
    virtual int OnRegisterMatch(CUser* user, PvP_Room* room);
    virtual int OnUnregisterMatch(CUser* user, PvP_Room* room);
    bool _MoveRoom(PvP_Room* destination, PvP_Room* source, bool primary);
    int _CheckMovableRoom(IMatch* match, PvP_Room* source,
                          PvP_Room** result_room);
    virtual int OnMatchRoom(PvP_Room* room, int auth_value);
    virtual float GetGap(IMatch* match);
    virtual void OnStartFight();
    virtual void OnCompleteLoad(CUser* user);
    // HARD BLOCKER — IDA_EA: 0x8564A8E
    // IDA_NAME: _ZN8fair_pvp10CFairMatch12OnConnectP2PEP5CUserP8PvP_RoomPci
    // This original body addresses CRelayBattleMgr embedded in PvP_Room at
    // +0x620.  PvP_Room has no recovered typed owner layout/accessor.
    virtual void OnConnectP2P(CUser* user, PvP_Room* room,
                              char* result, int count);
    virtual void OnConnectP2PTimeout(PvP_Room* room);
    virtual int OnGiveup(CUser* user);
    PvP_Room* _ReturnTeam(PvP_Room* room, int team, CUser* retained_user);
    int _GetOppositeTeam(int team);
    virtual int OnWalkOut(PvP_Room* room, int seat);
    virtual bool OnReady(CUser* user, bool ready);
    virtual void OnEndFight(PvP_Room* room);
    virtual CUser* GetUser(uint32_t index);
    // Writes the two TeamInfoThird records (0x48 bytes) evidenced at
    // 0x8565184; the inherited ABI intentionally returns void.
    virtual void GetUserList(void* destination);
    void _SendMatchStatePacket(int state, int team_count);
    // HARD BLOCKER — IDA_EA: 0x85652C8
    // IDA_NAME: _ZN8fair_pvp10CFairMatch16GiveTrainingSealEP5CUserb
    // The old training-seal fields are exposed through the typed
    // ServerParameterScript owner bridge.
    void GiveTrainingSeal(CUser* user, bool won);
    // HARD BLOCKER — IDA_EA: 0x8565700
    // IDA_NAME: _ZN8fair_pvp10CFairMatch11UpdateScoreEP8PvP_RoomRNS_15PlayerInfoThirdE22ENUM_SCHOOL_MATCH_TEAMiii
    // Uses the recovered PvP_Room relay manager and training-seal owner.
    char UpdateScore(PvP_Room* room, PlayerInfoThird& player,
                     ENUM_SCHOOL_MATCH_TEAM team, int won, int lost,
                     int draw);
    virtual void OnReward(PvP_Room* room, int team, bool draw);
    virtual void OnEndPvP(PvP_Room* room);
    virtual bool CheckMatchable(IMatch* match);
    int CheckRematch(PvP_Room* room);
    int _PullOutRoomTeam(PvP_Room* room, ENUM_SCHOOL_MATCH_TEAM team);
    // HARD BLOCKER — IDA_EA: 0x8565F92
    // IDA_NAME: _ZN8fair_pvp10CFairMatch16OnRematchTimeoutEP8PvP_Room
    // Like OnConnectP2P, the original starts embedded Relay players through
    // an unrecovered PvP_Room owner layout at +0x620.
    virtual void OnRematchTimeout(PvP_Room* room);
    int _FindTeam(CUser* user);
    int _GetOtherTeam(ENUM_SCHOOL_MATCH_TEAM team);
    virtual void OnWaitRematch(CUser* user, PvP_Room* room);
    virtual void OnAskRematch(CUser* user, int answer);
    virtual void OnEndPvPResult(PvP_Room* room, CUser* user);
    virtual int GetPlayerCount();
    virtual int GetEndPvPResultTimeout();
    virtual IMatch* GetNextMatch();
    virtual void SetNextMatch(IMatch* match);
    void _SetEnterCharacNo(int charac_no);
    float _GetRating(ENUM_SCHOOL_MATCH_TEAM team);

public:
    int state_;                         // +0x04
    PVP_ROOM_INFO room_info_;           // +0x08
    int auth_value_;                    // +0x18
    TeamInfoThird teams_[2];            // +0x1c
    int p2p_ready_count_;               // +0xac
    int rewarded_round_count_;          // +0xb0
    int enter_charac_no_;               // +0xb4
    int team_leader_seat_[2];           // +0xb8
    IMatch* next_match_;                // +0xc0

private:
    static CMultiMatchRoomList* pWaitingRoomList_;
    static int nWaitingRoomListRefCount_;
    static int nextAuthValue_;
    static bool bMatchable_;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(offsetof(CFairMatch, room_info_) == 0x08u,
              "CFairMatch::room_info_ must stay at +0x08");
static_assert(offsetof(CFairMatch, teams_) == 0x1cu,
              "CFairMatch::teams_ must stay at +0x1c");
static_assert(offsetof(CFairMatch, p2p_ready_count_) == 0xacu,
              "CFairMatch::p2p_ready_count_ must stay at +0xac");
static_assert(offsetof(CFairMatch, next_match_) == 0xc0u,
              "CFairMatch::next_match_ must stay at +0xc0");
static_assert(sizeof(CFairMatch) == 0xc4u,
              "CFairMatch IDA ABI size must stay 0xc4");
#endif

}  // namespace fair_pvp

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_FAIR_PVP_MATCH_RUNTIME_H_
