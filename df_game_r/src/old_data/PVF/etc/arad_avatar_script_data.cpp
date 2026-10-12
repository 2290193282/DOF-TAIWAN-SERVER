#include "old_data/PVF/common/arad_script_data.h"

#include "old_data/PVF/common/rdar_script.h"

#include <map>
#include <string>

// IDA_EA: 0x88B01E8
// IDA_NAME: _ZN30STAvatarHiddenAbilityVariationC2EN26AVATAR_HIDDEN_ABILITY_TYPE1TEii
// DEMANGLED: STAvatarHiddenAbilityVariation::STAvatarHiddenAbilityVariation(AVATAR_HIDDEN_ABILITY_TYPE::T,int,int)
STAvatarHiddenAbilityVariation::STAvatarHiddenAbilityVariation(
    AVATAR_HIDDEN_ABILITY_TYPE::T type,
    int value,
    int variation_value)
    : ability_type(type),
      is_float(false),
      float_value(0.0f),
      int_value(value),
      variation(variation_value) {}

// IDA_EA: 0x88B021A
// IDA_NAME: _ZN30STAvatarHiddenAbilityVariationC2EN26AVATAR_HIDDEN_ABILITY_TYPE1TEfi
// DEMANGLED: STAvatarHiddenAbilityVariation::STAvatarHiddenAbilityVariation(AVATAR_HIDDEN_ABILITY_TYPE::T,float,int)
STAvatarHiddenAbilityVariation::STAvatarHiddenAbilityVariation(
    AVATAR_HIDDEN_ABILITY_TYPE::T type,
    float value,
    int variation_value)
    : ability_type(type),
      is_float(true),
      float_value(value),
      int_value(0),
      variation(variation_value) {}

// IDA_EA: 0x88B0AAE
// IDA_NAME: _ZN30STAvatarHiddenAbilityVariationD2Ev
// DEMANGLED: STAvatarHiddenAbilityVariation::~STAvatarHiddenAbilityVariation()
STAvatarHiddenAbilityVariation::~STAvatarHiddenAbilityVariation() {}

namespace ARAD {
namespace SCRIPT {

// IDA_EA: 0x88AECAC
// IDA_NAME: _ZN4ARAD6SCRIPT23AvatarFixedHiddenOptionC2Ev
// DEMANGLED: ARAD::SCRIPT::AvatarFixedHiddenOption::AvatarFixedHiddenOption(void)
AvatarFixedHiddenOption::AvatarFixedHiddenOption() {
    clearScript();
}

// IDA_EA: 0x88AED5E
// IDA_NAME: _ZN4ARAD6SCRIPT23AvatarFixedHiddenOptionD2Ev
// DEMANGLED: ARAD::SCRIPT::AvatarFixedHiddenOption::~AvatarFixedHiddenOption()
AvatarFixedHiddenOption::~AvatarFixedHiddenOption() {
    clearScript();
}

// IDA_EA: 0x88AEDCE
// IDA_NAME: _ZN4ARAD6SCRIPT23AvatarFixedHiddenOption10loadScriptEv
// DEMANGLED: ARAD::SCRIPT::AvatarFixedHiddenOption::loadScript(void)
int AvatarFixedHiddenOption::loadScript() {
    if (!loadRDARScriptFile("Etc/Avatar_Roulette/",
                            "AvatarFixedHiddenOptionList.etc")) {
        return -1;
    }

    std::string type;
    while (ScanType(type, true) == 1) {
        if (type == "[upper]") {
            while (ScanType(type, true) == 1 &&
                   parseAbility(ability_lists[2], type) == 1) {
            }
        } else if (type == "[rare]") {
            while (ScanType(type, true) == 1 &&
                   parseAbility(ability_lists[3], type) == 1) {
            }
        } else if (type == "[mystic circle]") {
            material_item_index = ScanInt(static_cast<bool*>(0));
        }
    }
    return 0;
}

// IDA_EA: 0x88AEF52
// IDA_NAME: _ZN4ARAD6SCRIPT23AvatarFixedHiddenOption12parseAbilityERSt6vectorI30STAvatarHiddenAbilityVariationSaIS3_EERSs
// DEMANGLED: ARAD::SCRIPT::AvatarFixedHiddenOption::parseAbility(std::vector<STAvatarHiddenAbilityVariation,std::allocator<STAvatarHiddenAbilityVariation>> &,std::string &)
int AvatarFixedHiddenOption::parseAbility(
    std::vector<STAvatarHiddenAbilityVariation>& abilities,
    std::string& type) {
    static const std::map<std::string, int> lookup = {
        std::make_pair("[physical attack]", 70),
        std::make_pair("[magical attack]", 75),
        std::make_pair("[physical defense]", 80),
        std::make_pair("[magical defense]", 85),
        std::make_pair("[HP MAX]", 90),
        std::make_pair("[MP MAX]", 95),
        std::make_pair("[HP regen speed]", 100),
        std::make_pair("[MP Regen speed]", 105),
        std::make_pair("[attack speed]", 110),
        std::make_pair("[move speed]", 115),
        std::make_pair("[cast speed]", 120),
        std::make_pair("[inventory limit]", 125),
        std::make_pair("[stuck]", 130),
        std::make_pair("[stuck resistance]", 135),
        std::make_pair("[all activestatus resistance]", 140),
        std::make_pair("[hit recovery]", 145),
        std::make_pair("[equipment magical defence]", 150),
        std::make_pair("[equipment physical defence]", 156),
        std::make_pair("[jump power]", 162),
        std::make_pair("[physical critical hit]", 167),
        std::make_pair("[magical critical hit]", 172)};

    const std::map<std::string, int>::const_iterator it = lookup.find(type);
    if (it == lookup.end()) {
        return 0;
    }

    const int code = it->second;
    if (code == 130 || code == 167 || code == 172) {
        const float value = ScanFloat(static_cast<bool*>(0));
        const int variation = ScanInt(static_cast<bool*>(0));
        abilities.push_back(
            STAvatarHiddenAbilityVariation(
                static_cast<AVATAR_HIDDEN_ABILITY_TYPE::T>(
                    code == 130 ? 13 : code == 167 ? 20 : 21),
                value,
                variation));
        return 1;
    }

    if (code == 150 || code == 156) {
        ScanInt(static_cast<bool*>(0));
    }
    const int value = ScanInt(static_cast<bool*>(0));
    const int variation = ScanInt(static_cast<bool*>(0));
    const int ability_type =
        code == 70  ? 1  : code == 75  ? 2  : code == 80  ? 3  :
        code == 85  ? 4  : code == 90  ? 5  : code == 95  ? 6  :
        code == 100 ? 7  : code == 105 ? 8  : code == 110 ? 9  :
        code == 115 ? 10 : code == 120 ? 11 : code == 125 ? 12 :
        code == 135 ? 14 : code == 140 ? 15 : code == 145 ? 16 :
        code == 150 ? 17 : code == 156 ? 18 : 19;
    abilities.push_back(STAvatarHiddenAbilityVariation(
        static_cast<AVATAR_HIDDEN_ABILITY_TYPE::T>(ability_type),
        value,
        variation));
    return 1;
}

// IDA_EA: 0x88B00E0
// IDA_NAME: _ZN4ARAD6SCRIPT23AvatarFixedHiddenOption11clearScriptEv
// DEMANGLED: ARAD::SCRIPT::AvatarFixedHiddenOption::clearScript(void)
bool AvatarFixedHiddenOption::clearScript() {
    for (int i = 0; i <= 5; ++i) {
        ability_lists[i].clear();
        ability_lists[i].push_back(
            STAvatarHiddenAbilityVariation(
                static_cast<AVATAR_HIDDEN_ABILITY_TYPE::T>(0), 0, 0));
    }
    return false;
}

// IDA_EA: 0x88B0164
// IDA_NAME: _ZN4ARAD6SCRIPT23AvatarFixedHiddenOption20getMaterialItemIndexEv
// DEMANGLED: ARAD::SCRIPT::AvatarFixedHiddenOption::getMaterialItemIndex(void)
int AvatarFixedHiddenOption::getMaterialItemIndex() {
    return material_item_index;
}

// IDA_EA: 0x88B0170
// IDA_NAME: _ZN4ARAD6SCRIPT23AvatarFixedHiddenOption19isMaterialItemIndexEi
// DEMANGLED: ARAD::SCRIPT::AvatarFixedHiddenOption::isMaterialItemIndex(int)
bool AvatarFixedHiddenOption::isMaterialItemIndex(int item_index) {
    return material_item_index == item_index;
}

// IDA_EA: 0x88B0DD0
// IDA_NAME: _ZN4ARAD6SCRIPT14AvatarRechargeC2Ev
// DEMANGLED: ARAD::SCRIPT::AvatarRecharge::AvatarRecharge(void)
AvatarRecharge::AvatarRecharge() : is_loaded(false) {}

// IDA_EA: 0x88B0E46
// IDA_NAME: _ZN4ARAD6SCRIPT14AvatarRechargeD2Ev
// DEMANGLED: ARAD::SCRIPT::AvatarRecharge::~AvatarRecharge()
AvatarRecharge::~AvatarRecharge() {}

// IDA_EA: 0x88B0EBA
// IDA_NAME: _ZN4ARAD6SCRIPT14AvatarRecharge10loadScriptEv
// DEMANGLED: ARAD::SCRIPT::AvatarRecharge::loadScript(void)
int AvatarRecharge::loadScript() {
    if (is_loaded) {
        return 0;
    }
    if (!loadRDARScriptFile("Etc/EmblemRenewal/", "avatar_recharge_jpn.etc")) {
        return -1;
    }

    std::string type;
    bool is_type = false;
    bool success = false;
    while (ScanType(type, true) == 1) {
        if (type == "[avatar recharge limit]") {
            avatar_recharge_limit = ScanInt(static_cast<bool*>(0));
        } else if (type == "[emblem recharge stone info]") {
            while (true) {
                const int item_index = ScanTypeOrInt(is_type, type, success, true);
                if (is_type || !success) {
                    break;
                }
                const int recharge_value = ScanInt(static_cast<bool*>(0));
                if (emblem_recharge_stone_info.find(item_index) !=
                    emblem_recharge_stone_info.end()) {
                    std::printf("[emblem recharge stone info] duplicate item_index:%d",
                                item_index);
                    return -2;
                }
                emblem_recharge_stone_info.insert(
                    std::make_pair(item_index, recharge_value));
            }
        } else if (type == "[emblem recharge]") {
            while (true) {
                const int grade = ScanTypeOrInt(is_type, type, success, true);
                if (is_type || !success) {
                    break;
                }
                const int recharge_value = ScanInt(static_cast<bool*>(0));
                if (emblem_recharge.find(grade) != emblem_recharge.end()) {
                    std::printf("[emblem recharge] duplicate grade:%d", grade);
                    return -3;
                }
                emblem_recharge.insert(std::make_pair(grade, recharge_value));
            }
        } else if (type == "[recharge use emblem info]") {
            while (true) {
                const int grade = ScanTypeOrInt(is_type, type, success, true);
                if (is_type || !success || grade > 7) {
                    break;
                }
                const int value = ScanInt(static_cast<bool*>(0));
                if (recharge_use_emblem_info.find(grade) !=
                    recharge_use_emblem_info.end()) {
                    std::printf("[recharge use emblem info] duplicate grade:%d", grade);
                    return -4;
                }
                recharge_use_emblem_info.insert(std::make_pair(grade, value));
            }
        }
    }
    unloadRDARScriptFile();
    is_loaded = true;
    return 0;
}

// IDA_EA: 0x88B12D8
// IDA_NAME: _ZN4ARAD6SCRIPT14AvatarRecharge11clearScriptEv
// DEMANGLED: ARAD::SCRIPT::AvatarRecharge::clearScript(void)
int AvatarRecharge::clearScript() {
    avatar_recharge_limit = 0;
    emblem_recharge_stone_info.clear();
    emblem_recharge.clear();
    recharge_use_emblem_info.clear();
    return 0;
}

// IDA_EA: 0x88B1314
// IDA_NAME: _ZN4ARAD6SCRIPT14AvatarRecharge23getAvatarRecharageLimitEv
// DEMANGLED: ARAD::SCRIPT::AvatarRecharge::getAvatarRecharageLimit(void)
int AvatarRecharge::getAvatarRecharageLimit() {
    return avatar_recharge_limit;
}

// IDA_EA: 0x88B131E
// IDA_NAME: _ZN4ARAD6SCRIPT14AvatarRecharge26getEmblemRechargeStoneInfoEv
// DEMANGLED: ARAD::SCRIPT::AvatarRecharge::getEmblemRechargeStoneInfo(void)
std::map<int, int>* AvatarRecharge::getEmblemRechargeStoneInfo() {
    return &emblem_recharge_stone_info;
}

// IDA_EA: 0x88B132A
// IDA_NAME: _ZN4ARAD6SCRIPT14AvatarRecharge17getEmblemRechargeEi
// DEMANGLED: ARAD::SCRIPT::AvatarRecharge::getEmblemRecharge(int)
int AvatarRecharge::getEmblemRecharge(int grade) {
    const std::map<int, int>::const_iterator it = emblem_recharge.find(grade);
    return it != emblem_recharge.end() ? it->second : 0;
}

// IDA_EA: 0x88B13A6
// IDA_NAME: _ZN4ARAD6SCRIPT14AvatarRecharge21IsEmblemRechargeStoneEi
// DEMANGLED: ARAD::SCRIPT::AvatarRecharge::IsEmblemRechargeStone(int)
bool AvatarRecharge::IsEmblemRechargeStone(int item_index) {
    return emblem_recharge_stone_info.find(item_index) !=
           emblem_recharge_stone_info.end();
}

// IDA_EA: 0x88B1408
// IDA_NAME: _ZN4ARAD6SCRIPT14EmblemCompoundC2Ev
// DEMANGLED: ARAD::SCRIPT::EmblemCompound::EmblemCompound(void)
EmblemCompound::EmblemCompound() {}

// IDA_EA: 0x88B14C2
// IDA_NAME: _ZN4ARAD6SCRIPT14EmblemCompoundD2Ev
// DEMANGLED: ARAD::SCRIPT::EmblemCompound::~EmblemCompound()
EmblemCompound::~EmblemCompound() {}

// IDA_EA: 0x88B154A
// IDA_NAME: _ZN4ARAD6SCRIPT14EmblemCompound10loadScriptEv
// DEMANGLED: ARAD::SCRIPT::EmblemCompound::loadScript(void)
int EmblemCompound::loadScript() {
    if (!loadRDARScriptFile("Etc/EmblemRenewal/", "emblem_compound_jpn.etc")) {
        return -1;
    }

    std::string type;
    bool is_type = false;
    bool success = false;
    while (ScanType(type, true) == 1) {
        if (type == "[emblem compound cost]") {
            while (true) {
                const int grade = ScanTypeOrInt(is_type, type, success, true);
                if (is_type || !success) {
                    break;
                }
                const int cost = ScanInt(static_cast<bool*>(0));
                if (costs.find(grade) != costs.end()) {
                    std::printf("[emblem compound cost] duplicate grade:%d", grade);
                    return -2;
                }
                costs.insert(std::make_pair(grade, cost));
            }
        } else if (type == "[emblem compound result]") {
            while (true) {
                const int grade = ScanTypeOrInt(is_type, type, success, true);
                if (is_type || !success) {
                    break;
                }
                Result* result = new Result;
                result->list_index = static_cast<uint16_t>(
                    ScanInt(static_cast<bool*>(0)));
                result->result_grade = static_cast<uint16_t>(
                    ScanInt(static_cast<bool*>(0)));
                result->item_count = static_cast<uint16_t>(
                    ScanInt(static_cast<bool*>(0)));
                result->success_rate = static_cast<uint16_t>(
                    ScanInt(static_cast<bool*>(0)));
                result->enabled = ScanInt(static_cast<bool*>(0)) == 1;
                if (results.find(grade) != results.end()) {
                    delete result;
                    std::printf("[emblem compound result] duplicate grade:%d", grade);
                    return -2;
                }
                results.insert(std::make_pair(grade, result));
            }
        } else if (type == "[emblem list]") {
            while (true) {
                const int list_index =
                    ScanTypeOrInt(is_type, type, success, true);
                if (is_type || !success) {
                    break;
                }
                const unsigned long item_index = static_cast<unsigned long>(
                    ScanInt(static_cast<bool*>(0)));
                emblem_lists[list_index].push_back(item_index);
            }
        }
    }
    unloadRDARScriptFile();
    return 0;
}

// IDA_EA: 0x88B1926
// IDA_NAME: _ZN4ARAD6SCRIPT14EmblemCompound11clearScriptEv
// DEMANGLED: ARAD::SCRIPT::EmblemCompound::clearScript(void)
int EmblemCompound::clearScript() {
    costs.clear();
    for (std::map<int, Result*>::iterator it = results.begin();
         it != results.end();
         ++it) {
        delete it->second;
    }
    results.clear();
    return 0;
}

// IDA_EA: 0x88B19B0
// IDA_NAME: _ZN4ARAD6SCRIPT14EmblemCompound7getCostEi
// DEMANGLED: ARAD::SCRIPT::EmblemCompound::getCost(int)
int EmblemCompound::getCost(int grade) {
    const std::map<int, int>::const_iterator it = costs.find(grade);
    return it != costs.end() ? it->second : -1;
}

// IDA_EA: 0x88B1A14
// IDA_NAME: _ZN4ARAD6SCRIPT14EmblemCompound9getResultEi
// DEMANGLED: ARAD::SCRIPT::EmblemCompound::getResult(int)
EmblemCompound::Result* EmblemCompound::getResult(int grade) {
    const std::map<int, Result*>::const_iterator it = results.find(grade);
    return it != results.end() ? it->second : 0;
}

// IDA_EA: 0x88B1A7E
// IDA_NAME: _ZN4ARAD6SCRIPT14EmblemCompound7getListEi
// DEMANGLED: ARAD::SCRIPT::EmblemCompound::getList(int)
std::vector<unsigned long>* EmblemCompound::getList(int grade) {
    const std::map<int, Result*>::const_iterator it = results.find(grade);
    if (it == results.end()) {
        return 0;
    }
    return &emblem_lists[it->second->list_index];
}

// IDA_EA: 0x88B1B0E
// IDA_NAME: _ZN4ARAD6SCRIPT13AvatarConvertC2Ev
// DEMANGLED: ARAD::SCRIPT::AvatarConvert::AvatarConvert(void)
AvatarConvert::AvatarConvert() {}

// IDA_EA: 0x88B1B22
// IDA_NAME: _ZN4ARAD6SCRIPT13AvatarConvertD2Ev
// DEMANGLED: ARAD::SCRIPT::AvatarConvert::~AvatarConvert()
AvatarConvert::~AvatarConvert() {}

namespace {

void DestroyAvatarConvertResults(
    std::map<int, AvatarConvert::ConvertResult*>& values) {
    for (std::map<int, AvatarConvert::ConvertResult*>::iterator it =
             values.begin();
         it != values.end(); ++it) {
        delete it->second;
    }
    values.clear();
}

const char* AvatarConvertClosingTag(const std::string& opening_tag) {
    if (opening_tag == "[rare avatar convert result]") {
        return "[/rare avatar convert result]";
    }
    if (opening_tag == "[special avatar convert result]") {
        return "[/special avatar convert result]";
    }
    if (opening_tag == "[avatar convert result]") {
        return "[/avatar convert result]";
    }
    return 0;
}

}  // namespace

// IDA_EA: 0x88B1B36
// IDA_NAME: _ZN4ARAD6SCRIPT13AvatarConvert10loadScriptEv
// DEMANGLED: ARAD::SCRIPT::AvatarConvert::loadScript(void)
int AvatarConvert::loadScript() {
    if (!loadRDARScriptFile("Etc/EmblemRenewal/", "avatar_convert_jpn.etc")) {
        return -1;
    }

    std::map<int, ConvertResult*> parsed_results;
    std::string type;
    while (ScanType(type, true) == 1) {
        const char* const closing_tag = AvatarConvertClosingTag(type);
        if (closing_tag == 0) {
            DestroyAvatarConvertResults(parsed_results);
            unloadRDARScriptFile();
            return -2;
        }

        int cumulative_weight = 0;
        ConvertResult* result = new ConvertResult;
        result->grade = 0;
        result->total_random_weight = 0;
        result->is_select = false;
        bool success = false;
        result->grade = ScanInt(&success);
        if (!success) {
            delete result;
            DestroyAvatarConvertResults(parsed_results);
            unloadRDARScriptFile();
            return -2;
        }
        result->is_select = ScanInt(&success) == 1;
        if (!success) {
            delete result;
            DestroyAvatarConvertResults(parsed_results);
            unloadRDARScriptFile();
            return -2;
        }

        bool is_type = false;
        while (true) {
            GetInfo info;
            info.item_index = ScanTypeOrInt(is_type, type, success, true);
            if (is_type || !success) {
                break;
            }
            const int item_count = ScanInt(&success);
            if (!success) {
                break;
            }
            const int random_weight = ScanInt(&success);
            if (!success || info.item_index <= 0 || item_count <= 0 ||
                item_count > 0x7fff || random_weight <= 0 ||
                random_weight > 0x7fff) {
                success = false;
                break;
            }
            info.item_count = static_cast<int16_t>(item_count);
            info.random_weight = static_cast<int16_t>(random_weight);
            result->result_items.push_back(info);
            cumulative_weight += info.random_weight;
            result->cumulative_random_weights.push_back(cumulative_weight);
        }

        if (!is_type || !success || type != closing_tag ||
            result->result_items.empty() || cumulative_weight <= 0 ||
            parsed_results.find(result->grade) != parsed_results.end()) {
            delete result;
            DestroyAvatarConvertResults(parsed_results);
            unloadRDARScriptFile();
            return -2;
        }
        result->total_random_weight = cumulative_weight;
        parsed_results.insert(std::make_pair(result->grade, result));
    }
    unloadRDARScriptFile();

    if (parsed_results.empty()) {
        return -2;
    }
    clearScript();
    results.swap(parsed_results);
    return 0;
}

// IDA_EA: 0x88B1DC2
// IDA_NAME: _ZN4ARAD6SCRIPT13AvatarConvert11clearScriptEv
// DEMANGLED: ARAD::SCRIPT::AvatarConvert::clearScript(void)
int AvatarConvert::clearScript() {
    for (std::map<int, ConvertResult*>::iterator it = results.begin();
         it != results.end();
         ++it) {
        delete it->second;
    }
    results.clear();
    return 0;
}

// IDA_EA: 0x88B1E48
// IDA_NAME: _ZN4ARAD6SCRIPT13AvatarConvert16getConvertResultEi
// DEMANGLED: ARAD::SCRIPT::AvatarConvert::getConvertResult(int)
AvatarConvert::ConvertResult* AvatarConvert::getConvertResult(int grade) {
    const std::map<int, ConvertResult*>::const_iterator it = results.find(grade);
    return it != results.end() ? it->second : 0;
}

// IDA_EA: 0x88B1F42
// IDA_NAME: _ZN4ARAD6SCRIPT13AvatarConvert13ConvertResultC2Ev
// DEMANGLED: ARAD::SCRIPT::AvatarConvert::ConvertResult::ConvertResult(void)
AvatarConvert::ConvertResult::ConvertResult() {}

// IDA_EA: 0x88B1F8E
// IDA_NAME: _ZN4ARAD6SCRIPT13AvatarConvert13ConvertResultD2Ev
// DEMANGLED: ARAD::SCRIPT::AvatarConvert::ConvertResult::~ConvertResult()
AvatarConvert::ConvertResult::~ConvertResult() {}

// IDA_EA: 0x88B3148
// IDA_NAME: _ZN4ARAD6SCRIPT13AvatarConvert7GetInfoD2Ev
// DEMANGLED: ARAD::SCRIPT::AvatarConvert::GetInfo::~GetInfo()
AvatarConvert::GetInfo::~GetInfo() {}

}  // namespace SCRIPT
}  // namespace ARAD
