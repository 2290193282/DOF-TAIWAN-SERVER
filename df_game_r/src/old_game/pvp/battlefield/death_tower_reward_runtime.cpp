#include "old_game/pvp/battlefield/battle_field.h"

#include <stdint.h>

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "core/config/env/environment.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_generator_mgr_abi.h"
#include "common/manager/server_utility.h"
#include "old_game/dungeon/tower/tower_rank_blood_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/data/item_generator_types.h"
#include "old_data/channel/channel_script_t.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"

namespace WongWork {

// IDA_EA: 0x8461FC0
// IDA_NAME:
// _ZN8WongWork11CDeathTower9CPlayData18generateRewardItemEiiRKNS0_11CDungeonMgrEjjP6CParty
// DEMANGLED: WongWork::CDeathTower::CPlayData::generateRewardItem(
//     int,int,WongWork::CDeathTower::CDungeonMgr const&,unsigned int,
//     unsigned int,CParty *)
bool CDeathTower::CPlayData::generateRewardItem(
    int member_slot,
    int item_count,
    const CDungeonMgr& dungeon_manager,
    unsigned int character_level,
    unsigned int reward_probability_scale,
    CParty* party) {
    const stDeathTower_t& death_tower = G_CDataManager()->get_death_tower();
    CItemGenerator* const generator =
        G_CDataManager()->getItemGenerator()->getGenerator(
            CItemGeneratorMgr::kClearReward);
    stRewardItem_t& reward = reward_items_[member_slot];

    reward.item_count_ = static_cast<uint32_t>(item_count);
    for (int item_slot = 0; item_slot < item_count; ++item_slot) {
        stGenerateRefData_t reference;
        stGenerateResult_t random_result;
        if (party) {
            party->GetLuckPoint(&random_result.luck_point, false);
        }

        const unsigned int random_value = get_rand_int(1000);
        const unsigned int gold_probability =
            reward_probability_scale * static_cast<unsigned int>(
                                           death_tower.reward_item_prob_cumulative[0]);
        const unsigned int item_probability =
            reward_probability_scale * static_cast<unsigned int>(
                                           death_tower.reward_item_prob_cumulative[1]);

        if (random_value < gold_probability) {
            reference.generator_data.gold_reward.character_level =
                static_cast<uint8_t>(character_level);
            reference.generator_data.gold_reward.dungeon_difficulty = 0;
            reference.generator_data.gold_reward.party_member_count = 1;
            reference.generator_data.gold_reward.exp_bonus_context = 1;
            reference.generator_data.gold_reward.gold_weight_08 = 1;
            reference.generator_data.gold_reward.gold_weight_0a = 1;
            reference.generator_data.gold_reward.gold_reserved_0c = 1;
            reference.generator_data.gold_reward.gold_weight_0e = 1;
            // IDA 0x84620AB constructs this result after the branch.  The
            // luck-point-populated random result above stays exclusive to the
            // random-item generator path.
            stGenerateResult_t gold_result;
            generator->generateSpecificItem(reference, gold_result);

            Inven_Item& generated_item = gold_result.items.front();
            if (party) {
                QuickParty::RewardData_Param basic_parameter(party);
                basic_parameter.raw_04 =
                    dungeon_manager.getDungeon()->get_index();
                QuickParty::RewardData_Result basic_result;
                basic_result.raw_00 = generated_item.get_add_info();
                party->set_basic_reward(
                    static_cast<QuickParty::BasicRewardType>(11),
                    basic_parameter,
                    basic_result);
                generated_item.set_add_info(basic_result.raw_00);
            }

            reward.items_[item_slot].setCopy(generated_item);
            reward.items_[item_slot].set_add_info(static_cast<int>(
                static_cast<float>(reward.items_[item_slot].get_add_info()) *
                death_tower.reward_gold_weight));
        } else if (random_value < item_probability) {
            reference.generator_data.clear_reward.character_level_or_world_area =
                static_cast<uint8_t>(character_level);
            reference.generator_data.clear_reward
                .dungeon_difficulty_or_world_index = 0;
            reference.generator_data.clear_reward.item_probability_scale =
                1.0f;
            reference.generator_data.clear_reward
                .clear_reward_drop_probability = 0;
            reference.generator_data.clear_reward.dungeon =
                dungeon_manager.getDungeon();
            generator->generateRandomItem(reference, random_result);

            if (!random_result.items.empty()) {
                const Inven_Item& generated_item = random_result.items.front();
                reward.items_[item_slot].setCopy(generated_item);
                CItem* const item =
                    G_CDataManager()->find_item(generated_item.item_id_);
                if (item) {
                    CLuckPoint::UseLuckPoint(
                        party->get_user(member_slot),
                        static_cast<int>(generated_item.item_category_),
                        item->get_rarity(),
                        item->get_grade());
                }
            }
        }
    }

    // The original loop exits with its `i < item_count` result clear.
    return false;
}

// IDA_EA: 0x8462538
// IDA_NAME:
// _ZN8WongWork11CDeathTower9CPlayData17generateRewardExpEiijP6CPartyRNS0_11CDungeonMgrE
// DEMANGLED: WongWork::CDeathTower::CPlayData::generateRewardExp(
//     int,int,unsigned int,CParty *,WongWork::CDeathTower::CDungeonMgr &)
CDeathTower::CPlayData* CDeathTower::CPlayData::generateRewardExp(
    int member_slot,
    int current_stage,
    unsigned int stage_base_exp,
    CParty* party,
    CDungeonMgr& dungeon_manager) {
    CUser* const user = party->get_user(member_slot);
    CDungeon* const dungeon = dungeon_manager.getDungeon();
    const stDeathTower_t& death_tower = G_CDataManager()->get_death_tower();
    const float stage_exp =
        static_cast<float>(stage_base_exp * static_cast<unsigned int>(current_stage));
    const float exp_weight = dungeon->limitOfStackableItemInTower()
                                 ? death_tower.reward_exp_weight[current_stage]
                                 : death_tower.illusion_reward_exp_weight[current_stage];
    unsigned int reward_exp =
        static_cast<unsigned int>(stage_exp * exp_weight);

    if (reward_exp > 0x10C8E0u) {
        LogManager::logFormat(
            1,
            "DeathTower.cpp",
            "void WongWork::CDeathTower::CPlayData::generateRewardExp(int, int, unsigned int, CParty*, WongWork::CDeathTower::CDungeonMgr&)",
            0x2ff,
            "BLOOD_LOG : EROOR DEATHTOWER REWARD EXP(%u), LAST STAGE(%d), BASE EXP(%u), EXP WEIGHT(%f)\n",
            reward_exp,
            current_stage,
            stage_base_exp,
            static_cast<double>(death_tower.reward_exp_weight[current_stage]));
    }

    stClearRewardBonusExp_t bonus_exp;
    stClearRewardExpParameter_t parameter;
    parameter.avatar_bonus_rate =
        static_cast<float>(party->getAvatarBonus());
    parameter.creature_bonus_rate =
        static_cast<float>(party->getCreatureBonus(user));

    CEventBase* const pc_room_event =
        GlobalData::s_event_manager->GetRepeatEvent(8);
    if (pc_room_event->IsEventing(user) != 0) {
        CPCRoomBurningEvent* const event =
            static_cast<CPCRoomBurningEvent*>(pc_room_event);
        parameter.pc_room_burning_rate =
            static_cast<float>(event->GetExpFactor() / 100);
    }

    CEventBase* const double_exp_event =
        GlobalData::s_event_manager->GetRepeatEvent(3);
    if (double_exp_event->IsEventing(0) != 0) {
        CExpDoubleEvent* const event =
            static_cast<CExpDoubleEvent*>(double_exp_event);
        parameter.double_exp_event_rate =
            static_cast<float>(event->GetExpFactor()) / 100.0f;
    }

    CEventBase* const burning_fatigue_event =
        GlobalData::s_event_manager->GetRepeatEvent(15);
    if (burning_fatigue_event->IsEventing(0) != 0) {
        CBurningFatigueEvent* const event =
            static_cast<CBurningFatigueEvent*>(burning_fatigue_event);
        parameter.burning_fatigue_bonus_percent = static_cast<uint32_t>(
            event->getBonusExpRate(
                static_cast<uint16_t>(user->getCurCharacUsedFatigue())));
    }

    channel_script_t* const channels = G_CDataManager()->GetChannelScript();
    parameter.channel_bonus_rate = channels->getBonusRate(
        static_cast<uint8_t>(G_CEnvironment()->get_server_group()),
        static_cast<uint32_t>(G_CEnvironment()->get_channel_no()),
        static_cast<uint32_t>(dungeon->get_index()));

    if (user->IsHavePremiumAdvantage()) {
        parameter.premium_advantage_rate =
            static_cast<float>(
                user->GetPremiumInfo()->GetAdvantageExpRate()) / 100.0f;
    }
    parameter.server_event_rate =
        static_cast<float>(CServerEvent::GetExpRate()) / 100.0f;
    parameter.character_manage_rate =
        static_cast<float>(party->CalcurateManageBonus(
            ManageBonusType::Exp, user, reward_exp));

    unsigned int clear_reward_exp = party->getClearRewardBonusExp(
        user, reward_exp, parameter, bonus_exp);
    if (clear_reward_exp > 0x10C8E0u) {
        LogManager::logFormat(
            1,
            "DeathTower.cpp",
            "void WongWork::CDeathTower::CPlayData::generateRewardExp(int, int, unsigned int, CParty*, WongWork::CDeathTower::CDungeonMgr&)",
            0x324,
            "BLOOD_LOG : EROOR DEATHTOWER CLEAR REWARD EXP(%u)\n",
            clear_reward_exp);
        LogManager::logFormat(
            1,
            "DeathTower.cpp",
            "void WongWork::CDeathTower::CPlayData::generateRewardExp(int, int, unsigned int, CParty*, WongWork::CDeathTower::CDungeonMgr&)",
            0x32e,
            "BLOOD_LOG : DEATHTOWER EXP PARAMETER : %f, %f, %f, %f, %f, %f, %f, %d\n",
            static_cast<double>(parameter.avatar_bonus_rate),
            static_cast<double>(parameter.creature_bonus_rate),
            static_cast<double>(parameter.pc_room_burning_rate),
            static_cast<double>(parameter.double_exp_event_rate),
            static_cast<double>(parameter.channel_bonus_rate),
            static_cast<double>(parameter.premium_advantage_rate),
            static_cast<double>(parameter.server_event_rate),
            static_cast<int>(parameter.burning_fatigue_bonus_percent));
        clear_reward_exp = 1100000u;
    }

    reward_exp_[member_slot] = clear_reward_exp;
    return this;
}

}  // namespace WongWork
