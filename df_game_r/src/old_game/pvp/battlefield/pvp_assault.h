#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PVP_ASSAULT_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PVP_ASSAULT_H_

#include <stddef.h>
#include <stdint.h>
#include <map>
#include <vector>

#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"

class CUser;
class PacketGuard;
template <typename Type, int Count>
class StaticPool;
struct MapArea;
// IDA stack payload passed by CMD150 after its leading killer-index byte.
// CAssaultPlace::DieUser reads the two dwords followed by four shorts in
// this exact order; it is not a network-header object.
struct MSG_DIE_ASSAULT_PLAYER {
    uint32_t damage_value;
    uint32_t damage_count;
    int16_t attacker_stat0;
    int16_t attacker_stat1;
    int16_t attacker_stat2;
    int16_t attacker_stat3;
};

static_assert(sizeof(MSG_DIE_ASSAULT_PLAYER) == 0x10u,
              "CMD150 death payload must stay 16 bytes");
enum ENUM_ASSAULT_TEAM : int;
enum ENUM_POWER_SIDE_TYPE : int;
enum ENUM_POWER_WAR_WINNER_OR_LOSER : int;

namespace pvp_assault {

enum ENUM_ASSAULT_STATE : int;
class CAssaultPlaceFactory;

// The original placement tables are translation-unit-local static data for
// CStaticDisposition.  Keep their concrete coordinates in that C provider;
// they are not public pvp_assault globals.

class CForcedMoveCharacter {
public:
    void ForcedMove();

    int team;
    int point_index;
    CUser* user;
    int village;
    int area;
    int pos_x;
    int pos_y;
    int direction;
};

class CCandidateMatrix : public std::vector<CandidatePoint> {
public:
    CCandidateMatrix();
    ~CCandidateMatrix();

    void _AddCandidatePoint(const Point2d& candidate,
                            const Point2d& origin,
                            const Point2d& opponent,
                            int base_score);
    int At(unsigned int index, int& pos_x, int& pos_y);
    bool MakeCandidatePoint(const Point2d& origin,
                            const Point2d& opponent);
    void RemoveCandidatePoint(std::vector<MapArea>* map_areas);
    void ArrangeCandidatePoint();
};

class CExtraMatrix : public std::vector<Point2d> {
public:
    CExtraMatrix();
    ~CExtraMatrix();

    void AddPoint(Point2d point);
    void MovePoint(const Point2d& point);
    bool CheckPoint(std::vector<MapArea>* map_areas, Point2d& moved_point);
    bool MoveUser(CForcedMoveCharacter* character);
};

// IDA unit: pvp_assault::CDisposition @0x82F1052. GameWorld owns the
// area-vector handoff; concrete PvP dispositions own the placement policy.
class CDisposition {
public:
    CDisposition();

    virtual int SetVirtualMap(std::vector<MapArea>* map_areas) = 0;
    virtual int MoveUser(CForcedMoveCharacter* character) = 0;
    virtual void GetFlagPoint(int& pos_x, int& pos_y) = 0;
};

class CDynamicDisposition : public CDisposition {
public:
    CDynamicDisposition(CUser** users, int* team_counts);

    virtual int SetVirtualMap(std::vector<MapArea>* map_areas);
    virtual int MoveUser(CForcedMoveCharacter* character);
    virtual void GetFlagPoint(int& pos_x, int& pos_y);

    CUser* users[2];
    Point2d flag_point;
    CCandidateMatrix candidate_matrices[2];
};

class CStaticDisposition : public CDisposition {
public:
    CStaticDisposition(CUser** users, int* team_counts);
    ~CStaticDisposition();

    void _MoveMatrix(const Point2d& delta);
    bool _CheckMatrix(std::vector<MapArea>* map_areas, Point2d& moved_point);
    virtual int SetVirtualMap(std::vector<MapArea>* map_areas);
    virtual int MoveUser(CForcedMoveCharacter* character);
    virtual void GetFlagPoint(int& pos_x, int& pos_y);

    // Two words at +0x04/+0x08 are present in the original derived layout;
    // the static placement policy never reads them after construction.
    uint32_t reserved_users[2];
    Point2d flag_point;
    CExtraMatrix extra_matrices[2];
};

// This class layout is owned by C.  The offsets below are taken from the
// original pvp_assault bodies; it is not a proxy or a replacement PvP state.
class CAssaulter {
public:
    CAssaulter();
    ~CAssaulter();

    const char* GetCharacName() const;
    CUser* GetUser();
    int GetUserUniqueId();
    int GetHp();
    bool ChangeHp(int hp_value);
    void _PrepareAssault(CUser* assault_user);
    void AttachUser(CUser* assault_user, ENUM_ASSAULT_TEAM assault_team);
    void DetachUser();
    void GiveupPanalty();
    void DieUser();
    void SetUserConnectP2P(char* connect_state, int player_count);
    bool IsAttachAble();
    bool Empty();
    int GetTeam();
    void SetState(ENUM_ASSAULT_STATE assault_state);
    int GetState();
    void SetPowerWarWinner(bool winner);
    bool IsPowerWarWinner();
    void Reset();

    CUser* user;
    int team;
    int state;
    int update_time;
    int hp;
    uint8_t power_war_winner;
    uint8_t connect_p2p[11];
};

class CAssaultPlace {
public:
    // IDA_EA: 0x82E83C4
    // IDA_NAME: _ZN11pvp_assault13CAssaultPlace13SendUserStateEP5CUser
    // DEMANGLED: pvp_assault::CAssaultPlace::SendUserState(CUser *)
    static void SendUserState(CUser* user);

    // IDA_EA: 0x82ED6A6
    // IDA_NAME: _ZN11pvp_assault13CAssaultPlace25ForcedMoveAtPowerWarPlaceEP5CUser
    // DEMANGLED: pvp_assault::CAssaultPlace::ForcedMoveAtPowerWarPlace(CUser *)
    // The original code has no CAssaultPlace receiver; retain the class-scoped
    // static ABI so the manager and assault-close paths share the one helper.
    static void ForcedMoveAtPowerWarPlace(CUser* user);

    CAssaultPlace();
    ~CAssaultPlace();

    CAssaultPlace* Reset();
    int SendPacket(PacketGuard& packet);
    int SendPacket(PacketGuard& packet, ENUM_ASSAULT_TEAM team);
    int GetTeam(CUser* user, ENUM_ASSAULT_TEAM& team);
    int _SendPacketToArea(PacketGuard& packet);
    // IDA_EA: 0x82E6DCE
    // IDA_NAME: _ZN11pvp_assault13CAssaultPlace17_SendPacketToAreaER11PacketGuardRSt6vectorIP5CUserSaIS5_EE
    int _SendPacketToArea(PacketGuard& packet, std::vector<CUser*>& users);
    int _SendPacketToVill(PacketGuard& packet);
    int _DelUser(int index, bool unregister_disconnect);
    int _AddUser(CUser* user, ENUM_ASSAULT_TEAM team);
    int _AddTeam(CUser* user, ENUM_ASSAULT_TEAM team);
    void _DispositionUser(CUser** users);
    PartyInfo* _BackupPartyInfo(CParty* party, ENUM_ASSAULT_TEAM team);
    int _SeparateTeam(CUser* first_user, CUser* second_user);
    bool _SetReadyState();
    void _MoveAssaultPlace();
    bool MakeTeam(CUser* requester, CUser* target);
    int _GetTeamCount(ENUM_ASSAULT_TEAM team);
    bool _GetTeamCountAliveUser(int* team_count);
    bool _GetTeamCount(int* team_count);
    bool _GetTeamCount(int* team_count, ENUM_ASSAULT_STATE state);
    bool _PowerWarAssaultCheckIn();
    int _CheckPowerWarInPlayer(ENUM_POWER_SIDE_TYPE power_side,
                               ENUM_POWER_WAR_WINNER_OR_LOSER result,
                               int& winner_count,
                               int& loser_count,
                               int& live_count);
    void _SendStraightVictories(CUser* user);
    void _SendPowerWarAssaultResultToUser(CUser* user, int point);
    int _VictoryBonus(CUser* user, char winning_power_side);
    void _SendHoldVictory(CUser* winner, CUser* loser, int victories);
    void _SendVictoryBonus(CUser* user,
                           int victories,
                           int penalty,
                           int broken_victories,
                           int break_bonus);
    int _SendPowerWarPointToGuildServer(
        CUser* user,
        ENUM_POWER_WAR_WINNER_OR_LOSER result,
        int winner_count,
        int loser_count,
        int live_count);
    int _MoveUser(CDisposition* disposition, int village_id, int area_id);
    // IDA_EA: 0x82E75CA
    // IDA_NAME: _ZN11pvp_assault13CAssaultPlace17_AvailableAssaultEP5CUserRi
    char _AvailableAssault(CUser* user, int& unique_id);
    void _SendUserInfo();
    // IDA_EA: 0x82E7986
    // IDA_NAME: _ZN11pvp_assault13CAssaultPlace17_SendUserMiniInfoEv
    void _SendUserMiniInfo();
    void _SendUserUdpIpPort();
    void _SendGiveAlarm();
    void _SendUserState();
    int _SendCloseAssault(CUser* user, int hp);
    int _MakeCompleteLoad(PacketGuard* packet, int index);
    int _MakeStartAssault(PacketGuard* packet);
    int _MakeEndAssault(PacketGuard* packet);
    int _MakeViewAssaultIconPlayer(PacketGuard* packet, bool visible);
    int _MakeMoveAssaultPlace(PacketGuard* packet);
    // IDA_EA: 0x82E8B66
    // IDA_NAME: _ZN11pvp_assault13CAssaultPlace13_MakeUserInfoEP11PacketGuard13ENUM_USERINFO
    // DEMANGLED: pvp_assault::CAssaultPlace::_MakeUserInfo(PacketGuard *,ENUM_USERINFO)
    int _MakeUserInfo(PacketGuard* packet, ENUM_USERINFO user_info_type);
    int _MakeUserUdpIpPort(PacketGuard* packet);
    int _MakeGiveAlarm(PacketGuard* packet);
    int _MakeUserState(PacketGuard* packet);
    int _MakeUserState(PacketGuard* packet, CUser* user);
    int _MakeLeaveUser(PacketGuard* packet, int index);
    int _MakeChangeHp(PacketGuard* packet, int index, int hp);
    int _MakeCloseAssault(PacketGuard* packet, int hp);
    int _MakeDieAssaultPlayer(PacketGuard* packet, int index);
    // 0x82E8978 is class-scoped but static: its first machine argument is
    // PacketGuard, followed by the user ID.  Keeping it static is required
    // for the original two-argument ABI used by RevivalUser.
    static int _MakeRevivalAssaultPlayer(PacketGuard* packet, int index);
    int _CheckPlayable();
    int _CheckAssaultable();
    int DoConnectP2P();
    int _IsCannotConnectP2P(int source_index, int target_index);
    int _ClearConnectP2PAbsolute(int index);
    int _ClearConnectP2PRelative(int point);
    int _GetConnectP2PPoint(int index);
    bool _GetConnectP2PMaxMinPoint(int& maximum, int& minimum);
    int ClearBadP2PUser();
    int StartAssault();
    int EndAssault();
    int CloseAssault();
    int TimeoutEndAssault();
    int TimeoutCloseAssault();
    int TimeoutCompleteLoad();
    int TimeoutConnectP2P();
    int _IsBroadcastRunaway(CUser* user);
    int _BroadcastRunaway(CUser* user, int index);
    int _IsGiveupPanalty(bool evade);
    int _GetUserIndex(CUser* user, bool throw_on_missing);
    int LeaveAssaultPlace(int index, bool force, bool evade);
    int LeaveAssaultPlace(CUser* user, bool force);
    int IsArea(int village_id, int area_id);
    int ExistUser(CUser* user);
    int CheckPlayable();
    int GetState();
    int GetAssaultPlaceId();
    CAssaultPlace* SetAssaultPlaceId(unsigned int id);
    int GetAuthenValue();
    int GetAssaulterCount();
    int MakeViewAssaultIconOnePlayer(PacketGuard* packet, bool visible);
    const char* GetAnotherTeamCharacName(int index);
    int EvadeAssaultPlace(CUser* user);
    int AbuseUser(CUser* user);
    // IDA_EA: 0x82EB774
    // The original function returns the low byte of the second party's
    // SetAssaultState result when both parties have completed loading.
    char CompleteLoad(CUser* user);
    int ConnectP2P(CUser* user, char* states, int count);
    int ChangeHp(CUser* user, int hp);
    int _IsChangeDieState();
    int _ChangeDieState(int index);
    int _IsProcsssChaos();
    void _ProcessChaos(int dead_index, unsigned int killer_index);
    int DieUser(int dead_index,
                unsigned int killer_index,
                const MSG_DIE_ASSAULT_PLAYER& message);
    int DieUser(CUser* dead_user,
                unsigned int killer_index,
                const MSG_DIE_ASSAULT_PLAYER& message);
    // IDA_EA: 0x82ED43E
    // IDA_NAME: _ZN11pvp_assault13CAssaultPlace11RevivalUserEP5CUserRi
    // The binary has no CAssaultPlace receiver for this class-scoped helper.
    static void RevivalUser(CUser* user, int& remaining_seconds);

    CAssaulter assaulters[8];
    uint32_t assaulter_count;
    uint32_t assault_place_id;
    uint32_t authen_value;
    uint32_t state;
    uint32_t pos_x;
    uint32_t pos_y;
    uint32_t village;
    uint32_t area;
    // Directly observed bytes at +0x120/+0x121; their business labels are
    // intentionally deferred until all producers are restored.
    // +0x120: final winning-team flags, set by EndAssault.
    uint8_t team_result[2];
    uint8_t pad_122[2];
    uint32_t winner_team;
    // +0x128: the two teams' chaos-mode snapshots, set by _SeparateTeam.
    // The prior reconstruction incorrectly typed these bytes as the low
    // part of a CUser pointer, which made the CMD150 chaos branch invalid.
    uint8_t team_chaos_start[2];
    uint8_t pad_12a[2];
    PartyInfo party_info[2];
    CUser* request_user;
    CUser* assaulted_user;
    uint32_t assault_start_second;
};

class CAssaultMgr {
public:
    CAssaultMgr();
    ~CAssaultMgr();

    CAssaultMgr* SetAssaultChannel(bool enabled);
    unsigned int _GetNextAssaultPlaceId();
    void _SendVictoryInfo(CUser* requester, CUser* target);
    int FrocedCloseAssaults();
    // IDA_EA: 0x82ED966
    // IDA_NAME: _ZN11pvp_assault11CAssaultMgr5ResetEv
    CAssaultMgr* Reset();
    CAssaultPlace* _GetAssaultPlace(unsigned int assault_place_id);
    CAssaultPlace* _GetAssaultPlace(unsigned int assault_place_id,
                                    int authen_value);
    void FreeAssaultPlace(int assault_place_id,
                          CAssaultPlace* place);
    // IDA_EA: 0x82EDC86
    // IDA_NAME: _ZN11pvp_assault11CAssaultMgr26GetRequestAssaultNeedMoneyEP5CUserS2_
    int GetRequestAssaultNeedMoney(CUser* requester, CUser* target);
    // IDA_EA: 0x82EDFBA
    // IDA_NAME: _ZN11pvp_assault11CAssaultMgr16OnRequestAssaultEP5CUserS2_Pi
    // A non-null preflight_result validates a quoted CMD447 request without
    // allocating an assault place or charging money.
    int OnRequestAssault(CUser* requester,
                         CUser* target,
                         int* preflight_result);
    int OnCompleteLoad(CUser* user);
    int OnDieUser(CUser* user,
                  int killer_index,
                  const MSG_DIE_ASSAULT_PLAYER& message);
    // IDA_EA: 0x82EF464
    // IDA_NAME: _ZN11pvp_assault11CAssaultMgr13OnRevivalUserEP5CUser
    int OnRevivalUser(CUser* user);
    int OnConnectP2P(CUser* user, char* states, int count);
    int OnLeaveAssaultPlace(CUser* user, bool force);
    int OnChangeHp(CUser* user, int hp);
    int OnAbuseUser(CUser* user);
    int OnTimeoutCompleteLoad(unsigned int assault_place_id, int authen_value);
    // IDA_EA: 0x82EF86E
    // IDA_NAME: _ZN11pvp_assault11CAssaultMgr33OnTimeoutCompleteLoadAfterAssaultEji
    int OnTimeoutCompleteLoadAfterAssault(unsigned int party_index,
                                          int timer_key);
    int OnTimeoutConnectP2P(unsigned int assault_place_id, int authen_value);
    int OnTimeoutEndAssault(unsigned int assault_place_id, int authen_value);
    int OnTimeoutCloseAssault(unsigned int assault_place_id, int authen_value);
    int OnTimeoutConnectP2PAfterAssault(unsigned int party_id,
                                        int timer_key);
    int OnMoveArea(CUser* user);
    // IDA_EA: 0x82EFB54
    // IDA_NAME: _ZN11pvp_assault11CAssaultMgr10SendPacketEP5CUserbP11PacketGuard
    // DEMANGLED: pvp_assault::CAssaultMgr::SendPacket(CUser *,bool,PacketGuard *)
    int SendPacket(CUser* user, bool team_only, PacketGuard* packet);

    // IDA_EA: 0x82EF320
    // IDA_NAME: _ZN11pvp_assault11CAssaultMgr24OnConnectP2PAfterAssaultEP5CUserPci
    // DEMANGLED: pvp_assault::CAssaultMgr::OnConnectP2PAfterAssault(CUser *,char *,int)
    int OnConnectP2PAfterAssault(CUser* user, char* states, int count);

    // IDA_EA: 0x82ED672
    // IDA_NAME: _ZN11pvp_assault11CAssaultMgr25ForcedMoveAtPowerWarPlaceEP5CUser
    // DEMANGLED: pvp_assault::CAssaultMgr::ForcedMoveAtPowerWarPlace(CUser *)
    int ForcedMoveAtPowerWarPlace(CUser* user);

    // IDA_EA: 0x82EFE14
    // IDA_NAME: _ZN11pvp_assault11CAssaultMgr25check_party_teleport_userEcP5CUserS2_b
    // DEMANGLED: pvp_assault::CAssaultMgr::check_party_teleport_user(char,CUser *,CUser *,bool)
    int check_party_teleport_user(char request_type,
                                  CUser* requester,
                                  CUser* target,
                                  bool send_error);

    // IDA_EA: 0x82EFBF6
    // IDA_NAME: _ZN11pvp_assault11CAssaultMgr19OnEvadeAssaultPlaceEP5CUser
    // DEMANGLED: pvp_assault::CAssaultMgr::OnEvadeAssaultPlace(CUser *)
    // The concrete manager body is restored separately; packet consumers use
    // this canonical member declaration rather than an asm-name shim.
    int OnEvadeAssaultPlace(CUser* user);

    // The original declaration is a class-scoped static helper: its first
    // machine argument is CUser, not a CAssaultMgr receiver.
    // IDA_EA: 0x82EFF42
    // IDA_NAME: _ZN11pvp_assault11CAssaultMgr15NotifyGhostTimeEP5CUserii
    // DEMANGLED: pvp_assault::CAssaultMgr::NotifyGhostTime(CUser *,int,int)
    static void NotifyGhostTime(CUser* user,
                                int total_ghost_time,
                                int remaining_ghost_time);

    // IDA_EA: 0x82F0022
    // IDA_NAME: _ZN11pvp_assault11CAssaultMgr16AvailableAssaultEP5CUserPKS1_
    char AvailableAssault(CUser* user, const CUser* requester);

    uint8_t assault_channel;
    uint8_t pad_01[3];
    uint32_t next_assault_place_id;
    std::map<unsigned int, CAssaultPlace*> assault_places;
};

int GetDirectionByTeam(ENUM_ASSAULT_TEAM team);
CAssaultMgr* GetInstanceAssaultMgr();
CAssaultPlaceFactory* GetInstanceAssaultPlaceFactory();

// IDA_EA: 0x82E5A0B
// IDA_NAME: _ZN11pvp_assault25IsIncludeVirtualMoveAreasEPSt6vectorI7MapAreaSaIS1_EEiiRiS5_i
// DEMANGLED: pvp_assault::IsIncludeVirtualMoveAreas(std::vector<MapArea,std::allocator<MapArea>> *,int,int,int &,int &,int)
int IsIncludeVirtualMoveAreas(std::vector<MapArea>* map_areas,
                              int pos_x,
                              int pos_y,
                              int& moved_x,
                              int& moved_y,
                              int margin);

static_assert(sizeof(CAssaulter) == 0x20u,
              "CAssaulter IDA ABI size must stay 0x20");
static_assert(sizeof(CForcedMoveCharacter) == 0x20u,
              "CForcedMoveCharacter IDA ABI size must stay 0x20");
static_assert(sizeof(CCandidateMatrix) == 0x0Cu,
              "CCandidateMatrix IDA ABI size must stay 0x0C");
static_assert(sizeof(CExtraMatrix) == 0x0Cu,
              "CExtraMatrix IDA ABI size must stay 0x0C");
static_assert(sizeof(CDynamicDisposition) == 0x2Cu,
              "CDynamicDisposition IDA ABI size must stay 0x2C");
static_assert(sizeof(CStaticDisposition) == 0x2Cu,
              "CStaticDisposition IDA ABI size must stay 0x2C");
static_assert(offsetof(CAssaultPlace, assault_place_id) == 0x104u,
              "CAssaultPlace assault-place ID offset must stay 0x104");
static_assert(offsetof(CAssaultPlace, state) == 0x10cu,
              "CAssaultPlace state offset must stay 0x10c");
static_assert(offsetof(CAssaultPlace, village) == 0x118u,
              "CAssaultPlace village offset must stay 0x118");
static_assert(offsetof(CAssaultPlace, party_info) == 0x12cu,
              "CAssaultPlace party-info offset must stay 0x12C");
static_assert(offsetof(CAssaultPlace, assault_start_second) == 0x13cu,
              "CAssaultPlace start-time offset must stay 0x13C");
static_assert(sizeof(CAssaultPlace) == 0x140u,
              "CAssaultPlace IDA ABI size must stay 0x140");
static_assert(offsetof(CAssaultMgr, assault_places) == 0x08u,
              "CAssaultMgr map offset must stay 0x08");

}  // namespace pvp_assault

class Timer_AssaultGhostTimer {
public:
    // IDA_EA: 0x863B242
    // IDA_NAME: _ZN23Timer_AssaultGhostTimer17RegisterNextTimerEP5CUserij
    // DEMANGLED: Timer_AssaultGhostTimer::RegisterNextTimer(CUser *,int,unsigned int)
    static void RegisterNextTimer(CUser* user,
                                  int remaining_ghost_time,
                                  unsigned int charac_no);
};

// IDA_EA: 0x82E59F1
// IDA_NAME: _Z11GetDistanceN11pvp_assault7Point2dES0_
// DEMANGLED: GetDistance(pvp_assault::Point2d,pvp_assault::Point2d)
int GetDistance(pvp_assault::Point2d lhs, pvp_assault::Point2d rhs);

// IDA_EA: 0x82E5ED1
// IDA_NAME: _Z25IsIncludeVirtualMoveAreasPSt6vectorI7MapAreaSaIS0_EEii
// DEMANGLED: IsIncludeVirtualMoveAreas(std::vector<MapArea,std::allocator<MapArea>> *,int,int)
int IsIncludeVirtualMoveAreas(std::vector<MapArea>* map_areas,
                              int pos_x,
                              int pos_y);

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PVP_ASSAULT_H_
