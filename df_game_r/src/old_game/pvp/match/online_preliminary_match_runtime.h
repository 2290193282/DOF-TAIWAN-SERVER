#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_ONLINE_PRELIMINARY_MATCH_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_ONLINE_PRELIMINARY_MATCH_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>

#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/pvp/match/fair_pvp_match_interface.h"
#include "old_game/pvp/match/online_preliminary_match_types.h"

class CLeagueMatchRoomList;
class CUser;
class Inven_Item;
class PvP_Room;
enum ENUM_ELO_RATING_SYSTEM : int;
enum ENUM_SCHOOL_MATCH_TEAM : int;
enum ENUM_SEAT_STATE : int;

namespace online_preliminary {

// IDA unit: 99_misc_global/online_preliminary/COnlinePreliminary.
//
// This is the actual 0x70-byte dynamic matching object selected for channel
// type 14.  It deliberately derives from the shared IMatch ABI: slots that
// remain inherited below are exactly the original IMatch entries in the
// COnlinePreliminary vtable, not local replacement defaults.
class COnlinePreliminary : public IMatch {
public:
    // IDA_EA: 0x855FEE8
    // IDA_NAME: _ZN18online_preliminary18COnlinePreliminary22GetPlayCountPerOneTeamEv
    static int GetPlayCountPerOneTeam();

    // IDA_EA: 0x855FEFE
    // IDA_NAME: _ZN18online_preliminary18COnlinePreliminaryC2Ev
    COnlinePreliminary();

    // IDA_EA: 0x855FFD6 (D2), 0x8560064 (D0)
    // IDA_NAME: _ZN18online_preliminary18COnlinePreliminaryD2Ev
    // IDA_NAME: _ZN18online_preliminary18COnlinePreliminaryD0Ev
    virtual ~COnlinePreliminary();

    float _GetRating(CUser* user);
    bool _SetRating(CUser* user, float rating);
    bool _IncreaseWinCount(CUser* user);
    bool _IncreaseLoseCount(CUser* user);
    int _GetEachOtherPlayCount(CUser* user, CUser* opponent);
    bool _IncreaseEachOtherPlayCount(CUser* user, CUser* opponent);
    float _GetRating(ENUM_SCHOOL_MATCH_TEAM team);
    bool _SetRating(ENUM_SCHOOL_MATCH_TEAM team, float rating);
    bool _IncreaseWinCount(ENUM_SCHOOL_MATCH_TEAM team);
    bool _IncreaseLoseCount(ENUM_SCHOOL_MATCH_TEAM team);
    int _GetEachOtherPlayCount();
    bool _IncreaseEachOtherPlayCount();
    bool _CheckSameTeam(PvP_Room* room, CUser* user);
    bool _CheckObserver(PvP_Room* room, CUser* user);

    // IMatch vtable slots 28/29, respectively.
    virtual int CheckJoinable(PvP_Room* room, CUser* user) override;
    virtual int CheckSeat(int seat) override;

    // IDA_EA: 0x85608D4 / 0x85608DA.  The first old body is exactly `ret`;
    // the second stores its one-byte argument in the static gate.
    static void SetPlayCountPerOneTeam(uint32_t count);
    static int SetMatchAble(bool matchable);

    virtual void RequestMap(int16_t& map_no,
                            PVP_BATTLE_MODE& battle_mode) override;
    static bool CheckUnusableEquipment(const Inven_Item* item);
    static int CheckUnusableEquipmentInUser(CUser* user);

    virtual int OnCreateRoom(CUser* user) override;
    virtual void OnDestroyRoom(CUser* user) override;
    virtual void OnJoinRoom(CUser* user) override;
    bool _UpdateRating(int team, ENUM_ELO_RATING_SYSTEM rating_system);
    virtual void OnLeaveRoom(CUser* user,
                             PvP_Room* room,
                             int team,
                             int seat) override;

    void _PullOutRoom(CUser* user, PvP_Room* room, int seat);
    int _PutInRoom(CUser* user, PvP_Room* room, bool move_room);
    void _BackupRoomInfo(PvP_Room* room);
    PvP_Room* _MakeRoom(CUser* user);
    int _RegisterWaitingRoom(PvP_Room* room);
    int _CheckMatchable(PvP_Room* room) const;

    virtual int OnRegisterMatch(CUser* user, PvP_Room* room) override;
    virtual int OnMatchRoom(PvP_Room* room, int auth_value) override;
    virtual float GetGap(IMatch* match) override;
    virtual void OnStartFight() override;
    virtual void OnCompleteLoad(CUser* user) override;
    virtual void OnConnectP2P(CUser* user,
                              PvP_Room* room,
                              char* result,
                              int count) override;
    virtual void OnConnectP2PTimeout(PvP_Room* room) override;
    virtual int OnGiveup(CUser* user) override;

    COnlinePreliminary* _ReturnTeam(PvP_Room* room,
                                    int retained_team,
                                    CUser* retained_user);
    int _GetOppositeTeam(int team);
    virtual int OnWalkOut(PvP_Room* room, int seat) override;
    virtual bool OnReady(CUser* user, bool ready) override;
    virtual void OnEndFight(PvP_Room* room) override;
    virtual CUser* GetUser(uint32_t index) override;
    virtual void GetUserList(void* destination) override;
    void _SendMatchStatePacket(int state, int team_count);
    virtual void OnReward(PvP_Room* room, int team, bool draw) override;
    int GetTeamId();
    virtual bool CheckMatchable(IMatch* match) override;
    int _GetTeam(ENUM_SEAT_STATE state);

public:
    int state_;                      // IDA +0x04
    PVP_ROOM_INFO room_info_;        // IDA +0x08
    int auth_value_;                 // IDA +0x18
    TeamInfoSecond teams_[2];        // IDA +0x1c, +0x44
    int p2p_ready_count_;            // IDA +0x6c

private:
    static CLeagueMatchRoomList* pWaitingRoomList_;
    static int nWaitingRoomListRefCount_;
    static int nextAuthValue_;
    static bool bMatchable_;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(offsetof(COnlinePreliminary, state_) == 0x04u,
              "COnlinePreliminary::state_ must stay at +0x04");
static_assert(offsetof(COnlinePreliminary, room_info_) == 0x08u,
              "COnlinePreliminary::room_info_ must stay at +0x08");
static_assert(offsetof(COnlinePreliminary, auth_value_) == 0x18u,
              "COnlinePreliminary::auth_value_ must stay at +0x18");
static_assert(offsetof(COnlinePreliminary, teams_) == 0x1cu,
              "COnlinePreliminary::teams_ must stay at +0x1c");
static_assert(offsetof(COnlinePreliminary, teams_) + sizeof(TeamInfoSecond) ==
                  0x44u,
              "COnlinePreliminary second team must stay at +0x44");
static_assert(offsetof(COnlinePreliminary, p2p_ready_count_) == 0x6cu,
              "COnlinePreliminary::p2p_ready_count_ must stay at +0x6c");
static_assert(sizeof(COnlinePreliminary) == 0x70u,
              "COnlinePreliminary IDA allocation must stay 0x70");
#endif

}  // namespace online_preliminary

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_ONLINE_PRELIMINARY_MATCH_RUNTIME_H_
