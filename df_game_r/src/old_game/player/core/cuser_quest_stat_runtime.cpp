#include "old_game/player/core/cuser.h"

#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/quest/quest_state_runtime.h"

#include <utility>
#include <vector>

namespace {

// CQuestClear::enumQuestClear forwards the untouched i386 vararg block to
// its callback.  calc_quest_stat supplies these three pointers in this order.
struct QuestStatCallbackArguments {
    const CUser* user;
    const std::vector<int>* tracked_item_ids;
    std::vector<std::pair<int, int> >* matched_reward_items;
};

}  // namespace

// IDA_EA: 0x8664E8E
// IDA_NAME: _ZN5CUser17_fnStatQuestClearEjPc
// DEMANGLED: CUser::_fnStatQuestClear(unsigned int,char *)
bool CUser::_fnStatQuestClear(unsigned int quest_index,
                              char* argument_block) {
    const QuestStatCallbackArguments& arguments =
        *reinterpret_cast<const QuestStatCallbackArguments*>(argument_block);
    Quest* const quest = G_CDataManager()->find_quest(
        static_cast<int>(quest_index));
    if (!quest || (quest->reward_type != 0 && quest->reward_type != 25)) {
        return true;
    }

    const std::vector<RewardItemInfo>* const rewards =
        getUserQuestRewardItem(quest, arguments.user);
    for (std::vector<RewardItemInfo>::const_iterator reward = rewards->begin();
         reward != rewards->end();
         ++reward) {
        for (std::vector<int>::const_iterator item_id =
                 arguments.tracked_item_ids->begin();
             item_id != arguments.tracked_item_ids->end();
             ++item_id) {
            if (reward->item_id == *item_id) {
                add_matched_list(*arguments.matched_reward_items,
                                 reward->item_id,
                                 reward->item_count);
            }
        }
    }
    return true;
}

// IDA_EA: 0x8664C50
// IDA_NAME: _ZN5CUser15calc_quest_statER15_Additioal_info
// DEMANGLED: CUser::calc_quest_stat(_Additioal_info &)
void CUser::calc_quest_stat(_Additioal_info& calculated_stat) {
    std::vector<int> tracked_item_ids;
    for (int index = 0; index <= 7; ++index) {
        tracked_item_ids.push_back(0x8cf19a0 + 4 * index);
    }

    std::vector<std::pair<int, int> > owned_item_counts;
    count_specific_items(tracked_item_ids, owned_item_counts);

    std::vector<std::pair<int, int> > reward_item_counts;
    getCurCharacQuestR()->clear_quest.enumQuestClear(
        &CUser::_fnStatQuestClear,
        this,
        &tracked_item_ids,
        &reward_item_counts);
    complete_reward_list(reward_item_counts, owned_item_counts);

    for (std::vector<std::pair<int, int> >::iterator item =
             reward_item_counts.begin();
         item != reward_item_counts.end();
         ++item) {
        apply_elixir_item(calculated_stat, *item);
    }
}
