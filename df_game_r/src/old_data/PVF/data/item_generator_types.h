#ifndef OLDSRV_OLD_DATA_PVF_DATA_ITEM_GENERATOR_TYPES_H_
#define OLDSRV_OLD_DATA_PVF_DATA_ITEM_GENERATOR_TYPES_H_

#include <stddef.h>
#include <stdint.h>

#include <list>
#include <utility>
#include <vector>

#include "old_data/PVF/equipment/upgrade_table.h"
#include "old_game/dungeon/instance/luck_point_runtime.h"
#include "old_game/economy/inventory/inventory.h"

class CMTRand;
class CDungeon;

class TimerUpdateItemGenerateStatistics {
public:
    // IDA_EA: 0x8637318
    // IDA_NAME: _ZN33TimerUpdateItemGenerateStatistics15registNextTimerEl
    static int registNextTimer(long timer);
};

namespace WongWork {

// IDA C2 @0x814A706 proves the first 0x34 bytes of every item-generator
// request.  It deliberately stops there: the generator-specific union is
// caller-filled and must remain uninitialized by the C2.  The stack frame of
// CBossStage::killMonsterDropItem @0x814D628 places consecutive requests
// 0x48 bytes apart, and CBattle_Field::_MakeDropItems @0x830ADF6 writes as
// far as request +0x45.  The complete union therefore occupies +0x34..+0x47
// (0x14 bytes), not the former guessed 0x10-byte tail.

// World-drop only consumes the first two bytes of the shared union.  Keeping
// it separate from clear_reward avoids pretending the values are character
// and dungeon inputs on this path.
struct stGenerateRefWorldDrop_t {
    uint8_t world_area;                         // +0x00
    uint8_t world_index;                        // +0x01
};

// IDA 0x8536DA8 / 0x853818A: random monster and object drops use this
// interpretation.  The final four fields are kept as individual, proven
// storage positions (rather than a byte buffer): _MakeDropItems writes the
// first two and leaves the latter pair untouched before the random call.
struct stGenerateRefMonsterRandomDrop_t {
    uint8_t character_level;                    // +0x00
    int8_t monster_role;                        // +0x01
    uint8_t dungeon_standard_level;             // +0x02
    uint8_t dungeon_difficulty;                 // +0x03
    uint8_t party_member_count;                 // +0x04
    int8_t monster_drop_count;                  // +0x05
    uint16_t reserved_06_07;                    // +0x06
    int32_t bourgeois_rate;                     // +0x08
    uint8_t is_hell_party;                      // +0x0c
    uint8_t item_drop_dungeon_type;             // +0x0d
    uint8_t reserved_0e;                        // +0x0e
    uint8_t reserved_0f;                        // +0x0f
    uint8_t reserved_10;                        // +0x10
    uint8_t reserved_11;                        // +0x11
    uint16_t reserved_12_13;                    // +0x12
};

// IDA 0x830B148..0x830B21B fills this interpretation immediately before
// the CMonsterDrop slot +0x0c call.  It deliberately overlays the random
// fields above; in particular +0x38 changes from random-drop context to the
// map monster id.  CMonsterDrop::generateSpecificItem @0x8537070 consumes
// all named fields through +0x11.
struct stGenerateRefMonsterSpecificDrop_t {
    uint8_t character_level;                    // +0x00
    uint8_t dungeon_standard_level;             // +0x01
    uint8_t dungeon_difficulty;                 // +0x02
    uint8_t retained_dungeon_difficulty;        // +0x03
    uint32_t monster_id;                        // +0x04
    // CMonsterDrop::generateSpecificItem @0x8537070 uses a signed <= 4
    // branch here; values above four select the AI/HELL path.
    int8_t monster_kind;                        // +0x08
    // CMonsterDrop_Hell::hellPartySpecificItem @0x8535B08 sign-extends
    // this count before its per-item virtual loop.
    int8_t monster_drop_count;                  // +0x09
    uint8_t avatar_drop_context;                // +0x0a
    uint8_t party_member_max_level;             // +0x0b
    // HELL-party helper @0x8535B08 tests +0x0c before the vptr +0x0c loop.
    uint8_t hell_party_repeat_specific_items;   // +0x0c
    // Normal CMonsterDrop slot @0x8537070 tests +0x0d to enter the
    // HELL-party aggregate path.  It is a separate flag from +0x0c.
    uint8_t hell_party_specific_path;           // +0x0d
    // CMonsterDrop_Hell::generateSpecificItem @0x8535726 sign-extends this
    // byte to select one six-int rarity-decision row at CMonsterDrop_Hell+0x30.
    int8_t hell_rarity_decision_index;          // +0x0e
    uint8_t suppress_normal_specific_drop_a;    // +0x0f
    uint8_t suppress_normal_specific_drop_b;    // +0x10
    // HELL-party flow uses this as the catch-monster drop enable byte.
    uint8_t catch_monster_drop_enabled;         // +0x11
    uint16_t reserved_12_13;                    // +0x12
};

// CObjectDrop::generateRandomItem @0x853818A uses a different four-byte
// view of the same union.  It is not a CMonster random-drop request.
struct stGenerateRefObjectRandomDrop_t {
    uint8_t character_level;                    // +0x00
    uint8_t dungeon_difficulty;                 // +0x01
    uint8_t party_member_count;                 // +0x02
    int8_t item_count;                          // +0x03
};

// Generator type 4 consumes only this pointer at union +0x00.
struct stGenerateRefDungeonDrop_t {
    std::vector<std::pair<int, int> >* items;   // +0x00
};

// Generator type 1 consumes the list, the standard level and the count.  No
// artificial tail is modeled here: the physical union extent comes from the
// fully recovered monster views above.
struct stGenerateRefObjectDrop_t {
    std::list<std::pair<int, int> >* items;     // +0x00
    uint8_t standard_level;                     // +0x04
    int8_t item_count;                          // +0x05
};

// Clear-reward and clear-gold use distinct 0x10-byte prefixes of the same
// 0x14-byte union.  Their unconsumed final four bytes are owned by the
// active generator interpretation and are never initialized by the request
// constructor.
struct stGenerateRefClearReward_t {
    uint8_t character_level_or_world_area;       // +0x00
    uint8_t dungeon_difficulty_or_world_index;   // +0x01
    uint8_t party_member_count;                  // +0x02
    uint8_t reserved_03;                         // +0x03
    float item_probability_scale;                // +0x04
    int clear_reward_drop_probability;           // +0x08
    ::CDungeon* dungeon;                         // +0x0c
};

// The same final 16-byte union has a second interpretation in the clear-gold
// virtual path.  IDA 0x8538C84 proves the named byte/dword/word inputs below;
// it must not be read through the dungeon-item view above.
struct stGenerateRefGoldReward_t {
    uint8_t character_level;                     // +0x00
    uint8_t dungeon_difficulty;                  // +0x01
    uint8_t party_member_count;                  // +0x02
    uint8_t reserved_03;                         // +0x03
    int exp_bonus_context;                       // +0x04
    uint16_t gold_weight_08;                     // +0x08
    uint16_t gold_weight_0a;                     // +0x0a
    uint16_t gold_reserved_0c;                   // +0x0c
    uint16_t gold_weight_0e;                     // +0x0e
};

// CBossStage::killMonsterDropItem @0x814D628 supplies the independent-drop
// generator's first 16 union bytes through this interpretation.  The three
// named bytes are written at full-reference offsets +0x34, +0x36 and +0x3e.
// The original C2 deliberately leaves the rest of this union unwritten.
struct stGenerateRefBossMonsterDrop_t {
    uint8_t monster_kind;                        // +0x00
    uint8_t reserved_01;
    uint8_t member_count_class;                  // +0x02
    uint8_t reserved_03_09[7];
    uint8_t party_member_count;                  // +0x0a
    uint8_t reserved_0b_0f[5];
};

struct stGenerateRefData_t {
    stGenerateRefData_t();
    ~stGenerateRefData_t();

    int standard_drop_rate;                      // +0x00
    // CClearReward::generateRandomItem @0x853873A reads +0x04.
    float clear_reward_scale;                    // +0x04
    std::vector<bool> flags;                     // +0x08
    // CMonsterDrop::generateRandomItem @0x8536DA8 reads +0x1c.
    float monster_random_drop_scale;             // +0x1c
    int32_t premium_independent_drop_rate;       // +0x20
    uint8_t party_member_force_drop;             // +0x24
    uint8_t blue_marble_context;                 // +0x25
    uint16_t reserved_26;                        // +0x26
    uint32_t blue_marble_character_job;          // +0x28
    uint32_t blue_marble_character_level;        // +0x2c
    uint32_t blue_marble_buff_value;             // +0x30
    union {
        stGenerateRefWorldDrop_t world_drop;     // +0x34
        stGenerateRefMonsterRandomDrop_t monster_random_drop;
        stGenerateRefMonsterSpecificDrop_t monster_specific_drop;
        stGenerateRefObjectRandomDrop_t object_random_drop;
        stGenerateRefClearReward_t clear_reward; // +0x34
        stGenerateRefGoldReward_t gold_reward;
        stGenerateRefBossMonsterDrop_t boss_monster_drop;
        stGenerateRefObjectDrop_t object_drop;
        stGenerateRefDungeonDrop_t dungeon_drop;
        // Compatibility spelling retained for existing caller translation
        // units.  It is exactly dungeon_drop.items, not a separate facade.
        std::vector<std::pair<int, int> >* dungeon_specific_items;
    } generator_data;
};

struct stGenerateResult_t {
    stGenerateResult_t();
    ~stGenerateResult_t();

    std::vector<Inven_Item> items;               // +0x00
    CLuckPoint luck_point;                       // +0x0c
};

// IDA CGeneratorCommonData @0x8374EBC.  The RDAR loader indexes this
// directly by character level and stores an inclusive gold range per row.
struct GoldGenerateRange {
    int minimum;
    int maximum;
};

class CGeneratorCommonData {
public:
    // IDA_EA: 0x8374EBC / 0x8374EC2
    CGeneratorCommonData();
    ~CGeneratorCommonData();

    // IDA_EA: 0x8534456
    int loadScript(const char* filename);

    // IDA_EA: 0x8533424.  The returned pair is a function-local snapshot in
    // the old server because AutoMarket may adjust the minimum amount.
    GoldGenerateRange* getGoldGenTable(unsigned char character_level);

private:
    GoldGenerateRange gold_generate_range_[201];
};

class CGenerateRandomNumber {
public:
    // IDA_EA: 0x85334EA / 0x85335D0
    explicit CGenerateRandomNumber(int stream_count);
    ~CGenerateRandomNumber();

    // IDA_EA: 0x85334A4
    unsigned int generateNumber(int stream_index, int limit);

private:
    CMTRand* streams_;
    int stream_count_;
};

class GenerateRandomUpgrade {
public:
    // IDA_EA: 0x85335EE / 0x853364E
    GenerateRandomUpgrade();
    ~GenerateRandomUpgrade();

    // IDA_EA: 0x8533676 / 0x8533932.
    int generateUpgrade(ENUM_RARITY rarity, int dungeon_level);
    int generateEquipGrade(int dungeon_level);

private:
    CMTRand* random_;
};

class CItemGenerator {
public:
    // IDA_EA: 0x8533B3E / 0x8533B9A
    explicit CItemGenerator(int random_stream_count);
    ~CItemGenerator();

    // The original vtable has exactly this order.  In particular, callers of
    // CStage::consistMap use generateSpecificItem at slot +0x0c.
    // IDA_EA: 0x853A3F0
    virtual void setDropRatio(float ratio);
    // IDA_EA: 0x853A3F6
    virtual int loadScript(const char* filename);
    // IDA_EA: 0x853A400
    virtual void generateRandomItem(const stGenerateRefData_t& reference,
                                    stGenerateResult_t& result);
    // IDA_EA: 0x853A406
    virtual void generateSpecificItem(const stGenerateRefData_t& reference,
                                      stGenerateResult_t& result);

    // IDA_EA: 0x853A3E2 / 0x853A40C / 0x853A41A / 0x85BEFF4
    CItemGenerator* setCommonData(CGeneratorCommonData* common_data);
    CItemGenerator* setGeneratorType(int generator_type);
    int getGeneratorType();
    CGeneratorCommonData* getGeneratorCommonData();

protected:
    CGeneratorCommonData* common_data_;   // +0x04
    CGenerateRandomNumber random_number_; // +0x08
    GenerateRandomUpgrade random_upgrade_; // +0x10
    int generator_type_;                  // +0x14
};

#if __SIZEOF_POINTER__ == 4
static_assert(sizeof(stGenerateRefClearReward_t) == 0x10u,
              "stGenerateRefClearReward_t ABI");
static_assert(sizeof(stGenerateRefGoldReward_t) == 0x10u,
              "stGenerateRefGoldReward_t ABI");
static_assert(sizeof(stGenerateRefBossMonsterDrop_t) == 0x10u,
              "stGenerateRefBossMonsterDrop_t ABI");
static_assert(sizeof(stGenerateRefMonsterRandomDrop_t) == 0x14u,
              "stGenerateRefMonsterRandomDrop_t ABI");
static_assert(sizeof(stGenerateRefMonsterSpecificDrop_t) == 0x14u,
              "stGenerateRefMonsterSpecificDrop_t ABI");
static_assert(sizeof(stGenerateRefObjectRandomDrop_t) == 0x04u,
              "stGenerateRefObjectRandomDrop_t ABI");
static_assert(offsetof(stGenerateRefMonsterRandomDrop_t, bourgeois_rate) ==
                  0x08u,
              "monster random bourgeois-rate offset");
static_assert(offsetof(stGenerateRefMonsterRandomDrop_t, monster_role) ==
                  0x01u,
              "monster random role offset");
static_assert(offsetof(stGenerateRefMonsterRandomDrop_t, monster_drop_count) ==
                  0x05u,
              "monster random drop-count offset");
static_assert(offsetof(stGenerateRefMonsterRandomDrop_t, is_hell_party) ==
                  0x0cu,
              "monster random hell-party offset");
static_assert(offsetof(stGenerateRefMonsterSpecificDrop_t, monster_id) ==
                  0x04u,
              "monster specific id offset");
static_assert(offsetof(stGenerateRefMonsterSpecificDrop_t, monster_kind) ==
                  0x08u,
              "monster specific kind offset");
static_assert(offsetof(stGenerateRefMonsterSpecificDrop_t,
                       catch_monster_drop_enabled) ==
                  0x11u,
              "monster specific catch-drop enable offset");
static_assert(offsetof(stGenerateRefMonsterSpecificDrop_t, monster_drop_count) ==
                  0x09u,
              "monster specific HELL drop-count offset");
static_assert(offsetof(stGenerateRefMonsterSpecificDrop_t,
                       hell_rarity_decision_index) == 0x0eu,
              "monster specific HELL rarity-row offset");
static_assert(offsetof(stGenerateRefMonsterSpecificDrop_t,
                       hell_party_repeat_specific_items) == 0x0cu,
              "monster specific HELL repeat flag offset");
static_assert(offsetof(stGenerateRefMonsterSpecificDrop_t,
                       hell_party_specific_path) == 0x0du,
              "monster specific HELL aggregate-path flag offset");
static_assert(offsetof(stGenerateRefObjectDrop_t, standard_level) == 0x04u,
              "object-drop standard-level offset");
static_assert(offsetof(stGenerateRefObjectDrop_t, item_count) == 0x05u,
              "object-drop item-count offset");
static_assert(offsetof(stGenerateRefObjectRandomDrop_t, item_count) == 0x03u,
              "object-random item-count offset");
static_assert(offsetof(stGenerateRefClearReward_t, item_probability_scale) ==
                  0x04u &&
                  offsetof(stGenerateRefClearReward_t,
                           clear_reward_drop_probability) == 0x08u &&
                  offsetof(stGenerateRefClearReward_t, dungeon) == 0x0cu,
              "clear-reward reference offsets");
static_assert(offsetof(stGenerateRefBossMonsterDrop_t, member_count_class) ==
                  0x02u,
              "boss monster member-count class offset");
static_assert(offsetof(stGenerateRefBossMonsterDrop_t, party_member_count) ==
                  0x0au,
              "boss monster party-member count offset");
static_assert(offsetof(stGenerateRefData_t, generator_data) == 0x34u,
              "stGenerateRefData_t generator union offset");
static_assert(offsetof(stGenerateRefData_t, clear_reward_scale) == 0x04u,
              "stGenerateRefData_t clear-reward scale offset");
static_assert(offsetof(stGenerateRefData_t, monster_random_drop_scale) ==
                  0x1cu,
              "stGenerateRefData_t monster-random scale offset");
static_assert(offsetof(stGenerateRefData_t,
                       premium_independent_drop_rate) == 0x20u,
              "stGenerateRefData_t premium independent-drop rate offset");
static_assert(offsetof(stGenerateRefData_t, party_member_force_drop) ==
                  0x24u &&
                  offsetof(stGenerateRefData_t, blue_marble_context) ==
                  0x25u,
              "stGenerateRefData_t independent-drop flags offset");
static_assert(sizeof(stGenerateRefData_t) == 0x48u,
              "stGenerateRefData_t ABI");
static_assert(sizeof(stGenerateResult_t) == 0x30u,
              "stGenerateResult_t ABI");
static_assert(sizeof(GoldGenerateRange) == 0x8u,
              "CGeneratorCommonData row ABI");
static_assert(sizeof(CGeneratorCommonData) == 0x648u,
              "CGeneratorCommonData ABI");
static_assert(sizeof(CGenerateRandomNumber) == 0x8u,
              "CGenerateRandomNumber ABI");
static_assert(sizeof(GenerateRandomUpgrade) == 0x4u,
              "GenerateRandomUpgrade ABI");
static_assert(sizeof(CItemGenerator) == 0x18u,
              "CItemGenerator ABI");
#endif

}  // namespace WongWork

#endif  // OLDSRV_OLD_DATA_PVF_DATA_ITEM_GENERATOR_TYPES_H_
