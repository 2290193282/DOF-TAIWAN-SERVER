#include "old_game/activity/schedule/advance_altar_stage_control_runtime.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_game/activity/reward/advance_altar.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace advancealtar {

// IDA_EA: 0x812F3AE
// IDA_NAME: _ZN12advancealtar9ProcStage20addAchievementRewardERSt3mapIiNS_7_RewardESt4lessIiESaISt4pairIKiS2_EEE
// DEMANGLED: advancealtar::ProcStage::addAchievementReward(std::map<int,advancealtar::_Reward> &)
int ProcStage::addAchievementReward(std::map<int, _Reward>& rewards) {
    CUser* const user = character_data ? character_data->getUser() : 0;
    if (!user) {
        return 0;
    }

    int clear_summary[3] = {};
    user->advance_altar_manager_.getClearStageSummary(clear_summary);
    CDataManager* const data_manager = G_CDataManager();
    const RewardParameter* const reward_parameter =
        data_manager ? data_manager->get_advance_altar_reward_parameter() : 0;
    if (!reward_parameter) {
        return 0;
    }

    for (int difficulty = StageDifficulty::EASY;
         difficulty <= StageDifficulty::HARD;
         ++difficulty) {
        const AchievementRewardData* const reward_data =
            reward_parameter->getAchievementRewardData(
                static_cast<StageDifficulty::T>(difficulty),
                clear_summary[difficulty]);
        if (reward_data) {
            user->advance_altar_manager_.addRewardData(
                reward_data->achievement_id, rewards);
        }
    }
    return 0;
}

// IDA_EA: 0x812F0FA
// IDA_NAME: _ZN12advancealtar9ProcStage16rewardClearStageERSt6vectorINS_11_RewardItemESaIS2_EE
// DEMANGLED: advancealtar::ProcStage::rewardClearStage(std::vector<advancealtar::_RewardItem> &)
int ProcStage::rewardClearStage(std::vector<_RewardItem>& rewards) {
    if (!dungeon_) {
        return 0;
    }

    rewards.clear();
    const std::vector<_RewardItem>* reward_data = 0;
    if (getStageType() == StageType::SURVIVAL) {
        reward_data = dungeon_->survival_reward_data_.getRoundRewardData(
            getClearSurvivalRoundNo());
    } else if (stage_difficulty >= StageDifficulty::EASY &&
               stage_difficulty <= StageDifficulty::HARD) {
        reward_data =
            &dungeon_->clear_reward_data_.reward_items[stage_difficulty];
    }

    std::vector<_RewardItem> normal_mail_rewards;
    std::vector<_RewardItem> event_mail_rewards;
    if (reward_data) {
        for (std::vector<_RewardItem>::const_iterator it =
                 reward_data->begin();
             it != reward_data->end();
             ++it) {
            if (isOnEvent()) {
                rewards.push_back(*it);
                if (it->reward_type != RewardItemType::NORMAL) {
                    event_mail_rewards.push_back(*it);
                } else {
                    normal_mail_rewards.push_back(*it);
                }
            } else if (it->reward_type == RewardItemType::NORMAL) {
                rewards.push_back(*it);
                normal_mail_rewards.push_back(*it);
            }
        }
    }

    CUser* const user = character_data ? character_data->getUser() : 0;
    if (!normal_mail_rewards.empty()) {
        sendMailRewardItem(user, normal_mail_rewards,
                           "chn_game_server_msg_88",
                           "chn_game_server_msg_89");
    }
    if (isOnEvent() && !event_mail_rewards.empty()) {
        sendMailRewardItem(user, event_mail_rewards,
                           "chn_game_server_msg_86",
                           "chn_game_server_msg_87");
    }
    return 0;
}

// IDA_EA: 0x812E6DE
// IDA_NAME: _ZNK12advancealtar9ProcStage14sendClearStageERKSt6vectorINS_11_RewardItemESaIS2_EERKS1_INS_8_BuyItemESaIS7_EERKSt3mapIiNS_7_RewardESt4lessIiESaISt4pairIKiSD_EEEib
// DEMANGLED: advancealtar::ProcStage::sendClearStage(std::vector<advancealtar::_RewardItem> const &,std::vector<advancealtar::_BuyItem> const &,std::map<int,advancealtar::_Reward> const &,int,bool)const
void ProcStage::sendClearStage(
    const std::vector<_RewardItem>& rewards,
    const std::vector<_BuyItem>& unlocked_items,
    const std::map<int, _Reward>& achievement_rewards,
    int reward_star,
    bool improved_survival_record) const {
    CUser* const user = character_data ? character_data->getUser() : 0;
    if (!user) {
        return;
    }

    const AdvanceAltarData* const current_data =
        user->advance_altar_manager_.getCurrentAdvanceAltarData();
    const int survival_best = current_data
                                  ? static_cast<int>(static_cast<int16_t>(
                                        current_data->survival_best))
                                  : 0;

    NOTIPacketStruct::_NotiAdvanceAltarClearStage notification;
    notification.stage_type = static_cast<int32_t>(getStageType());
    notification.clear_survival_round_no = getClearSurvivalRoundNo();
    notification.survival_best = survival_best;
    notification.improved_survival_record =
        improved_survival_record ? 1u : 0u;
    notification.stage_index = getStageIndex();
    notification.stage_difficulty = stage_difficulty;
    notification.reward_star = reward_star;

    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_ADVANCE_ALTAR_CLEAR_STAGE);
    packet.put_binary(reinterpret_cast<const char*>(&notification),
                      sizeof(notification));
    packet.put_int(static_cast<int>(rewards.size()));
    for (std::vector<_RewardItem>::const_iterator it = rewards.begin();
         it != rewards.end();
         ++it) {
        packet.put_binary(reinterpret_cast<const char*>(&*it), sizeof(*it));
    }
    packet.put_int(static_cast<int>(unlocked_items.size()));
    for (std::vector<_BuyItem>::const_iterator it = unlocked_items.begin();
         it != unlocked_items.end();
         ++it) {
        packet.put_binary(reinterpret_cast<const char*>(&*it), sizeof(*it));
    }
    packet.put_int(static_cast<int>(achievement_rewards.size()));
    for (std::map<int, _Reward>::const_iterator it =
             achievement_rewards.begin();
         it != achievement_rewards.end();
         ++it) {
        packet.put_binary(reinterpret_cast<const char*>(&it->second),
                          sizeof(it->second));
    }
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x812E3F6
// IDA_NAME: _ZN12advancealtar9ProcStage19clearStageAndRewardEv
// DEMANGLED: advancealtar::ProcStage::clearStageAndReward(void)
CUser* ProcStage::clearStageAndReward() {
    if (!dungeon_) {
        return 0;
    }

    CUser* const user = character_data ? character_data->getUser() : 0;
    if (!user) {
        return 0;
    }

    int reward_star = 0;
    bool unlocked_stage = false;
    bool improved_survival_record = false;
    if (!user->advance_altar_manager_.clearStage(
            getStageIndex(),
            static_cast<StageDifficulty::T>(stage_difficulty),
            getStageType(),
            getClearSurvivalRoundNo(),
            reward_star,
            unlocked_stage,
            improved_survival_record)) {
        return user;
    }

    std::map<int, _Reward> achievement_rewards;
    addAchievementReward(achievement_rewards);

    std::vector<_RewardItem> rewards;
    rewardClearStage(rewards);
    user->advance_altar_manager_.setNextStage(getStageIndex());

    const StageType::T stage_type = getStageType();
    const bool has_stage_reward =
        stage_type == StageType::NORMAL ||
        (stage_type == StageType::SURVIVAL && !rewards.empty());
    if (!has_stage_reward || user->advance_altar_manager_.decTicket() == 1) {
        std::vector<_BuyItem> unlocked_items;
        if (unlocked_stage) {
            user->advance_altar_manager_.setUnlockBuyItem(
                AdvanceAltarShopType::UNIT, getStageIndex(), &unlocked_items);
            user->advance_altar_manager_.setUnlockBuyItem(
                AdvanceAltarShopType::SKILL, getStageIndex(), &unlocked_items);
            user->advance_altar_manager_.setUnlockBuyItem(
                AdvanceAltarShopType::TOWER, getStageIndex(), &unlocked_items);
        }
        sendClearStage(rewards, unlocked_items, achievement_rewards,
                       reward_star, improved_survival_record);
        if (reward_star > 0) {
            HistoryLog::starAdd(*user,
                                static_cast<AddStarType::T>(0),
                                getStageIndex(),
                                reward_star);
        }
    }
    return user;
}

}  // namespace advancealtar
