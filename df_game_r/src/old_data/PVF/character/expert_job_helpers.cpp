#include "old_data/PVF/character/expert_job_helpers.h"


namespace expert_job {

// IDA_EA: 0x896D6AB
// IDA_NAME: _ZN10expert_job19get_additional_itemERKSt6vectorI16stItemSelectRateSaIS1_EEiRS1_i
// DEMANGLED: expert_job::get_additional_item(std::vector<stItemSelectRate> const&,int,stItemSelectRate&,int)
int get_additional_item(const std::vector<stItemSelectRate>& items,
                        int random,
                        stItemSelectRate& selected,
                        int grade) {
    for (std::vector<stItemSelectRate>::const_iterator it = items.begin();
         it != items.end(); ++it) {
        selected = *it;
        if (selected.min_grade <= grade && selected.max_grade >= grade) {
            if (selected.select_rate >= random) {
                return selected.select_rate;
            }
            random -= selected.select_rate;
        }
    }
    return 0;
}

// IDA_EA: 0x896D75D
// IDA_NAME: _ZN10expert_job29get_disjoint_additional_itemsERSt8multimapIi16stItemSelectRateSt4lessIiESaISt4pairIKiS1_EEEiRSt6vectorIS1_SaIS1_EE
// DEMANGLED: expert_job::get_disjoint_additional_items(std::multimap<int,stItemSelectRate>&,int,std::vector<stItemSelectRate>&)
int get_disjoint_additional_items(
    std::multimap<int, stItemSelectRate>& source,
    int group,
    std::vector<stItemSelectRate>& output) {
    if (source.find(group) == source.end()) {
        return 0;
    }
    const std::pair<std::multimap<int, stItemSelectRate>::iterator,
                    std::multimap<int, stItemSelectRate>::iterator>
        range = source.equal_range(group);
    for (std::multimap<int, stItemSelectRate>::iterator it = range.first;
         it != range.second; ++it) {
        output.push_back(it->second);
    }
    return 1;
}

}  // namespace expert_job
