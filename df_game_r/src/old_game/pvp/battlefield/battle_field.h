#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_BATTLE_FIELD_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_BATTLE_FIELD_H_

#include <stddef.h>
#include <stdint.h>

#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <ext/hash_map>

#include "old_data/PVF/dungeon/maze_script.h"
#include "old_data/PVF/map/map_script_types.h"

#include "common/manager/active_static_info_abi.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/dungeon/instance/condition_level_chk_dungeon.h"
#include "old_game/activity/reward/tournament_reward_type.h"
#include "old_game/social/party/hell_party_group_types.h"
#include "old_game/world/region/world_area_types.h"
#include "old_game/world/region/world_value_types.h"
#include "infra/network/packet/message_base.h"
#include "infra/network/packet/stream.h"
#include "infra/concurrent/thread/mutex.h"

class CUser;
class DisPatcher_DeleteItem;
class CParty;
class PvP_Room;
class PacketGuard;
class CDungeon;
class CWorldMap;
class CMap;
class Dispatcher_BossDieCheck;
class TimerBloodPhaseRegister0;
class TimerBloodPhaseRegister1;
class TimerBloodPhaseRegister2;
class TimerBloodPhaseRegister3;
class TimerBloodPhaseRegister4;
class TimerBloodPhaseRegister5;
class TimerBloodPhaseRegister6;
class TimerBloodPhaseRegister7;
class TimerBloodPhaseRegister8;
class TimerBloodPhaseRegister9;
struct TournamentRewardRate;
struct TournamentRewardItemRate;
struct stDeathTowerRecordMemberInfo_t;
namespace WongWork {
struct stGenerateRefData_t;
struct stGenerateResult_t;
}
enum ENUM_DUNGEON_DIFFICULTY : int;
// IDA mangled-name evidence: ENUM_DUNGEON_MODE is passed by value in
// CBattle_Field::check_random_appear_hell_dungeon at 0x830A862.  Its
// enumerators have not yet been recovered, but the original i386 ABI is the
// verified int-sized opaque enum declaration used by the other public call
// boundaries (CParty and cUserHistoryLog).
enum ENUM_DUNGEON_MODE : int;
enum TIMER_MESSAGE : int;
struct MazeScript;
struct stDungeonAssignItem_t;
namespace QuickParty {
enum RandomBuffDungeon : int;
}
// IDA unit: 99_misc_global/CBossDungeonEntranceLog.  This map-derived
// aggregation object is global (not nested in WongWork): its original names
// begin _ZN23CBossDungeonEntranceLog, and CBossTower embeds it at +0xb00.
class CBossDungeonEntranceLog
    : public std::map<uint32_t, STBossDungeonTowerLog> {
public:
    // IDA_EA: 0x81456C2 / 0x8145704 / 0x8145746
    // IDA_NAME: _ZN23CBossDungeonEntranceLogC2Ev /
    //           _ZN23CBossDungeonEntranceLogD2Ev /
    //           _ZN23CBossDungeonEntranceLog5resetEv
    CBossDungeonEntranceLog();
    ~CBossDungeonEntranceLog();
    CBossDungeonEntranceLog* reset();

    // IDA_EA: 0x81564F0
    // IDA_NAME: _ZN23CBossDungeonEntranceLog23IncrementBossStageStartER5CUserhh
    int IncrementBossStageStart(CUser& user,
                                uint8_t stage,
                                uint8_t member_count);
    // IDA_EA: 0x815662E
    // IDA_NAME: _ZN23CBossDungeonEntranceLog21IncrementBossStageEndER5CUserhjh
    int IncrementBossStageEnd(CUser& user,
                              uint8_t stage,
                              unsigned int elapsed_seconds,
                              uint8_t member_count);
    // IDA_EA: 0x815678E
    // IDA_NAME: _ZN23CBossDungeonEntranceLog23IncrementBossPickUpItemER5CUserhh
    int IncrementBossPickUpItem(CUser& user,
                                uint8_t stage,
                                uint8_t member_count);
    // IDA_EA: 0x81568CC
    // IDA_NAME: _ZN23CBossDungeonEntranceLog20BossTowerLogDBUpdateEv
    // DEMANGLED: CBossDungeonEntranceLog::BossTowerLogDBUpdate(void)
    void BossTowerLogDBUpdate();
};

namespace WongWork {
class CDeathTower;
}

class Dispatcher_DeathTowerStageCommand;

// IDA dispatcher stack layout at 0x8150388: MSG_BASE is 0x0d bytes and the
// monster-death payload follows immediately.  These are the fields consumed
// by CBossStage and cUserHistoryLog.
#pragma pack(push, 1)
struct MSG_MONSTER_DIE : MSG_BASE {
    uint16_t monster_id_;                 // +0x0d
    uint16_t monster_type_;               // +0x0f
    int32_t field_11_;                    // +0x11
    int32_t field_15_;                    // +0x15
    uint8_t entry_count_;                 // +0x19
    uint16_t entry_u16_a_[256];           // +0x1a
    uint16_t entry_u16_b_[256];           // +0x21a
    uint32_t entry_u32_[256];             // +0x41a
    uint16_t entry_u16_c_[256];           // +0x81a
    uint32_t field_a1a_;                  // +0xa1a
    uint32_t field_a1e_;                  // +0xa1e
    uint8_t field_a22_;                   // +0xa22
    uint8_t field_a23_;                   // +0xa23
    uint16_t field_a24_;                  // +0xa24
    uint16_t field_a26_;                  // +0xa26
    int8_t field_a28_;                    // +0xa28
    int16_t field_a29_;                   // +0xa29
    int16_t field_a2b_;                   // +0xa2b
    int32_t field_a2d_;                   // +0xa2d
    int32_t field_a31_;                   // +0xa31
    int16_t field_a35_;                   // +0xa35
    int16_t field_a37_;                   // +0xa37
    int16_t field_a39_;                   // +0xa39
    int16_t field_a3b_;                   // +0xa3b
    int16_t field_a3d_;                   // +0xa3d
};
#pragma pack(pop)

static_assert(sizeof(MSG_MONSTER_DIE) == 0xa3fu,
              "MSG_MONSTER_DIE IDA ABI size must stay 0xa3f");
static_assert(offsetof(MSG_MONSTER_DIE, monster_id_) == 0x0du,
              "MSG_MONSTER_DIE monster id offset");
static_assert(offsetof(MSG_MONSTER_DIE, entry_u16_a_) == 0x1au,
              "MSG_MONSTER_DIE first entry array offset");
static_assert(offsetof(MSG_MONSTER_DIE, entry_u16_b_) == 0x21au,
              "MSG_MONSTER_DIE second entry array offset");
static_assert(offsetof(MSG_MONSTER_DIE, entry_u32_) == 0x41au,
              "MSG_MONSTER_DIE third entry array offset");
static_assert(offsetof(MSG_MONSTER_DIE, entry_u16_c_) == 0x81au,
              "MSG_MONSTER_DIE fourth entry array offset");
static_assert(offsetof(MSG_MONSTER_DIE, field_a1a_) == 0xa1au,
              "MSG_MONSTER_DIE history field offset");

// IDA unit: 99_misc_global/STKillMonsterData.  CParty builds this exact
// compact handoff immediately before CBattle_Field::kill_monster.  Only the
// first 18 bytes are consumed by the recovered normal-map kill path; the
// remaining two dwords retain the original C2's stable 1.0f tail.
#pragma pack(push, 1)
struct STKillMonsterData {
    uint16_t monster_unique_id;
    uint16_t monster_type;
    uint8_t party_member_count;
    uint8_t reserved_05_07[3];
    int32_t money_ratio;
    int32_t standard_drop_rate;
    uint8_t suppress_random_drop;
    uint8_t mark_monster_completed;
    uint8_t reserved_12_13[2];
    float random_drop_scale;
};
#pragma pack(pop)

static_assert(sizeof(STKillMonsterData) == 0x18u,
              "STKillMonsterData IDA ABI size must stay 0x18");

struct DungeonRidableObject : public RidableObject {
    // IDA_EA: 0x830E9BA
    // IDA_NAME: _ZN20DungeonRidableObjectC2Ev
    // DEMANGLED: DungeonRidableObject::DungeonRidableObject(void)
    DungeonRidableObject();

    // IDA_EA: 0x830E9D8
    // IDA_NAME: _ZN20DungeonRidableObjectaSE13RidableObject
    // DEMANGLED: DungeonRidableObject::operator=(RidableObject)
    // The by-value source is intentional.  The old body copies only the
    // first five RidableObject words and preserves value_14.
    DungeonRidableObject& operator=(RidableObject source);

    int state;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(DungeonRidableObject) == 0x1cu,
              "DungeonRidableObject IDA ABI size must stay 0x1C");
#endif

// The battle-field map containers own these real values.  CParty's typed
// lifecycle needs MapInfo as a vector element, so both element types must be
// complete rather than forward-declared byte views.
struct map_monster {
    // IDA_EA: 0x81511C8
    // IDA_NAME: _ZN11map_monsterC2Ev
    // DEMANGLED: map_monster::map_monster(void)
    map_monster();
    // IDA_EA: 0x8151AEA
    // IDA_NAME: _ZN11map_monsterC2ERKS_
    // DEMANGLED: map_monster::map_monster(map_monster const&)
    map_monster(const map_monster& rhs);
    // IDA_EA: 0x8151D3A
    // IDA_NAME: _ZN11map_monsteraSERKS_
    // DEMANGLED: map_monster::operator=(map_monster const&)
    map_monster& operator=(const map_monster& rhs);
    // IDA_EA: 0x81515AE
    // IDA_NAME: _ZN11map_monsterD2Ev
    // DEMANGLED: map_monster::~map_monster()
    ~map_monster();

    // IDA_EA: 0x8151182
    // IDA_NAME: _ZN11map_monster14isNamedMonsterEv
    // DEMANGLED: map_monster::isNamedMonster(void)
    bool isNamedMonster();

    int32_t spawn_sequence_;               // +0x00
    int32_t map_monster_unique_id_;        // +0x04
    int8_t monster_kind_;                  // +0x08
    uint8_t raw_09_0b_[3];
    int32_t monster_id_;                   // +0x0c
    // Tower erase paths 0x8461708 and 0x814D214 read this byte from the
    // std::pair value at pair+0x14 (map_monster+0x10).
    uint8_t monster_level_;                // +0x10
    uint8_t specific_drop_count_;          // +0x11
    uint8_t random_drop_count_;            // +0x12
    uint8_t packet_value_13_;              // +0x13
    uint8_t stage_monster_type_;           // +0x14
    uint8_t raw_15_17_[3];
    union {
        int32_t runtime_value_18_;
        float war_room_scale_a_;
    };
    // The first two dwords remain unnamed because no original reader has
    // recovered their gameplay role.  The third is the SDC anti-cheat
    // damage-packet counter: CBattle_Field::SetDamagedPacketCnt at 0x8308F4E
    // increments exactly pair.second +0x24.
    int32_t runtime_value_1c_;             // +0x1c
    int32_t runtime_value_20_;             // +0x20
    int32_t sdc_damaged_packet_count_;     // +0x24
    union {
        // CBattle_Field::SetDamageValue at 0x8308FF8 accumulates the
        // verified client/SDC damage value at pair.second +0x28.
        int32_t sdc_damage_value_;
        int32_t runtime_value_28_;
        float war_room_scale_b_;
    };
    // CMap::insert_monster copies STAssignMonster::team here.  Team 100 is
    // the ordinary dungeon-enemy/drop owner; TIMER100 uses the same member
    // when it selects remaining boss-room monsters.
    int32_t assigned_team_;                // +0x2c
    int spawn_weight_;                     // +0x30; old C2 initializes 100
    // CMap::insert_monster copies STAssignMonster::no_hit_animation here.
    // The 0627 START_MAP and CMD038 death packets both emit this exact byte;
    // it is not an alternate monster-kind field.
    uint8_t no_hit_animation_;              // +0x34
    uint8_t raw_35_37_[3];
    ActiveStaticInfo active_static_info_;  // +0x38, sizeof 0x2f0
    std::vector<int> related_monster_ids_; // +0x328
    uint16_t hell_party_group_order_;      // +0x334
    uint16_t hell_party_group_index_;      // +0x336
    // add_hell_party_monster records the selected script difficulty here.
    uint8_t hell_party_difficulty_;         // +0x338
    uint8_t reserved_339_;
    // IDA CBattle_Field blood-spawn readers reach pair.second +0x33a.
    uint16_t blood_small_round_order_;       // +0x33a
    // add_blood_monster fills this tail from STAssignBloodSystem and
    // STAssignBloodPhaseTime.  These are concrete protocol/runtime values,
    // not an opaque byte payload.
    uint16_t blood_spawn_rate_per_mille_;    // +0x33c
    uint16_t blood_system_value_08_;         // +0x33e
    uint16_t blood_system_value_0c_;         // +0x340
    uint16_t blood_system_value_10_;         // +0x342
    uint8_t blood_runtime_flag_;             // +0x344
    uint8_t blood_variant_;                  // +0x345
    uint16_t blood_round_;                   // +0x346
    uint8_t blood_round_difficulty_;         // +0x348
    uint8_t drop_limit_excluded_;            // +0x349
    // MapInfo::Add_Mob increments its typed count exactly when this byte is
    // set (IDA +0x34a).  The original name is not recovered, so keep a
    // neutral representation rather than inventing gameplay policy.
    uint8_t counted_by_map_info_;           // +0x34a
    uint8_t completed_;                     // +0x34b
};

struct MapInfo {
    // IDA_EA: 0x8151394
    // IDA_NAME: _ZN7MapInfoC2Ev
    // DEMANGLED: MapInfo::MapInfo(void)
    MapInfo();
    // IDA_EA: 0x81518BC
    // IDA_NAME: _ZN7MapInfoD2Ev
    // DEMANGLED: MapInfo::~MapInfo()
    ~MapInfo();
    // IDA_EA: 0x81514AE
    // IDA_NAME: _ZN7MapInfo5ClearEv
    // DEMANGLED: MapInfo::Clear(void)
    MapInfo* Clear();
    // IDA_EA: 0x81517E0
    // IDA_NAME: _ZN7MapInfo8Add_ItemE8map_item
    // DEMANGLED: MapInfo::Add_Item(map_item)
    void Add_Item(map_item item);
    // IDA_EA: 0x8151612
    // IDA_NAME: _ZN7MapInfo7Add_MobE11map_monster
    // DEMANGLED: MapInfo::Add_Mob(map_monster)
    void Add_Mob(map_monster monster);
    // IDA_EA: 0x830E800
    // IDA_NAME: _ZN7MapInfo26SelectDonsterItemDropLimitEj
    // DEMANGLED: MapInfo::SelectDonsterItemDropLimit(unsigned int)
    void SelectDonsterItemDropLimit(unsigned int limit);
    // IDA_EA: 0x830EEA2
    // IDA_NAME: _ZN7MapInfoaSERKS_
    // DEMANGLED: MapInfo::operator=(MapInfo const&)
    MapInfo& operator=(const MapInfo& source);

    uint8_t visited_;                       // +0x00
    uint8_t visited_goto_;                  // +0x01
    uint8_t visited_transition_;             // +0x02
    uint8_t pad_03_;
    int map_index_;                         // +0x04
    uint8_t map_layer_;                     // +0x08
    uint8_t pad_09_0b_[3];
    std::map<int, map_monster> monsters_;   // +0x0c
    std::map<int, map_item> items_;         // +0x24
    int next_monster_unique_id_;            // +0x3c
    // IDA 0x8150065 increments this after CBossStage::dropItem succeeds,
    // then stores the previous value in map_item::unique_id_.
    int next_drop_item_unique_id_;            // +0x40
    uint8_t map_clearable_;                 // +0x44
    // The dummy-boss selector marks the source cell while it temporarily
    // redirects the maze path to a randomly chosen boss map.
    uint8_t dummy_boss_map_pending_;        // +0x45
    uint8_t pad_46_47_[2];
    int32_t random_map_seed_;               // +0x48; Clear() resets it
    stMapMonsterKillChecker_t normal_kill_check_;  // +0x4c
    stMapMonsterKillChecker_t boss_kill_check_;    // +0x64
    stMapPlayInfo_t play_info_;             // +0x7c
    std::map<int, int> monster_kill_counts_;        // +0x8c
    uint8_t assign_npc_by_potion_;          // +0xa4
    uint8_t pad_a5_a7_[3];
    std::map<int, int> monster_identity_by_unique_id_; // +0xa8
    std::vector<int> monster_order_;        // +0xc0
    uint16_t named_monster_count_;          // +0xcc
    uint8_t pad_ce_cf_[2];
    std::multimap<int, map_monster> repeated_monsters_;  // +0xd0
    uint8_t completed_;                     // +0xe8
    uint8_t pad_e9_eb_[3];
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(map_monster) == 0x34cu,
              "map_monster IDA ABI size must stay 0x34c");
static_assert(offsetof(map_monster, active_static_info_) == 0x38u,
              "map_monster ActiveStaticInfo must stay at IDA +0x38");
static_assert(offsetof(map_monster, sdc_damaged_packet_count_) == 0x24u,
              "map_monster SDC damaged-packet counter must stay at +0x24");
static_assert(offsetof(map_monster, sdc_damage_value_) == 0x28u,
              "map_monster SDC damage value must stay at +0x28");
static_assert(offsetof(map_monster, assigned_team_) == 0x2cu,
              "map_monster assigned team must stay at +0x2c");
static_assert(offsetof(map_monster, packet_value_13_) == 0x13u,
              "map_monster special-passive marker must stay at +0x13");
static_assert(offsetof(map_monster, specific_drop_count_) == 0x11u &&
                  offsetof(map_monster, random_drop_count_) == 0x12u,
              "map_monster drop-count bytes must stay at +0x11/+0x12");
static_assert(offsetof(map_monster, no_hit_animation_) == 0x34u,
              "map_monster no-hit-animation byte must stay at +0x34");
static_assert(offsetof(map_monster, hell_party_group_order_) == 0x334u &&
                  offsetof(map_monster, hell_party_group_index_) == 0x336u,
              "map_monster HELL-party group fields must stay at +0x334/+0x336");
static_assert(offsetof(map_monster, blood_small_round_order_) == 0x33au,
              "map_monster blood small-round order must stay at +0x33a");
static_assert(offsetof(map_monster, blood_spawn_rate_per_mille_) == 0x33cu,
              "map_monster blood rate must stay at +0x33c");
static_assert(offsetof(map_monster, blood_round_) == 0x346u,
              "map_monster blood round must stay at +0x346");
static_assert(sizeof(MapInfo) == 0xecu,
              "MapInfo IDA ABI size must stay 0xec");
static_assert(offsetof(MapInfo, dummy_boss_map_pending_) == 0x45u,
              "MapInfo dummy-boss marker must stay at +0x45");
#endif

// The original named state type has no recovered enumerator table.  Preserve
// the real enum identity and its i386 integer ABI without fabricating values.
enum eRidableObjectState : int {};

class CRidable : public std::map<int, DungeonRidableObject> {
public:
    // IDA_EA: 0x82A4406
    // IDA_NAME: _ZN8CRidableC2Ev
    // DEMANGLED: CRidable::CRidable(void)
    CRidable();
    // IDA_EA: 0x82A441A
    // IDA_NAME: _ZN8CRidableD2Ev
    // DEMANGLED: CRidable::~CRidable()
    ~CRidable();

    // IDA_EA: 0x82FE6BC
    // IDA_NAME: _ZN8CRidable5ClearEv
    // DEMANGLED: CRidable::Clear(void)
    void Clear();

    // IDA_EA: 0x82FE6D0
    // IDA_NAME: _ZN8CRidable16SetRidableObjectER13RidableScript
    // DEMANGLED: CRidable::SetRidableObject(RidableScript &)
    char SetRidableObject(RidableScript& script);

    // IDA_EA: 0x82FE9C8 / 0x82FEC2C / 0x830EA0A
    // IDA_NAME: _ZN8CRidable24CheckAppearRidableObjectEP6CPartyiiP11PacketGuard
    //           _ZN8CRidable12GetObjectPosERSt6vectorISt4pairIiiESaIS2_EE
    //           _ZN8CRidable12GetObjectCntEv
    // DEMANGLED: CRidable::CheckAppearRidableObject(CParty *,int,int,PacketGuard *)
    //            CRidable::GetObjectPos(std::vector<std::pair<int,int>> &)
    //            CRidable::GetObjectCnt(void)
    int CheckAppearRidableObject(CParty* party,
                                 int pos_x,
                                 int pos_y,
                                 PacketGuard* packet);
    int GetObjectPos(std::vector<std::pair<int, int> >& positions);
    int GetObjectCnt();

    // IDA_EA: 0x82FEB5E
    // IDA_NAME: _ZN8CRidable21SetRidableObjectStateEi19eRidableObjectState
    // DEMANGLED: CRidable::SetRidableObjectState(int,eRidableObjectState)
    int SetRidableObjectState(int object_id,
                              eRidableObjectState requested_state);

private:
    friend struct CRidableLayoutAssert;
    // IDA 0x82FE6D0 stores these after the inherited 0x18-byte std::map.
    // They are part of CRidable itself, not anonymous CBattle_Field tail bytes.
    uint8_t reveal_pending_;       // +0x18
    uint8_t pad_19_1b_[3];
    int32_t requested_object_count_; // +0x1c
};

struct CRidableLayoutAssert {
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
    static_assert(__builtin_offsetof(CRidable, reveal_pending_) == 0x18u,
                  "CRidable reveal flag must stay at +0x18");
    static_assert(__builtin_offsetof(CRidable, requested_object_count_) == 0x1cu,
                  "CRidable requested count must stay at +0x1c");
    static_assert(sizeof(CRidable) == 0x20u,
                  "CRidable IDA ABI size must stay 0x20");
#endif
};

enum ENUM_DUNGEON_TYPE : int;

// IDA uses this named enum in the CBattle_Field ABI.  Its enumerator table is
// not present in the recovered unit, so retain the real enum identity and
// underlying representation without inventing values.
enum ENUM_BLOOD_STATE : int {};

// IDA unit: 99_misc_global/CClearCondition.  CBattle_Field constructs this
// at +0xf0 and the following vector begins at +0x118, leaving its proven
// 0x18-byte lifetime-bearing layout explicit.
class CClearCondition {
public:
    // IDA_EA: 0x82A4342
    // IDA_NAME: _ZN15CClearConditionC2Ev
    CClearCondition();
    // IDA_EA: 0x82A442E
    // IDA_NAME: _ZN15CClearConditionD2Ev
    ~CClearCondition();
    CClearCondition* Clear();
    bool IsClearCondition();
    bool Empty();
    // IDA_EA: 0x82FEF12 / 0x82FEFCE
    // IDA_NAME: _ZN15CClearCondition30SetDungeonClearConditionVectorERSt6vectorI21DungeonClearConditionSaIS1_EE /\
    //           _ZN15CClearCondition14ClearConditionE33ENUM_DUNGEON_CLEAR_CONDITION_TYPEi
    // The first provider is retained in the select-helper owner under its
    // original ABI symbol; the second advances one matching runtime entry.
    int SetDungeonClearConditionVector(
        std::vector<DungeonClearCondition>& conditions);
    bool ClearCondition(int condition_type, int value);

private:
    // CMD120 consumes these two physical fields directly at +0x118/+0x124
    // (Dispatcher_BossDieCheck::process @ 0x81CD7A2).  Grant that original
    // non-member owner access without introducing an offset accessor.
    friend class Dispatcher_BossDieCheck;

    int current_count_;               // +0x00
    int target_count_;                // +0x04
    std::vector<int> cleared_states_; // +0x08
    const std::vector<DungeonClearCondition>* condition_source_; // +0x14
};

// The old type is a 0x38-byte embedded object at CBattle_Field +0xc98.
// Its detailed reward policy remains owned by the event domain, but its C2/D2
// is represented here so the enclosing CParty lifecycle is real rather than
// a byte-only tail.
class CTournamentDungeonReward {
public:
    // IDA_EA: 0x8284770
    // IDA_NAME: _ZN24CTournamentDungeonRewardC2Ev
    CTournamentDungeonReward();
    // IDA_EA: 0x82847DC
    // IDA_NAME: _ZN24CTournamentDungeonRewardD2Ev
    virtual ~CTournamentDungeonReward();
    CTournamentDungeonReward* ResetReward();

    // IDA_EA: 0x8284890
    // IDA_NAME: _ZN24CTournamentDungeonReward10MakeRewardEPKSt3mapIi20TournamentRewardRateSt4lessIiESaISt4pairIKiS1_EEEPK24TournamentRewardItemRateib
    // DEMANGLED: CTournamentDungeonReward::MakeReward(
    //     std::map<int,TournamentRewardRate> const*,TournamentRewardItemRate const*,int,bool)
    int MakeReward(const std::map<int, TournamentRewardRate>* reward_rates,
                   const TournamentRewardItemRate* reward_items,
                   int member_count,
                   bool tournament_victory);

    // IDA_EA: 0x8284B8E
    // IDA_NAME: _ZN24CTournamentDungeonReward13SetRewardGoldERN8WongWork18stGenerateResult_tEf
    // DEMANGLED: CTournamentDungeonReward::SetRewardGold(WongWork::stGenerateResult_t &,float)
    bool SetRewardGold(WongWork::stGenerateResult_t& generated,
                       float gold_scale);

    // IDA_EA: 0x8284CAA
    // IDA_NAME: _ZN24CTournamentDungeonReward16MakeRewardPacketEP11PacketGuardb
    // DEMANGLED: CTournamentDungeonReward::MakeRewardPacket(PacketGuard *,bool)
    int MakeRewardPacket(PacketGuard* packet, bool tournament_victory);

    // IDA_EA: 0x8284D08
    // IDA_NAME: _ZN24CTournamentDungeonReward20isCardSelectCompleteE32ENUM_TOURNAMENT_REWARD_CARD_TYPERKSt6vectorIiSaIiEE
    // DEMANGLED: CTournamentDungeonReward::isCardSelectComplete(
    //     ENUM_TOURNAMENT_REWARD_CARD_TYPE,std::vector<int,std::allocator<int>> const&)
    bool isCardSelectComplete(
        ENUM_TOURNAMENT_REWARD_CARD_TYPE card_type,
        const std::vector<int>& valid_members);

    // IDA_EA: 0x8284EE4
    // IDA_NAME: _ZN24CTournamentDungeonReward10SelectCardEicc
    // DEMANGLED: CTournamentDungeonReward::SelectCard(int,char,char)
    bool SelectCard(int member_slot, char card_group, char reward_kind);

    // IDA_EA: 0x8284FA4
    // IDA_NAME: _ZN24CTournamentDungeonReward16MakeSelectPacketEP11PacketGuard
    // DEMANGLED: CTournamentDungeonReward::MakeSelectPacket(PacketGuard *)
    bool MakeSelectPacket(PacketGuard* packet);

    // IDA_EA: 0x828502E
    // IDA_NAME: _ZN24CTournamentDungeonReward15GetItemFromCardE32ENUM_TOURNAMENT_REWARD_CARD_TYPEiP10Inven_Itemj
    // DEMANGLED: CTournamentDungeonReward::GetItemFromCard(
    //     ENUM_TOURNAMENT_REWARD_CARD_TYPE,int,Inven_Item *,unsigned int)
    int GetItemFromCard(ENUM_TOURNAMENT_REWARD_CARD_TYPE card_type,
                        int member_slot,
                        Inven_Item* output_items,
                        unsigned int output_item_count);

    // IDA_EA: 0x828523C
    // IDA_NAME: _ZN24CTournamentDungeonReward15ForceSelectCardERSt6vectorIiSaIiEEb
    // DEMANGLED: CTournamentDungeonReward::ForceSelectCard(
    //     std::vector<int,std::allocator<int>> &,bool)
    bool ForceSelectCard(std::vector<int>& valid_members,
                         bool tournament_victory);

    // IDA_EA: 0x8285284
    // IDA_NAME: _ZN24CTournamentDungeonReward18isMemberSelectCardE32ENUM_TOURNAMENT_REWARD_CARD_TYPEi
    // DEMANGLED: CTournamentDungeonReward::isMemberSelectCard(
    //     ENUM_TOURNAMENT_REWARD_CARD_TYPE,int)
    bool isMemberSelectCard(ENUM_TOURNAMENT_REWARD_CARD_TYPE card_type,
                            int member_slot);

private:
    // IDA_EA: 0x8284A24
    // IDA_NAME: _ZN24CTournamentDungeonReward18DecisionRewardTypeE32ENUM_TOURNAMENT_REWARD_CARD_TYPER20TournamentRewardRatePK24TournamentRewardItemRate
    // DEMANGLED: CTournamentDungeonReward::DecisionRewardType(
    //     ENUM_TOURNAMENT_REWARD_CARD_TYPE,TournamentRewardRate &,TournamentRewardItemRate const*)
    bool DecisionRewardType(ENUM_TOURNAMENT_REWARD_CARD_TYPE card_type,
                            TournamentRewardRate& reward_rate,
                            const TournamentRewardItemRate* reward_items);

    // IDA_EA: 0x82852E4
    // IDA_NAME: _ZN24CTournamentDungeonReward17_MakeRewardPacketE32ENUM_TOURNAMENT_REWARD_CARD_TYPEP11PacketGuard
    // DEMANGLED: CTournamentDungeonReward::_MakeRewardPacket(
    //     ENUM_TOURNAMENT_REWARD_CARD_TYPE,PacketGuard *)
    char _MakeRewardPacket(ENUM_TOURNAMENT_REWARD_CARD_TYPE card_type,
                           PacketGuard* packet);

    // IDA_EA: 0x82853B8
    // IDA_NAME: _ZN24CTournamentDungeonReward16_ForceSelectCardE32ENUM_TOURNAMENT_REWARD_CARD_TYPERKSt6vectorIiSaIiEE
    // DEMANGLED: CTournamentDungeonReward::_ForceSelectCard(
    //     ENUM_TOURNAMENT_REWARD_CARD_TYPE,std::vector<int,std::allocator<int>> const&)
    bool _ForceSelectCard(ENUM_TOURNAMENT_REWARD_CARD_TYPE card_type,
                          const std::vector<int>& valid_members);

    int32_t reward_round_;                         // +0x04; +0x00 is vptr
    TournamentRewardType rewards_[2][2];           // +0x08
};

// IDA unit: 09_dungeon_pvp_party/CDungeonClearTracer.  CBattle_Field owns
// this physical value at +0x158; its setters are called while assembling a
// map, so it must remain a typed embedded object instead of byte storage.
struct CDungeonClearTracer {
    int Trace(const char* function);
    CDungeonClearTracer* SetMapId(int map_id);
    CDungeonClearTracer* SetMaxBossCount(int count);
    CDungeonClearTracer* SetKilledBossCount(int count);
    CDungeonClearTracer* SetMaxMonsterCount(int count);
    CDungeonClearTracer* SetKilledMonsterCount(int count);
    CDungeonClearTracer* SetClearedDungeon(bool cleared);
    CDungeonClearTracer* SetEndPoint(bool end_point);
    CDungeonClearTracer* IncrementKilledBossCount();
    CDungeonClearTracer* IncrementKilledMonsterCount();
    bool IsKilledBoss();
    CDungeonClearTracer* Reset();

    int map_id;
    int max_boss_count;
    int killed_boss_count;
    int max_monster_count;
    int killed_monster_count;
    uint8_t cleared_dungeon;
    uint8_t end_point;
    uint8_t pad_16[2];
};

static_assert(sizeof(CClearCondition) == 0x18u,
              "CClearCondition IDA ABI size must stay 0x18");
static_assert(sizeof(CTournamentDungeonReward) == 0x38u,
              "CTournamentDungeonReward IDA ABI size must stay 0x38");

// Canonical runtime declarations for the IDA CBattle_Field aggregate.  The
// full 0xcd0-byte lifetime is now expressed as real members.  Cross-TU calls
// must use these real members instead of C-style asm aliases.
class CBattle_Field {
public:
    // IDA_EA: 0x82A44FE
    // IDA_NAME: _ZN13CBattle_FieldC2Ev
    // DEMANGLED: CBattle_Field::CBattle_Field(void)
    CBattle_Field();
    // IDA_EA: 0x82A499C
    // IDA_NAME: _ZN13CBattle_FieldD2Ev
    // DEMANGLED: CBattle_Field::~CBattle_Field()
    ~CBattle_Field();

    // CParty physically owns this object through CBattle_Field at
    // CParty +0x0b24 + CBattle_Field +0x0c98 == +0x17bc.  Exposing the
    // named embedded value prevents event consumers from recreating that
    // address arithmetic.
    CTournamentDungeonReward& tournament_reward() { return tournament_reward_; }
    const CTournamentDungeonReward& tournament_reward() const {
        return tournament_reward_;
    }

    // IDA_EA: 0x830D848 / 0x830D902
    // IDA_NAME: _ZN13CBattle_Field25send_elevator_time_packetEc /
    //           _ZN13CBattle_Field20check_elevator_timerEv
    // DEMANGLED: CBattle_Field::send_elevator_time_packet(char) /
    //            CBattle_Field::check_elevator_timer(void)
    void send_elevator_time_packet(char state);
    void check_elevator_timer();

    // IDA_EA: 0x830DAEE
    // IDA_NAME: _ZN13CBattle_Field11pickup_itemEi
    // DEMANGLED: CBattle_Field::pickup_item(int)
    int pickup_item(int unique_id);

    // IDA_EA: 0x830D9C0
    // IDA_NAME: _ZN13CBattle_Field14get_field_itemEiR8map_item
    // DEMANGLED: CBattle_Field::get_field_item(int,map_item &)
    int get_field_item(int unique_id, map_item& item);

    // IDA_EA: 0x830D58C
    // IDA_NAME: _ZN13CBattle_Field18kill_blood_monsterEP5CUseriPt
    // DEMANGLED: CBattle_Field::kill_blood_monster(CUser *,int,unsigned short *)
    int kill_blood_monster(CUser* user,
                           int count,
                           unsigned short* monster_ids);

    // IDA_EA: 0x830BC78
    // IDA_NAME: _ZN13CBattle_Field12kill_monsterER11PacketGuardR11map_monsterRjRK17STKillMonsterData
    // DEMANGLED: CBattle_Field::kill_monster(PacketGuard &,map_monster &,
    //             unsigned int &,STKillMonsterData const&)
    int kill_monster(PacketGuard& packet,
                     map_monster& killed_monster,
                     unsigned int& gained_exp,
                     const STKillMonsterData& data);

    // IDA_EA: 0x830ADF6
    // IDA_NAME: _ZN13CBattle_Field14_MakeDropItemsERSt4listI8map_itemSaIS1_EER7MapInfoRK11map_monstertiiibRSt6vectorIbSaIbEEfi
    void _MakeDropItems(std::list<map_item>& dropped,
                        MapInfo& map,
                        const map_monster& monster,
                        unsigned short monster_type,
                        int party_member_count,
                        int money_ratio,
                        int standard_drop_rate,
                        bool hell_group_completed,
                        std::vector<bool>& first_kill_flags,
                        float monster_random_drop_scale,
                        int premium_independent_drop_rate);
    // IDA_EA: 0x830B848
    int setPremiumDropRate(WongWork::stGenerateRefData_t& reference,
                           int premium_independent_drop_rate);
    // IDA_EA: 0x830B8CC
    int _MakeNotiPacketDieMonster(PacketGuard& packet,
                                  std::list<map_item>& dropped,
                                  int monster_unique_id,
                                  unsigned short monster_type,
                                  const map_monster& live_monster,
                                  map_monster& killed_monster);
    // IDA_EA: 0x830AA46
    int getItemDropDungeonType();
    // IDA_EA: 0x830D704
    // IDA_NAME: _ZN13CBattle_Field28KillHellPartyGroupMonsterCntER7MapInfoR11map_monsterRb
    char KillHellPartyGroupMonsterCnt(MapInfo& map,
                                      map_monster& monster,
                                      bool& group_completed);

    // IDA_EA: 0x8304F8E
    // IDA_NAME: _ZN13CBattle_Field29checkKilledSpawnBloodMonstersEi
    // DEMANGLED: CBattle_Field::checkKilledSpawnBloodMonsters(int)
    void checkKilledSpawnBloodMonsters(int monster_count);

    struct TournamentInfo {
        TournamentInfo();
        ~TournamentInfo();
        TournamentInfo* Clear();
        bool operator<(const TournamentInfo& rhs) const;

        int16_t round_;                 // +0x00
        uint8_t pad_02_03_[2];
        TournamentMonster monsters_[2]; // +0x04
        int key_;                       // +0x14
    };

    // IDA tail at CBattle_Field +0x2ac.  The first value survives a field
    // reset; the remaining three are reset by reset_blood_values().
    struct CBloodRound {
        uint16_t pre_map_blood_round_order;
        uint16_t blood_round_order;
        uint16_t blood_small_round_order;
        uint16_t blood_last_small_round_order;

        // IDA_EA: 0x8306D66
        // IDA_NAME: _ZN13CBattle_Field11CBloodRound18reset_blood_valuesEv
        // DEMANGLED: CBattle_Field::CBloodRound::reset_blood_values(void)
        CBloodRound* reset_blood_values();

        // IDA_EA: 0x830EB72
        // IDA_NAME: _ZN13CBattle_Field11CBloodRound18setBloodRoundOrderEv
        // DEMANGLED: CBattle_Field::CBloodRound::setBloodRoundOrder(void)
        CBloodRound* setBloodRoundOrder();

        // IDA_EA: 0x830EB84
        // IDA_NAME: _ZN13CBattle_Field11CBloodRound24setPreMapBloodRoundOrderEv
        // DEMANGLED: CBattle_Field::CBloodRound::setPreMapBloodRoundOrder(void)
        CBloodRound* setPreMapBloodRoundOrder();

        // IDA_EA: 0x830EB96
        // IDA_NAME: _ZN13CBattle_Field11CBloodRound27setBloodLastSmallRoundOrderEi
        // DEMANGLED: CBattle_Field::CBloodRound::setBloodLastSmallRoundOrder(int)
        CBloodRound* setBloodLastSmallRoundOrder(int small_round);

        // IDA_EA: 0x830EBA8
        // IDA_NAME: _ZN13CBattle_Field11CBloodRound23setBloodSmallRoundOrderEs
        // DEMANGLED: CBattle_Field::CBloodRound::setBloodSmallRoundOrder(short)
        CBloodRound* setBloodSmallRoundOrder(short small_round);

        // IDA_EA: 0x830EBC2
        // IDA_NAME: _ZN13CBattle_Field11CBloodRound18getBloodRoundOrderEv
        // DEMANGLED: CBattle_Field::CBloodRound::getBloodRoundOrder(void)
        int getBloodRoundOrder();

        // IDA_EA: 0x830EBCE
        // IDA_NAME: _ZN13CBattle_Field11CBloodRound23getBloodSmallRoundOrderEv
        // DEMANGLED: CBattle_Field::CBloodRound::getBloodSmallRoundOrder(void)
        int getBloodSmallRoundOrder();

        // IDA_EA: 0x830EBDA
        // IDA_NAME: _ZN13CBattle_Field11CBloodRound27getBloodLastSmallRoundOrderEv
        // DEMANGLED: CBattle_Field::CBloodRound::getBloodLastSmallRoundOrder(void)
        int getBloodLastSmallRoundOrder();

        // IDA_EA: 0x830EBE6
        // IDA_NAME: _ZN13CBattle_Field11CBloodRound18incBloodRoundOrderEv
        // DEMANGLED: CBattle_Field::CBloodRound::incBloodRoundOrder(void)
        CBloodRound* incBloodRoundOrder();

        // IDA_EA: 0x830EBFC
        // IDA_NAME: _ZN13CBattle_Field11CBloodRound23incBloodSmallRoundOrderEv
        // DEMANGLED: CBattle_Field::CBloodRound::incBloodSmallRoundOrder(void)
        CBloodRound* incBloodSmallRoundOrder();
    };

    // IDA layout: 0x8306EE0 / 0x8306F5C.  These are nested Battle_Field
    // state holders, not free `CBattle_Field_*` ABI shims.
    struct CBloodClearRewardData {
        // IDA_EA: 0x82A448E
        // IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardDataC2Ev
        CBloodClearRewardData();

        struct stRewardItem_t {
            // IDA_EA: 0x82A4458
            // IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData14stRewardItem_tC2Ev
            stRewardItem_t();

            // IDA_EA: 0x8306F5C
            // IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData14stRewardItem_t5resetEv
            // DEMANGLED: CBattle_Field::CBloodClearRewardData::stRewardItem_t::reset(void)
            bool reset();

            // IDA 0x85BF100 / 0x85BF15A read and write this signed count
            // directly.  The group has no range guard in the original code.
            int item_count_;                         // +0x00
            // 0x04: ten complete, contiguous IDA Inven_Item records.
            // The trailing two bytes are ABI alignment padding: no source
            // field has been inferred for them.
            Inven_Item items_[10];                   // +0x04
        };

        // IDA_EA: 0x8306EE0
        // IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData5resetEv
        // DEMANGLED: CBattle_Field::CBloodClearRewardData::reset(void)
        CBloodClearRewardData* reset();

        // IDA_EA: 0x8306FAA
        // IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData17onStartBloodRoundEv
        int onStartBloodRound();

        // IDA_EA: 0x8306FC4
        // IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData18onFinishBloodRoundEbsP6CPartyPK8CDungeon
        bool onFinishBloodRound(bool blood_clear,
                                short blood_round,
                                CParty* party,
                                const CDungeon* dungeon);

        // IDA_EA: 0x8307096
        // IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData26incUltimateRoundClearPointEi
        CBloodClearRewardData* incUltimateRoundClearPoint(
            int next_round_difficulty);

        // IDA_EA: 0x83070CA
        // IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData21getUltimateRewardItemEv
        // DEMANGLED: CBattle_Field::CBloodClearRewardData::getUltimateRewardItem(void)
        int getUltimateRewardItem();

        // IDA_EA: 0x830EC3A
        // IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData11addPlayTimeEj
        CBloodClearRewardData* addPlayTime(unsigned int elapsed_msec);

        // IDA_EA: 0x830EC5A
        // IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData11getPlayTimeEv
        int getPlayTime();

        // IDA_EA: 0x830EC64
        // IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData22getBloodRoundClearTimeEv
        int getBloodRoundClearTime();

        // IDA_EA: 0x830EC7C
        // IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData13setBloodClearEb
        CBloodClearRewardData* setBloodClear(bool blood_clear);

        // IDA_EA: 0x830EC94
        // IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData26getUltimateRoundClearPointEv
        int getUltimateRoundClearPoint();

        // The following ten leaf accessors are all direct physical member
        // operations in the original i386 image.  They intentionally retain
        // the original unchecked indexing behavior.
        // IDA_EA: 0x85BF0E6 / 0x85BF0F2
        // IDA_NAME: _ZN13CBattle_Field21CBloodClearRewardData20getCurrentBloodRoundEv /
        //           _ZN13CBattle_Field21CBloodClearRewardData20setCurrentBloodRoundEi
        int getCurrentBloodRound();
        CBloodClearRewardData* setCurrentBloodRound(int blood_round);

        // IDA_EA: 0x85BF100 / 0x85BF11A
        int getRewardItemCount(int reward_group);
        Inven_Item* getRewardItem(int reward_group);

        // IDA_EA: 0x85BF132 / 0x85BF144
        int getRewardExp(int reward_group);
        CBloodClearRewardData* setRewardExp(int reward_group,
                                            unsigned int reward_exp);

        // IDA_EA: 0x85BF15A / 0x85BF176
        int setRewardItem(int reward_group, int item_count);
        Inven_Item* setRewarItemCopy(Inven_Item item,
                                     int reward_group,
                                     int item_slot);

        // IDA_EA: 0x85BF1A8 / 0x85BF21C
        Inven_Item* setRewardItemAddInfo(float multiplier,
                                         int reward_group,
                                         int item_slot);
        int isBloodClear();

        // The function inventory has no CBloodClearRewardData D2/D0 symbol:
        // destruction is the compiler-generated member lifetime only.  Do
        // not introduce a synthetic public destructor ABI.
        uint32_t play_time_msec_;                    // +0x00
        uint32_t previous_play_time_msec_;           // +0x04
        int current_blood_round_;                    // +0x08
        uint32_t blood_round_start_tick_;            // +0x0c
        uint32_t blood_round_finish_tick_;           // +0x10
        uint8_t blood_clear_;                        // +0x14
        uint8_t pad_15_17_[3];
        // IDA 0x8306F1C clears these four contiguous dwords.
        uint32_t reward_exp_[4];                     // +0x18
        stRewardItem_t reward_items_[4];             // +0x28
        int ultimate_round_clear_point_;             // +0x9c8
    };

    static_assert(sizeof(CBloodClearRewardData::stRewardItem_t) == 0x268,
                  "CBloodClearRewardData::stRewardItem_t ABI must stay 0x268");
    static_assert(offsetof(CBloodClearRewardData::stRewardItem_t,
                           item_count_) == 0x00,
                  "CBloodClearRewardData reward item count begins at +0x00");
    static_assert(offsetof(CBloodClearRewardData::stRewardItem_t,
                           items_) == 0x04,
                  "CBloodClearRewardData reward items begin at +0x04");
    static_assert(offsetof(CBloodClearRewardData, current_blood_round_) == 0x08,
                  "CBloodClearRewardData round must stay at +0x08");
    static_assert(offsetof(CBloodClearRewardData, blood_clear_) == 0x14,
                  "CBloodClearRewardData clear flag must stay at +0x14");
    static_assert(offsetof(CBloodClearRewardData, reward_exp_) == 0x18,
                  "CBloodClearRewardData reward exp begins at +0x18");
    static_assert(offsetof(CBloodClearRewardData, reward_items_) == 0x28,
                  "CBloodClearRewardData reward items begin at IDA +0x28");
    static_assert(offsetof(CBloodClearRewardData,
                           ultimate_round_clear_point_) == 0x9c8,
                  "CBloodClearRewardData ultimate score must stay at +0x9c8");
    static_assert(sizeof(CBloodClearRewardData) == 0x9cc,
                  "CBloodClearRewardData ABI must stay 0x9cc");

    // CMD042's clear path owns the transitions below.  Keep the two embedded
    // blood values and the diagnostic tracer behind their actual
    // CBattle_Field owner rather than recreating the original CParty-relative
    // addresses (+0x0dd0/+0x0ddc/+0x0c7c).
    CBloodRound& blood_round() { return blood_round_; }
    const CBloodRound& blood_round() const { return blood_round_; }
    CBloodClearRewardData& blood_clear_reward() {
        return blood_clear_reward_;
    }
    const CBloodClearRewardData& blood_clear_reward() const {
        return blood_clear_reward_;
    }
    CDungeonClearTracer& dungeon_clear_tracer() {
        return dungeon_clear_tracer_;
    }
    const CDungeonClearTracer& dungeon_clear_tracer() const {
        return dungeon_clear_tracer_;
    }

    // IDA_EA: 0x822CF26
    // IDA_NAME: _ZN13CBattle_Field20GetEnableHellDungeonEv
    // DEMANGLED: CBattle_Field::GetEnableHellDungeon(void)
    static uint8_t* GetEnableHellDungeon();

    // IDA_EA: 0x822CF30
    // IDA_NAME: _ZN13CBattle_Field20SetEnableHellDungeonEb
    // DEMANGLED: CBattle_Field::SetEnableHellDungeon(bool)
    static uint8_t* SetEnableHellDungeon(bool enabled);

    // IDA_EA: 0x830ED07
    // IDA_NAME: _ZN13CBattle_Field19IsEnableHellDungeonEv
    // DEMANGLED: CBattle_Field::IsEnableHellDungeon(void)
    static int IsEnableHellDungeon();

    // IDA_EA: 0x85BF250
    // IDA_NAME: _ZN13CBattle_Field22IsKilledAllHellGruoupsEv
    // DEMANGLED: CBattle_Field::IsKilledAllHellGruoups(void)
    int IsKilledAllHellGruoups();

    // IDA_EA: 0x822CBCC
    // IDA_NAME: _ZN13CBattle_Field17GetCurrentMapInfoEv
    // DEMANGLED: CBattle_Field::GetCurrentMapInfo(void)
    MapInfo* GetCurrentMapInfo();

    // IDA_EA: 0x830DE1C
    // IDA_NAME: _ZN13CBattle_Field21GetTotalSkillMaterialEv
    // DEMANGLED: CBattle_Field::GetTotalSkillMaterial(void)
    int GetTotalSkillMaterial();

    // IDA_EA: 0x85BF29A / 0x85BF2E8
    // IDA_NAME: _ZN13CBattle_Field14ClearConditionE33ENUM_DUNGEON_CLEAR_CONDITION_TYPEi /\
    //           _ZN13CBattle_Field20DestroyPassiveObjectEi
    bool ClearCondition(int condition_type, int value);
    int DestroyPassiveObject(int object_unique_id);

    // IDA_EA: 0x8308F4E / 0x8308FF8
    // IDA_NAME: _ZN13CBattle_Field19SetDamagedPacketCntEi /
    //           _ZN13CBattle_Field14SetDamageValueEii
    // These mutate the current map's map_monster SDC accounting fields.
    int SetDamagedPacketCnt(int monster_unique_id);
    int SetDamageValue(int monster_unique_id, int damage_value);

    // IDA_EA: 0x830DB84
    // IDA_NAME: _ZN13CBattle_Field9drop_itemE8map_item
    // DEMANGLED: CBattle_Field::drop_item(map_item)
    int drop_item(map_item item);

    // IDA_EA: 0x830DC74
    // IDA_NAME: _ZN13CBattle_Field10onStartMapEv
    // DEMANGLED: CBattle_Field::onStartMap(void)
    int onStartMap();

    // The field owns this selected dungeon pointer at +0x188.  CMD050's
    // blood-dungeon gate is a direct read of this exact relationship.
    CDungeon* get_dungeon() const { return dungeon_; }

    // IDA_EA: 0x822D05E
    // IDA_NAME: _ZN13CBattle_Field18GetCurrentMapIndexEv
    // DEMANGLED: CBattle_Field::GetCurrentMapIndex(void)
    int GetCurrentMapIndex();

    // IDA_EA: 0x830DBDE
    // IDA_NAME: _ZN13CBattle_Field9onUseItemEP5CUserRK10Inven_Item
    // DEMANGLED: CBattle_Field::onUseItem(CUser *,Inven_Item const&)
    MapInfo* onUseItem(CUser* user, const Inven_Item& item);

    // IDA_EA: 0x822CBBE
    // IDA_NAME: _ZN13CBattle_Field18GetMapInfoKeyValueEii
    // DEMANGLED: CBattle_Field::GetMapInfoKeyValue(int,int)
    int GetMapInfoKeyValue(int pos_x, int pos_y);

    // IDA_EA: 0x822CC48
    // IDA_NAME: _ZN13CBattle_Field17GetMapInfoFromPosEiib
    // DEMANGLED: CBattle_Field::GetMapInfoFromPos(int,int,bool)
    MapInfo* GetMapInfoFromPos(int pos_x, int pos_y, bool layered);

    // IDA_EA: 0x830E32E
    // IDA_NAME: _ZN13CBattle_Field15checkLayeredMapEii
    // DEMANGLED: CBattle_Field::checkLayeredMap(int,int)
    int checkLayeredMap(int pos_x, int pos_y);

    // IDA_EA: 0x8308AE6
    // IDA_NAME: _ZN13CBattle_Field7MoveMapEiiRb
    // DEMANGLED: CBattle_Field::MoveMap(int,int,bool &)
    int MoveMap(int pos_x, int pos_y, bool& layered);

    // IDA_EA: 0x8300C76..0x8302BC6, 0x83031D2, 0x83035C8..0x8304804.
    // These are the original map-assembly member identities, not C ABI
    // aliases.  `ConsistMap` calls the entire group in the old branch order.
    int checkClearedNamedMonsterMap(unsigned char position_index,
                                   int monster_id);
    CMap* getMap(int map_index);
    CDungeonClearTracer* setDungeonClearTrace(int map_id,
                                               int max_boss_count,
                                               int killed_boss_count,
                                               int killed_monster_count);
    int getAddChampionMobCnt();
    int get_monster_list(std::list<_mapMonster>& monsters);
    int add_monster_list_bossroom_condition(std::list<_mapMonster>& monsters,
                                            int pos_x,
                                            int pos_y);
    char add_monsters_current_map(std::list<_mapMonster>& source,
                                  std::vector<int>& champion_candidates,
                                  map_monster& scratch,
                                  int& normal_monster_count,
                                  int& next_unique_id,
                                  int& source_sequence);
    int add_monster_hell_party(map_monster& scratch,
                               bool& generated,
                               int& next_unique_id,
                               int& source_sequence);
    int add_monster_blood(map_monster& scratch,
                          int& next_unique_id,
                          int& source_sequence);
    void add_monster_APC_AI(int& next_unique_id);
    void add_monster_APC_AI_by_rate(map_monster& scratch,
                                    int& next_unique_id,
                                    int pos_x,
                                    int pos_y);
    bool add_monster_additional_champion(std::vector<int>& candidates,
                                         int additional_count);
    int add_monster_event(int& next_unique_id, int pos_x, int pos_y);
    char set_monster_ability();
    char insert_tournamentRoundMonster();
    int add_item_curr_map(int& next_item_unique_id,
                          int& next_monster_unique_id);
    int ConsistMap();

    bool ConsistTournament(
        const CMap* map,
        std::list<_mapMonster>& monsters,
        std::map<short, std::vector<TournamentInfo> >& rounds);
    bool MakeBasicRoundInfo(int member_count,
                            std::vector<TournamentMonster>& monsters,
                            std::vector<TournamentInfo>& basic_round);
    char ChoiceFirstGroup(std::vector<TournamentInfo>& basic_round,
                          std::vector<TournamentInfo>& first_group);
    int ConsistTournamentRound(
        std::vector<TournamentInfo>& first_group,
        std::vector<TournamentInfo>& basic_round,
        std::map<short, std::vector<TournamentInfo> >& rounds,
        std::list<_mapMonster>& monsters);
    bool InsertTournamentInfo(std::vector<int>& slots,
                              std::vector<TournamentInfo>& infos,
                              std::map<short, TournamentInfo>& bracket);
    int MakeRoundInfo(
        std::map<short, TournamentInfo>& bracket,
        std::map<short, std::vector<TournamentInfo> >& rounds,
        std::list<_mapMonster>& monsters);
    int MakeNextRoundInfo(int round,
                          std::map<short, TournamentInfo>& bracket);
    bool MakeTournamentMonsterList(
        std::map<short, std::vector<TournamentInfo> >& rounds,
        std::list<_mapMonster>& monsters);
    bool AddTournamentMonster(const TournamentInfo& info,
                              std::list<_mapMonster>& monsters,
                              bool final_round);
    int add_hell_party_monster(
        map_monster& scratch,
        int& next_unique_id,
        int& source_sequence,
        std::map<unsigned short, STHellPartyGroupOrder>* group_orders);
    int add_blood_monster(
        map_monster& scratch,
        int& next_unique_id,
        int& source_sequence,
        std::vector<STAssignBloodSystem> systems,
        std::vector<STAssignBloodPhaseTime> phases);

    // IDA_EA: 0x83090A4
    // IDA_NAME: _ZN13CBattle_Field10MakePacketEiPv
    // DEMANGLED: CBattle_Field::MakePacket(int,void *)
    void MakePacket(int packet_id, void* packet);

    // IDA_EA: 0x830ECA2
    // IDA_NAME: _ZN13CBattle_Field24CheckAppearRidableObjectEP11PacketGuard
    // DEMANGLED: CBattle_Field::CheckAppearRidableObject(PacketGuard *)
    int CheckAppearRidableObject(PacketGuard* packet);

    // IDA_EA: 0x830A0E8
    // IDA_NAME: _ZN13CBattle_Field16check_grid_clearEv
    // DEMANGLED: CBattle_Field::check_grid_clear(void)
    int check_grid_clear();

    // IDA_EA: 0x82FF486
    // IDA_NAME: _ZN13CBattle_Field13SelectDungeonEiPK8CDungeoni17ENUM_DUNGEON_TYPEiP9CWorldMapbi
    // DEMANGLED: CBattle_Field::SelectDungeon(int,CDungeon const*,int,ENUM_DUNGEON_TYPE,int,CWorldMap *,bool,int)
    bool SelectDungeon(int dungeon_index,
                       const CDungeon* dungeon,
                       int dungeon_diff,
                       ENUM_DUNGEON_TYPE dungeon_type,
                       int member_count,
                       CWorldMap* world_map,
                       bool hell_party,
                       int parameter);

    // IDA_EA: 0x830A862
    // IDA_NAME: _ZN13CBattle_Field32check_random_appear_hell_dungeonEPK8CDungeon17ENUM_DUNGEON_MODE17ENUM_DUNGEON_TYPERb
    // DEMANGLED: CBattle_Field::check_random_appear_hell_dungeon(CDungeon const*,ENUM_DUNGEON_MODE,ENUM_DUNGEON_TYPE,bool &)
    CParty* check_random_appear_hell_dungeon(
        const CDungeon* dungeon,
        ENUM_DUNGEON_MODE dungeon_mode,
        ENUM_DUNGEON_TYPE dungeon_type,
        bool& should_appear);

    // IDA_EA: 0x830DD2C
    // IDA_NAME: _ZN13CBattle_Field10onClearMapEb
    // DEMANGLED: CBattle_Field::onClearMap(bool)
    int onClearMap(bool cleared);

    // IDA_EA: 0x822D052
    // IDA_NAME: _ZN13CBattle_Field7getMazeEv
    // DEMANGLED: CBattle_Field::getMaze(void)
    MazeScript* getMaze();

    // IDA_EA: 0x830DF2E
    // IDA_NAME: _ZN13CBattle_Field18getLayeredMapIndexEv
    // DEMANGLED: CBattle_Field::getLayeredMapIndex(void)
    int getLayeredMapIndex();

    // IDA_EA: 0x830DF3C
    // IDA_NAME: _ZN13CBattle_Field18setLayeredMapIndexEv
    // DEMANGLED: CBattle_Field::setLayeredMapIndex(void)
    CBattle_Field* setLayeredMapIndex();

    // IDA_EA: 0x830DF56
    // IDA_NAME: _ZN13CBattle_Field21getLayeredMapKeyValueEi
    // DEMANGLED: CBattle_Field::getLayeredMapKeyValue(int)
    int getLayeredMapKeyValue(int layered_map_index);

    // IDA_EA: 0x830DF66
    // IDA_NAME: _ZN13CBattle_Field24getCurrentLayeredMapInfoEv
    // DEMANGLED: CBattle_Field::getCurrentLayeredMapInfo(void)
    MapInfo* getCurrentLayeredMapInfo();

    // IDA_EA: 0x830DF88
    // IDA_NAME: _ZN13CBattle_Field22getLayeredMapInfoIndexEi
    // DEMANGLED: CBattle_Field::getLayeredMapInfoIndex(int)
    MapInfo* getLayeredMapInfoIndex(int layered_map_index);

    // IDA_EA: 0x830E302
    // IDA_NAME: _ZN13CBattle_Field12setQuestMazeEb
    // DEMANGLED: CBattle_Field::setQuestMaze(bool)
    CBattle_Field* setQuestMaze(bool quest_maze);

    // IDA_EA: 0x830E31E
    // IDA_NAME: _ZN13CBattle_Field11isQuestMazeEv
    // DEMANGLED: CBattle_Field::isQuestMaze(void)
    int isQuestMaze();

    // IDA_EA: 0x85BF0B8
    // IDA_NAME: _ZN13CBattle_Field13setBloodStateE16ENUM_BLOOD_STATE
    // DEMANGLED: CBattle_Field::setBloodState(ENUM_BLOOD_STATE)
    bool setBloodState(ENUM_BLOOD_STATE state);

    // IDA_EA: 0x85BF228
    // IDA_NAME: _ZN13CBattle_Field21SetRidableObjectStateEi19eRidableObjectState
    // DEMANGLED: CBattle_Field::SetRidableObjectState(int,eRidableObjectState)
    int SetRidableObjectState(int object_id,
                              eRidableObjectState requested_state);

    CDungeon* GetDungeon();
    int IsAssignNPCByPotion();
    int getBloodState();
    int get_cur_map();
    CBattle_Field* setUltimateDifficulty(char difficulty);
    int getUltimateDifficylty();
    MapInfo* GetMapInfo();
    CBattle_Field* SetParty(CParty* party);
    CBattle_Field* SetTournamentCurRound(int round);
    int GetTournamentCurRound();
    int get_blood_round_difficylty_of_small_round(short small_round);
    int get_blood_round_of_small_round(short small_round);
    int get_blood_spawn_cnt_of_small_round(short small_round);
    int get_blood_spawn_small_round_interval(short small_round);
    int get_blood_spawn_time_of_next_small_round(short small_round);
    int get_blood_spawn_time_of_phase(short small_round);
    int get_blood_spawn_cnt_of_phase(short small_round, short phase);
    int get_blood_spawn_phase_total_cnt_of_small_round(short small_round);
    int checkBloodSpawnOrder(short small_round);
    // IDA_EA: 0x8304DD2
    // IDA_NAME: _ZN13CBattle_Field22BloodSpawnTimerManagerEbb
    char BloodSpawnTimerManager(bool finish_previous_round,
                                bool suppress_round_start);
    // IDA_EA: 0x83053D2
    // IDA_NAME: _ZN13CBattle_Field29BloodPhaseSpawnTimerResgisterEs
    int BloodPhaseSpawnTimerResgister(short small_round);
    // IDA_EA: 0x830553A
    // IDA_NAME: _ZN13CBattle_Field15BloodPhaseSpawnEsRsii
    // DEMANGLED: CBattle_Field::BloodPhaseSpawn(short,short &,int,int)
    void BloodPhaseSpawn(short small_round,
                         short& phase,
                         int timer_message,
                         int timer_slot);
    // IDA_EA: 0x8305992
    // IDA_NAME: _ZN13CBattle_Field23BloodMonsterSpawnPacketEiiss
    // DEMANGLED: CBattle_Field::BloodMonsterSpawnPacket(int,int,short,short)
    void BloodMonsterSpawnPacket(int monster_count,
                                 int phase_time_msec,
                                 short small_round,
                                 short phase);
    // IDA_EA: 0x8305D0C
    // IDA_NAME: _ZN13CBattle_Field28BloodCrazyMonsterSpawnPacketEv
    void BloodCrazyMonsterSpawnPacket();
    // IDA_EA: 0x8305E9C
    // IDA_NAME: _ZN13CBattle_Field24BloodRoundIntervalPacketEsi
    void BloodRoundIntervalPacket(short blood_round, int interval_msec);
    // IDA_EA: 0x8305F50
    // IDA_NAME: _ZN13CBattle_Field24forceValuesToChangeRoundEssi
    bool forceValuesToChangeRound(short blood_round,
                                  short small_round,
                                  int next_round_difficulty);
    // IDA_EA: 0x8306156
    // IDA_NAME: _ZN13CBattle_Field28getNextRoundRandomDifficultyEv
    int getNextRoundRandomDifficulty();
    // IDA_EA: 0x83061B6
    // IDA_NAME: _ZN13CBattle_Field22getNextRoundDifficultyEs
    int getNextRoundDifficulty(short blood_round);
    // IDA_EA: 0x8306212
    // IDA_NAME: _ZN13CBattle_Field22getNextSamllRoundOrderEsi
    int getNextSamllRoundOrder(short blood_round,
                               int next_round_difficulty);
    // IDA_EA: 0x830637C
    // IDA_NAME: _ZN13CBattle_Field28sendUltimateRandomDifficultyEiii
    void sendUltimateRandomDifficulty(int difficulty,
                                      int blood_round,
                                      int delay_seconds);
    // IDA_EA: 0x8306436
    // IDA_NAME: _ZN13CBattle_Field28sendUltimateRandomDifficultyEii
    void sendUltimateRandomDifficulty(int difficulty, int blood_round);
    // IDA_EA: 0x83064DE
    // IDA_NAME: _ZN13CBattle_Field18startUltimateRoundEv
    char startUltimateRound();
    // IDA_EA: 0x830EC12
    // IDA_NAME: _ZN13CBattle_Field16getBloodMaxRoundEv
    int getBloodMaxRound();
    int IsFirstEnterDungeon();
    CBattle_Field* ResetFirstEnterDungeon();
    int is_visited_map(int pos_x, int pos_y);
    int is_visited_map_goto(int pos_x, int pos_y);
    int get_invade_direction(int pos_x, int pos_y);
    // IDA_EA: 0x82FFB30
    // IDA_NAME: _ZN13CBattle_Field11SetGridPathEP10MazeScripti
    void SetGridPath(MazeScript* maze, int dummy_appear_count);
    int* getLastPosXY(int& pos_x, int& pos_y);
    MapInfo* set_visited_map(int pos_x, int pos_y, bool visited);
    MapInfo* set_visited_map_goto(int pos_x, int pos_y);
    int convertAPCRoleType2MonsterRoleType(char role_type) const;
    CBattle_Field* set_dungeon_diff(char difficulty);
    CMap* GetCurrentMap();
    int isWorldMapTowerArea();
    MapInfo* SetAssignNPCByPotion(bool enabled);

    struct CTowerOfDespairRewardData {
        CTowerOfDespairRewardData();
        ~CTowerOfDespairRewardData();
        void reset_tod();

        std::vector<std::pair<int, int> > today_reward_items;
        std::pair<int, int> period_reward;
    };

    // CMD042 keeps the delayed Tower-of-Despair reward payload in this
    // embedded owner.  Exposing the actual value avoids the old flattened
    // CParty +0x17a8/+0x17b4 aliases in the clear transaction.
    CTowerOfDespairRewardData& tower_reward() { return tower_reward_; }
    const CTowerOfDespairRewardData& tower_reward() const {
        return tower_reward_;
    }

    struct stConditionEnterBossRoomStatus_t {
        stConditionEnterBossRoomStatus_t();
        stConditionEnterBossRoomStatus_t(
            const stConditionEnterBossRoomStatus_t& rhs);
        ~stConditionEnterBossRoomStatus_t();
        stConditionEnterBossRoomStatus_t& operator=(
            const stConditionEnterBossRoomStatus_t& rhs);

        int condition_type;
        std::vector<std::pair<int, int> > condition_values;
        int boss_room_id;
        int status;
    };

    struct stEventMonsterStatus_t {
        stEventMonsterStatus_t();
        ~stEventMonsterStatus_t();

        int monster_id;                         // +0x00
        std::pair<int, int> map_position;      // +0x04
        int status_value;                       // +0x0c
        int status_first;                       // +0x10
        int status_second;                      // +0x14
    };

    int get_dungeon_diff();
    int get_dungeon_index();
    // IDA_EA: 0x830AAA4 / 0x830AC7A
    // IDA_NAME: _ZN13CBattle_Field24getRandomBuffDungeonTypeEi
    //           _ZN13CBattle_Field19setEplpWaitingTimerEN10QuickParty17RandomBuffDungeonE
    int getRandomBuffDungeonType(int dungeon_index);
    WongWork::CDeathTower* setEplpWaitingTimer(
        QuickParty::RandomBuffDungeon random_buff_dungeon);
    int getMapWidth();
    int getMapHeight();
    int* getCurPosXY(int& pos_x, int& pos_y);
    // RECONSTRUCTED_0828: the dungeon-mission packet owner needs real maze
    // cells, not client-supplied coordinates.  This layout-neutral member
    // keeps access to maze_/start/boss inside CBattle_Field while returning
    // only validated u8-addressable cells to the feature state machine.
    bool BuildDungeonMissionCoordinates(
        int mission_type,
        int requested_secondary_count,
        std::pair<int, int>& npc_position,
        std::pair<int, int>& primary,
        std::vector<std::pair<int, int> >& secondary) const;
    bool check_start_point();
    bool check_end_point();
    CBattle_Field* SetCurSeatNo(int seat);
    int IsClearDungeon();
    int get_map_index(int pos_x, int pos_y);
    int IsVisitedCurrentMap();
    MapInfo* GetLastMapInfo();
    CBattle_Field* setWorldMapTowerArea(bool tower_area);
    int GetCurPosX();
    int GetCurPosY();
    bool reset_hell_party_value();
    CBattle_Field* reset_blood_values();
    CBattle_Field* reset_field();

    // CParty::game_start reads this real CBattle_Field value at +0x1e8 to
    // form its hell-party percentage packet.  The recovered name is neutral
    // because the IDB preserves its role but not a stronger field label.
    int& hell_party_value_max() { return hell_party_value_max_; }
    const int& hell_party_value_max() const {
        return hell_party_value_max_;
    }

    // CParty::cal_hellparty_value consumes the real clear-time table below.
    // The original indices are bounded by dungeon difficulty (0..4), member
    // count minus one (0..3), and get_rate_level_difference (1..3).
    int& hell_party_clear_base_value() {
        return hell_party_clear_base_value_;
    }
    float hell_party_level_rate(unsigned int index) const {
        return hell_party_level_rates_[index];
    }
    float hell_party_difficulty_rate(unsigned int index) const {
        return hell_party_difficulty_rates_[index];
    }
    float hell_party_member_rate(unsigned int index) const {
        return hell_party_member_rates_[index];
    }
    float hell_party_extra_clear_rate() const {
        return hell_party_extra_clear_rate_;
    }
    int hell_party_min_value() const { return hell_party_min_value_; }
    float hell_party_sub_rate() const { return hell_party_sub_rate_; }
    bool skip_hell_party_clear_accumulation() const {
        return hell_party_enabled_ == 1 ||
               all_hell_groups_killed_ == 1 ||
               small_maze_hell_party_suppressed_ == 1;
    }

    // TimerVillageAttackedPlayTimeout @0x86390B1 compares its packet token
    // with this exact 16-bit village-monster UID at CBattle_Field +0x128.
    uint16_t village_monster_timer_token() const {
        return village_monster_uid_;
    }
    // CParty::check_allmember_loading @0x85B17A4 places this exact value in
    // NOTI030.  The village-monster entry path writes the four adjacent
    // context fields at CParty +0xc4c..+0xc58 before starting the dungeon.
    int village_monster_route() const { return village_monster_route_; }
    int village_monster_village() const { return village_monster_village_; }
    int village_monster_area() const { return village_monster_area_; }
    void set_village_monster_battle_context(uint16_t monster_uid,
                                            int village,
                                            int area,
                                            int route,
                                            int dungeon_index) {
        village_monster_uid_ = monster_uid;
        village_monster_village_ = village;
        village_monster_area_ = area;
        village_monster_route_ = route;
        dungeon_index_ = dungeon_index;
    }

    // This is the physical CParty +0xcd8 / CBattle_Field +0x1b4 mode tested
    // by CBossStage::handleKillMonster.  It has no separately exported ABI;
    // CMD16 is its original writer.
    int instance_dungeon_mode() const { return instance_dungeon_mode_; }
    CBattle_Field* set_instance_dungeon_mode(int mode) {
        instance_dungeon_mode_ = mode;
        return this;
    }
    // CUser::GetItemSerialChecker compares the pickup request against these
    // physical CBattle_Field +0x1bc/+0x1c8 values.  Naming them here keeps
    // that anti-automation predicate out of a flattened CParty offset.
    int monster_drop_source_map() const { return quest_stage_map_index_; }
    uint8_t monster_drop_event_flag() const {
        return quest_stage_map_selected_;
    }

    // IDA 0x81CD922..0x81CD9D4 mutates these exact physical members while
    // processing CMD120.  Keep access typed and layout-neutral for the
    // dispatcher; the members themselves remain at +0x118 and +0x124.
    std::vector<int>& map_order() { return map_order_; }
    const std::vector<int>& map_order() const { return map_order_; }
    int32_t& boss_die_check_started_at_sec() {
        return boss_die_check_started_at_sec_;
    }
    const int32_t& boss_die_check_started_at_sec() const {
        return boss_die_check_started_at_sec_;
    }

    // IDA_EA: 0x830766E..0x8308882.  These are genuine private member
    // functions in the 0627 binary; declaring them on the owner class keeps
    // their physical state access in normal C++ rather than asm-name facades.
    void moveMap_1_Log(int pos_x, int pos_y);
    void moveMap_2_Log(int pos_x, int pos_y);
    void moveMap_3_Log(int pos_x, int pos_y);
    void moveMap_4_Log(int pos_x, int pos_y, const GridScript* grid);
    int check_boss_map(int pos_x, int pos_y);
    int check_movemap_range(int pos_x, int pos_y);
    CBattle_Field* on_not_visited_map(int pos_x, int pos_y, bool& layered);
    int check_visited_map(int pos_x, int pos_y, bool& layered);
    void choose_random_bossmap_from_dummymaps(
        const CDungeon* dungeon,
        const GridScript* grid,
        int& selected_map,
        bool& visited,
        int pos_x,
        int pos_y,
        int invade_direction);
    int get_candidator_map(const CDungeon* dungeon,
                           const GridScript* grid,
                           int& selected_map,
                           std::vector<int>& candidates,
                           int pos_x,
                           int pos_y,
                           int invade_direction);
    CBattle_Field* swap_curpos_xy(int pos_x, int pos_y);
    int set_map_id(const GridScript* grid,
                   int& selected_map,
                   std::vector<int>& candidates);
    MazeScript* GetAppropriateMaze(
        ENUM_DUNGEON_DIFFICULTY difficulty) const;
    int CheckHellPartyCondition();
    int GetHellPartyDifficulty(ENUM_DUNGEON_TYPE dungeon_type);
    int GetHellPartyEventType(char difficulty);
    bool setQuestMazeInfo();

private:
    CParty* party_;                                      // +0x000
    std::pair<int, int> maze_start_position_;             // +0x004
    std::pair<int, int> maze_boss_position_;              // +0x00c
    std::pair<int, int> maze_boss_neighbor_position_;     // +0x014
    std::pair<int, int> start_positions_[4];              // +0x01c
    std::pair<int, int> last_positions_[4];               // +0x03c
    uint8_t initial_map_flag_;                            // +0x05c
    uint8_t pad_05d_05f_[3];
    std::pair<int, int> current_position_;                // +0x060
    std::pair<int, int> previous_position_;               // +0x068
    int32_t champion_spawn_rate_;                       // +0x070
    // IDA C2/C1 storage at +0x074 is consumed by getMaze() and the
    // general-map-to-layered-map selection path.  Keep the actual script
    // pointer visible instead of making map lookup recover it from bytes.
    MazeScript* maze_;                                    // +0x074
    std::map<int, MapInfo> map_infos_;                     // +0x078
    Mutex map_info_mutex_;                                 // +0x090
    Mutex state_mutex_;                                    // +0x0a8
    std::vector<std::pair<int, int> > map_path_;           // +0x0c0
    uint32_t map_path_index_;                              // +0x0cc
    CRidable ridable_;                                     // +0x0d0
    CClearCondition clear_condition_;                      // +0x0f0
    CMap* current_map_;                                  // +0x108
    int32_t layered_map_index_;                           // +0x10c
    uint8_t quest_maze_;                                  // +0x110
    uint8_t pad_111_113_[3];
    int32_t current_seat_no_;                             // +0x114
    std::vector<int> map_order_;                           // +0x118
    // IDA 0x81CD99A records the second in which CMD120 removes the last
    // map-order monster; the same value is passed as TimerEntry::param1 at
    // 0x81CD9D4 for timer message 100.
    int32_t boss_die_check_started_at_sec_;                // +0x124
    uint16_t village_monster_uid_;                     // +0x128
    uint8_t raw_12a_12b_[0x02];
    int32_t village_monster_village_;                  // +0x12c
    int32_t village_monster_area_;                     // +0x130
    int32_t village_monster_route_;                    // +0x134
    // TimerElevatorDungeon's original callback advances these two physical
    // bytes at +0x138/+0x139 before it requeues message 40.
    uint8_t elevator_timer_state_;                      // +0x138
    uint8_t elevator_timer_count_;                      // +0x139
    // reset_field arms this one-shot map-start timer. CParty consumes it
    // only for dungeon 53 before queueing TimerElevatorDungeon.
    uint8_t elevator_timer_armed_;                      // +0x13a
    uint8_t raw_13b_;
    std::map<short, std::vector<TournamentInfo> >
        tournament_infos_;                                 // +0x13c
    int32_t tournament_cur_round_;                       // +0x154
    CDungeonClearTracer dungeon_clear_tracer_;         // +0x158
    std::vector<stConditionEnterBossRoomStatus_t>
        boss_room_conditions_;                             // +0x170
    std::vector<stEventMonsterStatus_t> event_monsters_;  // +0x17c
    CDungeon* dungeon_;                                  // +0x188
    int32_t map_width_;                                // +0x18c
    int32_t map_height_;                               // +0x190
    int32_t dungeon_index_;                            // +0x194
    int32_t party_member_count_;                       // +0x198
    int32_t dungeon_select_parameter_;                 // +0x19c
    int32_t dungeon_difficulty_;                       // +0x1a0
    int32_t runtime_value_1a4_;                        // +0x1a4
    // kill_monster @0x830BC78 increments this before NOTI38 for converted
    // monster role 1 (99% roll).  CParty::kill_monster also sends this same
    // physical dword in NOTI50/KEY_COUNT, proving that it is dungeon key
    // progression rather than a generic death-notification sequence.
    int32_t key_count_;                                 // +0x1a8
    int32_t selected_maze_index_;                     // +0x1ac
    int32_t dungeon_type_value_;                       // +0x1b0
    int32_t instance_dungeon_mode_;                      // +0x1b4
    uint8_t hell_party_enabled_;                       // +0x1b8
    uint8_t raw_1b9_1bb_[0x03];
    int32_t quest_stage_map_index_;                     // +0x1bc
    int32_t quest_stage_pos_x_;                         // +0x1c0
    int32_t quest_stage_pos_y_;                         // +0x1c4
    uint8_t quest_stage_map_selected_;                  // +0x1c8
    uint8_t hell_party_runtime_pending_;                // +0x1c9
    uint8_t all_hell_groups_killed_;                    // +0x1ca
    uint8_t small_maze_hell_party_suppressed_;          // +0x1cb
    int8_t hell_party_difficulty_;                     // +0x1cc
    uint8_t hell_party_event_type_;                     // +0x1cd
    uint8_t hell_party_requested_;                      // +0x1ce
    uint8_t hell_party_runtime_flag_;                   // +0x1cf
    uint8_t hell_party_runtime_values_[0x18];           // +0x1d0
    int32_t hell_party_value_max_;                       // +0x1e8
    int32_t hell_party_min_value_;               // +0x1ec
    uint8_t raw_1f0_1f3_[0x04];
    int32_t hell_party_clear_base_value_;        // +0x1f4
    float hell_party_sub_rate_;                  // +0x1f8
    float hell_party_pass_sub_rate_;                    // +0x1fc
    float hell_party_level_rates_[4];            // +0x200
    float hell_party_difficulty_rates_[5];       // +0x210
    float hell_party_member_rates_[4];           // +0x224
    float hell_party_extra_clear_rate_;          // +0x234
    int32_t dimension_level_;                           // +0x238
    uint8_t world_map_tower_area_;                      // +0x23c
    uint8_t raw_23d_23f_[3];
    std::vector<STAssignBloodSystem> blood_systems_;       // +0x240
    std::vector<STAssignBloodPhaseTime> blood_phase_times_; // +0x24c
    std::vector<short> blood_rounds_;                      // +0x258
    std::vector<bool> blood_flags_;                        // +0x264
    // i386 libstdc++ represents vector<bool> as 0x14 bytes here, so the
    // following scalar gap begins at +0x278 rather than +0x270.
    struct BloodSpawnRuntimeState {
        int16_t registered_small_rounds[11];            // +0x00
        int16_t phase_finished[11];                     // +0x16
        int32_t selected_system_monster_id;             // +0x2c
        uint16_t selected_small_round;                  // +0x30
        uint8_t selected_variant;                       // +0x32
        uint8_t ultimate_difficulty;                    // +0x33
    } blood_spawn_runtime_;                             // +0x278
    CBloodRound blood_round_;                            // +0x2ac
    int32_t blood_state_;                               // +0x2b4
    CBloodClearRewardData blood_clear_reward_;             // +0x2b8
    CTowerOfDespairRewardData tower_reward_;               // +0xc84
    CTournamentDungeonReward tournament_reward_;           // +0xc98

    // CParty owns this physical aggregate at +0x0b24 and performs the
    // original map-load initialization directly on the three elevator bytes.
    // Friendship models that existing ownership without a wrapper ABI.
    friend class CParty;
    friend class CUser;
    friend class TimerBloodPhaseRegister0;
    friend class TimerBloodPhaseRegister1;
    friend class TimerBloodPhaseRegister2;
    friend class TimerBloodPhaseRegister3;
    friend class TimerBloodPhaseRegister4;
    friend class TimerBloodPhaseRegister5;
    friend class TimerBloodPhaseRegister6;
    friend class TimerBloodPhaseRegister7;
    friend class TimerBloodPhaseRegister8;
    friend class TimerBloodPhaseRegister9;
    friend struct CBattleFieldLayoutAssert;
};

static_assert(sizeof(CBattle_Field::CBloodRound) == 0x08u,
              "CBloodRound IDA ABI size must stay 0x08");

struct CBattleFieldLayoutAssert {
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
    static_assert(offsetof(CBattle_Field, party_) == 0x000u,
                  "CBattle_Field owning party must stay at +0x000");
    static_assert(sizeof(CBattle_Field::BloodSpawnRuntimeState) == 0x34u,
                  "CBattle_Field blood-spawn state size must stay 0x34");
    static_assert(offsetof(CBattle_Field::BloodSpawnRuntimeState,
                           phase_finished) == 0x16u,
                  "blood phase-finished table must stay at +0x16");
    static_assert(offsetof(CBattle_Field::BloodSpawnRuntimeState,
                           selected_system_monster_id) == 0x2cu,
                  "blood selected system must stay at +0x2c");
    static_assert(offsetof(CBattle_Field, current_map_) == 0x108u,
                  "CBattle_Field current map must stay at +0x108");
    static_assert(offsetof(CBattle_Field, maze_start_position_) == 0x004u,
                  "CBattle_Field maze start must stay at +0x004");
    static_assert(offsetof(CBattle_Field, maze_boss_position_) == 0x00cu,
                  "CBattle_Field maze boss must stay at +0x00c");
    static_assert(offsetof(CBattle_Field, current_position_) == 0x060u,
                  "CBattle_Field current position must stay at +0x060");
    static_assert(offsetof(CBattle_Field, previous_position_) == 0x068u,
                  "CBattle_Field previous position must stay at +0x068");
    static_assert(offsetof(CBattle_Field, village_monster_uid_) == 0x128u,
                  "CBattle_Field village-monster UID must stay at +0x128");
    static_assert(offsetof(CBattle_Field, village_monster_village_) == 0x12cu,
                  "CBattle_Field village-monster village must stay at +0x12c");
    static_assert(offsetof(CBattle_Field, village_monster_area_) == 0x130u,
                  "CBattle_Field village-monster area must stay at +0x130");
    static_assert(offsetof(CBattle_Field, village_monster_route_) == 0x134u,
                  "CBattle_Field village-monster route must stay at +0x134");
    static_assert(offsetof(CBattle_Field, elevator_timer_state_) == 0x138u,
                  "CBattle_Field elevator timer state must stay at +0x138");
    static_assert(offsetof(CBattle_Field, elevator_timer_count_) == 0x139u,
                  "CBattle_Field elevator timer count must stay at +0x139");
    static_assert(offsetof(CBattle_Field, dungeon_) == 0x188u,
                  "CBattle_Field dungeon must stay at +0x188");
    static_assert(offsetof(CBattle_Field, dungeon_difficulty_) == 0x1a0u,
                  "CBattle_Field dungeon difficulty must stay at +0x1a0");
    static_assert(offsetof(CBattle_Field, key_count_) == 0x1a8u,
                  "CBattle_Field dungeon key count must stay at +0x1a8");
    static_assert(offsetof(CBattle_Field, selected_maze_index_) == 0x1acu,
                  "CBattle_Field selected maze must stay at +0x1ac");
    static_assert(offsetof(CBattle_Field, quest_stage_map_index_) == 0x1bcu,
                  "CBattle_Field quest-stage map must stay at +0x1bc");
    static_assert(offsetof(CBattle_Field, quest_stage_map_selected_) == 0x1c8u,
                  "CBattle_Field quest-stage flag must stay at +0x1c8");
    static_assert(offsetof(CBattle_Field, all_hell_groups_killed_) == 0x1cau,
                  "CBattle_Field hell-group completion must stay at +0x1ca");
    static_assert(offsetof(CBattle_Field, world_map_tower_area_) == 0x23cu,
                  "CBattle_Field tower-area flag must stay at +0x23c");
    static_assert(offsetof(CBattle_Field, blood_spawn_runtime_) == 0x278u,
                  "CBattle_Field blood-spawn state must stay at +0x278");
    static_assert(offsetof(CBattle_Field, blood_round_) == 0x2acu,
                  "CBattle_Field blood round must stay at +0x2ac");
    static_assert(offsetof(CBattle_Field, blood_state_) == 0x2b4u,
                  "CBattle_Field blood state must stay at +0x2b4");
    static_assert(offsetof(CBattle_Field, map_infos_) == 0x78u,
                  "CBattle_Field MapInfo map must begin at +0x78");
    static_assert(offsetof(CBattle_Field, blood_clear_reward_) == 0x2b8u,
                  "CBattle_Field blood reward must begin at +0x2b8");
    static_assert(offsetof(CBattle_Field, tower_reward_) == 0xc84u,
                  "CBattle_Field tower reward must begin at +0xc84");
    static_assert(offsetof(CBattle_Field, tournament_reward_) == 0xc98u,
                  "CBattle_Field tournament reward must begin at +0xc98");
    static_assert(sizeof(CBattle_Field) == 0xcd0u,
                  "CBattle_Field IDA ABI size must stay 0xcd0");
#endif
};

// IDA unit: 99_misc_global/CTraceMobDieHack.  These layouts are established
// by the constructor at 0x82A517C and are shared by CParty's embedded trace
// state.  Keep the real container value types rather than byte-array views.
struct stMonsterDamageLog_t {
    uint16_t monster_id;
    uint16_t source_id;
    int occurrence_count;
    int damage_value;
};

struct stHackLog_t {
    uint32_t account_id;
    uint32_t charac_no;
    uint16_t hack_type;
    uint16_t raw_0a;
    int detail_value;
    int report_count;
};

class CTraceMobDieHack {
public:
    struct TraceUserTiming {
        CUser* user;
        int elapsed_seconds;
        int sample_count;
    };

    // IDA_EA: 0x82A517C
    // IDA_NAME: _ZN16CTraceMobDieHackC2Ev
    CTraceMobDieHack();
    // IDA_EA: 0x82A5236
    // IDA_NAME: _ZN16CTraceMobDieHackD2Ev
    ~CTraceMobDieHack();

    // IDA_EA: 0x82A50B0
    // IDA_NAME: _ZN16CTraceMobDieHack5resetEv
    // DEMANGLED: CTraceMobDieHack::reset(void)
    bool reset();

    // IDA_EA: 0x8599CB2
    // IDA_NAME: _ZN16CTraceMobDieHack14reportHackInfoEv
    // DEMANGLED: CTraceMobDieHack::reportHackInfo(void)
    bool reportHackInfo();

    // IDA_EA: 0x8599A88 / 0x85BF4F6 / 0x85BF628 / 0x85BF646
    // IDA_NAME: _ZN16CTraceMobDieHack8traceEndEP5CUserPS1_ /\
    //           _ZN16CTraceMobDieHack16addMonsterDamageEiti /\
    //           _ZN16CTraceMobDieHack12setMobKillerEi /\
    //           _ZN16CTraceMobDieHack14setDungeonDiffEi
    bool traceEnd(CUser* manager, CUser* const users[4]);
    int addMonsterDamage(int damage_value,
                         unsigned short monster_type,
                         int monster_id);
    void setMobKiller(int monster_id);
    CTraceMobDieHack* setDungeonDiff(int dungeon_diff);

    // IDA_EA: 0x85BF4CC
    // IDA_NAME: _ZN16CTraceMobDieHack10traceBeginElb
    // DEMANGLED: CTraceMobDieHack::traceBegin(long,bool)
    CTraceMobDieHack* traceBegin(long begin_second, bool special);

    int trace_begin_second;
    std::map<unsigned short, stMonsterDamageLog_t> monster_damage_log;
    std::map<unsigned short, stMonsterDamageLog_t> secondary_damage_log;
    std::vector<int> mob_killers;
    std::vector<stHackLog_t> hack_logs;
    bool trace_active;
    TraceUserTiming trace_users[4];
    bool special_dungeon;
    int dungeon_diff;
};

static_assert(sizeof(stMonsterDamageLog_t) == 0x0cu,
              "stMonsterDamageLog_t IDA ABI size must stay 0x0c");
static_assert(sizeof(stHackLog_t) == 0x14u,
              "stHackLog_t IDA ABI size must stay 0x14");
static_assert(sizeof(CTraceMobDieHack::TraceUserTiming) == 0x0cu,
              "CTraceMobDieHack TraceUserTiming ABI must stay 0x0c");
static_assert(sizeof(CTraceMobDieHack) == 0x88u,
              "CTraceMobDieHack IDA ABI size must stay 0x88");

// BattleData embeds GameResultType at +0x690, so the two complete value
// layouts must appear before the aggregate rather than later in this header.
struct DungeonUserShotCount {
    DungeonUserShotCount();
    DungeonUserShotCount* reset();

    uint16_t shot_count;
    uint8_t reserved_02[2];
    uint32_t raw_04;
};

// IDA unit: 99_misc_global/GameResultType.
struct GameResultType {
    // IDA_EA: 0x84B73D4
    // IDA_NAME: _ZN14GameResultTypeC2Ev
    GameResultType();
    // IDA_EA: 0x84B7416
    // IDA_NAME: _ZN14GameResultTypeD2Ev
    ~GameResultType();
    int Set(const GameResultType& source);
    GameResultType* Clear();
    int UpdateHighScore(const GameResultType& source);
    int GetUniqueKey();

    uint8_t raw_00;
    uint8_t reserved_01[3];
    int32_t raw_04;
    uint8_t raw_08;
    uint8_t raw_09;
    uint8_t raw_0a;
    uint8_t reserved_0b;
    int32_t raw_0c;
    uint8_t raw_10;
    uint8_t reserved_11[3];
    DungeonUserShotCount shot_counts[4];
    uint16_t score_values[8];
    int32_t best_time;
    uint16_t raw_48;
    uint16_t raw_4a;
    int32_t raw_4c;
};

class BattleData {
public:
    // IDA_EA: 0x82A4CEA
    // IDA_NAME: _ZN10BattleDataC2Ev
    // DEMANGLED: BattleData::BattleData(void)
    BattleData();
    // IDA_EA: 0x82A54D6
    // IDA_NAME: _ZN10BattleDataD2Ev
    // DEMANGLED: BattleData::~BattleData()
    ~BattleData();

    BattleData* SetHellPartyValueTotal(int value);
    int GetHellPartyValueTotal();
    bool Reset();

    // IDA_EA: 0x85BE6BA
    // IDA_NAME: _ZN10BattleData27ResetEventDungeonClearPointEv
    // DEMANGLED: BattleData::ResetEventDungeonClearPoint(void)
    // The event-dungeon clear counters occupy the physical +0x62c..+0x68b
    // range and must be cleared after each eligible party member's reward
    // pass, exactly as CParty::dungeon_clear does in the old binary.
    void* ResetEventDungeonClearPoint();

    // These values physically belong to BattleData, despite older CParty
    // consumers having addressed them through flattened CParty offsets.
    int& hell_party_initial_value() { return hell_party_initial_value_; }
    const int& hell_party_initial_value() const {
        return hell_party_initial_value_;
    }
    // CParty::ClearDungeon @0x85A9330 stores the whole-dungeon elapsed time
    // in the first BattleData qword (old flattened CParty +0x32c).  Normal
    // SetPlayResult consumes its low dword and TOD CMD347 serializes the same
    // physical value.  This is distinct from map_playing_time_ at +0x08,
    // which is reset whenever the party changes map.
    uint64_t& dungeon_playing_time() { return dungeon_playing_time_; }
    const uint64_t& dungeon_playing_time() const {
        return dungeon_playing_time_;
    }
    int tod_clear_reward_packet_value() const {
        return static_cast<int>(dungeon_playing_time_);
    }
    uint64_t& map_playing_time() { return map_playing_time_; }
    const uint64_t& map_playing_time() const { return map_playing_time_; }
    uint64_t& map_pause_time() { return map_pause_time_; }
    const uint64_t& map_pause_time() const { return map_pause_time_; }
    uint64_t& map_start_time() { return map_start_time_; }
    const uint64_t& map_start_time() const { return map_start_time_; }
    uint32_t& map_tick_211() { return map_tick_211_; }
    uint32_t& map_tick_212() { return map_tick_212_; }
    uint32_t& map_tick_214() { return map_tick_214_; }
    uint32_t& map_tick_215() { return map_tick_215_; }
    uint32_t& map_tick_216() { return map_tick_216_; }
    uint32_t& map_tick_217() { return map_tick_217_; }
    uint64_t dungeon_start_tick() const {
        return static_cast<uint64_t>(map_tick_211_) |
            (static_cast<uint64_t>(map_tick_212_) << 32u);
    }
    uint64_t dungeon_pause_time() const {
        return static_cast<uint64_t>(map_tick_214_) |
            (static_cast<uint64_t>(map_tick_215_) << 32u);
    }
    uint8_t& dungeon_start_statistic_flag(unsigned int index) {
        return dungeon_start_statistic_flags_[index];
    }
    const uint8_t& dungeon_start_statistic_flag(unsigned int index) const {
        return dungeon_start_statistic_flags_[index];
    }
    // CParty::clear_reward @ 0x85AE0D4 consumes the low words of the
    // otherwise opaque BattleData +0x040/+0x044/+0x048 records as the
    // clear-gold generator's weights +0x0a/+0x0e/+0x08, respectively.
    // Keep the original ownership in BattleData; callers must not recreate
    // the former flattened CParty offsets.
    uint16_t clear_reward_gold_weight_08() const {
        return static_cast<uint16_t>(monster_drop_limit_accepted_count_);
    }
    uint16_t clear_reward_gold_weight_0a() const {
        return static_cast<uint16_t>(monster_role_1_kill_count_);
    }
    uint16_t clear_reward_gold_weight_0e() const {
        return static_cast<uint16_t>(monster_role_3_kill_count_);
    }
    // IDA_EA: 0x85BF456
    // IDA_NAME: _ZNK10BattleData26GetTotalKilledMonsterCountEv
    // DEMANGLED: BattleData::GetTotalKilledMonsterCount(void)const
    int GetTotalKilledMonsterCount() const {
        return static_cast<int>(
            static_cast<uint32_t>(monster_role_other_kill_count_) +
            static_cast<uint32_t>(monster_role_1_kill_count_) +
            static_cast<uint32_t>(monster_role_3_kill_count_));
    }
    uint8_t& member_finish_loading(int seat) {
        return member_finish_loading_[seat];
    }
    const uint8_t& member_finish_loading(int seat) const {
        return member_finish_loading_[seat];
    }
    // CParty::finish_loading @ 0x85B15E0 separately records whether this
    // member has received the start-map acknowledgement.  The four bytes are
    // BattleData +0x7e4; the older generic name hid CMD040 ownership.
    uint8_t& member_start_map_finish_loading(int seat) {
        return member_start_map_finish_loading_[seat];
    }
    const uint8_t& member_start_map_finish_loading(int seat) const {
        return member_start_map_finish_loading_[seat];
    }
    float& hell_party_value_total() { return hell_party_value_total_; }
    const float& hell_party_value_total() const {
        return hell_party_value_total_;
    }
    uint8_t& member_live(int seat) { return member_live_[seat]; }
    const uint8_t& member_live(int seat) const { return member_live_[seat]; }
    uint8_t& member_enter_map(int seat) { return member_enter_map_[seat]; }
    const uint8_t& member_enter_map(int seat) const {
        return member_enter_map_[seat];
    }
    uint8_t& member_state_flag(int seat) { return member_state_flag_[seat]; }
    const uint8_t& member_state_flag(int seat) const {
        return member_state_flag_[seat];
    }
    int& member_map_move_count(int seat) {
        return member_map_move_count_[seat];
    }
    const int& member_map_move_count(int seat) const {
        return member_map_move_count_[seat];
    }
    int& map_move_count() { return map_move_count_; }
    const int& map_move_count() const { return map_move_count_; }
    // CParty::OnUsedSkill owns the state transition but the per-seat
    // counters physically reside in its embedded BattleData object.
    int& skill_use_count(int seat) { return skill_use_counts_[seat]; }
    const int& skill_use_count(int seat) const {
        return skill_use_counts_[seat];
    }
    // CMD044 consumes one of these per-seat limits for a paid dungeon
    // revival.  BattleData::Reset restores every member's limit to INT_MAX.
    int& member_coin_limit_remaining(int seat) {
        return member_coin_limit_remaining_[seat];
    }
    const int& member_coin_limit_remaining(int seat) const {
        return member_coin_limit_remaining_[seat];
    }
    // CParty::SendEachCardInfo serializes these exact four bytes after the
    // card-reward payload.  No original field identifier survived, so keep
    // the accessor scoped to the observed packet role rather than exposing
    // the storage as an arbitrary CParty offset.
    uint8_t& card_info_member_state(int seat) {
        return raw_060_063_[seat];
    }
    const uint8_t& card_info_member_state(int seat) const {
        return raw_060_063_[seat];
    }
    // IDA: CParty::_putItemIntoUser @0x85AAB84 and dungeon_clear @0x85AB736
    // consume each of these physical ItemPair members as a contiguous
    // money-record/item-record pair.  Keep the pairing in BattleData rather
    // than exposing its former flattened CParty offsets to reward code.
    Inven_Item& normal_reward_item(int seat, int reward_slot) {
        return reward_slot == 0 ? normal_rewards_[seat].first
                                : normal_rewards_[seat].second;
    }
    const Inven_Item& normal_reward_item(int seat, int reward_slot) const {
        return reward_slot == 0 ? normal_rewards_[seat].first
                                : normal_rewards_[seat].second;
    }
    Inven_Item& special_reward_item(int seat, int reward_slot) {
        return reward_slot == 0 ? special_rewards_[seat].first
                                : special_rewards_[seat].second;
    }
    const Inven_Item& special_reward_item(int seat, int reward_slot) const {
        return reward_slot == 0 ? special_rewards_[seat].first
                                : special_rewards_[seat].second;
    }
    Inven_Item& event_reward_item(int seat, int reward_slot) {
        return reward_slot == 0 ? event_rewards_[seat].first
                                : event_rewards_[seat].second;
    }
    const Inven_Item& event_reward_item(int seat, int reward_slot) const {
        return reward_slot == 0 ? event_rewards_[seat].first
                                : event_rewards_[seat].second;
    }

private:
    struct ItemPair {
        Inven_Item first;
        Inven_Item second;
    };

    uint64_t dungeon_playing_time_;            // +0x000
    union {
        // CParty::game_start initializes this low dword from the hell-party
        // script.  The same old storage is later read as the elapsed map
        // time, so preserve the proven alias as a real union.
        int hell_party_initial_value_;        // +0x008
        uint64_t map_playing_time_;           // +0x008
    };
    uint64_t map_pause_time_;                 // +0x010
    uint64_t map_start_time_;                 // +0x018
    uint32_t map_tick_211_;                   // +0x020
    uint32_t map_tick_212_;                   // +0x024
    // CParty::dungeon_start @ 0x85B929C writes these four bytes through its
    // old flattened +0x354..+0x357 addresses (BattleData +0x28..+0x2b).
    // The binary does not name their independent statistics, so retain the
    // proven ordinal names rather than assigning a speculative meaning.
    uint8_t dungeon_start_statistic_flags_[0x04];
    uint32_t map_tick_214_;                   // +0x02c
    uint32_t map_tick_215_;                   // +0x030
    uint32_t map_tick_216_;                   // +0x034
    uint32_t map_tick_217_;                   // +0x038
    // CParty::kill_monster @0x85A27E8 classifies every successful kill
    // through convertAPCRoleType2MonsterRoleType and updates these five
    // physical dwords.  The first three feed clear EXP; the last two enforce
    // the per-map drop-count limit and the accepted count also feeds card
    // reward weight 0x08.
    int32_t monster_role_other_kill_count_;        // +0x03c
    int32_t monster_role_1_kill_count_;            // +0x040
    int32_t monster_role_3_kill_count_;            // +0x044
    int32_t monster_drop_limit_accepted_count_;    // +0x048
    int32_t monster_drop_limit_candidate_count_;   // +0x04c
    uint8_t member_finish_loading_[4];        // +0x050
    uint8_t member_live_[4];                  // +0x054
    uint8_t member_enter_map_[4];             // +0x058
    uint8_t member_state_flag_[4];            // +0x05c
    uint8_t raw_060_063_[4];
    int member_map_move_count_[4];            // +0x064
    ItemPair normal_rewards_[4];            // +0x074
    ItemPair special_rewards_[4];           // +0x25c
    ItemPair event_rewards_[4];             // +0x444
    uint8_t raw_62c_68b_[0x60];
    int map_move_count_;                     // +0x68c
    GameResultType game_results_[4];        // +0x690
    // CParty::OnUsedSkill @ 0x85B5F8E increments this exact per-seat
    // counter through the old flattened CParty +0xafc expression.  It is
    // BattleData +0x7d0, reset by BattleData::Reset @ 0x82A4E8A.
    int32_t skill_use_counts_[4];            // +0x7d0
    float hell_party_value_total_;           // +0x7e0
    uint8_t member_start_map_finish_loading_[4];  // +0x7e4
    int32_t member_coin_limit_remaining_[4]; // +0x7e8

    friend class CParty;
    friend struct BattleDataLayoutAssert;
};

struct GameResultSet : public GameResultType {
    // IDA_EA: 0x84B75D2
    // IDA_NAME: _ZN13GameResultSetC2Ev
    GameResultSet();
    // IDA_EA: 0x84B75E6
    // IDA_NAME: _ZN13GameResultSetD2Ev
    ~GameResultSet();
    // IDA_EA: 0x84B75FA
    // IDA_NAME: _ZN13GameResultSet5ClearEv
    GameResultSet* Clear();
};

struct BattleDataLayoutAssert {
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
    static_assert(offsetof(BattleData, dungeon_playing_time_) == 0x00u,
                  "BattleData dungeon-playing time must begin at +0x00");
    static_assert(offsetof(BattleData, map_playing_time_) == 0x08u,
                  "BattleData map-playing time must begin at +0x08");
    static_assert(sizeof(BattleData::ItemPair) == 0x7au,
                  "BattleData Inven_Item pair must remain 0x7a");
    static_assert(offsetof(BattleData, normal_rewards_) == 0x74u,
                   "BattleData normal rewards must begin at +0x74");
    static_assert(offsetof(BattleData, map_pause_time_) == 0x10u,
                  "BattleData map-pause time must begin at +0x10");
    static_assert(offsetof(BattleData, map_start_time_) == 0x18u,
                  "BattleData map-start time must begin at +0x18");
    static_assert(offsetof(BattleData, map_tick_211_) == 0x20u,
                  "BattleData map tick 211 must begin at +0x20");
    static_assert(offsetof(BattleData, map_tick_214_) == 0x2cu,
                  "BattleData accumulated dungeon pause must begin at +0x2c");
    static_assert(offsetof(BattleData, map_tick_216_) == 0x34u,
                  "BattleData loading start tick must begin at +0x34");
    static_assert(offsetof(BattleData, monster_role_other_kill_count_) ==
                      0x3cu,
                  "BattleData other-role kill count must begin at +0x3c");
    static_assert(offsetof(BattleData, monster_role_1_kill_count_) == 0x40u,
                  "BattleData role-1 kill count must begin at +0x40");
    static_assert(offsetof(BattleData, monster_role_3_kill_count_) == 0x44u,
                  "BattleData role-3 kill count must begin at +0x44");
    static_assert(offsetof(BattleData, monster_drop_limit_accepted_count_) ==
                      0x48u,
                  "BattleData accepted drop count must begin at +0x48");
    static_assert(offsetof(BattleData, monster_drop_limit_candidate_count_) ==
                      0x4cu,
                  "BattleData candidate drop count must begin at +0x4c");
    static_assert(offsetof(BattleData, member_finish_loading_) == 0x50u,
                  "BattleData finish-loading flags must begin at +0x50");
    static_assert(offsetof(BattleData, member_live_) == 0x54u,
                  "BattleData live-member flags must begin at +0x54");
    static_assert(offsetof(BattleData, map_move_count_) == 0x68cu,
                  "BattleData map-move count must begin at +0x68c");
    static_assert(offsetof(BattleData, game_results_) == 0x690u,
                   "BattleData results must begin at +0x690");
    static_assert(offsetof(BattleData, hell_party_value_total_) == 0x7e0u,
                  "BattleData hell-party total must begin at +0x7e0");
    static_assert(sizeof(BattleData) == 0x7f8u,
                  "BattleData IDA ABI size must stay 0x7f8");
    static_assert(sizeof(GameResultSet) == 0x50u,
                  "GameResultSet IDA ABI size must stay 0x50");
#endif
};

namespace WongWork {

// The BossTower route is not a byte-offset shortcut: it inherits the full
// CBossPlay -> CBossStage layout.  C2/reset at 0x81519EC/0x814BD0C prove the
// MapInfo subobject at CBossStage +0x9f4, whose items_ member is +0x24.
// Therefore processRouting's original BossTower +0xa18 erase has this typed
// expression: tower->map_info_.items_.erase(unique_id).
class CBossPlay {
public:
    struct stRewardItem_t {
        // IDA_EA: 0x81519B6
        // IDA_NAME: _ZN8WongWork9CBossPlay14stRewardItem_tC2Ev
        stRewardItem_t();

        uint32_t reward_value_;                 // +0x000
        Inven_Item items_[10];                  // +0x004
    };

    // IDA_EA: 0x81519EC / 0x8151A3A
    // IDA_NAME: _ZN8WongWork9CBossPlayC2Ev / _ZN8WongWork9CBossPlayD2Ev
    CBossPlay();
    ~CBossPlay();

    // The old vtable starts with these eleven gameplay slots.  Destructors
    // are deliberately non-virtual: IDA's CBossTower vtable has no deleting
    // destructor entries before generateRewardItem.
    virtual int generateRewardItem(int member_slot,
                                   int item_count,
                                   unsigned int character_level,
                                   ENUM_DUNGEON_DIFFICULTY difficulty,
                                   unsigned int context,
                                   const CDungeon* dungeon);
    virtual int generateRewardExp(int member_slot,
                                  unsigned int stage,
                                  unsigned int multiplier,
                                  const CDungeon* dungeon);
    virtual int _checkMemberDie();
    virtual int checkAllMemberReady(int member_count);
    virtual int handleDieUser(CUser* user);
    virtual int handleLeaveUser(CUser* user);
    // IDA's CBossPlay vtable leaves the map/item/monster dispatch slots
    // abstract.  CBossStage supplies the first four and CBossTower supplies
    // the value-rarity pickup slot; making the contract explicit preserves
    // the original compiler-generated pure-virtual entries.
    virtual int handleMoveMap(unsigned short* map_path) = 0;
    virtual int handlePickupItem(CUser* user,
                                 int unique_id,
                                 bool client_flag,
                                 char& item_rarity) = 0;
    virtual int handleDropItem(CUser* user,
                               int item_id,
                               int item_count,
                               char rarity,
                               int x,
                               int y) = 0;
    virtual int handleKillMonster(CUser* user,
                                  unsigned short monster_id,
                                  unsigned short monster_type,
                                  const MSG_MONSTER_DIE& message,
                                  int* result) = 0;
    virtual int handlePickupItem(CUser* user,
                                 int unique_id,
                                 bool client_flag,
                                 char item_rarity) = 0;

    // IDA_EA: 0x8149120 / 0x8145682 / 0x8145692 / 0x814A7E4
    CBossPlay* reset();
    int isMemberAlive(int member_slot);
    int setMemberReady(int member_slot, bool ready);
    int setMemberAlive(int member_slot, bool alive);
    // IDA_EA: 0x814926E
    // IDA_NAME: _ZN8WongWork9CBossPlay12InitPlayUserEii
    // DEMANGLED: WongWork::CBossPlay::InitPlayUser(int,int)
    void InitPlayUser(int dungeon_index, int tower_index);
    // IDA_EA: 0x8149466
    // IDA_NAME: _ZN8WongWork9CBossPlay18getLiveMemberCountEv
    // DEMANGLED: WongWork::CBossPlay::getLiveMemberCount(void)
    int getLiveMemberCount();

    CParty* party_;                             // +0x004
    uint32_t state_08_;                         // +0x008
    uint8_t all_died_;                          // +0x00c
    uint8_t member_alive_[4];                   // +0x00d
    uint8_t member_ready_[4];                   // +0x011
    uint8_t pad_015_017_[3];
    uint32_t values_[4];                        // +0x018
    stRewardItem_t rewards_[4];                 // +0x028
    uint32_t tail_9c8_[4];                      // +0x9c8
    uint32_t tail_9d8_[4];                      // +0x9d8
};

class CBossStage : public CBossPlay {
public:
    // IDA_EA: 0x814BD0C / 0x814BDD6
    // IDA_NAME: _ZN8WongWork10CBossStageC2Ev / _ZN8WongWork10CBossStageD2Ev
    CBossStage();
    ~CBossStage();

    // IDA_EA: 0x814BEA2
    // IDA_NAME: _ZN8WongWork10CBossStage5resetEv
    // DEMANGLED: WongWork::CBossStage::reset(void)
    CBossStage* reset();

    // Non-virtual stage leaves shared by the four original gameplay vtable
    // slots.  Their declarations live with the true owner, never in a
    // free-function or byte-offset facade.
    // IDA_EA: 0x814D1B6
    // IDA_NAME: _ZN8WongWork10CBossStage13moveNextStageEv
    // DEMANGLED: WongWork::CBossStage::moveNextStage(void)
    int moveNextStage();
    // IDA_EA: 0x814D214
    // IDA_NAME: _ZN8WongWork10CBossStage16killMonsterEraseEtR11map_monsterRi
    // DEMANGLED: WongWork::CBossStage::killMonsterErase(unsigned short,map_monster &,int &)
    int killMonsterErase(uint16_t monster_id,
                         map_monster& monster,
                         int& monster_kind);
    // IDA_EA: 0x814F4FC
    // IDA_NAME: _ZN8WongWork10CBossStage14randomMapIndexERi
    // DEMANGLED: WongWork::CBossStage::randomMapIndex(int &)
    int randomMapIndex(int& map_index);
    // IDA_EA: 0x814F0E6
    // IDA_NAME: _ZN8WongWork10CBossStage14GoldPickupItemEP5CUserjRK8map_item
    // DEMANGLED: WongWork::CBossStage::GoldPickupItem(CUser *,unsigned int,map_item const&)
    int GoldPickupItem(CUser* user,
                       unsigned int unique_id,
                       const map_item& item);

    // IDA_EA: 0x814E57A / 0x814E684 / 0x814ECEC
    // IDA_NAME: _ZN8WongWork10CBossStage17RoutingPickupItemEP5CUserjR8map_item /
    //           _ZN8WongWork10CBossStage16RandomPickupItemEP5CUserjRK8map_item /
    //           _ZN8WongWork10CBossStage16NormalPickupItemEP5CUserjRK8map_item
    int RoutingPickupItem(CUser* user, unsigned int unique_id, map_item& item);
    int RandomPickupItem(CUser* user,
                         unsigned int unique_id,
                         const map_item& item);
    int NormalPickupItem(CUser* user,
                         unsigned int unique_id,
                         const map_item& item);
    // IDA_EA: 0x814DE1A
    // IDA_NAME: _ZN8WongWork10CBossStage8dropItemEP5CUserciiR8map_item
    // DEMANGLED: WongWork::CBossStage::dropItem(CUser *,char,int,int,map_item &)
    int dropItem(CUser* user,
                 char rarity,
                 int item_id,
                 int count,
                 map_item& item);
    // IDA_EA: 0x814D300 / 0x814D628
    // IDA_NAME: _ZN8WongWork10CBossStage17KillMonsterGetExpERK11map_monsterRj /
    //           _ZN8WongWork10CBossStage19killMonsterDropItemERK11map_monsteritt
    int KillMonsterGetExp(const map_monster& monster, unsigned int& gained_exp);
    int killMonsterDropItem(const map_monster& monster,
                            int monster_kind,
                            unsigned short monster_id,
                            unsigned short monster_type);
    // IDA_EA: 0x814BEF4 / 0x814CE12
    // IDA_NAME: _ZN8WongWork10CBossStage10consistMapEPK4CMap /
    //           _ZN8WongWork10CBossStage17SendMapInfoPacketEv
    int consistMap(const CMap* map);
    int SendMapInfoPacket();

    // IDA_EA: 0x814F426
    // IDA_NAME: _ZN8WongWork10CBossStage16RandomConsistMapER5CUser
    // DEMANGLED: WongWork::CBossStage::RandomConsistMap(CUser &)
    int RandomConsistMap(CUser& manager);

    // IDA_EA: 0x814F66A
    // IDA_NAME: _ZN8WongWork10CBossStage13handleMoveMapEPt
    // DEMANGLED: WongWork::CBossStage::handleMoveMap(unsigned short *)
    int handleMoveMap(unsigned short* map_path) override;
    int handlePickupItem(CUser* user,
                         int unique_id,
                         bool client_flag,
                         char& item_rarity) override;
    int handleDropItem(CUser* user,
                       int item_id,
                       int item_count,
                       char rarity,
                       int x,
                       int y) override;
    int handleKillMonster(CUser* user,
                          unsigned short monster_id,
                          unsigned short monster_type,
                          const MSG_MONSTER_DIE& message,
                          int* result) override;

    uint32_t stage_start_tick_;                 // +0x9e8
    uint32_t stage_end_tick_;                   // +0x9ec
    CDungeon* dungeon_;                         // +0x9f0
    MapInfo map_info_;                          // +0x9f4
    uint32_t current_stage_;                    // +0xae0
    uint32_t maximum_stage_;                    // +0xae4
    // IDA callers pass this byte to CBossDungeonEntranceLog as the compact
    // member-count class (1/2/3/4 members map to 0/1/2/3), not a map seed.
    uint32_t member_count_class_;               // +0xae8
    uint32_t stage_value_aec_;                  // +0xaec
};

class CBossTower : public CBossStage {
public:
    // IDA_EA: 0x81429B4 / 0x8142A74
    // IDA_NAME: _ZN8WongWork10CBossTowerC2Ev /
    //           _ZN8WongWork10CBossTowerD2Ev
    CBossTower();
    ~CBossTower();

    int handleDieUser(CUser* user) override;
    int handleLeaveUser(CUser* user) override;
    int handleMoveMap(unsigned short* map_path) override;
    int handleDropItem(CUser* user,
                       int item_id,
                       int item_count,
                       char rarity,
                       int x,
                       int y) override;
    int handleKillMonster(CUser* user,
                          unsigned short monster_id,
                          unsigned short monster_type,
                          const MSG_MONSTER_DIE& message,
                          int* result) override;
    int handlePickupItem(CUser* user,
                         int unique_id,
                         bool client_flag,
                         char item_rarity) override;

    // IDA_EA: 0x8142F32 / 0x8142FA6
    // IDA_NAME: _ZN8WongWork10CBossTower13InitBossTowerEP6CPartyi /
    //           _ZN8WongWork10CBossTower23checkStartGameConditionERhb
    int InitBossTower(CParty* party, int dungeon_index);
    int checkStartGameCondition(unsigned char& failed_member,
                                bool consume_ancient_items);

    // IDA_EA: 0x8142D96
    // IDA_NAME: _ZN8WongWork10CBossTower15handleStartGameEP6CPartyic17ENUM_DUNGEON_TYPE
    // DEMANGLED: WongWork::CBossTower::handleStartGame(CParty *,int,char,ENUM_DUNGEON_TYPE)
    int handleStartGame(CParty* party,
                        int dungeon_index,
                        char member_count_class,
                        ENUM_DUNGEON_TYPE dungeon_mode);

    // IDA_EA: 0x8142B72
    // IDA_NAME: _ZN8WongWork10CBossTower17handleEPLPCommandEi
    // DEMANGLED: WongWork::CBossTower::handleEPLPCommand(int)
    int handleEPLPCommand(int command);

    // These are non-virtual tower progression members invoked by the virtual
    // handlers below.  Keeping their true member identities prevents the
    // C2/D2 pool path from substituting a free-function facade.
    // IDA_EA: 0x8142D24 / 0x81440B2
    int _onPrepareFinishTower(bool completed);
    int onFinishStage();

    // IDA_EA: 0x8142CDE
    // IDA_NAME: _ZN8WongWork10CBossTower15_onTimerProcessE13TIMER_MESSAGEj
    // DEMANGLED: WongWork::CBossTower::_onTimerProcess(TIMER_MESSAGE,unsigned int)
    int _onTimerProcess(TIMER_MESSAGE message, unsigned int timer_key);

    // IDA_EA: 0x8142B36
    // IDA_NAME: _ZN8WongWork10CBossTower8_destroyEv
    // DEMANGLED: WongWork::CBossTower::_destroy(void)
    int _destroy();

    // IDA_EA: 0x81454B8
    // IDA_NAME: _ZN8WongWork10CBossTower13handleUseItemEP5CUsermh
    // DEMANGLED: WongWork::CBossTower::handleUseItem(CUser *,unsigned long,unsigned char)
    int handleUseItem(CUser* user,
                      unsigned long item_id,
                      unsigned char item_category);

    // IDA_EA: 0x8143F6A
    // IDA_NAME: _ZN8WongWork10CBossTower19handleFinishLoadingEP5CUser
    // DEMANGLED: WongWork::CBossTower::handleFinishLoading(CUser *)
    int handleFinishLoading(CUser* user);

    // The clear/return leaves below all have fully named owners (CParty,
    // CUser, CDataManager and CBossPlay).  Keep their original member ABI so
    // the timer path can call them without a free-function facade.
    // IDA_EA: 0x81441D4 / 0x814432A / 0x81446A2
    // IDA_NAME: _ZN8WongWork10CBossTower14SendELPLPacketEv /
    //           _ZN8WongWork10CBossTower16SendRewardPacketEv /
    //           _ZN8WongWork10CBossTower15InsertRewardExpEv
    int SendELPLPacket();
    int SendRewardPacket();
    int InsertRewardExp();

    // IDA_EA: 0x8144508 / 0x81447A0 / 0x8144D54 / 0x8144E36 / 0x8144FCA
    // IDA_NAME: _ZN8WongWork10CBossTower17generateRewardExpEv /
    //           _ZN8WongWork10CBossTower18generateRewardItemEv /
    //           _ZN8WongWork10CBossTower15onReturnVillageEv
    //           _ZN8WongWork10CBossTower16onClearBossTowerEv /
    //           _ZN8WongWork10CBossTower17InsertRewardItemsEv
    int onClearBossTower();
    int InsertRewardItems();
    int generateRewardExp();
    int generateRewardItem();
    int onReturnVillage();

    // IDA_EA: 0x814512A / 0x814530C
    // IDA_NAME: _ZN8WongWork10CBossTower17onReadyTimerCheckEv /
    //           _ZN8WongWork10CBossTower19onChargeRequireItemEP5CUser
    int onReadyTimerCheck();
    int onChargeRequireItem(CUser* user);

    // IDA_EA: 0x82AA716 / 0x82AA728.  StaticPool stores its object index
    // in this physical dword; the former timer-key label had no source use.
    uint32_t pool_index_;                       // +0xaf0
    uint32_t stage_prepare_tick_;               // +0xaf4
    uint32_t stage_finish_tick_;                // +0xaf8
    uint8_t prepare_finish_;                    // +0xafc
    uint8_t pad_afd_aff_[3];
    CBossDungeonEntranceLog entrance_log_;      // +0xb00

    // IDA_EA: 0x82AA716 / 0x82AA728
    // IDA_NAME: _ZN8WongWork10CBossTower6setIdxEi / _ZN8WongWork10CBossTower6getIdxEv
    CBossTower* setIdx(int index);
    int getIdx();
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CBossPlay::stRewardItem_t) == 0x268u,
              "CBossPlay reward item must retain the IDA 0x268-byte ABI");
static_assert(sizeof(CBossPlay) == 0x9e8u,
              "CBossPlay IDA ABI size must stay 0x9e8");
static_assert(offsetof(CBossPlay, party_) == 0x004u,
              "CBossPlay party must stay at IDA +0x04");
static_assert(offsetof(CBossPlay, tail_9c8_) == 0x9c8u,
              "CBossPlay first item-use counters must stay at IDA +0x9c8");
static_assert(offsetof(CBossPlay, tail_9d8_) == 0x9d8u,
              "CBossPlay second item-use counters must stay at IDA +0x9d8");
static_assert(offsetof(CBossStage, map_info_) == 0x9f4u,
              "CBossStage::map_info_ must stay at IDA +0x9f4");
static_assert(sizeof(CBossStage) == 0xaf0u,
              "CBossStage IDA ABI size must stay 0xaf0");
static_assert(offsetof(CBossTower, entrance_log_) == 0xb00u,
              "CBossTower entrance log must stay at IDA +0xb00");
static_assert(sizeof(CBossTower) == 0xb18u,
              "CBossTower IDA ABI size must stay 0xb18");
#endif

class CDungeonClear : public std::map<short, char> {
public:
    typedef std::map<short, char> Base;

    // IDA_EA: 0x8374F24
    // IDA_NAME: _ZN8WongWork13CDungeonClear17addClearedDungeonEsc
    // DEMANGLED: WongWork::CDungeonClear::addClearedDungeon(short,char)
    void addClearedDungeon(short dungeon_index, char difficulty);

    // IDA_EA: 0x85BF96C
    // IDA_NAME: _ZNK8WongWork13CDungeonClear21getClearedDungeonDiffEs
    // DEMANGLED: WongWork::CDungeonClear::getClearedDungeonDiff(short)const
    int getClearedDungeonDiff(short dungeon_index) const;

    // IDA_EA: 0x8483FAE
    // IDA_NAME: _ZN8WongWork13CDungeonClear14AradfromStringEP5CUserPKc
    // DEMANGLED: WongWork::CDungeonClear::AradfromString(CUser *,char const*)
    int AradfromString(CUser* user, const char* text);

    // IDA_EA: 0x84843FA
    // IDA_NAME: _ZN8WongWork13CDungeonClear10fromStringEP5CUserPc
    // DEMANGLED: WongWork::CDungeonClear::fromString(CUser *,char *)
    int fromString(CUser* user, char* text);

    // IDA_EA: 0x8695972
    // IDA_NAME: _ZN8WongWork13CDungeonClear5clearEv
    // DEMANGLED: WongWork::CDungeonClear::clear(void)
    void clear();

    // IDA_EA: 0x848441C
    // IDA_NAME: _ZNK8WongWork13CDungeonClear8toStringEPcj
    // DEMANGLED: WongWork::CDungeonClear::toString(char *,unsigned int)const
    int toString(char* out, unsigned int maxlen) const;

    // IDA_EA: 0x8484558
    // IDA_NAME: _ZNK8WongWork13CDungeonClear8toStringERSsi
    // DEMANGLED: WongWork::CDungeonClear::toString(std::string &,int)const
    int toString(std::string& out, int maxlen) const;

    // IDA_EA: 0x84846E2
    // IDA_NAME: _ZNK8WongWork13CDungeonClear16enumDungeonClearEP5CUserPFbscPcEz
    // DEMANGLED: WongWork::CDungeonClear::enumDungeonClear(CUser *,bool (*)(short,char,char *),...)const
    char enumDungeonClear(CUser* user,
                          bool (*callback)(short, char, char*),
                          ...) const;

    // IDA_EA: 0x869595E
    // IDA_NAME: _ZNK8WongWork13CDungeonClear19getDungeonClearSizeEv
    // DEMANGLED: WongWork::CDungeonClear::getDungeonClearSize(void)const
    int getDungeonClearSize() const;
};

class CDeathTower {
public:
    // IDA names these two nested enums in the stage/tower state accessors.
    // Their enumerator tables have not yet been recovered, so retain the
    // original identities and fixed i386 representation without inventing
    // values.
    enum ENUM_STAGE_STATE : int {};
    enum ENUM_TOWER_STATE : int {};

    class CDungeonMgr {
    public:
        // IDA_EA: 0x82A74B0 / 0x82A74B6
        CDungeonMgr();
        ~CDungeonMgr();
        CDungeonMgr* reset();

        // IDA_EA: 0x84605A2
        // IDA_NAME: _ZN8WongWork11CDeathTower11CDungeonMgr14initDungeonMgrEi
        // DEMANGLED: WongWork::CDeathTower::CDungeonMgr::initDungeonMgr(int)
        int initDungeonMgr(int dungeon_index);

        // IDA_EA: 0x82345F0
        // IDA_NAME: _ZNK8WongWork11CDeathTower11CDungeonMgr10getDungeonEv
        // DEMANGLED: WongWork::CDeathTower::CDungeonMgr::getDungeon(void)const
        CDungeon* getDungeon() const;

        // IDA_EA: 0x8469AB4
        // IDA_NAME: _ZN8WongWork11CDeathTower11CDungeonMgr11getEndStageEv
        // DEMANGLED: WongWork::CDeathTower::CDungeonMgr::getEndStage(void)
        int getEndStage();

        // IDA_EA: 0x84605EE
        // IDA_NAME: _ZN8WongWork11CDeathTower11CDungeonMgr11getStageMapEj
        // DEMANGLED: WongWork::CDeathTower::CDungeonMgr::getStageMap(unsigned int)
        // The B-owned CDungeon::getDeathTowerMapIndex provider is restored in
        // legacy_datamanager/map_runtime.cpp; this C leaf only resolves the
        // resulting map through CDataManager.
        CMap* getStageMap(unsigned int current_stage);

        // IDA_EA: 0x8469AC0 / 0x8469AD6 / 0x8469AEC
        int getStandardLevel() const;
        const std::vector<std::pair<int, int> >& getObjectDropItems() const;
        const std::vector<stDungeonAssignItem_t>&
        getSpecialPassiveObjectItems() const;

    private:
        CDungeon* dungeon_;             // +0x00
        int end_stage_;                  // +0x04
    };

    class CStage {
    public:
        // IDA_EA: 0x82A74CA / 0x82A74EC
        CStage();
        ~CStage();
        CStage* resetCurrentStage();
        CStage* reset();

        // IDA_EA: 0x82345FA
        // IDA_NAME: _ZN8WongWork11CDeathTower6CStage17getStageClearTimeEv
        // DEMANGLED: WongWork::CDeathTower::CStage::getStageClearTime(void)
        int getStageClearTime();

        // IDA_EA: 0x8469B02 / 0x8469B0C / 0x8469B18
        // IDA_NAME: _ZN8WongWork11CDeathTower6CStage15getCurrentStageEv /
        //           _ZN8WongWork11CDeathTower6CStage16getStageMapIndexEv /
        //           _ZN8WongWork11CDeathTower6CStage8getStateEv
        int getCurrentStage();
        int getStageMapIndex();
        int getState();

        // IDA_EA: 0x8461C26 / 0x8461C40
        // IDA_NAME: _ZN8WongWork11CDeathTower6CStage12onStartStageEv /
        //           _ZN8WongWork11CDeathTower6CStage13onFinishStageEv
        // DEMANGLED: WongWork::CDeathTower::CStage::onStartStage(void) /
        //            WongWork::CDeathTower::CStage::onFinishStage(void)
        int onStartStage();
        int onFinishStage();

        // IDA_EA: 0x84617DA / 0x8461B3E / 0x8461C5A
        // IDA_NAME: _ZN8WongWork11CDeathTower6CStage13moveNextStageEj /
        //           _ZN8WongWork11CDeathTower6CStage15checkClearStageEv /
        //           _ZN8WongWork11CDeathTower6CStage8setStateENS0_16ENUM_STAGE_STATEE
        int moveNextStage(unsigned int end_stage);
        int checkClearStage();
        int setState(ENUM_STAGE_STATE state);

        // IDA_EA: 0x8461828
        // IDA_NAME: _ZN8WongWork11CDeathTower6CStage15makeStagePacketER11PacketGuard
        // DEMANGLED: WongWork::CDeathTower::CStage::makeStagePacket(PacketGuard &)
        bool makeStagePacket(PacketGuard& packet);

        // IDA_EA: 0x8460634
        // IDA_NAME: _ZN8WongWork11CDeathTower6CStage10consistMapERKNS0_11CDungeonMgrEPK4CMapPS0_
        // DEMANGLED: WongWork::CDeathTower::CStage::consistMap(
        //     WongWork::CDeathTower::CDungeonMgr const&,CMap const*,
        //     WongWork::CDeathTower*)
        // This full map/AI/item-generator transaction is provided by the
        // C-owned death_tower_consist_map_runtime.cpp implementation.  It is
        // a real dependency graph, not a byte-layout facade.
        int consistMap(const CDungeonMgr& dungeon_manager,
                       const CMap* map,
                       CDeathTower* tower);

        // IDA_EA: 0x8461534 / 0x84615E4
        // IDA_NAME: _ZN8WongWork11CDeathTower6CStage8peekItemEiR8map_item /
        //           _ZN8WongWork11CDeathTower6CStage10pickupItemEiR8map_item
        int peekItem(int unique_id, map_item& item);
        int pickupItem(int unique_id, map_item& item);
        // IDA_EA: 0x84616A8
        // IDA_NAME: _ZN8WongWork11CDeathTower6CStage8dropItemER8map_item
        // DEMANGLED: WongWork::CDeathTower::CStage::dropItem(map_item &)
        int dropItem(map_item& item);
        // IDA_EA: 0x8461708
        // IDA_NAME: _ZN8WongWork11CDeathTower6CStage11killMonsterEiR11map_monster
        int killMonster(int monster_unique_id, map_monster& monster);

    private:
        int current_stage_;              // +0x00
        int stage_start_tick_;           // +0x04
        int stage_end_tick_;             // +0x08
        MapInfo map_info_;               // +0x0c
        ENUM_STAGE_STATE state_;         // +0xf8
    };

    // IDA_EA: 0x846483E
    // IDA_NAME: _ZN8WongWork11CDeathTower17handleKillMonsterEP5CUserttRK15MSG_MONSTER_DIEPi
    int handleKillMonster(CUser* user,
                          unsigned short monster_id,
                          unsigned short monster_type,
                          const MSG_MONSTER_DIE& message,
                          int* result);

    class CPlayData {
    public:
        class stRewardItem_t {
        public:
            stRewardItem_t();
            bool reset();

        public:
            // CPlayData's original accessors return these physical members
            // directly; keep them visible to that enclosing object rather
            // than adding non-original forwarding methods.
            uint32_t item_count_;        // +0x00
            Inven_Item items_[10];       // +0x04
            uint8_t pad_266_267_[2];
        };

        // IDA_EA: 0x8461D02.  Four adjacent entries begin at CPlayData+0x9cc;
        // the original writes the character name, level, job and grow type
        // at +0x00, +0x14, +0x15 and +0x16 respectively.
        struct stStartMemberInfo_t {
            char character_name_[20];    // +0x00
            uint8_t character_level_;    // +0x14
            uint8_t character_job_;      // +0x15
            uint8_t grow_type_;          // +0x16
        };

        CPlayData();
        ~CPlayData();
        bool reset();

        // IDA_EA: 0x8461FC0 / 0x8462538
        // IDA_NAME:
        // _ZN8WongWork11CDeathTower9CPlayData18generateRewardItemEiiRKNS0_11CDungeonMgrEjjP6CParty /
        // _ZN8WongWork11CDeathTower9CPlayData17generateRewardExpEiijP6CPartyRNS0_11CDungeonMgrE
        // DEMANGLED: WongWork::CDeathTower::CPlayData::generateRewardItem(
        //     int,int,WongWork::CDeathTower::CDungeonMgr const&,unsigned int,
        //     unsigned int,CParty *) /
        // WongWork::CDeathTower::CPlayData::generateRewardExp(
        //     int,int,unsigned int,CParty *,WongWork::CDeathTower::CDungeonMgr &)
        bool generateRewardItem(int member_slot,
                                int item_count,
                                const CDungeonMgr& dungeon_manager,
                                unsigned int character_level,
                                unsigned int reward_probability_scale,
                                CParty* party);
        CPlayData* generateRewardExp(int member_slot,
                                     int current_stage,
                                     unsigned int stage_base_exp,
                                     CParty* party,
                                     CDungeonMgr& dungeon_manager);

        // IDA_EA: 0x8461EE6 / 0x8469B76
        // IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData16resetMemberReadyEv /
        //           _ZN8WongWork11CDeathTower9CPlayData13isMemberAliveEi
        bool resetMemberReady();
        int isMemberAlive(int member_slot);

        // IDA_EA: 0x8469B5A / 0x8469B86 / 0x8469BA2
        // IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData14setMemberAliveEib /
        //           _ZN8WongWork11CDeathTower9CPlayData14setMemberReadyEib /
        //           _ZN8WongWork11CDeathTower9CPlayData13isMemberReadyEi
        int setMemberAlive(int member_slot, bool alive);
        int setMemberReady(int member_slot, bool ready);
        int isMemberReady(int member_slot);

        // IDA_EA: 0x8469BB2 / 0x8469BD4 / 0x8469BE0
        // IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData11addPlayTimeEj /
        //           _ZN8WongWork11CDeathTower9CPlayData11getPlayTimeEv /
        //           _ZN8WongWork11CDeathTower9CPlayData15getLastPlayTimeEv
        CPlayData* addPlayTime(uint32_t stage_clear_time);
        int getPlayTime();
        int getLastPlayTime();

        // IDA_EA: 0x8461E4A / 0x8461E7E
        // IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData18checkEnterStartMapEi /
        //           _ZN8WongWork11CDeathTower9CPlayData19checkAllMemberReadyEi
        int checkEnterStartMap(int member_slot);
        int checkAllMemberReady(int member_count);

        // IDA_EA: 0x8461D02 / 0x8469B34
        // IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData19makeStartMemberInfoEP6CParty /
        //           _ZN8WongWork11CDeathTower9CPlayData18getStartMemberInfoEv
        CPlayData* makeStartMemberInfo(CParty* party);
        stStartMemberInfo_t* getStartMemberInfo();

        // IDA_EA: 0x8469B26 / 0x8469B42 / 0x8469B4C
        // IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData17getStartMemberCntEv /
        //           _ZN8WongWork11CDeathTower9CPlayData17getLastRoutedTurnEv /
        //           _ZN8WongWork11CDeathTower9CPlayData17setLastRoutedTurnEi
        int getStartMemberCnt();
        int getLastRoutedTurn();
        CPlayData* setLastRoutedTurn(int member_slot);

        // IDA_EA: 0x8469BEC / 0x8469C06 / 0x8469C1E
        // IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData18getRewardItemCountEi /
        //           _ZN8WongWork11CDeathTower9CPlayData13getRewardItemEi /
        //           _ZN8WongWork11CDeathTower9CPlayData12getRewardExpEi
        int getRewardItemCount(int member_slot);
        Inven_Item* getRewardItem(int member_slot);
        int getRewardExp(int member_slot);

    private:
        int last_routed_turn_;           // +0x00
        uint8_t member_alive_[4];        // +0x04
        uint8_t member_ready_[4];        // +0x08
        uint8_t all_members_ready_;      // +0x0c
        uint8_t pad_0d_0f_[3];
        uint32_t play_time_;              // +0x10
        uint32_t last_play_time_;         // +0x14
        stRewardItem_t reward_items_[4]; // +0x18
        uint32_t reward_exp_[4];          // +0x9b8
        int start_member_count_;          // +0x9c8
        stStartMemberInfo_t start_member_info_[4];  // +0x9cc
        uint8_t enter_start_map_[4];     // +0xa28
    };

    // IDA_EA: 0x823462A / 0x823464C / 0x8469C86 / 0x8469C96 / 0x8469CDA
    // IDA_NAME: _ZN8WongWork11CDeathTower25stMapMonsterKillChecker_t4initEv /
    //           _ZN8WongWork11CDeathTower25stMapMonsterKillChecker_t19setUseSkillMaterialEj /
    //           _ZN8WongWork11CDeathTower25stMapMonsterKillChecker_t15initMonsterInfoEv /
    //           _ZN8WongWork11CDeathTower25stMapMonsterKillChecker_t10dieMonsterEjj /
    //           _ZN8WongWork11CDeathTower25stMapMonsterKillChecker_t23getUseSkillMaterialTimeEv
    // This is the distinct 0x0c Death-Tower checker at CDeathTower+0xb58.
    // It is not the 0x18 world-area stMapMonsterKillChecker_t.
    struct stMapMonsterKillChecker_t {
        stMapMonsterKillChecker_t* init();
        stMapMonsterKillChecker_t* setUseSkillMaterial(uint32_t tick);
        stMapMonsterKillChecker_t* initMonsterInfo();
        int dieMonster(uint32_t tick, uint32_t interval);
        int getUseSkillMaterialTime();

        uint32_t use_skill_material_tick_;      // +0x00
        uint32_t last_mob_die_tick_;            // +0x04
        uint32_t uncontinually_mob_die_count_;  // +0x08
    };

    class CPacketHandler;

    // IDA_EA: 0x82A75DA / 0x82A7664
    CDeathTower();
    ~CDeathTower();

    // IDA_EA: 0x82A7578
    CDeathTower* setIdx(int index);
    // IDA_EA: 0x82A7586.  The original symbol is non-const.
    int getIdx();
    // IDA_EA: 0x82A7592
    CDeathTower* reset();

    // IDA_EA: 0x830EDD2 / 0x830EDEC
    // IDA_NAME: _ZN8WongWork11CDeathTower11genTimerKeyE13TIMER_MESSAGE
    //           _ZN8WongWork11CDeathTower12_genTimerKeyE13TIMER_MESSAGE
    int genTimerKey(TIMER_MESSAGE message);
    int _genTimerKey(TIMER_MESSAGE message);

    // IDA_EA: 0x823461E
    // IDA_NAME: _ZN8WongWork11CDeathTower9getCStageEv
    // DEMANGLED: WongWork::CDeathTower::getCStage(void)
    CStage* getCStage();

    // IDA_EA: 0x8234612
    // IDA_NAME: _ZNK8WongWork11CDeathTower14getCDungeonMGrEv
    // DEMANGLED: WongWork::CDeathTower::getCDungeonMGr(void)const
    CDungeonMgr* getCDungeonMGr() const;

    // IDA_EA: 0x8465C94
    // IDA_NAME: _ZN8WongWork11CDeathTower13handleMoveMapEv
    // DEMANGLED: WongWork::CDeathTower::handleMoveMap(void)
    int handleMoveMap();

    // IDA_EA: 0x846595A
    // IDA_NAME: _ZN8WongWork11CDeathTower13handleDieUserEP5CUser
    // DEMANGLED: WongWork::CDeathTower::handleDieUser(CUser *)
    int handleDieUser(CUser* user);

    // IDA_EA: 0x8466024
    // IDA_NAME: _ZN8WongWork11CDeathTower19handleFinishLoadingEP5CUser
    // DEMANGLED: WongWork::CDeathTower::handleFinishLoading(CUser *)
    int handleFinishLoading(CUser* user);

    // IDA_EA: 0x8466150 / 0x8467CAC
    // IDA_NAME: _ZN8WongWork11CDeathTower18handleStageCommandEP5CUserh /
    //           _ZN8WongWork11CDeathTower13_onStartStageEv
    // DEMANGLED: WongWork::CDeathTower::handleStageCommand(CUser *,unsigned char) /
    //            WongWork::CDeathTower::_onStartStage(void)
    int handleStageCommand(CUser* user, uint8_t command);
    int _onStartStage();

    // IDA_EA: 0x8467CC2
    // IDA_NAME: _ZN8WongWork11CDeathTower14_onFinishStageEv
    // DEMANGLED: WongWork::CDeathTower::_onFinishStage(void)
    bool _onFinishStage();

    // IDA_EA: 0x8467E00 / 0x8467E60
    // IDA_NAME: _ZN8WongWork11CDeathTower21_onPrepareFinishTowerEv /
    //           _ZN8WongWork11CDeathTower8_onClearEb
    // DEMANGLED: WongWork::CDeathTower::_onPrepareFinishTower(void) /
    //            WongWork::CDeathTower::_onClear(bool)
    int _onPrepareFinishTower();
    bool _onClear(bool cleared);

    // IDA_EA: 0x84662CC
    // IDA_NAME: _ZN8WongWork11CDeathTower18handleUseStackableEP5CUser14ENUM_ITEMSPACEt
    // DEMANGLED: WongWork::CDeathTower::handleUseStackable(CUser *,
    //             ENUM_ITEMSPACE,unsigned short)
    int handleUseStackable(CUser* user,
                           ENUM_ITEMSPACE item_space,
                           unsigned short slot);

    // IDA_EA: 0x8466208
    // IDA_NAME: _ZN8WongWork11CDeathTower23handleSelectEPLPCommandEP5CUseri
    // DEMANGLED: WongWork::CDeathTower::handleSelectEPLPCommand(CUser *,int)
    int handleSelectEPLPCommand(CUser* user, int delay_sec);

    // IDA_EA: 0x84670F8 / 0x8467188
    // IDA_NAME: _ZN8WongWork11CDeathTower17_checkMemberReadyEv /
    //           _ZN8WongWork11CDeathTower15_checkMemberDieEv
    int _checkMemberReady();
    int _checkMemberDie();

    // IDA_EA: 0x8462A26 / 0x846398E
    // IDA_NAME: _ZN8WongWork11CDeathTower11attachPartyEP6CParty /
    //           _ZN8WongWork11CDeathTower15handleStartGameEic17ENUM_DUNGEON_TYPE
    // DEMANGLED: WongWork::CDeathTower::attachParty(CParty *) /
    //            WongWork::CDeathTower::handleStartGame(int,char,ENUM_DUNGEON_TYPE)
    int attachParty(CParty* party);
    int handleStartGame(int dungeon_index,
                        char dungeon_difficulty,
                        ENUM_DUNGEON_TYPE dungeon_mode);

    // IDA_EA: 0x84636F2
    // IDA_NAME: _ZN8WongWork11CDeathTower11onLeaveUserEP5CUser
    // DEMANGLED: WongWork::CDeathTower::onLeaveUser(CUser *)
    int onLeaveUser(CUser* user);

    // IDA_EA: 0x8465AC2
    // IDA_NAME: _ZN8WongWork11CDeathTower15handleLeaveUserEP5CUser
    // DEMANGLED: WongWork::CDeathTower::handleLeaveUser(CUser *)
    int handleLeaveUser(CUser* user);

    // IDA_EA: 0x8464036
    // IDA_NAME: _ZN8WongWork11CDeathTower16handlePickupItemEP5CUseribc
    // DEMANGLED: WongWork::CDeathTower::handlePickupItem(CUser *,int,bool,char)
    int handlePickupItem(CUser* user,
                         int unique_id,
                         bool client_flag,
                         char client_rarity);

    // IDA_EA: 0x846466E
    // IDA_NAME: _ZN8WongWork11CDeathTower14handleDropItemEP5CUseriicii
    // DEMANGLED: WongWork::CDeathTower::handleDropItem(
    //     CUser *,int,int,char,int,int)
    int handleDropItem(CUser* user,
                       int x,
                       int y,
                       char item_space,
                       int slot,
                       int count);

    // IDA_EA: 0x8469C5C
    // IDA_NAME: _ZN8WongWork11CDeathTower13getTowerStateEv
    // DEMANGLED: WongWork::CDeathTower::getTowerState(void)
    int getTowerState();

    // IDA_EA: 0x8462AAE
    // IDA_NAME: _ZN8WongWork11CDeathTower7onTimerE13TIMER_MESSAGEj
    // DEMANGLED: WongWork::CDeathTower::onTimer(TIMER_MESSAGE,unsigned int)
    int onTimer(TIMER_MESSAGE message, unsigned int timer_key);

private:
    // IDA_EA: 0x8463890 / 0x8467B50 / 0x8469C34
    // IDA_NAME: _ZN8WongWork11CDeathTower24_checkStartGameConditionERh /
    //           _ZN8WongWork11CDeathTower18_onStartDeathTowerEv /
    //           _ZN8WongWork11CDeathTower13setTowerStateENS0_16ENUM_TOWER_STATEE
    int _checkStartGameCondition(unsigned char& failed_member);
    // IDA_EA: 0x8467B20
    // IDA_NAME: _ZN8WongWork11CDeathTower8_destroyEv
    // DEMANGLED: WongWork::CDeathTower::_destroy(void)
    void _destroy();
    bool _onStartDeathTower();

    // IDA_EA: 0x8467208
    // IDA_NAME: _ZN8WongWork11CDeathTower23_beginTowerClearProcessEb
    // DEMANGLED: WongWork::CDeathTower::_beginTowerClearProcess(bool)
    void _beginTowerClearProcess(bool cleared);

    // IDA_EA: 0x8467F6E
    // IDA_NAME: _ZN8WongWork11CDeathTower19_onFinishDeathTowerEv
    void _onFinishDeathTower();
    int setTowerState(ENUM_TOWER_STATE state);

    // IDA_EA: 0x8469C68
    // IDA_NAME: _ZN8WongWork11CDeathTower14_checkTimerKeyE13TIMER_MESSAGEj
    bool _checkTimerKey(TIMER_MESSAGE message, unsigned int timer_key);

    // IDA_EA: 0x846638C / 0x8466B02 / 0x8466CFE
    // IDA_NAME: _ZN8WongWork11CDeathTower13_makeDropItemEP5CUserciiR8map_item /
    //           _ZN8WongWork11CDeathTower16_pickupItemMoneyEP5CUseriRK8map_itemPiS6_ /
    //           _ZN8WongWork11CDeathTower11_pickupItemEP5CUserPciRK8map_itembcPK5CItem
    bool _makeDropItem(CUser* user,
                       char item_space,
                       int slot,
                       int count,
                       map_item& item);
    bool _pickupItemMoney(CUser* user,
                          int member_count,
                          const map_item& item,
                          int* earned_money,
                          int* routed_money);
    CUser* _pickupItem(CUser* user,
                       char* picked_by,
                       int member_count,
                       const map_item& item,
                       bool client_flag,
                       char client_rarity,
                       const CItem* item_data);

    // IDA_EA: 0x846801E / 0x846805E
    // IDA_NAME: _ZN8WongWork11CDeathTower20_checkRenewTopRecordEjjjj /
    //           _ZN8WongWork11CDeathTower19_checkRenewMyRecordEjjjjj
    int _checkRenewTopRecord(unsigned int dungeon_index,
                             unsigned int member_count,
                             unsigned int stage,
                             unsigned int play_time);
    int _checkRenewMyRecord(unsigned int dungeon_index,
                            unsigned int member_count,
                            unsigned int charac_no,
                            unsigned int stage,
                            unsigned int play_time);

    // IDA_EA: 0x846966C
    // IDA_NAME: _ZN8WongWork11CDeathTower23deathTowerCardStatisticEiP5CUserPK10Inven_Item
    // DEMANGLED: WongWork::CDeathTower::deathTowerCardStatistic(
    //     int,CUser *,Inven_Item const*)
    // The original end-of-stage reward path records currency/item value and
    // Death-Tower distribution statistics before it sends the reward packet.
    bool deathTowerCardStatistic(int reward_item_count,
                                 CUser* user,
                                 const Inven_Item* reward_items);

    // The original CMD162 body directly invokes the embedded checker at
    // +0xb58.  Friendship preserves that physical-object operation without
    // introducing a synthetic public accessor.
    friend class ::Dispatcher_DeathTowerStageCommand;
    // CMD18 addresses this exact embedded Death-Tower checker when a skill
    // material is deleted in tower exercise mode.
    friend class ::DisPatcher_DeleteItem;

    CParty* party_;                       // +0x00
    uint32_t index_;                      // +0x04
    ENUM_TOWER_STATE tower_state_;        // +0x08
    CDungeonMgr dungeon_manager_;        // +0x0c
    CStage stage_;                       // +0x14
    CPlayData play_data_;                // +0x110
    uint32_t timer_keys_[7];             // +0xb3c
    stMapMonsterKillChecker_t map_monster_kill_checker_;  // +0xb58
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CDeathTower::CDungeonMgr) == 0x08u,
              "CDeathTower::CDungeonMgr must stay at IDA extent 0x08");
static_assert(sizeof(CDeathTower::CStage) == 0xfcu,
              "CDeathTower::CStage must stay at IDA extent 0xfc");
static_assert(sizeof(CDeathTower::CPlayData::stRewardItem_t) == 0x268u,
              "CDeathTower reward item must stay at IDA extent 0x268");
static_assert(sizeof(CDeathTower::CPlayData::stStartMemberInfo_t) == 0x17u,
              "CDeathTower start-member record must stay at IDA extent 0x17");
static_assert(sizeof(CDeathTower::CPlayData) == 0xa2cu,
              "CDeathTower::CPlayData must stay at IDA extent 0xa2c");
static_assert(sizeof(CDeathTower::stMapMonsterKillChecker_t) == 0x0cu,
              "CDeathTower kill checker must stay at IDA extent 0x0c");
static_assert(sizeof(CDeathTower) == 0xb64u,
              "CDeathTower must stay at the 0xb64-byte pool-object extent");
#endif

class CDeathTowerRanking {
public:
    // IDA_EA: 0x82A76C0 / 0x8469CE4 / 0x8469D30
    // IDA_NAME: _ZN8WongWork18CDeathTowerRanking14stBestRecord_tC2Ev /
    //           _ZN8WongWork18CDeathTowerRanking14stBestRecord_tgtERKS1_ /
    //           _ZN8WongWork18CDeathTowerRanking14stBestRecord_teqERKS1_
    // The record is a real 0x98-byte value held both in the five top-six
    // tables and in the per-character hash tables.  In particular, the four
    // 30-byte names are *not* a packed DB member array: DB input names are
    // 20 bytes and are copied into this client-packet representation.
    struct stBestRecord_t {
        stBestRecord_t();
        bool operator>(const stBestRecord_t& right);
        bool operator==(const stBestRecord_t& right);

        uint32_t charac_no_;                  // +0x00
        char charac_name_[4][30];             // +0x04
        uint8_t character_level_[4];          // +0x7c
        uint8_t character_job_[4];            // +0x80
        uint8_t character_grow_type_[4];      // +0x84
        uint32_t current_stage_;              // +0x88
        uint32_t play_time_;                  // +0x8c
        uint8_t valid_;                       // +0x90
        uint8_t reserved_91_93_[3];
        int32_t virtual_rank_;                // +0x94
    };

    typedef __gnu_cxx::hash_map<unsigned int, unsigned int> RankingTable;
    typedef __gnu_cxx::hash_map<unsigned int, stBestRecord_t> BestRecordTable;
    typedef std::map<unsigned int, unsigned int> StageRankingTable;

    // IDA_EA: 0x82A787C / 0x82A7B06
    // IDA_NAME: _ZN8WongWork18CDeathTowerRankingC2Ev /
    //           _ZN8WongWork18CDeathTowerRankingD2Ev
    CDeathTowerRanking();
    ~CDeathTowerRanking();

    // IDA_EA: 0x82A774C
    // IDA_NAME: _ZN8WongWork18CDeathTowerRanking12getRankTableEj
    // DEMANGLED: WongWork::CDeathTowerRanking::getRankTable(unsigned int)
    static CDeathTowerRanking* getRankTable(unsigned int member_count);

    // IDA_EA: 0x84680A4
    // IDA_NAME: _ZN8WongWork18CDeathTowerRanking4initEj
    CDeathTowerRanking* init(unsigned int member_count);

    // IDA_EA: 0x8468290 / 0x84682FE
    // IDA_NAME: _ZN8WongWork18CDeathTowerRanking19checkRenewTopRecordEjjj /
    //           _ZN8WongWork18CDeathTowerRanking18checkRenewMyRecordEjjjj
    int checkRenewTopRecord(unsigned int rank_index,
                            unsigned int stage,
                            unsigned int play_time);
    int checkRenewMyRecord(unsigned int rank_index,
                           unsigned int charac_no,
                           unsigned int stage,
                           unsigned int play_time);

    // IDA_EA: 0x84683DA
    // IDA_NAME: _ZN8WongWork18CDeathTowerRanking13registRankingEjjj
    char registRanking(unsigned int rank_index,
                       unsigned int charac_no,
                       unsigned int virtual_rank);

    // IDA_EA: 0x84684CE
    // IDA_NAME: _ZN8WongWork18CDeathTowerRanking13updateRankingEjjj
    int updateRanking(unsigned int rank_index,
                      unsigned int charac_no,
                      unsigned int virtual_rank);

    // IDA_EA: 0x8468594
    // IDA_NAME: _ZN8WongWork18CDeathTowerRanking12resetRankingEv
    bool resetRanking();

    // IDA_EA: 0x8468614
    // IDA_NAME: _ZN8WongWork18CDeathTowerRanking12registRankerEjjPK30stDeathTowerRecordMemberInfo_tjj
    char registRanker(unsigned int rank_index,
                      unsigned int member_count,
                      const stDeathTowerRecordMemberInfo_t* members,
                      unsigned int stage,
                      unsigned int play_time);

    // IDA_EA: 0x8468914
    // IDA_NAME: _ZN8WongWork18CDeathTowerRanking16registBestRecordEjjPK30stDeathTowerRecordMemberInfo_tjj
    int registBestRecord(unsigned int member_count,
                         unsigned int charac_no,
                         const stDeathTowerRecordMemberInfo_t* members,
                         unsigned int stage,
                         unsigned int play_time);

    // IDA_EA: 0x8468552
    // IDA_NAME: _ZN8WongWork18CDeathTowerRanking15unregistRankingEj
    // DEMANGLED: WongWork::CDeathTowerRanking::unregistRanking(unsigned int)
    bool unregistRanking(unsigned int charac_no);

    // IDA_EA: 0x8468B4C
    // IDA_NAME: _ZN8WongWork18CDeathTowerRanking18unregistBestRecordEj
    // DEMANGLED: WongWork::CDeathTowerRanking::unregistBestRecord(unsigned int)
    bool unregistBestRecord(unsigned int charac_no);

    // IDA_EA: 0x8468B8E / 0x8468C68
    // IDA_NAME: _ZN8WongWork18CDeathTowerRanking22makeMyBestRecordPacketEjjR11PacketGuard /
    //           _ZN8WongWork18CDeathTowerRanking25makeTopRankerRecordPacketEjR11PacketGuard
    int makeMyBestRecordPacket(unsigned int rank_index,
                               unsigned int charac_no,
                               PacketGuard& packet);
    bool makeTopRankerRecordPacket(unsigned int rank_index,
                                   PacketGuard& packet);

    // IDA_EA: 0x8468DE0
    // IDA_NAME: _ZN8WongWork18CDeathTowerRanking14getVirtualRankEjjj
    int getVirtualRank(unsigned int rank_index,
                       unsigned int stage,
                       unsigned int play_time);

    // IDA_EA: 0x8469D69
    // IDA_NAME: _ZN8WongWork18CDeathTowerRanking23makeDungeonIdx2TowerIdxEj
    static int makeDungeonIdx2TowerIdx(unsigned int dungeon_index);

private:
    // IDA_EA: 0x8468CB6
    // IDA_NAME: _ZN8WongWork18CDeathTowerRanking17_makeRecordPacketERKNS0_14stBestRecord_tER11PacketGuard
    int _makeRecordPacket(const stBestRecord_t& record,
                          PacketGuard& packet);

    // IDA_EA: 0x8469D90 / 0x8469DAE
    // IDA_NAME: _ZN8WongWork18CDeathTowerRanking16_getRankingTableEj /
    //           _ZN8WongWork18CDeathTowerRanking19_getBestRecordTableEj
    RankingTable* _getRankingTable(unsigned int rank_index);
    BestRecordTable* _getBestRecordTable(unsigned int rank_index);

    // Exact C2 ordering at 0x82A787C.  The fixed table bounds are original
    // behavior; higher-level callers validate their ids before reaching this
    // object, so no new bounds policy is introduced here.
    stBestRecord_t top_rankers_[5][6];       // +0x0000
    RankingTable ranking_table_[5];          // +0x11d0
    BestRecordTable best_record_table_[5];   // +0x1234
    StageRankingTable stage_ranking_[5][200]; // +0x1298
    stBestRecord_t top_record_;              // +0x7058
    uint32_t member_count_;                  // +0x70f0
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CDeathTowerRanking::stBestRecord_t) == 0x98u,
              "Death-Tower ranking record IDA ABI size must stay 0x98");
static_assert(sizeof(CDeathTowerRanking::RankingTable) == 0x14u,
              "Death-Tower ranking hash table IDA ABI size must stay 0x14");
static_assert(sizeof(CDeathTowerRanking::BestRecordTable) == 0x14u,
              "Death-Tower best-record hash table IDA ABI size must stay 0x14");
static_assert(sizeof(CDeathTowerRanking::StageRankingTable) == 0x18u,
              "Death-Tower stage ranking map IDA ABI size must stay 0x18");
static_assert(sizeof(CDeathTowerRanking) == 0x70f4u,
              "Death-Tower ranking IDA ABI size must stay 0x70f4");
#endif

}  // namespace WongWork

namespace Global {

class CBossTowerFunc {
public:
    // IDA_EA: 0x8146722
    // IDA_NAME: _ZN6Global14CBossTowerFunc13SelectDungeonEP6CPartys17ENUM_DUNGEON_TYPE
    // DEMANGLED: Global::CBossTowerFunc::SelectDungeon(CParty *,short,ENUM_DUNGEON_TYPE)
    static int SelectDungeon(CParty* party,
                             short dungeon_index,
                             ENUM_DUNGEON_TYPE dungeon_mode);

    // IDA_EA: 0x8146824
    // IDA_NAME: _ZN6Global14CBossTowerFunc18DieCharacInDungeonEP5CUser
    // DEMANGLED: Global::CBossTowerFunc::DieCharacInDungeon(CUser *)
    static int DieCharacInDungeon(CUser* user);

    // IDA_EA: 0x8146AD8
    // IDA_NAME: _ZN6Global14CBossTowerFunc10GiveUpGameEP5CUser
    // DEMANGLED: Global::CBossTowerFunc::GiveUpGame(CUser *)
    static int GiveUpGame(CUser* user);

    // IDA_EA: 0x81468AA
    // IDA_NAME: _ZN6Global14CBossTowerFunc6DieMobEP5CUserttRK15MSG_MONSTER_DIEPi
    static int DieMob(CUser* user,
                      unsigned short monster_id,
                      unsigned short monster_type,
                      const MSG_MONSTER_DIE& message,
                      int* result);

    // IDA_EA: 0x8146BA2
    // IDA_NAME: _ZN6Global14CBossTowerFunc7MoveMapEP5CUserPt
    // DEMANGLED: Global::CBossTowerFunc::MoveMap(CUser *,unsigned short *)
    static int MoveMap(CUser* user, unsigned short* map_path);

    // IDA_EA: 0x8146C20
    // IDA_NAME: _ZN6Global14CBossTowerFunc7UseItemEP5CUserRK10Inven_Item
    // DEMANGLED: Global::CBossTowerFunc::UseItem(CUser *,Inven_Item const&)
    static int UseItem(CUser* user, const Inven_Item& item);

    // IDA_EA: 0x8146916
    // IDA_NAME: _ZN6Global14CBossTowerFunc10FinishLoadEP5CUser
    // DEMANGLED: Global::CBossTowerFunc::FinishLoad(CUser *)
    static int FinishLoad(CUser* user);

    // IDA_EA: 0x8146994
    // IDA_NAME: _ZN6Global14CBossTowerFunc7GetItemEP5CUseribc
    // DEMANGLED: Global::CBossTowerFunc::GetItem(CUser *,int,bool,char)
    static int GetItem(CUser* user,
                       int unique_id,
                       bool client_flag,
                       char client_rarity);
};

}  // namespace Global

// IDA unit: 09_dungeon_pvp_party/PvpResultRecvFlag.
struct PvpResultRecvFlag {
    // IDA_EA: 0x85D5F7E
    // IDA_NAME: _ZN17PvpResultRecvFlagC2Ev
    // DEMANGLED: PvpResultRecvFlag::PvpResultRecvFlag(void)
    PvpResultRecvFlag();
    // IDA_EA: 0x85D5F92
    // IDA_NAME: _ZN17PvpResultRecvFlagD2Ev
    // DEMANGLED: PvpResultRecvFlag::~PvpResultRecvFlag()
    ~PvpResultRecvFlag();
    // IDA_EA: 0x85D5F98
    // IDA_NAME: _ZN17PvpResultRecvFlag11SetRecvFlagEi
    // DEMANGLED: PvpResultRecvFlag::SetRecvFlag(int)
    int SetRecvFlag(int index);
    // IDA_EA: 0x85D5FC6
    // IDA_NAME: _ZN17PvpResultRecvFlag5ClearEv
    // DEMANGLED: PvpResultRecvFlag::Clear(void)
    bool Clear();
    // IDA_EA: 0x85D5FF0
    // IDA_NAME: _ZNK17PvpResultRecvFlag6IsRecvEi
    // DEMANGLED: PvpResultRecvFlag::IsRecv(int)const
    int IsRecv(int index) const;
    // IDA_EA: 0x85DF7B2
    // IDA_NAME: _ZNK17PvpResultRecvFlag10IsValidIdxEi
    // DEMANGLED: PvpResultRecvFlag::IsValidIdx(int)const
    bool IsValidIdx(int index) const;

    uint8_t recv_flags[8];
};

// IDA unit: 09_dungeon_pvp_party/PvpResultType.
struct PvpResultType {
    // IDA_EA: 0x85D58DA
    // IDA_NAME: _ZN13PvpResultTypeC2Ev
    // DEMANGLED: PvpResultType::PvpResultType(void)
    PvpResultType();
    // IDA_EA: 0x85D58E0
    // IDA_NAME: _ZN13PvpResultTypeD2Ev
    // DEMANGLED: PvpResultType::~PvpResultType()
    ~PvpResultType();
    // IDA_EA: 0x85D58E6
    // IDA_NAME: _ZN13PvpResultType3SetERKS_
    // DEMANGLED: PvpResultType::Set(PvpResultType const&)
    PvpResultType* Set(const PvpResultType& rhs);
    // IDA_EA: 0x85D5924
    // IDA_NAME: _ZNK13PvpResultType3GetERS_
    // DEMANGLED: PvpResultType::Get(PvpResultType&)const
    int Get(PvpResultType& dst) const;
    // IDA_EA: 0x85D5944
    // IDA_NAME: _ZN13PvpResultType12AddNewResultERKS_
    // DEMANGLED: PvpResultType::AddNewResult(PvpResultType const&)
    PvpResultType* AddNewResult(const PvpResultType& rhs);
    // IDA_EA: 0x85D5DC0
    // IDA_NAME: _ZN13PvpResultType5ClearEv
    // DEMANGLED: PvpResultType::Clear(void)
    PvpResultType* Clear();
    // IDA_EA: 0x85D5EFE
    // IDA_NAME: _ZNK13PvpResultType13GetTotalScoreEv
    // DEMANGLED: PvpResultType::GetTotalScore(void)const
    int GetTotalScore() const;

    int32_t values[33];
};

// IDA unit: 09_dungeon_pvp_party/PvpResultSet.
struct PvpResultSet : public PvpResultType {
    // IDA_EA: 0x85D5F42
    // IDA_NAME: _ZN12PvpResultSetC2Ev
    // DEMANGLED: PvpResultSet::PvpResultSet(void)
    PvpResultSet();
    // IDA_EA: 0x85D5F56
    // IDA_NAME: _ZN12PvpResultSetD2Ev
    // DEMANGLED: PvpResultSet::~PvpResultSet()
    ~PvpResultSet();
    // IDA_EA: 0x85D5F6A
    // IDA_NAME: _ZN12PvpResultSet5ClearEv
    // DEMANGLED: PvpResultSet::Clear(void)
    PvpResultType* Clear();
};

#pragma pack(push, 1)

// IDA unit: 09_dungeon_pvp_party/MSG_RES_PVP_RANK.
struct MSG_RES_PVP_RANK {
    // IDA_EA: 0x8235378
    // IDA_NAME: _ZN16MSG_RES_PVP_RANKC2Ev
    // DEMANGLED: MSG_RES_PVP_RANK::MSG_RES_PVP_RANK(void)
    MSG_RES_PVP_RANK();
    // IDA_EA: 0x823538E
    // IDA_NAME: _ZN16MSG_RES_PVP_RANKD2Ev
    // DEMANGLED: MSG_RES_PVP_RANK::~MSG_RES_PVP_RANK()
    ~MSG_RES_PVP_RANK();

    uint8_t raw_00[13];
    PvpResultType pvp_result;
};

#pragma pack(pop)

// IDA unit: 09_dungeon_pvp_party/CDungeonGainedGold.
struct CDungeonGainedGold {
    CDungeonGainedGold();
    ~CDungeonGainedGold();

    CDungeonGainedGold* setGainedGold(unsigned int gold);
    unsigned int incGainedGold(unsigned int gold);
    // IDA_EA: 0x8697044
    // IDA_NAME: _ZN18CDungeonGainedGold13getGainedGoldEv
    // DEMANGLED: CDungeonGainedGold::getGainedGold(void)
    int getGainedGold();
    CDungeonGainedGold* reset();

    unsigned int gained_gold;
};

static_assert(sizeof(CDungeonGainedGold) == 0x04u,
              "CDungeonGainedGold IDA ABI size must stay 0x04");
static_assert(sizeof(DungeonUserShotCount) == 0x08u,
              "DungeonUserShotCount IDA ABI size must stay 0x08");
static_assert(sizeof(GameResultType) == 0x50u,
              "GameResultType IDA ABI size must stay 0x50");
static_assert(sizeof(CDungeonClearTracer) == 0x18u,
              "CDungeonClearTracer IDA ABI size must stay 0x18");
static_assert(sizeof(PvpResultRecvFlag) == 0x08u,
              "PvpResultRecvFlag IDA ABI size must stay 0x08");
static_assert(sizeof(PvpResultType) == 0x84u,
              "PvpResultType IDA ABI size must stay 0x84");
static_assert(sizeof(PvpResultSet) == 0x84u,
              "PvpResultSet IDA ABI size must stay 0x84");
static_assert(sizeof(MSG_RES_PVP_RANK) == 0x91u,
              "MSG_RES_PVP_RANK IDA ABI size must stay 0x91");

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_BATTLE_FIELD_H_
