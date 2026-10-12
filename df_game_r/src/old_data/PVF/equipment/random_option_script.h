#ifndef OLDSRV_OLD_DATA_PVF_EQUIPMENT_RANDOM_OPTION_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_EQUIPMENT_RANDOM_OPTION_SCRIPT_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <set>
#include <string>
#include <vector>

#include "common/manager/random_application_key_abi.h"
#include "old_data/PVF/equipment/upgrade_table.h"

class CMTRand;
class CUser;
struct Inven_Item;

// IDA unit: 99_misc_global/BaseItemKey.
struct BaseItemKey {
    BaseItemKey(uint8_t rarity_value,
                uint8_t usable_level_value,
                uint16_t item_group_name_value);

    uint8_t rarity;
    uint8_t usable_level;
    uint16_t item_group_name;
};

static_assert(sizeof(BaseItemKey) == 0x04u,
              "BaseItemKey IDA ABI size must stay 0x04");

struct RandomApplicationValue {
    int32_t percent;
    int32_t remain;
};

static_assert(sizeof(RandomApplicationValue) == 0x08u,
              "RandomApplicationValue IDA ABI size must stay 0x08");

struct LevelLimitDecisionValue {
    int32_t current;
    int32_t next;
};

static_assert(sizeof(LevelLimitDecisionValue) == 0x08u,
              "LevelLimitDecisionValue IDA ABI size must stay 0x08");

// IDA unit: 99_misc_global/PartSelectonField.
struct PartSelectonField {
    ~PartSelectonField();

    int32_t item_group_name;
    int32_t accumulated_weight;
};

static_assert(sizeof(PartSelectonField) == 0x08u,
              "PartSelectonField IDA ABI size must stay 0x08");

// IDA unit: 99_misc_global/PartSelectonValue.
struct PartSelectonValue {
    PartSelectonValue();
    PartSelectonValue(const PartSelectonValue& rhs);
    ~PartSelectonValue();

    int clear();

    std::vector<PartSelectonField> weighted_fields;
    int32_t total_weight;
    std::vector<PartSelectonField> raw_fields;
};

static_assert(sizeof(PartSelectonValue) == 0x1cu,
              "PartSelectonValue IDA ABI size must stay 0x1c");
static_assert(offsetof(PartSelectonValue, total_weight) == 0x0cu,
              "PartSelectonValue total_weight offset must stay 0x0c");
static_assert(offsetof(PartSelectonValue, raw_fields) == 0x10u,
              "PartSelectonValue raw_fields offset must stay 0x10");

// IDA map value used by CItemList and CDataManager::initRandomOptionItem.
struct BaseItemValue {
    int32_t grade;
    unsigned long item_index;
};

static_assert(sizeof(BaseItemValue) == 0x08u,
              "BaseItemValue IDA ABI size must stay 0x08");
static_assert(sizeof(unsigned long) == 0x04u,
              "old itemIndex_t ABI expects 32-bit unsigned long");

// IDA unit: 99_misc_global/BaseItemSelectionValue.
struct BaseItemSelectionValue {
    BaseItemSelectionValue();
    BaseItemSelectionValue(const BaseItemSelectionValue& rhs);
    ~BaseItemSelectionValue();

    BaseItemSelectionValue* clear();

    std::vector<unsigned int> weights;
    uint32_t total_weight;
};

static_assert(sizeof(BaseItemSelectionValue) == 0x10u,
              "BaseItemSelectionValue IDA ABI size must stay 0x10");
static_assert(offsetof(BaseItemSelectionValue, total_weight) == 0x0cu,
              "BaseItemSelectionValue total_weight offset must stay 0x0c");

// IDA unit: 99_misc_global/RandomOptionGive.
struct RandomOptionGive {
    RandomOptionGive();
    RandomOptionGive(const RandomOptionGive& rhs);
    ~RandomOptionGive();

    RandomOptionGive* clear();

    std::vector<unsigned int> weights;
    uint32_t total_weight;
};

static_assert(sizeof(RandomOptionGive) == 0x10u,
              "RandomOptionGive IDA ABI size must stay 0x10");
static_assert(offsetof(RandomOptionGive, total_weight) == 0x0cu,
              "RandomOptionGive total_weight offset must stay 0x0c");

// IDA unit: 99_misc_global/QuantityRatio.
struct QuantityRatio {
    ~QuantityRatio();

    int32_t quantity;
    int32_t ratio;
};

static_assert(sizeof(QuantityRatio) == 0x08u,
              "QuantityRatio IDA ABI size must stay 0x08");

struct OptionQuantityValue {
    int16_t begin_index;
    int16_t end_index;
};

static_assert(sizeof(OptionQuantityValue) == 0x04u,
              "OptionQuantityValue IDA ABI size must stay 0x04");
static_assert(offsetof(OptionQuantityValue, begin_index) == 0x00u,
              "OptionQuantityValue begin_index offset must stay 0x00");
static_assert(offsetof(OptionQuantityValue, end_index) == 0x02u,
              "OptionQuantityValue end_index offset must stay 0x02");

// IDA unit: 99_misc_global/OptionGroupChoiceValue.
struct OptionGroupChoiceValue {
    OptionGroupChoiceValue();
    OptionGroupChoiceValue(const OptionGroupChoiceValue& rhs);
    ~OptionGroupChoiceValue();

    OptionGroupChoiceValue* clear();

    std::vector<int> group_indexes;
    std::vector<unsigned int> weights;
    uint32_t total_weight;
};

static_assert(sizeof(OptionGroupChoiceValue) == 0x1cu,
              "OptionGroupChoiceValue IDA ABI size must stay 0x1c");
static_assert(offsetof(OptionGroupChoiceValue, group_indexes) == 0x00u,
              "OptionGroupChoiceValue group_indexes offset must stay 0x00");
static_assert(offsetof(OptionGroupChoiceValue, weights) == 0x0cu,
              "OptionGroupChoiceValue weights offset must stay 0x0c");
static_assert(offsetof(OptionGroupChoiceValue, total_weight) == 0x18u,
              "OptionGroupChoiceValue total_weight offset must stay 0x18");

// IDA unit: 99_misc_global/OptionValueRatio.
struct OptionValueRatio {
    OptionValueRatio();
    ~OptionValueRatio();

    float option_value;
    float ratio_value;
    int32_t ratio;
};

static_assert(sizeof(OptionValueRatio) == 0x0cu,
              "OptionValueRatio IDA ABI size must stay 0x0c");

// IDA unit: 99_misc_global/OPTION_VALUE_RATIO.
struct OPTION_VALUE_RATIO {
    OPTION_VALUE_RATIO();
    ~OPTION_VALUE_RATIO();

    OPTION_VALUE_RATIO* clear();

    std::vector<OptionValueRatio> ratios;
    uint32_t total_ratio;
};

static_assert(sizeof(OPTION_VALUE_RATIO) == 0x10u,
              "OPTION_VALUE_RATIO IDA ABI size must stay 0x10");
static_assert(offsetof(OPTION_VALUE_RATIO, total_ratio) == 0x0cu,
              "OPTION_VALUE_RATIO total_ratio offset must stay 0x0c");

// IDA unit: 99_misc_global/DifferentWeightValue.
struct DifferentWeightValue {
    DifferentWeightValue();

    int32_t begin_value;
    int32_t end_value;
};

static_assert(sizeof(DifferentWeightValue) == 0x08u,
              "DifferentWeightValue IDA ABI size must stay 0x08");

// IDA unit: 99_misc_global/OPTION_MODIFICATION_COST.
struct OPTION_MODIFICATION_COST {
    ~OPTION_MODIFICATION_COST();

    int16_t usable_level;
    uint8_t reserved_02[2];
    int32_t rarity_rare_cost;
    int32_t rarity_unique_cost;
};

static_assert(sizeof(OPTION_MODIFICATION_COST) == 0x0cu,
              "OPTION_MODIFICATION_COST IDA ABI size must stay 0x0c");
static_assert(offsetof(OPTION_MODIFICATION_COST, rarity_rare_cost) == 0x04u,
              "OPTION_MODIFICATION_COST rare cost offset must stay 0x04");
static_assert(offsetof(OPTION_MODIFICATION_COST, rarity_unique_cost) == 0x08u,
              "OPTION_MODIFICATION_COST unique cost offset must stay 0x08");

// IDA unit: 99_misc_global/OptionGroup.
struct OptionGroup {
    ~OptionGroup();

    uint32_t option_group;
    uint32_t weight;
};

static_assert(sizeof(OptionGroup) == 0x08u,
              "OptionGroup IDA ABI size must stay 0x08");

// IDA unit: 99_misc_global/OptionGroupValue.
struct OptionGroupValue {
    OptionGroupValue();
    OptionGroupValue(const OptionGroupValue& rhs);
    ~OptionGroupValue();

    OptionGroupValue* clear();

    std::vector<OptionGroup> groups;
    uint32_t total_weight;
};

static_assert(sizeof(OptionGroupValue) == 0x10u,
              "OptionGroupValue IDA ABI size must stay 0x10");
static_assert(offsetof(OptionGroupValue, total_weight) == 0x0cu,
              "OptionGroupValue total_weight offset must stay 0x0c");

// IDA unit: 99_misc_global/ChoosePrefix.
struct ChoosePrefix {
    ChoosePrefix(int begin_value, int end_value, int grade_value);
    ~ChoosePrefix();

    int32_t begin;
    int32_t end;
    int32_t grade;
};

static_assert(sizeof(ChoosePrefix) == 0x0cu,
              "ChoosePrefix IDA ABI size must stay 0x0c");

// IDA unit: 99_misc_global/ResetValue.
struct ResetValue {
    ~ResetValue();

    int32_t begin;
    int32_t end;
    uint32_t weight;
};

static_assert(sizeof(ResetValue) == 0x0cu,
              "ResetValue IDA ABI size must stay 0x0c");
static_assert(offsetof(ResetValue, weight) == 0x08u,
              "ResetValue weight must retain IDA +0x08");

// IDA unit: 99_misc_global/ResetValues.
struct ResetValues {
    ResetValues();
    ResetValues(const ResetValues& rhs);
    ~ResetValues();

    ResetValues* clear();

    std::vector<ResetValue> values;
    uint32_t total_weight;
};

static_assert(sizeof(ResetValues) == 0x10u,
              "ResetValues IDA ABI size must stay 0x10");
static_assert(offsetof(ResetValues, total_weight) == 0x0cu,
              "ResetValues total_weight offset must stay 0x0c");

class CRandomOptionGive {
public:
    CRandomOptionGive();

    virtual std::map<RandomApplicationKey, std::map<int, RandomOptionGive> >*
    get_option_type();
    virtual std::map<unsigned long long, OptionGroupChoiceValue>*
    get_option_group_choice();
};

static_assert(sizeof(CRandomOptionGive) == 0x04u,
              "CRandomOptionGive IDA ABI size must stay 0x04");

class CRandomOptionGiveNormal : public CRandomOptionGive {
public:
    CRandomOptionGiveNormal();

    virtual std::map<RandomApplicationKey, std::map<int, RandomOptionGive> >*
    get_option_type();
    virtual std::map<unsigned long long, OptionGroupChoiceValue>*
    get_option_group_choice();

    CRandomOptionGiveNormal* set(
        std::map<RandomApplicationKey, std::map<int, RandomOptionGive> >*
            option_type,
        std::map<unsigned long long, OptionGroupChoiceValue>*
            option_group_choice);

    std::map<RandomApplicationKey, std::map<int, RandomOptionGive> >*
        option_type;
    std::map<unsigned long long, OptionGroupChoiceValue>*
        option_group_choice;
};

static_assert(sizeof(CRandomOptionGiveNormal) == 0x0cu,
              "CRandomOptionGiveNormal IDA ABI size must stay 0x0c");
static_assert(offsetof(CRandomOptionGiveNormal, option_type) == 0x04u,
              "CRandomOptionGiveNormal option_type offset must stay 0x04");
static_assert(offsetof(CRandomOptionGiveNormal, option_group_choice) == 0x08u,
              "CRandomOptionGiveNormal option_group_choice offset must stay 0x08");

class CModifiedRandomOptionGive : public CRandomOptionGive {
public:
    CModifiedRandomOptionGive();

    virtual std::map<RandomApplicationKey, std::map<int, RandomOptionGive> >*
    get_option_type();
    virtual std::map<unsigned long long, OptionGroupChoiceValue>*
    get_option_group_choice();

    CModifiedRandomOptionGive* set(
        std::map<RandomApplicationKey, std::map<int, RandomOptionGive> >*
            option_type,
        std::map<unsigned long long, OptionGroupChoiceValue>*
            option_group_choice);

    std::map<RandomApplicationKey, std::map<int, RandomOptionGive> >*
        option_type;
    std::map<unsigned long long, OptionGroupChoiceValue>*
        option_group_choice;
};

static_assert(sizeof(CModifiedRandomOptionGive) == 0x0cu,
              "CModifiedRandomOptionGive IDA ABI size must stay 0x0c");
static_assert(offsetof(CModifiedRandomOptionGive, option_type) == 0x04u,
              "CModifiedRandomOptionGive option_type offset must stay 0x04");
static_assert(offsetof(CModifiedRandomOptionGive, option_group_choice) == 0x08u,
              "CModifiedRandomOptionGive option_group_choice offset must stay 0x08");

// IDA unit: 12_pvf_datamanager_scripts/RandomOptionScript.
struct RandomOptionScript {
    RandomOptionScript();
    ~RandomOptionScript();

    int clear();
    int getOptionModificationCost(short usable_level, ENUM_RARITY rarity) const;
    std::string getPrefix(int option_group, int percent, bool modified);
    int getPrefixGrade(int percent);

    std::map<RandomApplicationKey, RandomApplicationValue>
        random_application;
    std::map<RandomApplicationKey, LevelLimitDecisionValue>
        level_limit;
    std::map<int, PartSelectonValue> part_selection;
    std::map<RandomApplicationKey, BaseItemSelectionValue>
        base_item_selection;
    std::map<int, std::map<int, BaseItemValue> > base_items;
    std::map<unsigned long, int> item_ref;
    std::map<RandomApplicationKey, std::map<int, RandomOptionGive> >
        option_type;
    std::vector<QuantityRatio> quantity_ratio;
    std::map<int, OptionQuantityValue> option_quantity;
    std::map<unsigned long long, OptionGroupChoiceValue>
        option_group_choice;
    OPTION_VALUE_RATIO option_value_ratio;
    std::map<short, DifferentWeightValue> different_weight;
    std::vector<OPTION_MODIFICATION_COST> option_modification_cost;
    std::map<RandomApplicationKey, std::map<int, RandomOptionGive> >
        modified_option_type;
    std::map<unsigned long long, OptionGroupChoiceValue>
        modified_option_group_choice;
    CRandomOptionGiveNormal normal_give;
    CModifiedRandomOptionGive modified_give;
    std::map<int, OptionGroupValue> option_group;
    std::map<int, int> break_seal_cost;
    std::map<int, int> raw_190;
    std::map<int, std::vector<std::string> > prefixes;
    std::map<int, std::vector<std::string> > modified_prefixes;
    std::vector<ChoosePrefix> choose_prefix;
    std::string changed_prefix;
    std::map<unsigned char, ResetValues> reset_values;
    std::set<unsigned long> unable_modify_postfix_grade;
    std::set<unsigned long> gradeless_postfix;
};

static_assert(sizeof(RandomOptionScript) == 0x230u,
              "RandomOptionScript IDA ABI size must stay 0x230");
static_assert(offsetof(RandomOptionScript, base_items) == 0x060u,
              "RandomOptionScript base_items offset must stay 0x60");
static_assert(offsetof(RandomOptionScript, item_ref) == 0x078u,
              "RandomOptionScript item_ref offset must stay 0x78");
static_assert(offsetof(RandomOptionScript, option_value_ratio) == 0x0e4u,
              "RandomOptionScript option_value_ratio offset must stay 0xe4");
static_assert(offsetof(RandomOptionScript, normal_give) == 0x148u,
              "RandomOptionScript normal_give offset must stay 0x148");
static_assert(offsetof(RandomOptionScript, modified_give) == 0x154u,
              "RandomOptionScript modified_give offset must stay 0x154");
static_assert(offsetof(RandomOptionScript, changed_prefix) == 0x1e4u,
              "RandomOptionScript changed_prefix offset must stay 0x1e4");
static_assert(offsetof(RandomOptionScript, gradeless_postfix) == 0x218u,
              "RandomOptionScript gradeless_postfix offset must stay 0x218");

struct _str_cost {
    _str_cost(int first_value, int second_value);

    int32_t first;
    int32_t second;
};

static_assert(sizeof(_str_cost) == 0x08u,
              "_str_cost IDA ABI size must stay 0x08");

struct _str_option_value_ratio {
    _str_option_value_ratio(float first_value,
                            float second_value,
                            int ratio_value);
    ~_str_option_value_ratio();

    float first;
    float second;
    int32_t ratio;
};

static_assert(sizeof(_str_option_value_ratio) == 0x0cu,
              "_str_option_value_ratio IDA ABI size must stay 0x0c");

struct str_index_to_itemgroup {
    str_index_to_itemgroup();
    str_index_to_itemgroup(const str_index_to_itemgroup& rhs);
    ~str_index_to_itemgroup();

    int32_t item_group;
    std::string name;
    std::vector<std::string> item_groups;
};

static_assert(sizeof(str_index_to_itemgroup) == 0x14u,
              "str_index_to_itemgroup IDA ABI size must stay 0x14");
static_assert(offsetof(str_index_to_itemgroup, name) == 0x04u,
              "str_index_to_itemgroup name offset must stay 0x04");
static_assert(offsetof(str_index_to_itemgroup, item_groups) == 0x08u,
              "str_index_to_itemgroup item_groups offset must stay 0x08");

struct RandomOptionItem_Regenerator {
    RandomOptionItem_Regenerator();
    ~RandomOptionItem_Regenerator();

    std::map<int, str_index_to_itemgroup> item_groups;
    std::map<int, std::map<int, _str_cost> > costs;
    std::vector<int> quantity_ratio;
    std::vector<_str_option_value_ratio> value_ratio;
    uint32_t total_value_ratio;
};

static_assert(sizeof(RandomOptionItem_Regenerator) == 0x4cu,
              "RandomOptionItem_Regenerator IDA ABI size must stay 0x4c");
static_assert(offsetof(RandomOptionItem_Regenerator, costs) == 0x18u,
              "RandomOptionItem_Regenerator costs offset must stay 0x18");
static_assert(offsetof(RandomOptionItem_Regenerator, quantity_ratio) == 0x30u,
              "RandomOptionItem_Regenerator quantity_ratio offset must stay 0x30");
static_assert(offsetof(RandomOptionItem_Regenerator, value_ratio) == 0x3cu,
              "RandomOptionItem_Regenerator value_ratio offset must stay 0x3c");
static_assert(offsetof(RandomOptionItem_Regenerator, total_value_ratio) == 0x48u,
              "RandomOptionItem_Regenerator total_value_ratio offset must stay 0x48");

// IDA unit: 99_misc_global/RegenerationROI.
struct RegenerationROI {
    RegenerationROI();
    ~RegenerationROI();

    // IDA_EA: 0x85F853C
    void SendMessageToCUser(CUser* user, const char* message);
    // IDA_EA: 0x85F8646
    int Regenerate(int first_usable_level,
                   int second_usable_level,
                   int select_part,
                   int regeneration_mode,
                   int& cost,
                   Inven_Item& generated_item,
                   CUser* user);
    // IDA_EA: 0x85F8CB4 / 0x85F8DA6
    int RegenVer_get_option_quantity(int begin_index, int end_index);
    unsigned int RegenVer_get_option_value(int begin_value, int end_value);
    // IDA_EA: 0x85F8EC2 / 0x85F9094 / 0x85F93CC
    void _Select_Part(int select_part, std::vector<int>& item_groups);
    int _Calculate_RegenCost(int select_part,
                             int regeneration_mode,
                             int first_usable_level,
                             int second_usable_level);
    void _RandomLevel(int first_usable_level,
                      int second_usable_level,
                      int& selected_level);

    // IDA_EA: 0x85F963E
    int _0_Load_RegenerationRandomOptionItem();

    CMTRand* random;
    RandomOptionItem_Regenerator regenerator;
};

static_assert(sizeof(RegenerationROI) == 0x50u,
              "RegenerationROI IDA ABI size must stay 0x50");
static_assert(offsetof(RegenerationROI, regenerator) == 0x04u,
              "RegenerationROI regenerator offset must stay 0x04");

// IDA_EA: 0x8A70A90
int importRandomOptionScript(RandomOptionScript& script,
                             const char* filename)
    __asm__("_Z24importRandomOptionScriptR18RandomOptionScriptPKc");

// IDA_EA: 0x8A7E610
int importRegenerationRandomOptionScript(
    RandomOptionItem_Regenerator& regenerator,
    const char* filename)
    __asm__("_Z36importRegenerationRandomOptionScriptR28RandomOptionItem_RegeneratorPKc");



#endif  // OLDSRV_OLD_DATA_PVF_EQUIPMENT_RANDOM_OPTION_SCRIPT_H_
