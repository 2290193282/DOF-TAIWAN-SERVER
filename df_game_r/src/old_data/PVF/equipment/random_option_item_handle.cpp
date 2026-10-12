#include "old_data/PVF/equipment/random_option_item_handle.h"

#include <ctime>
#include <map>
#include <vector>

#include "common/manager/global_lifecycle.h"
#include "old_game/economy/equipment/item_option.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/equipment/random_option_script.h"
#include "common/manager/mt_rand.h"

namespace random_option {

namespace {

unsigned int choose_weighted_index(const std::vector<unsigned int>& weights,
                                   unsigned long random_value) {
    unsigned long accumulated_weight = 0;
    unsigned int selected = 0;
    for (unsigned int index = 0; index < weights.size(); ++index) {
        if (accumulated_weight <= random_value) {
            if (random_value - accumulated_weight < weights[index]) {
                selected = index;
            }
            accumulated_weight += weights[index];
        }
    }
    return selected;
}

int make_option_quantity_key(ENUM_RARITY rarity,
                             int item_group_or_any,
                             int option_type,
                             int usable_level) {
    const unsigned int packed =
        static_cast<unsigned int>(static_cast<unsigned char>(rarity)) |
        (static_cast<unsigned int>(
             static_cast<unsigned char>(item_group_or_any))
         << 8) |
        (static_cast<unsigned int>(static_cast<unsigned char>(option_type))
         << 16) |
        (static_cast<unsigned int>(static_cast<unsigned char>(usable_level))
         << 24);
    return static_cast<int>(packed);
}

unsigned long long make_option_group_choice_key(ENUM_RARITY rarity,
                                                 int item_group_or_any,
                                                 int item_group_name,
                                                 int usable_level,
                                                 int option_type) {
    return static_cast<unsigned long long>(
               static_cast<unsigned char>(rarity)) |
           (static_cast<unsigned long long>(
                static_cast<unsigned char>(item_group_or_any))
            << 8) |
           (static_cast<unsigned long long>(
                static_cast<unsigned char>(item_group_name))
            << 16) |
           (static_cast<unsigned long long>(
                static_cast<unsigned char>(usable_level))
            << 24) |
           (static_cast<unsigned long long>(
                static_cast<unsigned int>(option_type))
            << 32);
}

}  // namespace

// IDA_EA: 0x85F0EB0
// IDA_NAME: _ZN13random_option15GetRandomOptionEv
// DEMANGLED: random_option::GetRandomOption(void)
RandomOptionScript* GetRandomOption() {
    return G_CDataManager()->get_random_option_script();
}

// IDA_EA: 0x85F0D64
// IDA_NAME: _ZN13random_option23CRandomOptionItemHandleC2Ev
// DEMANGLED: random_option::CRandomOptionItemHandle::CRandomOptionItemHandle(void)
CRandomOptionItemHandle::CRandomOptionItemHandle()
    : random_streams_(new CMTRand[13]),
      random_stream_index_(0),
      disable_drop_(false),
      last_option_group_(0),
      random_option_give_(&GetRandomOption()->normal_give) {
    const unsigned long seed = static_cast<unsigned long>(std::time(0));
    for (int index = 0; index < 13; ++index) {
        random_streams_[index].seed(seed);
    }
}

// IDA_EA: 0x85F0E46
// IDA_NAME: _ZN13random_option23CRandomOptionItemHandleD2Ev
// DEMANGLED: random_option::CRandomOptionItemHandle::~CRandomOptionItemHandle()
CRandomOptionItemHandle::~CRandomOptionItemHandle() {
    delete[] random_streams_;
    random_streams_ = 0;
}

// IDA_EA: 0x8539E42
// IDA_NAME: _ZN13random_option23CRandomOptionItemHandle16set_disable_dropEv
// DEMANGLED: random_option::CRandomOptionItemHandle::set_disable_drop(void)
CRandomOptionItemHandle* CRandomOptionItemHandle::set_disable_drop() {
    disable_drop_ = true;
    return this;
}

// IDA_EA: 0x85F0EA2
// IDA_NAME: _ZN13random_option23CRandomOptionItemHandle22set_random_option_giveEP17CRandomOptionGive
// DEMANGLED: random_option::CRandomOptionItemHandle::set_random_option_give(CRandomOptionGive *)
CRandomOptionItemHandle* CRandomOptionItemHandle::set_random_option_give(
    CRandomOptionGive* random_option_give) {
    random_option_give_ = random_option_give;
    return this;
}

// IDA_EA: 0x85F2338
// IDA_NAME: _ZN13random_option23CRandomOptionItemHandle16_get_option_typeEm11ENUM_RARITYiiRh
// DEMANGLED: random_option::CRandomOptionItemHandle::_get_option_type(unsigned long,ENUM_RARITY,int,int,unsigned char &)
int CRandomOptionItemHandle::_get_option_type(unsigned long item_index,
                                              ENUM_RARITY rarity,
                                              int usable_level,
                                              int option_grade,
                                              unsigned char& option_type) {
    RandomApplicationKey key(static_cast<unsigned char>(rarity),
                             static_cast<unsigned char>(usable_level));
    std::map<RandomApplicationKey, std::map<int, RandomOptionGive> >*
        option_type_table = random_option_give_->get_option_type();
    std::map<RandomApplicationKey, std::map<int, RandomOptionGive> >::iterator
        application = option_type_table->find(key);
    if (application == option_type_table->end()) {
        return 575;
    }

    std::map<int, RandomOptionGive>::iterator give =
        application->second.find(option_grade);
    if (give == application->second.end()) {
        give = application->second.find(-1);
        if (give == application->second.end()) {
            return 597;
        }
    }

    const unsigned long random_value =
        random_streams_[4].randInt(give->second.total_weight);
    option_type = static_cast<unsigned char>(
        choose_weighted_index(give->second.weights, random_value) + 1);
    (void)item_index;
    return 0;
}

// IDA_EA: 0x85F25B0
// IDA_NAME: _ZN13random_option23CRandomOptionItemHandle20_get_option_quantityE11ENUM_RARITYiiiRi
// DEMANGLED: random_option::CRandomOptionItemHandle::_get_option_quantity(ENUM_RARITY,int,int,int,int &)
int CRandomOptionItemHandle::_get_option_quantity(ENUM_RARITY rarity,
                                                  int option_grade,
                                                  int option_type,
                                                  int usable_level,
                                                  int& quantity) {
    RandomOptionScript* const script = GetRandomOption();
    std::map<int, OptionQuantityValue>::iterator option_quantity =
        script->option_quantity.find(
            make_option_quantity_key(rarity, -1, option_type, usable_level));
    if (option_quantity == script->option_quantity.end()) {
        option_quantity = script->option_quantity.find(
            make_option_quantity_key(rarity,
                                     option_grade,
                                     option_type,
                                     usable_level));
        if (option_quantity == script->option_quantity.end()) {
            return 620;
        }
    }
    if (option_quantity->second.end_index == 0) {
        return 624;
    }

    std::vector<unsigned int> weights;
    unsigned long total_weight = 0;
    for (int index = option_quantity->second.begin_index - 1;
         index < option_quantity->second.end_index;
         ++index) {
        const unsigned int ratio = script->quantity_ratio[index].ratio;
        weights.push_back(ratio);
        total_weight += ratio;
    }
    if (weights.empty()) {
        return 637;
    }
    if (weights.size() == 1) {
        quantity = option_quantity->second.begin_index;
        return 0;
    }

    const unsigned long random_value = random_streams_[5].randInt(total_weight);
    quantity = static_cast<int>(choose_weighted_index(weights, random_value)) +
               option_quantity->second.begin_index;
    return 0;
}

// IDA_EA: 0x85F2882
// IDA_NAME: _ZN13random_option23CRandomOptionItemHandle20_choose_option_groupE11ENUM_RARITYiiiiRi
// DEMANGLED: random_option::CRandomOptionItemHandle::_choose_option_group(ENUM_RARITY,int,int,int,int,int &)
int CRandomOptionItemHandle::_choose_option_group(ENUM_RARITY rarity,
                                                  int option_grade,
                                                  int item_group_name,
                                                  int usable_level,
                                                  int option_type,
                                                  int& option_group) {
    std::map<unsigned long long, OptionGroupChoiceValue>* const choices =
        random_option_give_->get_option_group_choice();
    std::map<unsigned long long, OptionGroupChoiceValue>::iterator choice =
        choices->find(make_option_group_choice_key(rarity,
                                                    -1,
                                                    item_group_name,
                                                    usable_level,
                                                    option_type));
    if (choice == choices->end()) {
        choice = choices->find(make_option_group_choice_key(rarity,
                                                             option_grade,
                                                             item_group_name,
                                                             usable_level,
                                                             option_type));
        if (choice == choices->end()) {
            return 670;
        }
    }

    const unsigned long random_value =
        random_streams_[6].randInt(choice->second.total_weight - 1);
    const unsigned int selected =
        choose_weighted_index(choice->second.weights, random_value);
    if (selected >= choice->second.group_indexes.size()) {
        return 702;
    }
    option_group = choice->second.group_indexes[selected];
    return 0;
}

// IDA_EA: 0x85F2B02
// IDA_NAME: _ZN13random_option23CRandomOptionItemHandle17_get_option_valueEii
// DEMANGLED: random_option::CRandomOptionItemHandle::_get_option_value(int,int)
unsigned int CRandomOptionItemHandle::_get_option_value(int begin_value,
                                                        int end_value) {
    RandomOptionScript* const script = GetRandomOption();
    std::vector<unsigned int> weights;
    for (std::vector<OptionValueRatio>::const_iterator ratio =
             script->option_value_ratio.ratios.begin();
         ratio != script->option_value_ratio.ratios.end();
         ++ratio) {
        weights.push_back(static_cast<unsigned int>(ratio->ratio));
    }

    const unsigned long random_value = random_streams_[7].randInt(
        script->option_value_ratio.total_ratio);
    const unsigned int selected = choose_weighted_index(weights, random_value);
    OptionValueRatio ratio;
    if (selected < script->option_value_ratio.ratios.size()) {
        ratio = script->option_value_ratio.ratios[selected];
    }

    const unsigned long range = static_cast<unsigned long>(
        static_cast<long double>(end_value - begin_value) * ratio.option_value);
    const unsigned long offset = static_cast<unsigned long>(
        static_cast<long double>(begin_value) * ratio.ratio_value);
    return static_cast<unsigned int>(offset + random_streams_[8].randInt(range));
}

// IDA_EA: 0x85F2CC6
// IDA_NAME: _ZN13random_option23CRandomOptionItemHandle11give_optionEm11ENUM_RARITYiiiP12RandomOption
// DEMANGLED: random_option::CRandomOptionItemHandle::give_option(unsigned long,ENUM_RARITY,int,int,int,RandomOption *)
int CRandomOptionItemHandle::give_option(unsigned long item_index,
                                         ENUM_RARITY rarity,
                                         int usable_level,
                                         int item_group_name,
                                         int option_grade,
                                         RandomOption* option) {
    unsigned char option_type = 0;
    int error_line = _get_option_type(item_index,
                                      rarity,
                                      usable_level,
                                      option_grade,
                                      option_type);
    if (error_line != 0) {
        LogManager::logFormat(
            1,
            "RandomOption.cpp",
            "bool random_option::CRandomOptionItemHandle::give_option(itemIndex_t, ENUM_RARITY, int, int, int, RandomOption*)",
            0x2F9,
            "_get_option_type (%d,%d,%d,%d,%d) error_line(%d)",
            item_index,
            rarity,
            usable_level,
            option_grade,
            option_type,
            error_line);
        return 0;
    }

    int option_quantity = 0;
    error_line = _get_option_quantity(rarity,
                                      option_grade,
                                      option_type,
                                      usable_level,
                                      option_quantity);
    if (error_line != 0) {
        LogManager::logFormat(
            1,
            "RandomOption.cpp",
            "bool random_option::CRandomOptionItemHandle::give_option(itemIndex_t, ENUM_RARITY, int, int, int, RandomOption*)",
            0x300,
            "_get_option_quantity (%d,%d,%d,%d,%d) error_line(%d)",
            rarity,
            option_grade,
            option_type,
            usable_level,
            option_quantity,
            error_line);
        return 0;
    }

    int option_group = 0;
    error_line = _choose_option_group(rarity,
                                      option_grade,
                                      item_group_name,
                                      usable_level,
                                      option_type,
                                      option_group);
    if (error_line != 0) {
        LogManager::logFormat(
            1,
            "RandomOption.cpp",
            "bool random_option::CRandomOptionItemHandle::give_option(itemIndex_t, ENUM_RARITY, int, int, int, RandomOption*)",
            0x30D,
            "_choose_option_group (%d,%d,%d,%d,%d,%d) error_line(%d)",
            rarity,
            option_grade,
            item_group_name,
            usable_level,
            option_type,
            option_group,
            error_line);
        return 0;
    }

    const short different_weight_key = static_cast<short>(
        static_cast<unsigned char>(rarity) |
        (static_cast<unsigned short>(option_type) << 8));
    RandomOptionScript* const script = GetRandomOption();
    std::map<short, DifferentWeightValue>::iterator different_weight =
        script->different_weight.find(different_weight_key);
    if (different_weight == script->different_weight.end()) {
        LogManager::logFormat(
            1,
            "RandomOption.cpp",
            "bool random_option::CRandomOptionItemHandle::give_option(itemIndex_t, ENUM_RARITY, int, int, int, RandomOption*)",
            0x319,
            "different_weight_ (%d,%d) ",
            rarity,
            option_type);
        return 0;
    }

    std::map<int, OptionGroupValue>::iterator group_list =
        script->option_group.find(option_group);
    if (group_list == script->option_group.end()) {
        LogManager::logFormat(
            1,
            "RandomOption.cpp",
            "bool random_option::CRandomOptionItemHandle::give_option(itemIndex_t, ENUM_RARITY, int, int, int, RandomOption*)",
            0x321,
            "option_group_ (%d) ",
            option_group);
        return 0;
    }

    std::vector<OptionGroup> groups = group_list->second.groups;
    unsigned long total_weight = group_list->second.total_weight;
    for (int index = 0; index < option_quantity && index <= 2; ++index) {
        std::vector<unsigned int> weights;
        for (std::vector<OptionGroup>::const_iterator group = groups.begin();
             group != groups.end();
             ++group) {
            weights.push_back(group->weight);
        }

        const unsigned long random_value = random_streams_[9].randInt(total_weight);
        const unsigned int selected = choose_weighted_index(weights, random_value);
        if (selected >= groups.size()) {
            continue;
        }

        const OptionGroup group = groups[selected];
        option->option_fields_[index].option_index_ =
            static_cast<unsigned char>(group.option_group);
        total_weight -= group.weight;
        groups.erase(groups.begin() + selected);
        option->option_fields_[index].first_value_ =
            static_cast<unsigned char>(_get_option_value(
                different_weight->second.begin_value,
                different_weight->second.end_value));
        option->option_fields_[index].second_value_ =
            static_cast<unsigned char>(_get_option_value(
                different_weight->second.begin_value,
                different_weight->second.end_value));
    }

    option->initial_seed_.seed_ = static_cast<unsigned char>(
        (option_type & 3) | (option->initial_seed_.seed_ & 0xFC));
    const unsigned long random_limit = 64;
    option->initial_seed_.seed_ = static_cast<unsigned char>(
        (4 * (random_streams_[10].randInt(random_limit) & 0x3F)) |
        (option->initial_seed_.seed_ & 3));
    return 1;
}

// IDA_EA: 0x85F3340
// IDA_NAME: _ZN13random_option23CRandomOptionItemHandle13change_optionEm11ENUM_RARITYiiiiP12RandomOption
// DEMANGLED: random_option::CRandomOptionItemHandle::change_option(unsigned long,ENUM_RARITY,int,int,int,int,RandomOption *)
int CRandomOptionItemHandle::change_option(unsigned long item_index,
                                           ENUM_RARITY rarity,
                                           int usable_level,
                                           int item_group_name,
                                           int option_grade,
                                           int option_number,
                                           RandomOption* option) {
    if (option_number > 2) {
        return 0;
    }

    unsigned char option_type = 0;
    int error_line = _get_option_type(item_index, rarity, usable_level,
                                      option_grade, option_type);
    if (error_line != 0) {
        LogManager::logFormat(
            1, "RandomOption.cpp",
            "bool random_option::CRandomOptionItemHandle::change_option(itemIndex_t, ENUM_RARITY, int, int, int, int, RandomOption*)",
            0x356, "_get_option_type (%d,%d,%d,%d,%d) error_line(%d)",
            item_index, rarity, usable_level, option_grade, option_type,
            error_line);
        return 0;
    }

    int option_group = 0;
    error_line = _choose_option_group(rarity, option_grade, item_group_name,
                                      usable_level, option_type,
                                      option_group);
    if (error_line != 0) {
        LogManager::logFormat(
            1, "RandomOption.cpp",
            "bool random_option::CRandomOptionItemHandle::change_option(itemIndex_t, ENUM_RARITY, int, int, int, int, RandomOption*)",
            0x35D,
            "_choose_option_group (%d,%d,%d,%d,%d,%d) error_line(%d)",
            rarity, option_grade, item_group_name, usable_level, option_type,
            option_group, error_line);
        return 0;
    }

    const short different_weight_key = static_cast<short>(
        static_cast<unsigned char>(rarity) |
        (static_cast<unsigned short>(option_type) << 8));
    RandomOptionScript* const script = GetRandomOption();
    const std::map<short, DifferentWeightValue>::const_iterator
        different_weight = script->different_weight.find(different_weight_key);
    if (different_weight == script->different_weight.end()) {
        LogManager::logFormat(
            1, "RandomOption.cpp",
            "bool random_option::CRandomOptionItemHandle::change_option(itemIndex_t, ENUM_RARITY, int, int, int, int, RandomOption*)",
            0x369, "different_weight_ (%d,%d) ", rarity, option_type);
        return 0;
    }

    const std::map<int, OptionGroupValue>::const_iterator group_list =
        script->option_group.find(option_group);
    if (group_list == script->option_group.end()) {
        LogManager::logFormat(
            1, "RandomOption.cpp",
            "bool random_option::CRandomOptionItemHandle::change_option(itemIndex_t, ENUM_RARITY, int, int, int, int, RandomOption*)",
            0x371, "option_group_ (%d) ", option_group);
        return 0;
    }

    std::vector<OptionGroup> groups = group_list->second.groups;
    unsigned long total_weight = group_list->second.total_weight;
    for (int index = 0; index <= 2; ++index) {
        if (index == option_number ||
            option->option_fields_[index].option_index_ == 0) {
            continue;
        }

        for (std::vector<OptionGroup>::iterator group = groups.begin();
             group != groups.end();
             ++group) {
            if (group->option_group ==
                option->option_fields_[index].option_index_) {
                total_weight -= group->weight;
                groups.erase(group);
                break;
            }
        }
    }

    const unsigned long random_value = random_streams_[9].randInt(total_weight);
    std::vector<unsigned int> weights;
    weights.reserve(groups.size());
    for (std::vector<OptionGroup>::const_iterator group = groups.begin();
         group != groups.end();
         ++group) {
        weights.push_back(group->weight);
    }
    const unsigned int selected = choose_weighted_index(weights, random_value);
    if (selected >= groups.size()) {
        LogManager::logFormat(
            1, "RandomOption.cpp",
            "bool random_option::CRandomOptionItemHandle::change_option(itemIndex_t, ENUM_RARITY, int, int, int, int, RandomOption*)",
            0x390, "_option_group.size() %d, %d ", selected,
            groups.size());
        return 0;
    }

    const OptionGroup& selected_group = groups[selected];
    option->changed_option_.option_index_ =
        static_cast<unsigned char>(selected_group.option_group);
    option->changed_option_.first_value_ = static_cast<unsigned char>(
        _get_option_value(different_weight->second.begin_value,
                          different_weight->second.end_value));
    option->changed_option_.second_value_ = static_cast<unsigned char>(
        _get_option_value(different_weight->second.begin_value,
                          different_weight->second.end_value));
    option->changed_option_seed_.seed_ = static_cast<unsigned char>(
        (option_number & 3) | (option->changed_option_seed_.seed_ & 0xFCu));
    const unsigned long random_limit = 64;
    option->changed_option_seed_.seed_ = static_cast<unsigned char>(
        (4 * (random_streams_[10].randInt(random_limit) & 0x3Fu)) |
        (option->changed_option_seed_.seed_ & 3u));
    return 1;
}

// IDA_EA: 0x85F39D2
// IDA_NAME: _ZN13random_option23CRandomOptionItemHandle12reset_optionEm11ENUM_RARITYiP12RandomOption
// DEMANGLED: random_option::CRandomOptionItemHandle::reset_option(unsigned long,ENUM_RARITY,int,RandomOption *)
int CRandomOptionItemHandle::reset_option(unsigned long item_index,
                                          ENUM_RARITY rarity,
                                          int option_number,
                                          RandomOption* option) {
    (void)item_index;
    const ENUM_RANDOM_OPTION_NUMBER selected_option =
        static_cast<ENUM_RANDOM_OPTION_NUMBER>(option_number);
    const unsigned char option_index = static_cast<unsigned char>(
        option->get_option_index(selected_option));
    if (option_index == 0) {
        return 23;
    }

    RandomOptionScript* const script = GetRandomOption();
    if (script->gradeless_postfix.find(option_index) !=
        script->gradeless_postfix.end()) {
        return 23;
    }

    const std::map<unsigned char, ResetValues>::const_iterator values =
        script->reset_values.find(static_cast<unsigned char>(rarity));
    if (values == script->reset_values.end()) {
        return 19;
    }

    const unsigned long random_value = random_streams_[11].randInt(
        static_cast<unsigned long>(values->second.total_weight - 1));
    std::vector<unsigned int> weights;
    weights.reserve(values->second.values.size());
    for (std::vector<ResetValue>::const_iterator value =
             values->second.values.begin();
         value != values->second.values.end();
         ++value) {
        weights.push_back(value->weight);
    }
    const unsigned int selected = choose_weighted_index(weights, random_value);
    if (selected >= values->second.values.size()) {
        return 19;
    }

    const ResetValue& value = values->second.values[selected];
    const unsigned long range = static_cast<unsigned long>(
        value.end - value.begin - 1);
    unsigned int result = static_cast<unsigned int>(
        random_streams_[12].randInt(range) + value.begin);
    if (result > 100) {
        result = 100;
    }
    option->set_first_value(selected_option, static_cast<unsigned char>(result));
    option->set_second_value(selected_option,
                             static_cast<unsigned char>(result));
    return 0;
}

// IDA_EA: 0x85F0EC2
// IDA_NAME: _ZN13random_option23CRandomOptionItemHandle19_random_applicationE11ENUM_RARITYi
// DEMANGLED: random_option::CRandomOptionItemHandle::_random_application(ENUM_RARITY,int)
int CRandomOptionItemHandle::_random_application(ENUM_RARITY rarity,
                                                 int usable_level) {
    RandomOptionScript* const script = GetRandomOption();
    const RandomApplicationKey key(static_cast<uint8_t>(rarity),
                                   static_cast<uint8_t>(usable_level));
    const std::map<RandomApplicationKey, RandomApplicationValue>::const_iterator
        found = script->random_application.find(key);
    if (found == script->random_application.end()) {
        return 0;
    }

    const unsigned long limit = static_cast<unsigned long>(
        found->second.remain + found->second.percent);
    const unsigned long roll = random_streams_[0].randInt(limit);
    return found->second.remain < static_cast<int>(roll);
}

// IDA_EA: 0x85F0F9A
// IDA_NAME: _ZN13random_option23CRandomOptionItemHandle16_get_level_limitE11ENUM_RARITYi
// DEMANGLED: random_option::CRandomOptionItemHandle::_get_level_limit(ENUM_RARITY,int)
int CRandomOptionItemHandle::_get_level_limit(ENUM_RARITY rarity,
                                              int usable_level) {
    RandomOptionScript* const script = GetRandomOption();
    const RandomApplicationKey key(static_cast<uint8_t>(rarity),
                                   static_cast<uint8_t>(usable_level));
    const std::map<RandomApplicationKey, LevelLimitDecisionValue>::const_iterator
        found = script->level_limit.find(key);
    if (found == script->level_limit.end()) {
        return usable_level;
    }

    const unsigned long limit = static_cast<unsigned long>(
        found->second.current + found->second.next);
    const unsigned long roll = random_streams_[1].randInt(limit);
    if (found->second.current >= static_cast<int>(roll)) {
        return usable_level;
    }
    return usable_level == 1 ? 5 : usable_level + 5;
}

// IDA_EA: 0x85F1088
// IDA_NAME: _ZN13random_option23CRandomOptionItemHandle14_get_part_typeEi
// DEMANGLED: random_option::CRandomOptionItemHandle::_get_part_type(int)
int CRandomOptionItemHandle::_get_part_type(int usable_level) {
    RandomOptionScript* const script = GetRandomOption();
    const std::map<int, PartSelectonValue>::const_iterator part =
        script->part_selection.find(usable_level);
    if (part == script->part_selection.end()) {
        return 0;
    }

    const unsigned long roll =
        random_streams_[2].randInt(part->second.total_weight);
    for (std::vector<PartSelectonField>::const_iterator field =
             part->second.weighted_fields.begin();
         field != part->second.weighted_fields.end();
         ++field) {
        if (field->accumulated_weight > static_cast<int>(roll)) {
            return field->item_group_name;
        }
    }
    return 0;
}

// IDA_EA: 0x85F11A4
// IDA_NAME: _ZNK13random_option23CRandomOptionItemHandle27_get_part_type_by_ItemGroupEiRKSt6vectorIiSaIiEE
// DEMANGLED: random_option::CRandomOptionItemHandle::_get_part_type_by_ItemGroup(int,std::vector<int,std::allocator<int>> const&)const
int CRandomOptionItemHandle::_get_part_type_by_ItemGroup(
    int usable_level,
    const std::vector<int>& requested_item_groups) const {
    RandomOptionScript* const script = GetRandomOption();
    const std::map<int, PartSelectonValue>::const_iterator part =
        script->part_selection.find(usable_level);
    if (part == script->part_selection.end()) {
        return 0;
    }

    std::vector<unsigned int> weights(requested_item_groups.size(), 0);
    unsigned long total_weight = 0;
    for (std::vector<PartSelectonField>::const_iterator field =
             part->second.raw_fields.begin();
         field != part->second.raw_fields.end();
         ++field) {
        for (unsigned int index = 0; index < requested_item_groups.size();
             ++index) {
            if (field->item_group_name == requested_item_groups[index]) {
                weights[index] = static_cast<unsigned int>(
                    field->accumulated_weight);
                total_weight += weights[index];
                break;
            }
        }
    }

    const unsigned long roll = random_streams_[2].randInt(total_weight);
    unsigned long accumulated_weight = 0;
    for (unsigned int index = 0; index < weights.size(); ++index) {
        accumulated_weight += weights[index];
        if (roll < accumulated_weight) {
            return requested_item_groups[index];
        }
    }
    return 0;
}

// IDA_EA: 0x85F1426
// IDA_NAME: _ZN13random_option23CRandomOptionItemHandle14_get_base_itemE11ENUM_RARITYi
// DEMANGLED: random_option::CRandomOptionItemHandle::_get_base_item(ENUM_RARITY,int)
int CRandomOptionItemHandle::_get_base_item(ENUM_RARITY rarity,
                                            int usable_level) {
    RandomOptionScript* const script = GetRandomOption();
    const RandomApplicationKey key(static_cast<uint8_t>(rarity),
                                   static_cast<uint8_t>(usable_level));
    const std::map<RandomApplicationKey, BaseItemSelectionValue>::const_iterator
        found = script->base_item_selection.find(key);
    if (found == script->base_item_selection.end()) {
        return 0;
    }

    const unsigned long roll =
        random_streams_[3].randInt(found->second.total_weight);
    for (unsigned int index = 0; index < found->second.weights.size(); ++index) {
        // The parser stores cumulative thresholds in this exact vector.
        if (found->second.weights[index] > roll) {
            return static_cast<int>(index);
        }
    }
    return 0;
}

namespace {

int make_base_item_key(ENUM_RARITY rarity,
                       int usable_level,
                       int item_group_name) {
    const uint32_t packed =
        static_cast<uint32_t>(static_cast<uint8_t>(rarity)) |
        (static_cast<uint32_t>(static_cast<uint8_t>(usable_level)) << 8) |
        (static_cast<uint32_t>(static_cast<uint16_t>(item_group_name)) << 16);
    return static_cast<int>(packed);
}

int select_base_item_index(const std::map<int, BaseItemValue>& items,
                           int index) {
    if (items.empty()) {
        return -1;
    }
    std::map<int, BaseItemValue>::const_iterator selected = items.begin();
    for (int current = 0;
         current < index && selected != items.end();
         ++current) {
        ++selected;
    }
    if (selected == items.end()) {
        --selected;
    }
    return static_cast<int>(selected->second.item_index);
}

}  // namespace

// IDA_EA: 0x85F1560
// IDA_NAME: _ZN13random_option23CRandomOptionItemHandle11choose_itemER11ENUM_RARITYi
// DEMANGLED: random_option::CRandomOptionItemHandle::choose_item(ENUM_RARITY &,int)
int CRandomOptionItemHandle::choose_item(ENUM_RARITY& rarity,
                                         int usable_level) {
    if (disable_drop_) {
        return -1;
    }
    if (usable_level > 9) {
        usable_level = usable_level > 79
                           ? 80
                           : usable_level - usable_level % 10;
    }
    if (usable_level <= 9 && rarity == static_cast<ENUM_RARITY>(2)) {
        rarity = static_cast<ENUM_RARITY>(0);
        return -1;
    }
    if (!_random_application(rarity, usable_level)) {
        return -1;
    }

    ++random_stream_index_;
    const int selected_level = _get_level_limit(rarity, usable_level);
    const int item_group_name = _get_part_type(selected_level);
    const int selected_index = _get_base_item(rarity, selected_level);
    RandomOptionScript* const script = GetRandomOption();
    const std::map<int, std::map<int, BaseItemValue> >::const_iterator found =
        script->base_items.find(
            make_base_item_key(rarity, selected_level, item_group_name));
    if (found == script->base_items.end() || found->second.empty()) {
        rarity = static_cast<ENUM_RARITY>(0);
        return -1;
    }
    return select_base_item_index(found->second, selected_index);
}

// IDA_EA: 0x85F1802
// IDA_NAME: _ZN13random_option23CRandomOptionItemHandle20RegenVer_choose_itemE11ENUM_RARITYiRKSt6vectorIiSaIiEE
// DEMANGLED: random_option::CRandomOptionItemHandle::RegenVer_choose_item(ENUM_RARITY,int,std::vector<int,std::allocator<int>> const&)
int CRandomOptionItemHandle::RegenVer_choose_item(
    ENUM_RARITY rarity,
    int usable_level,
    const std::vector<int>& requested_item_groups) {
    const int selected_level = _get_level_limit(rarity, usable_level);
    const int item_group_name = requested_item_groups.empty()
                                    ? _get_part_type(selected_level)
                                    : _get_part_type_by_ItemGroup(
                                          selected_level,
                                          requested_item_groups);
    const int selected_index = _get_base_item(rarity, selected_level);
    RandomOptionScript* const script = GetRandomOption();
    const std::map<int, std::map<int, BaseItemValue> >::const_iterator found =
        script->base_items.find(
            make_base_item_key(rarity, selected_level, item_group_name));
    if (found == script->base_items.end()) {
        LogManager::logFormat(
            1, "RandomOption.cpp",
            "itemIndex_t random_option::CRandomOptionItemHandle::RegenVer_choose_item(ENUM_RARITY, int, const std::vector<int, std::allocator<int> >&)",
            0x19A, "choose_item (%d,%d,%d,%d)", rarity, selected_level,
            item_group_name, selected_index);
        return -1;
    }
    return select_base_item_index(found->second, selected_index);
}

// IDA_EA: 0x85F1A62
// IDA_NAME: _ZN13random_option23CRandomOptionItemHandle28RegenVer_get_option_quantityE11ENUM_RARITYiiiRiP15RegenerationROI
// DEMANGLED: random_option::CRandomOptionItemHandle::RegenVer_get_option_quantity(ENUM_RARITY,int,int,int,int &,RegenerationROI *)
int CRandomOptionItemHandle::RegenVer_get_option_quantity(
    ENUM_RARITY rarity,
    int option_grade,
    int option_type,
    int usable_level,
    int& quantity,
    RegenerationROI* regeneration) {
    RandomOptionScript* const script = GetRandomOption();
    std::map<int, OptionQuantityValue>::const_iterator found =
        script->option_quantity.find(
            make_option_quantity_key(rarity, -1, option_type, usable_level));
    if (found == script->option_quantity.end()) {
        found = script->option_quantity.find(make_option_quantity_key(
            rarity, option_grade, option_type, usable_level));
        if (found == script->option_quantity.end()) {
            return 429;
        }
    }
    if (found->second.end_index == 0) {
        return 433;
    }
    quantity = regeneration->RegenVer_get_option_quantity(
        found->second.begin_index, found->second.end_index);
    return quantity ? 0 : 437;
}

// IDA_EA: 0x85F1BB0
// IDA_NAME: _ZN13random_option23CRandomOptionItemHandle20RegenVer_give_optionEm11ENUM_RARITYiiiP12RandomOptionP15RegenerationROIi
// DEMANGLED: random_option::CRandomOptionItemHandle::RegenVer_give_option(unsigned long,ENUM_RARITY,int,int,int,RandomOption *,RegenerationROI *,int)
int CRandomOptionItemHandle::RegenVer_give_option(
    unsigned long item_index,
    ENUM_RARITY rarity,
    int usable_level,
    int item_group_name,
    int option_grade,
    RandomOption* option,
    RegenerationROI* regeneration,
    int regeneration_mode) {
    unsigned char option_type = 0;
    int error_line = _get_option_type(item_index, rarity, usable_level,
                                      option_grade, option_type);
    if (error_line != 0) {
        return 0;
    }

    int option_quantity = 0;
    error_line = regeneration_mode == 101
                     ? RegenVer_get_option_quantity(
                           rarity, option_grade, option_type, usable_level,
                           option_quantity, regeneration)
                     : _get_option_quantity(rarity, option_grade, option_type,
                                            usable_level, option_quantity);
    if (error_line != 0) {
        return 0;
    }

    int option_group = 0;
    error_line = _choose_option_group(rarity, option_grade, item_group_name,
                                      usable_level, option_type, option_group);
    if (error_line != 0) {
        return 0;
    }

    RandomOptionScript* const script = GetRandomOption();
    const short different_weight_key = static_cast<short>(
        static_cast<unsigned char>(rarity) |
        (static_cast<unsigned short>(option_type) << 8));
    const std::map<short, DifferentWeightValue>::const_iterator weight =
        script->different_weight.find(different_weight_key);
    if (weight == script->different_weight.end()) {
        return 0;
    }
    const std::map<int, OptionGroupValue>::const_iterator group_list =
        script->option_group.find(option_group);
    if (group_list == script->option_group.end()) {
        return 0;
    }

    std::vector<OptionGroup> groups = group_list->second.groups;
    unsigned long total_weight = group_list->second.total_weight;
    for (int index = 0; index < option_quantity && index <= 2; ++index) {
        std::vector<unsigned int> weights;
        for (std::vector<OptionGroup>::const_iterator group = groups.begin();
             group != groups.end();
             ++group) {
            weights.push_back(group->weight);
        }

        const unsigned int selected = choose_weighted_index(
            weights, random_streams_[9].randInt(total_weight));
        if (selected >= groups.size()) {
            continue;
        }
        const OptionGroup group = groups[selected];
        option->option_fields_[index].option_index_ =
            static_cast<unsigned char>(group.option_group);
        total_weight -= group.weight;
        groups.erase(groups.begin() + selected);

        const unsigned int first_value = regeneration_mode == 201
            ? regeneration->RegenVer_get_option_value(
                  weight->second.begin_value, weight->second.end_value)
            : _get_option_value(weight->second.begin_value,
                                weight->second.end_value);
        const unsigned int second_value = regeneration_mode == 201
            ? regeneration->RegenVer_get_option_value(
                  weight->second.begin_value, weight->second.end_value)
            : _get_option_value(weight->second.begin_value,
                                weight->second.end_value);
        option->option_fields_[index].first_value_ =
            static_cast<unsigned char>(first_value);
        option->option_fields_[index].second_value_ =
            static_cast<unsigned char>(second_value);
    }

    option->initial_seed_.seed_ = static_cast<unsigned char>(
        (option_type & 3) | (option->initial_seed_.seed_ & 0xFC));
    const unsigned long random_limit = 64;
    option->initial_seed_.seed_ = static_cast<unsigned char>(
        (4 * (random_streams_[10].randInt(random_limit) & 0x3F)) |
        (option->initial_seed_.seed_ & 3));
    return 1;
}

}  // namespace random_option
