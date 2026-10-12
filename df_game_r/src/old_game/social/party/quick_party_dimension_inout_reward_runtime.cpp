#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"

#include "old_game/social/party/party.h"

namespace QuickParty {

// IDA_EA: 0x826D5AC
// IDA_NAME: _ZN10QuickParty19DimensionInoutCount7processENS_16RewardData_ParamERNS_17RewardData_ResultE
// DEMANGLED: QuickParty::DimensionInoutCount::process(
//     QuickParty::RewardData_Param,QuickParty::RewardData_Result &)
int DimensionInoutCount::process(RewardData_Param param,
                                 RewardData_Result& result) {
    std::map<int, LevelDataRandomReward>::const_iterator reward =
        quick_party_data->random_rewards.find(random_buff_type);
    if (reward == quick_party_data->random_rewards.end()) {
        return 1;
    }

    const int row_index = param.party->get_member_count() - 2;
    if (row_index < 0 ||
        row_index >= static_cast<int>(reward->second.level_data_08.size())) {
        return row_index;
    }

    result.raw_00 = reward->second.level_data_08[row_index].raw_04;
    return param.party->increase_dimension_inout_count(result.raw_00);
}

}  // namespace QuickParty
