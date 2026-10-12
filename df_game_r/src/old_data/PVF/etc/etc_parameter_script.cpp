#include "old_data/PVF/etc/etc_parameter_script.h"

#include "old_data/PVF/common/rdar_script.h"
#include "infra/log/format/trace.h"

#include <string>

// IDA_EA: 0x89136A4
// IDA_NAME: _Z27importExperienceItemMapInfoRSt3mapIiiSt4lessIiESaISt4pairIKiiEEEb
// DEMANGLED: importExperienceItemMapInfo(std::map<int,int,std::less<int>,std::allocator<std::pair<int const,int>>> &,bool)
int importExperienceItemMapInfo(std::map<int, int>& table, bool reverse) {
    table.clear();
    if (!loadRDARScriptFile("Etc/ItemDictionary/",
                            "(R)baseItemDictionaryTable.etc")) {
        return 0;
    }

    std::string tag;
    if (ScanType(tag, true) != 1) {
        return 0;
    }
    if (tag != "[base table]") {
        return 0;
    }

    bool success = false;
    while (1) {
        const int item_id = ScanInt(&success);
        if (!success) {
            break;
        }

        const int map_index = ScanInt(&success);
        if (!success) {
            return 0;
        }
        if (map_index > 35000) {
            LogManager::logFormat(
                0,
                "../RDARScriptEtcParameter.cpp",
                "importExperienceItemMapInfo",
                0x30B8,
                "ItemDictionary mappingTable MAX INDEX(40000) WARNING READY TO DB_TOOL");
            return 0;
        }

        if (reverse) {
            table.insert(std::make_pair(map_index, item_id));
        } else {
            table.insert(std::make_pair(item_id, map_index));
        }
    }

    return 1;
}

// IDA_EA: 0x8913C7E
// IDA_NAME: _Z25importUltimateDungeonListPKcRSt3setIiSt4lessIiESaIiEE
// DEMANGLED: importUltimateDungeonList(char const*,std::set<int,std::less<int>,std::allocator<int>> &)
int importUltimateDungeonList(const char* filename, std::set<int>& list) {
    if (!loadRDARScriptFile("", filename)) {
        LogManager::logFormat(0,
                              "../RDARScriptEtcParameter.cpp",
                              "importUltimateDungeonList",
                              0x3209,
                              "error : %s script Data ",
                              filename);
        return 0;
    }

    std::string tag;
    list.clear();
    while (ScanType(tag, true) == 1) {
        bool success = false;
        if (tag == "[apply ultimate]") {
            while (1) {
                const int dungeon_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                list.insert(dungeon_id);
            }
        } else if (tag == "[/apply ultimate]") {
            break;
        }
    }
    return 1;
}

// IDA_EA: 0x8917848
// IDA_NAME: _Z23importHiddenNpcListInfoPKcRSt6vectorIiSaIiEE
// DEMANGLED: importHiddenNpcListInfo(char const*,std::vector<int,std::allocator<int>> &)
int importHiddenNpcListInfo(const char* filename, std::vector<int>& list) {
    int result = 0;
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    bool end_tag_found = false;
    list.clear();
    while (!end_tag_found) {
        if (ScanType(tag, true) != 1) {
            result = 0;
            break;
        }
        if (tag != "[hidden npc]") {
            result = 0;
            break;
        }

        while (1) {
            bool is_type = false;
            bool success = false;
            std::string value_text;
            const int value =
                ScanTypeOrInt(is_type, value_text, success, true);
            if (!success) {
                result = 0;
                break;
            }

            if (is_type) {
                if (value_text == "[/hidden npc]") {
                    end_tag_found = true;
                    result = 1;
                } else {
                    result = 0;
                }
                break;
            }

            list.push_back(value);
        }
        if (!end_tag_found) {
            break;
        }
    }

    return result;
}

// IDA_EA: 0x890A33E
// IDA_NAME: _Z32ImportDifficultPenaltyInfoScriptPKcR20DifficultPenaltyInfo
// DEMANGLED: ImportDifficultPenaltyInfoScript(char const*,DifficultPenaltyInfo &)
int ImportDifficultPenaltyInfoScript(const char* filename,
                                     DifficultPenaltyInfo& info) {
    if (!loadRDARScriptFile("", filename)) {
        return 7131;
    }

    std::string tag;
    while (ScanType(tag, true) == 1) {
        bool success = true;
        if (tag == "[COMMON TRAP EXPLOSION]") {
            for (int index = 0; index <= 4; ++index) {
                const int value = ScanInt(&success);
                if (success) {
                    info.common_trap_explosion[index] = value;
                }
            }
        }
    }
    return 0;
}

// IDA_EA: 0x890BB25
// IDA_NAME: _Z20importItmeGroupTablePKcS0_RSt3mapISsiSt4lessISsESaISt4pairIKSsiEEE
// DEMANGLED: importItmeGroupTable(char const*,char const*,std::map<std::string,int,std::less<std::string>,std::allocator<std::pair<std::string const,int>>> &)
int importItmeGroupTable(const char* dir,
                         const char* filename,
                         std::map<std::string, int>& table) {
    if (!loadRDARScriptFile(dir, filename)) {
        return 7858;
    }

    std::string name;
    bool success = false;
    while (1) {
        const int item_group = ScanInt(&success);
        if (!success) {
            break;
        }
        if (ScanStr(&name) != 1) {
            return 7869;
        }
        table.insert(std::make_pair(name, item_group));
    }
    return 0;
}
