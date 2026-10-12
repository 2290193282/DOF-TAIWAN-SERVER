#include "old_game/player/core/cuser.h"

#include "old_data/db/character/load_charac_payload.h"

#include <cstring>

// IDA_EA: 0x864BDBA
// IDA_NAME: _ZNK5CUser19getCharacInfoDetailEijP11CHARAC_DATA
// DEMANGLED: CUser::getCharacInfoDetail(int,unsigned int,CHARAC_DATA *)const
int CUser::getCharacInfoDetail(int charac_idx,
                               unsigned int charac_no,
                               CHARAC_DATA* payload) const {
    if (charac_idx < 0 ||
        characters_.size() <= static_cast<size_t>(charac_idx)) {
        return 0;
    }

    const _Charac_info& character =
        characters_[static_cast<size_t>(charac_idx)];
    const _Charac_info::Fields& fields = character.fields_;
    if (static_cast<unsigned int>(fields.character_no_) != charac_no) {
        return 0;
    }

    payload->character_no = fields.character_no_;
    std::strncpy(payload->character_name,
                 fields.character_name_,
                 sizeof(payload->character_name));
    payload->village = static_cast<char>(fields.village_);
    payload->job = static_cast<char>(fields.job_);
    payload->level = fields.level_;
    payload->experience = fields.experience_;
    payload->grow_type = static_cast<char>(fields.grow_type_.growth_type);
    payload->hp = fields.stamina_;

    payload->additional_info.max_hp = fields.additional_info_.max_hp;
    payload->additional_info.max_mp = fields.additional_info_.max_mp;
    payload->additional_info.physical_attack =
        fields.additional_info_.physical_attack;
    payload->additional_info.physical_defense =
        fields.additional_info_.physical_defense;
    payload->additional_info.magical_attack =
        fields.additional_info_.magical_attack;
    payload->additional_info.magical_defense =
        fields.additional_info_.magical_defense;
    for (size_t index = 0;
         index < sizeof(payload->additional_info.element_resist) /
                     sizeof(payload->additional_info.element_resist[0]);
        ++index) {
        payload->additional_info.element_resist[index] =
            fields.additional_info_.element_resist[index];
    }
    for (size_t index = 0;
         index < sizeof(payload->additional_info.spec_property) /
                     sizeof(payload->additional_info.spec_property[0]);
        ++index) {
        payload->additional_info.spec_property[index] =
            fields.additional_info_.spec_property[index];
    }
    payload->additional_info.inventory_weight =
        fields.additional_info_.inventory_weight;
    payload->additional_info.hp_regen_speed =
        fields.additional_info_.hp_regen_speed;
    payload->additional_info.mp_regen_speed =
        fields.additional_info_.mp_regen_speed;
    payload->additional_info.move_speed = fields.additional_info_.move_speed;
    payload->additional_info.attack_speed =
        fields.additional_info_.attack_speed;
    payload->additional_info.cast_speed = fields.additional_info_.cast_speed;
    payload->additional_info.hit_recovery =
        fields.additional_info_.hit_recovery;
    payload->additional_info.jump_power = fields.additional_info_.jump_power;
    payload->additional_info.character_weight =
        fields.additional_info_.character_weight;

    payload->fatigue = fields.fatigue_;
    payload->maximum_fatigue = fields.maximum_fatigue_;
    payload->fatigue_grownup_buff = fields.fatigue_grownup_buff_;
    payload->fatigue_battery_charging = fields.fatigue_battery_charging_;
    payload->last_play_time = fields.last_play_tick_;
    payload->power_war_last_play_time = fields.power_war_last_play_time_;
    payload->guild_id = fields.guild_id_;
    payload->guild_right = fields.guild_right_;
    payload->premium_fatigue = fields.premium_fatigue_;
    payload->maximum_premium_fatigue = fields.maximum_premium_fatigue_;
    payload->member_flag = fields.member_key_ ? 1u : 0u;
    fields.create_time_.GetCreateTime(payload->create_time);
    payload->used_fatigue = fields.used_fatigue_;
    payload->tutorial_flag = fields.tutorial_flag_;
    for (size_t index = 0;
         index < sizeof(payload->escalade_tutorial_flag) /
                     sizeof(payload->escalade_tutorial_flag[0]);
         ++index) {
        payload->escalade_tutorial_flag[index] =
            static_cast<char>(fields.escalade_tutorial_flag_[index]);
    }
    payload->help_abuse_ratio = fields.help_abuse_ratio_;
    payload->help_abuse_experience = fields.help_abuse_experience_;
    payload->trade_gold_total = fields.reliable_person_.GetTradeGoldTotal();
    payload->trade_gold_total_billion =
        fields.reliable_person_.GetTradeGoldTotalBillion();
    payload->trade_gold_daily = fields.reliable_person_.GetTradeGoldDaily();
    payload->chaos_point = fields.chaos_point_;
    payload->chaos_experience = fields.chaos_experience_;
    payload->chaos_mode_count = fields.chaos_mode_count_;
    payload->chaos_kill_count = fields.chaos_kill_count_;
    payload->chaos_die_count = fields.chaos_die_count_;
    payload->chaos_die_time = fields.chaos_die_time_;
    payload->assault_count = static_cast<int32_t>(fields.assault_count_);
    payload->chaos_kill_time = fields.chaos_kill_time_;
    payload->open_flag = static_cast<char>(fields.open_coin_flag_);
    payload->luck_point = fields.luck_point_;
    payload->dungeon_play_count = fields.dungeon_play_count_;
    payload->expert_job_experience = fields.expert_job_experience_;
    payload->expert_job = fields.expert_job_;
    payload->member_id = static_cast<uint32_t>(get_acc_id());
    payload->power_war_point = fields.power_war_point_;
    payload->previous_village = static_cast<char>(fields.previous_village_);
    payload->power_war_assault_count = fields.power_war_assault_count_;
    payload->power_war_victory_count = fields.power_war_victory_count_;
    payload->power_war_victory_count = fields.power_war_victory_count_;
    payload->emotion = fields.emotion_;
    payload->skill_tree_index = static_cast<char>(fields.skill_tree_index_);
    payload->add_slot_flag = static_cast<char>(fields.exp_equip_slot_state_);
    payload->event_character_grow_type =
        static_cast<char>(fields.event_character_grow_type_);
    payload->member_dungeon_flag =
        static_cast<char>(fields.member_dungeon_flag_);
    payload->member_bonus_fatigue = fields.member_bonus_fatigue_;
    payload->guild_secede = static_cast<char>(fields.guild_secede_);
    payload->last_play_dungeon_index = fields.last_play_dungeon_index_;
    payload->add_equipslot_flag =
        static_cast<char>(fields.equipslot_flag_);
    payload->channel_equipslot_switch =
        static_cast<char>(fields.channel_equipslot_switch_);
    payload->expand_equipslot_switch =
        static_cast<char>(fields.expand_equipslot_switch_);
    payload->event_bring_up_reward_flag = fields.event_bring_up_reward_flag_;
    payload->event_bring_up_material_count =
        fields.event_bring_up_material_total_;
    payload->visible_flags =
        static_cast<char>(fields.visible_flag_.visible_flag);
    payload->growth_power_reward = fields.growth_power_reward_;
    payload->chaos_response_time = fields.chaos_response_time_late_;
    return 1;
}

// IDA_EA: 0x86904F0
// IDA_NAME: _ZNK5CUser16getTagCharacInfoEijP15TAG_CHARAC_DATA
// DEMANGLED: CUser::getTagCharacInfo(int,unsigned int,TAG_CHARAC_DATA *)const
int CUser::getTagCharacInfo(int charac_idx,
                            unsigned int charac_no,
                            TAG_CHARAC_DATA* payload) const {
    if (charac_idx < 0 ||
        characters_.size() <= static_cast<size_t>(charac_idx)) {
        return 0;
    }

    const _Charac_info& character =
        characters_[static_cast<size_t>(charac_idx)];
    const _Charac_info::Fields& fields = character.fields_;
    if (static_cast<unsigned int>(fields.character_no_) != charac_no) {
        return 0;
    }

    payload->character_no = fields.character_no_;
    std::strncpy(payload->character_name,
                 fields.character_name_,
                 sizeof(payload->character_name));
    payload->job = static_cast<char>(fields.job_);
    payload->level = fields.level_;
    payload->grow_type = static_cast<char>(fields.grow_type_.growth_type);
    payload->skill_tree_index = static_cast<char>(fields.skill_tree_index_);
    payload->additional_info.max_hp = fields.additional_info_.max_hp;
    payload->additional_info.max_mp = fields.additional_info_.max_mp;
    payload->additional_info.physical_attack =
        fields.additional_info_.physical_attack;
    payload->additional_info.physical_defense =
        fields.additional_info_.physical_defense;
    payload->additional_info.magical_attack =
        fields.additional_info_.magical_attack;
    payload->additional_info.magical_defense =
        fields.additional_info_.magical_defense;
    for (size_t index = 0;
         index < sizeof(payload->additional_info.element_resist) /
                     sizeof(payload->additional_info.element_resist[0]);
        ++index) {
        payload->additional_info.element_resist[index] =
            fields.additional_info_.element_resist[index];
    }
    for (size_t index = 0;
         index < sizeof(payload->additional_info.spec_property) /
                     sizeof(payload->additional_info.spec_property[0]);
        ++index) {
        payload->additional_info.spec_property[index] =
            fields.additional_info_.spec_property[index];
    }
    payload->additional_info.inventory_weight =
        fields.additional_info_.inventory_weight;
    payload->additional_info.hp_regen_speed =
        fields.additional_info_.hp_regen_speed;
    payload->additional_info.mp_regen_speed =
        fields.additional_info_.mp_regen_speed;
    payload->additional_info.move_speed = fields.additional_info_.move_speed;
    payload->additional_info.attack_speed =
        fields.additional_info_.attack_speed;
    payload->additional_info.cast_speed = fields.additional_info_.cast_speed;
    payload->additional_info.hit_recovery =
        fields.additional_info_.hit_recovery;
    payload->additional_info.jump_power = fields.additional_info_.jump_power;
    payload->additional_info.character_weight =
        fields.additional_info_.character_weight;
    return 1;
}
