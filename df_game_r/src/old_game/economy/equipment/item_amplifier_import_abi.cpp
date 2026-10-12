#include "old_game/economy/equipment/item_amplifier_abi.h"

#include "common/config.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "infra/log/format/trace.h"

#include <cstring>
#include <string>
#include <utility>

// IDA_EA: 0x890A6B0
// IDA_NAME: _Z17importAmplifyItemPKcR21stAmplifyItemScript_t
// DEMANGLED: importAmplifyItem(char const*,stAmplifyItemScript_t &)
int importAmplifyItem(const char* filename, stAmplifyItemScript_t& script) {
    if (!loadRDARScriptFile("", filename)) {
        return 7457;
    }

    std::string tag;
    std::string option_name;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[option mapping table]") {
            while (ScanStr(&option_name) == 1) {
                const int option_type = ScanInt(&success);
                if (!success) {
                    return 7477;
                }

                const std::pair<std::map<std::string, ENUM_AMPLIFY_TYPE>::iterator,
                                bool>
                    inserted = script.amplify_types.insert(std::make_pair(
                        option_name,
                        static_cast<ENUM_AMPLIFY_TYPE>(option_type)));
                if (!inserted.second) {
                    return 7481;
                }
            }
        } else if (tag == "[amplification weight by level]") {
            const float level_weight = ScanFloat(3, &success);
            std::memcpy(&script.reserved_40[0],
                        &level_weight,
                        sizeof(level_weight));
            if (!success) {
                LogManager::logFormat(
                    0,
                    "../RDARScriptEtcParameter.cpp",
                    "importAmplifyItem",
                    0x1D40,
                    "ERROR [amplification weight by level]");
            }
        } else if (tag == "[amplification rate by rarity]") {
            for (int rarity = 0;; ++rarity) {
                const int rate = ScanInt(&success);
                if (!success) {
                    break;
                }
                if (rarity > 5) {
                    return 7501;
                }
                script.random_option_rates[rarity] = rate;
            }
        } else if (tag == "[option data]") {
            int cumulative_rate = 0;
            while (ScanStr(&option_name) == 1) {
                const std::map<std::string, ENUM_AMPLIFY_TYPE>::const_iterator
                    option_it = script.amplify_types.find(option_name);
                if (option_it == script.amplify_types.end()) {
                    return 7518;
                }

                const int rate = ScanInt(&success);
                if (!success) {
                    return 7524;
                }

                stAmplifyOptionInfo_t option_info;
                option_info.ability_type = static_cast<int>(option_it->second);
                option_info.rate = static_cast<uint16_t>(rate + cumulative_rate);
                option_info.value = static_cast<uint16_t>(ScanInt(&success));
                if (!success) {
                    return 7529;
                }

                cumulative_rate += rate;
                script.option_infos.push_back(option_info);
            }
        } else if (tag == "[rarity weight]") {
            while (ScanStr(&option_name) == 1) {
                const float rarity_weight = ScanFloat(&success);
                if (!success) {
                    return 7544;
                }

                const int rarity = getItemRarity(option_name.c_str());
                if (rarity == 6) {
                    return 7547;
                }
                script.invest_rates[rarity] = rarity_weight;
            }
        } else if (tag == "[bonus weight by equipment type]") {
            float* const equipment_weights =
                reinterpret_cast<float*>(&script.reserved_40[0x2C]);
            for (int equipment_type =
                     oldsrv::config::kOrdinaryEquipmentTypeFirst;
                 equipment_type <=
                     oldsrv::config::kOrdinaryEquipmentTypeLast;
                 ++equipment_type) {
                const float weight = ScanFloat(4, &success);
                equipment_weights[
                    equipment_type -
                    oldsrv::config::kOrdinaryEquipmentTypeFirst] = weight;
                if (!success) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptEtcParameter.cpp",
                        "importAmplifyItem",
                        0x1D89,
                        "ERROR : [bonus weight by equipment type]");
                }
            }
        } else if (tag == "[equip level const]") {
            script.invest_min_grade = ScanInt(&success);
            if (!success) {
                return 7570;
            }
        } else if (tag == "[purify material]") {
            const int item_idx = ScanInt(&success);
            if (!success) {
                return 7576;
            }
            const int item_count = ScanInt(&success);
            if (!success) {
                return 7579;
            }
            script.purify_material_item_idx[0] = item_idx;
            script.purify_material_item_count[0] = item_count;
        } else if (tag == "[purify only material]") {
            const int item_idx = ScanInt(&success);
            if (!success) {
                return 7592;
            }
            const int item_count = ScanInt(&success);
            if (!success) {
                return 7595;
            }
            script.purify_material_item_idx[1] = item_idx;
            script.purify_material_item_count[1] = item_count;
        } else if (tag == "[invest option]") {
            while (ScanStr(&option_name) == 1) {
                const std::map<std::string, ENUM_AMPLIFY_TYPE>::const_iterator
                    option_it = script.amplify_types.find(option_name);
                if (option_it == script.amplify_types.end()) {
                    return 7613;
                }

                const int item_idx = ScanInt(&success);
                if (!success) {
                    return 7619;
                }
                const int item_count = ScanInt(&success);
                if (!success) {
                    return 7623;
                }

                stInvestAmplifyOption_t option;
                std::memset(&option, 0, sizeof(option));
                option.ability_type = static_cast<int>(option_it->second);
                std::memcpy(&option.value, &item_count, sizeof(item_count));
                script.invest_materials[0].insert(std::make_pair(
                    static_cast<unsigned long>(item_idx), option));
            }
        } else if (tag == "[reinvest option]") {
            while (ScanStr(&option_name) == 1) {
                const std::map<std::string, ENUM_AMPLIFY_TYPE>::const_iterator
                    option_it = script.amplify_types.find(option_name);
                if (option_it == script.amplify_types.end()) {
                    return 7640;
                }

                const int item_idx = ScanInt(&success);
                if (!success) {
                    return 7646;
                }
                const int item_count = ScanInt(&success);
                if (!success) {
                    return 7650;
                }

                stInvestAmplifyOption_t option;
                std::memset(&option, 0, sizeof(option));
                option.ability_type = static_cast<int>(option_it->second);
                std::memcpy(&option.value, &item_count, sizeof(item_count));
                script.invest_materials[1].insert(std::make_pair(
                    static_cast<unsigned long>(item_idx), option));
            }
        } else if (tag == "[random invest upgrade option]") {
            while (ScanStr(&option_name) == 1) {
                const std::map<std::string, ENUM_AMPLIFY_TYPE>::const_iterator
                    option_it = script.amplify_types.find(option_name);
                if (option_it == script.amplify_types.end()) {
                    return 7668;
                }

                const int item_idx = ScanInt(&success);
                if (!success) {
                    return 7674;
                }
                const int item_count = ScanInt(&success);
                if (!success) {
                    return 7678;
                }

                stInvestAmplifyOption_t option;
                std::memset(&option, 0, sizeof(option));
                option.ability_type = static_cast<int>(option_it->second);
                std::memcpy(&option.value, &item_count, sizeof(item_count));
                script.invest_materials[2].insert(std::make_pair(
                    static_cast<unsigned long>(item_idx), option));
            }
        }
    }
    return 0;
}
