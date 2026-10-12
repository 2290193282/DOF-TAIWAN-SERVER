#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PVP_ROOM_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PVP_ROOM_H_

#include <stddef.h>
#include <stdint.h>
#include <vector>

#include "old_data/PVF/pvp_mission/datamanager/pvp_mission_system.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/pvp/match/pvp_matching_system_runtime.h"
#include "old_game/pvp/battlefield/relay_battle_mgr_runtime.h"
#include "old_game/social/party/party.h"
#include "infra/concurrent/thread/mutex.h"
#include "old_game/pvp/battlefield/relay_battle_mgr_runtime.h"

class CUser;
class PacketGuard;
class PvP_Room;
class PvpUserTable;
struct PVP_ROOM_INFO;
struct PvpResultType;
enum ENUM_SEAT_STATE : int;
enum ENUM_WALKOUT_TYPE : int;

// IDA unit: 99_misc_global/cElection<int,8,8>.  The PvP room owns this
// complete 0x100-byte election table at +0x12c.  It is distinct from the
// party's four-seat specialization and cannot be represented as padding.
template <>
class cElection<int, 8, 8> {
public:
    class cVoter {
    public:
        cVoter();
        cVoter* Init();

    private:
        int votes_[8];
        friend class cElection<int, 8, 8>;
    };

    cElection();
    bool Init();
    cElection* Vote(int voter, int candidate);
    int GetElected() const;
    bool CancelCandidate(int candidate);
    cVoter* CancelVoter(int voter);

private:
    cVoter voters_[8];
};

// IDA unit: 99_misc_global/CDeathMatchBattleMgr.  PvP_Room embeds this
// value at +0x69c.  Its constructor is trivial in the old binary; reset and
// parent assignment are the two named state transitions used by room reset.
class CDeathMatchBattleMgr {
public:
    CDeathMatchBattleMgr* Reset();
    CDeathMatchBattleMgr* SetParent(PvP_Room* parent);
    void OnCreateRoom(int seat);
    void OnJoinRoom(int seat);
    void OnLeaveRoom(int seat);
    void OnChangeTeam(int seat, int team);
    int OnReady(int seat, bool ready);
    CDeathMatchBattleMgr* OnStart();
    unsigned int IncreaseKillCount(unsigned int seat);
    unsigned int IncreaseDeathCount(unsigned int seat);
    int InsertTimerKilledUserForRevive(int room_index, int seat);
    void OnFight();
    int getMaxKillCount();
    bool checkGameOverForSinglePlay(int player_count);
    int checkGameOverForTeamPlay(CUser** users,
                                 int team_count,
                                 unsigned char* teams);
    bool checkWinnerForTeamPlay(CUser** users,
                                bool* live,
                                unsigned char* teams,
                                PvpUserTable& table);
    bool checkWinnerForSinglePlay(CUser** users,
                                  bool* live,
                                  PvpUserTable& table);
    // IDA_EA: 0x85DF576
    // IDA_NAME: _ZNK20CDeathMatchBattleMgr10getRankingEPP5CUserPi
    // DEMANGLED: CDeathMatchBattleMgr::getRanking(CUser **,int *)const
    int getRanking(CUser** users, int* ranking) const;
    int getKillCount(int seat);
    int getDeathCount(int seat);

private:
    PvP_Room* parent_;       // +0x00
    int kill_count_[8];      // +0x04
    int death_count_[8];     // +0x24
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(cElection<int, 8, 8>::cVoter) == 0x20u,
              "cElection<int,8,8>::cVoter IDA ABI size must stay 0x20");
static_assert(sizeof(cElection<int, 8, 8>) == 0x100u,
              "cElection<int,8,8> IDA ABI size must stay 0x100");
static_assert(sizeof(CDeathMatchBattleMgr) == 0x44u,
              "CDeathMatchBattleMgr IDA ABI size must stay 0x44");
#endif

// IDA unit: 09_dungeon_pvp_party/PvpUserTable. PvP_Room embeds this complete
// result-calculation table at +0x268 and places PvpResultRecvFlag at +0x5b4.
// The adjacent offsets therefore fix the exact i386 layout at 0x34c bytes.
class PvpUserTable {
public:
    struct UserSlot {
        CUser* user;
        bool won;
    };

    // IDA_EA: 0x85D48F0
    // IDA_NAME: _ZN12PvpUserTableC2Ev
    // DEMANGLED: PvpUserTable::PvpUserTable(void)
    PvpUserTable();

    // IDA_EA: 0x85D4904
    // IDA_NAME: _ZN12PvpUserTableD2Ev
    // DEMANGLED: PvpUserTable::~PvpUserTable()
    ~PvpUserTable();

    // IDA_EA: 0x85D490A
    // IDA_NAME: _ZN12PvpUserTable5ClearEv
    // DEMANGLED: PvpUserTable::Clear(void)
    PvpUserTable* Clear();

    // IDA_EA: 0x85D4980
    // IDA_NAME: _ZN12PvpUserTable3SetEiP5CUser
    // DEMANGLED: PvpUserTable::Set(int,CUser *)
    int Set(int team, CUser* user);

    // IDA_EA: 0x85D4A14
    // IDA_NAME: _ZN12PvpUserTable11IsValidTeamEi
    // DEMANGLED: PvpUserTable::IsValidTeam(int)
    bool IsValidTeam(int team);

    // IDA_EA: 0x85D4A32
    // IDA_NAME: _ZN12PvpUserTable21IsValidConvertedGradeEi
    // DEMANGLED: PvpUserTable::IsValidConvertedGrade(int)
    bool IsValidConvertedGrade(int grade);

    // IDA_EA: 0x85D4A50
    // IDA_NAME: _ZN12PvpUserTable16GetTeamUserCountEi
    // DEMANGLED: PvpUserTable::GetTeamUserCount(int)
    int GetTeamUserCount(int team);

    // IDA_EA: 0x85D4AB8
    // IDA_NAME: _ZN12PvpUserTable12GetExpectValEiiP5CUser
    // DEMANGLED: PvpUserTable::GetExpectVal(int,int,CUser *)
    long double GetExpectVal(int grade_a,
                             int grade_b,
                             CUser* comparison_user);

    // IDA_EA: 0x85D4BD0
    // IDA_NAME: _ZN12PvpUserTable9CalculateEv
    // DEMANGLED: PvpUserTable::Calculate(void)
    int Calculate();

    // IDA_EA: 0x85D4D5E
    // IDA_NAME: _ZN12PvpUserTable18GetTeamVsTeamPointEii
    // DEMANGLED: PvpUserTable::GetTeamVsTeamPoint(int,int)
    int GetTeamVsTeamPoint(int team, int opposing_team);

    // IDA_EA: 0x85D4FCA
    // IDA_NAME: _ZN12PvpUserTable19GetGhostVsTeamPointEii
    // DEMANGLED: PvpUserTable::GetGhostVsTeamPoint(int,int)
    int GetGhostVsTeamPoint(int team, int opposing_team);

    // IDA_EA: 0x85D5064
    // IDA_NAME: _ZN12PvpUserTable9GetResultEibi
    // DEMANGLED: PvpUserTable::GetResult(int,bool,int)
    int GetResult(int team, bool special_result, int pvp_grade);

    // IDA_EA: 0x85D5730
    // IDA_NAME: _ZN12PvpUserTable15GetLeavePenaltyEi
    // DEMANGLED: PvpUserTable::GetLeavePenalty(int)
    int GetLeavePenalty(int team);

    // IDA_EA: 0x85D578A
    // IDA_NAME: _ZN12PvpUserTable11GradeFilterEi
    // DEMANGLED: PvpUserTable::GradeFilter(int)
    int GradeFilter(int grade);

    // IDA_EA: 0x85D57C2
    // IDA_NAME: _ZN12PvpUserTable8CheckDanEv
    // DEMANGLED: PvpUserTable::CheckDan(void)
    bool CheckDan();

    // IDA_EA: 0x85D585E
    // IDA_NAME: _ZN12PvpUserTable10SetWinTeamEi
    // DEMANGLED: PvpUserTable::SetWinTeam(int)
    bool SetWinTeam(int team);

    // IDA_EA: 0x85DF7A4
    // IDA_NAME: _ZN12PvpUserTable10GetWinTeamEv
    // DEMANGLED: PvpUserTable::GetWinTeam(void)
    int GetWinTeam();

    // IDA_EA: 0x85DFDDC
    // IDA_NAME: _ZN12PvpUserTable18GetUserVsTeamPointEiiiP5CUser
    // DEMANGLED: PvpUserTable::GetUserVsTeamPoint(int,int,int,CUser *)
    int GetUserVsTeamPoint(int user_index,
                           int team,
                           int opposing_team,
                           CUser* comparison_user);

    float team_vs_team_point[8][8];  // IDA +0x000
    float expected_value_sum[8];     // IDA +0x100
    UserSlot users[8][8];            // IDA +0x120
    int win_team;                    // IDA +0x320
    float leave_penalty[8];          // IDA +0x324
    bool has_dan[8];                 // IDA +0x344
};

static_assert(sizeof(PvpUserTable::UserSlot) == 0x08u,
              "PvpUserTable UserSlot IDA ABI size must stay 0x08");
static_assert(offsetof(PvpUserTable, users) == 0x120u,
              "PvpUserTable users must stay at +0x120");
static_assert(offsetof(PvpUserTable, win_team) == 0x320u,
              "PvpUserTable win team must stay at +0x320");
static_assert(sizeof(PvpUserTable) == 0x34cu,
              "PvpUserTable IDA ABI size must stay 0x34c");

// IDA unit: 09_dungeon_pvp_party/PvP_Room. The pooled storage is owned by the
// game-manager runtime; C owns these gameplay transition declarations. Field
// accesses remain audited in the one-to-one function bodies.
class PvP_Room {
public:
    // IDA_EA: 0x85D6012
    // IDA_NAME: _ZN8PvP_RoomC2Ev
    // DEMANGLED: PvP_Room::PvP_Room(void)
    PvP_Room();

    // IDA_EA: 0x85D61A6
    // IDA_NAME: _ZN8PvP_RoomD2Ev
    // DEMANGLED: PvP_Room::~PvP_Room()
    ~PvP_Room();

    // IDA_EA: 0x822D974
    // IDA_NAME: _ZN8PvP_Room15IsExistPasswordEv
    // DEMANGLED: PvP_Room::IsExistPassword(void)
    int IsExistPassword();

    // IDA_EA: 0x822D984
    // IDA_NAME: _ZNK8PvP_Room13isRematchableEv
    // DEMANGLED: PvP_Room::isRematchable(void)const
    int isRematchable() const;

    // IDA_EA: 0x822D994
    // IDA_NAME: _ZN8PvP_Room9get_indexEv
    // DEMANGLED: PvP_Room::get_index(void)
    int get_index();

    // IDA_EA: 0x82A4322
    // IDA_NAME: _ZN8PvP_Room24IsInsertTimerRecvPvpRankEv
    // DEMANGLED: PvP_Room::IsInsertTimerRecvPvpRank(void)
    int IsInsertTimerRecvPvpRank();

    // IDA_EA: 0x82A4332
    // IDA_NAME: _ZN8PvP_Room14IsEndPvpBattleEv
    // DEMANGLED: PvP_Room::IsEndPvpBattle(void)
    int IsEndPvpBattle();

    // IDA_EA: 0x8568D0A
    // IDA_NAME: _ZNK8PvP_Room10GetManagerEv
    // DEMANGLED: PvP_Room::GetManager(void)const
    CUser* GetManager() const;

    // IDA_EA: 0x8568D16
    // IDA_NAME: _ZNK8PvP_Room14GetManagerSeatEv
    // DEMANGLED: PvP_Room::GetManagerSeat(void)const
    int GetManagerSeat() const;

    // IDA_EA: 0x8568D22
    // IDA_NAME: _ZN8PvP_Room12GetTeamCountEi
    // DEMANGLED: PvP_Room::GetTeamCount(int)
    int GetTeamCount(int team);

    // IDA_EA: 0x8568D62
    // IDA_NAME: _ZN8PvP_Room14setReamtchableEb
    // DEMANGLED: PvP_Room::setReamtchable(bool)
    PvP_Room* setReamtchable(bool rematchable);

    // IDA_EA: 0x8568D7E
    // IDA_NAME: _ZN8PvP_Room9set_stateEi
    // DEMANGLED: PvP_Room::set_state(int)
    PvP_Room* set_state(int state);

    // IDA_EA: 0x8568D90
    // IDA_NAME: _ZN8PvP_Room13gen_timer_keyE13TIMER_MESSAGE
    // DEMANGLED: PvP_Room::gen_timer_key(TIMER_MESSAGE)
    int gen_timer_key(TIMER_MESSAGE message);

    // IDA_EA: 0x85D62AC
    // IDA_NAME: _ZN8PvP_Room6SetIDXEi
    // DEMANGLED: PvP_Room::SetIDX(int)
    int SetIDX(int index);

    // Declaration-only external provider.  The original body is present in
    // the old df_game_r IDB at 0x85D6430 but has not yet been restored into
    // the canonical runtime source tree.
    // IDA_EA: 0x85D6430
    // IDA_NAME: _ZN8PvP_Room12battle_resetEv
    // DEMANGLED: PvP_Room::battle_reset(void)
    int battle_reset();

    // IDA_EA: 0x85D6774
    // IDA_NAME: _ZN8PvP_Room16get_pvp_team_cntEv
    // DEMANGLED: PvP_Room::get_pvp_team_cnt(void)
    int get_pvp_team_cnt();

    // IDA_EA: 0x85D6844
    // IDA_NAME: _ZNK8PvP_Room15get_remain_seatEv
    // DEMANGLED: PvP_Room::get_remain_seat(void)const
    int get_remain_seat() const;

    // IDA_EA: 0x85D68A0
    // IDA_NAME: _ZN8PvP_Room16get_player_countEv
    // DEMANGLED: PvP_Room::get_player_count(void)
    int get_player_count();

    // Declaration-only external provider.
    // IDA_EA: 0x85D6908
    // IDA_NAME: _ZN8PvP_Room11create_roomEP5CUsercPcsbPKci15PVP_BATTLE_MODE
    // DEMANGLED: PvP_Room::create_room(CUser *,char,char *,short,bool,char const*,int,PVP_BATTLE_MODE)
    int create_room(CUser* user,
                    char room_title_mode,
                    char* room_title,
                    int16_t requested_map_no,
                    bool password_enabled,
                    const char* password,
                    int password_length,
                    PVP_BATTLE_MODE battle_mode);

    // Restored typed provider in pvp_room_create_runtime.cpp.
    // IDA_EA: 0x85D6CF2
    // IDA_NAME: _ZN8PvP_Room9join_roomEP5CUserRib
    // DEMANGLED: PvP_Room::join_room(CUser *,int &,bool)
    int join_room(CUser* user, int& seat, bool move_room);

    // Phase-2 Zombie owner bridge.  This is a non-virtual member solely so
    // the packet-11 writer can read CUser's private network endpoint through
    // PvP_Room's existing friendship.  Adding a member function changes
    // neither the object layout nor the original vtable/ABI.
    bool send_zombie_owner_full_peer_snapshot(CUser* recipient,
                                              CUser* const* members,
                                              size_t member_count);

    // Project-owned load-barrier detach.  Unlike leave_room this never calls
    // penalty, winner, relay or ordinary IMatch callbacks; it only removes
    // the exact not-loaded physical seat before Zombie BeginRoom exists.
    bool detach_zombie_loading_seat(CUser* user,
                                    int expected_seat,
                                    bool& manager_changed);

    // IDA_EA: 0x85D7726
    // IDA_NAME: _ZN8PvP_Room23is_closeable_seat_stateEv
    // DEMANGLED: PvP_Room::is_closeable_seat_state(void)
    int is_closeable_seat_state();

    // IDA_EA: 0x85D77C0
    // IDA_NAME: _ZN8PvP_Room14set_seat_stateEP5CUseri15ENUM_SEAT_STATEPS1_RbS4_S4_
    // DEMANGLED: PvP_Room::set_seat_state(CUser *,int,ENUM_SEAT_STATE,CUser **,bool &,bool &,bool &)
    int set_seat_state(CUser* user,
                       int seat,
                       ENUM_SEAT_STATE state,
                       CUser** leaving_user,
                       bool& make_seat_info,
                       bool& make_state_info,
                       bool& room_reset);

    // IDA_EA: 0x85DA046
    // IDA_NAME: _ZN8PvP_Room13IsPvpObserverEi
    // DEMANGLED: PvP_Room::IsPvpObserver(int)
    int IsPvpObserver(int seat);

    // IDA_EA: 0x85D6862
    // IDA_NAME: _ZNK8PvP_Room16get_waiter_countEv
    // DEMANGLED: PvP_Room::get_waiter_count(void)const
    int get_waiter_count() const;

    // IDA_EA: 0x822D99E
    // IDA_NAME: _ZN8PvP_Room17SetRequestedMapNoEs
    // DEMANGLED: PvP_Room::SetRequestedMapNo(short)
    PvP_Room* SetRequestedMapNo(int16_t map_no);

    // IDA_EA: 0x822D9BC
    // IDA_NAME: _ZN8PvP_Room17GetRequestedMapNoEv
    // DEMANGLED: PvP_Room::GetRequestedMapNo(void)
    int GetRequestedMapNo();

    // IDA_EA: 0x822D9CC
    // IDA_NAME: _ZN8PvP_Room19get_pvp_battle_modeEv
    // DEMANGLED: PvP_Room::get_pvp_battle_mode(void)
    int get_pvp_battle_mode();

    // IDA_EA: 0x85DC2CC
    // IDA_NAME: _ZN8PvP_Room23get_recv_pvp_rank_countEv
    // DEMANGLED: PvP_Room::get_recv_pvp_rank_count(void)
    int get_recv_pvp_rank_count();

    // IDA_EA: 0x85D84C6
    // IDA_NAME: _ZN8PvP_Room12set_pvp_modeEP5CUseri
    // DEMANGLED: PvP_Room::set_pvp_mode(CUser *,int)
    int set_pvp_mode(CUser* user, int mode);

    // Declaration-only external provider.
    // IDA_EA: 0x85D8290
    // IDA_NAME: _ZN8PvP_Room15set_ready_stateEP5CUserb
    // DEMANGLED: PvP_Room::set_ready_state(CUser *,bool)
    int set_ready_state(CUser* user, bool ready);

    // IDA_EA: 0x85DD936
    // IDA_NAME: _ZN8PvP_Room15CheckPvPMapModeEP5CUseriRb
    // DEMANGLED: PvP_Room::CheckPvPMapMode(CUser *,int,bool &)
    int CheckPvPMapMode(CUser* user,
                        int map_no,
                        bool& changed_to_individual);

    // IDA_EA: 0x85DB9E2
    // IDA_NAME: _ZN8PvP_Room14make_seat_infoEPci
    // DEMANGLED: PvP_Room::make_seat_info(char *,int)
    int make_seat_info(char* packet, int seat_filter);

    // IDA_EA: 0x85DBD3A
    // IDA_NAME: _ZN8PvP_Room15make_state_infoEPc
    // DEMANGLED: PvP_Room::make_state_info(char *)
    int make_state_info(char* packet);

    // IDA_EA: 0x85D7F6C
    // IDA_NAME: _ZN8PvP_Room10leave_roomEP5CUserRb
    // DEMANGLED: PvP_Room::leave_room(CUser *,bool &)
    int leave_room(CUser* user, bool& manager_changed);

    // IDA_EA: 0x85D81D0
    // IDA_NAME: _ZN8PvP_Room12destroy_roomEi
    // DEMANGLED: PvP_Room::destroy_room(int)
    int destroy_room(int preferred_seat);

    // IDA_EA: 0x85DC476
    // IDA_NAME: _ZN8PvP_Room12send_pvp_endEv
    // DEMANGLED: PvP_Room::send_pvp_end(void)
    int send_pvp_end();

    // IDA_EA: 0x85D91E2
    // IDA_NAME: _ZN8PvP_Room13get_user_seatEP5CUser
    // DEMANGLED: PvP_Room::get_user_seat(CUser *)
    int get_user_seat(CUser* user);

    // IDA_EA: 0x85D921E
    // IDA_NAME: _ZN8PvP_Room13get_user_seatEi
    // DEMANGLED: PvP_Room::get_user_seat(int)
    CUser* get_user_seat(int seat);

    // IDA_EA: 0x85D9386
    // IDA_NAME: _ZN8PvP_Room10die_characEP5CUserS1_
    // DEMANGLED: PvP_Room::die_charac(CUser *,CUser *)
    int die_charac(CUser* dead_user, CUser* killer_user);

    // IDA_EA: 0x85D9242
    // IDA_NAME: _ZN8PvP_Room16get_manager_teamEv
    // DEMANGLED: PvP_Room::get_manager_team(void)
    int get_manager_team();

    // IDA_EA: 0x85D9272
    // IDA_NAME: _ZN8PvP_Room8get_teamEi
    // DEMANGLED: PvP_Room::get_team(int)
    int get_team(int seat);

    // IDA_EA: 0x85DF946
    // IDA_NAME: _ZN8PvP_Room9get_stateEv
    // DEMANGLED: PvP_Room::get_state(void)
    int get_state();

    // IDA_EA: 0x863BCB4
    // IDA_NAME: _ZN8PvP_Room13get_timer_keyE13TIMER_MESSAGE
    // DEMANGLED: PvP_Room::get_timer_key(TIMER_MESSAGE)
    int get_timer_key(TIMER_MESSAGE message);

    // IDA_EA: 0x863BCCC
    // IDA_NAME: _ZN8PvP_Room13CheckTimerKeyE13TIMER_MESSAGEi
    // DEMANGLED: PvP_Room::CheckTimerKey(TIMER_MESSAGE,int)
    bool CheckTimerKey(TIMER_MESSAGE message, int key);

    // IDA_EA: 0x85D65B4
    // IDA_NAME: _ZN8PvP_Room15check_authorityEP5CUseri
    // DEMANGLED: PvP_Room::check_authority(CUser *,int)
    bool check_authority(CUser* user, int seat);

    // IDA_EA: 0x85D65E8
    // IDA_NAME: _ZN8PvP_Room17check_start_stateEv
    // DEMANGLED: PvP_Room::check_start_state(void)
    int check_start_state();

    // IDA_EA: 0x85D6680
    // IDA_NAME: _ZN8PvP_Room14check_team_pvpEv
    // DEMANGLED: PvP_Room::check_team_pvp(void)
    bool check_team_pvp();

    // IDA_EA: 0x85D7D9C
    // IDA_NAME: _ZN8PvP_Room11is_last_winEi
    // DEMANGLED: PvP_Room::is_last_win(int)
    bool is_last_win(int seat);

    // IDA_EA: 0x85D7DF0
    // IDA_NAME: _ZN8PvP_Room24change_manager_to_winnerEv
    // DEMANGLED: PvP_Room::change_manager_to_winner(void)
    int change_manager_to_winner();

    // IDA_EA: 0x85D7EF4
    // IDA_NAME: _ZN8PvP_Room18select_new_managerEv
    // DEMANGLED: PvP_Room::select_new_manager(void)
    int select_new_manager();

    // IDA_EA: 0x85D9296
    // IDA_NAME: _ZN8PvP_Room13get_room_infoEP13PVP_ROOM_INFO
    // DEMANGLED: PvP_Room::get_room_info(PVP_ROOM_INFO *)
    void get_room_info(PVP_ROOM_INFO* room_info);

    // IDA_EA: 0x85D92FC
    // IDA_NAME: _ZN8PvP_Room7end_pvpEv
    // DEMANGLED: PvP_Room::end_pvp(void)
    int end_pvp();

    // IDA_EA: 0x85D932A
    // IDA_NAME: _ZN8PvP_Room13get_user_listEPv
    // DEMANGLED: PvP_Room::get_user_list(void *)
    int get_user_list(void* user_list);

    // IDA_EA: 0x85D9358
    // IDA_NAME: _ZN8PvP_Room19get_extra_win_pointEi
    // DEMANGLED: PvP_Room::get_extra_win_point(int)
    int get_extra_win_point(int win_point);

    // IDA_EA: 0x85DA1D8
    // IDA_NAME: _ZN8PvP_Room31CalculateVictoryPointOfTeamPlayEibi
    // DEMANGLED: PvP_Room::CalculateVictoryPointOfTeamPlay(int,bool,int)
    int CalculateVictoryPointOfTeamPlay(int rank, bool won, int point);

    // IDA_EA: 0x85DA210
    // IDA_NAME: _ZN8PvP_Room20CalcuateVictoryPointEPii
    // DEMANGLED: PvP_Room::CalcuateVictoryPoint(int *,int)
    // The spelling is the original exported ABI.
    void CalcuateVictoryPoint(int* ranking, int ranking_count);

    // IDA_EA: 0x85DA58A
    // IDA_NAME: _ZN8PvP_Room18IsAllMoreThanGradeEi
    // DEMANGLED: PvP_Room::IsAllMoreThanGrade(int)
    int IsAllMoreThanGrade(int grade);

    // IDA_EA: 0x85DA5F4
    // IDA_NAME: _ZN8PvP_Room22IsThereSameGuildMemberEv
    // DEMANGLED: PvP_Room::IsThereSameGuildMember(void)
    int IsThereSameGuildMember();

    // IDA_EA: 0x85DA7CC
    // IDA_NAME: _ZN8PvP_Room25check_acquire_guild_pointEv
    // DEMANGLED: PvP_Room::check_acquire_guild_point(void)
    bool check_acquire_guild_point();

    // IDA_EA: 0x85DA840
    // IDA_NAME: _ZN8PvP_Room14count_pvp_userEv
    // DEMANGLED: PvP_Room::count_pvp_user(void)
    int count_pvp_user();

    // IDA_EA: 0x85DBF82
    // IDA_NAME: _ZN8PvP_Room22set_recv_pvp_rank_flagEP5CUser
    // DEMANGLED: PvP_Room::set_recv_pvp_rank_flag(CUser *)
    int set_recv_pvp_rank_flag(CUser* user);

    // IDA_EA: 0x85DBFEC
    // IDA_NAME: _ZN8PvP_Room28set_recv_pvp_end_result_flagEP5CUser
    // DEMANGLED: PvP_Room::set_recv_pvp_end_result_flag(CUser *)
    int set_recv_pvp_end_result_flag(CUser* user);

    // IDA_EA: 0x85DC034
    // IDA_NAME: _ZN8PvP_Room12SetPVPResultEiRK13PvpResultType
    // DEMANGLED: PvP_Room::SetPVPResult(int,PvpResultType const&)
    void SetPVPResult(int seat, const PvpResultType& result);

    // IDA_EA: 0x85DC07E
    // IDA_NAME: _ZNK8PvP_Room23recv_pvp_end_result_allEv
    // DEMANGLED: PvP_Room::recv_pvp_end_result_all(void)const
    int recv_pvp_end_result_all() const;

    // IDA_EA: 0x85DC0F0
    // IDA_NAME: _ZNK8PvP_Room24recv_pvp_end_result_teamEP5CUser
    // DEMANGLED: PvP_Room::recv_pvp_end_result_team(CUser *)const
    int recv_pvp_end_result_team(CUser* user) const;

    // IDA_EA: 0x85DC1C8
    // IDA_NAME: _ZNK8PvP_Room29get_recv_pvp_end_result_countEv
    // DEMANGLED: PvP_Room::get_recv_pvp_end_result_count(void)const
    int get_recv_pvp_end_result_count() const;

    // IDA_EA: 0x85DC4BA
    // IDA_NAME: _ZNK8PvP_Room17recv_pvp_rank_allEv
    // DEMANGLED: PvP_Room::recv_pvp_rank_all(void)const
    int recv_pvp_rank_all() const;

    // IDA_EA: 0x85DC64C
    // IDA_NAME: _ZNK8PvP_Room6GetAceEv
    // DEMANGLED: PvP_Room::GetAce(void)const
    int GetAce() const;

    // IDA_EA: 0x85DC52C
    // IDA_NAME: _ZNK8PvP_Room10GetRankingEPi
    // DEMANGLED: PvP_Room::GetRanking(int *)const
    int GetRanking(int* ranking) const;

    // IDA_EA: 0x85DC6EE
    // IDA_NAME: _ZN8PvP_Room12GetTeamIndexEP5CUser
    // DEMANGLED: PvP_Room::GetTeamIndex(CUser *)
    int GetTeamIndex(CUser* user);

    // IDA_EA: 0x85DC778
    // IDA_NAME: _ZN8PvP_Room19calculate_pvp_pointEv
    // DEMANGLED: PvP_Room::calculate_pvp_point(void)
    int calculate_pvp_point();

    // IDA_EA: 0x85DCA48
    // IDA_NAME: _ZN8PvP_Room14end_pvp_resultEv
    // DEMANGLED: PvP_Room::end_pvp_result(void)
    int end_pvp_result();

    // IDA_EA: 0x85DCABA
    // IDA_NAME: _ZN8PvP_Room23SkipLastRankResultTimerEv
    // DEMANGLED: PvP_Room::SkipLastRankResultTimer(void)
    int SkipLastRankResultTimer();

    // IDA_EA: 0x85DCAD6
    // IDA_NAME: _ZN8PvP_Room14CancelAllTimerEv
    // DEMANGLED: PvP_Room::CancelAllTimer(void)
    int CancelAllTimer();

    // IDA_EA: 0x85DDEAE
    // IDA_NAME: _ZN8PvP_Room13CheckPasswordEPKci
    // DEMANGLED: PvP_Room::CheckPassword(char const*,int)
    bool CheckPassword(const char* password, int length);

    // IDA_EA: 0x85DDEF6
    // IDA_NAME: _ZN8PvP_Room22OnConnectP2PPvPTimeoutEv
    // DEMANGLED: PvP_Room::OnConnectP2PPvPTimeout(void)
    int OnConnectP2PPvPTimeout();

    // IDA_EA: 0x85DDFBE
    // IDA_NAME: _ZN8PvP_Room13request_fightEP5CUser
    // DEMANGLED: PvP_Room::request_fight(CUser *)
    int request_fight(CUser* user);

    // Batch 028: PvP room join, map selection, result timeout and cleanup.
    bool IsQuickJoinablePVPRoom(CUser* user);
    int check_joinable(CUser* user);
    void change_team(int seat, int team);
    bool GetCandidateMapIndexList(std::vector<int>& map_indexes);
    int DrawRandomMapIndex(std::vector<int>& map_indexes);
    PvP_Room* ChangePvPMode(int map_no);
    int SelectRandomMap();

    // Declaration-only external provider.
    // IDA_EA: 0x85D8C3A
    // IDA_NAME: _ZN8PvP_Room9start_pvpEv
    // DEMANGLED: PvP_Room::start_pvp(void)
    void start_pvp();

    // Declaration-only external provider.
    // IDA_EA: 0x85D9B0E
    // IDA_NAME: _ZN8PvP_Room8time_outEPi
    // DEMANGLED: PvP_Room::time_out(int *)
    int time_out(int* scores);
    int CalculateVictoryPointOfSinglePlay(int rank, bool won, int point);
    int make_room_info(char* packet);
    int make_ready_info(char* packet, int seat);
    bool send_to_pvp(PacketGuard& packet);
    // IDA_EA: 0x85DBE64
    // IDA_NAME: _ZN8PvP_Room16send_to_pvp_teamER11PacketGuardP5CUser
    // DEMANGLED: PvP_Room::send_to_pvp_team(PacketGuard &,CUser *)
    bool send_to_pvp_team(PacketGuard& packet, CUser* sender);
    int insert_timer_recv_pvp_end_result();
    int insert_timer_recv_pvp_rank();
    int check_recv_pvp_rank();
    int SendWalkOut(int seat, ENUM_WALKOUT_TYPE type);

    // IDA_EA: 0x85DCF1A
    // IDA_NAME: _ZN8PvP_Room12VoteLackUserEP5CUseri
    // DEMANGLED: PvP_Room::VoteLackUser(CUser *,int)
    int VoteLackUser(CUser* voter, int target_seat);

    // IDA_EA: 0x85DCE28
    // IDA_NAME: _ZN8PvP_Room22OnTimerWalkOutLackUserEv
    // DEMANGLED: PvP_Room::OnTimerWalkOutLackUser(void)
    int OnTimerWalkOutLackUser();

    // Declaration-only external provider.
    // IDA_EA: 0x85DD070
    // IDA_NAME: _ZN8PvP_Room17OnSetPVPSeatStateEP5CUseri15ENUM_SEAT_STATE17ENUM_WALKOUT_TYPE
    // DEMANGLED: PvP_Room::OnSetPVPSeatState(CUser *,int,ENUM_SEAT_STATE,ENUM_WALKOUT_TYPE)
    int OnSetPVPSeatState(CUser* user,
                          int seat,
                          ENUM_SEAT_STATE state,
                          ENUM_WALKOUT_TYPE walkout_type);
    // IDA_EA: 0x85DD4A4
    // IDA_NAME: _ZN8PvP_Room7WalkOutEP5CUseri15ENUM_SEAT_STATE17ENUM_WALKOUT_TYPE
    // DEMANGLED: PvP_Room::WalkOut(CUser *,int,ENUM_SEAT_STATE,ENUM_WALKOUT_TYPE)
    int WalkOut(CUser* user,
                int seat,
                ENUM_SEAT_STATE state,
                ENUM_WALKOUT_TYPE walkout_type);
    // IDA_EA: 0x85DD754
    // IDA_NAME: _ZN8PvP_Room10DisconnectEP5CUser
    // DEMANGLED: PvP_Room::Disconnect(CUser *)
    int Disconnect(CUser* user);
    void ClearPvPRoom(CUser* user);
    void HandleTimerUserRevive(int seat);

    // Declaration-only external provider.
    // IDA_EA: 0x85DDF36
    // IDA_NAME: _ZN8PvP_Room14revengeMissionEb
    // DEMANGLED: PvP_Room::revengeMission(bool)
    bool revengeMission(bool revenge);

    // Existing direct PvP-room ABI boundaries used by the three transitions.
    // IDA_EA: 0x85DCAF2
    // IDA_NAME: _ZN8PvP_Room16GiveOuterPenaltyEP5CUser
    // DEMANGLED: PvP_Room::GiveOuterPenalty(CUser *)
    void GiveOuterPenalty(CUser* user);
    int check_winner();
    void pvp_request_pvp_rank();
    int reset();
    bool update_pvp_point();
    // IDA_EA: 0x85DA882
    // IDA_NAME: _ZN8PvP_Room10pvp_rewardEv
    // DEMANGLED: PvP_Room::pvp_reward(void)
    void pvp_reward();
    // IDA_EA: 0x85DF93A
    // IDA_NAME: _ZN8PvP_Room4lockEv
    // DEMANGLED: PvP_Room::lock(void)
    void lock();

    // IDA_EA: 0x85DF940
    // IDA_NAME: _ZN8PvP_Room6unlockEv
    // DEMANGLED: PvP_Room::unlock(void)
    void unlock();

public:
    // Public solely for cross-unit ABI auditing; runtime code continues to
    // use the named member functions above.
    // The scalar prefix is intentionally named where the IDA bodies prove a
    // field.  Neutral byte intervals remain only for fields whose identities
    // are not recovered; all lifetime-bearing subobjects are explicit.
    int index_;                                      // +0x000
    PVP_BATTLE_MODE battle_mode_;                    // +0x004
    uint8_t room_title_mode_;                        // +0x008
    char room_title_[30];                            // +0x009
    uint8_t pad_027_;                                // +0x027
    CUser* manager_;                                 // +0x028
    int manager_seat_;                               // +0x02c
    CUser* users_[8];                                // +0x030
    int teams_[8];                                   // +0x050
    uint8_t ready_flags_[8];                         // +0x070
    int battle_scores_[8];                           // +0x078
    int battle_kill_counts_[8];                      // +0x098
    int battle_state_;                               // +0x0b8
    int battle_player_count_;                        // +0x0bc
    uint8_t state_flag_0c0_;                         // +0x0c0
    uint8_t raw_0c1_0c3_[3];                         // +0x0c1
    PvP_GuildWar_Log guild_war_log_;                 // +0x0c4
    cElection<int, 8, 8> election_;                  // +0x12c
    int pvp_timer_keys_[15];                         // +0x22c
    PvpUserTable user_table_;                        // +0x268
    PvpResultRecvFlag recv_pvp_rank_;                // +0x5b4
    PvpResultRecvFlag recv_pvp_end_result_;          // +0x5bc
    int16_t requested_map_no_;                       // +0x5c4
    int16_t selected_map_no_;                        // +0x5c6
    uint8_t battle_live_flags_[8];                   // +0x5c8
    uint8_t battle_teams_[8];                        // +0x5d0
    uint32_t battle_runtime_seed_;                   // +0x5d8
    uint8_t battle_runtime_flags_[12];               // +0x5dc
    Mutex mutex_;                                    // +0x5e8
    int member_count_;                               // +0x600
    uint8_t password_exists_;                        // +0x604
    char password_[9];                               // +0x605
    uint8_t pad_60e_60f_[2];                         // +0x60e
    int password_length_;                            // +0x610
    uint8_t timed_out_;                              // +0x614
    uint8_t pvp_rank_requested_;                     // +0x615
    uint8_t recv_pvp_rank_timer_inserted_;           // +0x616
    uint8_t pvp_battle_ended_;                       // +0x617
    int battle_start_time_sec_;                       // +0x618
    uint8_t rematchable_;                            // +0x61c
    uint8_t raw_61d_61f_[3];                         // +0x61d
    CRelayBattleMgr relay_battle_mgr_;               // +0x620
    CDeathMatchBattleMgr death_match_battle_mgr_;    // +0x69c
    CMatchingSystem* matching_system_;               // +0x6e0
    IMatch* current_match_;                          // +0x6e4
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(offsetof(PvP_Room, index_) == 0x000u,
              "PvP_Room index must stay at +0x000");
static_assert(offsetof(PvP_Room, battle_mode_) == 0x004u,
              "PvP_Room battle mode must stay at +0x004");
static_assert(offsetof(PvP_Room, room_title_mode_) == 0x008u,
              "PvP_Room title mode must stay at +0x008");
static_assert(offsetof(PvP_Room, room_title_) == 0x009u,
              "PvP_Room title must stay at +0x009");
static_assert(offsetof(PvP_Room, manager_) == 0x028u,
              "PvP_Room manager must stay at +0x028");
static_assert(offsetof(PvP_Room, manager_seat_) == 0x02cu,
              "PvP_Room manager seat must stay at +0x02c");
static_assert(offsetof(PvP_Room, users_) == 0x030u,
              "PvP_Room users must stay at +0x030");
static_assert(offsetof(PvP_Room, teams_) == 0x050u,
              "PvP_Room teams must stay at +0x050");
static_assert(offsetof(PvP_Room, ready_flags_) == 0x070u,
              "PvP_Room ready flags must stay at +0x070");
static_assert(offsetof(PvP_Room, guild_war_log_) == 0x0c4u,
              "PvP_Room guild log must stay at +0x0c4");
static_assert(offsetof(PvP_Room, battle_state_) == 0x0b8u,
              "PvP_Room battle state must stay at +0x0b8");
static_assert(offsetof(PvP_Room, battle_player_count_) == 0x0bcu,
              "PvP_Room battle player count must stay at +0x0bc");
static_assert(offsetof(PvP_Room, battle_scores_) == 0x078u,
              "PvP_Room battle scores must stay at +0x078");
static_assert(offsetof(PvP_Room, battle_kill_counts_) == 0x098u,
              "PvP_Room battle kill counts must stay at +0x098");
static_assert(offsetof(PvP_Room, requested_map_no_) == 0x5c4u,
              "PvP_Room requested map must stay at +0x5c4");
static_assert(offsetof(PvP_Room, battle_live_flags_) == 0x5c8u,
              "PvP_Room battle flags must stay at +0x5c8");
static_assert(offsetof(PvP_Room, election_) == 0x12cu,
              "PvP_Room election must stay at +0x12c");
static_assert(offsetof(PvP_Room, user_table_) == 0x268u,
              "PvP_Room user table must stay at +0x268");
static_assert(offsetof(PvP_Room, recv_pvp_rank_) == 0x5b4u,
              "PvP_Room rank receive flags must stay at +0x5b4");
static_assert(offsetof(PvP_Room, recv_pvp_end_result_) == 0x5bcu,
              "PvP_Room end-result flags must stay at +0x5bc");
static_assert(offsetof(PvP_Room, mutex_) == 0x5e8u,
              "PvP_Room mutex must stay at +0x5e8");
static_assert(offsetof(PvP_Room, member_count_) == 0x600u,
              "PvP_Room member count must stay at +0x600");
static_assert(offsetof(PvP_Room, password_exists_) == 0x604u,
              "PvP_Room password flag must stay at +0x604");
static_assert(offsetof(PvP_Room, password_) == 0x605u,
              "PvP_Room password must stay at +0x605");
static_assert(offsetof(PvP_Room, password_length_) == 0x610u,
              "PvP_Room password length must stay at +0x610");
static_assert(offsetof(PvP_Room, timed_out_) == 0x614u,
              "PvP_Room timeout flag must stay at +0x614");
static_assert(offsetof(PvP_Room, pvp_rank_requested_) == 0x615u,
              "PvP_Room rank-request flag must stay at +0x615");
static_assert(offsetof(PvP_Room, recv_pvp_rank_timer_inserted_) == 0x616u,
              "PvP_Room rank timer flag must stay at +0x616");
static_assert(offsetof(PvP_Room, pvp_battle_ended_) == 0x617u,
              "PvP_Room battle-end flag must stay at +0x617");
static_assert(offsetof(PvP_Room, battle_start_time_sec_) == 0x618u,
              "PvP_Room battle start time must stay at +0x618");
static_assert(offsetof(PvP_Room, rematchable_) == 0x61cu,
              "PvP_Room rematch flag must stay at +0x61c");
static_assert(offsetof(PvP_Room, relay_battle_mgr_) == 0x620u,
              "PvP_Room relay manager must stay at +0x620");
static_assert(offsetof(PvP_Room, death_match_battle_mgr_) == 0x69cu,
              "PvP_Room death-match manager must stay at +0x69c");
static_assert(offsetof(PvP_Room, matching_system_) == 0x6e0u,
              "PvP_Room matching system must stay at +0x6e0");
static_assert(offsetof(PvP_Room, current_match_) == 0x6e4u,
              "PvP_Room current match must stay at +0x6e4");
static_assert(sizeof(PvP_Room) == 0x6e8u,
              "PvP_Room IDA ABI size must stay 0x6e8");
#endif


#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PVP_ROOM_H_
