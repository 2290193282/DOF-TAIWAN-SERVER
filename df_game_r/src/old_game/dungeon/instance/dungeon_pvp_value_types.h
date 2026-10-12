#ifndef OLDSRV_09_DUNGEON_PVP_PARTY_DUNGEON_PVP_VALUE_TYPES_H_
#define OLDSRV_09_DUNGEON_PVP_PARTY_DUNGEON_PVP_VALUE_TYPES_H_

#include <stddef.h>
#include <stdint.h>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "common/config.h"
#include "old_game/activity/reward/event_base.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/pvp/battlefield/statistics_pvp_runtime.h"
#include "old_game/social/party/party.h"
#include "infra/network/packet/message_base.h"
#include "old_data/PVF/event/blue_marble_script.h"
#include "old_data/PVF/dungeon/maze_script.h"
#include "old_data/PVF/event/seria_room_deco_value_types.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "old_data/PVF/etc/party_matching_info.h"

// Named ABI enums whose original value tables have not yet been recovered.
// The fixed underlying type preserves the IDA C++ symbol identity.
enum ENUM_ASSAULT_TEAM : int;
enum ENUM_CHARACTERJOB : int;


// IDA unit: 09_dungeon_pvp_party/CPCRoomBurningEvent.
class CPCRoomBurningEvent : public CEventBase {
public:
    CPCRoomBurningEvent();
    virtual ~CPCRoomBurningEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
    virtual int IsEventing(CUser* user) const;

    int GetExpFactor();

private:
    int exp_factor_;
};

// IDA unit: 09_dungeon_pvp_party/CPCRoomFatigueEvent.
class CPCRoomFatigueEvent : public CEventBase {
public:
    CPCRoomFatigueEvent();
    virtual ~CPCRoomFatigueEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
    virtual int IsEventing(CUser* user) const;
};

// IDA unit: 09_dungeon_pvp_party/BlueMarbleDungeonEvent.
class BlueMarbleDungeonEvent : public CEventBase {
public:
    BlueMarbleDungeonEvent();
    virtual ~BlueMarbleDungeonEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 09_dungeon_pvp_party/CBloodDungeonRewardFirstEvent.
class CBloodDungeonRewardFirstEvent : public CEventBase {
public:
    CBloodDungeonRewardFirstEvent();
    virtual ~CBloodDungeonRewardFirstEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);

    int GetGold();

private:
    int gold_;
};

// IDA unit: 09_dungeon_pvp_party/CBloodDungeonRewardSecondEvent.
class CBloodDungeonRewardSecondEvent : public CEventBase {
public:
    CBloodDungeonRewardSecondEvent();
    virtual ~CBloodDungeonRewardSecondEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);

    int GetGold();

private:
    int gold_;
};

// IDA unit: 09_dungeon_pvp_party/CDeathTowerWinPointEvent.
class CDeathTowerWinPointEvent : public CEventBase {
public:
    CDeathTowerWinPointEvent();
    virtual ~CDeathTowerWinPointEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);

    long double getBonusRate();

private:
    int bonus_rate_;
};

// IDA unit: 09_dungeon_pvp_party/CPvPExpPenaltyEvent.
class CPvPExpPenaltyEvent : public CEventBase {
public:
    CPvPExpPenaltyEvent();
    virtual ~CPvPExpPenaltyEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 09_dungeon_pvp_party/CPcRoomCardBlankItemEvent.
class CPcRoomCardBlankItemEvent : public CEventBase {
public:
    CPcRoomCardBlankItemEvent();
    virtual ~CPcRoomCardBlankItemEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 09_dungeon_pvp_party/CIntegratedPvPServerMatchEvent.
class CIntegratedPvPServerMatchEvent : public CEventBase {
public:
    CIntegratedPvPServerMatchEvent();
    virtual ~CIntegratedPvPServerMatchEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 09_dungeon_pvp_party/CTournamentPvPEvent.
class CTournamentPvPEvent : public CEventBase {
public:
    CTournamentPvPEvent();
    virtual ~CTournamentPvPEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 09_dungeon_pvp_party/CPvPLiveEvent.
class CPvPLiveEvent : public CEventBase {
public:
    CPvPLiveEvent();
    virtual ~CPvPLiveEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

// IDA unit: 09_dungeon_pvp_party/DimensionPartyCount.
struct DimensionPartyCount {
    DimensionPartyCount();
    DimensionPartyCount(char first_count, char second_count, char third_count);
    ~DimensionPartyCount();

    bool operator==(const DimensionPartyCount& rhs) const;

    char first_count;
    char second_count;
    char third_count;
};

// IDA unit: 09_dungeon_pvp_party/DungeonHoldBalance.
struct DungeonHoldBalance {
    DungeonHoldBalance();
    ~DungeonHoldBalance();

    DungeonHoldBalance* clear();

    int raw_00;
    int raw_04;
};

// IDA unit: 09_dungeon_pvp_party/DungeonPartyBalance.
// IDA refs: 0x8A4B618
struct DungeonPartyBalance {
    ~DungeonPartyBalance();

    float balances[13][4];
    float raw_d0;
    float raw_d4;
};

// IDA unit: 09_dungeon_pvp_party/TowerOfDespairStatistic_Key.
struct TowerOfDespairStatistic_Key {
    TowerOfDespairStatistic_Key();

    int raw_00;
    int raw_04;
};

// IDA unit: 09_dungeon_pvp_party/STPvPGuild.
struct STPvPGuild {
    STPvPGuild();

    int guild_no;
    int point;
    uint8_t grade;
    uint8_t pad_09[3];
};

// IDA unit: 09_dungeon_pvp_party/PvP_GuildWar_Log.
struct PvP_GuildWar_Log {
    PvP_GuildWar_Log();
    ~PvP_GuildWar_Log();

    void* Clear();
    int InsertPvPGuild(uint32_t guild_key, int guildwar_point);
    PvP_GuildWar_Log* SetpvpStartTime(long cur_sec);
    // IDA_EA: 0x85DDAD0
    // IDA_NAME: _ZN16PvP_GuildWar_Log19WriteGuildWarPvPLogEicPP5CUser
    // DEMANGLED: PvP_GuildWar_Log::WriteGuildWarPvPLog(int,char,CUser **)
    void WriteGuildWarPvPLog(int room_index,
                             char battle_mode,
                             CUser** users);

    uint8_t guild_count;
    uint8_t pad_01[3];
    int pvp_start_time;
    STPvPGuild guilds[8];
};

// IDA unit: 09_dungeon_pvp_party/CLevelDungeonPlayStatistic.
struct CLevelDungeonPlayStatistic {
    struct stDungeonPlayInfo {
        int play_count;
        int play_value;
    };

    CLevelDungeonPlayStatistic();
    ~CLevelDungeonPlayStatistic();

    int IncreaseLevelDungeonPlay(int charac_level, int play_value);
    // IDA_EA: 0x860EDF4
    // IDA_NAME: _ZN26CLevelDungeonPlayStatistic14InsertDatabaseEv
    void InsertDatabase();

    std::map<int, stDungeonPlayInfo> entries;
};

// IDA unit: 09_dungeon_pvp_party/CDungeonExpStatistic.
struct CDungeonExpStatistic {
    struct stDungeonExp {
        int exp_value;
        int bonus_value;
        int play_count;
    };

    CDungeonExpStatistic();

    int IncreaseDungeonExp(int charac_level, int exp_value, int bonus_value);
    // IDA_EA: 0x860F110
    // IDA_NAME: _ZN20CDungeonExpStatistic14InsertDatabaseEv
    void InsertDatabase();

    std::map<int, stDungeonExp> entries;
};

// IDA unit: 09_dungeon_pvp_party/UseCoinDungeonDiff.
struct UseCoinDungeonDiff {
    UseCoinDungeonDiff();

    int dungeon_index;
    uint8_t difficulty;
    uint8_t pad_05[3];
};

// IDA unit: 09_dungeon_pvp_party/STDungeonInoutCnt.
struct STDungeonInoutCnt {
    STDungeonInoutCnt();

    int normal_entrance_total;  // +0x00: normal-dungeon entrances
    int tower_entrance_total;   // +0x04: tower-dungeon entrances
    int normal_entrance_hour;   // +0x08: normal entrances since hourly flush
    int normal_active_count;    // +0x0c: current normal-dungeon occupants
};

static_assert(sizeof(STDungeonInoutCnt) == 0x10u &&
                  offsetof(STDungeonInoutCnt, normal_entrance_total) == 0x00u &&
                  offsetof(STDungeonInoutCnt, tower_entrance_total) == 0x04u &&
                  offsetof(STDungeonInoutCnt, normal_entrance_hour) == 0x08u &&
                  offsetof(STDungeonInoutCnt, normal_active_count) == 0x0cu,
              "STDungeonInoutCnt fields must retain the 0627 ABI");

// IDA unit: 09_dungeon_pvp_party/STPvPBuddyDBInfo.
struct STPvPBuddyDBInfo {
    STPvPBuddyDBInfo();

    uint8_t raw[43];
};

// IDA unit: 09_dungeon_pvp_party/STDeadTowerData.
struct STDeadTowerData {
    STDeadTowerData();
    ~STDeadTowerData();

    int clear();

    std::vector<int> values;
};

// IDA unit: 09_dungeon_pvp_party/STDeadTowerDialog.
struct STDeadTowerDialog {
    STDeadTowerDialog();
    STDeadTowerDialog(const STDeadTowerDialog& rhs);
    ~STDeadTowerDialog();

    STDeadTowerDialog* clear();
    STDeadTowerDialog& operator=(const STDeadTowerDialog& rhs);

    uint32_t raw_00;
    std::string dialog;
    uint8_t raw_08;
    uint8_t raw_09;
    uint8_t raw_0a;
};

// IDA unit: 09_dungeon_pvp_party/STTowerDialog.
struct STTowerDialog {
    STTowerDialog();
    STTowerDialog(const STTowerDialog& rhs);
    ~STTowerDialog();

    STTowerDialog* clear();
    STTowerDialog& operator=(const STTowerDialog& rhs);

    uint32_t raw_00;
    std::string dialog;
    uint8_t raw_08;
    uint8_t raw_09;
    uint8_t raw_0a;
};

// IDA unit: 09_dungeon_pvp_party/STTowerFullDialog.
struct STTowerFullDialog {
    STTowerFullDialog();
    ~STTowerFullDialog();

    int clear();

    std::vector<STTowerDialog> start_dialogs;
    std::vector<STTowerDialog> end_dialogs;
    std::vector<STTowerDialog> clear_dialogs;
};

// IDA unit: 09_dungeon_pvp_party/stDungeonAssignItem_t.
struct stDungeonAssignItem_t {
    stDungeonAssignItem_t();
    stDungeonAssignItem_t(const stDungeonAssignItem_t& rhs);
    ~stDungeonAssignItem_t();

    stDungeonAssignItem_t& operator=(const stDungeonAssignItem_t& rhs);
    int clear();

    int dungeon_index;
    std::list<std::pair<int, int> > items;
};

// IDA unit: 09_dungeon_pvp_party/stPvpRefTable_t.
struct stPvpRefTable_t {
    struct stPvPGradeExp_t {
        stPvPGradeExp_t();
        stPvPGradeExp_t(int grade_value, unsigned int exp_value);

        int grade;
        unsigned int exp;
    };

    stPvpRefTable_t();
    ~stPvpRefTable_t();
    void swap(stPvpRefTable_t& other);

    int max_grade_plus_one;
    std::vector<stPvPGradeExp_t> grade_exp_table;
    int pvp_exp_loss_grade;
    int pvp_exp_loss_exp;
    int pvp_rank_rate_grade;
    std::vector<int> pvp_rank_rate;
    std::vector<int> raw_28_values;

    // Appended 0828 pvp_ref.etc scalars.  Keeping the recovered 0627
    // members first preserves every existing member offset while extending
    // the concrete value object to the newer file schema.
    int pvp_rank_total_count;
    int pvp_direct_wait;
    int pvp_timeout;
    int pvp_rematch;
    int pvp_reward_item_index;
    int pvp_reward_item_count;
    int pvp_reward_item_limit_per_day;
};

// IDA unit: 09_dungeon_pvp_party/RefPvpGrade.
struct RefPvpGrade {
    struct RefPvpGradeData {
        int curr_rank_point;
        int next_rank_point;
    };

    RefPvpGrade();
    ~RefPvpGrade();
    void swap(RefPvpGrade& other);

    int getPvPExpLossGrade();
    int getPvPExpLossExp();
    int getPvPRankRateGrade();
    std::vector<int>* getPvPRankRate();
    int getPvPRankTotalCount() const;
    int getPvPDirectWait() const;
    int getPvPTimeout() const;
    int getPvPRematch() const;
    int getPvPRewardItemIndex() const;
    int getPvPRewardItemCount() const;
    int getPvPRewardItemLimitPerDay() const;
    bool Clear();
    int SetLastGrade(int grade);
    int GetLastGrade() const;
    int SetPvpGrade(int grade, const RefPvpGradeData& data);
    int GetPvpGrade(int pvp_point) const;
    int GetPvpPoint(int grade) const;
    int GetPvpNextRankPoint(int grade) const;
    int GetPvpCurrRankPoint(int grade) const;
    std::vector<int>* set(const stPvpRefTable_t& table);

    int last_grade;
    RefPvpGradeData grade_ranges[38];
    int pvp_exp_loss_grade;
    int pvp_exp_loss_exp;
    int pvp_rank_rate_grade;
    std::vector<int> pvp_rank_rate;
    int pvp_rank_total_count;
    int pvp_direct_wait;
    int pvp_timeout;
    int pvp_rematch;
    int pvp_reward_item_index;
    int pvp_reward_item_count;
    int pvp_reward_item_limit_per_day;
};

// IDA unit: 09_dungeon_pvp_party/DungeonKey.
struct DungeonKey {
    ~DungeonKey();

    bool operator<(const DungeonKey& rhs) const;

    int dungeon_no;
    int dungeon_type;
};

// IDA unit: 09_dungeon_pvp_party/AdditionalDungeonInfo.
struct AdditionalDungeonInfo {
    ~AdditionalDungeonInfo();

    // ImportQuestScript @0x8A61813 reads exactly two signed dwords for each
    // [dungeon info] row.  The old vector<AdditionalDungeonInfo>::push_back
    // advances its end pointer by eight bytes at 0x8A66428, fixing this
    // concrete value layout.
    int32_t dungeon_index;
    int32_t value;
};

static_assert(sizeof(AdditionalDungeonInfo) == 0x08u,
              "AdditionalDungeonInfo IDA ABI size must stay 0x08");

// IDA unit: 09_dungeon_pvp_party/DungeonClearItem.
struct DungeonClearItem {
    DungeonClearItem();
    ~DungeonClearItem();

    DungeonClearItem* clear();

    int reward_type;
    int dungeon_index;
    int item_id;
    int item_count;
    int difficulty;
    int raw_14;
};

// IDA unit: 09_dungeon_pvp_party/stDunGeonLagInfo.
struct stDunGeonLagInfo {
    stDunGeonLagInfo();
    ~stDunGeonLagInfo();

    uint8_t raw[0x18];
};

// IDA unit: 09_dungeon_pvp_party/dungeonEventHanaseStruct.
// IDA refs: 0x83E53C4, 0x8A60F78, 0x8A6100E, 0x8A61164, 0x8A615FE
struct dungeonEventHanaseStruct {
    dungeonEventHanaseStruct();
    dungeonEventHanaseStruct(const dungeonEventHanaseStruct& rhs);
    ~dungeonEventHanaseStruct();

    dungeonEventHanaseStruct& operator=(
        const dungeonEventHanaseStruct& rhs);
    std::string* getNewDialog(ENUM_CHARACTERJOB character_job);

    uint32_t raw_00;
    uint32_t raw_04;
    uint32_t raw_08;
    uint32_t raw_0c;
    uint8_t raw_10;
    uint8_t pad_11[3];
    uint32_t raw_14;
    uint32_t raw_18;
    uint32_t raw_1c;
    std::string default_dialog;
    uint32_t raw_24;
    std::string* job_dialogs[oldsrv::config::kCharacterJobCount];
};

// IDA unit: 09_dungeon_pvp_party/stPvPTempTable_t.
// IDA refs: 0x8458E92
struct stPvPTempTable_t {
    ~stPvPTempTable_t();

    uint32_t charac_no;
    int pvp_grade;
    int pvp_point;
    int last_play_time;
};

#pragma pack(push, 1)

// IDA unit: 09_dungeon_pvp_party/SIG_SAVE_PC_ROOM_PLAY_TIME.
// IDA refs: 0x848FB64
struct SIG_SAVE_PC_ROOM_PLAY_TIME {
    SIG_SAVE_PC_ROOM_PLAY_TIME();

    uint32_t account_id;
    uint32_t play_time;
    uint8_t reward_index;
};

#pragma pack(pop)

// IDA unit: 09_dungeon_pvp_party/SIG_SAVE_PC_ROOM_RENT_INFO.
// IDA refs: 0x848F98E, 0x848F9A2
struct SIG_SAVE_PC_ROOM_RENT_INFO {
    SIG_SAVE_PC_ROOM_RENT_INFO();

    SIG_SAVE_PC_ROOM_RENT_INFO* clear();

    uint32_t item_id;
    uint8_t slot;
    uint8_t rent_type;
    uint8_t state;
    uint8_t pad_07;
    int expire_time;
};

// IDA unit: 09_dungeon_pvp_party/SIG_SAVE_PC_ROOM_RENT_ITEM.
// IDA refs: 0x8491CBE, 0x8694C9A
struct SIG_SAVE_PC_ROOM_RENT_ITEM {
    SIG_SAVE_PC_ROOM_RENT_ITEM();

    bool clear();

    // IDA_EA: CUser::GetPcRoomRentInfo @0x86769EA writes exactly one
    // SIG_SAVE_PC_ROOM_RENT_INFO record into this 0x0c-byte payload.
    SIG_SAVE_PC_ROOM_RENT_INFO items[1];
};

namespace ExtreamDungeon {

// IDA unit: 09_dungeon_pvp_party/ExtreamDungeon/OUTPUT_ITEM_STR.
struct OUTPUT_ITEM_STR {
    OUTPUT_ITEM_STR(int first_value, int second_value);
    ~OUTPUT_ITEM_STR();

    bool operator<(const OUTPUT_ITEM_STR& rhs) const;

    int first;
    int second;
};

// IDA unit: 09_dungeon_pvp_party/ExtreamDungeon/forceRule_ResultItem.
struct forceRule_ResultItem {
    forceRule_ResultItem(bool force_value, int item_index_value);

    bool force;
    uint8_t pad_01[3];
    int item_index;
};

// IDA unit: 09_dungeon_pvp_party/ExtreamDungeon.
bool BindTable_Predicate(
    const OUTPUT_ITEM_STR& lhs,
    const OUTPUT_ITEM_STR& rhs);

}  // namespace ExtreamDungeon

// IDA unit: 09_dungeon_pvp_party/stLeavePartyParam.
struct stLeavePartyParam : public ParamBase {
    stLeavePartyParam();
    virtual ~stLeavePartyParam();

    int raw_04;
};

void stLeavePartyParam_deleting_destructor(stLeavePartyParam* self);

// IDA unit: 09_dungeon_pvp_party/stPartyTeleport.
struct stPartyTeleport : public ParamBase {
    stPartyTeleport();
    virtual ~stPartyTeleport();

    // IDA_EA: 0x81DC001, 0x81DC014, 0x81DC02D.
    // The concrete dispatcher writes these three fields at +0x04, +0x08,
    // and +0x0c respectively.  The implicit i386 alignment before the final
    // word is compiler layout, not an opaque ABI tail.
    int result_code;
    uint8_t village_index;
    uint32_t aura_cooldown_remaining;
};

void stPartyTeleport_deleting_destructor(stPartyTeleport* self);

// IDA unit: 09_dungeon_pvp_party/stPartyTeleportConfirm.
struct stPartyTeleportConfirm : public ParamBase {
    stPartyTeleportConfirm();
    virtual ~stPartyTeleportConfirm();

    // IDA_EA: 0x81DC5EE.  The concrete CMD436 dispatcher writes the
    // transaction result at +0x04 and copies the request state to +0x08.
    // The trailing three bytes are compiler alignment after the named byte,
    // not an opaque ABI tail.
    int result_code;
    uint8_t requested_state;
};

#if defined(__i386__)
static_assert(sizeof(stPartyTeleportConfirm) == 0x0cu,
              "CMD436 parameter must retain its original i386 extent");
static_assert(offsetof(stPartyTeleportConfirm, result_code) == 0x04u,
              "CMD436 result code must remain at +0x04");
static_assert(offsetof(stPartyTeleportConfirm, requested_state) == 0x08u,
              "CMD436 requested state must remain at +0x08");
#endif

void stPartyTeleportConfirm_deleting_destructor(
    stPartyTeleportConfirm* self);

// IDA unit: 09_dungeon_pvp_party/stQuickPartyParam.
struct stQuickPartyParam : public ParamBase {
    stQuickPartyParam();
    virtual ~stQuickPartyParam();

    // IDA_EA: 0x81DE234, 0x81DE374.  Each quick-party dispatcher stores
    // the operation result here and send() serializes it as the command
    // error byte.
    int result_code;
};

void stQuickPartyParam_deleting_destructor(stQuickPartyParam* self);

// IDA unit: 09_dungeon_pvp_party/stBossDieCheckParam.
// C2 @ 0x8231140 establishes this exact 0x10-byte ParamBase-derived layout.
// The byte at +0x09 is compiler alignment between the completion flag and
// the acknowledged monster id; it is intentionally represented by natural
// alignment rather than a synthetic padding member.
struct stBossDieCheckParam : public ParamBase {
    stBossDieCheckParam();
    virtual ~stBossDieCheckParam();

    int result_code;                         // +0x04
    uint8_t map_order_depleted;              // +0x08
    uint16_t acknowledged_monster_unique_id; // +0x0a
    int cmd183_status;                       // +0x0c
};

#if defined(__i386__)
static_assert(sizeof(stBossDieCheckParam) == 0x10u,
              "stBossDieCheckParam must retain its IDA 16-byte ABI");
static_assert(offsetof(stBossDieCheckParam, result_code) == 0x04u,
              "stBossDieCheckParam result-code offset");
static_assert(offsetof(stBossDieCheckParam, map_order_depleted) == 0x08u,
              "stBossDieCheckParam completion-flag offset");
static_assert(offsetof(stBossDieCheckParam,
                       acknowledged_monster_unique_id) == 0x0au,
              "stBossDieCheckParam monster-id offset");
static_assert(offsetof(stBossDieCheckParam, cmd183_status) == 0x0cu,
              "stBossDieCheckParam command-status offset");
#endif

// IDA unit: 09_dungeon_pvp_party/stOnPvPTimeOut.
struct stOnPvPTimeOut : public ParamBase {
    stOnPvPTimeOut();
    virtual ~stOnPvPTimeOut();

    int room_id;
};

void stOnPvPTimeOut_deleting_destructor(stOnPvPTimeOut* self);

// IDA unit: 09_dungeon_pvp_party/stOnSetPvPReadyParam.
// IDA refs: 0x8230E5A, 0x8247E40, 0x8247E70
struct stOnSetPvPReadyParam : public ParamBase {
    stOnSetPvPReadyParam();
    virtual ~stOnSetPvPReadyParam();

    int raw_04;
};

void stOnSetPvPReadyParam_deleting_destructor(
    stOnSetPvPReadyParam* self);

// IDA unit: 09_dungeon_pvp_party/stSetPartyInfo.
// IDA refs: 0x8230CAE, 0x82480B0, 0x82480E0
struct stSetPartyInfo : public ParamBase {
    stSetPartyInfo();
    virtual ~stSetPartyInfo();

    int raw_04;
};

void stSetPartyInfo_deleting_destructor(stSetPartyInfo* self);

// IDA unit: 09_dungeon_pvp_party/stSelectDungeonParam.
// IDA refs: 0x8230DD8, 0x8247EDC, 0x8247F0C
struct stSelectDungeonParam : public ParamBase {
    stSelectDungeonParam();
    virtual ~stSelectDungeonParam();

    int raw_04;
    int raw_08;
    int raw_0c;
    int raw_10;
    int raw_14;
};

void stSelectDungeonParam_deleting_destructor(
    stSelectDungeonParam* self);

// IDA unit: 09_dungeon_pvp_party/stChangeHostWarRoom.
// IDA refs: 0x8231700, 0x8246DCC, 0x8246DFC
struct stChangeHostWarRoom : public ParamBase {
    stChangeHostWarRoom();
    virtual ~stChangeHostWarRoom();

    // IDA 0x81D73F6 writes the WarRoom host-change return value here.
    int result_code;
};

void stChangeHostWarRoom_deleting_destructor(
    stChangeHostWarRoom* self);

// IDA unit: 09_dungeon_pvp_party/MSG_CHANGE_HOST_WARROOM.
// Dispatcher_ChangeHostWarRoom::read @ 0x81D735E reads the seat selector
// immediately after the 13-byte C2S base, followed by the unaligned dword
// selection value.  This is a wire object, so retain the old one-byte pack.
#pragma pack(push, 1)
struct MSG_CHANGE_HOST_WARROOM : public MSG_BASE {
    char requested_seat;               // +0x0d
    uint32_t host_change_value;        // +0x0e
};
#pragma pack(pop)

static_assert(sizeof(MSG_CHANGE_HOST_WARROOM) == 0x12u,
              "CMD404 request must retain the 18-byte old-server ABI");
static_assert(offsetof(MSG_CHANGE_HOST_WARROOM, requested_seat) == 0x0du,
              "CMD404 seat selector must stay at +0x0d");
static_assert(offsetof(MSG_CHANGE_HOST_WARROOM, host_change_value) == 0x0eu,
              "CMD404 host-change value must stay at +0x0e");

// IDA unit: 09_dungeon_pvp_party/MSG_WARROOM_WP_PER_MONSTER.
// The native client sender at 0xA221E2 writes two dwords followed by the
// target world id and a party-seat selector (0xff means no selected seat).
#pragma pack(push, 1)
struct MSG_WARROOM_WP_PER_MONSTER : public MSG_BASE {
    int32_t monster_index;        // +0x0d
    int32_t monster_hp;           // +0x11
    uint16_t target_world_id;     // +0x15; 0xffff means no target user
    uint16_t target_party_seat;   // +0x17; 0x00ff means no selected seat
};
#pragma pack(pop)

static_assert(sizeof(MSG_WARROOM_WP_PER_MONSTER) == 0x19u,
              "CMD374 request must retain the native 25-byte ABI");
static_assert(offsetof(MSG_WARROOM_WP_PER_MONSTER, monster_index) == 0x0du,
              "CMD374 monster index must stay at +0x0d");
static_assert(offsetof(MSG_WARROOM_WP_PER_MONSTER, monster_hp) == 0x11u,
              "CMD374 monster HP must stay at +0x11");
static_assert(offsetof(MSG_WARROOM_WP_PER_MONSTER, target_world_id) == 0x15u,
              "CMD374 target world id must stay at +0x15");
static_assert(offsetof(MSG_WARROOM_WP_PER_MONSTER, target_party_seat) == 0x17u,
              "CMD374 target party seat must stay at +0x17");

// IDA unit: 09_dungeon_pvp_party/stWalkoutPartyParam.
// IDA refs: 0x8230F3E, 0x8247CBA, 0x8247CEA
struct stWalkoutPartyParam : public ParamBase {
    stWalkoutPartyParam();
    virtual ~stWalkoutPartyParam();

    int raw_04;
};

void stWalkoutPartyParam_deleting_destructor(
    stWalkoutPartyParam* self);

// IDA unit: 09_dungeon_pvp_party/MSG_WALKOUT_PARTY.
// The old state-3 template reads exactly the one signed selector byte after
// the 13-byte C2S base before calling CParty::walkout_member.
#pragma pack(push, 1)
struct MSG_WALKOUT_PARTY : public MSG_BASE {
    char seat;  // +0x0d, signed selector in the original member call
};
#pragma pack(pop)

static_assert(sizeof(MSG_WALKOUT_PARTY) == 0x0eu,
              "MSG_WALKOUT_PARTY must retain the 14-byte old-server ABI");

// IDA unit: 09_dungeon_pvp_party/MSG_SET_READY_STATE.
// CMD56 reads exactly one ready-state byte after the confirmed 13-byte C2S
// base. The raw template owns an unconstructed 14-byte stack message.
#pragma pack(push, 1)
struct MSG_SET_READY_STATE : public MSG_BASE {
    char ready;
};
#pragma pack(pop)

static_assert(sizeof(MSG_SET_READY_STATE) == 0x0eu,
              "CMD56 must retain the 14-byte old-server request ABI");

// IDA unit: 09_dungeon_pvp_party/MSG_PVP_TIMEOUT.
// CMD59 consumes eight signed 32-bit score values immediately after MSG_BASE.
#pragma pack(push, 1)
struct MSG_PVP_TIMEOUT : public MSG_BASE {
    int32_t scores[8];
};
#pragma pack(pop)

static_assert(sizeof(MSG_PVP_TIMEOUT) == 0x2du,
              "CMD59 must retain the 45-byte old-server request ABI");

// IDA unit: 09_dungeon_pvp_party/MSG_SELECT_DUNGEON.
// DisPatcher_SelectDungeon::read @ 0x81C7E10 reads exactly these four
// C2S fields after MSG_BASE.  Its process/check_error bodies at
// 0x81C7F32/0x81C8102 prove the index is a short and the trailing request
// flag must be zero; the two bytes between are passed unchanged to the
// dungeon/altar/tower start paths.
#pragma pack(push, 1)
struct MSG_SELECT_DUNGEON : public MSG_BASE {
    int16_t dungeon_index;       // +0x0d
    char dungeon_difficulty;     // +0x0f
    char dungeon_mode;           // +0x10
    char request_flag;           // +0x11; only 0 is an accepted request
};
#pragma pack(pop)

static_assert(sizeof(MSG_SELECT_DUNGEON) == 0x12u,
              "CMD16 request must retain the 18-byte old-server ABI");
static_assert(offsetof(MSG_SELECT_DUNGEON, dungeon_index) == 0x0du,
              "CMD16 dungeon index must stay at +0x0d");
static_assert(offsetof(MSG_SELECT_DUNGEON, dungeon_difficulty) == 0x0fu,
              "CMD16 difficulty must stay at +0x0f");
static_assert(offsetof(MSG_SELECT_DUNGEON, dungeon_mode) == 0x10u,
              "CMD16 mode must stay at +0x10");
static_assert(offsetof(MSG_SELECT_DUNGEON, request_flag) == 0x11u,
              "CMD16 request flag must stay at +0x11");

// IDA units: 09_dungeon_pvp_party/CMDPacketStruct.
//
// The two event-dungeon request objects are constructed from STBaseRequest,
// which the old i386 code proves is just MSG_BASE, then zeroed over their
// exact stack extents (0x18 and 0x14).  The response objects inherit the
// polymorphic ParamBase/STBaseResponse chain.  Their constructor deliberately
// clears the same leading 0x06/0x0c bytes as the old binary; the class packing
// below therefore follows the observed template stack extents rather than a
// guessed semantic schema.
namespace CMDPacketStruct {

#pragma pack(push, 1)
struct _STReqEventDungeonDestoryObject : public MSG_BASE {
    _STReqEventDungeonDestoryObject();

    uint8_t byte_0d;
    uint16_t word_0e;
    uint8_t raw_10[8];
};

struct _STResEventDungeonDestoryObject : public ParamBase {
    _STResEventDungeonDestoryObject();
    virtual ~_STResEventDungeonDestoryObject();

    uint16_t word_04;
};

struct _STReqEventDungeonClearRoom : public MSG_BASE {
    _STReqEventDungeonClearRoom();

    uint8_t byte_0d;
    uint16_t word_0e;
    uint16_t word_10;
    uint16_t word_12;
};
#pragma pack(pop)

struct _STResEventDungeonClearRoom : public ParamBase {
    _STResEventDungeonClearRoom();
    virtual ~_STResEventDungeonClearRoom();

    uint16_t word_04;
    uint16_t word_06;
    uint16_t word_08;
};

}  // namespace CMDPacketStruct

#if defined(__i386__)
static_assert(sizeof(CMDPacketStruct::_STReqEventDungeonDestoryObject) == 0x18u,
              "CMD539 request must retain its old-server 24-byte extent");
static_assert(sizeof(CMDPacketStruct::_STResEventDungeonDestoryObject) == 0x06u,
              "CMD539 response must retain its old-server 6-byte extent");
static_assert(sizeof(CMDPacketStruct::_STReqEventDungeonClearRoom) == 0x14u,
              "CMD540 request must retain its old-server 20-byte extent");
static_assert(sizeof(CMDPacketStruct::_STResEventDungeonClearRoom) == 0x0cu,
              "CMD540 response must retain its old-server 12-byte extent");
#endif

// IDA unit: 09_dungeon_pvp_party/stWarRoomWp.
// IDA refs: 0x8231606, 0x8246FA0, 0x8246FD0
struct stWarRoomWp : public ParamBase {
    stWarRoomWp();
    virtual ~stWarRoomWp();

    int raw_04;
};

void stWarRoomWp_deleting_destructor(stWarRoomWp* self);

// IDA unit: 09_dungeon_pvp_party/PcRoomPlayTimeRewardParam.
// IDA refs: 0x8249CA0, 0x8249CBC, 0x8249CEC
struct PcRoomPlayTimeRewardParam : public ParamBase {
    PcRoomPlayTimeRewardParam();
    virtual ~PcRoomPlayTimeRewardParam();

    // IDA_EA: 0x81E4FED. The dispatcher stores the event result at +0x04.
    int result_;
};

void PcRoomPlayTimeRewardParam_deleting_destructor(
    PcRoomPlayTimeRewardParam* self);

// IDA unit: 09_dungeon_pvp_party/stSeriaRoomDecoEvent.
// IDA refs: 0x8231D60, 0x8246660, 0x8246690
struct stSeriaRoomDecoEvent : public ParamBase {
    stSeriaRoomDecoEvent();
    virtual ~stSeriaRoomDecoEvent();

    // Dispatcher_SeriaRoom_Deco_Event writes/serializes these exact values
    // at +0x04/+0x08/+0x0a (0x81E5CDE/0x81E5E86).
    int error_code;
    uint16_t selected_inventory_slot;
    uint16_t deco_flow;
};

void stSeriaRoomDecoEvent_deleting_destructor(
    stSeriaRoomDecoEvent* self);

// IDA unit: 09_dungeon_pvp_party/seriaRoom_AniDeco.
// IDA refs: 0x822EE18
struct seriaRoom_AniDeco {
    seriaRoom_AniDeco();

    uint16_t receive_flow;
    uint16_t deco_flow;
};

static_assert(offsetof(seriaRoom_AniDeco, receive_flow) == 0x00u &&
                  offsetof(seriaRoom_AniDeco, deco_flow) == 0x02u &&
                  sizeof(seriaRoom_AniDeco) == 0x04u,
              "Seria-room animation/deco progress ABI changed");

// IDA unit: 09_dungeon_pvp_party/MSG_PARTY_TELEPORT.
// IDA refs: 0x822DFA4
#pragma pack(push, 1)
struct MSG_PARTY_TELEPORT : public MSG_BASE {
    MSG_PARTY_TELEPORT();

    uint8_t village_index;  // +0x0d
    uint8_t area_index;     // +0x0e
    int16_t position_x;     // +0x0f
    int16_t position_y;     // +0x11
    uint8_t direction;      // +0x13
};

#pragma pack(pop)

// IDA unit: 09_dungeon_pvp_party/MSG_BOSS_DIE_CHECK.
// Dispatcher_BossDieCheck::read @ 0x81CD642 writes its two shorts at +0x0e
// and +0x10 then the checksum at +0x14.  Natural i386 alignment after the
// packed 13-byte MSG_BASE produces the original +0x0d alignment hole without
// modelling it as an opaque/raw payload.
struct MSG_BOSS_DIE_CHECK : public MSG_BASE {
    uint16_t client_reported_monster_id; // +0x0e; 0xffff is hack type 203
    uint16_t monster_unique_id;          // +0x10; matched against map_order_
    int32_t skill_checksum;              // +0x14
};

static_assert(sizeof(MSG_BOSS_DIE_CHECK) == 0x18u,
              "MSG_BOSS_DIE_CHECK must retain its IDA 24-byte ABI");
static_assert(offsetof(MSG_BOSS_DIE_CHECK, client_reported_monster_id) ==
                  0x0eu,
              "MSG_BOSS_DIE_CHECK first short offset");
static_assert(offsetof(MSG_BOSS_DIE_CHECK, monster_unique_id) == 0x10u,
              "MSG_BOSS_DIE_CHECK unique-id offset");
static_assert(offsetof(MSG_BOSS_DIE_CHECK, skill_checksum) == 0x14u,
              "MSG_BOSS_DIE_CHECK checksum offset");

// IDA unit: 09_dungeon_pvp_party/MSG_PARTY_TELEPORT_CONFIRM.
// IDA refs: 0x822DFDE
#pragma pack(push, 1)
struct MSG_PARTY_TELEPORT_CONFIRM : public MSG_BASE {
    MSG_PARTY_TELEPORT_CONFIRM();

    uint8_t byte_0d;  // +0x0d
};
#pragma pack(pop)

// IDA unit: 09_dungeon_pvp_party/MSG_QUICK_PARTY.
// IDA refs: 0x822E06A
#pragma pack(push, 1)
struct MSG_QUICK_PARTY : public MSG_BASE {
    MSG_QUICK_PARTY();

    uint8_t reserved_0d;  // +0x0d, original C2 leaves unchanged
    uint16_t pool_id;     // +0x0e
    uint8_t channel_type; // +0x10
};
#pragma pack(pop)

static_assert(sizeof(MSG_QUICK_PARTY) == 0x11u,
              "CMD444 request must retain the 17-byte old-server ABI");
static_assert(offsetof(MSG_QUICK_PARTY, pool_id) == 0x0eu &&
                  offsetof(MSG_QUICK_PARTY, channel_type) == 0x10u,
              "CMD444 quick-party fields moved from the 0627 ABI");

// IDA unit: 09_dungeon_pvp_party/MSG_MAKE_PVP_ROOM.
// IDA refs: 0x81F81BA
#pragma pack(push, 1)
struct MSG_MAKE_PVP_ROOM : public MSG_BASE {
    char mode;                    // +0x0d
    char title[30];               // +0x0e
    int16_t word_2c;              // +0x2c
    char option_type;             // +0x2e
    int32_t option_length;        // +0x2f
    char option_payload[8];       // +0x33
    uint8_t reserved_3b;          // +0x3b, raw extractor leaves unchanged
    char option_enabled;          // +0x3c
};
#pragma pack(pop)

// IDA unit: 09_dungeon_pvp_party/PVP_ROOM_INFO.
// IDA refs: 0x8568F16, 0x8568F5C
struct PVP_ROOM_INFO {
    PVP_ROOM_INFO();
    ~PVP_ROOM_INFO();

    // get_room_info @0x85D9296 and make_room_info @0x85DB7F6 prove that
    // mode 0 serializes/copies the title, while a nonzero mode suppresses
    // it.  The old server does not establish a narrower business meaning.
    std::string room_title;          // +0x00
    uint8_t room_title_mode;         // +0x04
    uint16_t requested_map_no;       // +0x06
    uint8_t password_enabled;        // +0x08 (PvP_Room::IsExistPassword)
    std::string password;            // +0x0c
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(offsetof(PVP_ROOM_INFO, room_title_mode) == 0x04u,
              "PVP_ROOM_INFO title mode must stay at +0x04");
static_assert(offsetof(PVP_ROOM_INFO, requested_map_no) == 0x06u,
              "PVP_ROOM_INFO requested map must stay at +0x06");
static_assert(offsetof(PVP_ROOM_INFO, password_enabled) == 0x08u,
              "PVP_ROOM_INFO password flag must stay at +0x08");
static_assert(offsetof(PVP_ROOM_INFO, password) == 0x0cu,
              "PVP_ROOM_INFO password must stay at +0x0c");
static_assert(sizeof(PVP_ROOM_INFO) == 0x10u,
              "PVP_ROOM_INFO IDA ABI size must stay 0x10");
#endif

// IDA unit: 09_dungeon_pvp_party/stConditionEnterBossRoom_t.
// IDA refs: 0x836A9BC, 0x836B552, 0x836B568
struct stConditionEnterBossRoom_t {
    stConditionEnterBossRoom_t();
    ~stConditionEnterBossRoom_t();

    int clear();

    int enabled;
    std::vector<int> conditions;
};

namespace pvp_assault {

// IDA unit: 09_dungeon_pvp_party/pvp_assault.
// IDA_EA: 0x82E564C
int GetDirectionByTeam(ENUM_ASSAULT_TEAM team);

// IDA unit: 09_dungeon_pvp_party/pvp_assault/Point2d.
// IDA refs: 0x82F046A, 0x82F0470, 0x82F0486, 0x82F04C0, 0x82F34C0
struct Point2d {
    Point2d();
    Point2d(int x_value, int y_value);
    ~Point2d();

    unsigned int operator-(const Point2d& rhs) const;
    Point2d& operator+=(const Point2d& rhs);

    int x;
    int y;
};

// IDA unit: 09_dungeon_pvp_party/pvp_assault/CandidatePoint.
// IDA refs: 0x82F04EA, 0x82F103C, 0x82F264A
struct CandidatePoint {
    CandidatePoint();
    ~CandidatePoint();

    bool operator<(const CandidatePoint& rhs) const;

    // IDA layout: the ranking value precedes the embedded map point.
    // CCandidateMatrix reads the coordinates from +0x04/+0x08 and the
    // ordering operator reads the value at +0x00.
    int score;
    Point2d point;
};

static_assert(sizeof(CandidatePoint) == 0x0Cu,
              "CandidatePoint IDA ABI size must stay 0x0C");

// IDA unit: 09_dungeon_pvp_party/pvp_assault/PartyInfo.
// IDA refs: 0x82F05EE, 0x82F05FC, 0x82F060A
struct PartyInfo {
    PartyInfo();

    PartyInfo* Set(CParty* party_value);
    CParty* Get();

    CParty* party;
};

}  // namespace pvp_assault

namespace fair_pvp {

// IDA unit: 09_dungeon_pvp_party/fair_pvp/JobKey.
// IDA refs: 0x84A63F2, 0x84A643A
struct JobKey {
    JobKey(ENUM_CHARACTERJOB job_value, uint8_t grow_type_value);

    bool operator<(const JobKey& rhs) const;

    int job;
    uint8_t grow_type;
    uint8_t pad_05[3];
};

}  // namespace fair_pvp

namespace QuickParty {

enum RandomBuffType : int;
enum RandomBuffDungeon : int;
enum ENUM_REWARD_TYPE : int;

// IDA unit: 09_dungeon_pvp_party/QuickParty.
// IDA refs: 0x8A6E100
int getRandomBuffTypeByName(const std::string& name);

// IDA unit: 09_dungeon_pvp_party/QuickParty/RewardData_Param.
// IDA refs: 0x8469976
struct RewardData_Param {
    explicit RewardData_Param(CParty* party_value);

    CParty* party;
    int raw_04;
};

// IDA unit: 09_dungeon_pvp_party/QuickParty/RewardData_Result.
// IDA refs: 0x846998E, 0x84699C6
struct RewardData_Result {
    RewardData_Result();

    RewardData_Result* clear();

    int raw_00;
    int raw_04;
    int raw_08;
    int raw_0c;
    int raw_10;
    float raw_14;
    int raw_18;
};

struct QuickPartyData;

// IDA unit: 09_dungeon_pvp_party/QuickParty/IRewardType.
struct IRewardType {
    // Old IDA address: 0x826DBEE.
    explicit IRewardType(QuickPartyData& data);

    virtual void check(RewardData_Param param) = 0;
    // CParty::set_basic_reward and set_random_reward dispatch the second
    // vtable slot directly.  The six concrete reward implementations below
    // prove that it is the old process(param, result) operation.
    virtual int process(RewardData_Param param,
                        RewardData_Result& result) = 0;

    QuickPartyData* quick_party_data;
};

// IDA unit: 09_dungeon_pvp_party/QuickParty/IBasicReward.
struct IBasicReward : public IRewardType {
    // Old IDA address: 0x826DC06.
    explicit IBasicReward(QuickPartyData& data);

    // Old IDA address: 0x826DC2A.
    virtual void check(RewardData_Param param);
};

// IDA unit: 09_dungeon_pvp_party/QuickParty/IRandomwReward.
struct IRandomwReward : public IRewardType {
    // Old IDA address: 0x826DC78.
    explicit IRandomwReward(QuickPartyData& data);

    // Old IDA address: 0x826DC9C.
    virtual void check(RewardData_Param param);
};

// These six concrete records are allocated by
// CQuickPartyRewardManager::init.  The old allocations establish their exact
// ABI sizes: basic rewards are 0x08 bytes and random rewards add the 0x08
// random-buff selector, for 0x0c bytes total.
struct RewardCardGold : public IBasicReward {
    // IDA_EA: 0x826DC30
    // IDA_NAME: _ZN10QuickParty14RewardCardGoldC2ENS_15BasicRewardTypeERNS_14QuickPartyDataE
    explicit RewardCardGold(BasicRewardType reward_type, QuickPartyData& data);
    // IDA_EA: 0x826D070
    // IDA_NAME: _ZN10QuickParty14RewardCardGold7processENS_16RewardData_ParamERNS_17RewardData_ResultE
    virtual int process(RewardData_Param param, RewardData_Result& result);
};

struct HellPartyDifficulty : public IBasicReward {
    // IDA_EA: 0x826DC54
    // IDA_NAME: _ZN10QuickParty19HellPartyDifficultyC2ENS_15BasicRewardTypeERNS_14QuickPartyDataE
    explicit HellPartyDifficulty(BasicRewardType reward_type,
                                 QuickPartyData& data);
    // IDA_EA: 0x826D116
    // IDA_NAME: _ZN10QuickParty19HellPartyDifficulty7processENS_16RewardData_ParamERNS_17RewardData_ResultE
    virtual int process(RewardData_Param param, RewardData_Result& result);
};

struct KillMonsterExp : public IRandomwReward {
    // IDA_EA: 0x826DCA2
    // IDA_NAME: _ZN10QuickParty14KillMonsterExpC2ENS_14RandomBuffTypeERNS_14QuickPartyDataE
    explicit KillMonsterExp(RandomBuffType random_buff_type,
                            QuickPartyData& data);
    // IDA_EA: 0x826D198
    // IDA_NAME: _ZN10QuickParty14KillMonsterExp7processENS_16RewardData_ParamERNS_17RewardData_ResultE
    virtual int process(RewardData_Param param, RewardData_Result& result);

    int random_buff_type;
};

struct EquipItemDropRate : public IRandomwReward {
    // IDA_EA: 0x826DCCE
    // IDA_NAME: _ZN10QuickParty17EquipItemDropRateC2ENS_14RandomBuffTypeERNS_14QuickPartyDataE
    explicit EquipItemDropRate(RandomBuffType random_buff_type,
                               QuickPartyData& data);

    // IDA_EA: 0x826D2AA
    // IDA_NAME: _ZN10QuickParty17EquipItemDropRate7processENS_16RewardData_ParamERNS_17RewardData_ResultE
    virtual int process(RewardData_Param param, RewardData_Result& result);

    int random_buff_type;
};

struct CointLimitCount : public IRandomwReward {
    // IDA_EA: 0x826DCFA
    // IDA_NAME: _ZN10QuickParty15CointLimitCountC2ENS_14RandomBuffTypeERNS_14QuickPartyDataE
    explicit CointLimitCount(RandomBuffType random_buff_type,
                             QuickPartyData& data);
    // IDA_EA: 0x826D4AC
    // IDA_NAME: _ZN10QuickParty15CointLimitCount7processENS_16RewardData_ParamERNS_17RewardData_ResultE
    virtual int process(RewardData_Param param, RewardData_Result& result);

    int random_buff_type;
};

struct ReturnAncientDungeonTicket : public IRandomwReward {
    // IDA_EA: 0x826DD26
    // IDA_NAME: _ZN10QuickParty26ReturnAncientDungeonTicketC2ENS_14RandomBuffTypeERNS_14QuickPartyDataE
    explicit ReturnAncientDungeonTicket(RandomBuffType random_buff_type,
                                        QuickPartyData& data);
    // IDA_EA: 0x826D598
    // IDA_NAME: _ZN10QuickParty26ReturnAncientDungeonTicket7processENS_16RewardData_ParamERNS_17RewardData_ResultE
    virtual int process(RewardData_Param param, RewardData_Result& result);

    int random_buff_type;
};

// This old binary type is not allocated by CQuickPartyRewardManager::init;
// nevertheless the concrete process slot is present and is reached by the
// quick-party random-reward table when its type is selected.  Keep it as the
// original IRandomwReward-derived record instead of substituting a generic
// callback.
struct DimensionInoutCount : public IRandomwReward {
    // IDA_EA: 0x826D5AC
    // IDA_NAME: _ZN10QuickParty19DimensionInoutCount7processENS_16RewardData_ParamERNS_17RewardData_ResultE
    virtual int process(RewardData_Param param, RewardData_Result& result);

    int random_buff_type;
};

// IDA unit: 09_dungeon_pvp_party/QuickParty/LevelData.
// IDA refs: 0x8A6EF72, 0x8A6FCB8
struct LevelData {
    LevelData();
    ~LevelData();

    int raw_00;
    int raw_04;
};

// IDA unit: 09_dungeon_pvp_party/QuickParty/LevelDataHell.
// IDA refs: 0x8A6EF8A, 0x8A6FEFA
struct LevelDataHell {
    LevelDataHell();
    ~LevelDataHell();

    int raw_00;
    int raw_04;
};

// IDA unit: 09_dungeon_pvp_party/QuickParty/BasicReward.
// IDA refs: 0x826D9F0, 0x826DA7A
struct BasicReward {
    BasicReward();
    ~BasicReward();

    std::vector<LevelData> level_data;
    std::vector<LevelDataHell> hell_level_data;
    int raw_18;
    int raw_1c;
};

// IDA unit: 09_dungeon_pvp_party/QuickParty/LevelDataRandomReward.
// IDA refs: 0x826E77E, 0x8A6EFA2, 0x8A6F112
struct LevelDataRandomReward {
    LevelDataRandomReward();
    LevelDataRandomReward(const LevelDataRandomReward& rhs);
    ~LevelDataRandomReward();

    int raw_00;
    std::string text_04;
    std::vector<LevelData> level_data_08;
};

// IDA unit: 09_dungeon_pvp_party/QuickParty/QuickPartyData.
// IDA refs: 0x826DAD4, 0x826DD52
struct QuickPartyData {
    QuickPartyData();
    ~QuickPartyData();

    std::vector<int> unable_to_quick_party;
    std::vector<int> dungeon_idx_list;
    BasicReward basic_reward;
    std::map<int, LevelDataRandomReward> random_rewards;
    int random_buff_probs[12][7];
    int require_quick_party_min_count[2][4];
    void* reward_type_objects[2][13];
};

// IDA unit: 09_dungeon_pvp_party/QuickParty/CQuickPartyRewardManager.
// IDA refs: 0x826D688, 0x826D69C, 0x826D860, 0x826D896, 0x826D8A2,
//           0x826D8E2, 0x826D91A
struct CQuickPartyRewardManager : public QuickPartyData {
    CQuickPartyRewardManager();
    ~CQuickPartyRewardManager();

    int init();

    int get_random_buff_prob(RandomBuffType random_buff_type,
                             RandomBuffDungeon random_buff_dungeon);
    std::vector<int>* get_dungeon_idx_list();
    int getRequireQuickPartyMinCount(int row, int column);
    void* get_reward_type_object(ENUM_REWARD_TYPE reward_type,
                                 unsigned int index);
    bool set_rand_buff_total_prob();
};

}  // namespace QuickParty


#endif  // OLDSRV_09_DUNGEON_PVP_PARTY_DUNGEON_PVP_VALUE_TYPES_H_
