#include "old_data/PVF/common/script_initializers.h"

#include "common/config.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/datamanager/script_file_list.h"
#include "old_data/PVF/creature/creature_script.h"
#include "old_data/PVF/equipment/equipment_script.h"
#include "common/manager/find_price_abi.h"
#include "old_game/player/core/character_script_growth_data.h"

#include <cstdio>
#include <cstring>
#include <string>

extern char g_szCharacterScriptBaseDirectory[256];
extern char g_szPassiveObjectScriptBaseDirectory[256];
extern char g_szEquipmentScriptBaseDirectory[256];
extern char g_szMapScriptBaseDirectory[256];
extern char g_szDungeonScriptBaseDirectory[256];
extern char g_questScriptBaseDirectory[256];
extern char g_szSkillScriptBaseDirectory[256];
extern char g_aiCharacterScriptBaseDirectory[256];
extern char g_eventCharacterScriptBaseDirectory[256];

extern STScriptFileList g_stCharacterScriptFileList;
extern STScriptFileList g_stPassiveObjectScriptFileList;
extern STScriptFileList g_stEquipmentScriptFileList;
extern STScriptFileList g_stMapScriptFileList;
extern STScriptFileList g_stDungeonScriptFileList;
extern STScriptFileList g_questScriptFileList;
extern STScriptFileList
    g_stSkillScriptFileList[oldsrv::config::kCharacterJobCount];
extern STScriptFileList g_aiCharacterScriptFileList;
extern STScriptFileList g_eventCharacterScriptFileList;

extern char g_PvpMissionScriptBaseDirectory[256];
extern STScriptFileList g_PvpMissionScriptFileList;
extern char g_pvpSkillTreeScriptBaseDirectory[256];
extern STScriptFileList g_pvpSkillTreeScriptFileList;

namespace {

std::string CanonicalizeOfficialRegistryPath(const std::string& path) {
    std::string canonical(path);
    for (std::string::size_type index = 0; index < canonical.size(); ++index) {
        unsigned char value =
            static_cast<unsigned char>(canonical[index]);
        if (value == static_cast<unsigned char>('\\')) {
            value = static_cast<unsigned char>('/');
        } else if (value >= static_cast<unsigned char>('A') &&
                   value <= static_cast<unsigned char>('Z')) {
            value = static_cast<unsigned char>(
                value + static_cast<unsigned char>('a' - 'A'));
        }
        canonical[index] = static_cast<char>(value);
    }
    return canonical;
}

bool SameOfficialRegistryPath(const std::string& actual,
                              const char* expected_path) {
    return expected_path != 0 &&
           CanonicalizeOfficialRegistryPath(actual) ==
               CanonicalizeOfficialRegistryPath(expected_path);
}

enum SouthernDaleResourceState {
    SOUTHERN_DALE_RESOURCE_MISSING = 0,
    SOUTHERN_DALE_RESOURCE_VALID = 1,
    SOUTHERN_DALE_RESOURCE_MALFORMED = 2
};

SouthernDaleResourceState InspectOfficialSouthernDaleResource(
    const char* base_directory) {
    using namespace oldsrv::official_registry;

    if (loadRDARScriptFile(base_directory,
                           SouthernDaleDungeonScriptPath()) != 1) {
        return SOUTHERN_DALE_RESOURCE_MISSING;
    }

    bool valid = true;
    bool saw_power_dungeon_type = false;
    bool saw_powerwar_marker = false;
    while (true) {
        const DNFLEX_TOKEN token = g_dnfLex.getToken();
        if (token == DNF_TOK_EOF) {
            break;
        }
        if (token == DNF_TOK_ERROR) {
            valid = false;
            break;
        }
        if (token != DNF_TOK_TYPE) {
            continue;
        }

        const char* tag_value = g_dnfLex.getType();
        const std::string tag(tag_value != 0 ? tag_value : "");
        if (tag == "[dungeon type]") {
            if (g_dnfLex.getToken() != DNF_TOK_STRING) {
                valid = false;
                break;
            }
            const char* dungeon_type = g_dnfLex.getString();
            if (dungeon_type == 0 ||
                std::strcmp(dungeon_type, "power dungeon") != 0) {
                valid = false;
                break;
            }
            saw_power_dungeon_type = true;
        } else if (tag == "[powerwar dungeon]") {
            const DNFLEX_TOKEN enabled_token = g_dnfLex.getToken();
            int enabled = 0;
            if (enabled_token == DNF_TOK_DECIMAL) {
                enabled = g_dnfLex.getDecimal();
            } else if (enabled_token == DNF_TOK_HEXADECIMAL) {
                enabled = g_dnfLex.getHexDecimal();
            } else {
                valid = false;
                break;
            }
            if (enabled != 1) {
                valid = false;
                break;
            }
            saw_powerwar_marker = true;
        }
    }
    unloadRDARScriptFile();
    return valid && saw_power_dungeon_type && saw_powerwar_marker
               ? SOUTHERN_DALE_RESOURCE_VALID
               : SOUTHERN_DALE_RESOURCE_MALFORMED;
}

bool RestoreOfficialSouthernDaleRegistry() {
    using namespace oldsrv::official_registry;

    const STScriptFileList::const_iterator existing =
        g_stDungeonScriptFileList.find(kSouthernDaleDungeonIndex);
    if (existing != g_stDungeonScriptFileList.end()) {
        if (!SameOfficialRegistryPath(existing->second,
                                      SouthernDaleDungeonScriptPath())) {
            std::printf("Southern Dale Registry Error: "
                        "dungeon=%d resource=%s\n",
                        kSouthernDaleDungeonIndex,
                        existing->second.c_str());
            return false;
        }
        const SouthernDaleResourceState registered_resource_state =
            InspectOfficialSouthernDaleResource(
                g_szDungeonScriptBaseDirectory);
        if (registered_resource_state != SOUTHERN_DALE_RESOURCE_VALID) {
            std::printf("Southern Dale Script Error: %s\n",
                        SouthernDaleDungeonScriptPath());
            return false;
        }
        return true;
    }
    for (STScriptFileList::const_iterator entry =
             g_stDungeonScriptFileList.begin();
         entry != g_stDungeonScriptFileList.end(); ++entry) {
        if (SameOfficialRegistryPath(entry->second,
                                     SouthernDaleDungeonScriptPath())) {
            std::printf("Southern Dale Registry Error: "
                        "resource=%s already owned by dungeon=%d\n",
                        entry->second.c_str(),
                        entry->first);
            return false;
        }
    }
    const SouthernDaleResourceState resource_state =
        InspectOfficialSouthernDaleResource(g_szDungeonScriptBaseDirectory);
    if (resource_state == SOUTHERN_DALE_RESOURCE_MISSING) {
        // Older PVFs legitimately predate Southern Dale.  Preserve that
        // layout; a present-but-invalid 0828 resource is handled below.
        return true;
    }
    if (resource_state != SOUTHERN_DALE_RESOURCE_VALID) {
        std::printf("Southern Dale Script Error: %s\n",
                    SouthernDaleDungeonScriptPath());
        return false;
    }

    const std::pair<STScriptFileList::iterator, bool> inserted =
        g_stDungeonScriptFileList.insert(std::make_pair(
            kSouthernDaleDungeonIndex,
            std::string(SouthernDaleDungeonScriptPath())));
    if (!inserted.second) {
        return false;
    }

    std::printf("SOUTHERN_DALE_0828_REGISTRY_FALLBACK: "
                "dungeon=%d resource=%s\n",
                kSouthernDaleDungeonIndex,
                SouthernDaleDungeonScriptPath());
    return true;
}

}  // namespace

// IDA_EA: 0x8A0CC3C
// IDA_NAME: _Z19InitCharacterScriptPKcS0_S0_S0_
// DEMANGLED: InitCharacterScript(char const*,char const*,char const*,char const*)
int InitCharacterScript(const char* src,
                        const char* list_file,
                        const char* exp_table,
                        const char* pvp_price_table) {
    strncpy(g_szCharacterScriptBaseDirectory, src, 0x100u);
    if (g_szCharacterScriptBaseDirectory
                [strlen(g_szCharacterScriptBaseDirectory) - 1] != '\\' &&
        g_szCharacterScriptBaseDirectory
                [strlen(g_szCharacterScriptBaseDirectory) - 1] != '/') {
        const size_t length = strlen(g_szCharacterScriptBaseDirectory);
        memcpy(g_szCharacterScriptBaseDirectory + length, "\\", 2u);
    }
    if (loadRDARScriptListFile(&g_stCharacterScriptFileList,
                               g_szCharacterScriptBaseDirectory,
                               list_file) != 1) {
        return 0;
    }

    for (int index = 0; index <= 199; ++index) {
        STCharacterScript::naExpTable[index] = 0;
    }
    if (loadRDARScriptFile(g_szCharacterScriptBaseDirectory, exp_table) != 1) {
        return 0;
    }
    for (int index = 0; index <= 199; ++index) {
        STCharacterScript::naExpTable[index] =
            ScanInt(static_cast<bool*>(0));
    }

    if (pvp_price_table != 0) {
        for (int index = 0; index <= 199; ++index) {
            STCharacterScript::pvpPrices_[index] = 0;
        }
        if (loadRDARScriptFile(g_szCharacterScriptBaseDirectory,
                               pvp_price_table) != 1) {
            return 0;
        }
        for (int index = 0; index <= 199; ++index) {
            STCharacterScript::pvpPrices_[index] =
                ScanInt(static_cast<bool*>(0));
        }
    }
    return 1;
}

// IDA_EA: 0x8A29EAD
// IDA_NAME: _Z23InitPassiveObjectScriptPKcS0_
// DEMANGLED: InitPassiveObjectScript(char const*,char const*)
int InitPassiveObjectScript(const char* src, const char* list_file) {
    strncpy(g_szPassiveObjectScriptBaseDirectory, src, 0x100u);
    if (g_szPassiveObjectScriptBaseDirectory
                [strlen(g_szPassiveObjectScriptBaseDirectory) - 1] != '\\' &&
        g_szPassiveObjectScriptBaseDirectory
                [strlen(g_szPassiveObjectScriptBaseDirectory) - 1] != '/') {
        const size_t length = strlen(g_szPassiveObjectScriptBaseDirectory);
        memcpy(g_szPassiveObjectScriptBaseDirectory + length, "\\", 2u);
    }
    return loadRDARScriptListFile(&g_stPassiveObjectScriptFileList,
                                  g_szPassiveObjectScriptBaseDirectory,
                                  list_file) == 1;
}

// IDA_EA: 0x8991601
// IDA_NAME: _Z19InitEquipmentScriptPKcS0_S0_
// DEMANGLED: InitEquipmentScript(char const*,char const*,char const*)
int InitEquipmentScript(const char* src,
                        const char* list_file,
                        const char* price_table) {
    strncpy(g_szEquipmentScriptBaseDirectory, src, 0x100u);
    if (g_szEquipmentScriptBaseDirectory
                [strlen(g_szEquipmentScriptBaseDirectory) - 1] != '\\' &&
        g_szEquipmentScriptBaseDirectory
                [strlen(g_szEquipmentScriptBaseDirectory) - 1] != '/') {
        const size_t length = strlen(g_szEquipmentScriptBaseDirectory);
        memcpy(g_szEquipmentScriptBaseDirectory + length, "\\", 2u);
    }
    if (loadRDARScriptListFile(&g_stEquipmentScriptFileList,
                               g_szEquipmentScriptBaseDirectory,
                               list_file) != 1) {
        return 0;
    }
    if (loadRDARScriptFile(g_szEquipmentScriptBaseDirectory, price_table) !=
        1) {
        return 0;
    }

    for (int row = 0; row <= 29; ++row) {
        for (int column = 0; column <= 99; ++column) {
            STEquipmentScript::priceTable_[row * 100 + column] = 0;
        }
    }

    std::string tag;
    ScanType(tag, true);
    for (int index = 0; index <= 2; ++index) {
        itemPriceRates_[index] = ScanInt(static_cast<bool*>(0));
    }
    ScanType(tag, true);
    if (tag == "[repair cost]") {
        itemRepairCost_ = ScanFloat(static_cast<bool*>(0));
    }
    for (int row = 0; row <= 29 && ScanType(tag, true) == 1; ++row) {
        for (int column = 0; column <= 99; ++column) {
            STEquipmentScript::priceTable_[row * 100 + column] =
                ScanInt(static_cast<bool*>(0));
        }
    }
    return 1;
}

// IDA_EA: 0x89DCE98
// IDA_NAME: _Z13InitMapScriptPKcS0_
// DEMANGLED: InitMapScript(char const*,char const*)
int InitMapScript(const char* src, const char* list_file) {
    strncpy(g_szMapScriptBaseDirectory, src, 0x100u);
    if (g_szMapScriptBaseDirectory[strlen(g_szMapScriptBaseDirectory) - 1] !=
            '\\' &&
        g_szMapScriptBaseDirectory[strlen(g_szMapScriptBaseDirectory) - 1] !=
            '/') {
        const size_t length = strlen(g_szMapScriptBaseDirectory);
        memcpy(g_szMapScriptBaseDirectory + length, "\\", 2u);
    }
    return loadRDARScriptListFile(&g_stMapScriptFileList,
                                  g_szMapScriptBaseDirectory,
                                  list_file) == 1;
}

// IDA_EA: 0x88E5E3C
// IDA_NAME: _Z17InitDungeonScriptPcS_
// DEMANGLED: InitDungeonScript(char*,char*)
int InitDungeonScript(char* src, char* list_file) {
    strncpy(g_szDungeonScriptBaseDirectory, src, 0x100u);
    if (g_szDungeonScriptBaseDirectory
                [strlen(g_szDungeonScriptBaseDirectory) - 1] != '\\' &&
        g_szDungeonScriptBaseDirectory
                [strlen(g_szDungeonScriptBaseDirectory) - 1] != '/') {
        const size_t length = strlen(g_szDungeonScriptBaseDirectory);
        memcpy(g_szDungeonScriptBaseDirectory + length, "\\", 2u);
    }
    if (loadRDARScriptListFile(&g_stDungeonScriptFileList,
                               g_szDungeonScriptBaseDirectory,
                               list_file) != 1) {
        return 0;
    }
    return RestoreOfficialSouthernDaleRegistry() ? 1 : 0;
}

// IDA_EA: 0x8A61642
// IDA_NAME: _Z15initQuestScriptPKcS0_
// DEMANGLED: initQuestScript(char const*,char const*)
int initQuestScript(const char* src, const char* list_file) {
    strncpy(g_questScriptBaseDirectory, src, 0x100u);
    if (g_questScriptBaseDirectory[strlen(g_questScriptBaseDirectory) - 1] !=
            '\\' &&
        g_questScriptBaseDirectory[strlen(g_questScriptBaseDirectory) - 1] !=
            '/') {
        const size_t length = strlen(g_questScriptBaseDirectory);
        memcpy(g_questScriptBaseDirectory + length, "\\", 2u);
    }
    return loadRDARScriptListFile(&g_questScriptFileList,
                                  g_questScriptBaseDirectory,
                                  list_file) == 1;
}

// IDA_EA: 0x8A97438
// IDA_NAME: _Z15InitSkillScriptPKcS0_
// DEMANGLED: InitSkillScript(char const*,char const*)
int InitSkillScript(const char* src, const char* list_file) {
    strncpy(g_szSkillScriptBaseDirectory, src, 0x100u);
    if (g_szSkillScriptBaseDirectory
                [strlen(g_szSkillScriptBaseDirectory) - 1] != '\\' &&
        g_szSkillScriptBaseDirectory
                [strlen(g_szSkillScriptBaseDirectory) - 1] != '/') {
        const size_t length = strlen(g_szSkillScriptBaseDirectory);
        memcpy(g_szSkillScriptBaseDirectory + length, "\\", 2u);
    }

    STScriptFileList script_file_list;
    if (loadRDARScriptListFile(&script_file_list,
                               g_szSkillScriptBaseDirectory,
                               list_file) != 1) {
        return 0;
    }

    for (STScriptFileList::iterator it = script_file_list.begin();
         it != script_file_list.end();
         ++it) {
        if (it->first >= 0 &&
            it->first < oldsrv::config::kCharacterJobCount &&
            loadRDARScriptListFile(&g_stSkillScriptFileList[it->first],
                                   g_szSkillScriptBaseDirectory,
                                   it->second.c_str()) != 1) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x88C98D4
// IDA_NAME: _Z21initAICharacterScriptPKcS0_
// DEMANGLED: initAICharacterScript(char const*,char const*)
int initAICharacterScript(const char* src, const char* filename) {
    strncpy(g_aiCharacterScriptBaseDirectory, src, 0x100u);
    if (g_aiCharacterScriptBaseDirectory
                [strlen(g_aiCharacterScriptBaseDirectory) - 1] != '\\' &&
        g_aiCharacterScriptBaseDirectory
                [strlen(g_aiCharacterScriptBaseDirectory) - 1] != '/') {
        const size_t length = strlen(g_aiCharacterScriptBaseDirectory);
        memcpy(g_aiCharacterScriptBaseDirectory + length, "\\", 2u);
    }
    return loadRDARScriptListFile(&g_aiCharacterScriptFileList,
                                  g_aiCharacterScriptBaseDirectory,
                                  filename) == 1;
}

// IDA_EA: 0x895AE5C
// IDA_NAME: _Z24initEventCharacterScriptPcS_
// DEMANGLED: initEventCharacterScript(char*,char*)
int initEventCharacterScript(char* src, char* filename) {
    strncpy(g_eventCharacterScriptBaseDirectory, src, 0x100u);
    if (g_eventCharacterScriptBaseDirectory
                [strlen(g_eventCharacterScriptBaseDirectory) - 1] != '\\' &&
        g_eventCharacterScriptBaseDirectory
                [strlen(g_eventCharacterScriptBaseDirectory) - 1] != '/') {
        const size_t length = strlen(g_eventCharacterScriptBaseDirectory);
        memcpy(g_eventCharacterScriptBaseDirectory + length, "/", 2u);
    }
    return loadRDARScriptListFile(&g_eventCharacterScriptFileList,
                                  g_eventCharacterScriptBaseDirectory,
                                  filename) == 1;
}

// IDA_EA: 0x8A5AAD5
// IDA_NAME: _Z20initPvpMissionSystemPcS_
// DEMANGLED: initPvpMissionSystem(char*,char*)
int initPvpMissionSystem(char* src, char* filename) {
    strncpy(g_PvpMissionScriptBaseDirectory, src, 0xFFu);
    if (g_PvpMissionScriptBaseDirectory
                [strlen(g_PvpMissionScriptBaseDirectory) - 1] != '\\' &&
        g_PvpMissionScriptBaseDirectory
                [strlen(g_PvpMissionScriptBaseDirectory) - 1] != '/') {
        const size_t length = strlen(g_PvpMissionScriptBaseDirectory);
        memcpy(g_PvpMissionScriptBaseDirectory + length, "\\", 2u);
    }
    return loadRDARScriptListFile(&g_PvpMissionScriptFileList,
                                  g_PvpMissionScriptBaseDirectory,
                                  filename) == 1;
}

// IDA_EA: 0x8A5CD6C
// IDA_NAME: _Z22initPvPSkillTreeScriptPcS_
// DEMANGLED: initPvPSkillTreeScript(char*,char*)
int initPvPSkillTreeScript(char* src, char* filename) {
    strncpy(g_pvpSkillTreeScriptBaseDirectory, src, 0xFFu);
    if (g_pvpSkillTreeScriptBaseDirectory
                [strlen(g_pvpSkillTreeScriptBaseDirectory) - 1] != '\\' &&
        g_pvpSkillTreeScriptBaseDirectory
                [strlen(g_pvpSkillTreeScriptBaseDirectory) - 1] != '/') {
        const size_t length = strlen(g_pvpSkillTreeScriptBaseDirectory);
        memcpy(g_pvpSkillTreeScriptBaseDirectory + length, "\\", 2u);
    }
    return loadRDARScriptListFile(&g_pvpSkillTreeScriptFileList,
                                  g_pvpSkillTreeScriptBaseDirectory,
                                  filename) == 1;
}
