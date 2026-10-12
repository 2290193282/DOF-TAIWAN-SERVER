#include "old_data/db/character/db_save_charac.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_data/Login/simple_sso.h"
#include "infra/network/packet/stream.h"

#include <cstring>
#include <stdint.h>


// IDA_EA: 0x8415C2E
// IDA_NAME: _ZN13DB_SaveCharacC2Ev
// DEMANGLED: DB_SaveCharac::DB_SaveCharac(void)
DB_SaveCharac::DB_SaveCharac() {}

// IDA_EA: 0x8415C4A
// IDA_NAME: _ZN13DB_SaveCharacD2Ev
// DEMANGLED: DB_SaveCharac::~DB_SaveCharac()
// IDA_EA: 0x8415C7A
// IDA_NAME: _ZN13DB_SaveCharacD0Ev
// DEMANGLED: DB_SaveCharac::~DB_SaveCharac() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_SaveCharac::~DB_SaveCharac() {}

// IDA_EA: 0x8415C98
// IDA_NAME: _ZN13DB_SaveCharac8dispatchEiiP6Stream
// DEMANGLED: DB_SaveCharac::dispatch(int,int,Stream *)
bool DB_SaveCharac::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_SAVE_CHARAC payload;
    if (stream->get_binary(&payload, sizeof(payload)) != 1) {
        return false;
    }

    if ((payload.save_charac_info || payload.save_charac_etc) &&
        _SaveCharacInfo(&payload) != true) {
        return false;
    }
    if (payload.save_charac_manage_info &&
        _SaveCharacManageInfo(&payload) != true) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "virtual bool DB_SaveCharac::dispatch(int, int, Stream*)",
            0x3F7D,
            "[SAVE CHARAC MANAGE INFO ERROR] characNo(%u), tagCharacNo(%u), skillIndex(%u)",
            payload.charac_no,
            payload.tag_charac_no,
            payload.striker_skill_index);
    }
    if (payload.save_charac_stat && _SaveCharacStat(&payload) != true) {
        return false;
    }
    if (payload.save_cube_premium_info) {
        _SaveContractOfCubePremiumInfo(payload);
    }
    return true;
}
// IDA_EA: 0x8415DBE
// IDA_NAME: _ZN13DB_SaveCharac30_SaveContractOfCubePremiumInfoERK15SIG_SAVE_CHARAC
// DEMANGLED: DB_SaveCharac::_SaveContractOfCubePremiumInfo(SIG_SAVE_CHARAC const&)
bool DB_SaveCharac::_SaveContractOfCubePremiumInfo(const SIG_SAVE_CHARAC& payload_data) {
    if (payload_data.save_cube_premium_info != 1) {
        return true;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, SERVER_GROUP_DEFAULT);
    db_handle->set_query(
        "upDate cube_premium set selected=%u, cube_type=%u where charac_no=%u",
        payload_data.cube_premium_selected,
        payload_data.cube_premium_type,
        payload_data.charac_no);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (!db_handle->getAffectedRowCount()) {
        db_handle->set_query(
            "inSert into cube_premium(charac_no, selected, cube_type) values(%u,%u,%u)",
            payload_data.charac_no,
            payload_data.cube_premium_selected,
            payload_data.cube_premium_type);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}
// IDA_EA: 0x8415EDA
// IDA_NAME: _ZN13DB_SaveCharac15_SaveCharacInfoEP15SIG_SAVE_CHARAC
// DEMANGLED: DB_SaveCharac::_SaveCharacInfo(SIG_SAVE_CHARAC *)
bool DB_SaveCharac::_SaveCharacInfo(SIG_SAVE_CHARAC* payload_data) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, SERVER_GROUP_DEFAULT);
    if (payload_data->save_charac_info && payload_data->save_charac_etc) {
        char* spec_property = db_handle->blob_to_str(
            1, payload_data->stats.spec_property,
            sizeof(payload_data->stats.spec_property));
        char* element_resist = db_handle->blob_to_str(
            0, payload_data->stats.element_resist,
            sizeof(payload_data->stats.element_resist));
        db_handle->set_query(
            "upDate charac_info set lev=%d,grow_type=%d,maxHP=%d,maxMP=%d,phy_attack=%d,phy_defense=%d,mag_attack=%d,mag_defense=%d,inven_weight=%d,hp_regen=%d,mp_regen=%d,move_speed=%d,attack_speed=%d,cast_speed=%d,hit_recovery=%d,jump=%d,charac_weight=%d,element_resist='%s',spec_property='%s',max_premium_fatigue=%d,guild_right=%d,expert_job=%d,skill_tree_index=%d, event_charac_level=%d where charac_no = %u",
            payload_data->character_level,
            payload_data->grow_type,
            payload_data->stats.max_hp,
            payload_data->stats.max_mp,
            payload_data->stats.physical_attack,
            payload_data->stats.physical_defense,
            payload_data->stats.magical_attack,
            payload_data->stats.magical_defense,
            payload_data->stats.inventory_weight,
            payload_data->stats.hp_regen_speed,
            payload_data->stats.mp_regen_speed,
            payload_data->stats.move_speed,
            payload_data->stats.attack_speed,
            payload_data->stats.cast_speed,
            payload_data->stats.hit_recovery,
            payload_data->stats.jump_power,
            payload_data->stats.character_weight,
            element_resist,
            spec_property,
            payload_data->maximum_premium_fatigue,
            payload_data->guild_right,
            payload_data->expert_job_type,
            payload_data->skill_tree_index,
            payload_data->event_character_grow_type,
            payload_data->charac_no);
    } else if (payload_data->save_charac_info) {
        char* spec_property = db_handle->blob_to_str(
            1, payload_data->stats.spec_property,
            sizeof(payload_data->stats.spec_property));
        char* element_resist = db_handle->blob_to_str(
            0, payload_data->stats.element_resist,
            sizeof(payload_data->stats.element_resist));
        db_handle->set_query(
            "upDate charac_info set lev=%d,grow_type=%d,maxHP=%d,maxMP=%d,phy_attack=%d,phy_defense=%d,mag_attack=%d,mag_defense=%d,inven_weight=%d,hp_regen=%d,mp_regen=%d,move_speed=%d,attack_speed=%d,cast_speed=%d,hit_recovery=%d,jump=%d,charac_weight=%d,element_resist='%s',spec_property='%s', expert_job=%d, skill_tree_index=%d where charac_no = %u",
            payload_data->character_level,
            payload_data->grow_type,
            payload_data->stats.max_hp,
            payload_data->stats.max_mp,
            payload_data->stats.physical_attack,
            payload_data->stats.physical_defense,
            payload_data->stats.magical_attack,
            payload_data->stats.magical_defense,
            payload_data->stats.inventory_weight,
            payload_data->stats.hp_regen_speed,
            payload_data->stats.mp_regen_speed,
            payload_data->stats.move_speed,
            payload_data->stats.attack_speed,
            payload_data->stats.cast_speed,
            payload_data->stats.hit_recovery,
            payload_data->stats.jump_power,
            payload_data->stats.character_weight,
            element_resist,
            spec_property,
            payload_data->expert_job_type,
            payload_data->skill_tree_index,
            payload_data->charac_no);
    } else if (payload_data->save_charac_etc) {
        db_handle->set_query(
            "upDate charac_info set max_premium_fatigue=%d,guild_right=%d,event_charac_level=%d where charac_no = %u",
            payload_data->maximum_premium_fatigue,
            payload_data->guild_right,
            payload_data->event_character_grow_type,
            payload_data->charac_no);
    }
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x84163F2
// IDA_NAME: _ZN13DB_SaveCharac15_SaveCharacStatEP15SIG_SAVE_CHARAC
// DEMANGLED: DB_SaveCharac::_SaveCharacStat(SIG_SAVE_CHARAC *)
bool DB_SaveCharac::_SaveCharacStat(SIG_SAVE_CHARAC* payload_data) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, SERVER_GROUP_DEFAULT);
    char* escalade_tutorial_flag = db_handle->blob_to_str(
        0,
        payload_data->escalade_tutorial_flag,
        sizeof(payload_data->escalade_tutorial_flag));
    db_handle->set_query(
        "upDate charac_stat set village=%d,exp=%d,HP=%d,fatigue=%d,premium_fatigue=%d,last_play_time=from_unixtime(%d),used_fatigue=%d,tutorial_flag=%d,trade_gold_total=%d,trade_gold_total_billion=trade_gold_total_billion+%d,trade_gold_daily=%d,help_abuse_ratio=%d,help_abuse_exp=%d,chaos_point=%d,chaos_exp=%d,chaos_mode_count=%d,chaos_kill_count=%d,chaos_die_count=%d,chaos_die_time=from_unixtime(%d),assault_count=%d,luck_point=%d,dungeon_play_count=%d,chaos_kill_time=from_unixtime(%d),expert_job_exp=%d,open_flag=%d,fatigue_battery_charging=%d,escalade_tutorial_flag='%s',power_war_point=%d,village_prev=%d,power_war_assault_count=%d,power_war_assault_victory_count=%d,last_play_time_powerwar=from_unixtime(%d),fatigue_grownup_buff=%d, emotion=%d, add_slot_flag=%d, member_dungeon_flag=%d, member_bonus_fatigue=%d, last_play_dungeon_index=%u, add_equipslot_flag=%d, channel_equipslot_switch=%d, expand_equipslot_switch=%d, visible_flags=%d, growth_power_reward=%d, chaos_respon_time=from_unixtime(%d) where charac_no = %u",
        payload_data->village,
        payload_data->experience,
        payload_data->stamina,
        payload_data->fatigue,
        payload_data->premium_fatigue,
        payload_data->last_play_time,
        payload_data->used_fatigue,
        payload_data->tutorial_flag,
        payload_data->trade_gold_total,
        payload_data->trade_gold_total_billion,
        payload_data->trade_gold_daily,
        payload_data->help_abuse_ratio,
        payload_data->help_abuse_experience,
        payload_data->chaos_point,
        payload_data->chaos_experience,
        payload_data->chaos_mode_count,
        payload_data->chaos_kill_count,
        payload_data->chaos_die_count,
        payload_data->chaos_die_time,
        payload_data->assault_count,
        payload_data->luck_point,
        payload_data->dungeon_play_count,
        payload_data->chaos_kill_time,
        payload_data->expert_job_experience,
        payload_data->open_coin_flag,
        payload_data->fatigue_battery_charging,
        escalade_tutorial_flag,
        payload_data->power_war_point,
        payload_data->previous_village,
        payload_data->power_war_assault_count,
        payload_data->power_war_victory_count,
        payload_data->last_play_time_power_war,
        payload_data->fatigue_grownup_buff,
        payload_data->emotion,
        payload_data->expanded_slot_flag,
        payload_data->member_dungeon_flag,
        payload_data->member_bonus_fatigue,
        payload_data->last_play_dungeon_index,
        payload_data->equipslot_flag,
        payload_data->channel_equipslot_switch,
        payload_data->expand_equipslot_switch,
        payload_data->visible_flags,
        payload_data->growth_power_reward,
        payload_data->chaos_response_time,
        payload_data->charac_no);
    if (db_handle->exec(true) != true) {
        return false;
    }

    WongWork::CSimpleSSO::stUpdateSSOExpireTime_t update_data;
    update_data.report_count = payload_data->report_mannerless_count;
    update_data.trade_gold_daily = payload_data->over_trade_gold_count;
    update_data.dungeon_gain_gold = payload_data->dungeon_gained_gold;
    update_data.rating = payload_data->rating;
    update_data.tutorial_skipable = payload_data->tutorial_skipable;
    std::memset(update_data.reserved_11, 0, sizeof(update_data.reserved_11));
    return GlobalData::s_psimpleSSO->updateSSOExpireTime(
               payload_data->account_id, update_data) == true;
}

// IDA_EA: 0x8416904
// IDA_NAME: _ZN13DB_SaveCharac10SendResultEiiP15SIG_SAVE_CHARAC
// DEMANGLED: DB_SaveCharac::SendResult(int,int,SIG_SAVE_CHARAC *)
bool DB_SaveCharac::SendResult(int signal,
                               int user_idx,
                               SIG_SAVE_CHARAC* payload) {
    (void)signal;
    (void)user_idx;
    (void)payload;
    return true;
}

// IDA_EA: 0x841690E
// IDA_NAME: _ZN13DB_SaveCharac21_SaveCharacManageInfoEP15SIG_SAVE_CHARAC
// DEMANGLED: DB_SaveCharac::_SaveCharacManageInfo(SIG_SAVE_CHARAC *)
bool DB_SaveCharac::_SaveCharacManageInfo(SIG_SAVE_CHARAC* payload_data) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, SERVER_GROUP_DEFAULT);
    db_handle->set_query(
        "inSert into charac_manage_info(charac_no, tag_charac_no, striker_skill_index, max_equip_level) values(%u,%u,%u,%u)",
        payload_data->charac_no,
        payload_data->tag_charac_no,
        payload_data->striker_skill_index,
        payload_data->maximum_equip_level);
    if (db_handle->exec(true) != true) {
        if (db_handle->getDBError() != 1062) {
            return false;
        }
        db_handle->set_query(
            "upDate charac_manage_info set tag_charac_no=%u, striker_skill_index=%u, max_equip_level=%u where charac_no=%u",
            payload_data->tag_charac_no,
            payload_data->striker_skill_index,
            payload_data->maximum_equip_level,
            payload_data->charac_no);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}
