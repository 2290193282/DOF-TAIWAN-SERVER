#ifndef OLDSRV_OLD_DATA_PVF_STACKABLE_STACKABLE_SCRIPT_TYPES_H_
#define OLDSRV_OLD_DATA_PVF_STACKABLE_STACKABLE_SCRIPT_TYPES_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "old_game/economy/equipment/enchant_system_data.h"
#include "common/manager/striker_public_abi.h"

// 0828 client-owned payload of `[disguise appendage]`.  The block is a
// repeated (ability-name, unit-name, signed value) tuple; keeping the unit is
// important because percentage and absolute modifiers are not equivalent.
struct StackableDisguiseAppendageModifier {
    std::string ability_name;
    std::string unit_name;
    int32_t value;
};

// `[emancipate grow apply]` stores a named group, a declared member count and
// then exactly that many grow-type ids.  The 0828 client also accepts count
// -1/0 as a wildcard/empty sentinel, so the declaration is retained instead
// of flattening the block into an untyped integer vector.
struct StackableEmancipateGrowGroup {
    int32_t group;
    int32_t declared_count;
    std::vector<int> grow_types;
};

// Taiwan's anniversary firework block is a sequence of five-field rows.
// These paths are presentation assets, but the item/reward selector and its
// probability are gameplay data and therefore remain in the server model.
struct StackableRandomImageEntry {
    int32_t image_index;
    int32_t probability;
    int32_t reward_item_id;
    std::string body_animation;
    std::string effect_animation;
};

// IDA unit: 99_misc_global/CountinuousItemOption.
struct CountinuousItemOption {
    CountinuousItemOption();
    CountinuousItemOption* clear();

    uint8_t option_type;
    uint8_t option_value;
    uint8_t enabled;
};

// IDA unit: 99_misc_global/STStatChange.
struct STStatChange {
    STStatChange();
    ~STStatChange();
    STStatChange* clear();

    int32_t stat_type;
    int32_t value;
    uint8_t enabled;
    uint8_t reserved_09[3];
    int32_t duration;
};

// IDA unit: 99_misc_global/STStatChangeVector.
struct STStatChangeVector {
    STStatChangeVector();
    ~STStatChangeVector();
    STStatChangeVector& operator=(const STStatChangeVector& other);
    STStatChangeVector* clear();

    std::vector<STStatChange> values;
    int32_t value_0c;
    int32_t value_10;
    uint8_t enabled;
    uint8_t reserved_15[3];
};

// IDA unit: 99_misc_global/STStatusGenerate.
struct STStatusGenerate {
    STStatusGenerate();
    ~STStatusGenerate();
    STStatusGenerate& operator=(const STStatusGenerate& other);
    STStatusGenerate* clear();

    std::vector<float> values;
    int32_t value_0c;
    int32_t value_10;
    uint8_t enabled;
    uint8_t reserved_15[3];
};

// IDA unit: 99_misc_global/CRabbitBox.
class CRabbitBox {
public:
    CRabbitBox();
    CRabbitBox(const CRabbitBox& other);
    virtual ~CRabbitBox();

    CRabbitBox& operator=(const CRabbitBox& other);
    int getItemIndex() const;
    int getItemCount() const;
    CRabbitBox* setItemIndex(int item_index);
    CRabbitBox* SetItemCount(int item_count);
    CRabbitBox* SetDropRate(int drop_rate);

private:
    int32_t option_type_;
    int32_t item_index_;
    int32_t item_count_;
    int32_t drop_rate_;
};

// IDA unit: 99_misc_global/stLimitCubeResult.
struct stLimitCubeResult {
    stLimitCubeResult();
    ~stLimitCubeResult();

    int32_t item_index;
    int32_t item_count;
    int32_t probability;
};

// IDA unit: 99_misc_global/stLimitCubeInfo.
struct stLimitCubeInfo {
    stLimitCubeInfo();
    ~stLimitCubeInfo();
    stLimitCubeInfo& operator=(const stLimitCubeInfo& other);

    std::vector<std::pair<int, int> > condition_items;
    int32_t value_0c;
    int32_t value_10;
    std::vector<stLimitCubeResult> results;
};

// IDA unit: 99_misc_global/BoosterSelectInfo.
struct BoosterSelectInfo {
    BoosterSelectInfo();
    ~BoosterSelectInfo();
    BoosterSelectInfo& operator=(const BoosterSelectInfo& other);

    int32_t value_00;
    int32_t value_04;
    int32_t value_08;
    std::vector<std::string> filenames;
    uint8_t enabled;
    uint8_t reserved_19[3];
};

// IDA unit: 06_userinfo_stat_inventory/AvatarOptionChangeType.
struct AvatarOptionChangeType {
    AvatarOptionChangeType();
    AvatarOptionChangeType* clear();

    int8_t option_0;
    int8_t option_1;
    int8_t option_2;
};

// IDA unit: 05_world_enter_village/STEnchantSystemMapKey.
struct STEnchantSystemMapKey : STEnchantSystemData {
    STEnchantSystemMapKey();
    ~STEnchantSystemMapKey();
    bool operator<(const STEnchantSystemMapKey& other) const;

    uint8_t job;
    uint8_t reserved_0d[3];
};

// IDA unit: 05_world_enter_village/STEnchantSystemMapData.
struct STEnchantSystemMapData {
    STEnchantSystemMapData();
    STEnchantSystemMapData(const STEnchantSystemMapData& other);
    ~STEnchantSystemMapData();

    std::vector<EquipmentSkillDataUp> skill_data;
    std::string skill_explain;
};

// IDA unit: 99_misc_global/stStackableBoosterItemInfo_t.
struct stStackableBoosterItemInfo_t {
    ~stStackableBoosterItemInfo_t();

    int32_t item_index;
    int32_t accumulated_rate;
    uint16_t item_count;
    uint8_t equipment_upgrade;
    uint8_t default_selected;
    uint8_t recommended;
    uint8_t avatar_option_0d;
    uint8_t avatar_option_0e;
    uint8_t reserved_0f;
    // CLIENT_EA: ImportStackableScript_ParseBoosterSelectRandomProbabilityPairs
    // @ 0x0156B500. The first value is the resulting reinforcement level and
    // the second value is its selection weight.
    std::vector<std::pair<int, int> > reinforcement_level_weights;
    // CLIENT_EA:
    // ImportStackableScript_ParseBoosterSelectForceInsertAvatarOptionPairs
    // @ 0x0156B5AC and wrapper construction @ 0x0060AB60. Each pair is
    // {reward avatar item id, forced avatar ability number}; the sender uses
    // the forced value instead of the UI choice for the matching wrapper.
    std::vector<std::pair<int, int> > forced_avatar_options;
};

// IDA unit: 99_misc_global/stStackableBoosterElement_t.
struct stStackableBoosterElement_t {
    stStackableBoosterElement_t();
    stStackableBoosterElement_t(const stStackableBoosterElement_t& other);
    ~stStackableBoosterElement_t();
    stStackableBoosterElement_t& operator=(
        const stStackableBoosterElement_t& other);

    int32_t category;
    int32_t item_count;
    int32_t total_rate;
    std::vector<stStackableBoosterItemInfo_t> items;
};

// IDA unit: 99_misc_global/stStackableBooster_t.
struct stStackableBooster_t {
    stStackableBooster_t();
    ~stStackableBooster_t();
    stStackableBooster_t& operator=(const stStackableBooster_t& other);
    int tag2Type(const char* tag);
    int tag2EndType(const char* tag);
    stStackableBooster_t* clear();

    std::vector<stStackableBoosterElement_t> elements;
    std::map<std::pair<char, char>, std::vector<stStackableBoosterItemInfo_t> >
        category_items;
};

static_assert(sizeof(CountinuousItemOption) == 0x03u,
              "CountinuousItemOption IDA ABI size must stay 0x03");
static_assert(sizeof(STStatChange) == 0x10u,
              "STStatChange IDA ABI size must stay 0x10");
static_assert(sizeof(STStatChangeVector) == 0x18u,
              "STStatChangeVector IDA ABI size must stay 0x18");
static_assert(sizeof(STStatusGenerate) == 0x18u,
              "STStatusGenerate IDA ABI size must stay 0x18");
static_assert(sizeof(CRabbitBox) == 0x14u,
              "CRabbitBox IDA ABI size must stay 0x14");
static_assert(sizeof(stLimitCubeResult) == 0x0cu,
              "stLimitCubeResult IDA ABI size must stay 0x0c");
static_assert(sizeof(stLimitCubeInfo) == 0x20u,
              "stLimitCubeInfo IDA ABI size must stay 0x20");
static_assert(sizeof(BoosterSelectInfo) == 0x1cu,
              "BoosterSelectInfo IDA ABI size must stay 0x1c");
static_assert(sizeof(AvatarOptionChangeType) == 0x03u,
              "AvatarOptionChangeType IDA ABI size must stay 0x03");
static_assert(sizeof(STEnchantSystemMapKey) == 0x10u,
              "STEnchantSystemMapKey IDA ABI size must stay 0x10");
static_assert(sizeof(STEnchantSystemMapData) == 0x10u,
              "STEnchantSystemMapData IDA ABI size must stay 0x10");
static_assert(
    offsetof(stStackableBoosterItemInfo_t, reinforcement_level_weights) ==
        0x10u &&
        offsetof(stStackableBoosterItemInfo_t, forced_avatar_options) ==
            0x1cu &&
        sizeof(stStackableBoosterItemInfo_t) == 0x28u,
    "stStackableBoosterItemInfo_t must preserve its 0x10-byte old prefix");
static_assert(sizeof(stStackableBoosterElement_t) == 0x18u,
              "stStackableBoosterElement_t IDA ABI size must stay 0x18");
static_assert(sizeof(stStackableBooster_t) == 0x24u,
              "stStackableBooster_t IDA ABI size must stay 0x24");

#endif  // OLDSRV_OLD_DATA_PVF_STACKABLE_STACKABLE_SCRIPT_TYPES_H_
