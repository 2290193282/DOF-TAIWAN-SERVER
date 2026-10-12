#include "old_game/activity/schedule/advance_altar_data.h"
#include "old_game/activity/schedule/advance_altar_stage_control_runtime.h"

#include "old_game/activity/reward/advance_altar.h"
#include "common/manager/data_manager.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"

#include <cstring>
#include <utility>
#include <vector>

namespace advancealtar {

// IDA_EA: 0x813234E
// IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager21sendAchievementRewardEP5CUseriPPKSt6vectorINS_11_RewardItemESaIS4_EE
// DEMANGLED: advancealtar::CharacAdvanceAltarManager::sendAchievementReward(CUser *,int,std::vector<advancealtar::_RewardItem,std::allocator<advancealtar::_RewardItem>> const**)
int CharacAdvanceAltarManager::sendAchievementReward(
    CUser* user,
    int achievement_id,
    const std::vector<_RewardItem>** reward_items) {
    if (!user || !current) {
        return 2;
    }

    std::map<int, _Reward>::iterator reward =
        current->rewards_b4.find(achievement_id);
    if (reward == current->rewards_b4.end()) {
        return 2;
    }
    if (reward->second.received) {
        return 3;
    }

    CDataManager* const data_manager = G_CDataManager();
    const RewardParameter* const reward_parameter =
        data_manager->get_advance_altar_reward_parameter();
    const AchievementRewardData* const reward_data =
        reward_parameter->getAchievementRewardData(achievement_id);
    if (!reward_data) {
        return 3;
    }

    std::vector<_RewardItem> normal_reward_items;
    std::vector<_RewardItem> event_reward_items;
    for (std::vector<_RewardItem>::const_iterator it =
             reward_data->reward_items.begin();
         it != reward_data->reward_items.end(); ++it) {
        if (isOnEvent()) {
            if (it->reward_type) {
                event_reward_items.push_back(*it);
                continue;
            }
        } else if (it->reward_type) {
            continue;
        }
        normal_reward_items.push_back(*it);
    }

    if (!normal_reward_items.empty()) {
        ProcStage::sendMailRewardItem(user, normal_reward_items,
                                      reward_data->text.c_str(),
                                      reward_data->tooltip.c_str());
    }
    if (!event_reward_items.empty()) {
        ProcStage::sendMailRewardItem(user, event_reward_items,
                                      "chn_game_server_msg_86",
                                      reward_data->tooltip.c_str());
    }

    *reward_items = &reward_data->reward_items;
    reward->second.received = 1;
    current->updated = 1;
    return 0;
}

// IDA_EA: 0x812F47A
// IDA_NAME: _ZN12advancealtar9ProcStage18sendMailRewardItemEP5CUserRKSt6vectorINS_11_RewardItemESaIS4_EEPKcSA_
// DEMANGLED: advancealtar::ProcStage::sendMailRewardItem(CUser *,std::vector<advancealtar::_RewardItem,std::allocator<advancealtar::_RewardItem>> const&,char const*,char const*)
void ProcStage::sendMailRewardItem(CUser* user,
                                   const std::vector<_RewardItem>& rewards,
                                   const char* title_key,
                                   const char* text_key) {
    if (!user || !title_key || !text_key) {
        return;
    }

    char title[31];
    std::strncpy(title, g_scriptStringManager_.findString(4, title_key, 0),
                 0x1eu);
    char text[256];
    std::strncpy(text, g_scriptStringManager_.findString(4, text_key, 0),
                 0xffu);

    Inven_Item mail_items[10];
    std::vector<std::pair<int, int> > mail_rewards;
    unsigned int count = 0;
    for (std::vector<_RewardItem>::const_iterator it = rewards.begin();
         it != rewards.end() && count < 10; ++it, ++count) {
        mail_rewards.push_back(std::make_pair(it->item_id, it->item_count));
    }

    WongWork::CMailBoxHelper::MakeSystemMultiMailPostal(
        mail_rewards, mail_items, count);
    WongWork::CMailBoxHelper::ReqDBSendNewSystemMultiMail(
        title, mail_items, count, 0,
        static_cast<unsigned int>(user->get_charac_no(-1)), text,
        static_cast<int>(std::strlen(text)), 15,
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()), false);
}

}  // namespace advancealtar
