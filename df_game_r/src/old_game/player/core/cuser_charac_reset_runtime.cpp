#include "old_game/player/core/cuser.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "common/manager/user.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"

#include <cstring>
#include <list>

// IDA_EA: 0x865851C
// IDA_NAME: _ZN5CUser14ResetCurCharacEv
// DEMANGLED: CUser::ResetCurCharac(void)
void* CUser::ResetCurCharac() {
    CUser* const user = this;

    user->getPVPResultRefW()->Clear();
    user->getBattleRecordRefW()->clear();
    reset_list_.clear();
    user->getDungeonClearRefW()->clear();

    user->gm_boss_tower_stage_override_ = 0;
    user->gm_boss_tower_map_index_override_ = 0;

    if (user->getCurCharacR()) {
        CInventory* inventory = user->getCurCharacInvenR();
        reinterpret_cast<WongWork::CAvatarItemMgr*>(
            inventory->GetAvatarItemMgrW())->Reset();
        user->getCurCharacSkillR()->clear_all_skills_both();
        user->getCurCharacCargoR()->reset();
        user->getCurCharacQuestR()->reset();
        user_creature::CCreatureMgr* const creature_mgr =
            reinterpret_cast<user_creature::CCreatureMgr*>(
                inventory->GetCreatureMgrW());
        creature_mgr->Reset();
        creature_mgr->SetUser(user);
        if (static_cast<unsigned char>(user->isDisguiseCharac())) {
            user->setDisguiseCharac(false, 0, 0);
        }
    }

    user->reset_guild_temp_exp();
    user->reset_guild_exp();
    user->reset_level_before_dungeon();
    for (unsigned int member_count = 1; member_count <= 4; ++member_count) {
        const unsigned int charac_no =
            static_cast<unsigned int>(user->getCurCharacNo());
        WongWork::CDeathTowerRanking* table =
            WongWork::CDeathTowerRanking::getRankTable(member_count);
        table->unregistBestRecord(charac_no);
        WongWork::CDeathTowerRanking::getRankTable(member_count)
            ->unregistRanking(
            static_cast<unsigned int>(user->getCurCharacNo()));
    }
    user->resetCurCharacSchoolPoint();

    if (user->getCurCharacR()) {
        const int charac_job = user->get_charac_job();
        --G_GameWorld()->occupation_counts_[charac_job];
    }

    user->setCurCharac(0);
    user->setTagCharac(0);
    party_index_ = -1;
    trade_space_index_ = -1;
    pvp_index_ = -1;
    war_room_index_ = -1;
    blue_marble_index_ = -1;
    move_space_ = 0;
    game_master_mode_ = 0;
    network_lifecycle_state_.mercenary_reward_dice_override = 0;
    network_lifecycle_state_.mercenary_selection_index = -1;
    ting_flag_ = 0;
    playing_dungeon_index_ = -1;
    program_logout_ = 0;
    quest_reward_request_count_ = 0;
    quest_reward_request_tick_ = 0;

    WongWork::CMailBox* mailbox = user->GetMailBox();
    if (mailbox) {
        mailbox->Init();
    }

    user->set_guildwar_point_per_pvpplay(0);
    std::memset(&guild_info_, 0, sizeof(guild_info_));
    guild_agit_info_.enabled_ = 0;
    user->resetGuildCreateFlag();
    user->resetTempGuildNameAddr();
    user->reset_pvp_masterid_walkingout_me();
    mcap_manager_.resetExposedCount();
    revival_time_ = 0;
    use_apc_potion_time_ = 0;
    village_attacked_reward_ = 0;
    pc_room_auth_ = 0;
    std::memset(&exp_reward_, 0, 4u);
    user->setForceDropFlag(false);
    save_rental_info_to_exchange_ = 1;
    load_rental_info_from_exchange_ = 0;
    combo_skill_set_ = 0;
    return user;
}
