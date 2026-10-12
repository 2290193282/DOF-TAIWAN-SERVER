#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_LEAGUE_MATCH_STATIC_STATE_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_LEAGUE_MATCH_STATIC_STATE_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>

#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/pvp/match/fair_pvp_match_interface.h"
#include "old_game/pvp/match/league_match_leaf_runtime.h"

class CLeagueMatchRoomList;

// IDA unit: 99_misc_global/CLeagueMatch.  The object is not a convenience
// facade: its only instance state is the IMatch base followed by the exact
// scalar/PVP_ROOM_INFO/two-TeamInfo sequence below (0xfc bytes on i386).
//
// The virtual declarations intentionally mirror the target vtable.  Methods
// not yet restored have declarations only; inheriting an IMatch default in
// one of those slots would silently produce a different CLeagueMatch vtable.
class CLeagueMatch : public IMatch {
public:
    // IDA_EA: 0x855CD74
    // IDA_NAME: _ZN12CLeagueMatchC2Ev
    // DEMANGLED: CLeagueMatch::CLeagueMatch(void)
    CLeagueMatch();

    // IDA_EA: 0x855CE4C (complete destructor)
    // IDA_NAME: _ZN12CLeagueMatchD2Ev
    // IDA_EA: 0x855CEDA (deleting destructor)
    // IDA_NAME: _ZN12CLeagueMatchD0Ev
    // DEMANGLED: CLeagueMatch::~CLeagueMatch()
    virtual ~CLeagueMatch() override;

    // These are listed in the target vtable order inherited from IMatch.
    // A declaration without a body marks a real still-missing provider;
    // it must not fall back to an unrelated IMatch implementation.
    // IDA_EA: 0x8568834
    int GetIndentification() override;
    // IDA_EA: 0x855E12A
    float GetGap(IMatch* match) override;
    // IDA_EA: 0x855E5E4
    CUser* GetUser(uint32_t index) override;
    // IDA_EA: 0x855E642
    void GetUserList(void* destination) override;
    // IDA_EA: 0x855D096
    void RequestMap(int16_t& map_no, PVP_BATTLE_MODE& battle_mode) override;
    // IDA_EA: 0x855D0BC
    int GetExtraWinPoint(int value) override;
    // IDA_EA: 0x855D104
    int OnCreateRoom(CUser* user) override;
    // IDA_EA: 0x855D168
    void OnDestroyRoom(CUser* user) override;
    // IDA_EA: 0x855D198
    void OnJoinRoom(CUser* user) override;
    // IDA_EA: 0x855D1C0
    void OnLeaveRoom(CUser* user, PvP_Room* room, int team, int seat) override;
    // IDA_EA: 0x855DB58
    int OnRegisterMatch(CUser* user, PvP_Room* room) override;
    // IDA_EA: 0x855DC64
    int OnMatchRoom(PvP_Room* room, int auth_value) override;
    // IDA_EA: 0x855E21C
    void OnStartFight() override;
    // IDA_EA: 0x855E55A
    void OnEndFight(PvP_Room* room) override;
    // IDA_EA: 0x855E4C4
    int OnWalkOut(PvP_Room* room, int seat) override;
    // IDA_EA: 0x855E52A
    bool OnReady(CUser* user, bool ready) override;
    // IDA_EA: 0x855EB22
    void OnReward(PvP_Room* room, int team, bool draw) override;
    // IDA_EA: 0x855E22C
    void OnCompleteLoad(CUser* user) override;
    // IDA_EA: 0x855CF8C
    int CheckJoinable(PvP_Room* room, CUser* user) override;
    // IDA_EA: 0x855D058
    int CheckSeat(int seat) override;
    // IDA_EA: 0x855FA14
    bool CheckMatchable(IMatch* match) override;

    // IDA_EA: 0x855F9AA
    // IDA_NAME: _ZN12CLeagueMatch11GetSchoolNoEv
    // DEMANGLED: CLeagueMatch::GetSchoolNo(void)
    // This is the CLeague-only virtual tail entry at vptr +0x90.  In
    // particular, GetIndentification dispatches this slot instead of
    // returning a hard-coded match identity.
    virtual int GetSchoolNo();

    // IDA_EA: 0x855CEF8
    // IDA_NAME: _ZN12CLeagueMatch12_CheckSchoolEP8PvP_RoomP5CUser
    // DEMANGLED: CLeagueMatch::_CheckSchool(PvP_Room *,CUser *)
    int _CheckSchool(PvP_Room* room, CUser* user);

    // IDA_EA: 0x855DA56
    // IDA_NAME: _ZNK12CLeagueMatch15_CheckMatchableEP8PvP_Room
    // DEMANGLED: CLeagueMatch::_CheckMatchable(PvP_Room *)const
    int _CheckMatchable(PvP_Room* room) const;

private:
    // The following helpers are non-virtual members from the same IDA unit.
    // They preserve the original room handoff and notification contracts; no
    // matching-system facade is involved.
    // IDA_EA: 0x855D3D2
    // IDA_NAME: _ZN12CLeagueMatch12_PullOutRoomEP5CUserP8PvP_Roomi
    void _PullOutRoom(CUser* user, PvP_Room* room, int seat);

    // IDA_EA: 0x855D42A
    // IDA_NAME: _ZN12CLeagueMatch10_PutInRoomEP5CUserP8PvP_Roomb
    int _PutInRoom(CUser* user, PvP_Room* room, bool move_room);

    // IDA_EA: 0x855D554
    // IDA_NAME: _ZN12CLeagueMatch15_BackupRoomInfoEP8PvP_Room
    void _BackupRoomInfo(PvP_Room* room);

    // IDA_EA: 0x855D572
    // IDA_NAME: _ZN12CLeagueMatch9_MakeRoomEP5CUser
    PvP_Room* _MakeRoom(CUser* user);

    // IDA_EA: 0x855D884
    // IDA_NAME: _ZN12CLeagueMatch20_RegisterWaitingRoomEP8PvP_Room
    int _RegisterWaitingRoom(PvP_Room* room);

    // IDA_EA: 0x855E378
    // IDA_NAME: _ZN12CLeagueMatch11_ReturnTeamEP8PvP_RoomiP5CUser
    // The original returns this, not the newly-created PvP_Room.
    CLeagueMatch* _ReturnTeam(PvP_Room* room,
                              int retained_team,
                              CUser* retained_user);

    // IDA_EA: 0x855E664
    // IDA_NAME: _ZN12CLeagueMatch21_SendMatchStatePacketEii
    void _SendMatchStatePacket(int state, int team_count);

public:

    // IDA_EA: 0x84ED37B
    // IDA_NAME: _ZN12CLeagueMatch22GetPlayCountPerOneTeamEv
    // DEMANGLED: CLeagueMatch::GetPlayCountPerOneTeam(void)
    static int GetPlayCountPerOneTeam();

    // IDA_EA: 0x84ED387
    // IDA_NAME: _ZN12CLeagueMatch13GetBattleModeEv
    // DEMANGLED: CLeagueMatch::GetBattleMode(void)
    static int GetBattleMode();

    // IDA_EA: 0x855D070
    // IDA_NAME: _ZN12CLeagueMatch22SetPlayCountPerOneTeamEj
    // DEMANGLED: CLeagueMatch::SetPlayCountPerOneTeam(unsigned int)
    static void SetPlayCountPerOneTeam(uint32_t player_count_per_team);

    // IDA_EA: 0x855D07E
    // IDA_NAME: _ZN12CLeagueMatch12SetMatchAbleEb
    // DEMANGLED: CLeagueMatch::SetMatchAble(bool)
    // The old body stores its bool argument as one byte, then returns the
    // corresponding zero-extended integer in EAX.
    static int SetMatchAble(bool matchable);

    // IDA_EA: 0x82743A0
    // IDA_NAME: _ZN12CLeagueMatch13SetBattleModeEi
    // DEMANGLED: CLeagueMatch::SetBattleMode(int)
    static void SetBattleMode(int battle_mode);

    // These two old members do not read or write a CLeagueMatch instance.
    // They remain with the unique CLeagueMatch owner alongside the restored
    // physical object; neither is a surrogate-vtable implementation.
    // IDA_EA: 0x855E7A8
    // IDA_NAME: _ZN12CLeagueMatch14_getSameSchoolEPjiij
    // DEMANGLED: CLeagueMatch::_getSameSchool(unsigned int *,int,int,unsigned int)
    int _getSameSchool(unsigned int* school_numbers,
                       int self_index,
                       int player_count,
                       unsigned int school_number);

    // IDA_EA: 0x855E7EE
    // IDA_NAME: _ZN12CLeagueMatch8addBadgeEP10PlayerInfo15ENUM_BADGE_TYPEPhb
    // DEMANGLED: CLeagueMatch::addBadge(PlayerInfo *,ENUM_BADGE_TYPE,unsigned char *,bool)
    int addBadge(PlayerInfo* player,
                 ENUM_BADGE_TYPE badge_type,
                 unsigned char* awarded,
                 bool same_school);

public:
    // IDA layout: IMatch is the vptr-only +0x00 base.  No raw storage or
    // synthetic padding is used for the rest of the object.
    int state_;                  // +0x04
    PVP_ROOM_INFO room_info_;    // +0x08
    int auth_value_;             // +0x18
    TeamInfo teams_[2];          // +0x1c, +0x8c

private:
    // IDA_EA: 0x93ABB74 / 0x93ABB75.  Both old functions read/write one byte;
    // the public getters zero-extend it to int.
    static uint8_t nPlayerCountPerOneTeam_;
    static uint8_t nBattleMode_;

    // IDA_EA: 0x943E060.  This is a one-byte .bss static; the only old
    // consumer accepts exactly value 1 as match-enabled.
    static bool bMatchable_;

    // IDA_EA: 0x943E064
    // IDA_NAME: _ZN12CLeagueMatch17pWaitingRoomList_E
    // The exact BSS pointer is shared by the real CLeague C2/D2 and its
    // waiting-room operations.
    static CLeagueMatchRoomList* pWaitingRoomList_;

    // IDA_EA: 0x943E068
    // IDA_NAME: _ZN12CLeagueMatch25nWaitingRoomListRefCount_E
    static int nWaitingRoomListRefCount_;

    // IDA_EA: 0x943E06C
    // IDA_NAME: _ZN12CLeagueMatch14nextAuthValue_E
    static int nextAuthValue_;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(offsetof(CLeagueMatch, state_) == 0x04u,
              "CLeagueMatch::state_ must stay at +0x04");
static_assert(offsetof(CLeagueMatch, room_info_) == 0x08u,
              "CLeagueMatch::room_info_ must stay at +0x08");
static_assert(offsetof(CLeagueMatch, auth_value_) == 0x18u,
              "CLeagueMatch::auth_value_ must stay at +0x18");
static_assert(offsetof(CLeagueMatch, teams_) == 0x1cu,
              "CLeagueMatch::teams_[0] must stay at +0x1c");
static_assert(offsetof(CLeagueMatch, teams_) + sizeof(TeamInfo) == 0x8cu,
              "CLeagueMatch::teams_[1] must stay at +0x8c");
static_assert(sizeof(CLeagueMatch) == 0xfcu,
              "CLeagueMatch IDA ABI size must stay 0xfc");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_LEAGUE_MATCH_STATIC_STATE_RUNTIME_H_
