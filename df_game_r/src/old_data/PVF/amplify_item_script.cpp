#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "common/config.h"
#include "old_game/economy/equipment/item_amplifier_abi.h"
#include "infra/log/format/trace.h"

#include <cstring>
#include <string>
#include <utility>

namespace {

enum {
    kMagiPurifyItemIndexOffset = 4,
    kMagiPurifyItemCountOffset = 8,
    kCeraPurifyItemIndexOffset = 12,
    kCeraPurifyItemCountOffset = 16,
};

bool LastTypeIs(const char* expected) {
    return expected != 0 &&
           g_dnfLex.getLastToken() == DNF_TOK_TYPE &&
           std::strcmp(g_dnfLex.getType(), expected) == 0;
}

bool ReadPurifyMaterial(int* item_index, int* item_count) {
    bool success = false;
    *item_index = ScanInt(&success);
    if (!success) return false;
    *item_count = ScanInt(&success);
    return success && *item_index > 0 && *item_count > 0;
}

void StoreMagiPurifyMaterial(stAmplifyItemScript_t& script,
                             int item_index,
                             int item_count) {
    std::memcpy(&script.reserved_40[kMagiPurifyItemIndexOffset],
                &item_index,
                sizeof(item_index));
    std::memcpy(&script.reserved_40[kMagiPurifyItemCountOffset],
                &item_count,
                sizeof(item_count));
}

void StoreCeraPurifyMaterial(stAmplifyItemScript_t& script,
                             int item_index,
                             int item_count) {
    std::memcpy(&script.reserved_40[kCeraPurifyItemIndexOffset],
                &item_index,
                sizeof(item_index));
    std::memcpy(&script.reserved_40[kCeraPurifyItemCountOffset],
                &item_count,
                sizeof(item_count));
}

}  // namespace

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
    bool seen_mapping = false;
    bool seen_level_weight = false;
    bool seen_rarity_rate = false;
    bool seen_option_data = false;
    bool seen_rarity_weight = false;
    bool seen_equipment_weight = false;
    bool seen_min_grade = false;
    bool seen_purify = false;
    bool seen_legacy_only_purify = false;
    bool seen_magi_purify = false;
    bool seen_cera_purify = false;
    bool seen_invest = false;
    bool seen_reinvest = false;
    bool seen_random_invest = false;
    int mapping_count = 0;
    int rarity_rate_count = 0;
    int option_data_count = 0;
    int rarity_weight_count = 0;
    int invest_count = 0;
    int reinvest_count = 0;
    int random_invest_count = 0;
    bool mapped_types[6] = {false, false, false, false, false, false};
    bool weighted_rarities[6] = {false, false, false, false, false, false};
    while (ScanType(tag, true) == 1) {
        if (tag == "[option mapping table]") {
            if (seen_mapping) return 7460;
            while (ScanStr(&option_name) == 1) {
                const int option_type = ScanInt(&success);
                if (!success || option_type < 0 || option_type > 5 ||
                    mapped_types[option_type]) {
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
                mapped_types[option_type] = true;
                ++mapping_count;
            }
            if (!LastTypeIs("[/option mapping table]") ||
                mapping_count != 6) {
                return 7482;
            }
            seen_mapping = true;
        } else if (tag == "[amplification weight by level]") {
            if (seen_level_weight) return 7483;
            const float level_weight = ScanFloat(3, &success);
            std::memcpy(&script.reserved_40[0],
                        &level_weight,
                        sizeof(level_weight));
            if (!success || level_weight <= 0.0f) {
                LogManager::logFormat(
                    0,
                    "../RDARScriptEtcParameter.cpp",
                    "importAmplifyItem",
                    0x1D40,
                    "ERROR [amplification weight by level]");
                return 7484;
            }
            seen_level_weight = true;
        } else if (tag == "[amplification rate by rarity]") {
            if (seen_rarity_rate) return 7485;
            for (int rarity = 0;; ++rarity) {
                const int rate = ScanInt(&success);
                if (!success) {
                    break;
                }
                if (rarity > 5) {
                    return 7501;
                }
                if (rate < 0 || rate > 10000) return 7501;
                script.random_option_rates[rarity] = rate;
                ++rarity_rate_count;
            }
            if (!LastTypeIs("[/amplification rate by rarity]") ||
                rarity_rate_count != 6) {
                return 7502;
            }
            seen_rarity_rate = true;
        } else if (tag == "[option data]") {
            if (seen_option_data) return 7503;
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
                ++option_data_count;
            }
            if (!LastTypeIs("[/option data]") || option_data_count != 4) {
                return 7530;
            }
            seen_option_data = true;
        } else if (tag == "[rarity weight]") {
            if (seen_rarity_weight) return 7531;
            while (ScanStr(&option_name) == 1) {
                const float rarity_weight = ScanFloat(&success);
                if (!success || rarity_weight <= 0.0f) {
                    return 7544;
                }

                const int rarity = getItemRarity(option_name.c_str());
                if (rarity < 0 || rarity > 5 || weighted_rarities[rarity]) {
                    return 7547;
                }
                script.invest_rates[rarity] = rarity_weight;
                weighted_rarities[rarity] = true;
                ++rarity_weight_count;
            }
            if (!LastTypeIs("[/rarity weight]") ||
                rarity_weight_count != 6) {
                return 7548;
            }
            seen_rarity_weight = true;
        } else if (tag == "[bonus weight by equipment type]") {
            if (seen_equipment_weight) return 7549;
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
                    return 7550;
                }
            }
            if (ScanType(tag, true) != 1 ||
                tag != "[/bonus weight by equipment type]") {
                return 7551;
            }
            seen_equipment_weight = true;
        } else if (tag == "[equip level const]") {
            if (seen_min_grade) return 7569;
            script.invest_min_grade = ScanInt(&success);
            if (!success) {
                return 7570;
            }
            seen_min_grade = true;
        } else if (tag == "[purify material]") {
            if (seen_purify) return 7571;
            int item_idx = 0;
            int item_count = 0;
            if (!ReadPurifyMaterial(&item_idx, &item_count)) {
                return 7576;
            }
            script.purify_material_item_idx[0] = item_idx;
            script.purify_material_item_count[0] = item_count;
            seen_purify = true;
        } else if (tag == "[purify only material]") {
            if (seen_legacy_only_purify) return 7588;
            int item_idx = 0;
            int item_count = 0;
            if (!ReadPurifyMaterial(&item_idx, &item_count)) {
                return 7592;
            }
            script.purify_material_item_idx[1] = item_idx;
            script.purify_material_item_count[1] = item_count;
            seen_legacy_only_purify = true;
        } else if (tag == "[purify only cera material]") {
            if (seen_cera_purify) return 7594;
            int item_idx = 0;
            int item_count = 0;
            if (!ReadPurifyMaterial(&item_idx, &item_count)) {
                return 7595;
            }
            // 0828 keeps the Cera-only material in addition to the legacy
            // [purify only material].  Publish it in otherwise-unused ABI
            // reserve bytes so a PVF containing both tags retains both IDs.
            StoreCeraPurifyMaterial(script, item_idx, item_count);
            seen_cera_purify = true;
        } else if (tag == "[purify material for magi]") {
            if (seen_magi_purify) return 7596;
            int item_idx = 0;
            int item_count = 0;
            if (!ReadPurifyMaterial(&item_idx, &item_count)) {
                return 7598;
            }
            // Preserve the 0627 0x100-byte script ABI.  Bytes 4..11 of the
            // old reserved region are unused by the level weight at 0..3 and
            // the twelve equipment weights at 0x2c..0x5b.  The staged loader
            // swaps the whole region atomically, so Magi data publishes only
            // with the rest of AmplifyItem/AmplifyUpgrade.
            StoreMagiPurifyMaterial(script, item_idx, item_count);
            seen_magi_purify = true;
        } else if (tag == "[invest option]") {
            if (seen_invest) return 7600;
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
                if (!success || item_idx <= 0 || item_count <= 0) {
                    return 7623;
                }

                stInvestAmplifyOption_t option;
                std::memset(&option, 0, sizeof(option));
                option.ability_type = static_cast<int>(option_it->second);
                std::memcpy(&option.value, &item_count, sizeof(item_count));
                const std::pair<
                    __gnu_cxx::hash_map<unsigned long,
                                          stInvestAmplifyOption_t>::iterator,
                    bool>
                    inserted = script.invest_materials[0].insert(
                        std::make_pair(static_cast<unsigned long>(item_idx),
                                       option));
                if (!inserted.second) return 7623;
                ++invest_count;
            }
            if (!LastTypeIs("[/invest option]") || invest_count != 6) {
                return 7624;
            }
            seen_invest = true;
        } else if (tag == "[reinvest option]") {
            if (seen_reinvest) return 7630;
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
                if (!success || item_idx <= 0 || item_count <= 0) {
                    return 7650;
                }

                stInvestAmplifyOption_t option;
                std::memset(&option, 0, sizeof(option));
                option.ability_type = static_cast<int>(option_it->second);
                std::memcpy(&option.value, &item_count, sizeof(item_count));
                const std::pair<
                    __gnu_cxx::hash_map<unsigned long,
                                          stInvestAmplifyOption_t>::iterator,
                    bool>
                    inserted = script.invest_materials[1].insert(
                        std::make_pair(static_cast<unsigned long>(item_idx),
                                       option));
                if (!inserted.second) return 7650;
                ++reinvest_count;
            }
            if (!LastTypeIs("[/reinvest option]") || reinvest_count != 6) {
                return 7651;
            }
            seen_reinvest = true;
        } else if (tag == "[random invest upgrade option]") {
            if (seen_random_invest) return 7660;
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
                if (!success || item_idx <= 0 || item_count <= 0) {
                    return 7678;
                }

                stInvestAmplifyOption_t option;
                std::memset(&option, 0, sizeof(option));
                option.ability_type = static_cast<int>(option_it->second);
                std::memcpy(&option.value, &item_count, sizeof(item_count));
                const std::pair<
                    __gnu_cxx::hash_map<unsigned long,
                                          stInvestAmplifyOption_t>::iterator,
                    bool>
                    inserted = script.invest_materials[2].insert(
                        std::make_pair(static_cast<unsigned long>(item_idx),
                                       option));
                if (!inserted.second) return 7678;
                ++random_invest_count;
            }
            if (!LastTypeIs("[/random invest upgrade option]") ||
                random_invest_count != 7) {
                return 7679;
            }
            seen_random_invest = true;
        } else {
            return 7680;
        }
    }
    if (g_dnfLex.getLastToken() != DNF_TOK_EOF) return 7681;
    return seen_mapping && seen_level_weight && seen_rarity_rate &&
                   seen_option_data && seen_rarity_weight &&
                   seen_equipment_weight && seen_min_grade && seen_purify &&
                   seen_magi_purify && seen_cera_purify && seen_invest &&
                   seen_reinvest && seen_random_invest
               ? 0
               : 7682;
}
