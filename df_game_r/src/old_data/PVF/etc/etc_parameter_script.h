#ifndef OLDSRV_OLD_DATA_PVF_ETC_ETC_PARAMETER_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_ETC_ETC_PARAMETER_SCRIPT_H_

#include <stdint.h>

#include <map>
#include <set>
#include <string>
#include <vector>

// IDA_EA: 0x89136A4, 0x8913C7E, 0x8917848, 0x890A33E, 0x890BB25.
// These typed ETC RDAR parsers are called during CDataManager::init.  Their
// records remain B-owned PVF data, not C PvP/runtime state.
struct DifficultPenaltyInfo {
    int32_t common_trap_explosion[5];
};

int importExperienceItemMapInfo(std::map<int, int>& table, bool reverse);
int importUltimateDungeonList(const char* filename, std::set<int>& list);
int importHiddenNpcListInfo(const char* filename, std::vector<int>& list);
int ImportDifficultPenaltyInfoScript(const char* filename,
                                     DifficultPenaltyInfo& info);
int importItmeGroupTable(const char* dir,
                         const char* filename,
                         std::map<std::string, int>& table);

static_assert(sizeof(DifficultPenaltyInfo) == 0x14u,
              "DifficultPenaltyInfo IDA ABI size must stay 0x14");

#endif  // OLDSRV_OLD_DATA_PVF_ETC_ETC_PARAMETER_SCRIPT_H_
