#include "old_data/PVF/equipment/random_option_script.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_list.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/equipment/random_option_item_handle.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/mt_rand.h"

#include <cstring>
#include <new>
#include <stdio.h>
#include <sstream>

// IDA_EA: 0x8513F82
// IDA_NAME: _ZN11BaseItemKeyC2Ehht
// DEMANGLED: BaseItemKey::BaseItemKey(unsigned char,unsigned char,unsigned short)
BaseItemKey::BaseItemKey(uint8_t rarity_value,
                         uint8_t usable_level_value,
                         uint16_t item_group_name_value)
    : rarity(rarity_value),
      usable_level(usable_level_value),
      item_group_name(item_group_name_value) {}

// IDA_EA: 0x8A787F8
// IDA_NAME: _ZN17PartSelectonFieldD2Ev
// DEMANGLED: PartSelectonField::~PartSelectonField()
PartSelectonField::~PartSelectonField() {}

// IDA_EA: 0x8A73B8E
// IDA_NAME: _ZN17PartSelectonValueC2Ev
// DEMANGLED: PartSelectonValue::PartSelectonValue(void)
PartSelectonValue::PartSelectonValue()
    : weighted_fields(),
      total_weight(0),
      raw_fields() {
    clear();
}

// IDA_EA: 0x8A741FC
// IDA_NAME: _ZN17PartSelectonValueC2ERKS_
// DEMANGLED: PartSelectonValue::PartSelectonValue(PartSelectonValue const&)
PartSelectonValue::PartSelectonValue(const PartSelectonValue& rhs)
    : weighted_fields(rhs.weighted_fields),
      total_weight(rhs.total_weight),
      raw_fields(rhs.raw_fields) {}

// IDA_EA: 0x83CED2C
// IDA_NAME: _ZN17PartSelectonValueD2Ev
// DEMANGLED: PartSelectonValue::~PartSelectonValue()
PartSelectonValue::~PartSelectonValue() {}

// IDA_EA: 0x8A73BF6
// IDA_NAME: _ZN17PartSelectonValue5clearEv
// DEMANGLED: PartSelectonValue::clear(void)
int PartSelectonValue::clear() {
    weighted_fields.clear();
    total_weight = 0;
    raw_fields.clear();
    return 0;
}

// IDA_EA: 0x8A73C22
// IDA_NAME: _ZN22BaseItemSelectionValueC2Ev
// DEMANGLED: BaseItemSelectionValue::BaseItemSelectionValue(void)
BaseItemSelectionValue::BaseItemSelectionValue()
    : weights(),
      total_weight(0) {
    clear();
}

// IDA_EA: 0x8A74308
// IDA_NAME: _ZN22BaseItemSelectionValueC2ERKS_
// DEMANGLED: BaseItemSelectionValue::BaseItemSelectionValue(BaseItemSelectionValue const&)
BaseItemSelectionValue::BaseItemSelectionValue(
    const BaseItemSelectionValue& rhs)
    : weights(rhs.weights),
      total_weight(rhs.total_weight) {}

// IDA_EA: 0x83CEE22
// IDA_NAME: _ZN22BaseItemSelectionValueD2Ev
// DEMANGLED: BaseItemSelectionValue::~BaseItemSelectionValue()
BaseItemSelectionValue::~BaseItemSelectionValue() {}

// IDA_EA: 0x8A73C64
// IDA_NAME: _ZN22BaseItemSelectionValue5clearEv
// DEMANGLED: BaseItemSelectionValue::clear(void)
BaseItemSelectionValue* BaseItemSelectionValue::clear() {
    weights.clear();
    total_weight = 0;
    return this;
}

// IDA_EA: 0x8A73C82
// IDA_NAME: _ZN16RandomOptionGiveC2Ev
// DEMANGLED: RandomOptionGive::RandomOptionGive(void)
RandomOptionGive::RandomOptionGive()
    : weights(),
      total_weight(0) {
    clear();
}

// IDA_EA: 0x8A74402
// IDA_NAME: _ZN16RandomOptionGiveC2ERKS_
// DEMANGLED: RandomOptionGive::RandomOptionGive(RandomOptionGive const&)
RandomOptionGive::RandomOptionGive(const RandomOptionGive& rhs)
    : weights(rhs.weights),
      total_weight(rhs.total_weight) {}

// IDA_EA: 0x83F01A2
// IDA_NAME: _ZN16RandomOptionGiveD2Ev
// DEMANGLED: RandomOptionGive::~RandomOptionGive()
RandomOptionGive::~RandomOptionGive() {}

// IDA_EA: 0x8A73CC4
// IDA_NAME: _ZN16RandomOptionGive5clearEv
// DEMANGLED: RandomOptionGive::clear(void)
RandomOptionGive* RandomOptionGive::clear() {
    weights.clear();
    total_weight = 0;
    return this;
}

// IDA_EA: 0x8A79264
// IDA_NAME: _ZN13QuantityRatioD2Ev
// DEMANGLED: QuantityRatio::~QuantityRatio()
QuantityRatio::~QuantityRatio() {}

// IDA_EA: 0x8A73CE2
// IDA_NAME: _ZN22OptionGroupChoiceValueC2Ev
// DEMANGLED: OptionGroupChoiceValue::OptionGroupChoiceValue(void)
OptionGroupChoiceValue::OptionGroupChoiceValue()
    : group_indexes(),
      weights(),
      total_weight(0) {
    clear();
}

// IDA_EA: 0x8A746A6
// IDA_NAME: _ZN22OptionGroupChoiceValueC2ERKS_
// DEMANGLED: OptionGroupChoiceValue::OptionGroupChoiceValue(OptionGroupChoiceValue const&)
OptionGroupChoiceValue::OptionGroupChoiceValue(
    const OptionGroupChoiceValue& rhs)
    : group_indexes(rhs.group_indexes),
      weights(rhs.weights),
      total_weight(rhs.total_weight) {}

// IDA_EA: 0x83CF12A
// IDA_NAME: _ZN22OptionGroupChoiceValueD2Ev
// DEMANGLED: OptionGroupChoiceValue::~OptionGroupChoiceValue()
OptionGroupChoiceValue::~OptionGroupChoiceValue() {}

// IDA_EA: 0x8A73D4A
// IDA_NAME: _ZN22OptionGroupChoiceValue5clearEv
// DEMANGLED: OptionGroupChoiceValue::clear(void)
OptionGroupChoiceValue* OptionGroupChoiceValue::clear() {
    group_indexes.clear();
    weights.clear();
    total_weight = 0;
    return this;
}

// IDA_EA: 0x85F4176
// IDA_NAME: _ZN16OptionValueRatioC2Ev
// DEMANGLED: OptionValueRatio::OptionValueRatio(void)
OptionValueRatio::OptionValueRatio()
    : option_value(0.0f),
      ratio_value(0.0f),
      ratio(0) {}

// IDA_EA: 0x8A799F8
// IDA_NAME: _ZN16OptionValueRatioD2Ev
// DEMANGLED: OptionValueRatio::~OptionValueRatio()
OptionValueRatio::~OptionValueRatio() {}

// IDA_EA: 0x837288C
// IDA_NAME: _ZN18OPTION_VALUE_RATIOC2Ev
// DEMANGLED: OPTION_VALUE_RATIO::OPTION_VALUE_RATIO(void)
OPTION_VALUE_RATIO::OPTION_VALUE_RATIO()
    : ratios(),
      total_ratio(0) {}

// IDA_EA: 0x83728A0
// IDA_NAME: _ZN18OPTION_VALUE_RATIOD2Ev
// DEMANGLED: OPTION_VALUE_RATIO::~OPTION_VALUE_RATIO()
OPTION_VALUE_RATIO::~OPTION_VALUE_RATIO() {}

// IDA_EA: 0x8372532
// IDA_NAME: _ZN18OPTION_VALUE_RATIO5clearEv
// DEMANGLED: OPTION_VALUE_RATIO::clear(void)
OPTION_VALUE_RATIO* OPTION_VALUE_RATIO::clear() {
    ratios.clear();
    total_ratio = 0;
    return this;
}

// IDA_EA: 0x85F419A
// IDA_NAME: _ZN20DifferentWeightValueC2Ev
// DEMANGLED: DifferentWeightValue::DifferentWeightValue(void)
DifferentWeightValue::DifferentWeightValue()
    : begin_value(0),
      end_value(0) {}

// IDA_EA: 0x8A797AC
// IDA_NAME: _ZN24OPTION_MODIFICATION_COSTD2Ev
// DEMANGLED: OPTION_MODIFICATION_COST::~OPTION_MODIFICATION_COST()
OPTION_MODIFICATION_COST::~OPTION_MODIFICATION_COST() {}

// IDA_EA: 0x85F5988
// IDA_NAME: _ZN11OptionGroupD2Ev
// DEMANGLED: OptionGroup::~OptionGroup()
OptionGroup::~OptionGroup() {}

// IDA_EA: 0x8A73D76
// IDA_NAME: _ZN16OptionGroupValueC2Ev
// DEMANGLED: OptionGroupValue::OptionGroupValue(void)
OptionGroupValue::OptionGroupValue()
    : groups(),
      total_weight(0) {
    clear();
}

// IDA_EA: 0x8A74912
// IDA_NAME: _ZN16OptionGroupValueC2ERKS_
// DEMANGLED: OptionGroupValue::OptionGroupValue(OptionGroupValue const&)
OptionGroupValue::OptionGroupValue(const OptionGroupValue& rhs)
    : groups(rhs.groups),
      total_weight(rhs.total_weight) {}

// IDA_EA: 0x83CF30E
// IDA_NAME: _ZN16OptionGroupValueD2Ev
// DEMANGLED: OptionGroupValue::~OptionGroupValue()
OptionGroupValue::~OptionGroupValue() {}

// IDA_EA: 0x8A73DB8
// IDA_NAME: _ZN16OptionGroupValue5clearEv
// DEMANGLED: OptionGroupValue::clear(void)
OptionGroupValue* OptionGroupValue::clear() {
    groups.clear();
    total_weight = 0;
    return this;
}

// IDA_EA: 0x8A73DD6
// IDA_NAME: _ZN12ChoosePrefixC2Eiii
// DEMANGLED: ChoosePrefix::ChoosePrefix(int,int,int)
ChoosePrefix::ChoosePrefix(int begin_value, int end_value, int grade_value)
    : begin(begin_value),
      end(end_value),
      grade(grade_value) {}

// IDA_EA: 0x8A7BF4A
// IDA_NAME: _ZN12ChoosePrefixD2Ev
// DEMANGLED: ChoosePrefix::~ChoosePrefix()
ChoosePrefix::~ChoosePrefix() {}

// IDA_EA: 0x8A7A958
// IDA_NAME: _ZN10ResetValueD2Ev
// DEMANGLED: ResetValue::~ResetValue()
ResetValue::~ResetValue() {}

// IDA_EA: 0x8A73DF6
// IDA_NAME: _ZN11ResetValuesC2Ev
// DEMANGLED: ResetValues::ResetValues(void)
ResetValues::ResetValues()
    : values(),
      total_weight(0) {
    clear();
}

// IDA_EA: 0x8A74CDA
// IDA_NAME: _ZN11ResetValuesC2ERKS_
// DEMANGLED: ResetValues::ResetValues(ResetValues const&)
ResetValues::ResetValues(const ResetValues& rhs)
    : values(rhs.values),
      total_weight(rhs.total_weight) {}

// IDA_EA: 0x83CF4B4
// IDA_NAME: _ZN11ResetValuesD2Ev
// DEMANGLED: ResetValues::~ResetValues()
ResetValues::~ResetValues() {}

// IDA_EA: 0x8A73E38
// IDA_NAME: _ZN11ResetValues5clearEv
// DEMANGLED: ResetValues::clear(void)
ResetValues* ResetValues::clear() {
    values.clear();
    total_weight = 0;
    return this;
}

// IDA_EA: 0x8372568
// IDA_NAME: _ZN17CRandomOptionGiveC2Ev
// DEMANGLED: CRandomOptionGive::CRandomOptionGive(void)
CRandomOptionGive::CRandomOptionGive() {}

std::map<RandomApplicationKey, std::map<int, RandomOptionGive> >*
CRandomOptionGive::get_option_type() {
    return 0;
}

std::map<unsigned long long, OptionGroupChoiceValue>*
CRandomOptionGive::get_option_group_choice() {
    return 0;
}

// IDA_EA: 0x8372576
// IDA_NAME: _ZN23CRandomOptionGiveNormalC2Ev
// DEMANGLED: CRandomOptionGiveNormal::CRandomOptionGiveNormal(void)
CRandomOptionGiveNormal::CRandomOptionGiveNormal()
    : CRandomOptionGive(),
      option_type(0),
      option_group_choice(0) {}

// IDA_EA: 0x8372550
// IDA_NAME: _ZN23CRandomOptionGiveNormal15get_option_typeEv
// DEMANGLED: CRandomOptionGiveNormal::get_option_type(void)
std::map<RandomApplicationKey, std::map<int, RandomOptionGive> >*
CRandomOptionGiveNormal::get_option_type() {
    return option_type;
}

// IDA_EA: 0x837255C
// IDA_NAME: _ZN23CRandomOptionGiveNormal23get_option_group_choiceEv
// DEMANGLED: CRandomOptionGiveNormal::get_option_group_choice(void)
std::map<unsigned long long, OptionGroupChoiceValue>*
CRandomOptionGiveNormal::get_option_group_choice() {
    return option_group_choice;
}

// IDA_EA: 0x83725A6
// IDA_NAME: _ZN23CRandomOptionGiveNormal3setEPSt3mapI20RandomApplicationKeyS0_Ii16RandomOptionGiveSt4lessIiESaISt4pairIKiS2_EEES3_IS1_ESaIS5_IKS1_S9_EEEPS0_Iy22OptionGroupChoiceValueS3_IyESaIS5_IKySG_EEE
// DEMANGLED: CRandomOptionGiveNormal::set(...)
CRandomOptionGiveNormal* CRandomOptionGiveNormal::set(
    std::map<RandomApplicationKey, std::map<int, RandomOptionGive> >*
        option_type_value,
    std::map<unsigned long long, OptionGroupChoiceValue>*
        option_group_choice_value) {
    option_type = option_type_value;
    option_group_choice = option_group_choice_value;
    return this;
}

// IDA_EA: 0x83725D6
// IDA_NAME: _ZN25CModifiedRandomOptionGiveC2Ev
// DEMANGLED: CModifiedRandomOptionGive::CModifiedRandomOptionGive(void)
CModifiedRandomOptionGive::CModifiedRandomOptionGive()
    : CRandomOptionGive(),
      option_type(0),
      option_group_choice(0) {}

// IDA_EA: 0x83725BE
// IDA_NAME: _ZN25CModifiedRandomOptionGive15get_option_typeEv
// DEMANGLED: CModifiedRandomOptionGive::get_option_type(void)
std::map<RandomApplicationKey, std::map<int, RandomOptionGive> >*
CModifiedRandomOptionGive::get_option_type() {
    return option_type;
}

// IDA_EA: 0x83725CA
// IDA_NAME: _ZN25CModifiedRandomOptionGive23get_option_group_choiceEv
// DEMANGLED: CModifiedRandomOptionGive::get_option_group_choice(void)
std::map<unsigned long long, OptionGroupChoiceValue>*
CModifiedRandomOptionGive::get_option_group_choice() {
    return option_group_choice;
}

// IDA_EA: 0x8372606
// IDA_NAME: _ZN25CModifiedRandomOptionGive3setEPSt3mapI20RandomApplicationKeyS0_Ii16RandomOptionGiveSt4lessIiESaISt4pairIKiS2_EEES3_IS1_ESaIS5_IKS1_S9_EEEPS0_Iy22OptionGroupChoiceValueS3_IyESaIS5_IKySG_EEE
// DEMANGLED: CModifiedRandomOptionGive::set(...)
CModifiedRandomOptionGive* CModifiedRandomOptionGive::set(
    std::map<RandomApplicationKey, std::map<int, RandomOptionGive> >*
        option_type_value,
    std::map<unsigned long long, OptionGroupChoiceValue>*
        option_group_choice_value) {
    option_type = option_type_value;
    option_group_choice = option_group_choice_value;
    return this;
}

// IDA_EA: 0x8372904
// IDA_NAME: _ZN18RandomOptionScriptC2Ev
// DEMANGLED: RandomOptionScript::RandomOptionScript(void)
RandomOptionScript::RandomOptionScript()
    : random_application(),
      level_limit(),
      part_selection(),
      base_item_selection(),
      base_items(),
      item_ref(),
      option_type(),
      quantity_ratio(),
      option_quantity(),
      option_group_choice(),
      option_value_ratio(),
      different_weight(),
      option_modification_cost(),
      modified_option_type(),
      modified_option_group_choice(),
      normal_give(),
      modified_give(),
      option_group(),
      break_seal_cost(),
      raw_190(),
      prefixes(),
      modified_prefixes(),
      choose_prefix(),
      changed_prefix(),
      reset_values(),
      unable_modify_postfix_grade(),
      gradeless_postfix() {
    clear();
}

// IDA_EA: 0x837C9E8
// IDA_NAME: _ZN18RandomOptionScriptD2Ev
// DEMANGLED: RandomOptionScript::~RandomOptionScript()
RandomOptionScript::~RandomOptionScript() {}

// IDA_EA: 0x837261E
// IDA_NAME: _ZN18RandomOptionScript5clearEv
// DEMANGLED: RandomOptionScript::clear(void)
int RandomOptionScript::clear() {
    random_application.clear();
    level_limit.clear();
    part_selection.clear();
    base_item_selection.clear();
    base_items.clear();
    item_ref.clear();
    option_type.clear();
    quantity_ratio.clear();
    option_quantity.clear();
    option_group_choice.clear();
    option_value_ratio.clear();
    different_weight.clear();
    option_modification_cost.clear();
    modified_option_type.clear();
    modified_option_group_choice.clear();
    normal_give.set(&option_type, &option_group_choice);
    modified_give.set(&modified_option_type, &modified_option_group_choice);
    option_group.clear();
    break_seal_cost.clear();
    raw_190.clear();
    prefixes.clear();
    modified_prefixes.clear();
    choose_prefix.clear();
    reset_values.clear();
    unable_modify_postfix_grade.clear();
    gradeless_postfix.clear();
    return 0;
}

// IDA_EA: 0x8A738A0
// IDA_NAME: _ZNK18RandomOptionScript25getOptionModificationCostEs11ENUM_RARITY
// DEMANGLED: RandomOptionScript::getOptionModificationCost(short,ENUM_RARITY)const
int RandomOptionScript::getOptionModificationCost(short usable_level,
                                                  ENUM_RARITY rarity) const {
    for (std::vector<OPTION_MODIFICATION_COST>::const_iterator it =
             option_modification_cost.begin();
         it != option_modification_cost.end(); ++it) {
        if (it->usable_level >= usable_level) {
            if (rarity == 2) {
                return it->rarity_rare_cost;
            }
            if (rarity == 3) {
                return it->rarity_unique_cost;
            }
        }
    }
    return -1;
}

// IDA_EA: 0x8A73956
// IDA_NAME: _ZN18RandomOptionScript9getPrefixEiib
// DEMANGLED: RandomOptionScript::getPrefix(int,int,bool)
std::string RandomOptionScript::getPrefix(int option_group_value,
                                          int percent,
                                          bool modified) {
    std::string result;
    std::map<int, std::vector<std::string> >& table =
        modified ? modified_prefixes : prefixes;
    int prefix_grade = getPrefixGrade(percent);
    if (modified) {
        prefix_grade += 3;
    }
    std::map<int, std::vector<std::string> >::iterator it =
        table.find(option_group_value);
    if (it != table.end()) {
        for (unsigned int index = 0; index < it->second.size(); ++index) {
            if (prefix_grade == static_cast<int>(index)) {
                result = it->second[index];
                return result;
            }
        }
    }
    return result;
}

// IDA_EA: 0x8A73A80
// IDA_NAME: _ZN18RandomOptionScript14getPrefixGradeEi
// DEMANGLED: RandomOptionScript::getPrefixGrade(int)
int RandomOptionScript::getPrefixGrade(int percent) {
    int result = 0;
    for (unsigned int index = 0; index < choose_prefix.size(); ++index) {
        if (choose_prefix[index].begin <= percent &&
            choose_prefix[index].end > percent) {
            return choose_prefix[index].grade - 1;
        }
    }
    return result;
}

// IDA_EA: 0x8A7EA82
// IDA_NAME: _ZN9_str_costC2Eii
// DEMANGLED: _str_cost::_str_cost(int,int)
_str_cost::_str_cost(int first_value, int second_value)
    : first(first_value),
      second(second_value) {}

// IDA_EA: 0x8A7EA98
// IDA_NAME: _ZN23_str_option_value_ratioC2Effi
// DEMANGLED: _str_option_value_ratio::_str_option_value_ratio(float,float,int)
_str_option_value_ratio::_str_option_value_ratio(float first_value,
                                                 float second_value,
                                                 int ratio_value)
    : first(first_value),
      second(second_value),
      ratio(ratio_value) {}

// IDA_EA: 0x8A7FBF2
// IDA_NAME: _ZN23_str_option_value_ratioD2Ev
// DEMANGLED: _str_option_value_ratio::~_str_option_value_ratio()
_str_option_value_ratio::~_str_option_value_ratio() {}

// IDA_EA: 0x8A7EAB8
// IDA_NAME: _ZN22str_index_to_itemgroupC2Ev
// DEMANGLED: str_index_to_itemgroup::str_index_to_itemgroup(void)
str_index_to_itemgroup::str_index_to_itemgroup()
    : item_group(0),
      name(),
      item_groups() {}

// IDA_EA: 0x8A7EF9E
// IDA_NAME: _ZN22str_index_to_itemgroupC2ERKS_
// DEMANGLED: str_index_to_itemgroup::str_index_to_itemgroup(str_index_to_itemgroup const&)
str_index_to_itemgroup::str_index_to_itemgroup(
    const str_index_to_itemgroup& rhs)
    : item_group(rhs.item_group),
      name(rhs.name),
      item_groups(rhs.item_groups) {}

// IDA_EA: 0x85FA494
// IDA_NAME: _ZN22str_index_to_itemgroupD2Ev
// DEMANGLED: str_index_to_itemgroup::~str_index_to_itemgroup()
str_index_to_itemgroup::~str_index_to_itemgroup() {}

// IDA_EA: 0x85F9712
// IDA_NAME: _ZN28RandomOptionItem_RegeneratorC2Ev
// DEMANGLED: RandomOptionItem_Regenerator::RandomOptionItem_Regenerator(void)
RandomOptionItem_Regenerator::RandomOptionItem_Regenerator()
    : item_groups(),
      costs(),
      quantity_ratio(),
      value_ratio(),
      total_value_ratio(0) {}

// IDA_EA: 0x85F97E2
// IDA_NAME: _ZN28RandomOptionItem_RegeneratorD2Ev
// DEMANGLED: RandomOptionItem_Regenerator::~RandomOptionItem_Regenerator()
RandomOptionItem_Regenerator::~RandomOptionItem_Regenerator() {}

// IDA_EA: 0x85F8496
// IDA_NAME: _ZN15RegenerationROIC2Ev
// DEMANGLED: RegenerationROI::RegenerationROI(void)
RegenerationROI::RegenerationROI()
    : random(0),
      regenerator() {
    random = new CMTRand();
}

// IDA_EA: 0x85F8506
// IDA_NAME: _ZN15RegenerationROID2Ev
// DEMANGLED: RegenerationROI::~RegenerationROI()
RegenerationROI::~RegenerationROI() {
    if (random != 0) {
        ::operator delete(random);
    }
    random = 0;
}

// IDA_EA: 0x85F963E
// IDA_NAME: _ZN15RegenerationROI36_0_Load_RegenerationRandomOptionItemEv
// DEMANGLED: RegenerationROI::_0_Load_RegenerationRandomOptionItem(void)
int RegenerationROI::_0_Load_RegenerationRandomOptionItem() {
    const int result =
        importRegenerationRandomOptionScript(
            regenerator,
            "Etc/RandomOption/RegenerationRandomOption.etc");
    if (result == 0) {
        return 0;
    }
    printf("Fail to Load Random Option Item Regenerator Line : %d\n",
           result);
    return result;
}

// IDA_EA: 0x85F853C
// IDA_NAME: _ZN15RegenerationROI18SendMessageToCUserEP5CUserPKc
// DEMANGLED: RegenerationROI::SendMessageToCUser(CUser *,char const*)
void RegenerationROI::SendMessageToCUser(CUser* user, const char* message) {
    if (user == 0 || message == 0 || user->isGMUser() == 0) {
        return;
    }
    PacketGuard packet;
    packet.put_header(0, 12);
    packet.put_byte(0);
    packet.put_short(0);
    packet.put_byte(0);
    const int length = static_cast<int>(std::strlen(message));
    packet.put_int(length);
    packet.put_str(message, length);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x85F8EC2
// IDA_NAME: _ZN15RegenerationROI12_Select_PartEiRSt6vectorIiSaIiEE
// DEMANGLED: RegenerationROI::_Select_Part(int,std::vector<int,std::allocator<int>> &)
void RegenerationROI::_Select_Part(int select_part,
                                   std::vector<int>& item_groups) {
    const std::map<int, str_index_to_itemgroup>::const_iterator selection =
        regenerator.item_groups.find(select_part);
    if (selection == regenerator.item_groups.end()) {
        cMyTrace trace(
            "void RegenerationROI::_Select_Part(int, std::vector<int, std::allocator<int> >&)",
            214, 5);
        trace("packet:%d groupName:%s\n", select_part, "");
        return;
    }

    std::map<std::string, int>* const group_name_table =
        getItemGroupNameTable();
    for (std::vector<std::string>::const_iterator name =
             selection->second.item_groups.begin();
         name != selection->second.item_groups.end();
         ++name) {
        const std::map<std::string, int>::const_iterator found =
            group_name_table->find(*name);
        if (found != group_name_table->end()) {
            item_groups.push_back(found->second);
        }
    }
}

// IDA_EA: 0x85F9094
// IDA_NAME: _ZN15RegenerationROI20_Calculate_RegenCostEiiii
// DEMANGLED: RegenerationROI::_Calculate_RegenCost(int,int,int,int)
int RegenerationROI::_Calculate_RegenCost(int select_part,
                                          int regeneration_mode,
                                          int first_usable_level,
                                          int second_usable_level) {
    const std::map<int, std::map<int, _str_cost> >::const_iterator
        part_costs = regenerator.costs.find(select_part);
    int select_part_cost = 0;
    if (select_part != 0 && select_part <= 99) {
        if (part_costs == regenerator.costs.end()) {
            return -1;
        }
        const std::map<int, _str_cost>::const_iterator first =
            part_costs->second.find(first_usable_level);
        const std::map<int, _str_cost>::const_iterator second =
            part_costs->second.find(second_usable_level);
        if (first == part_costs->second.end() ||
            second == part_costs->second.end()) {
            return -1;
        }
        if (first->second.first == 0) {
            select_part_cost = first->second.second;
        }
        if (second->second.first == 0) {
            // The old code adds the second item then halves the accumulated
            // price, including a zero first-side price.
            select_part_cost += second->second.second;
            select_part_cost /= 2;
        }
    }

    int mode_cost = 0;
    if (regeneration_mode > 100) {
        const std::map<int, std::map<int, _str_cost> >::const_iterator
            costs = regenerator.costs.find(regeneration_mode);
        if (costs == regenerator.costs.end()) {
            return -1;
        }
        const std::map<int, _str_cost>::const_iterator first =
            costs->second.find(first_usable_level);
        const std::map<int, _str_cost>::const_iterator second =
            costs->second.find(second_usable_level);
        if (first == costs->second.end() || second == costs->second.end()) {
            return -1;
        }
        if (first->second.first == 0) {
            mode_cost = first->second.second;
        }
        if (second->second.first == 0) {
            mode_cost += second->second.second;
            mode_cost /= 2;
        }
    }
    return select_part_cost + mode_cost;
}

// IDA_EA: 0x85F93CC
// IDA_NAME: _ZN15RegenerationROI12_RandomLevelEiiR16select_base_itemRSt18basic_stringstreamIcSt11char_traitsIcESaIcEE
// DEMANGLED: RegenerationROI::_RandomLevel(int,int,select_base_item &,std::basic_stringstream<char,std::char_traits<char>> &)
void RegenerationROI::_RandomLevel(int first_usable_level,
                                   int second_usable_level,
                                   int& selected_level) {
    if (first_usable_level > second_usable_level) {
        const int temporary = first_usable_level;
        first_usable_level = second_usable_level;
        second_usable_level = temporary;
    }

    std::vector<int> levels;
    const int first_level = first_usable_level +
        (first_usable_level % 5 == 0 ? 0 : 5 - first_usable_level % 5);
    const int last_level = second_usable_level - second_usable_level % 5;
    for (int level = first_level; level <= last_level; level += 5) {
        levels.push_back(level);
    }
    if (levels.empty()) {
        return;
    }

    const unsigned long last_index =
        static_cast<unsigned long>(levels.size() - 1);
    selected_level = levels[random->randInt(last_index)];
    if (selected_level % 10 != 0 && selected_level > 10) {
        selected_level += 5;
    }
}

// IDA_EA: 0x85F8CB4
// IDA_NAME: _ZN15RegenerationROI28RegenVer_get_option_quantityEii
// DEMANGLED: RegenerationROI::RegenVer_get_option_quantity(int,int)
int RegenerationROI::RegenVer_get_option_quantity(int begin_index,
                                                   int end_index) {
    if (static_cast<unsigned int>(begin_index) >= 4u ||
        static_cast<unsigned int>(end_index) >= 4u) {
        return 0;
    }
    int total_ratio = 0;
    for (int index = begin_index - 1; index < end_index; ++index) {
        total_ratio += regenerator.quantity_ratio[index];
    }
    const unsigned long limit = static_cast<unsigned long>(total_ratio);
    const unsigned long roll = random->randInt(limit);
    int accumulated_ratio = 0;
    for (int index = begin_index - 1; index < end_index; ++index) {
        accumulated_ratio += regenerator.quantity_ratio[index];
        if (roll <= static_cast<unsigned long>(accumulated_ratio)) {
            return index + 1;
        }
    }
    return regenerator.quantity_ratio[begin_index];
}

// IDA_EA: 0x85F8DA6
// IDA_NAME: _ZN15RegenerationROI25RegenVer_get_option_valueEii
// DEMANGLED: RegenerationROI::RegenVer_get_option_value(int,int)
unsigned int RegenerationROI::RegenVer_get_option_value(int begin_value,
                                                        int end_value) {
    const unsigned long selection_limit = regenerator.total_value_ratio;
    const unsigned long roll = random->randInt(selection_limit);
    unsigned long accumulated_ratio = 0;
    _str_option_value_ratio selected(0.0f, 0.0f, 0);
    for (std::vector<_str_option_value_ratio>::const_iterator ratio =
             regenerator.value_ratio.begin();
         ratio != regenerator.value_ratio.end();
         ++ratio) {
        accumulated_ratio += static_cast<unsigned long>(ratio->ratio);
        if (roll <= accumulated_ratio) {
            selected = *ratio;
            break;
        }
    }
    const unsigned long range = static_cast<unsigned long>(
        static_cast<long double>(end_value - begin_value) * selected.second);
    const unsigned long offset = static_cast<unsigned long>(
        static_cast<long double>(begin_value) * selected.first);
    return static_cast<unsigned int>(offset + random->randInt(range));
}

// IDA_EA: 0x85F8646
// IDA_NAME: _ZN15RegenerationROI10RegenerateEiiiiRiR10Inven_ItemP5CUser
// DEMANGLED: RegenerationROI::Regenerate(int,int,int,int,int &,Inven_Item &,CUser *)
int RegenerationROI::Regenerate(int first_usable_level,
                                int second_usable_level,
                                int select_part,
                                int regeneration_mode,
                                int& cost,
                                Inven_Item& generated_item,
                                CUser* user) {
    int selected_level = 0;
    _RandomLevel(first_usable_level, second_usable_level, selected_level);
    static random_option::CRandomOptionItemHandle handle;
    if (regeneration_mode != 0 && regeneration_mode != 101) {
        return 17;
    }

    cost = _Calculate_RegenCost(select_part, regeneration_mode,
                                first_usable_level, second_usable_level);
    if (cost == -1) {
        cMyTrace trace(
            "int RegenerationROI::Regenerate(int, int, int, int, int&, Inven_Item&, CUser*)",
            71, 5);
        trace("ROI regen Error : %d, %d, %d, %d", select_part);
        return 17;
    }

    std::vector<int> item_groups;
    if (select_part != 0) {
        _Select_Part(select_part, item_groups);
    }
    if (select_part == -1) {
        return 17;
    }

    const int item_index = handle.RegenVer_choose_item(
        static_cast<ENUM_RARITY>(2), selected_level, item_groups);
    std::ostringstream debug;
    debug << "base item index : " << item_index << "\n";
    CItem* const item = G_CDataManager()->find_item(item_index);
    if (item == 0) {
        debug << "base item is Null\n";
        SendMessageToCUser(user, debug.str().c_str());
        return 17;
    }

    const int rarity = item->get_rarity();
    const int usable_level = item->getUsableLevel();
    const int option_grade = item->vtable_->get_random_option_grade(item);
    debug << "rarity : " << rarity << " level : " << usable_level
          << " base item item : " << option_grade << "\n";

    generated_item.reset();
    generated_item.item_id_ = item_index;
    G_CDataManager()->get_item_list()->create_item(
        ENUM_ITEM_CREATE_TYPE_DEFAULT, generated_item, 1);
    if (handle.RegenVer_give_option(
            static_cast<unsigned long>(item_index),
            static_cast<ENUM_RARITY>(rarity), usable_level,
            item->getItemGroupName(), option_grade,
            &generated_item.chronicle_random_option_.random_option_, this,
            regeneration_mode) != 1) {
        debug << "give_option false\n";
        SendMessageToCUser(user, debug.str().c_str());
        return 19;
    }

    for (int index = 0; index <= 2; ++index) {
        const RandomOptionField& option =
            generated_item.chronicle_random_option_.random_option_.option_fields_[index];
        debug << "[" << index << "] :" << static_cast<int>(option.option_index_)
              << ":" << static_cast<int>(option.first_value_)
              << ":" << static_cast<int>(option.second_value_) << "\n";
    }
    SendMessageToCUser(user, debug.str().c_str());
    return 0;
}

// IDA_EA: 0x8A70A90
// IDA_NAME: _Z24importRandomOptionScriptR18RandomOptionScriptPKc
// DEMANGLED: importRandomOptionScript(RandomOptionScript &,char const*)
int importRandomOptionScript(RandomOptionScript& script, const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 17;
    }

    std::string tag;
    std::string text;
    bool got_value = false;

    while (ScanType(tag, true) == 1) {
        if (tag == "[random application]") {
            RandomApplicationKey key;
            RandomApplicationValue value;
            while (true) {
                key.rarity =
                    static_cast<uint8_t>(ScanInt(&got_value));
                if (!got_value) {
                    break;
                }
                key.grade =
                    static_cast<uint8_t>(ScanInt(&got_value));
                if (!got_value) {
                    return 39;
                }
                value.percent = ScanInt(&got_value);
                if (!got_value) {
                    return 43;
                }
                value.remain = ScanInt(&got_value);
                if (!got_value) {
                    return 47;
                }
                if (!script.random_application.insert(
                         std::make_pair(key, value)).second) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptRandomOption.cpp",
                        "importRandomOptionScript",
                        0x33,
                        "importRandomOptionScript - random_application_ duplicate key error");
                    return 52;
                }
            }
        } else if (tag == "[level limit]") {
            RandomApplicationKey key;
            LevelLimitDecisionValue value;
            while (true) {
                key.rarity =
                    static_cast<uint8_t>(ScanInt(&got_value));
                if (!got_value) {
                    break;
                }
                key.grade =
                    static_cast<uint8_t>(ScanInt(&got_value));
                if (!got_value) {
                    return 68;
                }
                value.current = ScanInt(&got_value);
                if (!got_value) {
                    return 72;
                }
                value.next = ScanInt(&got_value);
                if (!got_value) {
                    return 76;
                }
                if (!script.level_limit.insert(std::make_pair(key, value))
                         .second) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptRandomOption.cpp",
                        "importRandomOptionScript",
                        0x50,
                        "importRandomOptionScript - level_limit_ duplicate key error");
                    return 81;
                }
            }
        } else if (tag == "[part type]") {
            std::string item_group_name;
            while (true) {
                const int part = ScanInt(&got_value);
                if (!got_value) {
                    break;
                }
                item_group_name.clear();
                if (ScanStr(&item_group_name) != 1) {
                    return 100;
                }
                std::map<std::string, int>* table =
                    getItemGroupNameTable();
                std::map<std::string, int>::iterator name_it =
                    table->find(item_group_name);
                if (name_it == table->end()) {
                    return 104;
                }
                PartSelectonField field;
                field.item_group_name = name_it->second;
                field.accumulated_weight = ScanInt(&got_value);
                if (!got_value) {
                    return 110;
                }

                std::map<int, PartSelectonValue>::iterator part_it =
                    script.part_selection.find(part);
                if (part_it != script.part_selection.end()) {
                    part_it->second.raw_fields.push_back(field);
                    part_it->second.total_weight += field.accumulated_weight;
                    field.accumulated_weight = part_it->second.total_weight;
                    part_it->second.weighted_fields.push_back(field);
                } else {
                    PartSelectonValue value;
                    value.raw_fields.push_back(field);
                    value.weighted_fields.push_back(field);
                    value.total_weight = field.accumulated_weight;
                    script.part_selection.insert(std::make_pair(part, value));
                }
            }
        } else if (tag == "[base item]") {
            RandomApplicationKey key;
            BaseItemSelectionValue value;
            while (true) {
                key.rarity =
                    static_cast<uint8_t>(ScanInt(&got_value));
                if (!got_value) {
                    break;
                }
                key.grade =
                    static_cast<uint8_t>(ScanInt(&got_value));
                if (!got_value) {
                    return 148;
                }
                const int count = ScanInt(&got_value);
                if (!got_value) {
                    return 152;
                }
                value.clear();
                for (int index = 0; index < count; ++index) {
                    const int weight = ScanInt(&got_value);
                    if (!got_value) {
                        return 159;
                    }
                    value.total_weight +=
                        static_cast<uint32_t>(weight);
                    value.weights.push_back(value.total_weight);
                }
                if (!script.base_item_selection.insert(
                         std::make_pair(key, value)).second) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptRandomOption.cpp",
                        "importRandomOptionScript",
                        0xA8,
                        "importRandomOptionScript - base_item_selection_ duplicate key error");
                    return 169;
                }
            }
        } else if (tag == "[option type]") {
            RandomApplicationKey key;
            RandomOptionGive value;
            while (true) {
                key.rarity =
                    static_cast<uint8_t>(ScanInt(&got_value));
                if (!got_value) {
                    break;
                }
                const int option_type = ScanInt(&got_value);
                if (!got_value) {
                    return 188;
                }
                key.grade =
                    static_cast<uint8_t>(ScanInt(&got_value));
                if (!got_value) {
                    return 192;
                }
                value.clear();
                for (int index = 0; index <= 2; ++index) {
                    const int weight = ScanInt(&got_value);
                    if (!got_value) {
                        return 199;
                    }
                    value.total_weight += static_cast<uint32_t>(weight);
                    value.weights.push_back(static_cast<unsigned int>(weight));
                }

                std::map<RandomApplicationKey,
                         std::map<int, RandomOptionGive> >::iterator outer =
                    script.option_type.find(key);
                if (outer != script.option_type.end()) {
                    if (!outer->second.insert(
                             std::make_pair(option_type, value)).second) {
                        LogManager::logFormat(
                            0,
                            "../RDARScriptRandomOption.cpp",
                            "importRandomOptionScript",
                            0xD3,
                            "importRandomOptionScript - option_type_ duplicate key error");
                        return 212;
                    }
                } else {
                    std::map<int, RandomOptionGive> inner;
                    inner.insert(std::make_pair(option_type, value));
                    script.option_type.insert(std::make_pair(key, inner));
                }
            }
        } else if (tag == "[quantity ratio]") {
            while (true) {
                QuantityRatio value;
                value.quantity = ScanInt(&got_value);
                if (!got_value) {
                    break;
                }
                value.ratio = ScanInt(&got_value);
                if (!got_value) {
                    return 234;
                }
                script.quantity_ratio.push_back(value);
            }
        } else if (tag == "[option quantity]") {
            while (true) {
                const int rarity = ScanInt(&got_value);
                if (!got_value) {
                    break;
                }
                const int item_group_or_any = ScanInt(&got_value);
                if (!got_value) {
                    return 251;
                }
                const int option_type = ScanInt(&got_value);
                if (!got_value) {
                    return 255;
                }
                const int usable_level = ScanInt(&got_value);
                if (!got_value) {
                    return 259;
                }
                OptionQuantityValue value;
                value.begin_index =
                    static_cast<int16_t>(ScanInt(&got_value));
                if (!got_value) {
                    return 263;
                }
                value.end_index =
                    static_cast<int16_t>(ScanInt(&got_value));
                if (!got_value) {
                    return 267;
                }
                const uint32_t packed =
                    static_cast<uint32_t>(
                        static_cast<unsigned char>(rarity)) |
                    (static_cast<uint32_t>(
                         static_cast<unsigned char>(item_group_or_any))
                     << 8) |
                    (static_cast<uint32_t>(
                         static_cast<unsigned char>(option_type))
                     << 16) |
                    (static_cast<uint32_t>(
                         static_cast<unsigned char>(usable_level))
                     << 24);
                const int key = static_cast<int>(static_cast<int32_t>(packed));
                if (!script.option_quantity.insert(std::make_pair(key, value))
                         .second) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptRandomOption.cpp",
                        "importRandomOptionScript",
                        0x10F,
                        "importRandomOptionScript - option_quantity_ duplicate key error");
                    return 272;
                }
            }
        } else if (tag == "[choose option group]") {
            OptionGroupChoiceValue value;
            std::string item_group_name;
            while (true) {
                const int rarity = ScanInt(&got_value);
                if (!got_value) {
                    break;
                }
                const int item_group_or_any = ScanInt(&got_value);
                if (!got_value) {
                    return 293;
                }
                item_group_name.clear();
                if (ScanStr(&item_group_name) != 1) {
                    return 297;
                }
                std::map<std::string, int>* table =
                    getItemGroupNameTable();
                std::map<std::string, int>::iterator name_it =
                    table->find(item_group_name);
                if (name_it == table->end()) {
                    return 301;
                }
                const int option_type = ScanInt(&got_value);
                if (!got_value) {
                    return 307;
                }
                const int usable_level = ScanInt(&got_value);
                if (!got_value) {
                    return 311;
                }
                const int count = ScanInt(&got_value);
                if (!got_value) {
                    return 315;
                }
                value.clear();
                for (int index = 0; index < count; ++index) {
                    const int group_index = ScanInt(&got_value);
                    if (!got_value) {
                        return 322;
                    }
                    value.group_indexes.push_back(group_index);
                }
                for (int index = 0; index < count; ++index) {
                    const int weight = ScanInt(&got_value);
                    if (!got_value) {
                        return 331;
                    }
                    value.weights.push_back(static_cast<unsigned int>(weight));
                    value.total_weight += static_cast<uint32_t>(weight);
                }
                const unsigned long long key =
                    static_cast<unsigned long long>(
                        static_cast<unsigned char>(rarity)) |
                    (static_cast<unsigned long long>(
                         static_cast<unsigned char>(item_group_or_any))
                     << 8) |
                    (static_cast<unsigned long long>(
                         static_cast<unsigned char>(name_it->second))
                     << 16) |
                    (static_cast<unsigned long long>(
                         static_cast<unsigned char>(usable_level))
                     << 24) |
                    (static_cast<unsigned long long>(
                         static_cast<uint32_t>(option_type))
                     << 32);
                if (!script.option_group_choice.insert(
                         std::make_pair(key, value)).second) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptRandomOption.cpp",
                        "importRandomOptionScript",
                        0x153,
                        "importRandomOptionScript - option_group_choice_ duplicate key error");
                    return 340;
                }
            }
        } else if (tag == "[modified option selection]") {
            OptionGroupChoiceValue value;
            std::string item_group_name;
            while (true) {
                const int rarity = ScanInt(&got_value);
                if (!got_value) {
                    break;
                }
                const int item_group_or_any = ScanInt(&got_value);
                if (!got_value) {
                    return 362;
                }
                item_group_name.clear();
                if (ScanStr(&item_group_name) != 1) {
                    return 366;
                }
                std::map<std::string, int>* table =
                    getItemGroupNameTable();
                std::map<std::string, int>::iterator name_it =
                    table->find(item_group_name);
                if (name_it == table->end()) {
                    return 370;
                }
                const int option_type = ScanInt(&got_value);
                if (!got_value) {
                    return 376;
                }
                const int usable_level = ScanInt(&got_value);
                if (!got_value) {
                    return 380;
                }
                const int count = ScanInt(&got_value);
                if (!got_value) {
                    return 384;
                }
                value.clear();
                for (int index = 0; index < count; ++index) {
                    const int group_index = ScanInt(&got_value);
                    if (!got_value) {
                        return 391;
                    }
                    value.group_indexes.push_back(group_index);
                }
                for (int index = 0; index < count; ++index) {
                    const int weight = ScanInt(&got_value);
                    if (!got_value) {
                        return 400;
                    }
                    value.weights.push_back(static_cast<unsigned int>(weight));
                    value.total_weight += static_cast<uint32_t>(weight);
                }
                const unsigned long long key =
                    static_cast<unsigned long long>(
                        static_cast<unsigned char>(rarity)) |
                    (static_cast<unsigned long long>(
                         static_cast<unsigned char>(item_group_or_any))
                     << 8) |
                    (static_cast<unsigned long long>(
                         static_cast<unsigned char>(name_it->second))
                     << 16) |
                    (static_cast<unsigned long long>(
                         static_cast<unsigned char>(usable_level))
                     << 24) |
                    (static_cast<unsigned long long>(
                         static_cast<uint32_t>(option_type))
                     << 32);
                if (!script.modified_option_group_choice.insert(
                         std::make_pair(key, value)).second) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptRandomOption.cpp",
                        "importRandomOptionScript",
                        0x198,
                        "importRandomOptionScript - modified_option_group_choice_ duplicate key error");
                    return 409;
                }
            }
        } else if (tag == "[modified option type]") {
            RandomApplicationKey key;
            RandomOptionGive value;
            while (true) {
                key.rarity =
                    static_cast<uint8_t>(ScanInt(&got_value));
                if (!got_value) {
                    break;
                }
                const int option_type = ScanInt(&got_value);
                if (!got_value) {
                    return 428;
                }
                key.grade =
                    static_cast<uint8_t>(ScanInt(&got_value));
                if (!got_value) {
                    return 432;
                }
                value.clear();
                for (int index = 0; index <= 2; ++index) {
                    const int weight = ScanInt(&got_value);
                    if (!got_value) {
                        return 439;
                    }
                    value.total_weight += static_cast<uint32_t>(weight);
                    value.weights.push_back(static_cast<unsigned int>(weight));
                }

                std::map<RandomApplicationKey,
                         std::map<int, RandomOptionGive> >::iterator outer =
                    script.modified_option_type.find(key);
                if (outer != script.modified_option_type.end()) {
                    if (!outer->second.insert(
                             std::make_pair(option_type, value)).second) {
                        LogManager::logFormat(
                            0,
                            "../RDARScriptRandomOption.cpp",
                            "importRandomOptionScript",
                            0x1C3,
                            "importRandomOptionScript - modified_option_type_ duplicate key error");
                        return 452;
                    }
                } else {
                    std::map<int, RandomOptionGive> inner;
                    inner.insert(std::make_pair(option_type, value));
                    script.modified_option_type.insert(
                        std::make_pair(key, inner));
                }
            }
        } else if (tag == "[option modification]") {
            while (true) {
                OPTION_MODIFICATION_COST value;
                value.usable_level =
                    static_cast<int16_t>(ScanInt(&got_value));
                if (!got_value) {
                    break;
                }
                value.reserved_02[0] = 0;
                value.reserved_02[1] = 0;
                value.rarity_rare_cost = ScanInt(&got_value);
                if (!got_value) {
                    return 474;
                }
                value.rarity_unique_cost = ScanInt(&got_value);
                if (!got_value) {
                    return 478;
                }
                script.option_modification_cost.push_back(value);
            }
        } else if (tag == "[option value ratio]") {
            OptionValueRatio value;
            while (true) {
                value.ratio_value = ScanFloat(&got_value);
                if (!got_value) {
                    break;
                }
                value.option_value = ScanFloat(&got_value);
                if (!got_value) {
                    break;
                }
                value.ratio = ScanInt(&got_value);
                if (!got_value) {
                    return 501;
                }
                script.option_value_ratio.total_ratio +=
                    static_cast<uint32_t>(value.ratio);
                script.option_value_ratio.ratios.push_back(value);
            }
        } else if (tag == "[option group]") {
            OptionGroupValue value;
            const int key = ScanInt(&got_value);
            if (!got_value) {
                return 515;
            }
            while (true) {
                OptionGroup group;
                group.option_group =
                    static_cast<uint32_t>(ScanInt(&got_value));
                if (!got_value) {
                    break;
                }
                if (group.option_group == 0 || group.option_group > 255U) {
                    return 524;
                }
                group.weight =
                    static_cast<uint32_t>(ScanInt(&got_value));
                if (!got_value) {
                    return 528;
                }
                value.groups.push_back(group);
                value.total_weight += group.weight;
            }
            if (!script.option_group.insert(std::make_pair(key, value))
                     .second) {
                LogManager::logFormat(
                    0,
                    "../RDARScriptRandomOption.cpp",
                    "importRandomOptionScript",
                    0x218,
                    "importRandomOptionScript - option_group_ duplicate key error");
                return 537;
            }
        } else if (tag == "[postfix]") {
            const int postfix_count = ScanInt(&got_value);
            if (!got_value) {
                break;
            }
            std::string postfix_text;
            while (true) {
                const int group = ScanInt(&got_value);
                if (!got_value) {
                    break;
                }
                std::vector<std::string> values;
                for (int index = 0; index < postfix_count; ++index) {
                    postfix_text.clear();
                    if (ScanStr(&postfix_text) != 1) {
                        return 0;
                    }
                    values.push_back(postfix_text);
                }
                script.prefixes[group] = values;
                values.clear();
                for (int index = 0; index < postfix_count; ++index) {
                    postfix_text.clear();
                    if (ScanStr(&postfix_text) != 1) {
                        return 0;
                    }
                    values.push_back(postfix_text);
                }
                script.modified_prefixes[group] = values;
            }
        } else if (tag == "[choose postfix]") {
            while (true) {
                const int begin = ScanInt(&got_value);
                if (!got_value) {
                    break;
                }
                const int end = ScanInt(&got_value);
                if (!got_value) {
                    break;
                }
                const int grade = ScanInt(&got_value);
                if (!got_value) {
                    break;
                }
                ChoosePrefix value(begin, end, grade);
                script.choose_prefix.push_back(value);
            }
        } else if (tag == "[break seal cost]") {
            while (true) {
                const int rarity = ScanInt(&got_value);
                if (!got_value) {
                    break;
                }
                const int usable_level = ScanInt(&got_value);
                if (!got_value) {
                    return 655;
                }
                const int grade = ScanInt(&got_value);
                if (!got_value) {
                    return 659;
                }
                const int cost = ScanInt(&got_value);
                if (!got_value) {
                    return 663;
                }
                const uint32_t packed =
                    static_cast<uint32_t>(
                        static_cast<unsigned char>(rarity)) |
                    (static_cast<uint32_t>(
                         static_cast<unsigned char>(grade))
                     << 8) |
                    (static_cast<uint32_t>(
                         static_cast<uint16_t>(usable_level))
                     << 16);
                const int key = static_cast<int>(static_cast<int32_t>(packed));
                if (!script.break_seal_cost.insert(std::make_pair(key, cost))
                         .second) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptRandomOption.cpp",
                        "importRandomOptionScript",
                        0x29B,
                        "importRandomOptionScript - break_seal_cost_ duplicate key error");
                    return 668;
                }
            }
        } else if (tag == "[option text]") {
            std::string option_text;
            while (true) {
                ScanInt(&got_value);
                if (!got_value) {
                    break;
                }
                option_text.clear();
                if (ScanStr(&option_text) != 1) {
                    return 683;
                }
            }
        } else if (tag == "[different weight]") {
            while (true) {
                const int rarity = ScanInt(&got_value);
                if (!got_value) {
                    break;
                }
                const int option_type = ScanInt(&got_value);
                if (!got_value) {
                    return 698;
                }
                DifferentWeightValue value;
                value.begin_value = ScanInt(&got_value);
                if (!got_value) {
                    return 702;
                }
                value.end_value = ScanInt(&got_value);
                if (!got_value) {
                    return 706;
                }
                if (value.end_value <= value.begin_value) {
                    return 709;
                }
                if (static_cast<unsigned int>(value.end_value) > 100U) {
                    return 712;
                }
                if (static_cast<unsigned int>(value.begin_value) > 100U) {
                    return 715;
                }
                const uint16_t packed =
                    static_cast<uint16_t>(
                        static_cast<unsigned char>(rarity)) |
                    static_cast<uint16_t>(
                        static_cast<uint16_t>(
                            static_cast<unsigned char>(option_type)) << 8);
                const short key = static_cast<short>(packed);
                if (!script.different_weight.insert(
                         std::make_pair(key, value)).second) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptRandomOption.cpp",
                        "importRandomOptionScript",
                        0x2CF,
                        "importRandomOptionScript - different_weight_ duplicate key error");
                    return 720;
                }
            }
        } else if (tag == "[changed postfix]") {
            std::string changed;
            ScanStr(&changed);
            script.changed_prefix = changed;
        } else if (tag == "[postfix grade modification]") {
            while (true) {
                const int grade = ScanInt(&got_value);
                if (!got_value) {
                    break;
                }
                ScanInt(&got_value);
                if (!got_value) {
                    return 747;
                }
                ResetValue value;
                value.begin = ScanInt(&got_value);
                if (!got_value) {
                    return 751;
                }
                value.end = ScanInt(&got_value);
                if (!got_value) {
                    return 755;
                }
                const int weight = ScanInt(&got_value);
                if (!got_value) {
                    return 759;
                }
                value.weight = static_cast<uint32_t>(weight);
                const unsigned char key =
                    static_cast<unsigned char>(grade);
                std::map<unsigned char, ResetValues>::iterator reset_it =
                    script.reset_values.find(key);
                if (reset_it != script.reset_values.end()) {
                    reset_it->second.total_weight +=
                        static_cast<uint32_t>(weight);
                    reset_it->second.values.push_back(value);
                } else {
                    ResetValues values;
                    values.total_weight = static_cast<uint32_t>(weight);
                    values.values.push_back(value);
                    script.reset_values.insert(std::make_pair(key, values));
                }
            }
        } else if (tag == "[unable to modify postfix grade]") {
            while (true) {
                const int value = ScanInt(&got_value);
                if (!got_value) {
                    break;
                }
                script.unable_modify_postfix_grade.insert(
                    static_cast<unsigned long>(value));
            }
        } else if (tag == "[gradeless postfix]") {
            while (true) {
                const int value = ScanInt(&got_value);
                if (!got_value) {
                    break;
                }
                script.gradeless_postfix.insert(
                    static_cast<unsigned long>(value));
            }
        }
    }

    return 0;
}

// IDA_EA: 0x8A7E610
// IDA_NAME: _Z36importRegenerationRandomOptionScriptR28RandomOptionItem_RegeneratorPKc
// DEMANGLED: importRegenerationRandomOptionScript(RandomOptionItem_Regenerator &,char const*)
int importRegenerationRandomOptionScript(
    RandomOptionItem_Regenerator& regenerator,
    const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    std::string text;
    bool got_value = false;
    int group = 0;

    while (ScanType(tag, true) == 1) {
        if (tag == "[choose part]") {
            group = ScanInt(&got_value);
            if (!got_value) {
                return 29;
            }

            ScanStr(&text);
            regenerator.item_groups[group].name = text;
            while (ScanStr(&text) == 1) {
                regenerator.item_groups[group].item_groups.push_back(text);
            }
        } else if (tag == "[adjust quantity ratio]") {
            group = ScanInt(&got_value);
            ScanStr(&text);
            regenerator.item_groups[group].name = text;
            for (int index = 0; index <= 2; ++index) {
                int ratio = 0;
                ScanInt(&ratio);
                regenerator.quantity_ratio.push_back(ratio);
            }
        } else if (tag == "[adjust value ratio]") {
            group = ScanInt(&got_value);
            ScanStr(&text);
            regenerator.item_groups[group].name = text;
            _str_option_value_ratio ratio(0.0f, 0.0f, 0);
            while (ScanFloat(&ratio.first) == 1) {
                ScanFloat(&ratio.second);
                ScanInt(&ratio.ratio);
                regenerator.value_ratio.push_back(ratio);
                regenerator.total_value_ratio += ratio.ratio;
            }
        } else if (tag == "[regeneration price]") {
            int max_level = ScanInt(&got_value);
            while (ScanInt(&group) == 1) {
                if (regenerator.costs.find(group) != regenerator.costs.end()) {
                    printf("random option item script error %u, group : %d\n",
                           92u,
                           group);
                    return 93;
                }

                std::map<int, _str_cost> group_costs;
                int level = 0;
                do {
                    int first = 0;
                    int second = 0;
                    ScanInt(&level);
                    ScanInt(&first);
                    ScanInt(&second);

                    const _str_cost cost(first, second);
                    std::map<int, _str_cost>::iterator cost_it =
                        group_costs.find(level);
                    if (cost_it != group_costs.end()) {
                        cost_it->second = cost;
                    } else {
                        group_costs.insert(std::make_pair(level, cost));
                    }
                } while (level < max_level);

                regenerator.costs.insert(std::make_pair(group, group_costs));
            }
        }
    }

    return 0;
}
