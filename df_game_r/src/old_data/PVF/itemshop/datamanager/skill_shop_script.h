#ifndef OLDSRV_DATAMANAGER_SKILL_SHOP_SCRIPT_H_
#define OLDSRV_DATAMANAGER_SKILL_SHOP_SCRIPT_H_

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "common/manager/global_tools.h"

// IDA_EA: 0x8913F48, 0x891427F, 0x8914DD1.
// IDA unit: 99_misc_global/Global skill-shop PVF/RDAR import boundaries used
// by DataManager script population. Character job enum values are the old
// integer values returned by global getCharacterJob(char const*).
int importCommonSkillIndexes(const char* filename,
                             std::vector<int>& common_skill_indexes,
                             std::map<ENUM_CHARACTERJOB, std::set<int> >&
                                 except_skill_info);
int importSkillShopTree(const char* filename,
                        std::map<ENUM_CHARACTERJOB, std::string>&
                            skill_shop_tree);
int importCancelSkillIndices(
    const char* filename,
    std::map<std::pair<ENUM_CHARACTERJOB, int>, std::set<int> >&
        cancel_skill_indices);

#endif  // OLDSRV_DATAMANAGER_SKILL_SHOP_SCRIPT_H_
