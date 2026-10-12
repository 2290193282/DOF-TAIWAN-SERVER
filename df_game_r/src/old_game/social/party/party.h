#ifndef OLDSRV_OLD_GAME_GROUP_PARTY_H_
#define OLDSRV_OLD_GAME_GROUP_PARTY_H_

#include <stddef.h>
#include <stdint.h>
#include <list>
#include <map>
#include <vector>

#include "old_game/activity/reward/event_base.h"
#include "old_data/PVF/data/datamanager/required_item.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/social/party/party_messages.h"
#include "old_game/economy/mall/secret_shop_runtime.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/concurrent/thread/mutex.h"

class CDungeon;
class CItem;
class CLuckPoint;
class CUser;
enum ENUM_MAZE_QUEST_TYPE : int;
class CWorldMap;
class Quest;
class PacketGuard;
class Dispatcher_TournamentRewardSelect;
class DisPatcher_DeleteItem;
struct stClearRewardBonusExp_t;
struct stClearRewardExpParameter_t;
struct MSG_MOVE_MAP;
struct MSG_MONSTER_DIE;
enum ENUM_USER_DUNGEON_STATE : int;
enum eCoinSubReason : int;
enum eClearRewardCardType_t : int;
enum ENUM_WALKOUT_TYPE : int;
// IDA named parameter type for CParty::checkInoutConditionDungeon's
// command-driven overload.  No enumerator names are reconstructed yet, so
// retain the verified int ABI without inventing semantic values.
enum ENUM_DUNGEON_MODE : int;

// IDA unit: 09_dungeon_pvp_party/ManageBonusType.
// This namespace-scoped enum preserves the original parameter type of
// CParty::CalcurateManageBonus.
namespace ManageBonusType {
enum T {
    Exp = 0,
    Gold = 1,
};
}  // namespace ManageBonusType

// IDA unit: 99_misc_global/STGainedEachExpData.  The constructor initializes
// only the base experience and scale; callers supply the member-level gap.
struct STGainedEachExpData {
    // IDA_EA: 0x8151A56
    // IDA_NAME: _ZN19STGainedEachExpDataC2Ev
    // DEMANGLED: STGainedEachExpData::STGainedEachExpData(void)
    STGainedEachExpData();

    unsigned int gained_exp;  // +0x00
    float exp_scale;          // +0x04
    int member_level_gap;     // +0x08
};

static_assert(sizeof(STGainedEachExpData) == 0x0cu,
              "STGainedEachExpData IDA ABI");

namespace QuickParty {
enum BasicRewardType : int;
enum RandomBuffType : int;
struct RewardData_Param;
struct RewardData_Result;
}  // namespace QuickParty

namespace WongWork {
struct stGenerateResult_t;
}  // namespace WongWork

namespace secretshop {
struct SECRET_SHOP_DATA;
struct SECRET_SHOP_INFO;
}  // namespace secretshop

// IDA shared party ABI types. Values are intentionally not reconstructed here:
// callers retain the original numeric values at their IDA call sites.
enum ENUM_PARTY_INFO_TYPE {
    ENUM_PARTY_INFO_TYPE_ABI_PLACEHOLDER = 0,
};

namespace RetryFailReason {
enum T {
    T_ABI_PLACEHOLDER = 0,
};
}  // namespace RetryFailReason

// CParty::CParty at 0x82A5516 clears only the party timer-key slots for
// values 11 through 47.  The names below are corroborated by the matching
// TimerDispatcher registrations at 0x863120A; values outside this CParty
// interval remain outside this partial enum recovery.
enum TIMER_MESSAGE : int {
    TIMER_MESSAGE_ABI_PLACEHOLDER = 0,
    TIMER_MESSAGE_MAP_LOADING = 11,
    TIMER_MESSAGE_SET_RESULT = 12,
    TIMER_MESSAGE_DUNGEON_CLEAR = 13,
    TIMER_MESSAGE_DUNGEON_FAIL = 14,
    TIMER_MESSAGE_PARTY_WALK_OUT_LACK_USER = 15,
    TIMER_MESSAGE_CARD_SELECT = 16,
    TIMER_MESSAGE_CLEAR_DUNGEON_REWARD = 17,
    TIMER_MESSAGE_CLEAR_DUNGEON_SCORE_SCROLL = 18,
    TIMER_MESSAGE_CLEAR_DUNGEON_CARD_SELECT_RIGHT = 19,
    TIMER_MESSAGE_EPLP_RETURN_VILLAGE = 20,
    TIMER_MESSAGE_EPLP_COMMAND_WAIT = 21,
    TIMER_MESSAGE_BLOOD_SMALL_ROUND_REGISTER = 22,
    TIMER_MESSAGE_BLOOD_PHASE_REGISTER_0 = 23,
    TIMER_MESSAGE_BLOOD_PHASE_REGISTER_1 = 24,
    TIMER_MESSAGE_BLOOD_PHASE_REGISTER_2 = 25,
    TIMER_MESSAGE_BLOOD_PHASE_REGISTER_3 = 26,
    TIMER_MESSAGE_BLOOD_PHASE_REGISTER_4 = 27,
    TIMER_MESSAGE_BLOOD_PHASE_REGISTER_5 = 28,
    TIMER_MESSAGE_BLOOD_PHASE_REGISTER_6 = 29,
    TIMER_MESSAGE_BLOOD_PHASE_REGISTER_7 = 30,
    TIMER_MESSAGE_BLOOD_PHASE_REGISTER_8 = 31,
    TIMER_MESSAGE_BLOOD_PHASE_REGISTER_9 = 32,
    TIMER_MESSAGE_BLOOD_CRAZY_MONSTER = 33,
    TIMER_MESSAGE_BLOOD_DUNGEON_CLEAR_REWARD_0 = 34,
    TIMER_MESSAGE_BLOOD_DUNGEON_CLEAR_REWARD_1 = 35,
    TIMER_MESSAGE_BLOOD_DUNGEON_CLEAR_REWARD_2 = 36,
    TIMER_MESSAGE_BLOOD_DUNGEON_CLEAR_REWARD_3 = 37,
    TIMER_MESSAGE_ULTIMATE_NEXT_ROUND_START = 38,
    TIMER_MESSAGE_VILLAGE_ATTACKED_PLAY_TIMEOUT = 39,
    TIMER_MESSAGE_ELEVATOR_DUNGEON = 40,
    TIMER_MESSAGE_SET_PLAY_RESULT = 41,
    TIMER_MESSAGE_COMPLETE_LOAD_AFTER_ASSAULT = 42,
    TIMER_MESSAGE_ROUTING_ITEM_WAIT = 43,
    TIMER_MESSAGE_TOD_SEND_CLEAR_REWARD = 44,
    TIMER_MESSAGE_QUICK_PARTY_WAITING_START_GAME = 45,
    TIMER_MESSAGE_TOURNAMENT_DUNGEON_CLEAR_STATE_NOTI_REWARD = 46,
    TIMER_MESSAGE_TOURNAMENT_DUNGEON_CLEAR_STATE_SELECT = 47,
    // IDA: TimerPVPWalkOutLackUser is installed in TimerDispatcher slot 49
    // and PvP_Room::VoteLackUser schedules this exact message.
    TIMER_MESSAGE_PVP_WALK_OUT_LACK_USER = 49,
};

class CParty;

// IDA unit: 09_dungeon_pvp_party/CPartyTelePort.
// The CParty constructor at 0x82A5516 constructs this record at +0x1AD0;
// the next CParty field begins at +0x1AE0, establishing the 0x10-byte ABI.
class CPartyTelePort {
public:
    // IDA_EA: 0x85BB832
    // IDA_NAME: _ZN14CPartyTelePortC2Ev
    // DEMANGLED: CPartyTelePort::CPartyTelePort(void)
    CPartyTelePort();
    // IDA_EA: 0x85BB848
    // IDA_NAME: _ZN14CPartyTelePortD2Ev
    // DEMANGLED: CPartyTelePort::~CPartyTelePort()
    ~CPartyTelePort();

    // IDA_EA: 0x822D7EE
    // IDA_NAME: _ZN14CPartyTelePort18get_teleport_stateEv
    // DEMANGLED: CPartyTelePort::get_teleport_state(void)
    int get_teleport_state();
    // IDA_EA: 0x822D7FA
    // IDA_NAME: _ZN14CPartyTelePort18set_teleport_stateEc
    // DEMANGLED: CPartyTelePort::set_teleport_state(char)
    CPartyTelePort* set_teleport_state(char state);
    // IDA_EA: 0x85BB84E
    // IDA_NAME: _ZN14CPartyTelePort4initEP6CParty
    // DEMANGLED: CPartyTelePort::init(CParty *)
    CPartyTelePort* init(CParty* party);
    // IDA_EA: 0x85BB85C
    // IDA_NAME: _ZN14CPartyTelePort17set_teleport_dataEhhsshc
    // DEMANGLED: CPartyTelePort::set_teleport_data(unsigned char,unsigned char,short,short,unsigned char,char)
    bool set_teleport_data(unsigned char village,
                           unsigned char area,
                           short pos_x,
                           short pos_y,
                           unsigned char direction,
                           char state);
    // IDA_EA: 0x85BC2B0
    // IDA_NAME: _ZN14CPartyTelePort39send_teleport_effect_at_different_placeEi
    // DEMANGLED: CPartyTelePort::send_teleport_effect_at_different_place(int)
    CParty* send_teleport_effect_at_different_place(int effect_type);
    // IDA_EA: 0x85BC64C
    // IDA_NAME: _ZN14CPartyTelePort35send_teleport_effect_at_equal_placeEi
    // DEMANGLED: CPartyTelePort::send_teleport_effect_at_equal_place(int)
    void send_teleport_effect_at_equal_place(int effect_type);
    // IDA_EA: 0x85BB91C
    // IDA_NAME: _ZN14CPartyTelePort25set_teleport_member_stateEic
    // DEMANGLED: CPartyTelePort::set_teleport_member_state(int,char)
    CParty* set_teleport_member_state(int seat, char state);
    // IDA_EA: 0x85BC044
    // IDA_NAME: _ZN14CPartyTelePort19reset_teleport_dataEv
    // DEMANGLED: CPartyTelePort::reset_teleport_data(void)
    CPartyTelePort* reset_teleport_data();
    // IDA_EA: 0x85BB96E
    // IDA_NAME: _ZN14CPartyTelePort20send_teleport_statusEh
    // DEMANGLED: CPartyTelePort::send_teleport_status(unsigned char)
    void send_teleport_status(unsigned char error);
    // IDA_EA: 0x85BBB08
    // IDA_NAME: _ZN14CPartyTelePort24check_cur_teleport_stateEv
    // DEMANGLED: CPartyTelePort::check_cur_teleport_state(void)
    int check_cur_teleport_state();
    // IDA_EA: 0x85BBBB8
    // IDA_NAME: _ZN14CPartyTelePort16process_teleportEv
    // DEMANGLED: CPartyTelePort::process_teleport(void)
    int process_teleport();
    // IDA_EA: 0x85BC0A0
    // IDA_NAME: _ZN14CPartyTelePort30process_leave_user_at_teleportEP5CUser
    // DEMANGLED: CPartyTelePort::process_leave_user_at_teleport(CUser *)
    char process_leave_user_at_teleport(CUser* user);
    // IDA_EA: 0x85BC252
    // IDA_NAME: _ZN14CPartyTelePort25get_teleport_member_stateEi
    // DEMANGLED: CPartyTelePort::get_teleport_member_state(int)
    int get_teleport_member_state(int seat);
    // IDA_EA: 0x85BF658
    // IDA_NAME: _ZN14CPartyTelePort20get_teleport_villageEv
    // DEMANGLED: CPartyTelePort::get_teleport_village(void)
    int get_teleport_village();
    // IDA_EA: 0x85BF664
    // IDA_NAME: _ZN14CPartyTelePort23get_teleport_area_indexEv
    // DEMANGLED: CPartyTelePort::get_teleport_area_index(void)
    int get_teleport_area_index();
    // IDA_EA: 0x85BF670
    // IDA_NAME: _ZN14CPartyTelePort17get_teleport_posxEv
    // DEMANGLED: CPartyTelePort::get_teleport_posx(void)
    int get_teleport_posx();
    // IDA_EA: 0x85BF67C
    // IDA_NAME: _ZN14CPartyTelePort17get_teleport_posyEv
    // DEMANGLED: CPartyTelePort::get_teleport_posy(void)
    int get_teleport_posy();
    // IDA_EA: 0x85BF688
    // IDA_NAME: _ZN14CPartyTelePort22get_teleport_directionEv
    // DEMANGLED: CPartyTelePort::get_teleport_direction(void)
    int get_teleport_direction();

private:
    CParty* party_;                 // +0x00, 0x85BB84E
    char state_;                    // +0x04, 0x822D7EE
    uint8_t village_;               // +0x05, 0x85BF658
    uint8_t area_index_;            // +0x06, 0x85BF664
    uint8_t pad_07_;
    uint16_t pos_x_;                // +0x08, 0x85BF670
    uint16_t pos_y_;                // +0x0A, 0x85BF67C
    uint8_t direction_;             // +0x0C, 0x85BF688
    uint8_t pad_0d_0f_[3];

    friend class CParty;
};

static_assert(sizeof(CPartyTelePort) == 0x10u,
              "CPartyTelePort IDA ABI size must stay 0x10");

// IDA unit: 09_dungeon_pvp_party/CPartyExpBonusEvent.
// The party event's shared ABI belongs with CParty.  Its one implementation is
// in old_game/group/cparty_exp_bonus_event_runtime.cpp.
class CPartyExpBonusEvent : public CEventBase {
public:
    // IDA_EA: 0x8116D4A
    // IDA_NAME: _ZN19CPartyExpBonusEventC2Ev
    // DEMANGLED: CPartyExpBonusEvent::CPartyExpBonusEvent(void)
    CPartyExpBonusEvent();
    // IDA_EA: 0x8116D70
    // IDA_NAME: _ZN19CPartyExpBonusEventD2Ev
    // DEMANGLED: CPartyExpBonusEvent::~CPartyExpBonusEvent()
    virtual ~CPartyExpBonusEvent();

    // IDA_EA: 0x8116C70
    // IDA_NAME: _ZN19CPartyExpBonusEvent10StartEventEv
    // DEMANGLED: CPartyExpBonusEvent::StartEvent(void)
    virtual void StartEvent();
    // IDA_EA: 0x8116CF8
    // IDA_NAME: _ZN19CPartyExpBonusEvent8EndEventEv
    // DEMANGLED: CPartyExpBonusEvent::EndEvent(void)
    virtual void EndEvent();
    // IDA_EA: 0x8116CD0
    // IDA_NAME: _ZN19CPartyExpBonusEvent10StartEventE10Word_Param
    // DEMANGLED: CPartyExpBonusEvent::StartEvent(Word_Param)
    virtual void StartEvent(Word_Param param);

    // IDA_EA: 0x85BFE1A
    // IDA_NAME: _ZN19CPartyExpBonusEvent12getBonusRateEib
    // DEMANGLED: CPartyExpBonusEvent::getBonusRate(int,bool)
    long double getBonusRate(int member_count, bool abuse_party);

private:
    int bonus_rate_;
};

// IDA unit: 09_dungeon_pvp_party/CPartyResultRecvFlag.
struct CPartyResultRecvFlag {
    // IDA_EA: 0x82A4CB2
    // IDA_NAME: _ZN20CPartyResultRecvFlagC2Ev
    // DEMANGLED: CPartyResultRecvFlag::CPartyResultRecvFlag(void)
    CPartyResultRecvFlag();
    // IDA_EA: 0x82A4CC6
    // IDA_NAME: _ZN20CPartyResultRecvFlag5ClearEv
    // DEMANGLED: CPartyResultRecvFlag::Clear(void)
    void* Clear();
    // IDA_EA: 0x85BF3D8
    // IDA_NAME: _ZN20CPartyResultRecvFlag11SetRecvFlagEi
    // DEMANGLED: CPartyResultRecvFlag::SetRecvFlag(int)
    int SetRecvFlag(int index);
    // IDA_EA: 0x85BF406
    // IDA_NAME: _ZNK20CPartyResultRecvFlag6IsRecvEi
    // DEMANGLED: CPartyResultRecvFlag::IsRecv(int)const
    int IsRecv(int index) const;
    // IDA_EA: 0x85BF438
    // IDA_NAME: _ZNK20CPartyResultRecvFlag10IsValidIdxEi
    // DEMANGLED: CPartyResultRecvFlag::IsValidIdx(int)const
    bool IsValidIdx(int index) const;

    uint8_t recv_flags[4];
};

// IDA unit: 99_misc_global/cElection<int,4,4>.  CParty owns this real
// 0x40-byte election state at +0xd8; it is not an opaque padding interval.
template <typename T, int VoterCount, int CandidateCount>
class cElection;

template <>
class cElection<int, 4, 4> {
public:
    class cVoter {
    public:
        // IDA_EA: 0x82B5846
        // IDA_NAME: _ZN9cElectionIiLi4ELi4EE6cVoterC2Ev
        cVoter();
        // IDA_EA: 0x82BEB62
        // IDA_NAME: _ZN9cElectionIiLi4ELi4EE6cVoter4InitEv
        cVoter* Init();

    private:
        int votes_[4];

        friend class cElection<int, 4, 4>;
    };

    // IDA_EA: 0x82AAFC4
    // IDA_NAME: _ZN9cElectionIiLi4ELi4EEC2Ev
    cElection();
    // IDA_EA: 0x82B585A
    // IDA_NAME: _ZN9cElectionIiLi4ELi4EE4InitEv
    bool Init();
    // IDA_EA: 0x85C038C
    // IDA_NAME: _ZN9cElectionIiLi4ELi4EE4VoteEii
    cElection* Vote(int voter, int candidate);
    // IDA_EA: 0x85C03A8
    // IDA_NAME: _ZNK9cElectionIiLi4ELi4EE10GetElectedEv
    int GetElected() const;
    // IDA_EA: 0x85C047C
    // IDA_NAME: _ZN9cElectionIiLi4ELi4EE15CancelCandidateEi
    bool CancelCandidate(int candidate);
    // IDA_EA: 0x85C04B2
    // IDA_NAME: _ZN9cElectionIiLi4ELi4EE11CancelVoterEi
    cVoter* CancelVoter(int voter);

private:
    cVoter voters_[4];
};

// IDA unit: 99_misc_global/SecKillMonsterInfo.  This element is owned by
// Secu_DungeonAverageCheck's real vector at CParty +0x1ac0.
struct SecKillMonsterInfo {
    // IDA_EA: 0x8279D30
    // IDA_NAME: _ZN18SecKillMonsterInfoC2Ejjjj
    SecKillMonsterInfo(unsigned int monster_id,
                       unsigned int kill_count,
                       unsigned int hp_total,
                       unsigned int exp_total);
    // IDA_EA: 0x827A488
    // IDA_NAME: _ZN18SecKillMonsterInfoD2Ev
    ~SecKillMonsterInfo();

    uint32_t monster_id_;          // +0x00
    uint32_t kill_count_;          // +0x04
    uint32_t hp_total_;            // +0x08
    uint32_t exp_total_;           // +0x0c
    uint32_t hp_outlier_count_;    // +0x10
    uint32_t exp_outlier_count_;   // +0x14
};

// IDA unit: 09_dungeon_pvp_party/Secu_DungeonAverageCheck.  No vtable is
// present in the old layout: the party pointer is followed by a real vector.
class Secu_DungeonAverageCheck {
public:
    // IDA_EA: 0x82A4C4A
    // IDA_NAME: _ZN24Secu_DungeonAverageCheckC2Ev
    Secu_DungeonAverageCheck();
    // IDA_EA: 0x82A4C6A
    // IDA_NAME: _ZN24Secu_DungeonAverageCheckD2Ev
    ~Secu_DungeonAverageCheck();

    // These named members retain the original ABI identities.  Their policy
    // bodies are intentionally outside this pre-ABI lifecycle handoff.
    void reset();
    void SetPartyPtr(CParty* party);
    void startDungeon();
    void setKillMonsterInfo(const MSG_MONSTER_DIE* message,
                            map_monster* monster);
    std::vector<SecKillMonsterInfo>::iterator findKillMonsterInfo(
        unsigned int monster_id);
    void dungeonClear();
    void dungeonFail();
    void CheckHackTypeEndDungeon();

protected:
    CParty* party_;                                  // +0x00
    std::vector<SecKillMonsterInfo> kill_monsters_;  // +0x04
};

// IDA unit: 09_dungeon_pvp_party/Secu_HackLogCheckByParty.  The derived
// object is embedded at CParty +0x1abc and spans exactly 0x14 bytes.
class Secu_HackLogCheckByParty : public Secu_DungeonAverageCheck {
public:
    // IDA_EA: 0x82A4C80
    // IDA_NAME: _ZN24Secu_HackLogCheckByPartyC2Ev
    Secu_HackLogCheckByParty();
    // IDA_EA: 0x82A4C9E
    // IDA_NAME: _ZN24Secu_HackLogCheckByPartyD2Ev
    ~Secu_HackLogCheckByParty();

    // IDA_EA: 0x827A908
    // IDA_NAME: _ZN24Secu_HackLogCheckByParty4InitEP6CParty
    void Init(CParty* party);

    // The existing old-server behavior identities stay direct member calls;
    // no raw-offset or asm wrapper bridge is retained in CParty consumers.
    void startDungeon();
    void DieMob(const MSG_MONSTER_DIE* message, map_monster* monster);
    void CheckMoveMapFitInPathHackCnt(CUser* user,
                                      CParty* party,
                                      unsigned int client_x,
                                      unsigned int client_y,
                                      int old_map_index);
    void dungeonClear();
    void dungeonFail();

private:
    CParty* party_for_check_;  // +0x10
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(cElection<int, 4, 4>::cVoter) == 0x10u,
              "cElection<int,4,4>::cVoter ABI size must stay 0x10");
static_assert(sizeof(cElection<int, 4, 4>) == 0x40u,
              "cElection<int,4,4> ABI size must stay 0x40");
static_assert(sizeof(SecKillMonsterInfo) == 0x18u,
              "SecKillMonsterInfo ABI size must stay 0x18");
static_assert(sizeof(Secu_DungeonAverageCheck) == 0x10u,
              "Secu_DungeonAverageCheck ABI size must stay 0x10");
static_assert(sizeof(Secu_HackLogCheckByParty) == 0x14u,
              "Secu_HackLogCheckByParty ABI size must stay 0x14");
#endif

// IDA unit: 09_dungeon_pvp_party/CParty.  CParty::ClearMapHitCount at
// 0x85BF850 proves this complete POD aggregate at +0x1874.  The three
// 45-entry series are consumed by checkMoveMapHackCnt (0x85A83A2) and
// CheckHackAverageHitCount (0x85BB0AC); keeping them typed is required by
// Secu_DungeonAverageCheck::setKillMonsterInfo as well.
struct CPartyMapHitStatistics {
    int map_count_;                         // +0x000
    int total_client_hit_count_;            // +0x004
    int client_hit_counts_[45];             // +0x008
    int total_monster_field_a1a_;           // +0x0bc
    int monster_field_a1a_[45];             // +0x0c0
    int total_monster_field_a1e_;           // +0x174
    int monster_field_a1e_[45];             // +0x178
    uint8_t has_a1e_tenth_multiple_;        // +0x22c
    uint8_t pad_22d_22f_[3];
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(offsetof(CPartyMapHitStatistics, client_hit_counts_) == 0x08u,
              "CParty map-hit client values must begin at +0x08");
static_assert(offsetof(CPartyMapHitStatistics, total_monster_field_a1a_) ==
                  0xbcu,
              "CParty map-hit a1a total must begin at +0xbc");
static_assert(offsetof(CPartyMapHitStatistics, total_monster_field_a1e_) ==
                  0x174u,
              "CParty map-hit a1e total must begin at +0x174");
static_assert(offsetof(CPartyMapHitStatistics,
                       has_a1e_tenth_multiple_) == 0x22cu,
              "CParty map-hit a1e marker must begin at +0x22c");
static_assert(sizeof(CPartyMapHitStatistics) == 0x230u,
              "CParty map-hit statistics ABI size must stay 0x230");
#endif

// IDA unit: 09_dungeon_pvp_party/CParty.
//
// CParty is pool-backed.  The prefix and tail below are binary-proven from
// CParty::CParty at 0x82A5516 plus the accessor instruction streams recorded
// in docs/ida_restored/C/dungeon_pvp.md.  Opaque ranges retain their byte
// extent only until their owning runtime types are independently recovered.
class CParty {
public:
    // IDA_EA: 0x82A5516
    // IDA_NAME: _ZN6CPartyC2Ev
    // DEMANGLED: CParty::CParty(void)
    CParty();
    // IDA_EA: 0x82A57FE
    // IDA_NAME: _ZN6CPartyD2Ev
    // DEMANGLED: CParty::~CParty()
    ~CParty();

    struct CItemRoutingData {
        // IDA_EA: 0x82A52D2
        // IDA_NAME: _ZN6CParty16CItemRoutingDataC2Ev
        // DEMANGLED: CParty::CItemRoutingData::CItemRoutingData(void)
        CItemRoutingData();
        // IDA_EA: 0x82A52F4
        // IDA_NAME: _ZN6CParty16CItemRoutingDataD2Ev
        // DEMANGLED: CParty::CItemRoutingData::~CItemRoutingData()
        ~CItemRoutingData();
        // IDA_EA: 0x814575A
        // IDA_NAME: _ZN6CParty16CItemRoutingData15GetRoutingStateEv
        // DEMANGLED: CParty::CItemRoutingData::GetRoutingState(void)
        int GetRoutingState();
        // IDA_EA: 0x82A52FA
        // IDA_NAME: _ZN6CParty16CItemRoutingData5ResetEv
        // DEMANGLED: CParty::CItemRoutingData::Reset(void)
        bool Reset();
        // IDA_EA: 0x85BF694
        // IDA_NAME: _ZN6CParty16CItemRoutingData21GetMemberRoutingStateEi
        // DEMANGLED: CParty::CItemRoutingData::GetMemberRoutingState(int)
        int GetMemberRoutingState(int seat);
        // IDA_EA: 0x85BF6B8
        // IDA_NAME: _ZN6CParty16CItemRoutingData21SetMemberRoutingStateEic
        // DEMANGLED: CParty::CItemRoutingData::SetMemberRoutingState(int,char)
        unsigned int SetMemberRoutingState(int seat, char state);
        // IDA_EA: 0x860B53A
        // IDA_NAME: _ZN6CParty16CItemRoutingData19GetRoutingItemIndexEv
        // DEMANGLED: CParty::CItemRoutingData::GetRoutingItemIndex(void)
        int GetRoutingItemIndex();
        // IDA_EA: 0x860B546
        // IDA_NAME: _ZN6CParty16CItemRoutingData19GetRoutingInvenItemEv
        // DEMANGLED: CParty::CItemRoutingData::GetRoutingInvenItem(void)
        Inven_Item* GetRoutingInvenItem();
        // IDA_EA: 0x860B552
        // IDA_NAME: _ZN6CParty16CItemRoutingData22GetRoutingItemUniqueIDEv
        // DEMANGLED: CParty::CItemRoutingData::GetRoutingItemUniqueID(void)
        int GetRoutingItemUniqueID();
        // IDA_EA: 0x860B55E
        // IDA_NAME: _ZN6CParty16CItemRoutingData15SetRoutingStateEc
        // DEMANGLED: CParty::CItemRoutingData::SetRoutingState(char)
        CItemRoutingData* SetRoutingState(char state);
        // IDA_EA: 0x860B576
        // IDA_NAME: _ZN6CParty16CItemRoutingData19SetRoutingItemIndexER10Inven_Itemj
        // DEMANGLED: CParty::CItemRoutingData::SetRoutingItemIndex(Inven_Item &,unsigned int)
        CItemRoutingData* SetRoutingItemIndex(Inven_Item& item,
                                              unsigned int unique_id);

        int routing_state;                 // +0x00
        Inven_Item routing_inven_item;     // +0x04
        uint8_t pad_41_43[3];
        int routing_item_unique_id;        // +0x44
        uint8_t member_routing_state[4];   // +0x48
    };

    struct cMember {
        // IDA_EA: 0x8599A14
        // IDA_NAME: _ZN6CParty7cMemberC2Ev
        // DEMANGLED: CParty::cMember::cMember(void)
        cMember();
        // IDA_EA: 0x8599A28
        // IDA_NAME: _ZN6CParty7cMember4InitEv
        // DEMANGLED: CParty::cMember::Init(void)
        cMember* Init();

        CUser* user;
        uint8_t state;
        uint8_t pad_05[3];
        int user_uid;
        uint8_t p2p_received;
        uint8_t last_channel;
        uint8_t p2p_connectivity[4];
        uint8_t teleport_state;
        uint8_t pad_13;
        // resetMemberUseCoin clears this counter, set_charac_live increments
        // it on the revive state, and DungeonPermission requires it to stay
        // zero for a no-coin clear.  It is not a map identifier.
        int used_coin_count;
    };

    // IDA_EA: 0x822D91C
    // IDA_NAME: _ZN6CParty9GetMemberEv
    // DEMANGLED: CParty::GetMember(void)
    cMember* GetMember();

    // IDA_EA: 0x814581C
    // IDA_NAME: _ZN6CParty12GetEPLPStateEv
    // DEMANGLED: CParty::GetEPLPState(void)
    int GetEPLPState();
    // IDA_EA: 0x84699DA
    // IDA_NAME: _ZN6CParty18GetSelectedEPLPCmdEv
    // DEMANGLED: CParty::GetSelectedEPLPCmd(void)
    int GetSelectedEPLPCmd();
    // IDA_EA: 0x814578C
    // IDA_NAME: _ZN6CParty12SetEPLPStateEc
    // DEMANGLED: CParty::SetEPLPState(char)
    char SetEPLPState(char state);
    // IDA_EA: 0x85BA906 / 0x85BAA06
    // IDA_NAME: _ZN6CParty16OnTODRewardTimerE13TIMER_MESSAGE /
    //           _ZN6CParty21_Send_TOD_ClearRewardEv
    // DEMANGLED: CParty::OnTODRewardTimer(TIMER_MESSAGE) /
    //            CParty::_Send_TOD_ClearReward(void)
    int OnTODRewardTimer(TIMER_MESSAGE message);
    int _Send_TOD_ClearReward();
    // IDA_EA: 0x8145828
    // IDA_NAME: _ZN6CParty18SetSelectedEPLPCmdEc
    // DEMANGLED: CParty::SetSelectedEPLPCmd(char)
    CParty* SetSelectedEPLPCmd(char command);
    // IDA_EA: 0x8145840
    // IDA_NAME: _ZN6CParty14IsRoutingStateEv
    // DEMANGLED: CParty::IsRoutingState(void)
    int IsRoutingState();

    // IDA_EA: 0x85A7786
    // IDA_NAME: _ZN6CParty18isMonsterDrop_itemEi
    // DEMANGLED: CParty::isMonsterDrop_item(int)
    int isMonsterDrop_item(int unique_id);

    // IDA_EA: 0x85A3B98
    // IDA_NAME: _ZN6CParty8get_itemEP5CUseribcRiRhRb
    // DEMANGLED: CParty::get_item(CUser *,int,bool,char,int &,unsigned char &,bool &)
    int get_item(CUser* user,
                 int unique_id,
                 bool client_flag,
                 char client_rarity,
                 int& item_id,
                 unsigned char& upgrade,
                 bool& stackable);
    // IDA_EA: 0x85B949C
    // IDA_NAME: _ZN6CParty10_onGetItemEP5CUserjj
    // DEMANGLED: CParty::_onGetItem(CUser *,unsigned int,unsigned int)
    void _onGetItem(CUser* user,
                    unsigned int item_id,
                    unsigned int item_count);
    // IDA_EA: 0x85A27E8
    // IDA_NAME: _ZN6CParty12kill_monsterEP5CUseritRK15MSG_MONSTER_DIEPi
    int kill_monster(CUser* user,
                     int monster_unique_id,
                     unsigned short monster_type,
                     const MSG_MONSTER_DIE& message,
                     int* result);
    // IDA_EA: 0x85B5A4C
    // IDA_NAME: _ZN6CParty15OnKilledMonsterEP5CUseri11map_monsterRK15MSG_MONSTER_DIE
    int OnKilledMonster(CUser* user,
                        int monster_id,
                        map_monster monster,
                        const MSG_MONSTER_DIE& message);
    // IDA_EA: 0x85BAE6E
    // IDA_NAME: _ZN6CParty25MemberRegisterKillMonsterER11map_monsterh
    // DEMANGLED: CParty::MemberRegisterKillMonster(map_monster &,unsigned char)
    char MemberRegisterKillMonster(map_monster& monster,
                                   unsigned char dungeon_difficulty);
    // IDA_EA: 0x85B6540
    // IDA_NAME: _ZN6CParty20DestroyPassiveObjectEi
    int DestroyPassiveObject(int object_unique_id);
    // IDA_EA: 0x85B66FA
    // IDA_NAME: _ZN6CParty10isMyMemberEj
    int isMyMember(unsigned int user_unique_id);
    // IDA_EA: DisPatcher_DieMob::dispatch_sig @ 0x81EB0C4.  CMD042 records
    // at most 200 per-map monster reports in this real CParty +0x1844 map
    // before it enters either the passive-object or ordinary kill path.
    void recordDieMobReport(unsigned short monster_unique_id);
    // IDA_EA: 0x85BA762
    // IDA_NAME: _ZN6CParty18setMonsterMoveHackEv
    bool setMonsterMoveHack();
    // IDA_EA: 0x82A5A34
    // IDA_NAME: _ZN6CParty15SetAssaultStateEc
    // DEMANGLED: CParty::SetAssaultState(char)
    CParty* SetAssaultState(char state);
    // IDA_EA: 0x85BF82A
    // IDA_NAME: _ZN6CParty13SetEndRoutingEv
    // DEMANGLED: CParty::SetEndRouting(void)
    bool SetEndRouting();
    // IDA_EA: 0x82A59CA
    // IDA_NAME: _ZN6CParty6SetIDXEi
    // DEMANGLED: CParty::SetIDX(int)
    CParty* SetIDX(int index);
    // IDA_EA: 0x85BA574
    // IDA_NAME: _ZN6CParty24GetFirstValidMemberStateEv
    // DEMANGLED: CParty::GetFirstValidMemberState(void)
    int GetFirstValidMemberState();
    // IDA_EA: 0x860B616
    // IDA_NAME: _ZN6CParty22GetRoutingItemUniqueIDEv
    // DEMANGLED: CParty::GetRoutingItemUniqueID(void)
    int GetRoutingItemUniqueID();
    // IDA_EA: 0x85BF718
    // IDA_NAME: _ZN6CParty25ResetPremiumGoldCardPartyEv
    // DEMANGLED: CParty::ResetPremiumGoldCardParty(void)
    CParty* ResetPremiumGoldCardParty();
    // IDA_EA: 0x85BF748
    // IDA_NAME: _ZN6CParty31ResetPremiumGoldCardDefaultItemEv
    // DEMANGLED: CParty::ResetPremiumGoldCardDefaultItem(void)
    CParty* ResetPremiumGoldCardDefaultItem();
    // IDA_EA: 0x859B3E4
    // IDA_NAME: _ZN6CParty21init_quick_party_dataEv
    // DEMANGLED: CParty::init_quick_party_data(void)
    CParty* init_quick_party_data();

    // IDA_EA: 0x859B2B6
    // IDA_NAME: _ZN6CParty9join_userEP5CUser
    // DEMANGLED: CParty::join_user(CUser *)
    int join_user(CUser* user);

    // IDA_EA: 0x859B52C
    // IDA_NAME: _ZN6CParty28delete_party_for_quick_partyEP5CUserRSt4listIS1_SaIS1_EE
    // DEMANGLED: CParty::delete_party_for_quick_party(
    //     CUser *,std::list<CUser *,std::allocator<CUser *> > &)
    int delete_party_for_quick_party(CUser* user,
                                     std::list<CUser*>& detached_users);

    // IDA_EA: 0x859B404
    // IDA_NAME: _ZN6CParty30change_manager_for_quick_partyEv
    // DEMANGLED: CParty::change_manager_for_quick_party(void)
    int change_manager_for_quick_party();
    // IDA_EA: 0x85B29BC
    // IDA_NAME: _ZN6CParty15set_charac_liveEP5CUser23ENUM_USER_DUNGEON_STATE
    // DEMANGLED: CParty::set_charac_live(CUser *,ENUM_USER_DUNGEON_STATE)
    void set_charac_live(CUser* user, ENUM_USER_DUNGEON_STATE state);
    // IDA_EA: 0x85B2B52
    // IDA_NAME: _ZN6CParty19check_allmember_dieEv
    // DEMANGLED: CParty::check_allmember_die(void)
    int check_allmember_die();
    // IDA_EA: 0x85A7828
    // IDA_NAME: _ZN6CParty8die_userEP5CUser
    // DEMANGLED: CParty::die_user(CUser *)
    void die_user(CUser* user);
    // IDA_EA: 0x85B8E5C
    // IDA_NAME: _ZN6CParty18BloodHandleDieUserEP5CUser
    // DEMANGLED: CParty::BloodHandleDieUser(CUser *)
    int BloodHandleDieUser(CUser* user);
    // IDA_EA: 0x85B3520
    // IDA_NAME: _ZN6CParty8SaveCoinE14eCoinSubReason
    // DEMANGLED: CParty::SaveCoin(eCoinSubReason)
    bool SaveCoin(eCoinSubReason reason);
    // IDA_EA: 0x859B142
    // IDA_NAME: _ZN6CParty15set_single_playEP5CUser
    // DEMANGLED: CParty::set_single_play(CUser *)
    int set_single_play(CUser* user);
    // IDA_EA: 0x8145882
    // IDA_NAME: _ZN6CParty14getMemberNamesEPc
    // DEMANGLED: CParty::getMemberNames(char *)
    char* getMemberNames(char* buffer);
    // IDA_EA: 0x822D90E
    // IDA_NAME: _ZN6CParty16GetPartyTelePortEv
    // DEMANGLED: CParty::GetPartyTelePort(void)
    CPartyTelePort* GetPartyTelePort();
    // IDA_EA: 0x859D1AA
    // IDA_NAME: _ZN6CParty23send_to_party_cmd_errorE14ENUM_CMDPACKETh
    // DEMANGLED: CParty::send_to_party_cmd_error(ENUM_CMDPACKET,unsigned char)
    void send_to_party_cmd_error(ENUM_CMDPACKET command,
                                 unsigned char error);
    // IDA_EA: 0x85B4BDE
    // IDA_NAME: _ZN6CParty15_getMemberNamesEPc
    // DEMANGLED: CParty::_getMemberNames(char *)
    char* _getMemberNames(char* buffer);
    // IDA_EA: 0x859AE76
    // IDA_NAME: _ZN6CParty13gen_timer_keyE13TIMER_MESSAGE
    // DEMANGLED: CParty::gen_timer_key(TIMER_MESSAGE)
    int gen_timer_key(TIMER_MESSAGE message);
    // IDA_EA: 0x85BD6DE
    // IDA_NAME: _ZN6CParty28InsertTournamentDungeonTimerE13TIMER_MESSAGE
    // DEMANGLED: CParty::InsertTournamentDungeonTimer(TIMER_MESSAGE)
    int InsertTournamentDungeonTimer(TIMER_MESSAGE message);
    // IDA_EA: 0x859AEA4
    // IDA_NAME: _ZN6CParty15check_timer_keyE13TIMER_MESSAGEi
    // DEMANGLED: CParty::check_timer_key(TIMER_MESSAGE,int)
    bool check_timer_key(TIMER_MESSAGE message, int key);
    // IDA_EA: 0x859AD4C
    // IDA_NAME: _ZN6CParty7destroyEv
    // DEMANGLED: CParty::destroy(void)
    CParty* destroy();
    // IDA_EA: 0x859F75E
    // IDA_NAME: _ZN6CParty31get_blood_dungeon_admission_feeEP5CUser
    // DEMANGLED: CParty::get_blood_dungeon_admission_fee(CUser *)
    int get_blood_dungeon_admission_fee(CUser* user);
    // IDA_EA: 0x85BE8A0
    // IDA_NAME: _ZN6CParty31isDungeonDifficultyInfromNoticeEb
    // DEMANGLED: CParty::isDungeonDifficultyInfromNotice(bool)
    int isDungeonDifficultyInfromNotice(bool tower);
    // IDA_EA: 0x85BE39A
    // IDA_NAME: _ZN6CParty25checkFreeRevivalConditionEPK8CDungeon
    // DEMANGLED: CParty::checkFreeRevivalCondition(CDungeon const*)
    int checkFreeRevivalCondition(const CDungeon* dungeon);
    // IDA_EA: 0x85B27C8 / 0x85BE4AE / 0x85BE56C.
    int ReviveUser(CUser* user, uint16_t target_unique_id);
    int checkUnLimitUsedCoinCondition(const CDungeon* dungeon);
    int checkLimitUsedCoinCondition(const CDungeon* dungeon);
    // IDA_EA: 0x859AEC2
    // IDA_NAME: _ZN6CParty17battle_data_resetEv
    // DEMANGLED: CParty::battle_data_reset(void)
    int battle_data_reset();
    // IDA_EA: 0x85A932A
    // IDA_NAME: _ZN6CParty19check_passive_skillEv
    // DEMANGLED: CParty::check_passive_skill(void)
    void check_passive_skill();

    // IDA_EA: 0x859E528
    // IDA_NAME: _ZN6CParty24CheckEnterAncientDungeonEPK8CDungeonRhP10Inven_ItemPi
    // DEMANGLED: CParty::CheckEnterAncientDungeon(CDungeon const*,unsigned char &,Inven_Item *,int *)
    bool CheckEnterAncientDungeon(const CDungeon* dungeon,
                                  unsigned char& failed_member,
                                  Inven_Item* inventory_snapshots,
                                  int* inventory_slots);

    // IDA_EA: 0x859E7FE
    // IDA_NAME: _ZN6CParty24CheckEnterAncientDungeonEPK8CDungeon
    // DEMANGLED: CParty::CheckEnterAncientDungeon(CDungeon const*)
    bool CheckEnterAncientDungeon(const CDungeon* dungeon);

    // IDA_EA: 0x859EAC2
    // IDA_NAME: _ZN6CParty22UseAncientDungeonItemsEPK8CDungeonP10Inven_ItemPi
    // DEMANGLED: CParty::UseAncientDungeonItems(CDungeon const*,Inven_Item *,int *)
    int UseAncientDungeonItems(const CDungeon* dungeon,
                               Inven_Item* inventory_snapshots,
                               int* inventory_slots);

    // IDA_EA: 0x859EE66
    // IDA_NAME: _ZN6CParty22CheckEnterEventDungeonEPK8CDungeonRhP10Inven_ItemPi
    // DEMANGLED: CParty::CheckEnterEventDungeon(CDungeon const*,unsigned char &,Inven_Item *,int *)
    int CheckEnterEventDungeon(const CDungeon* dungeon,
                               unsigned char& failed_member,
                               Inven_Item* inventory_snapshots,
                               int* inventory_slots);

    // IDA_EA: 0x859F1CE
    // IDA_NAME: _ZN6CParty26CheckEnterDimensionDungeonEPK8CDungeonRhb
    // DEMANGLED: CParty::CheckEnterDimensionDungeon(CDungeon const*,unsigned char &,bool)
    int CheckEnterDimensionDungeon(const CDungeon* dungeon,
                                   unsigned char& failed_member,
                                   bool check_party_count);

    // IDA_EA: 0x859F36A
    // IDA_NAME: _ZN6CParty26CheckEnterDimensionDungeonEPK8CDungeonRhc
    // DEMANGLED: CParty::CheckEnterDimensionDungeon(CDungeon const*,unsigned char &,char)
    int CheckEnterDimensionDungeon(const CDungeon* dungeon,
                                   unsigned char& failed_member,
                                   char region);

    // IDA_EA: 0x859F3FC
    // IDA_NAME: _ZN6CParty26CheckEnterDimensionDungeonEPK8CDungeon
    // DEMANGLED: CParty::CheckEnterDimensionDungeon(CDungeon const*)
    int CheckEnterDimensionDungeon(const CDungeon* dungeon);

    // IDA_EA: 0x859F582
    // IDA_NAME: _ZN6CParty22getPartyMemberAvgLevelEv
    // DEMANGLED: CParty::getPartyMemberAvgLevel(void)
    int getPartyMemberAvgLevel();

    // IDA_EA: 0x859F612
    // IDA_NAME: _ZN6CParty25setStandardDimensionLevelEPK8CDungeon
    // DEMANGLED: CParty::setStandardDimensionLevel(CDungeon const*)
    int setStandardDimensionLevel(const CDungeon* dungeon);

    // IDA_EA: 0x859F6D4
    // IDA_NAME: _ZN6CParty25getStandardDimensionLevelEv
    // DEMANGLED: CParty::getStandardDimensionLevel(void)
    int getStandardDimensionLevel();

    // IDA_EA: 0x859F6E2
    // IDA_NAME: _ZN6CParty20checkDugeonInoutTimeEi
    // DEMANGLED: CParty::checkDugeonInoutTime(int)
    int checkDugeonInoutTime(int dungeon_index);

    // IDA_EA: 0x859BFE2
    // IDA_NAME: _ZN6CParty30increase_dimension_inout_countEi
    // DEMANGLED: CParty::increase_dimension_inout_count(int)
    char increase_dimension_inout_count(int value);

    // IDA_EA: 0x859F7FA
    // IDA_NAME: _ZN6CParty19check_dungeon_startEPK8CDungeonRi
    // DEMANGLED: CParty::check_dungeon_start(CDungeon const*,int &)
    int check_dungeon_start(const CDungeon* dungeon, int& failed_member);

    // IDA_EA: 0x859FB16
    // IDA_NAME: _ZN6CParty19check_dungeon_startEPK8CDungeon
    // DEMANGLED: CParty::check_dungeon_start(CDungeon const*)
    int check_dungeon_start(const CDungeon* dungeon);

    // IDA_EA: 0x85A0954
    // IDA_NAME: _ZN6CParty13dungeon_startEic17ENUM_DUNGEON_TYPE
    // DEMANGLED: CParty::dungeon_start(int,char,ENUM_DUNGEON_TYPE)
    //
    // CMD16 and RestartDungeon both enter the same stateful start transaction
    // through this member.  Keep the physical CParty identity instead of
    // replacing it with a dispatcher-local success path.
    int dungeon_start(int dungeon_index,
                      char dungeon_difficulty,
                      ENUM_DUNGEON_TYPE dungeon_mode);

    // IDA_EA: 0x85A2246
    // IDA_NAME: _ZN6CParty14RestartDungeonEv
    // DEMANGLED: CParty::RestartDungeon(void)
    int RestartDungeon();

    // Original CMD16 start-transaction leaves.  They stay on CParty because
    // their checks and state changes are coupled to the physical party
    // aggregate rather than to the packet dispatcher.
    int CheckHaveHellPartyPassItem(const CDungeon* dungeon,
                                   ENUM_DUNGEON_TYPE dungeon_mode);
    int _Send_TOD_APCInfo(int apc_index);
    int GetDungeonIndex_isTOD(int& dungeon_index, int& apc_index);
    // IDA_EA: 0x85BB62A
    // IDA_NAME: _ZN6CParty21SetProperDungeonEventEP5CUseri
    // DEMANGLED: CParty::SetProperDungeonEvent(CUser *,int)
    // The old body does not consume the forwarded dungeon index, but callers
    // pass it and the parameter is part of the binary ABI.
    int SetProperDungeonEvent(CUser* user, int dungeon_index);
    int SetProperLevelDungeon(CUser* user);
    void SendPartyMemberTagChracInfo();
    bool checkDungeonDifficulty(char dungeon_difficulty,
                                const CDungeon* dungeon);
    int isPremiumGoldCardReward(const CDungeon* dungeon,
                                unsigned int& item_id);

    // IDA_EA: 0x85A7D86
    // IDA_NAME: _ZN6CParty29CheckDestroyConditionSealDoorEPK8CDungeonRSt6vectorIiSaIiEES6_
    // DEMANGLED: CParty::CheckDestroyConditionSealDoor(CDungeon const*,std::vector<int,std::allocator<int>> &,std::vector<int,std::allocator<int>> &)
    int CheckDestroyConditionSealDoor(const CDungeon* dungeon,
                                      std::vector<int>& failed_members,
                                      std::vector<int>& seal_item_ids);

    // IDA_EA: 0x85A8038
    // IDA_NAME: _ZN6CParty16UseSealDoorItemsERSt6vectorIiSaIiEE
    // DEMANGLED: CParty::UseSealDoorItems(std::vector<int,std::allocator<int>> &)
    int UseSealDoorItems(std::vector<int>& seal_item_ids);

    // IDA_EA: 0x85A82D2
    // IDA_NAME: _ZN6CParty14IsClearedQuestEi
    // DEMANGLED: CParty::IsClearedQuest(int)
    int IsClearedQuest(int quest_index);

    // IDA_EA: 0x85A834A
    // IDA_NAME: _ZN6CParty19CheckHellCoinLimintEi
    // DEMANGLED: CParty::CheckHellCoinLimint(int)
    bool CheckHellCoinLimint(int limit);

    // IDA_EA: 0x822D8CE
    // IDA_NAME: _ZN6CParty21IncreaseUsedCoinCountEv
    // DEMANGLED: CParty::IncreaseUsedCoinCount(void)
    CParty* IncreaseUsedCoinCount();

    // IDA_EA: 0x822D8E8
    // IDA_NAME: _ZN6CParty16GetUsedCoinCountEv
    // DEMANGLED: CParty::GetUsedCoinCount(void)
    int GetUsedCoinCount();

    // IDA_EA: 0x85A8C88
    // IDA_NAME: _ZN6CParty12canFatigueUpEh
    // DEMANGLED: CParty::canFatigueUp(unsigned char)
    bool canFatigueUp(unsigned char state);

    // IDA_EA: 0x85BA804
    // IDA_NAME: _ZN6CParty30CheckEnterVillageAttackRevengeEPK8CDungeon
    // DEMANGLED: CParty::CheckEnterVillageAttackRevenge(CDungeon const*)
    int CheckEnterVillageAttackRevenge(const CDungeon* dungeon);

    // IDA_EA: 0x85BC86E
    // IDA_NAME: _ZN6CParty38CheckHellDungeonFreepassItemHaveAndDelEP5CUserb
    // DEMANGLED: CParty::CheckHellDungeonFreepassItemHaveAndDel(CUser *,bool)
    bool CheckHellDungeonFreepassItemHaveAndDel(CUser* user, bool consume);

    // IDA_EA: 0x85B2BAA
    // IDA_NAME: _ZN6CParty11giveup_gameEP5CUserbbb
    // DEMANGLED: CParty::giveup_game(CUser *,bool,bool,bool)
    void giveup_game(CUser* user,
                     bool apply_giveup_penalty,
                     bool clear_dungeon,
                     bool force_return);

    // IDA_EA: 0x859FCFC
    // IDA_NAME: _ZN6CParty22do_after_dungeon_startEPK8CDungeonP5CUser
    // DEMANGLED: CParty::do_after_dungeon_start(CDungeon const*,CUser *)
    int do_after_dungeon_start(const CDungeon* dungeon, CUser* user);
    // IDA_EA: 0x85A0756
    // IDA_NAME: _ZN6CParty40do_after_dungeon_start_checkndelete_itemERK12RequiredItemP5CUser
    // DEMANGLED: CParty::do_after_dungeon_start_checkndelete_item(RequiredItem const&,CUser *)
    int do_after_dungeon_start_checkndelete_item(const RequiredItem& item,
                                                 CUser* user);

    // IDA_EA: 0x85A63F4
    // IDA_NAME: _ZN6CParty9drop_itemEP5CUseriicii
    // DEMANGLED: CParty::drop_item(CUser *,int,int,char,int,int)
    int drop_item(CUser* user,
                  int pos_x,
                  int pos_y,
                  char item_space,
                  int slot,
                  int count);
    // IDA_EA: 0x85A73A6
    // IDA_NAME: _ZN6CParty12drop_item_gmEP5CUserj
    // DEMANGLED: CParty::drop_item_gm(CUser *,unsigned int)
    int drop_item_gm(CUser* user, unsigned int item_id);
    // 0828 CMD588 generated-item owner.  This is deliberately non-virtual:
    // the full-tree rebuild publishes a typed CParty boundary without adding
    // an object field or changing the recovered vtable/layout.
    int drop_item_generated(CUser* user,
                            unsigned int item_id,
                            int pos_x,
                            int pos_y);

    // IDA_EA: 0x85B5098
    // IDA_NAME: _ZN6CParty21SendDungeonCharacInfoEP5CUserb
    // DEMANGLED: CParty::SendDungeonCharacInfo(CUser *,bool)
    int SendDungeonCharacInfo(CUser* user, bool cleared);

    // IDA_EA: 0x85B5158
    // IDA_NAME: _ZN6CParty25SendDungeonStatisticPartyEbP5CUser
    // DEMANGLED: CParty::SendDungeonStatisticParty(bool,CUser *)
    int SendDungeonStatisticParty(bool cleared, CUser* user);

    // IDA_EA: 0x85B52A6
    // IDA_NAME: _ZN6CParty28SendDungeonStatisticPartyJobEbP5CUser
    // DEMANGLED: CParty::SendDungeonStatisticPartyJob(bool,CUser *)
    int SendDungeonStatisticPartyJob(bool cleared, CUser* user);

    // IDA_EA: 0x85B53AA
    // IDA_NAME: _ZN6CParty31SendDungeonStatisticPartyCharacEbP5CUser
    // DEMANGLED: CParty::SendDungeonStatisticPartyCharac(bool,CUser *)
    int SendDungeonStatisticPartyCharac(bool cleared, CUser* user);

    // IDA_EA: 0x85B5518
    // IDA_NAME: _ZN6CParty14OnDungeonClearEP5CUserb
    // DEMANGLED: CParty::OnDungeonClear(CUser *,bool)
    int OnDungeonClear(CUser* user, bool cleared);

    // IDA_EA: 0x85BF7DA
    // IDA_NAME: _ZN6CParty20OnFailedDungeonClearEP5CUser
    // DEMANGLED: CParty::OnFailedDungeonClear(CUser *)
    int OnFailedDungeonClear(CUser* user);

    // IDA_EA: 0x85BF7FC
    // IDA_NAME: _ZN6CParty23OnSuccessedDungeonClearEP5CUser
    // DEMANGLED: CParty::OnSuccessedDungeonClear(CUser *)
    int OnSuccessedDungeonClear(CUser* user);

    // IDA_EA: 0x85ACA0A
    // IDA_NAME: _ZN6CParty28sendPremiumGoldCardStatisticEiii
    // DEMANGLED: CParty::sendPremiumGoldCardStatistic(int,int,int)
    void sendPremiumGoldCardStatistic(int standard_level,
                                      int member_count,
                                      int selected_card_count);

    // IDA_EA: 0x85AAB84
    // IDA_NAME: _ZN6CParty16_putItemIntoUserEP5CUserP10Inven_ItemR11PacketGuard
    // DEMANGLED: CParty::_putItemIntoUser(CUser *,Inven_Item *,PacketGuard &)
    int _putItemIntoUser(CUser* user,
                         Inven_Item* reward_pair,
                         PacketGuard& packet);

    // IDA_EA: 0x85AD0BE
    // IDA_NAME: _ZN6CParty19_makeRewardItemInfoEPK5CUserP10Inven_ItemR11PacketGuard
    // DEMANGLED: CParty::_makeRewardItemInfo(CUser const*,Inven_Item *,PacketGuard &)
    // Serializes the one member's clear-reward card pair into CMD 35 and
    // performs the original eager inventory insertion for the item card.
    int _makeRewardItemInfo(const CUser* user,
                            Inven_Item* reward_pair,
                            PacketGuard& packet);

    // IDA_EA: 0x85AB736
    // IDA_NAME: _ZN6CParty13dungeon_clearEv
    // DEMANGLED: CParty::dungeon_clear(void)
    void dungeon_clear();

    // IDA_EA: 0x85B3670
    // IDA_NAME: _ZN6CParty18party_dungeon_failEv
    // DEMANGLED: CParty::party_dungeon_fail(void)
    // The all-members-dead completion path owns coin persistence, anti-hack
    // completion, per-member failure accounting and the final party update.
    void party_dungeon_fail();

    // IDA_EA: 0x85A9330 / 0x85AA39A
    // IDA_NAME: _ZN6CParty12ClearDungeonEv / _ZN6CParty21prepare_dungeon_clearEv
    void ClearDungeon();
    void prepare_dungeon_clear();

    // IDA_EA: 0x85AA5D4
    // IDA_NAME: _ZN6CParty16OnTimeoutBossDieEij
    // DEMANGLED: CParty::OnTimeoutBossDie(int,unsigned int)
    int OnTimeoutBossDie(int timer_key, unsigned int charac_no);

    // IDA_EA: 0x85AE0D4
    // IDA_NAME: _ZN6CParty12clear_rewardEi
    // DEMANGLED: CParty::clear_reward(int)
    // CMD049 prepares the three card-reward pairs before the card-selection
    // state machine consumes them through dungeon_clear().
    void clear_reward(int rank_index);

    // IDA_EA: 0x859BA98
    // IDA_NAME: _ZN6CParty25increase_result_card_goldERN8WongWork18stGenerateResult_tE
    // DEMANGLED: CParty::increase_result_card_gold(WongWork::stGenerateResult_t &)
    bool increase_result_card_gold(WongWork::stGenerateResult_t& result);

    // IDA_EA: 0x85B916E
    // IDA_NAME: _ZN6CParty33Reset_party_overlapped_drop_ratioEv
    // DEMANGLED: CParty::Reset_party_overlapped_drop_ratio(void)
    char Reset_party_overlapped_drop_ratio();

    // IDA_EA: 0x85BB0AC
    // IDA_NAME: _ZN6CParty24CheckHackAverageHitCountEv
    // DEMANGLED: CParty::CheckHackAverageHitCount(void)
    char CheckHackAverageHitCount();

    // IDA_EA: 0x859A974
    // IDA_NAME: _ZN6CParty4initEv
    // DEMANGLED: CParty::init(void)
    CParty* init();

    // IDA_EA: 0x8145780
    // IDA_NAME: _ZN6CParty10getManagerEv
    // DEMANGLED: CParty::getManager(void)
    CUser* getManager();

    // IDA_EA: 0x85B2068 / 0x85B2412
    // IDA_NAME: _ZN6CParty15set_play_resultEP5CUserR13GameResultSet /
    //           _ZN6CParty13SetPlayResultEP5CUser
    // DEMANGLED: CParty::set_play_result(CUser *,GameResultSet &) /
    //             CParty::SetPlayResult(CUser *)
    int set_play_result(CUser* manager, GameResultSet& result);
    int SetPlayResult(CUser* manager);
    // IDA_EA: 0x85B26E8
    // IDA_NAME: _ZN6CParty22forced_set_play_resultEv
    // DEMANGLED: CParty::forced_set_play_result(void)
    void forced_set_play_result();

    // IDA_EA: 0x85BE178 / 0x85BE1F8
    // IDA_NAME: _ZN6CParty16setBestClearTimeEiii /
    //           _ZN6CParty23makeBestClearTimePacketER11PacketGuardi
    bool setBestClearTime(int dungeon_index,
                          int dungeon_difficulty,
                          int clear_time);
    bool makeBestClearTimePacket(PacketGuard& packet, int clear_time);

    // IDA_EA: 0x8145764
    // IDA_NAME: _ZN6CParty8get_userEi
    // DEMANGLED: CParty::get_user(int)
    CUser* get_user(int seat);

    // IDA_EA: 0x85B65A2
    // IDA_NAME: _ZN6CParty26HasNoQuestGoWithApcInPartyEPK5Quest
    // DEMANGLED: CParty::HasNoQuestGoWithApcInParty(Quest const*)
    int HasNoQuestGoWithApcInParty(const Quest* quest);

    // IDA_EA: 0x85B4D12
    // IDA_NAME: _ZN6CParty15_checkValidUserEi
    // DEMANGLED: CParty::_checkValidUser(int)
    int _checkValidUser(int seat);

    // IDA_EA: 0x8145868
    // IDA_NAME: _ZN6CParty14checkValidUserEi
    // DEMANGLED: CParty::checkValidUser(int)
    int checkValidUser(int seat);

    // IDA_EA: 0x822D812
    // IDA_NAME: _ZN6CParty12IsSinglePlayEv
    // DEMANGLED: CParty::IsSinglePlay(void)
    int IsSinglePlay();

    // IDA_EA: 0x822D83A
    // IDA_NAME: _ZN6CParty9get_stateEv
    // DEMANGLED: CParty::get_state(void)
    int get_state();

    // IDA_EA: 0x822D846
    // IDA_NAME: _ZN6CParty13GetPartyIndexEv
    // DEMANGLED: CParty::GetPartyIndex(void)
    int GetPartyIndex();

    // IDA_EA: 0x82A59E6
    // IDA_NAME: _ZN6CParty8getTitleEv
    // DEMANGLED: CParty::getTitle(void)
    char* getTitle();

    // IDA_EA: 0x82A59F4
    // IDA_NAME: _ZN6CParty13getTitleIndexEv
    // DEMANGLED: CParty::getTitleIndex(void)
    int getTitleIndex();

    // IDA_EA: 0x82A5A04
    // IDA_NAME: _ZN6CParty10getUserMaxEv
    // DEMANGLED: CParty::getUserMax(void)
    int getUserMax();

    // IDA_EA: 0x82A5A14
    // IDA_NAME: _ZNK6CParty12getDungIndexEv
    // DEMANGLED: CParty::getDungIndex(void)const
    int getDungIndex() const;

    // IDA_EA: 0x82A5A24
    // IDA_NAME: _ZNK6CParty12getDungDiffiEv
    // DEMANGLED: CParty::getDungDiffi(void)const
    int getDungDiffi() const;

    // IDA_EA: 0x822D928
    // IDA_NAME: _ZN6CParty21get_quick_party_indexEv
    // DEMANGLED: CParty::get_quick_party_index(void)
    int get_quick_party_index();

    // IDA_EA: 0x822D936
    // IDA_NAME: _ZN6CParty15set_quick_partyEb
    // DEMANGLED: CParty::set_quick_party(bool)
    CParty* set_quick_party(bool quick_party);

    // IDA_EA: 0x822D952
    // IDA_NAME: _ZN6CParty14is_quick_partyEv
    // DEMANGLED: CParty::is_quick_party(void)
    int is_quick_party();

    // IDA_EA: 0x859B814
    // IDA_NAME: _ZN6CParty18choose_random_buffEN10QuickParty17RandomBuffDungeonE
    // DEMANGLED: CParty::choose_random_buff(QuickParty::RandomBuffDungeon)
    //
    // The dungeon-start paths for Death Tower, Boss Tower and normal maps
    // share this selection.  The return value and random_buff_type_ are the
    // same original 0..11 compact buff identity (11 means no selection).
    int choose_random_buff(QuickParty::RandomBuffDungeon dungeon_type);

    // IDA_EA: 0x859B8FA
    // IDA_NAME: _ZN6CParty17set_random_rewardEN10QuickParty14RandomBuffTypeENS0_16RewardData_ParamERNS0_17RewardData_ResultE
    // DEMANGLED: CParty::set_random_reward(QuickParty::RandomBuffType,
    //             QuickParty::RewardData_Param,
    //             QuickParty::RewardData_Result &)
    int set_random_reward(QuickParty::RandomBuffType reward_type,
                          QuickParty::RewardData_Param parameter,
                          QuickParty::RewardData_Result& result);

    // IDA_EA: 0x859B992
    // IDA_NAME: _ZN6CParty16set_basic_rewardEN10QuickParty15BasicRewardTypeENS0_16RewardData_ParamERNS0_17RewardData_ResultE
    // DEMANGLED: CParty::set_basic_reward(QuickParty::BasicRewardType,
    //             QuickParty::RewardData_Param,
    //             QuickParty::RewardData_Result &)
    char set_basic_reward(QuickParty::BasicRewardType reward_type,
                          QuickParty::RewardData_Param parameter,
                          QuickParty::RewardData_Result& result);

    // IDA_EA: 0x85B9F50
    // IDA_NAME: _ZN6CParty17IsReturnUserPartyEv
    // DEMANGLED: CParty::IsReturnUserParty(void)
    int IsReturnUserParty();

    // IDA_EA: 0x85B9FD4
    // IDA_NAME: _ZN6CParty18IsEventCharacPartyEv
    // DEMANGLED: CParty::IsEventCharacParty(void)
    int IsEventCharacParty();

    // IDA_EA: 0x826B978
    // IDA_NAME: _ZN6CParty9set_stateEc
    // DEMANGLED: CParty::set_state(char)
    CParty* set_state(char state);

    // IDA_EA: 0x826B9AC
    // IDA_NAME: _ZN6CParty21set_quick_party_indexEi
    // DEMANGLED: CParty::set_quick_party_index(int)
    CParty* set_quick_party_index(int quick_party_index);

    // IDA_EA: 0x859AC7C
    // IDA_NAME: _ZNK6CParty15GetMemberSlotNoEPK5CUser
    // DEMANGLED: CParty::GetMemberSlotNo(CUser const*) const
    int GetMemberSlotNo(const CUser* user) const;

    // IDA_EA: 0x859ACC2
    // IDA_NAME: _ZN6CParty15GetMemberSlotNoEj
    // DEMANGLED: CParty::GetMemberSlotNo(unsigned int)
    CUser* GetMemberSlotNo(unsigned int charac_no);

    // IDA_EA: 0x85A277A
    // IDA_NAME: _ZN6CParty16get_party_seatnoEP5CUser
    // DEMANGLED: CParty::get_party_seatno(CUser *)
    int get_party_seatno(CUser* user);

    // IDA_EA: 0x85B9E20
    // IDA_NAME: _ZN6CParty15GetUserPositionEP5CUser
    // DEMANGLED: CParty::GetUserPosition(CUser *)
    int GetUserPosition(CUser* user);

    // IDA_EA: 0x85B9E80
    // IDA_NAME: _ZN6CParty17GetUserSecretShopEP5CUser
    // DEMANGLED: CParty::GetUserSecretShop(CUser *)
    secretshop::SECRET_SHOP_INFO* GetUserSecretShop(CUser* user);

    // IDA_EA: 0x85B9ECC
    // IDA_NAME: _ZN6CParty22SendSecretShopItemListEP5CUser
    // DEMANGLED: CParty::SendSecretShopItemList(CUser *)
    void SendSecretShopItemList(CUser* user);

    // IDA_EA: 0x8694A1A
    // IDA_NAME: _ZN6CParty17GetSecretShopDataEv
    // DEMANGLED: CParty::GetSecretShopData(void)
    secretshop::SECRET_SHOP_DATA* GetSecretShopData();

    // IDA_EA: 0x859A16A
    // IDA_NAME: _ZN6CParty16get_member_countEv
    // DEMANGLED: CParty::get_member_count(void)
    int get_member_count();

    // IDA_EA: 0x859D14E
    // IDA_NAME: _ZN6CParty13send_to_partyER11PacketGuard
    // DEMANGLED: CParty::send_to_party(PacketGuard &)
    bool send_to_party(PacketGuard& packet);

    // IDA_EA: 0x85BA5D6
    // IDA_NAME: _ZN6CParty20send_to_alive_memberER11PacketGuard
    // DEMANGLED: CParty::send_to_alive_member(PacketGuard &)
    bool send_to_alive_member(PacketGuard& packet);

    // IDA_EA: 0x85B4AE2
    // IDA_NAME: _ZN6CParty18_getMemberLevelGapEv
    // DEMANGLED: CParty::_getMemberLevelGap(void)
    int _getMemberLevelGap();

    // IDA_EA: 0x85B624E
    // IDA_NAME: _ZN6CParty16IsExistCharacterEi
    // DEMANGLED: CParty::IsExistCharacter(int)
    int IsExistCharacter(int charac_no);

    // IDA_EA: 0x85B62C0
    // IDA_NAME: _ZN6CParty12GetPartyTypeEv
    // DEMANGLED: CParty::GetPartyType(void)
    int GetPartyType();

    // IDA_EA: 0x85B6410
    // IDA_NAME: _ZN6CParty16IsExistInvisibleEv
    // DEMANGLED: CParty::IsExistInvisible(void)
    int IsExistInvisible();


    // IDA_EA: 0x859C114
    // IDA_NAME: _ZN6CParty10leave_userEP5CUser20ENUM_PARTY_INFO_TYPE
    // DEMANGLED: CParty::leave_user(CUser *,ENUM_PARTY_INFO_TYPE)
    int leave_user(CUser* user, ENUM_PARTY_INFO_TYPE info_type);

    // IDA_EA: 0x859CA4A
    // IDA_NAME: _ZN6CParty14walkout_memberEP5CUseri
    // DEMANGLED: CParty::walkout_member(CUser *,int)
    int walkout_member(CUser* user, int seat);

    // IDA_EA: 0x859CB00
    // IDA_NAME: _ZN6CParty11SendWalkoutEi17ENUM_WALKOUT_TYPE
    // DEMANGLED: CParty::SendWalkout(int,ENUM_WALKOUT_TYPE)
    int SendWalkout(int seat, ENUM_WALKOUT_TYPE type);

    // IDA_EA: 0x85B3E12
    // IDA_NAME: _ZN6CParty22OnTimerWalkOutLackUserEv
    // DEMANGLED: CParty::OnTimerWalkOutLackUser(void)
    int OnTimerWalkOutLackUser();

    // IDA_EA: 0x85B3C90
    // IDA_NAME: _ZN6CParty12VoteLackUserEP5CUseri
    // DEMANGLED: CParty::VoteLackUser(CUser *,int)
    int VoteLackUser(CUser* voter, int target_seat);

    // IDA_EA: 0x859D5E8
    // IDA_NAME: _ZN6CParty18CheckMemberFatigueEv
    // DEMANGLED: CParty::CheckMemberFatigue(void)
    int CheckMemberFatigue();

    // IDA_EA: 0x859E0D2
    // IDA_NAME: _ZN6CParty23make_blood_dungeon_infoEP5CUserR11PacketGuard
    // DEMANGLED: CParty::make_blood_dungeon_info(CUser *,PacketGuard &)
    int make_blood_dungeon_info(CUser* manager, PacketGuard& packet);

    // IDA_EA: 0x85B7DB6
    // IDA_NAME: _ZN6CParty22historyBloodRoundClearEijjPKc
    // DEMANGLED: CParty::historyBloodRoundClear(int,unsigned int,unsigned int,char const*)
    bool historyBloodRoundClear(int blood_round,
                                unsigned int round_seconds,
                                unsigned int total_play_seconds,
                                const char* dungeon_name);

    // IDA_EA: 0x85B8BF8
    // IDA_NAME: _ZN6CParty22BloodClearRewardPacketEiiiPK10Inven_ItemiS2_iS2_iS2_
    // DEMANGLED: CParty::BloodClearRewardPacket(
    //     int,int,int,Inven_Item const*,int,Inven_Item const*,int,
    //     Inven_Item const*,int,Inven_Item const*)
    void BloodClearRewardPacket(int member_slot,
                                int current_blood_round,
                                int first_item_count,
                                const Inven_Item* first_items,
                                int second_item_count,
                                const Inven_Item* second_items,
                                int third_item_count,
                                const Inven_Item* third_items,
                                int fourth_item_count,
                                const Inven_Item* fourth_items);

    // IDA_EA: 0x85B6A48
    // IDA_NAME: _ZN6CParty18onBloodRewardTimerE13TIMER_MESSAGE
    // DEMANGLED: CParty::onBloodRewardTimer(TIMER_MESSAGE)
    int onBloodRewardTimer(TIMER_MESSAGE message);

    // IDA_EA: 0x85B75D4
    // IDA_NAME: _ZN6CParty22beginBloodClearProcessEb
    // DEMANGLED: CParty::beginBloodClearProcess(bool)
    char beginBloodClearProcess(bool blood_clear);

    // IDA_EA: 0x85B7C9E
    // IDA_NAME: _ZN6CParty12onBloodClearEbi
    // DEMANGLED: CParty::onBloodClear(bool,int)
    int onBloodClear(bool blood_clear, int member_slot);

    // IDA_EA: 0x85B7EA2
    // IDA_NAME: _ZN6CParty22generateBloodRewardExpEiijs
    // DEMANGLED: CParty::generateBloodRewardExp(int,int,unsigned int,short)
    int generateBloodRewardExp(int member_slot,
                               int blood_max_round,
                               unsigned int base_exp,
                               short inout_count);

    // IDA_EA: 0x85B840E
    // IDA_NAME: _ZN6CParty23generateBloodRewardItemEiijjb
    // DEMANGLED: CParty::generateBloodRewardItem(int,int,unsigned int,
    //                                              unsigned int,bool)
    void generateBloodRewardItem(int member_slot,
                                 int item_count,
                                 unsigned int character_level,
                                 unsigned int reward_probability_scale,
                                 bool blood_clear);

    // IDA_EA: 0x85B8BAE
    // IDA_NAME: _ZN6CParty23checkBloodRenewMyRecordEiiii
    // DEMANGLED: CParty::checkBloodRenewMyRecord(int,int,int,int)
    int checkBloodRenewMyRecord(int member_slot,
                                int play_time,
                                int blood_max_round,
                                int dungeon_index);

    // IDA_EA: 0x859E480
    // IDA_NAME: _ZN6CParty29make_blood_dungeon_info_blankER11PacketGuard
    // DEMANGLED: CParty::make_blood_dungeon_info_blank(PacketGuard &)
    int make_blood_dungeon_info_blank(PacketGuard& packet);

    // IDA_EA: 0x85A7A72
    // IDA_NAME: _ZN6CParty24CheckClearQuestHellPartyERSt6vectorIiSaIiEE
    // DEMANGLED: CParty::CheckClearQuestHellParty(std::vector<int,std::allocator<int>> &)
    bool CheckClearQuestHellParty(std::vector<int>& quests);

    // IDA_EA: 0x85AC59C
    // IDA_NAME: _ZN6CParty26checkInoutConditionDungeonEv
    // DEMANGLED: CParty::checkInoutConditionDungeon(void)
    int checkInoutConditionDungeon();

    // IDA_EA: 0x85ABC80
    // IDA_NAME: _ZN6CParty26checkInoutConditionDungeonEPK8CDungeon17ENUM_DUNGEON_MODE14ENUM_CMDPACKET
    // DEMANGLED: CParty::checkInoutConditionDungeon(
    //     CDungeon const*,ENUM_DUNGEON_MODE,ENUM_CMDPACKET)
    int checkInoutConditionDungeon(const CDungeon* dungeon,
                                   ENUM_DUNGEON_MODE mode,
                                   ENUM_CMDPACKET command);

    // IDA_EA: 0x85AC95E
    // IDA_NAME: _ZN6CParty25sendInoutConditionDungeonEN15RetryFailReason1TE
    // DEMANGLED: CParty::sendInoutConditionDungeon(RetryFailReason::T)
    void sendInoutConditionDungeon(RetryFailReason::T reason);

    // IDA_EA: 0x85ACA60
    // IDA_NAME: _ZN6CParty15ReturnToVillageEv
    // DEMANGLED: CParty::ReturnToVillage(void)
    void ReturnToVillage();

    // IDA_EA: 0x85B1FE0
    // IDA_NAME: _ZN6CParty21send_loadingcheck_msgEv
    // DEMANGLED: CParty::send_loadingcheck_msg(void)
    int send_loadingcheck_msg();
    // IDA_EA: 0x85B1E66
    // IDA_NAME: _ZN6CParty22map_load_forced_finishEv
    // DEMANGLED: CParty::map_load_forced_finish(void)
    void map_load_forced_finish();
    // IDA_EA: 0x85B15E0 / 0x85B170A / 0x85B17A4
    // IDA_NAME: _ZN6CParty14finish_loadingEP5CUser /
    //           _ZN6CParty23OnStartMapFinishLoadingEP5CUser /
    //           _ZN6CParty23check_allmember_loadingEb
    int finish_loading(CUser* user);
    int OnStartMapFinishLoading(CUser* user);
    void check_allmember_loading(bool enter_each_map);
    // IDA_EA: 0x859B73E
    // IDA_NAME: _ZN6CParty32send_quick_party_matching_resultEbP5CUser
    // DEMANGLED: CParty::send_quick_party_matching_result(bool,CUser *)
    void send_quick_party_matching_result(bool success, CUser* user);
    // IDA_EA: 0x859C832
    // IDA_NAME: _ZN6CParty14change_managerEv
    // DEMANGLED: CParty::change_manager(void)
    int change_manager();
    // IDA_EA: 0x859CBAC
    // IDA_NAME: _ZN6CParty23get_party_realtime_infoER11PacketGuard
    // DEMANGLED: CParty::get_party_realtime_info(PacketGuard &)
    int get_party_realtime_info(PacketGuard& packet);
    // IDA_EA: 0x859CD24
    // IDA_NAME: _ZN6CParty24send_party_realtime_infoEv
    // DEMANGLED: CParty::send_party_realtime_info(void)
    void send_party_realtime_info();
    // IDA_EA: 0x859CEA2
    // IDA_NAME: _ZN6CParty17send_party_ipinfoEv
    // DEMANGLED: CParty::send_party_ipinfo(void)
    void send_party_ipinfo();
    // IDA_EA: 0x859CDBC
    // IDA_NAME: _ZN6CParty8set_hostEv
    // DEMANGLED: CParty::set_host(void)
    int set_host();
    // IDA_EA: 0x859CDD2
    // IDA_NAME: _ZN6CParty14send_host_infoEv
    // DEMANGLED: CParty::send_host_info(void)
    void send_host_info();
    // IDA_EA: 0x85A21CA
    // IDA_NAME: _ZN6CParty16isHelpAbusePartyEP5CUser
    // DEMANGLED: CParty::isHelpAbuseParty(CUser *)
    int isHelpAbuseParty(CUser* user);
    // IDA_EA: 0x85A21D4
    // IDA_NAME: _ZN6CParty16isHelpAbusePartyEv
    // DEMANGLED: CParty::isHelpAbuseParty(void)
    int isHelpAbuseParty();
    // IDA_EA: 0x859D718
    // IDA_NAME: _ZN6CParty10game_startEP5CUser
    // DEMANGLED: CParty::game_start(CUser *)
    void game_start(CUser* manager);
    // Phase-2 Zombie owner adapter.  It preserves the singleton party's
    // battle-data initialization, goto-dungeon transition, NOTI3 state and
    // NOTI27 dungeon-selection context, while suppressing the ordinary
    // singleton NOTI2 actor snapshot and NOTI26 host election.  It is
    // intentionally non-virtual and adds no object state.
    bool game_start_zombie_owner(CUser* manager);
    // IDA_EA: 0x85B9596
    // IDA_NAME: _ZN6CParty21OnFightVillageMonsterEP5CUseri
    // DEMANGLED: CParty::OnFightVillageMonster(CUser *,int)
    int OnFightVillageMonster(CUser* user, int monster_index);
    // IDA_EA: 0x85B99AC
    // IDA_NAME: _ZN6CParty30OnFinishVillageMonsterFightingEP5CUserb
    // DEMANGLED: CParty::OnFinishVillageMonsterFighting(CUser *,bool)
    int OnFinishVillageMonsterFighting(CUser* user, bool timeout);
    // IDA_EA: 0x85B6768
    // IDA_NAME: _ZN6CParty17GetMapPlayingTimeEv
    // DEMANGLED: CParty::GetMapPlayingTime(void)
    uint64_t GetMapPlayingTime();
    // IDA_EA: 0x85A83A2
    // IDA_NAME: _ZN6CParty19checkMoveMapHackCntER12MSG_MOVE_MAP
    // DEMANGLED: CParty::checkMoveMapHackCnt(MSG_MOVE_MAP &)
    int checkMoveMapHackCnt(MSG_MOVE_MAP& message);
    // IDA_EA: 0x85A8D1E
    // IDA_NAME: _ZN6CParty21changePartyMemberInfoER12MSG_MOVE_MAPc
    // DEMANGLED: CParty::changePartyMemberInfo(MSG_MOVE_MAP &,char)
    int changePartyMemberInfo(MSG_MOVE_MAP& message, char state);
    // IDA_EA: 0x85A8E0E
    // IDA_NAME: _ZN6CParty21resetFinishLoadingVarEv
    // DEMANGLED: CParty::resetFinishLoadingVar(void)
    bool resetFinishLoadingVar();
    // IDA_EA: 0x85A8E3C
    // IDA_NAME: _ZN6CParty30changePartyMemberInfoOnMoveMapER12MSG_MOVE_MAPb
    // DEMANGLED: CParty::changePartyMemberInfoOnMoveMap(MSG_MOVE_MAP &,bool)
    char changePartyMemberInfoOnMoveMap(MSG_MOVE_MAP& message, bool layered);
    // IDA_EA: 0x85A8EE0
    // IDA_NAME: _ZN6CParty25changeBattleDataOnMoveMapEb
    // DEMANGLED: CParty::changeBattleDataOnMoveMap(bool)
    void* changeBattleDataOnMoveMap(bool layered);
    // IDA_EA: 0x85A8F4E
    // IDA_NAME: _ZN6CParty8move_mapER12MSG_MOVE_MAP
    // DEMANGLED: CParty::move_map(MSG_MOVE_MAP &)
    int move_map(MSG_MOVE_MAP& message);
    // IDA_EA: 0x85BDC76
    // IDA_NAME: _ZN6CParty22getDungeonDropTestFlagEv
    // DEMANGLED: CParty::getDungeonDropTestFlag(void)
    int getDungeonDropTestFlag();
    // IDA_EA: 0x85BF930
    // IDA_NAME: _ZN6CParty19getDungeonMapSavingEv
    // DEMANGLED: CParty::getDungeonMapSaving(void)
    int getDungeonMapSaving();

    // IDA_EA: 0x85BDD2A
    // IDA_NAME: _ZN6CParty18getDungeonTypeKindEPK8CDungeon
    // DEMANGLED: CParty::getDungeonTypeKind(CDungeon const*)
    int getDungeonTypeKind(const CDungeon* dungeon);

    // IDA_EA: 0x8151A70
    // IDA_NAME: _ZN6CParty17getMemberLevelGapEv
    // DEMANGLED: CParty::getMemberLevelGap(void)
    int getMemberLevelGap();
    // IDA_EA: 0x822D81E
    // IDA_NAME: _ZN6CParty17SetRecvResultFlagEb
    // DEMANGLED: CParty::SetRecvResultFlag(bool)
    CParty* SetRecvResultFlag(bool received);
    // IDA_EA: 0x822D852
    // IDA_NAME: _ZN6CParty12setDungIndexEt
    // DEMANGLED: CParty::setDungIndex(unsigned short)
    CParty* setDungIndex(unsigned short dungeon_index);
    // IDA_EA: 0x822D870
    // IDA_NAME: _ZN6CParty12setDungDiffiEh
    // DEMANGLED: CParty::setDungDiffi(unsigned char)
    CParty* setDungDiffi(unsigned char dungeon_difficulty);
    // IDA_EA: 0x822D88C
    // IDA_NAME: _ZN6CParty13IsAutoCreatedEv
    // DEMANGLED: CParty::IsAutoCreated(void)
    int IsAutoCreated();
    // IDA_EA: 0x822D89C
    // IDA_NAME: _ZN6CParty23get_dungeon_clear_stateEv
    // DEMANGLED: CParty::get_dungeon_clear_state(void)
    int get_dungeon_clear_state();
    // IDA_EA: 0x822D8F6
    // IDA_NAME: _ZN6CParty13checkBossRoomEv
    // DEMANGLED: CParty::checkBossRoom(void)
    bool checkBossRoom();
    // IDA_EA: 0x822D962
    // IDA_NAME: _ZN6CParty23set_gm_random_buff_typeEN10QuickParty14RandomBuffTypeE
    // DEMANGLED: CParty::set_gm_random_buff_type(QuickParty::RandomBuffType)
    CParty* set_gm_random_buff_type(QuickParty::RandomBuffType buff_type);
    // IDA_EA: 0x826525E
    // IDA_NAME: _ZN6CParty14checkStartRoomEv
    // DEMANGLED: CParty::checkStartRoom(void)
    bool checkStartRoom();
    // IDA_EA: 0x826B960
    // IDA_NAME: _ZN6CParty13SetSinglePlayEb
    // DEMANGLED: CParty::SetSinglePlay(bool)
    CParty* SetSinglePlay(bool single_play);
    // IDA_EA: 0x826B990
    // IDA_NAME: _ZN6CParty14SetAutoCreatedEb
    // DEMANGLED: CParty::SetAutoCreated(bool)
    CParty* SetAutoCreated(bool auto_created);
    // IDA_EA: 0x827AB3A
    // IDA_NAME: _ZN6CParty15IsFirstMapClearEv
    // DEMANGLED: CParty::IsFirstMapClear(void)
    int IsFirstMapClear();
    // IDA_EA: 0x827AB4A
    // IDA_NAME: _ZN6CParty16SetFirstMapClearEb
    // DEMANGLED: CParty::SetFirstMapClear(bool)
    CParty* SetFirstMapClear(bool first_map_clear);
    // IDA_EA: 0x82F07A4
    // IDA_NAME: _ZN6CParty11IsExistUserEP5CUser
    // DEMANGLED: CParty::IsExistUser(CUser *)
    int IsExistUser(CUser* user);
    // IDA_EA: 0x82F07EC
    // IDA_NAME: _ZN6CParty20getStraightVictoriesEv
    // DEMANGLED: CParty::getStraightVictories(void)
    int getStraightVictories();
    // IDA_EA: 0x82F07FC
    // IDA_NAME: _ZN6CParty20incStraightVictoriesEv
    // DEMANGLED: CParty::incStraightVictories(void)
    CParty* incStraightVictories();
    // IDA_EA: 0x82F0818
    // IDA_NAME: _ZN6CParty22resetStraightVictoriesEv
    // DEMANGLED: CParty::resetStraightVictories(void)
    CParty* resetStraightVictories();
    // IDA_EA: 0x830ED3A
    // IDA_NAME: _ZN6CParty20get_random_buff_typeEv
    // DEMANGLED: CParty::get_random_buff_type(void)
    int get_random_buff_type();
    // IDA_EA: 0x84B426E
    // IDA_NAME: _ZN6CParty20setStraightVictoriesEt
    // DEMANGLED: CParty::setStraightVictories(unsigned short)
    CParty* setStraightVictories(unsigned short victories);
    // IDA_EA: 0x84B428C
    // IDA_NAME: _ZN6CParty19setDungeonMapSavingEb
    // DEMANGLED: CParty::setDungeonMapSaving(bool)
    CParty* setDungeonMapSaving(bool saving);
    // IDA_EA: 0x8550FEE
    // IDA_NAME: _ZNK6CParty36Get_party_overlapped_drop_ratio_rareEv
    // DEMANGLED: CParty::Get_party_overlapped_drop_ratio_rare(void)const
    int Get_party_overlapped_drop_ratio_rare() const;
    // IDA_EA: 0x8550FFC
    // IDA_NAME: _ZNK6CParty38Get_party_overlapped_drop_ratio_uniqueEv
    // DEMANGLED: CParty::Get_party_overlapped_drop_ratio_unique(void)const
    int Get_party_overlapped_drop_ratio_unique() const;
    // IDA_EA: 0x859A1AE
    // IDA_NAME: _ZN6CParty15get_total_levelEv
    // DEMANGLED: CParty::get_total_level(void)
    int get_total_level();
    // IDA_EA: 0x859A318
    // IDA_NAME: _ZN6CParty28CanDoPartyActionAssalutStateEv
    // DEMANGLED: CParty::CanDoPartyActionAssalutState(void)
    bool CanDoPartyActionAssalutState();
    // IDA_EA: 0x859A8BC
    // IDA_NAME: _ZN6CParty14get_live_countEv
    // DEMANGLED: CParty::get_live_count(void)
    int get_live_count();
    // IDA_EA: 0x859A918
    // IDA_NAME: _ZN6CParty24get_live_count_enter_mapEv
    // DEMANGLED: CParty::get_live_count_enter_map(void)
    int get_live_count_enter_map();
    // IDA_EA: 0x85A2320
    // IDA_NAME: _ZNK6CParty20GetExpPenaltyPercentEii
    // DEMANGLED: CParty::GetExpPenaltyPercent(int,int)const
    int GetExpPenaltyPercent(int character_level, int monster_level) const;
    // IDA_EA: 0x85AD9B4
    // IDA_NAME: _ZN6CParty25calDeadUserDungeonPenaltyEi
    // DEMANGLED: CParty::calDeadUserDungeonPenalty(int)
    long double calDeadUserDungeonPenalty(int seat);
    // IDA_EA: 0x85ADA30
    // IDA_NAME: _ZN6CParty20CalcurateManageBonusEN15ManageBonusType1TEP5CUserj
    // DEMANGLED: CParty::CalcurateManageBonus(ManageBonusType::T,CUser *,unsigned int)
    long double CalcurateManageBonus(ManageBonusType::T bonus_type,
                                     CUser* user,
                                     unsigned int base_value);
    // IDA_EA: 0x85ADB00
    // IDA_NAME: _ZN6CParty30set_clear_reward_exp_parameterEP5CUserjR27stClearRewardExpParameter_t
    // DEMANGLED: CParty::set_clear_reward_exp_parameter(CUser *,unsigned int,stClearRewardExpParameter_t &)
    int set_clear_reward_exp_parameter(
        CUser* user,
        unsigned int base_exp,
        stClearRewardExpParameter_t& parameter);
    // IDA_EA: 0x85B12F8 / 0x85BDDFC
    // IDA_NAME: _ZN6CParty17DungeonPermissionEi /
    //           _ZN6CParty15addDungeonClearEPK8CDungeonii
    char DungeonPermission(int rank_index);
    int addDungeonClear(const CDungeon* dungeon,
                        int rank_index,
                        int member_slot);
    // IDA_EA: 0x85B231E
    // IDA_NAME: _ZN6CParty21GetSetPlayResultCountEv
    // DEMANGLED: CParty::GetSetPlayResultCount(void)
    int GetSetPlayResultCount();
    // IDA_EA: 0x85B401C
    // IDA_NAME: _ZN6CParty23HaveAlreadySelectedCardEc22eClearRewardCardType_t
    // DEMANGLED: CParty::HaveAlreadySelectedCard(char,eClearRewardCardType_t)
    int HaveAlreadySelectedCard(char card,
                                eClearRewardCardType_t card_type);
    // IDA_EA: 0x85B407C
    // IDA_NAME: _ZN6CParty24_checkGoldCardSelectableEP5CUser22eClearRewardCardType_ti
    // DEMANGLED: CParty::_checkGoldCardSelectable(CUser *,eClearRewardCardType_t,int)
    int _checkGoldCardSelectable(CUser* user,
                                 eClearRewardCardType_t card_type,
                                 int member_slot);
    // IDA_EA: 0x85B415A
    // IDA_NAME: _ZN6CParty13SetCardNumberEP5CUserc22eClearRewardCardType_t
    // DEMANGLED: CParty::SetCardNumber(CUser *,char,eClearRewardCardType_t)
    int SetCardNumber(CUser* user,
                      char card,
                      eClearRewardCardType_t card_type);
    // Shared native type-1 post-selection owner.  ChampionBreak skips only
    // the paid-card fee and must retain inventory preflight/reward clearing
    // plus the action-point notifications before dungeon_clear grants items.
    void ApplyGoldCardSelectionSideEffectsForRuntime(CUser* user,
                                                      int member_slot);
    // IDA_EA: 0x85B44D6
    // IDA_NAME: _ZN6CParty16SendEachCardInfoEv
    // DEMANGLED: CParty::SendEachCardInfo(void)
    void SendEachCardInfo();
    // IDA_EA: 0x85B4734
    // IDA_NAME: _ZN6CParty12IsSelectCardEc22eClearRewardCardType_t
    // DEMANGLED: CParty::IsSelectCard(char,eClearRewardCardType_t)
    int IsSelectCard(char card, eClearRewardCardType_t card_type);
    // IDA_EA: 0x85B4786
    // IDA_NAME: _ZN6CParty18CompleteCardSelectEv
    // DEMANGLED: CParty::CompleteCardSelect(void)
    bool CompleteCardSelect();
    // IDA_EA: 0x85B489C
    // IDA_NAME: _ZN6CParty29IsCompleteCardSelectOfMembersE22eClearRewardCardType_t
    // DEMANGLED: CParty::IsCompleteCardSelectOfMembers(eClearRewardCardType_t)
    int IsCompleteCardSelectOfMembers(eClearRewardCardType_t card_type);
    // IDA_EA: 0x85BB5B0
    // IDA_NAME: _ZN6CParty15isCheckUseSkillEii
    // DEMANGLED: CParty::isCheckUseSkill(int,int)
    bool isCheckUseSkill(int character_job, int skill_index);
    // IDA_EA: 0x85A77D8 / 0x85B5F8E
    // IDA_NAME: _ZN6CParty9use_skillEP5CUserii /
    //           _ZN6CParty11OnUsedSkillEP5CUserii
    // DEMANGLED: CParty::use_skill(CUser *,int,int) /
    //            CParty::OnUsedSkill(CUser *,int,int)
    int use_skill(CUser* user, int skill_slot, int skill_index);
    int OnUsedSkill(CUser* user, int skill_slot, int skill_index);
    // IDA_EA: 0x85BCA8A
    // IDA_NAME: _ZN6CParty15SetWeekendEventEb
    // DEMANGLED: CParty::SetWeekendEvent(bool)
    CParty* SetWeekendEvent(bool active);
    // IDA_EA: 0x85BCAA6
    // IDA_NAME: _ZN6CParty14IsWeekendEventEv
    // DEMANGLED: CParty::IsWeekendEvent(void)
    int IsWeekendEvent();
    // IDA_EA: 0x85BD6AC
    // IDA_NAME: _ZN6CParty30GetTournamentDungeonClearStateEv
    // DEMANGLED: CParty::GetTournamentDungeonClearState(void)
    int GetTournamentDungeonClearState();
    // IDA_EA: 0x85BD79E
    // IDA_NAME: _ZN6CParty19isTournamentVictoryEv
    // DEMANGLED: CParty::isTournamentVictory(void)
    int isTournamentVictory();
    // IDA_EA: 0x85BE54C
    // IDA_NAME: _ZN6CParty23GetPartyMemberCoinLimitEv
    // DEMANGLED: CParty::GetPartyMemberCoinLimit(void)
    int GetPartyMemberCoinLimit();
    // IDA_EA: 0x85BE55A
    // IDA_NAME: _ZN6CParty23SetPartyMemberCoinLimitEi
    // DEMANGLED: CParty::SetPartyMemberCoinLimit(int)
    CParty* SetPartyMemberCoinLimit(int coin_limit);
    // IDA_EA: 0x85BF6F8
    // IDA_NAME: _ZN6CParty22IsPremiumGoldCardPartyEv
    // DEMANGLED: CParty::IsPremiumGoldCardParty(void)
    int IsPremiumGoldCardParty();
    // IDA_EA: 0x85BF708
    // IDA_NAME: _ZN6CParty23SetPremiumGoldCardPartyEv
    // DEMANGLED: CParty::SetPremiumGoldCardParty(void)
    CParty* SetPremiumGoldCardParty();
    // IDA_EA: 0x85BF728
    // IDA_NAME: _ZN6CParty29GetPremiumGoldCardDefaultItemEv
    // DEMANGLED: CParty::GetPremiumGoldCardDefaultItem(void)
    int GetPremiumGoldCardDefaultItem();
    // IDA_EA: 0x85BF736
    // IDA_NAME: _ZN6CParty29SetPremiumGoldCardDefaultItemEj
    // DEMANGLED: CParty::SetPremiumGoldCardDefaultItem(unsigned int)
    CParty* SetPremiumGoldCardDefaultItem(unsigned int item_id);
    // IDA_EA: 0x85BF75A
    // IDA_NAME: _ZN6CParty22setStartGamePartyCountEi
    // DEMANGLED: CParty::setStartGamePartyCount(int)
    CParty* setStartGamePartyCount(int member_count);
    // IDA_EA: 0x85BF76C
    // IDA_NAME: _ZN6CParty22getStartGamePartyCountEv
    // DEMANGLED: CParty::getStartGamePartyCount(void)
    int getStartGamePartyCount();
    // IDA_EA: 0x85BF7A2
    // IDA_NAME: _ZN6CParty13setTitleIndexEh
    // DEMANGLED: CParty::setTitleIndex(unsigned char)
    CParty* setTitleIndex(unsigned char title_index);
    // IDA_EA: 0x85BF7BE
    // IDA_NAME: _ZN6CParty10setUserMaxEh
    // DEMANGLED: CParty::setUserMax(unsigned char)
    CParty* setUserMax(unsigned char user_max);
    // IDA_EA: 0x85BF81E
    // IDA_NAME: _ZN6CParty15GetAssaultStateEv
    // DEMANGLED: CParty::GetAssaultState(void)
    int GetAssaultState();
    // IDA_EA: 0x85BF83E
    // IDA_NAME: _ZN6CParty16SetUsedCoinCountEi
    // DEMANGLED: CParty::SetUsedCoinCount(int)
    CParty* SetUsedCoinCount(int used_coin_count);
    // IDA_EA: 0x860B5EE
    // IDA_NAME: _ZN6CParty16GetRoutingItemIDEv
    // DEMANGLED: CParty::GetRoutingItemID(void)
    int GetRoutingItemID();

    // IDA_EA: 0x85BA294
    // IDA_NAME: _ZN6CParty24GetRoutingDiceMemberListERSt6vectorISt4pairIiiESaIS2_EEP5CItem
    // DEMANGLED: CParty::GetRoutingDiceMemberList(std::vector<std::pair<int,int>> &,CItem *)
    char GetRoutingDiceMemberList(std::vector<std::pair<int, int>>& members,
                                  CItem* item);

    // IDA_EA: 0x860B62A
    // IDA_NAME: _ZN6CParty21DeleteDungeonDropItemEi
    // DEMANGLED: CParty::DeleteDungeonDropItem(int)
    int DeleteDungeonDropItem(int unique_id);

    // IDA_EA: 0x822D8AC
    // IDA_NAME: _ZN6CParty23set_dungeon_clear_stateEc
    // DEMANGLED: CParty::set_dungeon_clear_state(char)
    CParty* set_dungeon_clear_state(char state);

    // IDA_EA: 0x859A286
    // IDA_NAME: _ZN6CParty22is_limit_minimum_levelEiRi
    // DEMANGLED: CParty::is_limit_minimum_level(int,int &)
    int is_limit_minimum_level(int minimum_level, int& unique_id);

    // IDA_EA: 0x859A48C
    // IDA_NAME: _ZN6CParty10ConnectP2PEP5CUserPci
    // DEMANGLED: CParty::ConnectP2P(CUser *,char *,int)
    char ConnectP2P(CUser* user, char* states, int count);

    // IDA_EA: 0x859A610
    // IDA_NAME: _ZN6CParty19_IsCannotConnectP2PEii
    // DEMANGLED: CParty::_IsCannotConnectP2P(int,int)
    bool _IsCannotConnectP2P(int seat, int peer_seat);

    // IDA_EA: 0x859A664
    // IDA_NAME: _ZN6CParty24_ClearConnectP2PAbsoluteEi
    // DEMANGLED: CParty::_ClearConnectP2PAbsolute(int)
    int _ClearConnectP2PAbsolute(int seat);

    // IDA_EA: 0x859A6DC
    // IDA_NAME: _ZN6CParty24_ClearConnectP2PRelativeEi
    // DEMANGLED: CParty::_ClearConnectP2PRelative(int)
    int _ClearConnectP2PRelative(int connection_count);

    // IDA_EA: 0x859A74C
    // IDA_NAME: _ZN6CParty19_GetConnectP2PPointEi
    // DEMANGLED: CParty::_GetConnectP2PPoint(int)
    int _GetConnectP2PPoint(int seat);

    // IDA_EA: 0x859A7A2
    // IDA_NAME: _ZN6CParty25_GetConnectP2PMaxMinPointERiS0_
    // DEMANGLED: CParty::_GetConnectP2PMaxMinPoint(int &,int &)
    bool _GetConnectP2PMaxMinPoint(int& maximum, int& minimum);

    // IDA_EA: 0x859B1BE
    // IDA_NAME: _ZN6CParty12create_partyEP5CUser
    // DEMANGLED: CParty::create_party(CUser *)
    int create_party(CUser* manager);

    // IDA_EA: 0x859C978
    // IDA_NAME: _ZN6CParty14change_managerEP5CUseri
    // DEMANGLED: CParty::change_manager(CUser *,int)
    int change_manager(CUser* requester, int seat);

    // IDA_EA: 0x859D258
    // IDA_NAME: _ZN6CParty15send_invite_pvpEi
    // DEMANGLED: CParty::send_invite_pvp(int)
    void send_invite_pvp(int unique_id);

    // IDA_EA: 0x859D388
    // IDA_NAME: _ZN6CParty19send_invite_warroomEi
    // DEMANGLED: CParty::send_invite_warroom(int)
    void send_invite_warroom(int unique_id);

    // IDA_EA: 0x859D4B8
    // IDA_NAME: _ZN6CParty22send_invite_bluemarbleEi
    // DEMANGLED: CParty::send_invite_bluemarble(int)
    void send_invite_bluemarble(int unique_id);

    // IDA_EA: 0x859D656
    // IDA_NAME: _ZN6CParty15CheckMemberAreaEv
    // DEMANGLED: CParty::CheckMemberArea(void)
    int CheckMemberArea();

    // IDA_EA: 0x85A2140
    // IDA_NAME: _ZN6CParty16FindUserByCharIdEj
    // DEMANGLED: CParty::FindUserByCharId(unsigned int)
    CUser* FindUserByCharId(unsigned int charac_no);

    // IDA_EA: 0x85AD084
    // IDA_NAME: _ZN6CParty17CalcCreatureBonusEP5CUser
    // DEMANGLED: CParty::CalcCreatureBonus(CUser *)
    long double CalcCreatureBonus(CUser* user);

    // IDA_EA: 0x85ACFDE
    // IDA_NAME: _ZN6CParty15CalcAvatarBonusEv
    // DEMANGLED: CParty::CalcAvatarBonus(void)
    long double CalcAvatarBonus();
    // IDA_EA: 0x814A9C2
    // IDA_NAME: _ZN6CParty14getAvatarBonusEv
    // DEMANGLED: CParty::getAvatarBonus(void)
    long double getAvatarBonus();
    // IDA_EA: 0x814A9E2
    // IDA_NAME: _ZN6CParty16getCreatureBonusEP5CUser
    // DEMANGLED: CParty::getCreatureBonus(CUser *)
    long double getCreatureBonus(CUser* user);
    // IDA_EA: 0x85AD278
    // IDA_NAME: _ZN6CParty22getClearRewardBonusExpEP5CUserRjRK27stClearRewardExpParameter_tR23stClearRewardBonusExp_t
    // DEMANGLED: CParty::getClearRewardBonusExp(CUser *,unsigned int &,stClearRewardExpParameter_t const&,stClearRewardBonusExp_t &)
    unsigned int getClearRewardBonusExp(
        CUser* user,
        unsigned int& base_exp,
        const stClearRewardExpParameter_t& parameter,
        stClearRewardBonusExp_t& bonus_exp);

    // IDA_EA: 0x85B357C
    // IDA_NAME: _ZN6CParty12GetCoinCountEP5CUser
    // DEMANGLED: CParty::GetCoinCount(CUser *)
    int GetCoinCount(CUser* user);

    // IDA_EA: 0x85B3F34
    // IDA_NAME: _ZN6CParty26set_recv_party_packet_flagEP5CUser
    // DEMANGLED: CParty::set_recv_party_packet_flag(CUser *)
    int set_recv_party_packet_flag(CUser* user);

    // IDA_EA: 0x85B3F72
    // IDA_NAME: _ZN6CParty21recv_party_packet_allEv
    // DEMANGLED: CParty::recv_party_packet_all(void)
    int recv_party_packet_all();

    // IDA_EA: 0x859A34E
    // IDA_NAME: _ZN6CParty27TimeoutCompleteAfterAssaultEv
    // DEMANGLED: CParty::TimeoutCompleteAfterAssault(void)
    int TimeoutCompleteAfterAssault();

    // IDA_EA: 0x85B3FD8
    // IDA_NAME: _ZN6CParty22CheckRecvFromAllMemberEP5CUser
    // DEMANGLED: CParty::CheckRecvFromAllMember(CUser *)
    int CheckRecvFromAllMember(CUser* user);

    // IDA_EA: 0x85B49D0
    // IDA_NAME: _ZN6CParty12isGuildPartyEv
    // DEMANGLED: CParty::isGuildParty(void)
    int isGuildParty();

    // IDA_EA: 0x85B4A70
    // IDA_NAME: _ZN6CParty15hasPCRoomPlayerEv
    // DEMANGLED: CParty::hasPCRoomPlayer(void)
    int hasPCRoomPlayer();

    // IDA_EA: 0x85B603E
    // IDA_NAME: _ZN6CParty17getMemberMinLevelEv
    // DEMANGLED: CParty::getMemberMinLevel(void)
    int getMemberMinLevel();

    // IDA_EA: 0x85B60BE
    // IDA_NAME: _ZN6CParty17getMemberMaxLevelEv
    // DEMANGLED: CParty::getMemberMaxLevel(void)
    int getMemberMaxLevel();

    // IDA_EA: 0x85B613E
    // IDA_NAME: _ZN6CParty30getMemberMaxLevelBeforeDungeonEv
    // DEMANGLED: CParty::getMemberMaxLevelBeforeDungeon(void)
    int getMemberMaxLevelBeforeDungeon();

    // IDA_EA: 0x85B61BE
    // IDA_NAME: _ZN6CParty12GetLuckPointEP10CLuckPointb
    // DEMANGLED: CParty::GetLuckPoint(CLuckPoint *,bool)
    bool GetLuckPoint(CLuckPoint* luck_point, bool live_members_only);

    // IDA_EA: 0x85B67CA
    // IDA_NAME: _ZN6CParty19existSamePCRoomUserEj
    // DEMANGLED: CParty::existSamePCRoomUser(unsigned int)
    int existSamePCRoomUser(unsigned int pc_room_no);

    // IDA_EA: 0x85B687C
    // IDA_NAME: _ZN6CParty17isSamePCRoomPartyEv
    // DEMANGLED: CParty::isSamePCRoomParty(void)
    int isSamePCRoomParty();

    // IDA_EA: 0x85B8F92
    // IDA_NAME: _ZN6CParty18finishBloodDungeonEv
    // DEMANGLED: CParty::finishBloodDungeon(void)
    void finishBloodDungeon();

    // IDA_EA: 0x85A3B6A
    // IDA_NAME: _ZN6CParty18kill_blood_monsterEP5CUseriPt
    // DEMANGLED: CParty::kill_blood_monster(CUser *,int,unsigned short *)
    int kill_blood_monster(CUser* user,
                           int value,
                           unsigned short* result);

    // IDA_EA: 0x85B8FA6
    // IDA_NAME: _ZN6CParty19IsThereGhostInPartyEv
    // DEMANGLED: CParty::IsThereGhostInParty(void)
    int IsThereGhostInParty();

    // IDA_EA: 0x85B900C
    // IDA_NAME: _ZN6CParty18getMemberPassPartyEP5CUser
    // DEMANGLED: CParty::getMemberPassParty(CUser *)
    bool getMemberPassParty(CUser* user);

    // IDA_EA: 0x85B9F28
    // IDA_NAME: _ZN6CParty21SetRidableObjectStateEi19eRidableObjectState
    // DEMANGLED: CParty::SetRidableObjectState(int,eRidableObjectState)
    int SetRidableObjectState(int object_id,
                              eRidableObjectState requested_state);

    // IDA_EA: 0x85BA150
    // IDA_NAME: _ZN6CParty21GetMemberRoutingStateEP5CUser
    // DEMANGLED: CParty::GetMemberRoutingState(CUser *)
    int GetMemberRoutingState(CUser* user);

    // IDA_EA: 0x85BA1C0
    // IDA_NAME: _ZN6CParty21HasAllMemberSelectionEv
    // DEMANGLED: CParty::HasAllMemberSelection(void)
    int HasAllMemberSelection();

    // IDA_EA: 0x85BA222
    // IDA_NAME: _ZN6CParty19SetRoutingTimerOverEv
    // DEMANGLED: CParty::SetRoutingTimerOver(void)
    bool SetRoutingTimerOver();

    // IDA_EA: 0x85BA680
    // IDA_NAME: _ZN6CParty13switchPostionEcc
    // DEMANGLED: CParty::switchPostion(char,char)
    int switchPostion(char first, char second);

    // IDA_EA: 0x85BCAB6
    // IDA_NAME: _ZN6CParty29CheckMemberLastTryAssaultTimeEv
    // DEMANGLED: CParty::CheckMemberLastTryAssaultTime(void)
    int CheckMemberLastTryAssaultTime();

    // IDA_EA: 0x85BD6BC
    // IDA_NAME: _ZN6CParty30SetTournamentDungeonClearStateEc
    // DEMANGLED: CParty::SetTournamentDungeonClearState(char)
    CParty* SetTournamentDungeonClearState(char state);

    // IDA_EA: 0x85BCB40
    // IDA_NAME: _ZN6CParty20MakeTournamentRewardER24CTournamentDungeonReward
    // DEMANGLED: CParty::MakeTournamentReward(CTournamentDungeonReward &)
    void MakeTournamentReward(CTournamentDungeonReward& reward);

    // IDA_EA: 0x85BD7AE
    // IDA_NAME: _ZN6CParty21MakeQuestRewardPacketEP11PacketGuardP5CUser
    // DEMANGLED: CParty::MakeQuestRewardPacket(PacketGuard *,CUser *)
    char MakeQuestRewardPacket(PacketGuard* packet, CUser* user);

    // IDA_EA: 0x85BD236
    // IDA_NAME: _ZN6CParty22TournamentDungeonClearEb
    // DEMANGLED: CParty::TournamentDungeonClear(bool)
    void TournamentDungeonClear(bool tournament_victory);

    // IDA_EA: 0x85BF6E0
    // IDA_NAME: _ZN6CParty15reset_recv_flagEv
    // DEMANGLED: CParty::reset_recv_flag(void)
    void* reset_recv_flag();

    // Batch 028: canonical world/dungeon party state-machine surface.
    std::vector<MapInfo>* getPassedMapInfoList();
    int is_limit_minimum_age(int minimum_age, int& unique_id);
    char ClearBadP2PUser();
    CParty* SetPartyInfoUI(const MSG_PARTY_SET_INFO& message);
    int increase_dungeon_coin_count(int count);
    char increase_ancient_dungeon_ticket();
    int SelectEPLPCommand(int delay_sec);
    int getMonsterTotalExp(unsigned int base_exp,
                           char rank,
                           int level_index,
                           float level_weight,
                           float dungeon_weight,
                           bool triple_exp);
    // IDA_EA: 0x859D70E
    // IDA_NAME: _ZN6CParty17MemberPenaltyRateEv
    // DEMANGLED: CParty::MemberPenaltyRate(void)
    int MemberPenaltyRate();
    // IDA_EA: 0x85A2488
    // IDA_NAME: _ZN6CParty16sumGainedEachExpEP5CUserRK19STGainedEachExpData
    // DEMANGLED: CParty::sumGainedEachExp(CUser *,
    //             STGainedEachExpData const&)
    int sumGainedEachExp(CUser* user,
                         const STGainedEachExpData& gained_exp);
    int insert_time_dungeon_clear_msg(int delay_sec);
    int insert_time_set_play_result_msg(int delay_sec);
    int insert_time_card_select_msg(int delay_sec);
    int insert_time_clear_dungeon_reward_msg(int delay_sec);
    int insert_time_dungeon_score_scroll_msg(int delay_sec);
    int insert_time_card_select_right_msg(int delay_sec);
    int IncreaseSetPlayResultCount(CUser* user);
    const char* GetNoSendSetPlayResultUser(int seat);
    int send_dungeon_fail_msg();
    void SendCardSelectComplete();
    // Runtime bridge for the 0828 ChampionBreak composite CMD71 packet.  This
    // is read-only and preserves the original selected_cards_ owner/layout.
    int GetSelectedCardOwnerForRuntime(int card_type, int card) const;
    // ChampionBreak uses the ordinary type-1/gold row without charging the
    // legacy paid-gold-card fee.  Recording the owner in the native array
    // keeps CParty::dungeon_clear as the sole base-reward producer.
    int SetChampionCardNumberForRuntime(CUser* user, char card);
    // IDA_EA: 0x85B58AC
    // IDA_NAME: _ZN6CParty19cal_hellparty_valueEiii
    // DEMANGLED: CParty::cal_hellparty_value(int,int,int)
    int cal_hellparty_value(int dungeon_difficulty,
                            int level_difference_rate,
                            int other_member_count);
    int get_rate_level_difference();
    int sub_hellparty_value();
    int CheckQuestConnection(int quest_index,
                             ENUM_MAZE_QUEST_TYPE quest_type);
    bool increase_hell_party_ratio(char* weights);
    void send_hellParty_value();
    char enumPartyMember(bool (*callback)(CUser*));
    char SetMemberRoutingState(CUser* user, char state);
    bool GetValidPartyMember(std::vector<int>& members);
    int GetPartyMemberForceDropFlag();
    bool resetMemberUseCoin();
    int IsUseFatigueDungeon();
    // IDA_EA: 0x85BEA4A
    // IDA_NAME: _ZN6CParty13CheckMapClearEv
    // DEMANGLED: CParty::CheckMapClear(void)
    int CheckMapClear();
    char* setTitle(const char* title);
    CParty* ClearMapHitCount();
    bool SetStartRouting();
    CItemRoutingData* SetRoutingItem(Inven_Item& item,
                                     unsigned int unique_id);
    Inven_Item* GetRoutingInvenItem();

    // IDA_EA: 0x85A7800
    // IDA_NAME: _ZN6CParty7useItemEP5CUserRK10Inven_Item
    // DEMANGLED: CParty::useItem(CUser *,Inven_Item const&)
    MapInfo* useItem(CUser* user, const Inven_Item& item);

    // Typed CParty subobjects used by non-member old-server consumers.  They
    // expose the physical members directly; they are not raw offset bridges.
    CBattle_Field& battle_field() { return battle_field_; }
    const CBattle_Field& battle_field() const { return battle_field_; }
    BattleData& battle_data() { return battle_data_; }
    const BattleData& battle_data() const { return battle_data_; }
    GameResultSet& game_result_set() { return game_result_set_; }
    const GameResultSet& game_result_set() const { return game_result_set_; }
    Secu_HackLogCheckByParty& secu() { return secu_; }
    const Secu_HackLogCheckByParty& secu() const { return secu_; }
    std::vector<MapInfo>& passed_map_info_list() {
        return passed_map_info_list_;
    }
    const std::vector<MapInfo>& passed_map_info_list() const {
        return passed_map_info_list_;
    }

private:
    CItemRoutingData routing_data_;                   // +0x0000
    Mutex mutex_;                                     // +0x004c
    int party_index_;                                 // +0x0064
    uint8_t member_count_;                            // +0x0068
    uint8_t single_play_;                             // +0x0069
    uint8_t state_;                                   // +0x006a
    uint8_t eplp_state_;                              // +0x006b
    uint8_t selected_eplp_command_;                   // +0x006c
    uint8_t assault_state_;                           // +0x006d
    uint8_t pad_6e_6f_[2];
    // CParty::set_host copies manager_ here and set_play_result requires
    // this owner to match the caller before it commits CMD049.
    CUser* host_;                                     // +0x0070
    CUser* manager_;                                  // +0x0074
    cMember members_[4];                              // +0x0078
    cElection<int, 4, 4> election_;                   // +0x00d8
    uint8_t recv_result_flag_;                       // +0x0118
    uint8_t loading_check_pending_;                  // +0x0119, CParty::send_loadingcheck_msg
    uint8_t tournament_victory_;                     // +0x011a
    uint8_t title_index_;                            // +0x011b
    char title_[0x20];                               // +0x011c, CParty::getTitle
    uint8_t auto_created_;                           // +0x013c, CParty::destroy
    uint8_t user_max_;                               // +0x013d, CParty::getUserMax
    uint16_t dungeon_index_;                         // +0x013e, CParty::getDungIndex
    uint8_t dungeon_difficulty_;                     // +0x0140, CParty::getDungDiffi
    uint8_t unrecovered_141_143_[3];
    // CParty::game_start chooses the member whose loading packet seeds the
    // random dungeon state and stores that seat here.
    uint32_t random_member_index_;                    // +0x0144
    // +0x0148. CParty::CParty clears one dword for each TIMER_MESSAGE 11..47.
    int timer_keys_[37];
    CPartyResultRecvFlag result_recv_flag_;          // +0x01dc
    uint8_t dungeon_clear_state_;                    // +0x01e0
    uint8_t pad_1e1_1e3_[3];
    int play_result_values_[2][4];                   // +0x01e4
    uint8_t selected_cards_[2][4];                   // +0x0204
    uint8_t tournament_dungeon_clear_state_;         // +0x020c
    uint8_t pad_20d_20f_[3];
    CTraceMobDieHack trace_mob_die_hack_;             // +0x0210
    int member_level_gap_;                            // +0x0298
    uint16_t straight_victories_;                    // +0x029c
    uint8_t pad_29e_29f_[2];
    int start_game_party_count_;                     // +0x02a0
    secretshop::SECRET_SHOP_DATA secret_shop_data_;   // +0x02a4
    uint8_t premium_gold_card_party_;                 // +0x031c
    uint8_t pad_31d_31f_[3];
    uint32_t premium_gold_card_default_item_;         // +0x0320
    uint8_t first_map_clear_;                        // +0x0324
    uint8_t pad_325_327_[3];
    int party_member_coin_limit_;                    // +0x0328
    BattleData battle_data_;                          // +0x032c
    CBattle_Field battle_field_;                      // +0x0b24
    GameResultSet game_result_set_;                   // +0x17f4
    std::map<int, int> map_int_int_;                  // +0x1844
    // Original game_start timestamp used by clear-time and anti-hack paths.
    uint32_t game_start_sec_;                         // +0x185c
    uint32_t used_coin_count_;                       // +0x1860
    // Set when a Hell-party entry has no uncleared prerequisite quest; the
    // client receives the corresponding one-byte notice in CMD16.
    uint8_t hell_quest_notice_;                       // +0x1864
    // CParty::CheckHaveHellPartyPassItem writes this exact byte at +0x1865;
    // CParty::dungeon_start consumes it before deleting Hell-door items.
    uint8_t hell_dungeon_seal_door_ready_;           // +0x1865
    uint8_t unrecovered_1866_1867_[2];
    CWorldMap* world_map_;                           // +0x1868
    int party_overlapped_drop_ratio_rare_;           // +0x186c
    int party_overlapped_drop_ratio_unique_;         // +0x1870
    CPartyMapHitStatistics map_hit_statistics_;      // +0x1874
    // CParty::OnUsedSkill increments this only for the job/skill pairs
    // accepted by isCheckUseSkill.  CParty::checkMoveMapHackCnt later
    // persists it to ETC 132 and resets it, proving it is a real per-seat
    // counter rather than spare padding.
    int skill_quest_use_counts_[4];          // +0x1aa4
    // Cleared together with the map-hit statistic aggregate by
    // ClearMapHitCount.  Keep the physical scalar named until its writer is
    // recovered; do not access it through CParty+0x1ab4.
    int map_hit_auxiliary_count_;                 // +0x1ab4
    int hit_time_divisor_;                         // +0x1ab8
    Secu_HackLogCheckByParty secu_;                   // +0x1abc
    CPartyTelePort party_teleport_;                   // +0x1ad0
    int quick_party_index_;                           // +0x1ae0
    uint8_t quick_party_;                             // +0x1ae4
    uint8_t pad_1ae5_1ae7_[3];
    int random_buff_type_;                           // +0x1ae8
    int gm_random_buff_type_;                        // +0x1aec
    uint8_t weekend_event_;                          // +0x1af0
    uint8_t dungeon_map_saving_;                     // +0x1af1
    uint8_t pad_1af2_1af3_[2];
    std::vector<MapInfo> passed_map_info_list_;       // +0x1af4

    friend class CPartyTelePort;
    // CMD459 addresses the original embedded reward object through the
    // CParty aggregate.  This friend preserves that physical relationship
    // without an offset helper or an invented public facade.
    friend class Dispatcher_TournamentRewardSelect;
    // CBattle_Field::check_random_appear_hell_dungeon owns the original
    // Hell-map eligibility decision but reads these physical party members.
    friend class CBattle_Field;
    friend class CUser;
    // CMD18 reads the original CParty+0xcd8 coin-limit selector as part of
    // its Death-Tower exercise-mode bypass.  Keep that physical read in the
    // packet owner rather than introducing an offset or policy surrogate.
    friend class DisPatcher_DeleteItem;
    // The embedded security checker reads this concrete CParty-owned map
    // statistics object.  Friendship preserves that original aggregate
    // relationship without a helper ABI that did not exist in df_game_r.
    friend class Secu_DungeonAverageCheck;
    friend struct CPartyLayoutAssert;
};

struct CPartyLayoutAssert {
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
    static_assert(sizeof(CParty::CItemRoutingData) == 0x4cu,
                  "CParty::CItemRoutingData IDA ABI size must stay 0x4c");
    static_assert(sizeof(CParty::cMember) == 0x18u,
                  "CParty::cMember IDA ABI size must stay 0x18");
    static_assert(offsetof(CParty::cMember, p2p_received) == 0x0cu,
                  "CParty::cMember P2P receive flag must stay at +0x0c");
    static_assert(offsetof(CParty::cMember, p2p_connectivity) == 0x0eu,
                  "CParty::cMember P2P matrix must stay at +0x0e");
    static_assert(offsetof(CParty::cMember, used_coin_count) == 0x14u,
                  "CParty::cMember used-coin counter must stay at +0x14");
    static_assert(offsetof(CParty, party_index_) == 0x64u,
                  "CParty::party_index_ IDA offset mismatch");
    static_assert(offsetof(CParty, manager_) == 0x74u,
                  "CParty::manager_ IDA offset mismatch");
    static_assert(offsetof(CParty, members_) == 0x78u,
                  "CParty::members_ IDA offset mismatch");
    static_assert(offsetof(CParty, election_) == 0xd8u,
                  "CParty::election_ IDA offset mismatch");
    static_assert(offsetof(CParty, title_) == 0x11cu,
                  "CParty::title_ IDA offset mismatch");
    static_assert(offsetof(CParty, timer_keys_) == 0x148u,
                  "CParty::timer_keys_ IDA offset mismatch");
    static_assert(offsetof(CParty, random_member_index_) == 0x144u,
                  "CParty::random_member_index_ IDA offset mismatch");
    static_assert(offsetof(CParty, result_recv_flag_) == 0x1dcu,
                  "CParty::result_recv_flag_ IDA offset mismatch");
    static_assert(offsetof(CParty, trace_mob_die_hack_) == 0x210u,
                  "CParty::trace_mob_die_hack_ IDA offset mismatch");
    static_assert(offsetof(CParty, secret_shop_data_) == 0x2a4u,
                  "CParty::secret_shop_data_ IDA offset mismatch");
    static_assert(offsetof(CParty, battle_data_) == 0x32cu,
                  "CParty::battle_data_ IDA offset mismatch");
    static_assert(offsetof(CParty, battle_field_) == 0xb24u,
                  "CParty::battle_field_ IDA offset mismatch");
    static_assert(offsetof(CParty, game_result_set_) == 0x17f4u,
                  "CParty::game_result_set_ IDA offset mismatch");
    static_assert(offsetof(CParty, map_int_int_) == 0x1844u,
                  "CParty::map_int_int_ IDA offset mismatch");
    static_assert(offsetof(CParty, game_start_sec_) == 0x185cu,
                  "CParty::game_start_sec_ IDA offset mismatch");
    static_assert(offsetof(CParty, hell_quest_notice_) == 0x1864u,
                  "CParty::hell_quest_notice_ IDA offset mismatch");
    static_assert(offsetof(CParty, party_teleport_) == 0x1ad0u,
                  "CParty::party_teleport_ IDA offset mismatch");
    static_assert(offsetof(CParty, recv_result_flag_) == 0x118u,
                  "CParty::recv_result_flag_ IDA offset mismatch");
    static_assert(offsetof(CParty, dungeon_clear_state_) == 0x1e0u,
                  "CParty::dungeon_clear_state_ IDA offset mismatch");
    static_assert(offsetof(CParty, selected_cards_) == 0x204u,
                  "CParty::selected_cards_ IDA offset mismatch");
    static_assert(offsetof(CParty, straight_victories_) == 0x29cu,
                  "CParty::straight_victories_ IDA offset mismatch");
    static_assert(offsetof(CParty, first_map_clear_) == 0x324u,
                  "CParty::first_map_clear_ IDA offset mismatch");
    static_assert(offsetof(CParty, used_coin_count_) == 0x1860u,
                  "CParty::used_coin_count_ IDA offset mismatch");
    static_assert(offsetof(CParty, map_hit_statistics_) == 0x1874u,
                  "CParty::map_hit_statistics_ IDA offset mismatch");
    static_assert(offsetof(CParty, random_buff_type_) == 0x1ae8u,
                  "CParty::random_buff_type_ IDA offset mismatch");
    static_assert(offsetof(CParty, weekend_event_) == 0x1af0u,
                  "CParty::weekend_event_ IDA offset mismatch");
    static_assert(offsetof(CParty, quick_party_index_) == 0x1ae0u,
                  "CParty::quick_party_index_ IDA offset mismatch");
    static_assert(offsetof(CParty, secu_) == 0x1abcu,
                  "CParty::secu_ IDA offset mismatch");
    static_assert(offsetof(CParty, hit_time_divisor_) == 0x1ab8u,
                  "CParty::hit_time_divisor_ IDA offset mismatch");
    static_assert(offsetof(CParty, map_hit_auxiliary_count_) == 0x1ab4u,
                  "CParty::map-hit auxiliary count IDA offset mismatch");
    static_assert(offsetof(CParty, passed_map_info_list_) == 0x1af4u,
                  "CParty::passed_map_info_list_ IDA offset mismatch");
    static_assert(sizeof(CParty) == 0x1b00u,
                  "CParty IDA ABI size must stay 0x1b00");
#endif
};

static_assert(sizeof(CPartyResultRecvFlag) == 0x04u,
              "CPartyResultRecvFlag IDA ABI size must stay 0x04");


#endif  // OLDSRV_OLD_GAME_GROUP_PARTY_H_
