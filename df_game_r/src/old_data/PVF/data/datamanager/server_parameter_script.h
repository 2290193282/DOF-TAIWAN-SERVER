#ifndef OLDSRV_DATAMANAGER_SERVER_PARAMETER_SCRIPT_H_
#define OLDSRV_DATAMANAGER_SERVER_PARAMETER_SCRIPT_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

// Shared 0627 value identities used by the two in-place item-generation
// reference vectors.  The compatibility header owns no ServerParameterScript
// storage; it only declares these original value types and the legacy C2/D2
// entry points.
#include "common/manager/server_parameter_script_abi.h"
#include "common/manager/shared_server_message_abi.h"
#include "old_data/PVF/etc/assault_parameter_types.h"
#include "old_data/PVF/etc/blood_dungeon_reward_info.h"
#include "old_game/activity/schedule/event_value_types.h"

// IDA_EA: CUser::getCompoundUpgrade @0x8682082.  Each recipe upgrade or
// amplify row contains the 32 cumulative-choice weights for output upgrade
// levels 0..31.  Four 32-row tables are physically owned below.
struct CompoundUpgradeRateRow {
    int32_t rates[32];
};

static_assert(sizeof(CompoundUpgradeRateRow) == 0x80u,
              "compound recipe rate row must stay 32 i386 integers");

// IDA unit: 10_mail_trade_auction_cash/stAuctionLimitItemParam_t.
// ServerParameterScript::C2 constructs the exact vector at +0x480 and the
// old CMD186 dispatcher compares these three signed fields directly.
struct AuctionRegisterItemLimit {
    int32_t item_index;
    int32_t start_sec;
    int32_t end_sec;
};

typedef std::vector<AuctionRegisterItemLimit> AuctionRegisterItemLimitVector;

static_assert(sizeof(AuctionRegisterItemLimit) == 0x0cu,
              "auction registration limit entry must stay 0x0c");

// IDA unit: 99_misc_global/ItemFarmingData.
// ServerParameterScript::C2 constructs std::map<int, ItemFarmingData> at
// +0x4C0 and importServerParameterScript fills it from [drop upgrade rarity].
// GenerateRandomUpgrade::generateUpgrade @0x8533676 reads that exact map
// through CDataManager+0x528.  This must remain a named global type: the old
// executable exports its C2, copy-C2, and D2 under ItemFarmingData's ABI name.
struct ItemFarmingData {
    ItemFarmingData();
    ItemFarmingData(const ItemFarmingData& rhs);
    ~ItemFarmingData();

    std::vector<std::pair<int, int> > list;
    int32_t total_rate;
};

typedef std::map<int, ItemFarmingData> ItemFarmingDataMap;
typedef std::vector<std::pair<int, float> > IntFloatPairVector;

// IDA unit: 09_dungeon_pvp_party/STDungeonInoutTime. The importer constructs
// a map of these at ServerParameterScript +0xC8 from [dungeon inout time],
// while the open/close timers consume that same map. Keep one global ABI type
// instead of allowing the importer and gameplay timer to reinterpret two
// unrelated four-int records.
struct STDungeonInoutTime {
    // IDA_EA: 0x8A8CCFC
    // IDA_NAME: _ZN18STDungeonInoutTimeC2Ev
    // DEMANGLED: STDungeonInoutTime::STDungeonInoutTime(void)
    STDungeonInoutTime();

    int32_t start_hour;
    int32_t end_hour;
    int32_t start_minute;
    int32_t end_minute;
};

static_assert(sizeof(STDungeonInoutTime) == 0x10u &&
                  offsetof(STDungeonInoutTime, start_hour) == 0x00u &&
                  offsetof(STDungeonInoutTime, end_hour) == 0x04u &&
                  offsetof(STDungeonInoutTime, start_minute) == 0x08u &&
                  offsetof(STDungeonInoutTime, end_minute) == 0x0cu,
              "STDungeonInoutTime fields must retain the 0627 ABI");

typedef std::map<int, STDungeonInoutTime> DungeonInoutTimeMap;

// These value/container types are the physical children constructed by
// ServerParameterScript::C2.  Keeping them in the owner header lets the class
// express its real 0627 object graph instead of reopening it through byte
// offsets in the implementation.
struct stServerIndex {
    int32_t server_group;
    int32_t channel_no;

    bool operator<(const stServerIndex& rhs) const {
        if (channel_no < rhs.channel_no) {
            return true;
        }
        return channel_no == rhs.channel_no && server_group < rhs.server_group;
    }
};

struct stLightServerNotice_t {
    int32_t server_group;
    int32_t channel_no;
    std::string message;
};

struct stLuckPoint_t {
    int32_t point_threshold;
    int32_t awarded_luck_point;
};

struct stLuckPointDeduction_t {
    int32_t maximum_character_level;
    std::vector<int> deduction_rates;
};

struct stUnusableEquipmentInfo {
    int32_t equipment_kind;
    int32_t minimum_level;
    int32_t maximum_level;
};

typedef std::vector<std::pair<int, int> > PairIntVector;

// 0828 ServerParameter.etc extensions.  The client does not import this
// server-owned file, so preserve the timestamp text and the unresolved tail
// value exactly until their server-side consumers are recovered.
typedef std::vector<uint32_t> BroadcastItemIdVector;

struct TimedItemGiveEntry {
    std::string begin_time_text;
    std::string end_time_text;
    uint32_t item_id;
    int32_t count;
    int32_t raw_value;
};

typedef std::vector<TimedItemGiveEntry> TimedItemGiveEntryVector;

#if defined(__i386__)
static_assert(sizeof(TimedItemGiveEntry) == 0x14u,
              "timed item-give entry must stay 0x14 on i386");
#endif
typedef std::map<int, int> IntIntMap;
typedef std::vector<int> IntVector;
typedef std::vector<float> FloatVector;
typedef std::vector<stLightServerNotice_t> LightServerNoticeVector;
typedef std::vector<stLuckPoint_t> LuckPointVector;
typedef std::vector<stLuckPointDeduction_t> LuckPointDeductionVector;
typedef std::map<stServerIndex, bool> ServerIndexBoolMap;
typedef std::map<int, bool> IntBoolMap;
typedef std::map<int, PairIntVector> IntPairVectorMap;
typedef std::vector<stUnusableEquipmentInfo> UnusableEquipmentVector;
typedef std::set<uint32_t> UIntSet;
typedef std::map<int, UIntSet> IntUIntSetMap;
typedef std::vector<stFatigueEventParam_t> FatigueEventParamVector;
typedef std::vector<stAssaultChaosPointWeight>
    AssaultChaosPointWeightVector;
typedef std::vector<stAssaultLevelGhostPanalty>
    AssaultLevelGhostPanaltyVector;
typedef std::vector<stAssaultChaosPointGhostPanalty>
    AssaultChaosPointGhostPanaltyVector;
typedef std::map<int, uint32_t> IntUIntMap;

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(stServerIndex) == 0x08u,
              "stServerIndex IDA ABI size must stay 0x08");
static_assert(sizeof(stLightServerNotice_t) == 0x0cu,
              "stLightServerNotice_t IDA ABI size must stay 0x0c");
static_assert(sizeof(stLuckPointDeduction_t) == 0x10u,
              "stLuckPointDeduction_t IDA ABI size must stay 0x10");
static_assert(sizeof(stUnusableEquipmentInfo) == 0x0cu,
              "stUnusableEquipmentInfo IDA ABI size must stay 0x0c");
#endif

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(ItemFarmingData) == 0x10u,
              "ItemFarmingData IDA ABI size must stay 0x10");
static_assert(sizeof(ItemFarmingDataMap) == 0x18u,
              "ItemFarmingDataMap IDA ABI size must stay 0x18");
static_assert(sizeof(IntFloatPairVector) == 0x0cu,
              "IntFloatPairVector IDA ABI size must stay 0x0c");
#endif

// IDA unit: 99_misc_global/stUpgradeItemTicketParam_t.
// importServerParameterScript fills this four-byte value for
// [protect of upgrade down] and CItemUpgrade::ProcUpgrade consumes the first
// three bytes as the inclusive level range and penalty-protection mode.  The
// fourth byte remains ABI-compatible with the original jpn_only flag.  Its
// previously-unused high bits carry the 0828 typed upgrade scope; a zero scope
// is the legacy policy and therefore applies to both reinforcement modes.
struct stUpgradeItemTicketParam_t {
    stUpgradeItemTicketParam_t();

    uint8_t minimum_upgrade;
    uint8_t maximum_upgrade;
    uint8_t penalty_protection;
    uint8_t jpn_only;
};

enum UpgradeProtectionPolicyFlag {
    kUpgradeProtectionJpnOnly = 0x01,
    kUpgradeProtectionReinforcement = 0x02,
    kUpgradeProtectionAmplification = 0x04,
    kUpgradeProtectionScopeMask =
        kUpgradeProtectionReinforcement |
        kUpgradeProtectionAmplification
};

inline bool IsUpgradeProtectionPolicyModeAllowed(
    const stUpgradeItemTicketParam_t& policy,
    uint8_t upgrade_mode) {
    if ((policy.jpn_only & kUpgradeProtectionJpnOnly) != 0u) {
        return false;
    }

    const uint8_t scope = static_cast<uint8_t>(
        policy.jpn_only & kUpgradeProtectionScopeMask);
    if (scope == 0u) {
        return upgrade_mode <= 1u;
    }
    if (upgrade_mode == 0u) {
        return (scope & kUpgradeProtectionReinforcement) != 0u;
    }
    if (upgrade_mode == 1u) {
        return (scope & kUpgradeProtectionAmplification) != 0u;
    }
    return false;
}

inline bool IsUpgradeProtectionPolicyLevelAllowed(
    const stUpgradeItemTicketParam_t& policy,
    unsigned int current_upgrade) {
    return current_upgrade >= policy.minimum_upgrade &&
           current_upgrade <= policy.maximum_upgrade;
}

inline bool IsUpgradeProtectionPolicyApplicable(
    const stUpgradeItemTicketParam_t& policy,
    unsigned int current_upgrade,
    uint8_t upgrade_mode,
    bool destruction_protection_only) {
    return IsUpgradeProtectionPolicyModeAllowed(policy, upgrade_mode) &&
           IsUpgradeProtectionPolicyLevelAllowed(policy, current_upgrade) &&
           (!destruction_protection_only ||
            policy.penalty_protection == 0u);
}

typedef std::map<uint32_t, stUpgradeItemTicketParam_t>
    UpgradeItemTicketMap;

static_assert(sizeof(stUpgradeItemTicketParam_t) == 0x04u,
              "upgrade item ticket parameter must stay 0x04");

// IDA_DATA: importServerParameterScript @0x8A870B7 parses
// [weekend bonus fatigue] and [weekend bonus exp] into +0x4B4/+0x4B8.
struct WeekendBonus {
    uint16_t fatigue;       // +0x00
    uint16_t reserved_02;   // +0x02
    int32_t exp;            // +0x04
};

static_assert(sizeof(WeekendBonus) == 0x08u,
              "weekend bonus record must stay 0x08");

// IDA unit: 99_misc_global/stFatigueBatteryConstantTable_t.  This exact
// object is constructed by ServerParameterScript::C2 at +0x5c0 and populated
// by importServerParameterScript's fatigue-battery tags.  Gameplay reaches it
// through ServerParameterScript; consumers must not reopen the enclosing
// CDataManager layout.
struct stFatigueBatteryConstantTable_t {
    // IDA_EA: 0x836F2CC
    // IDA_NAME: _ZN31stFatigueBatteryConstantTable_tC2Ev
    stFatigueBatteryConstantTable_t();

    // IDA_EA: 0x84A84B6
    // IDA_NAME:
    // _ZNK31stFatigueBatteryConstantTable_t36getLimitGrownupBuffCntFatigueBatteryEi
    int getLimitGrownupBuffCntFatigueBattery(int charac_level) const;

    uint16_t limit_fatigue_battery_charging_amount_;  // +0x000
    uint16_t reserved_002_;
    float end_level_fatigue_battery_weight_;          // +0x004
    uint8_t character_level_fatigue_battery_constance_; // +0x008
    uint8_t limit_fatigue_battery_level_;               // +0x009
    uint16_t reserved_00a_;
    int32_t fatigue_battery_money_amount_;              // +0x00c
    uint8_t fatigue_battery_level_for_money_active_;    // +0x010
    uint8_t reserved_011_;
    uint16_t limit_grownup_buff_count_by_level_[200];   // +0x012
    uint8_t reserved_1a2_[2];
    uint16_t fatigue_battery_per_grownup_buff_active_;  // +0x1a4
    uint16_t grownup_buff_per_fatigue_battery_active_;  // +0x1a6
    uint16_t min_fatigue_battery_per_buff_active_;      // +0x1a8
    uint16_t reserved_1aa_;                             // +0x1aa
    float pc_room_party_exp_bonus_rate_;                // +0x1ac
};

static_assert(sizeof(stFatigueBatteryConstantTable_t) == 0x1b0u,
              "fatigue-battery table must retain its 0x1b0-byte ABI");
static_assert(offsetof(stFatigueBatteryConstantTable_t,
                       end_level_fatigue_battery_weight_) == 0x004u &&
                  offsetof(stFatigueBatteryConstantTable_t,
                           character_level_fatigue_battery_constance_) ==
                      0x008u &&
                  offsetof(stFatigueBatteryConstantTable_t,
                           fatigue_battery_level_for_money_active_) ==
                      0x010u &&
                  offsetof(stFatigueBatteryConstantTable_t,
                           fatigue_battery_per_grownup_buff_active_) ==
                      0x1a4u &&
                  offsetof(stFatigueBatteryConstantTable_t,
                           min_fatigue_battery_per_buff_active_) == 0x1a8u &&
                  offsetof(stFatigueBatteryConstantTable_t,
                           pc_room_party_exp_bonus_rate_) == 0x1acu,
              "fatigue-battery table field offsets must retain the old ABI");

// IDA unit: 05_world_enter_village/Inter_VillageAttackedEnd.  The
// ServerParameterScript C2/D2 construct/destroy this vector at +0x498, and
// the original event-end receiver selects the first inclusive attack-count
// range before persisting the user's revenge-dungeon count.
struct ConditionEnterRevenge {
    int32_t minimum_attack_count;
    int32_t maximum_attack_count;
    int32_t revenge_dungeon_count;
};

typedef std::vector<ConditionEnterRevenge> ConditionEnterRevengeVector;

static_assert(sizeof(ConditionEnterRevenge) == 0x0cu,
              "revenge-dungeon condition entry must stay 0x0c");

// IDA_EA: importServerParameterScript @0x8A870B7 parses
// [emblem compound info] at +0x7C0.  Each original row is selected by
// emblem grade and keeps the money charge followed by five socket-kind
// result item ids.  CMD260 used to project the same storage through
// CDataManager+0x828; it now crosses this B-owned boundary by name.
struct EmblemCompoundInfo {
    int32_t money_cost;
    int32_t result_item_ids[5];
};

static_assert(sizeof(EmblemCompoundInfo) == 0x18u,
              "emblem compound row must retain the old 0x18-byte layout");

// CUser::lottery_item @0x86733A2 reads this contiguous scalar policy from
// ServerParameterScript+0x528 (CDataManager+0x590).  It belongs to the
// embedded server-parameter owner rather than to a CDataManager overlay.
struct LotteryUpgradePolicy {
    int32_t first_upgrade_base;
    int32_t first_upgrade_ceiling;
    int32_t first_band_probability;
    int32_t first_band_roll_range;
    int32_t second_upgrade_base;
    int32_t second_upgrade_ceiling;
    int32_t second_band_probability;
    int32_t second_band_roll_range;
    int32_t third_upgrade_base;
    int32_t third_upgrade_ceiling;
    int32_t third_band_probability;
    int32_t third_band_roll_range;
    int32_t broadcast_minimum_rarity;
    uint8_t broadcast_avatar_item_types[0x1c];
    int32_t broadcast_minimum_upgrade;
};

static_assert(sizeof(LotteryUpgradePolicy) == 0x54u,
              "lottery upgrade policy must stay 0x54");


// IDA unit: 12_pvf_datamanager_scripts/ServerParameterScript plus the
// B-owned importServerParameterScript boundary from RDARScriptServerParameter.
class ServerParameterScript {
public:
    struct PvPPlayerKey {
        PvPPlayerKey(unsigned char charac_job,
                     unsigned char first_grow_type,
                     unsigned char pvp_grade);
        bool operator<(const PvPPlayerKey& rhs) const;

        unsigned char charac_job;
        unsigned char first_grow_type;
        unsigned char pvp_grade;
    };

    typedef std::map<PvPPlayerKey, PairIntVector> PvpFirstGiveMap;

    ServerParameterScript();
    ~ServerParameterScript();

    uint32_t get_script_version() const;
    uint32_t get_antibot_server_version() const;
    void set_antibot_server_version(uint32_t version);
    uint8_t get_antibot_version_digit(unsigned int index) const;
    void apply_accept_handshake_reward_weights();
    int get_assault_price_time_weight_limit() const;
    int get_assault_price_time_limit_seconds() const;
    float get_assault_price_level_gap_weight() const;
    int get_compound_avatar_upper_rate_bonus() const;
    int get_compound_avatar_lower_rate_reduction() const;
    // 0828 PVF semantic extension: [master bind cube rate bonus].
    // No 0627 server IDA symbol exists for this field.
    int get_compound_avatar_master_rate_bonus() const;
    // Typed owner boundaries for the 0828-only server-parameter records.
    const BroadcastItemIdVector& get_broadcast_item_ids() const;
    const TimedItemGiveEntryVector& get_timed_item_gives() const;
    int get_normal_fatigue_luck_point_rate() const;
    int get_premium_fatigue_luck_point_rate() const;
    ItemGenRefVector* get_item_gen_ref_vector_0();
    ItemGenRefVector* get_item_gen_ref_vector_1();
    const BurningFatigueParamVector& get_burning_fatigue_params() const;
    int get_event_dungeon_clear_reward_item_id() const;
    float get_premium_gold_card_commission_multiplier() const;
    const LotteryUpgradePolicy& get_lottery_upgrade_policy() const;
    int get_prevent_prefix_count() const;
    const char* get_prevent_prefix(int index) const;

    void* clear();
    // IDA_EA: importServerParameterScript @0x8A870B7 recognizes
    // [black list help parameter] and stores the two scanned integers at
    // +0x34C/+0x350.  DB_ReqBlackCount @0x8424CE6 consumes those exact
    // typed values through CDataManager's embedded script at +0x68.
    int getBlackListHelpPeriodDays() const;
    int getBlackListHelpOffsetPointsPerPeriod() const;
    // Declaration-only B ABI bridge, not an old exported method.  C2/D2 and
    // importServerParameterScript prove the vector at +0x480; the C commerce
    // dispatcher uses this typed edge rather than an opaque layout overlay.
    const AuctionRegisterItemLimitVector&
    get_auction_register_item_limits() const;
    // B-owned typed boundary for the vector constructed at +0x498.  This is
    // not a newly exported old-server method; it keeps world gameplay from
    // reinterpreting ServerParameterScript storage directly.
    const ConditionEnterRevengeVector&
    get_condition_enter_revenge() const;
    // B-owned typed bridge to the map constructed at +0x468.  The enclosing
    // CDataManager embeds this script at +0x68, matching CItemUpgrade's
    // original CDataManager+0x4D0 access exactly.
    UpgradeItemTicketMap& get_upgrade_item_tickets();
    const UpgradeItemTicketMap& get_upgrade_item_tickets() const;
    // B-owned typed edges for the two original drop-upgrade tables.  C2/D2
    // prove their exact offsets (+0x4C0/+0x4D8); the generator must not
    // reinterpret CDataManager storage outside this canonical owner.
    ItemFarmingDataMap& get_item_farming_data();
    const ItemFarmingDataMap& get_item_farming_data() const;
    const IntFloatPairVector&
    get_drop_bonus_of_dungeon_level_increase() const;
    // B-owned projection of [member bonus fatigue limit].  The importer
    // stores these two values at +0x488c/+0x4890, and the monitor tax return
    // consumes them through the enclosing CDataManager at +0x48f4/+0x48f8.
    // This keeps the restored gameplay receiver out of private PVF storage.
    int get_member_bonus_fatigue_charac_limit() const;
    int get_member_bonus_fatigue_account_limit() const;
    // B-owned typed projection for CUser::RecoverFatigue @0x8657ADA.  The
    // original reads this 16-bit value through CDataManager+0x51C after the
    // repeat-event 87 gate succeeds.
    uint16_t get_weekend_bonus_fatigue() const;
    // CParty::sumGainedEachExp @0x85A2488 reads the adjacent signed dword at
    // CDataManager+0x520. It is the [weekend bonus exp] member imported into
    // WeekendBonus::exp, not anonymous DataManager storage.
    int get_weekend_bonus_exp() const;
    // _MakeDropItems @0x830ADF6 selects CDataManager+0x524+(members-1).
    // CDataManager embeds this script at +0x68, proving the physical owner
    // is [drop routin by party] at ServerParameterScript+0x4bc.
    uint8_t getDropRoutineByParty(int member_count) const;
    // CItemList::create_item reads CDataManager+0x4A50 when it randomizes the
    // initial endurance of generated equipment. CDataManager embeds this
    // script at +0x68, so the owner field is +0x49E8 here.
    int get_damaged_equipment_ratio() const;
    float get_monster_kind_drop_bonus_rate(int kind) const;
    float get_dungeon_difficulty_exp_bonus_rate(int level_index) const;
    float get_quest_item_drop_bonus_starter_server(int member_count) const;
    stSharedServerMessageInfo* get_shared_server_message_info();
    const stSharedServerMessageInfo* get_shared_server_message_info() const;
    // Typed access to the actual stFatigueBatteryConstantTable_t constructed
    // at +0x5c0.  This is a source-side owner boundary, not a new legacy ABI.
    stFatigueBatteryConstantTable_t& get_fatigue_battery_constant_table();
    const stFatigueBatteryConstantTable_t&
    get_fatigue_battery_constant_table() const;
    const EmblemCompoundInfo* get_emblem_compound_info(int grade) const;
    // CUser::getCompoundUpgrade @0x8682082 indexes one of the four 32x32
    // recipe tables owned by this script (+0x0868/+0x1868/+0x2868/+0x3868).
    // This source-side owner boundary preserves the original unchecked row
    // lookup after CUser has enforced the 0..31 equipment-upgrade contract.
    const CompoundUpgradeRateRow& get_recipe_compound_upgrade_rates(
        bool use_second_table,
        bool amplified_material,
        unsigned int equipment_upgrade) const;
    // IDA_EA: 0x822ADFC
    int get_pvp_item_give(int pvp_grade) const;
    // IDA_EA: 0x826868C
    unsigned char setDungeonOpen(int dungeon_id, bool open);
    // IDA_EA: 0x82687FC
    int isDungeonOpen(int dungeon_id) const;
    // B-owned typed projection of the map constructed at +0xC8 and filled by
    // importServerParameterScript. This is deliberately an owner-side bridge,
    // not a new exported old-server ABI: 09's timer chain accesses the same
    // persistent PVF map without reopening CDataManager/ServerParameterScript
    // byte offsets.
    const DungeonInoutTimeMap& get_dungeon_inout_times() const;
    // IDA_EA: 0x82694FA
    std::vector<std::pair<int, int> >* GetPvPLiveEventReward();
    // IDA_EA: 0x8269508
    int GetPvPLiveEventPlayCount() const;
    // IDA_EA: 0x82F03BC
    int getAssaultPrice(int charac_level) const;
    // IDA_EA: 0x82F03F8
    const char* getAssaultUserRunawayNotice() const;
    // IDA_EA: 0x82F0410
    long double getChaosPointWeight(int chaos_point) const;
    // IDA_EA: 0x83703E2
    int CheckTraningFirstGive() const;
    // IDA_EA: 0x84A60E2
    int GetTraningSealGive(int pvp_grade) const;
    // IDA_EA: 0x84A61C4
    int GetTraningFirstGive(
        int charac_job,
        int first_grow_type,
        int pvp_grade,
        std::vector<std::pair<int, int> >& reward) const;
    // Typed B-domain bridges for the bytes parsed from
    // [private training seal gain count],
    // [relay battle training seal gain count], and
    // [training seal bonus count].
    int get_training_seal_gain_count(bool relay_battle,
                                     bool won,
                                     bool bonus) const;
    uint16_t get_training_seal_bonus_count() const;
    // IDA_EA: 0x8568AF6
    int CheckUnusableEquipmentInfo(int unusable_equipment_type,
                                   int usable_level);
    // IDA_EA: 0x8568BB0
    int GetDailyMatchCount() const;
    // IDA_EA: 0x8568BBE
    int CheckUnusableItem(int equipment_type, unsigned long item_id) const;
    // IDA_EA: 0x85BEAF6
    int GetPriceAverage(int charac_level) const;
    // IDA_EA: 0x85BEBB6
    int GetAdmissionIncrease(int enter_count) const;
    // IDA_EA: 0x85BEC76
    int GetLottePoint(int enter_count) const;
    // IDA_EA: 0x85BED36
    int GetLottoCost() const;
    // IDA_EA: 0x85BED44
    int GetJackpot() const;
    // IDA_EA: 0x86938A2
    int getRecoverStaminaPrice(int charac_level, int discount_percent) const;
    // IDA_EA: 0x8693918
    long double GetPvPPenaltyRevision(int pvp_grade) const;
    // IDA_EA: 0x8693994
    int GetPvPChannelGradePenalty(int pvp_grade) const;
    // IDA_EA: 0x86D17EC
    int GetCleanChattingCount(int charac_level);
    // IDA_EA: 0x86D18AC
    int GetMaxCleanChattingCount();
    // IDA_EA: 0x86D18BA
    int GetPaneltyDupMessage();
    // IDA_EA: 0x8A8C28C
    long double GetExpBonusWeight(unsigned int rank_bonus_index) const;
    // Typed accessors for the two scalar tables consumed by
    // CParty::clear_reward @0x85AE0D4.  The original reads them from the
    // embedded server-parameter script at CDataManager+0x68: member-count
    // factor at script +0x368..+0x374 and dungeon-difficulty factor at
    // script +0x398..+0x3a4.  Keep the owner-side projection here instead
    // of reintroducing those offsets in party reward code.
    float GetClearDungeonPartyExpWeight(int member_count) const;
    float GetClearDungeonDifficultyExpWeight(int difficulty) const;
    // IDA_EA: 0x8A8C2C0
    int CheckCollectDungeonStatisticServer(int server_group,
                                           int channel_no) const;
    // IDA_EA: 0x8A8C372
    int CheckLightServer(int server_group);
    // IDA_EA: 0x8A8C3EE
    int checkAssaultServer(int server_group, int channel_no) const;
    // IDA_EA: 0x8A8C486
    int getLuckPoint(int point, int& luck_point) const;
    // IDA_EA: 0x8A8C4FE
    int getLuckPointDeductionRate(int charac_level, int point) const;
    // Typed access to the [premium rarity dicision] rows parsed into the
    // original stPremiumGoldCardEvent at +0x4894.
    int getPremiumGoldCardRarityDecision(int rarity) const;
    // Source-side typed access to one [premium card drop] row.  The original
    // CParty::isPremiumGoldCardReward reads this embedded table directly from
    // CDataManager; keep the four fields at their recovered order rather than
    // exposing the packed script storage to party code.
    bool getPremiumGoldCardDropRule(int index,
                                    int& minimum_level,
                                    int& maximum_level,
                                    int& probability,
                                    unsigned int& item_id) const;
    // This is a source-side typed bridge, not an old exported method.
    // ServerParameterScript::clear @0x836F38C initializes the scalar at
    // +0x4968, and importServerParameterScript @0x8A870B7 fills it from
    // [max monster drop per map].  CBattle_Field::ConsistMap @0x83031D2
    // consumes the same embedded CDataManager field at +0x49d0.
    int get_max_monster_drop_per_map() const;
    // IDA_EA: 0x8A8C5F2
    std::vector<std::pair<int, int> >* GetEscaladeTutorialReward(
        unsigned int tutorial_id);
    // IDA_EA: 0x8A8C668
    long double getAssaultPriceChaospointWeight(int chaos_point) const;
    // IDA_EA: 0x8A8C804
    int getAssaultGhostTime(int charac_level, int chaos_point) const;

private:
    struct stLevelCond4PremiumGoldCard {
        int32_t minimum_level;
        int32_t maximum_level;
        int32_t probability;
        int32_t item_id;
    };

    struct stPremiumGoldCardEvent {
        float card_cost;
        int32_t drop_rule_count;
        stLevelCond4PremiumGoldCard drop_rules[10];
        int32_t rarity_decision[6];
    };

    // Physical order reconstructed from C2 @0x836FCA4 and D2 @0x837BB42.
    // Every placement-managed STL child is represented by its real type; no
    // byte storage or owner-relative offset remains in the production class.
    ServerIndexBoolMap collect_dungeon_statistic_servers_;       // +0x0000
    PairIntVector assault_servers_;                               // +0x0018
    IntBoolMap light_servers_;                                    // +0x0024
    LightServerNoticeVector light_server_notices_;                // +0x003c
    LuckPointVector luck_points_;                                 // +0x0048
    LuckPointDeductionVector rare_luck_point_deductions_;         // +0x0054
    LuckPointDeductionVector unique_luck_point_deductions_;       // +0x0060
    IntVector luck_point_deduction_gaps_;                         // +0x006c
    int32_t point_per_fatigue_;                                   // +0x0078
    int32_t event_point_per_fatigue_;                             // +0x007c
    int32_t point_per_play_time_;                                 // +0x0080
    int32_t play_time_for_point_;                                 // +0x0084
    IntPairVectorMap escalade_tutorial_rewards_;                  // +0x0088
    PairIntVector pvp_live_event_rewards_;                        // +0x00a0
    int32_t pvp_live_event_play_count_;                           // +0x00ac
    IntIntMap pvp_exp_penalty_revisions_;                         // +0x00b0
    DungeonInoutTimeMap dungeon_inout_times_;                     // +0x00c8
    IntVector closed_dungeons_;                                   // +0x00e0
    UnusableEquipmentVector unusable_equipment_info_;             // +0x00ec
    int32_t daily_match_count_;                                   // +0x00f8
    UIntSet unusable_items_;                                      // +0x00fc
    IntUIntSetMap usable_items_by_equipment_type_;                // +0x0114
    int32_t prevent_prefix_count_;                                // +0x012c
    char prevent_prefixes_[18][30];                               // +0x0130
    int32_t black_list_help_period_days_;                         // +0x034c
    int32_t black_list_help_points_per_period_;                   // +0x0350
    uint8_t version_digits_[4];                                   // +0x0354
    uint32_t script_version_;                                     // +0x0358
    uint32_t antibot_server_version_;                             // +0x035c
    float clear_exp_bonus_rate_;                                  // +0x0360
    float monster_exp_bonus_rate_;                                // +0x0364
    float party_user_number_exp_bonus_rates_[4];                  // +0x0368
    float starter_server_party_exp_bonus_rates_[4];               // +0x0378
    float starter_server_quest_item_drop_bonus_rates_[4];         // +0x0388
    float dungeon_difficulty_exp_bonus_rates_[5];                 // +0x0398
    float clear_rank_exp_bonus_rates_[5];                         // +0x03ac
    ItemGenRefVector dungeon_drop_probabilities_;                 // +0x03c0
    float dungeon_difficulty_drop_bonus_rates_[5];                // +0x03cc
    float party_user_number_drop_bonus_rates_[4];                 // +0x03e0
    float monster_kind_drop_bonus_rates_[4];                      // +0x03f0
    ItemGenRefVector result_reward_probabilities_;                // +0x0400
    float dungeon_difficulty_reward_bonus_rates_[5];              // +0x040c
    float party_user_number_reward_bonus_rates_[4];               // +0x0420
    int32_t item_decision_basis_;                                 // +0x0430
    int32_t equipment_rarity_decision_basis_[4];                  // +0x0434
    int32_t stackable_rarity_decision_basis_[4];                  // +0x0444
    stEventIdolBringUpParam_t bring_up_event_parameters_;         // +0x0454
    UpgradeItemTicketMap upgrade_item_tickets_;                   // +0x0468
    AuctionRegisterItemLimitVector auction_register_item_limits_; // +0x0480
    BurningFatigueParamVector burning_fatigue_parameters_;        // +0x048c
    ConditionEnterRevengeVector condition_enter_revenge_;         // +0x0498
    int32_t event_dungeon_clear_reward_item_id_;                  // +0x04a4
    FatigueEventParamVector fatigue_event_parameters_;            // +0x04a8
    WeekendBonus weekend_bonus_;                                  // +0x04b4
    uint8_t drop_routine_by_party_[4];                            // +0x04bc
    ItemFarmingDataMap item_farming_data_;                        // +0x04c0
    IntFloatPairVector drop_bonus_of_dungeon_level_increase_;     // +0x04d8
    AssaultChaosPointWeightVector assault_chaos_point_weights_;   // +0x04e4
    AssaultLevelGhostPanaltyVector assault_level_ghost_penalties_; // +0x04f0
    AssaultChaosPointGhostPanaltyVector
        assault_chaos_point_ghost_penalties_;                     // +0x04fc
    int32_t pre_lottery_policy_values_[8];                        // +0x0508
    LotteryUpgradePolicy lottery_upgrade_policy_;                 // +0x0528
    IntVector recover_stamina_prices_;                            // +0x057c
    IntVector assault_prices_;                                    // +0x0588
    std::string assault_user_runaway_notice_;                     // +0x0594
    int32_t assault_price_time_weight_limit_;                     // +0x0598
    int32_t assault_price_time_limit_seconds_;                    // +0x059c
    float assault_price_level_gap_weight_;                        // +0x05a0
    FloatVector assault_chaos_point_weight_table_;                // +0x05a4
    int32_t increase_abuse_point_;                                // +0x05b0
    int32_t decrease_abuse_point_;                                // +0x05b4
    int32_t abuse_point_penalty_;                                 // +0x05b8
    int32_t decrease_experience_point_;                           // +0x05bc
    stFatigueBatteryConstantTable_t fatigue_battery_constant_table_; // +0x05c0
    stSharedServerMessageInfo shared_server_message_info_;        // +0x0770
    stBloodDungeonRewardInfo blood_dungeon_reward_info_;          // +0x0794
    int32_t lotto_cost_;                                          // +0x07b8
    int32_t jackpot_;                                             // +0x07bc
    EmblemCompoundInfo emblem_compound_info_[7];                  // +0x07c0
    CompoundUpgradeRateRow recipe_upgrade_table_[32];             // +0x0868
    CompoundUpgradeRateRow recipe_amplify_table_[32];             // +0x1868
    CompoundUpgradeRateRow recipe_upgrade_table_second_[32];      // +0x2868
    CompoundUpgradeRateRow recipe_amplify_table_second_[32];      // +0x3868
    IntUIntMap full_level_rewards_;                               // +0x4868
    PairIntVector ontime_event_rewards_;                          // +0x4880
    int32_t member_bonus_fatigue_charac_limit_;                   // +0x488c
    int32_t member_bonus_fatigue_account_limit_;                  // +0x4890
    stPremiumGoldCardEvent premium_gold_card_event_;              // +0x4894
    PairIntVector clean_chatting_counts_;                         // +0x4954
    int32_t max_clean_chatting_count_;                            // +0x4960
    int32_t duplicate_message_penalty_;                           // +0x4964
    int32_t max_monster_drop_per_map_;                            // +0x4968
    int32_t max_monster_count_per_map_;                           // +0x496c
    uint8_t training_seal_gain_counts_[8];                        // +0x4970
    uint16_t training_seal_bonus_count_;                          // +0x4978
    uint16_t reserved_497a_;                                      // +0x497a
    IntIntMap pvp_channel_grade_penalties_;                       // +0x497c
    IntIntMap training_seal_give_;                               // +0x4994
    PvpFirstGiveMap training_first_give_;                         // +0x49ac
    IntIntMap training_first_give_count_by_grade_;                // +0x49c4
    IntVector pvp_item_give_grades_;                              // +0x49dc
    int32_t damaged_equipment_ratio_;                             // +0x49e8
    int32_t compound_avatar_upper_rate_bonus_;                    // +0x49ec
    int32_t compound_avatar_lower_rate_reduction_;                // +0x49f0
    int32_t compound_avatar_master_rate_bonus_;                   // +0x49f4 (0828)
    BroadcastItemIdVector broadcast_item_ids_;                     // +0x49f8 (0828)
    TimedItemGiveEntryVector timed_item_gives_;                     // +0x4a04 (0828)

    ServerParameterScript(const ServerParameterScript&);
    ServerParameterScript& operator=(const ServerParameterScript&);

    friend struct ServerParameterScriptLayoutAssert;
    friend int importServerParameterScript(ServerParameterScript*,
                                           const char*);
};

typedef ServerParameterScript::PvpFirstGiveMap PvpFirstGiveMap;

struct ServerParameterScriptLayoutAssert {
#if defined(__i386__)
    static_assert(offsetof(ServerParameterScript,
                           collect_dungeon_statistic_servers_) == 0x0000u &&
                      offsetof(ServerParameterScript, assault_servers_) ==
                          0x0018u &&
                      offsetof(ServerParameterScript, light_servers_) ==
                          0x0024u &&
                      offsetof(ServerParameterScript, light_server_notices_) ==
                          0x003cu &&
                      offsetof(ServerParameterScript, point_per_fatigue_) ==
                          0x0078u &&
                      offsetof(ServerParameterScript, point_per_play_time_) ==
                          0x0080u,
                  "ServerParameterScript server/luck block offset drift");
    static_assert(offsetof(ServerParameterScript,
                           escalade_tutorial_rewards_) == 0x0088u &&
                      offsetof(ServerParameterScript,
                               dungeon_inout_times_) == 0x00c8u &&
                      offsetof(ServerParameterScript,
                               unusable_equipment_info_) == 0x00ecu &&
                      offsetof(ServerParameterScript, unusable_items_) ==
                          0x00fcu &&
                      offsetof(ServerParameterScript,
                               usable_items_by_equipment_type_) == 0x0114u &&
                      offsetof(ServerParameterScript, prevent_prefixes_) ==
                          0x0130u,
                  "ServerParameterScript PvP/item policy block offset drift");
    static_assert(offsetof(ServerParameterScript,
                           black_list_help_period_days_) == 0x034cu &&
                      offsetof(ServerParameterScript, version_digits_) ==
                          0x0354u &&
                      offsetof(ServerParameterScript, script_version_) ==
                          0x0358u &&
                      offsetof(ServerParameterScript,
                               antibot_server_version_) == 0x035cu &&
                      offsetof(ServerParameterScript,
                               dungeon_difficulty_exp_bonus_rates_) ==
                          0x0398u &&
                      offsetof(ServerParameterScript,
                               clear_rank_exp_bonus_rates_) == 0x03acu,
                  "ServerParameterScript version/reward block offset drift");
    static_assert(offsetof(ServerParameterScript,
                           dungeon_drop_probabilities_) == 0x03c0u &&
                      offsetof(ServerParameterScript,
                               monster_kind_drop_bonus_rates_) == 0x03f0u &&
                      offsetof(ServerParameterScript,
                               result_reward_probabilities_) == 0x0400u &&
                      offsetof(ServerParameterScript,
                               bring_up_event_parameters_) == 0x0454u &&
                      offsetof(ServerParameterScript, upgrade_item_tickets_) ==
                          0x0468u &&
                      offsetof(ServerParameterScript,
                               auction_register_item_limits_) == 0x0480u,
                  "ServerParameterScript item-generation block offset drift");
    static_assert(offsetof(ServerParameterScript,
                           burning_fatigue_parameters_) == 0x048cu &&
                      offsetof(ServerParameterScript,
                               condition_enter_revenge_) == 0x0498u &&
                      offsetof(ServerParameterScript, weekend_bonus_) ==
                          0x04b4u &&
                      offsetof(ServerParameterScript,
                               drop_routine_by_party_) == 0x04bcu &&
                      offsetof(ServerParameterScript, item_farming_data_) ==
                          0x04c0u &&
                      offsetof(ServerParameterScript,
                               lottery_upgrade_policy_) == 0x0528u &&
                      offsetof(ServerParameterScript,
                               recover_stamina_prices_) == 0x057cu,
                  "ServerParameterScript event/legacy block offset drift");
    static_assert(offsetof(ServerParameterScript,
                           fatigue_battery_constant_table_) == 0x05c0u &&
                      offsetof(ServerParameterScript,
                               shared_server_message_info_) == 0x0770u &&
                      offsetof(ServerParameterScript,
                               blood_dungeon_reward_info_) == 0x0794u &&
                      offsetof(ServerParameterScript,
                               emblem_compound_info_) == 0x07c0u &&
                      offsetof(ServerParameterScript,
                               recipe_upgrade_table_) == 0x0868u &&
                      offsetof(ServerParameterScript,
                               recipe_amplify_table_second_) == 0x3868u,
                  "ServerParameterScript fatigue/recipe block offset drift");
    static_assert(offsetof(ServerParameterScript, full_level_rewards_) ==
                          0x4868u &&
                      offsetof(ServerParameterScript,
                               premium_gold_card_event_) == 0x4894u &&
                      offsetof(ServerParameterScript,
                               clean_chatting_counts_) == 0x4954u &&
                      offsetof(ServerParameterScript,
                               pvp_channel_grade_penalties_) == 0x497cu &&
                      offsetof(ServerParameterScript,
                               training_first_give_) == 0x49acu &&
                      offsetof(ServerParameterScript,
                               damaged_equipment_ratio_) == 0x49e8u &&
                      offsetof(ServerParameterScript,
                               compound_avatar_lower_rate_reduction_) ==
                          0x49f0u &&
                      offsetof(ServerParameterScript,
                               compound_avatar_master_rate_bonus_) ==
                          0x49f4u,
                  "ServerParameterScript late policy block offset drift");
#endif
};

int importServerParameterScript(ServerParameterScript* script,
                                const char* filename);

static_assert(sizeof(ServerParameterScript::PvPPlayerKey) == 0x03u,
              "PvPPlayerKey IDA ABI size must stay 0x03");
static_assert(sizeof(ServerParameterScript) == 0x4A10u,
              "ServerParameterScript 0828 extension size must stay 0x4A10");

#endif  // OLDSRV_DATAMANAGER_SERVER_PARAMETER_SCRIPT_H_
