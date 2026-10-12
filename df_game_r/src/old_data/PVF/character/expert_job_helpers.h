#ifndef OLDSRV_OLD_DATA_PVF_CHARACTER_EXPERT_JOB_HELPERS_H_
#define OLDSRV_OLD_DATA_PVF_CHARACTER_EXPERT_JOB_HELPERS_H_

#include "old_game/progression/quest/quest_skill_expert_job.h"

#include <map>
#include <vector>


namespace expert_job {

int get_additional_item(const std::vector<stItemSelectRate>& items,
                        int random,
                        stItemSelectRate& selected,
                        int grade);

int get_disjoint_additional_items(
    std::multimap<int, stItemSelectRate>& source,
    int group,
    std::vector<stItemSelectRate>& output);

}  // namespace expert_job

#endif  // OLDSRV_OLD_DATA_PVF_CHARACTER_EXPERT_JOB_HELPERS_H_
