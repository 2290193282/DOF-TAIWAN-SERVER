#include "old_game/progression/quest/quest_item_drop_rate_runtime.h"

namespace QuickParty {

// IDA_EA: 0x826D398
// IDA_NAME: _ZN10QuickParty17QuestItemDropRate7processENS_16RewardData_ParamERNS_17RewardData_ResultE
// DEMANGLED: QuickParty::QuestItemDropRate::process(
//     QuickParty::RewardData_Param, QuickParty::RewardData_Result &)
int QuestItemDropRate::process(RewardData_Param param,
                               RewardData_Result& result) {
    std::map<int, LevelDataRandomReward>::iterator reward =
        quick_party_data->random_rewards.find(random_buff_type);
    if (reward == quick_party_data->random_rewards.end()) {
        return 1;
    }

    const int row_index = param.party->get_member_count() - 2;
    if (row_index < 0 ||
        row_index >= static_cast<int>(reward->second.level_data_08.size())) {
        return row_index;
    }

    const float rate =
        static_cast<float>(reward->second.level_data_08[row_index].raw_04) /
            100.0f +
        1.0f;
    result.raw_18 = static_cast<int>(
        static_cast<long double>(result.raw_18) * rate);
    return reinterpret_cast<intptr_t>(&result);
}

}  // namespace QuickParty
