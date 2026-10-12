#include "old_data/PVF/etc/pcbang_dimension_script.h"

#include "common/manager/global_tools.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/pvp_mission/fair_pvp_equipment_script.h"

#include <stdint.h>

#include <string>
#include <utility>
#include <vector>


// IDA_EA: 0x88945E2
// IDA_NAME: _Z11tokenizeStrRKSsRSt6vectorISsSaISsEES0_
// DEMANGLED: tokenizeStr(std::string const&,std::vector<std::string>&,std::string const&)
int tokenizeStr(const std::string& text,
                std::vector<std::string>& tokens,
                const std::string& separator) {
    std::string::size_type begin = text.find_first_not_of(separator, 0);
    std::string::size_type end = text.find_first_of(separator, begin);
    tokens.clear();
    while (end != std::string::npos || begin != std::string::npos) {
        tokens.push_back(text.substr(begin, end - begin));
        begin = text.find_first_not_of(separator, end);
        end = text.find_first_of(separator, begin);
    }
    return end != std::string::npos || begin != std::string::npos;
}

// IDA_EA: 0x8915606
// IDA_NAME: _Z27importPcBangEquipmentRendarPKcP20PcBangItemRentarData
// DEMANGLED: importPcBangEquipmentRendar(char const*,PcBangItemRentarData*)
int importPcBangEquipmentRendar(const char* filename,
                                PcBangItemRentarData* data) {
    if (!data) {
        return 0;
    }
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    std::string text;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[section]") {
            while (1) {
                EventRewardSection section;
                section.byte_00 = static_cast<uint8_t>(ScanInt(&success));
                if (!success) {
                    break;
                }
                section.byte_01 =
                    static_cast<uint8_t>(ScanInt(static_cast<bool*>(0)));
                section.byte_02 =
                    static_cast<uint8_t>(ScanInt(static_cast<bool*>(0)));
                const int job_count = ScanInt(static_cast<bool*>(0));
                for (int index = 0; index < job_count; ++index) {
                    const uint8_t job = static_cast<uint8_t>(
                        ScanInt(static_cast<bool*>(0)));
                    section.values.push_back(job);
                }
                data->sections.push_back(section);
            }
        } else if (tag == "[group]") {
            if (ScanStr(&text) != 1) {
                return 0;
            }
            const int character_job = ::getCharacterJob(text.c_str());
            const int rank = ScanInt(static_cast<bool*>(0));
            const int package_key =
                makeFairPVPEquipmentPackageKey(character_job, rank);
            std::vector<std::vector<EventRewardItemInfo> > package_list;

            while (ScanType(tag, true) == 1) {
                if (tag == "[package selection]") {
                    std::vector<EventRewardItemInfo> package;
                    int item_index = 0;
                    while (ScanInt(&item_index)) {
                        EventRewardItemInfo item;
                        item.first_dword = static_cast<uint32_t>(item_index);
                        item.word_08 = static_cast<uint16_t>(
                            ScanInt(static_cast<bool*>(0)));
                        item.second_dword = static_cast<uint32_t>(
                            ScanInt(static_cast<bool*>(0)));
                        ScanStr(&text);
                        const std::string grow_types(text.c_str());
                        std::vector<std::string> tokens;
                        const std::string separator(",");
                        tokenizeStr(grow_types, tokens, separator);
                        for (std::vector<std::string>::iterator it =
                                 tokens.begin();
                             it != tokens.end();
                             ++it) {
                            item.values.push_back(
                                ::getGrowTypeIndex(it->c_str()));
                        }
                        package.push_back(item);
                    }
                    package_list.push_back(package);
                } else if (tag == "[/group]") {
                    data->selection_groups.insert(
                        std::make_pair(package_key, package_list));
                    break;
                }
            }
        }
    }

    return 1;
}

// IDA_EA: 0x89160F0
// IDA_NAME: _Z29importDimensionActivationDataPKcP23DimensionActivationData
// DEMANGLED: importDimensionActivationData(char const*,DimensionActivationData*)
int importDimensionActivationData(const char* filename,
                                  DimensionActivationData* data) {
    if (!data) {
        return 0;
    }
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    std::string text;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[play day count]") {
            data->active_flag =
                static_cast<uint32_t>(ScanInt(&success));
        } else if (tag == "[coin info]") {
            while (ScanStr(&text) == 1) {
                int difficulty = -1;
                if (text == "[easy]") {
                    difficulty = 0;
                } else if (text == "[medium]") {
                    difficulty = 1;
                } else if (text == "[hard]") {
                    difficulty = 2;
                } else if (text == "[ultimate]") {
                    difficulty = 3;
                }
                DimensionCoinInfo coin_info;
                coin_info.item_id =
                    static_cast<uint32_t>(ScanInt(&success));
                const int by_character = ScanInt(&success);
                coin_info.by_character =
                    static_cast<uint8_t>(by_character != 0);
                data->coin_infos.insert(
                    std::make_pair(difficulty, coin_info));
            }
        } else if (tag == "[section]") {
            while (1) {
                EventRewardSection section;
                section.byte_00 = static_cast<uint8_t>(ScanInt(&success));
                if (!success) {
                    break;
                }
                section.byte_01 =
                    static_cast<uint8_t>(ScanInt(static_cast<bool*>(0)));
                section.byte_02 =
                    static_cast<uint8_t>(ScanInt(static_cast<bool*>(0)));
                const int job_count = ScanInt(static_cast<bool*>(0));
                for (int index = 0; index < job_count; ++index) {
                    const uint8_t job = static_cast<uint8_t>(
                        ScanInt(static_cast<bool*>(0)));
                    section.values.push_back(job);
                }
                data->reward_data.sections.push_back(section);
            }
        } else if (tag == "[select group]") {
            if (ScanStr(&text) != 1) {
                return 0;
            }
            const int character_job = ::getCharacterJob(text.c_str());
            const int rank = ScanInt(static_cast<bool*>(0));
            const int package_key =
                makeFairPVPEquipmentPackageKey(character_job, rank);
            std::vector<std::vector<EventRewardItemInfo> > package_list;

            while (ScanType(tag, true) == 1) {
                if (tag == "[package selection]") {
                    std::vector<EventRewardItemInfo> package;
                    int item_index = 0;
                    while (ScanInt(&item_index)) {
                        EventRewardItemInfo item;
                        item.first_dword = static_cast<uint32_t>(item_index);
                        item.word_08 = static_cast<uint16_t>(
                            ScanInt(static_cast<bool*>(0)));
                        item.second_dword = static_cast<uint32_t>(
                            ScanInt(static_cast<bool*>(0)));
                        ScanStr(&text);
                        const std::string grow_types(text.c_str());
                        std::vector<std::string> tokens;
                        const std::string separator(",");
                        tokenizeStr(grow_types, tokens, separator);
                        for (std::vector<std::string>::iterator it =
                                 tokens.begin();
                             it != tokens.end();
                             ++it) {
                            item.values.push_back(
                                ::getGrowTypeIndex(it->c_str()));
                        }
                        package.push_back(item);
                    }
                    package_list.push_back(package);
                } else if (tag == "[/select group]") {
                    data->reward_data.select_groups.insert(
                        std::make_pair(package_key, package_list));
                    break;
                }
            }
        } else if (tag == "[give group]") {
            if (ScanStr(&text) != 1) {
                return 0;
            }
            const int character_job = ::getCharacterJob(text.c_str());
            while (ScanType(tag, true) == 1) {
                if (tag == "[/give group]") {
                    break;
                }
                if (tag != "[grow type]") {
                    continue;
                }
                if (ScanStr(&text) != 1) {
                    break;
                }
                const int grow_type = ::getGrowTypeIndex(text.c_str());
                std::vector<std::vector<EventRewardItemInfo> > package_list;
                while (ScanType(tag, true) == 1) {
                    if (tag == "[/grow type]") {
                        data->reward_data.give_groups.insert(std::make_pair(
                            std::make_pair(character_job, grow_type),
                            package_list));
                        package_list.clear();
                        break;
                    }
                    if (tag != "[package selection]") {
                        continue;
                    }
                    ScanInt(&success);
                    std::vector<EventRewardItemInfo> package;
                    int item_index = 0;
                    EventRewardItemInfo item;
                    while (ScanInt(&item_index)) {
                        item.first_dword =
                            static_cast<uint32_t>(item_index);
                        item.word_08 = static_cast<uint16_t>(ScanInt(
                            &success));
                        item.second_dword =
                            static_cast<uint32_t>(ScanInt(&success));
                        package.push_back(item);
                        item.clear();
                    }
                    package_list.push_back(package);
                    package.clear();
                }
            }
        }
    }

    return 1;
}
