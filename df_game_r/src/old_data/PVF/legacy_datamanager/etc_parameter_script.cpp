#include "old_data/PVF/data/datamanager/etc_parameter_script.h"

#include "old_game/world/region/world_value_types.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "infra/log/format/trace.h"

#include <string.h>

#include <map>
#include <string>
#include <vector>


typedef char SecurityReward_Size_Must_Be_0x30[
    (sizeof(SecurityReward) == 0x30) ? 1 : -1];
typedef char DifficultPenaltyInfo_Size_Must_Be_0x14[
    (sizeof(DifficultPenaltyInfo) == 0x14) ? 1 : -1];

extern int PVPMAP_BIG_IMAGE[20] __asm__("PVPMAP_BIG_IMAGE");
extern int PVPMAP_SMALL_IMAGE[20] __asm__("PVPMAP_SMALL_IMAGE");
extern unsigned char PVPMAP_DEATHMATCH_MODE[20]
    __asm__("PVPMAP_DEATHMATCH_MODE");
extern unsigned char PVPMAP_ONLY_TOURNAMENT_EXPOSURE[20]
    __asm__("PVPMAP_ONLY_TOURNAMENT_EXPOSURE");
extern unsigned char PVPMAP_ONLY_NORMAL_EXPOSURE[20]
    __asm__("PVPMAP_ONLY_NORMAL_EXPOSURE");
extern unsigned char PVPMAP_ONLY_TOURNAMENT_RANDOM[20]
    __asm__("PVPMAP_ONLY_TOURNAMENT_RANDOM");

// IDA_EA: 0x89113EC
// IDA_NAME: _Z20ImportSecurityRewardP14SecurityReward
// DEMANGLED: ImportSecurityReward(SecurityReward *)
int ImportSecurityReward(SecurityReward* reward) {
    if (!loadRDARScriptFile("", "Etc/SecurityReward.etc")) {
        LogManager::logFormat(0,
                              "../RDARScriptEtcParameter.cpp",
                              "ImportSecurityReward",
                              0x2C36,
                              "CRITICAL ERROR : SecurityReward faild!");
        return 0;
    }

    int* values = reinterpret_cast<int*>(reward);
    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[GoblinPad Reward]") {
            ScanInt(values + 0);
            ScanInt(values + 1);
            ScanInt(values + 2);
            ScanInt(values + 3);
        } else if (tag == "[SecurityCard Reward]") {
            ScanInt(values + 4);
            ScanInt(values + 5);
            ScanInt(values + 6);
            ScanInt(values + 7);
        } else if (tag == "[OTP Reward]") {
            ScanInt(values + 8);
            ScanInt(values + 9);
            ScanInt(values + 10);
            ScanInt(values + 11);
        }
    }
    return 1;
}

// IDA_EA: 0x88FDB69
// IDA_NAME: _Z22importGoldLimitByLevelPj
// DEMANGLED: importGoldLimitByLevel(unsigned int *)
int importGoldLimitByLevel(unsigned int* gold_limit_by_level) {
    memset(gold_limit_by_level, 0, 0x190u);
    if (!loadRDARScriptFile("", "Etc/GoldLimitByLevel.etc")) {
        LogManager::logFormat(0,
                              "../RDARScriptEtcParameter.cpp",
                              "importGoldLimitByLevel",
                              0xE3,
                              "CRITICAL ERROR : ");
        return 0;
    }

    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[gold limit from level]") {
            for (int index = 0; index <= 99; ++index) {
                const int level = ScanInt(&success);
                if (!success) {
                    break;
                }
                gold_limit_by_level[level] =
                    static_cast<unsigned int>(ScanInt(static_cast<bool*>(0)));
            }
        }
    }
    return 1;
}

// IDA_EA: 0x8919127
// IDA_NAME: _Z22importQuestPieceScriptPKcRSt3mapIiiSt4lessIiESaISt4pairIKiiEEE
// DEMANGLED: importQuestPieceScript(char const*,std::map<int,int,std::less<int>,std::allocator<std::pair<int const,int>>> &)
int importQuestPieceScript(const char* filename, std::map<int, int>& table) {
    if (loadRDARScriptFile("", filename)) {
        std::string tag;
        bool success = false;
        while (ScanType(tag, true) == 1) {
            if (tag == "[piece quest list]") {
                while (1) {
                    const int quest_id = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    const int piece_count = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    table.insert(std::make_pair(quest_id, piece_count));
                }
            }
        }
    }
    return 0;
}

// IDA_EA: 0x890A427
// IDA_NAME: _Z25loadPvpMapBasicDataScriptPKcPSt6vectorI11PVPMAP_DATASaIS2_EE
// DEMANGLED: loadPvpMapBasicDataScript(char const*,std::vector<PVPMAP_DATA,std::allocator<PVPMAP_DATA>> *)
int loadPvpMapBasicDataScript(const char* filename,
                              std::vector<PVPMAP_DATA>* pvp_map_data) {
    if (!loadRDARScriptFile("", filename)) {
        LogManager::logFormat(0,
                              "../RDARScriptEtcParameter.cpp",
                              "loadPvpMapBasicDataScript",
                              0x1C00,
                              "error : %s ",
                              filename);
        return 0;
    }

    if (pvp_map_data != 0) {
        pvp_map_data->clear();
    }

    std::string tag;
    int map_index = 0;
    bool success = false;
    int row = 0;
    while (ScanType(tag, true) == 1) {
        if (tag == "[pvp map basic data]") {
            ScanInt(static_cast<bool*>(0));
            PVPMAP_BIG_IMAGE[row] = ScanInt(static_cast<bool*>(0));
            PVPMAP_SMALL_IMAGE[row] = ScanInt(static_cast<bool*>(0));
            PVPMAP_DEATHMATCH_MODE[row] =
                ScanInt(static_cast<bool*>(0)) != 0 ? 1 : 0;
            PVPMAP_ONLY_TOURNAMENT_EXPOSURE[row] =
                ScanInt(static_cast<bool*>(0)) != 0 ? 1 : 0;
            PVPMAP_ONLY_NORMAL_EXPOSURE[row] =
                ScanInt(static_cast<bool*>(0)) != 0 ? 1 : 0;
            PVPMAP_ONLY_TOURNAMENT_RANDOM[row] =
                ScanInt(static_cast<bool*>(0)) != 0 ? 1 : 0;
            if (++row > 19) {
                LogManager::logFormat(0,
                                      "../RDARScriptEtcParameter.cpp",
                                      "loadPvpMapBasicDataScript",
                                      0x1C2C,
                                      "ERROR : %d ",
                                      20);
                break;
            }
        } else if (tag == "[pvp map order]") {
            while (pvp_map_data != 0) {
                map_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                PVPMAP_DATA data;
                data.map_index = static_cast<uint32_t>(map_index);
                data.clear();
                pvp_map_data->push_back(data);
            }
        }
    }

    return 1;
}
