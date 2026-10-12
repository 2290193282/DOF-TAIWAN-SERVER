#include "old_game/dungeon/instance/compound_extream_dun_item_runtime.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_game/economy/inventory/item.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"

#include <algorithm>
#include <ctime>
#include <string>
#include <utility>
#include <vector>

namespace ExtreamDungeon {

// IDA_EA: 0x832C402
// IDA_NAME: _ZN14ExtreamDungeon25CCompound_ExtreamDun_ItemC2Ev
// DEMANGLED: ExtreamDungeon::CCompound_ExtreamDun_Item::CCompound_ExtreamDun_Item(void)
CCompound_ExtreamDun_Item::CCompound_ExtreamDun_Item()
    : script_data_(),
      level_bonus_script_data_() {
    const unsigned long seed_value =
        static_cast<unsigned long>(std::time(0));
    random_ = new CMTRand(seed_value);
}

// IDA_EA: 0x832C524
// IDA_NAME: _ZN14ExtreamDungeon25CCompound_ExtreamDun_ItemD2Ev
// DEMANGLED: ExtreamDungeon::CCompound_ExtreamDun_Item::~CCompound_ExtreamDun_Item()
CCompound_ExtreamDun_Item::~CCompound_ExtreamDun_Item() {
    delete random_;
    random_ = 0;
}

// IDA_EA: 0x832C65E
// IDA_NAME: _ZN14ExtreamDungeon25CCompound_ExtreamDun_Item10LoadScriptEPKcS2_
// DEMANGLED: ExtreamDungeon::CCompound_ExtreamDun_Item::LoadScript(char const*,char const*)
int CCompound_ExtreamDun_Item::LoadScript(const char* directory,
                                          const char* filename) {
    if (loadRDARScriptFile(directory, filename) != 1) {
        return 98;
    }

    std::string ignored;
    std::string section;
    while (ScanType(section, true) == 1) {
        if (section == "[insert rarity]") {
            for (int index = 0; index <= 2; ++index) {
                ScanStr(&ignored);
            }
        } else if (section == "[rarity dicision]") {
            for (int input_rarity = 0; input_rarity <= 2;
                 ++input_rarity) {
                for (int output_rarity = 0; output_rarity <= 3;
                     ++output_rarity) {
                    script_data_.rarity_decision_thresholds[input_rarity]
                                                          [output_rarity] =
                        static_cast<uint32_t>(
                            ScanInt(static_cast<bool*>(0)));
                }
            }
        } else if (section == "[level dicision]") {
            const unsigned int count =
                static_cast<unsigned int>(ScanInt(static_cast<bool*>(0)));
            for (unsigned int index = 0; index < count; ++index) {
                level_bonus_script_data_[0].level_boundaries.push_back(
                    static_cast<unsigned int>(ScanInt(static_cast<bool*>(0))));
            }
            for (int input_rarity = 0; input_rarity <= 2;
                 ++input_rarity) {
                for (unsigned int index = 0; index < count; ++index) {
                    level_bonus_script_data_[0]
                        .decision_threshold_by_input_rarity[input_rarity]
                        .push_back(static_cast<unsigned int>(
                            ScanInt(static_cast<bool*>(0))));
                }
            }
        } else if (section == "[silver level dicision]") {
            const unsigned int count =
                static_cast<unsigned int>(ScanInt(static_cast<bool*>(0)));
            for (unsigned int index = 0; index < count; ++index) {
                level_bonus_script_data_[1].level_boundaries.push_back(
                    static_cast<unsigned int>(ScanInt(static_cast<bool*>(0))));
            }
            for (int input_rarity = 0; input_rarity <= 2;
                 ++input_rarity) {
                for (unsigned int index = 0; index < count; ++index) {
                    level_bonus_script_data_[1]
                        .decision_threshold_by_input_rarity[input_rarity]
                        .push_back(static_cast<unsigned int>(
                            ScanInt(static_cast<bool*>(0))));
                }
            }
        }
    }
    return 0;
}

// IDA_EA: 0x832C5AC
// IDA_NAME: _ZNK14ExtreamDungeon25CCompound_ExtreamDun_Item18_convertOutputItemE28ENUM_UNUSABLE_EQUIPMENT_TYPE
int CCompound_ExtreamDun_Item::_convertOutputItem(
    ENUM_UNUSABLE_EQUIPMENT_TYPE type) const {
    switch (static_cast<int>(type)) {
        case 1:
            return 1;
        case 2:
            return 2;
        case 3:
            return 3;
        case 9:
            return 0;
        default:
            return 4;
    }
}

// IDA_EA: 0x832C5F0
// IDA_NAME: _ZNK14ExtreamDungeon25CCompound_ExtreamDun_Item23_convertInput_To_OutputENS_22ENUM_INPUTITEMS_RARITYE
int CCompound_ExtreamDun_Item::_convertInput_To_Output(
    ENUM_INPUTITEMS_RARITY rarity) const {
    switch (static_cast<int>(rarity)) {
        case 0:
            return 0;
        case 1:
            return 2;
        case 2:
            return 3;
        default:
            return 4;
    }
}

// IDA_EA: 0x832C620
// IDA_NAME: _ZNK14ExtreamDungeon25CCompound_ExtreamDun_Item17_convertInputItemE28ENUM_UNUSABLE_EQUIPMENT_TYPE
int CCompound_ExtreamDun_Item::_convertInputItem(
    ENUM_UNUSABLE_EQUIPMENT_TYPE type) const {
    switch (static_cast<int>(type)) {
        case 1:
            return 0;
        case 2:
            return 1;
        case 3:
            return 2;
        case 9:
            return 0;
        default:
            return 3;
    }
}

// IDA_EA: 0x832C976
// IDA_NAME: _ZNK14ExtreamDungeon25CCompound_ExtreamDun_Item17_determine_rarityEjNS_22ENUM_INPUTITEMS_RARITYE
int CCompound_ExtreamDun_Item::_determine_rarity(
    unsigned int random_value,
    ENUM_INPUTITEMS_RARITY input_rarity) const {
    const int input = static_cast<int>(input_rarity);
    if (input < 0 || input > 2) {
        return 0;
    }
    int rarity = 0;
    while (rarity <= 3 &&
           script_data_.rarity_decision_thresholds[input][rarity] <
               random_value) {
        ++rarity;
    }
    return rarity;
}

// IDA_EA: 0x832CC02
// IDA_NAME: _ZNK14ExtreamDungeon25CCompound_ExtreamDun_Item28_determine_level_bonus_indexEjNS_22ENUM_INPUTITEMS_RARITYENS_15eBindSphereTypeE
unsigned int CCompound_ExtreamDun_Item::_determine_level_bonus_index(
    unsigned int level,
    ENUM_INPUTITEMS_RARITY input_rarity,
    eBindSphereType sphere_type) const {
    const int input = static_cast<int>(input_rarity);
    const int sphere = static_cast<int>(sphere_type);
    if (input < 0 || input > 2 || sphere < 0 || sphere > 1) {
        return 0;
    }

    const std::vector<unsigned int>& thresholds =
        level_bonus_script_data_[sphere]
            .decision_threshold_by_input_rarity[input];
    unsigned int index = 0;
    while (index < thresholds.size() && thresholds[index] < level) {
        ++index;
    }
    return index;
}

// IDA_EA: 0x832C9CA
// IDA_NAME: _ZNK14ExtreamDungeon25CCompound_ExtreamDun_Item22_determine_level_bonusEjNS_22ENUM_INPUTITEMS_RARITYERjNS_15eBindSphereTypeE
std::pair<unsigned int, unsigned int>
CCompound_ExtreamDun_Item::_determine_level_bonus(
    unsigned int level,
    ENUM_INPUTITEMS_RARITY input_rarity,
    unsigned int& index,
    eBindSphereType sphere_type) const {
    const int input = static_cast<int>(input_rarity);
    const int sphere = static_cast<int>(sphere_type);
    index = _determine_level_bonus_index(level, input_rarity, sphere_type);
    if (input < 0 || input > 2 || sphere < 0 || sphere > 1) {
        return std::make_pair(0u, 0u);
    }

    // The per-rarity vector decides which bucket the random roll enters.  The
    // returned item-level interval comes from the separate boundary vector
    // (IDA object +0xb8c/+0), not from those probability thresholds.
    const std::vector<unsigned int>& boundaries =
        level_bonus_script_data_[sphere].level_boundaries;
    if (index == 0 || index >= boundaries.size()) {
        return std::make_pair(0u, 0u);
    }
    return std::make_pair(boundaries[index - 1] + 1u, boundaries[index]);
}

// IDA_EA: 0x832CADC
// IDA_NAME: _ZNK14ExtreamDungeon25CCompound_ExtreamDun_Item22_determine_level_bonusENS_22ENUM_INPUTITEMS_RARITYEjNS_23ENUM_OUTPUTITEMS_RARITYENS_15eBindSphereTypeE
std::pair<unsigned int, unsigned int>
CCompound_ExtreamDun_Item::_determine_level_bonus(
    ENUM_INPUTITEMS_RARITY input_rarity,
    unsigned int random_value,
    ENUM_OUTPUTITEMS_RARITY output_rarity,
    eBindSphereType sphere_type) const {
    const int input = static_cast<int>(input_rarity);
    const int output = static_cast<int>(output_rarity);
    const int sphere = static_cast<int>(sphere_type);
    unsigned int index =
        _determine_level_bonus_index(random_value, input_rarity, sphere_type);
    if (input == 0 && output > 1 && output <= 3 && index != 0) {
        --index;
    }
    if (input < 0 || input > 2 || sphere < 0 || sphere > 1) {
        return std::make_pair(0u, 0u);
    }

    const std::vector<unsigned int>& boundaries =
        level_bonus_script_data_[sphere].level_boundaries;
    if (index == 0 || index >= boundaries.size()) {
        return std::make_pair(0u, 0u);
    }
    return std::make_pair(boundaries[index - 1] + 1u, boundaries[index]);
}

// IDA_EA: 0x832D4B4
// IDA_NAME: _ZNK14ExtreamDungeon25CCompound_ExtreamDun_Item27_FindItemRelateCreationRateEN9__gnu_cxx17__normal_iteratorIPKNS_15OUTPUT_ITEM_STRESt6vectorIS3_SaIS3_EEEES9_
int CCompound_ExtreamDun_Item::_FindItemRelateCreationRate(
    std::vector<OUTPUT_ITEM_STR>::const_iterator first,
    std::vector<OUTPUT_ITEM_STR>::const_iterator last) const {
    if (first == last) {
        return 0;
    }

    typedef std::pair<unsigned int, int> WeightedItem;
    std::vector<WeightedItem> candidates;
    unsigned int total_creation_rate = 0;
    for (; first != last; ++first) {
        CItem* const item = G_CDataManager()->find_item(first->second);
        if (!item) {
            continue;
        }

        unsigned int creation_rate =
            static_cast<unsigned int>(item->get_gen_rate());
        const std::map<int, forceRule_ResultItem>::const_iterator rule =
            script_data_.force_rule_result_items.find(first->second);
        if (rule != script_data_.force_rule_result_items.end()) {
            creation_rate = static_cast<unsigned int>(rule->second.item_index);
        }
        candidates.push_back(std::make_pair(creation_rate, first->second));
        total_creation_rate += creation_rate;
    }

    if (candidates.empty()) {
        return 0;
    }
    if (total_creation_rate == 0) {
        return candidates[random_->randInt() % candidates.size()].second;
    }

    const unsigned int selected = random_->randInt() % total_creation_rate + 1u;
    unsigned int accumulated = 0;
    for (std::vector<WeightedItem>::const_iterator it = candidates.begin();
         it != candidates.end(); ++it) {
        accumulated += it->first;
        if (selected <= accumulated) {
            return it->second;
        }
    }
    return 0;
}

// IDA_EA: 0x832D15E
// IDA_NAME: _ZNK14ExtreamDungeon25CCompound_ExtreamDun_Item15_FindSelectItemEjNS_23ENUM_OUTPUTITEMS_RARITYERKSt4pairIjjE
int CCompound_ExtreamDun_Item::_FindSelectItem(
    unsigned int item_group,
    ENUM_OUTPUTITEMS_RARITY output_rarity,
    const std::pair<unsigned int, unsigned int>& level_range) const {
    const int rarity = static_cast<int>(output_rarity);
    if (item_group == 0 || item_group > 58 || rarity < 0 || rarity >= 4) {
        return 0;
    }
    // Item group zero is the intentionally unused ABI slot.  The original
    // table is indexed directly by the proven 1..58 group value.
    const std::vector<OUTPUT_ITEM_STR>& values =
        script_data_.output_items[item_group][rarity];
    const OUTPUT_ITEM_STR lower(static_cast<int>(level_range.first), 0);
    const OUTPUT_ITEM_STR upper(static_cast<int>(level_range.second),
                                0x7fffffff);
    const std::vector<OUTPUT_ITEM_STR>::const_iterator first =
        std::lower_bound(values.begin(), values.end(), lower,
                         BindTable_Predicate);
    const std::vector<OUTPUT_ITEM_STR>::const_iterator last =
        std::upper_bound(values.begin(), values.end(), upper,
                         BindTable_Predicate);
    return _FindItemRelateCreationRate(first, last);
}

// IDA_EA: 0x832D2CE
// IDA_NAME: _ZNK14ExtreamDungeon25CCompound_ExtreamDun_Item15_FindRandomItemENS_23ENUM_OUTPUTITEMS_RARITYERKSt4pairIjjE
int CCompound_ExtreamDun_Item::_FindRandomItem(
    ENUM_OUTPUTITEMS_RARITY output_rarity,
    const std::pair<unsigned int, unsigned int>& level_range) const {
    const int rarity = static_cast<int>(output_rarity);
    if (rarity < 0 || rarity >= 4) {
        return 0;
    }
    const std::vector<OUTPUT_ITEM_STR>& values =
        script_data_.secondary_output_items[rarity];
    const OUTPUT_ITEM_STR lower(static_cast<int>(level_range.first), 0);
    const OUTPUT_ITEM_STR upper(static_cast<int>(level_range.second),
                                0x7fffffff);
    const std::vector<OUTPUT_ITEM_STR>::const_iterator first =
        std::lower_bound(values.begin(), values.end(), lower,
                         BindTable_Predicate);
    const std::vector<OUTPUT_ITEM_STR>::const_iterator last =
        std::upper_bound(values.begin(), values.end(), upper,
                         BindTable_Predicate);
    return _FindItemRelateCreationRate(first, last);
}

// IDA_EA: 0x832D438
// IDA_NAME: _ZNK14ExtreamDungeon25CCompound_ExtreamDun_Item23_FindItemAll_BindSphereEPPK10CEquipItemcNS_23ENUM_OUTPUTITEMS_RARITYESt4pairIjjE
int CCompound_ExtreamDun_Item::_FindItemAll_BindSphere(
    const CEquipItem* const input_items[2],
    char selection_type,
    ENUM_OUTPUTITEMS_RARITY output_rarity,
    const std::pair<unsigned int, unsigned int>& level_range) const {
    if (selection_type == 2) {
        return _FindRandomItem(output_rarity, level_range);
    }
    if (selection_type == 0 || selection_type == 1) {
        return _FindSelectItem(
            static_cast<unsigned int>(
                input_items[static_cast<unsigned int>(selection_type)]
                    ->getItemGroupName()),
            output_rarity,
            level_range);
    }
    return 0;
}

// IDA_EA: 0x832CFFE
// IDA_NAME: _ZNK14ExtreamDungeon25CCompound_ExtreamDun_Item15_exception_RareEPPK10CEquipItemcNS_23ENUM_OUTPUTITEMS_RARITYEj
int CCompound_ExtreamDun_Item::_exception_Rare(
    const CEquipItem* const input_items[2],
    char selection_type,
    ENUM_OUTPUTITEMS_RARITY output_rarity,
    unsigned int base_level) const {
    for (unsigned int span = 0; span <= 1; ++span) {
        for (int rarity = static_cast<int>(output_rarity); rarity >= 0;
             --rarity) {
            std::pair<unsigned int, unsigned int> range;
            if (rarity > 1) {
                range = std::make_pair(base_level + span,
                                       base_level + span * 5u);
            } else if (rarity == 0) {
                range = std::make_pair(base_level + span * 5u + 1u,
                                       base_level + span * 5u + 5u);
            } else {
                continue;
            }
            const int item = _FindItemAll_BindSphere(
                input_items,
                selection_type,
                static_cast<ENUM_OUTPUTITEMS_RARITY>(rarity),
                range);
            if (item != 0) {
                return item;
            }
        }
    }
    return 0;
}

// IDA_EA: 0x832CCC0
// IDA_NAME: _ZN14ExtreamDungeon25CCompound_ExtreamDun_Item16makeCompoundItemEPPK10CEquipItemcR10Inven_ItemNS_15eBindSphereTypeE
int CCompound_ExtreamDun_Item::makeCompoundItem(
    const CEquipItem* const input_items[2],
    char selection_type,
    Inven_Item& output,
    eBindSphereType sphere_type) {
    output.reset();
    const unsigned int base_level = _get_base_level(input_items);
    const int input_rarity = _convertInputItem(
        static_cast<ENUM_UNUSABLE_EQUIPMENT_TYPE>(
            input_items[0]->GetUsableEquipmentType()));
    const unsigned int level_roll = random_->randInt() % 1000000u + 1u;
    const unsigned int rarity_roll = random_->randInt() % 1000000u + 1u;
    const int output_rarity = input_rarity != 0
                                  ? _convertInput_To_Output(
                                        static_cast<ENUM_INPUTITEMS_RARITY>(
                                            input_rarity))
                                  : _determine_rarity(
                                        rarity_roll,
                                        static_cast<ENUM_INPUTITEMS_RARITY>(0));

    const std::pair<unsigned int, unsigned int> bonus =
        _determine_level_bonus(
            static_cast<ENUM_INPUTITEMS_RARITY>(input_rarity),
            level_roll,
            static_cast<ENUM_OUTPUTITEMS_RARITY>(output_rarity),
            sphere_type);

    int result_item_id = 0;
    if (bonus.second != 10u || input_rarity == 1 ||
        base_level % 10u <= 4u) {
        result_item_id = _FindItemAll_BindSphere(
            input_items,
            selection_type,
            static_cast<ENUM_OUTPUTITEMS_RARITY>(output_rarity),
            std::make_pair(base_level + bonus.first,
                           base_level + bonus.second));
    }

    if (result_item_id == 0) {
        if (input_rarity == 0) {
            result_item_id = _exception_Rare(
                input_items,
                selection_type,
                static_cast<ENUM_OUTPUTITEMS_RARITY>(output_rarity),
                base_level);
        } else if (input_rarity == 1 || input_rarity == 2) {
            result_item_id = _FindItemAll_BindSphere(
                input_items,
                selection_type,
                static_cast<ENUM_OUTPUTITEMS_RARITY>(output_rarity),
                std::make_pair(base_level + 1u, base_level + 5u));
            if (result_item_id == 0) {
                result_item_id = _FindItemAll_BindSphere(
                    input_items,
                    selection_type,
                    static_cast<ENUM_OUTPUTITEMS_RARITY>(output_rarity),
                    std::make_pair(base_level + 6u, base_level + 10u));
            }
        }
    }

    if (result_item_id == 0) {
        return 0;
    }
    output.item_id_ = result_item_id;
    CItem* const prototype = G_CDataManager()->find_item(result_item_id);
    if (!prototype || !prototype->vtable_ || !prototype->vtable_->make_item) {
        return 0;
    }
    prototype->vtable_->make_item(prototype, output);
    return 1;
}

// IDA_EA: 0x832D784
// IDA_NAME: _ZN14ExtreamDungeon25CCompound_ExtreamDun_Item27ForceResultItemRule_SettingEibi
int CCompound_ExtreamDun_Item::ForceResultItemRule_Setting(
    int item_index,
    bool force,
    int creation_rate) {
    const forceRule_ResultItem result_item(force, creation_rate);
    std::map<int, forceRule_ResultItem>::iterator found =
        script_data_.force_rule_result_items.find(item_index);
    if (found == script_data_.force_rule_result_items.end()) {
        script_data_.force_rule_result_items.insert(
            std::make_pair(item_index, result_item));
    } else {
        found->second = result_item;
    }
    return force ? 1 : 0;
}

// IDA_EA: 0x832D7D6
// IDA_NAME: _ZNK14ExtreamDungeon25CCompound_ExtreamDun_Item17checkCompoundableEP10Inven_ItemPPK10CEquipItemc
int CCompound_ExtreamDun_Item::checkCompoundable(
    Inven_Item input_items[2],
    const CEquipItem* const item_prototypes[2],
    char sphere_type) const {
    int input_rarity[2] = {0, 0};
    for (int i = 0; i <= 1; ++i) {
        const CEquipItem* const item = item_prototypes[i];
        if (input_items[i].isEmpty() || !item) {
            return 49;
        }
        input_rarity[i] = _convertInputItem(
            static_cast<ENUM_UNUSABLE_EQUIPMENT_TYPE>(
                item->GetUsableEquipmentType()));
        if (item->vtable_ && item->vtable_->is_random_option &&
            item->vtable_->is_random_option(item)) {
            return 7;
        }
        if (input_rarity[i] == 3) {
            return 23;
        }
        if (input_rarity[i] == 2 && item->getItemGroupName() > 25 &&
            item->getItemGroupName() <= 50) {
            return 23;
        }
        if (input_items[i].amplify_option_.isIdentified() != 1) {
            return 19;
        }
        if (input_items[i].seal_or_state_ != 1) {
            return 18;
        }
        if (item->getItemGroupName() <= 0 || item->getItemGroupName() > 58) {
            return 17;
        }
        if (item->GetAttachType() == 1) {
            return 1;
        }
        if (item->vtable_ && item->vtable_->period_after_unsealing &&
            item->vtable_->period_after_unsealing(item)) {
            return 1;
        }
        if (input_items[i].upgrade_separate_.IsTradeRestriction()) {
            return 1;
        }
    }

    if (input_rarity[0] != input_rarity[1]) {
        return 22;
    }
    if (sphere_type != 0) {
        if (sphere_type != 1 && sphere_type != 3) {
            return 17;
        }
        if (input_rarity[0] != 1 && input_rarity[0] != 2) {
            return 17;
        }
    } else if (input_rarity[0] != 0) {
        return 17;
    }

    if (sphere_type == 3) {
        return item_prototypes[0]->getUsableLevel() > 60 ||
                       item_prototypes[1]->getUsableLevel() > 60
                   ? 17
                   : 0;
    }
    return item_prototypes[0]->getUsableLevel() > 69 &&
                   item_prototypes[1]->getUsableLevel() > 69
               ? 17
               : 0;
}

// IDA_EA: 0x832DB4E
// IDA_NAME: _ZNK14ExtreamDungeon25CCompound_ExtreamDun_Item21_isPossibleResultItemER10CEquipItem
bool CCompound_ExtreamDun_Item::_isPossibleResultItem(CEquipItem& item) const {
    const int group = item.getItemGroupName();
    if (group <= 0 || group > 58) {
        return false;
    }
    const std::map<int, forceRule_ResultItem>::const_iterator rule =
        script_data_.force_rule_result_items.find(item.get_index());
    if (rule != script_data_.force_rule_result_items.end()) {
        return !rule->second.force;
    }

    const int input_rarity = _convertInputItem(
        static_cast<ENUM_UNUSABLE_EQUIPMENT_TYPE>(
            item.GetUsableEquipmentType()));
    if (input_rarity == 2 && group > 25 && group <= 50) {
        return false;
    }
    // Bind-sphere candidates retain the original feature ceiling; using the
    // global 0828 character cap admits level 71+ results that CMD326 never
    // allowed and that its input/sphere bounds cannot produce consistently.
    if (item.getUsableLevel() > 70) {
        return false;
    }
    if (item.get_gen_rate() != 0) {
        return !item.vtable_ || !item.vtable_->is_random_option ||
               !item.vtable_->is_random_option(&item);
    }
    return input_rarity == 1;
}

// IDA_EA: 0x832DD1C
// IDA_NAME: _ZN14ExtreamDungeon25CCompound_ExtreamDun_Item8_AddItemER10CEquipItem
bool CCompound_ExtreamDun_Item::_AddItem(CEquipItem& item) {
    const int output_rarity = _convertOutputItem(
        static_cast<ENUM_UNUSABLE_EQUIPMENT_TYPE>(
            item.GetUsableEquipmentType()));
    const int group = item.getItemGroupName();
    if (output_rarity == 4 || group <= 0 || group > 58) {
        return false;
    }
    const OUTPUT_ITEM_STR value(item.getUsableLevel(), item.get_index());
    script_data_.secondary_output_items[output_rarity].push_back(value);
    script_data_.output_items[group][output_rarity].push_back(value);
    return true;
}

// IDA_EA: 0x832DB00
// IDA_NAME: _ZN14ExtreamDungeon25CCompound_ExtreamDun_Item15check_N_addItemER10CEquipItem
bool CCompound_ExtreamDun_Item::check_N_addItem(CEquipItem& item) {
    return _isPossibleResultItem(item) && _AddItem(item);
}

// IDA_EA: 0x832DE18
// IDA_NAME: _ZN14ExtreamDungeon25CCompound_ExtreamDun_Item13after_addItemEv
bool CCompound_ExtreamDun_Item::after_addItem() {
    for (int group = 1; group <= 58; ++group) {
        for (int rarity = 0; rarity < 4; ++rarity) {
            std::sort(script_data_.output_items[group][rarity].begin(),
                      script_data_.output_items[group][rarity].end());
        }
    }
    for (int rarity = 0; rarity < 4; ++rarity) {
        std::sort(script_data_.secondary_output_items[rarity].begin(),
                  script_data_.secondary_output_items[rarity].end());
    }
    return true;
}

// IDA_EA: 0x832E40C
// IDA_NAME: _ZNK14ExtreamDungeon25CCompound_ExtreamDun_Item15_get_base_levelEPKPK10CEquipItem
unsigned int CCompound_ExtreamDun_Item::_get_base_level(
    const CEquipItem* const input_items[2]) const {
    return static_cast<unsigned int>(
        (input_items[0]->getUsableLevel() +
         input_items[1]->getUsableLevel()) >> 1);
}

}  // namespace ExtreamDungeon
