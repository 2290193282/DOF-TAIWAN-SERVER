#include "old_game/player/core/character_view_data.h"
#include "common/manager/global_data.h"
#include "old_data/db/character/db_save_charac.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/activity/schedule/freely_give_item.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <cstring>
#include <stdint.h>


extern uint8_t* setBit_Byte(uint8_t& value, bool bit, int enabled)
    __asm__("_Z11setBit_ByteRhbi");

namespace {

void CopyAdditionalInfo(SIG_SAVE_CHARAC_ADDITIONAL_INFO& destination,
                        const _Additioal_info& source) {
    destination.max_hp = source.max_hp;
    destination.max_mp = source.max_mp;
    destination.physical_attack = source.physical_attack;
    destination.physical_defense = source.physical_defense;
    destination.magical_attack = source.magical_attack;
    destination.magical_defense = source.magical_defense;
    std::memcpy(destination.element_resist,
                source.element_resist,
                sizeof(destination.element_resist));
    std::memcpy(destination.spec_property,
                source.spec_property,
                sizeof(destination.spec_property));
    destination.inventory_weight = source.inventory_weight;
    destination.hp_regen_speed = source.hp_regen_speed;
    destination.mp_regen_speed = source.mp_regen_speed;
    destination.move_speed = source.move_speed;
    destination.attack_speed = source.attack_speed;
    destination.cast_speed = source.cast_speed;
    destination.hit_recovery = source.hit_recovery;
    destination.jump_power = source.jump_power;
    destination.character_weight = source.character_weight;
}

}  // namespace


// IDA_EA: 0x8650AA2
// IDA_NAME: _ZN5CUser15_SaveCharacDataEv
// DEMANGLED: CUser::_SaveCharacData(void)
// Active reconstructed-tree ABI: _ZN5CUser15_SaveCharacDataEv.
void CUser::_SaveCharacData() {
    CUser* const user = this;
    const int now = GlobalData::system_time()->getCurSec();
    SetAccountLastPlayTime(now);

    if (static_cast<unsigned char>(this->getCurCharacVill()) ==
        7u) {
        setCurCharacLastPlayTickPowerWar(
            GlobalData::system_time()->getCurSec());
        SetCurCharacChaosDieTime(0);
        character_view_.enableSaveCharacView();
    }

    this->setCurCharacLastPlayTick(
        GlobalData::system_time()->getCurSec());

    if (static_cast<unsigned char>(isSaveCharacInfo()) !=
            1u &&
        static_cast<unsigned char>(IsSaveCharacStat()) !=
            1u &&
        static_cast<unsigned char>(isSaveCharacEtc()) !=
            1u &&
        static_cast<unsigned char>(IsSaveCharacManageInfo()) !=
            1u) {
        return;
    }

    CStreamGuard guard(GlobalData::stream_pool()->Acquire("user.cpp", 6595),
                       true);
    *guard << 34;
    *guard << this->GetUID();

    SIG_SAVE_CHARAC* const payload =
        guard.GetInBuffer<SIG_SAVE_CHARAC>();
    std::memset(payload, 0, sizeof(*payload));

    payload->dungeon_gained_gold = reinterpret_cast<CDungeonGainedGold*>(
        getDungeonGainedGold())->getGainedGold();
    payload->account_id = static_cast<uint32_t>(get_acc_id());
    payload->report_mannerless_count = report_mannerless_user_count_;
    payload->over_trade_gold_count = GetOverTradeGoldCount();
    payload->rating = GetRating();
    payload->tutorial_skipable = GetTutorialSkipable();
    payload->charac_no = static_cast<uint32_t>(getCurCharacNo());

    payload->save_charac_info = static_cast<uint8_t>(isSaveCharacInfo());
    payload->save_charac_stat = static_cast<uint8_t>(IsSaveCharacStat());
    payload->save_charac_etc = static_cast<uint8_t>(isSaveCharacEtc());
    payload->save_charac_manage_info =
        static_cast<uint8_t>(IsSaveCharacManageInfo());
    payload->save_cube_premium_info =
        static_cast<uint8_t>(isSavedPremiumCubeInfo());
    if (payload->save_cube_premium_info) {
        uint8_t selected = 0;
        uint8_t cube_type = 0;
        getContractOfCubePremiumInfo(selected, cube_type);
        payload->cube_premium_selected = selected;
        payload->cube_premium_type = cube_type;
    }

    if (payload->save_charac_info) {
        payload->character_level = user->get_charac_level();
        payload->grow_type = user->getCurCharacGrowType();
        CopyAdditionalInfo(payload->stats, *user->getCurCharacAddInfoR());
        payload->expert_job_type = user->GetCurCharacExpertJobType();
    }

    if (payload->save_charac_stat) {
        payload->experience = user->getCurCharacExp();
        payload->village = user->getCurCharacVill4Save();
        payload->fatigue = user->GetFatigue();
        payload->premium_fatigue = user->getCurCharacPremiumFatigue();
        payload->stamina = user->getCurCharacStamina();
        payload->used_fatigue = user->getCurCharacUsedFatigue();
        payload->tutorial_flag = user->getCurCharacTutorialFlag();

        if (user->getCurCharacR()) {
            std::memcpy(payload->escalade_tutorial_flag,
                        user->getCurCharacEscaladeTutorialFlag(),
                        sizeof(payload->escalade_tutorial_flag));
        }

        payload->trade_gold_total = user->getCurCharacTradeGoldTotal();
        payload->trade_gold_total_billion =
            user->getCurCharacTradeGoldTotalBillion();
        payload->trade_gold_daily = user->getCurCharacTradeGoldDaily();
        payload->help_abuse_ratio =
            user->getCurCharacDungeonHelpAbuseRatio();
        payload->help_abuse_experience =
            user->getCurCharacDungeonHelpAbuseExp();
        payload->chaos_point = user->GetCurCharacChaosPoint();
        payload->chaos_experience = user->GetCurCharacChaosExp();
        payload->chaos_mode_count = user->GetCurCharacChaosModeCount();
        payload->chaos_kill_count = user->GetCurCharacChaosKillCount();
        payload->chaos_die_count = user->GetCurCharacChaosDieCount();
        payload->chaos_die_time = user->GetCurCharacChaosDieTime();
        payload->assault_count = user->GetAssaultCount();
        payload->chaos_kill_time = user->GetCurCharacChaosKillTime();
        payload->open_coin_flag = user->GetOpencoin();
        payload->luck_point = user->GetCurCharacLuckPoint();
        payload->dungeon_play_count = user->GetCurCharacDungeonPlayCount();
        payload->expert_job_experience = user->GetCurCharacExpertJobExp();
        payload->fatigue_battery_charging =
            user->GetFatigueBatteryCharging();
        payload->power_war_point = user->GetUserPowerWarPoint();
        payload->previous_village = user->getCurCharacVillPrev();
        payload->power_war_assault_count = user->GetPowerWarAssaultCount();
        payload->power_war_victory_count =
            user->GetPowerWarAssaultVictoryCount();
        payload->last_play_time_power_war =
            user->getCurCharacLastPlayTickPowerWar();
        payload->fatigue_grownup_buff =
            user->getCurCharacFatigueGrownUpBuff();
        payload->emotion = user->GetCurCharacEmotion();
        payload->skill_tree_index = user->GetCurCharacSkillTreeIndex();
        payload->expanded_slot_flag = user->GetCurExEquipSlotStat();
        payload->member_dungeon_flag = user->getMemberDungeonFlag();
        payload->member_bonus_fatigue =
            user->getCurCharacMemberBonusFatigue();
        payload->last_play_dungeon_index =
            user->getCurCharacLastPlayDungeonIndex();
        payload->equipslot_flag = user->getCurEquipslotFlag();
        payload->channel_equipslot_switch =
            user->getCurChannelEquipslotSwitch();
        payload->expand_equipslot_switch =
            user->getCurExpandEquipslotSwitch();
        payload->growth_power_reward = user->get_growth_power_reward();
        payload->chaos_response_time = user->GetCurCharacChaosResponTime();

        reinterpret_cast<WongWork::CAvatarItemMgr*>(
            reinterpret_cast<CInventory*>(user->getCurCharacInvenW())
                ->GetAvatarItemMgrW())
            ->SaveEndurance();
    }

    if (payload->save_charac_etc) {
        payload->guild_right = user->get_guild_create_right();
        payload->maximum_premium_fatigue =
            user->getCurCharacMaxPremiumFatigue();
        payload->event_character_grow_type =
            user->getCurCharacEventCharacterGrowtype();
    }

    if (payload->save_charac_manage_info) {
        payload->striker_skill_index = user->getCurCharacStrikerSkillIndex();
        payload->tag_charac_no = user->getCurCharacTagCharacNo();
        payload->maximum_equip_level = user->GetCurCharacMaxEquipLevel();
    }

    payload->last_play_time = user->getCurCharacLastPlayTick();
    uint8_t visible_flags = user->get_charac_visible_values();
    setBit_Byte(visible_flags, false, 0);
    payload->visible_flags = visible_flags;

    const bool saved_character_level = payload->save_charac_info != 0u;
    const uint32_t persisted_character_level = payload->character_level;
    const int queued =
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    // The feature request is queued after signal 34 on the same DB_Q.  Its DB
    // owner re-reads charac_info.lev and therefore only awards a level-up when
    // the authoritative save above actually committed.
    if (queued == 1 && saved_character_level) {
        freely_give_item::OnLevelUp(user, persisted_character_level);
    }
    user->disableSaveCharacInfo();
    user->disableSaveCharacStat();
    user->disableSaveCharacEtc();
    user->disableSaveCharacManageInfo();
    user->disableSavedPremiumCubeInfo();
}
