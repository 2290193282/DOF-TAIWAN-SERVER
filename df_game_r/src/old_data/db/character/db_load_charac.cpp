#include "old_data/db/character/db_load_charac.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/data_manager.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "infra/log/format/trace.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>
#include <string>
#include <string.h>


namespace {

enum {
    kEventBringUpEventType = 0x19B,
    kEventBringUpEventEntityType = 155,
};

}  // namespace

// IDA_EA: 0x844D8C6
// IDA_NAME: _ZN13DB_LoadCharacC2Ev
// DEMANGLED: DB_LoadCharac::DB_LoadCharac(void)
DB_LoadCharac::DB_LoadCharac() {}

// IDA_EA: 0x844D8E2
// IDA_NAME: _ZN13DB_LoadCharacD2Ev
// DEMANGLED: DB_LoadCharac::~DB_LoadCharac()
// IDA_EA: 0x844D912
// IDA_NAME: _ZN13DB_LoadCharacD0Ev
// DEMANGLED: DB_LoadCharac::~DB_LoadCharac() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_LoadCharac::~DB_LoadCharac() {}

// IDA_EA: 0x8408A3C
// IDA_NAME: _ZN13DB_LoadCharac8dispatchEiiP6Stream
// DEMANGLED: DB_LoadCharac::dispatch(int,int,Stream *)
bool DB_LoadCharac::dispatch(int signal, int user_idx, Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }

    SIG_LOAD_CHARAC* payload = stream->GetOutBuffer<SIG_LOAD_CHARAC>();
    payload->result = 0;

    if (_getCharacInfo(payload) != true) {
        payload->result = 1;
    } else if (_getCharacStat(payload) != true) {
        payload->result = 3;
    } else {
        CDataManager* data_manager = G_CDataManager();
        EventClassify::CEventScriptMng* event_script_mng =
            data_manager ? data_manager->get_event_script_mng() : 0;
        if (event_script_mng &&
            event_script_mng->is_eventing(
                                        kEventBringUpEventType) &&
            _LoadEventBringUpInfo(payload) != true) {
            payload->result = 5;
        }
    }

    _sendResult(signal, user_idx, payload);
    if (G_GameWorld()->IsSchoolPvPChannel() &&
        payload->charac_data.school_id) {
        _UpdateSchoolCharac(payload);
    }
    return true;
}

// IDA_EA: 0x8408C0A
// IDA_NAME: _ZN13DB_LoadCharac10_getGameDBEv
// DEMANGLED: DB_LoadCharac::_getGameDB(void)
MySQL* DB_LoadCharac::_getGameDB() {
    return GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
}

// IDA_EA: 0x8408C30
// IDA_NAME: _ZN13DB_LoadCharac19_UpdateSchoolCharacEP15SIG_LOAD_CHARAC
// DEMANGLED: DB_LoadCharac::_UpdateSchoolCharac(SIG_LOAD_CHARAC *)
bool DB_LoadCharac::_UpdateSchoolCharac(SIG_LOAD_CHARAC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(16),static_cast<ENUM_SERVER_GROUP>( 0));
    const char* account_id = NumberToString(payload->member_id, 0);
    db_handle->set_query(
        "upDate school_member set job=%d,grow_type=%d,lev=%d where school_id=%d and m_id=%s and charac_no=%d",
        payload->charac_data.job,
        payload->charac_data.grow_type,
        payload->charac_data.level,
        payload->charac_data.school_id,
        account_id,
        payload->character_no);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x8408CF4
// IDA_NAME: _ZN13DB_LoadCharac14_getCharacInfoEP15SIG_LOAD_CHARAC
// DEMANGLED: DB_LoadCharac::_getCharacInfo(SIG_LOAD_CHARAC *)
bool DB_LoadCharac::_getCharacInfo(SIG_LOAD_CHARAC* payload) {
    CHARAC_DATA& data = payload->charac_data;
    CREATE_CHARACTER_INITIAL_STATS& stats = data.db_stats;
    MySQL* db_handle = _getGameDB();
    db_handle->set_query(
        "seLect charac_no,charac_name,job,lev,grow_type,maxHP,maxMP,phy_attack,phy_defense,mag_attack,mag_defense,inven_weight,hp_regen,mp_regen,move_speed, attack_speed, cast_speed, hit_recovery, jump, charac_weight, max_fatigue, create_time,guild_id,guild_right,max_premium_fatigue,member_flag,element_resist,spec_property,m_id,expert_job, skill_tree_index, event_charac_level, guild_secede from charac_info where charac_no=%d and delete_flag!=1",
        payload->character_no);
    if (db_handle->exec(true) != true || db_handle->fetch() != true) {
        return false;
    }
    if (db_handle->get_n_rows() != 1) {
        return false;
    }
    if (db_handle->get_int(0, data.character_no) != true) return false;
    if (db_handle->get_str(1, data.character_name, 29) != true) return false;
    if (db_handle->get_byte(2, data.job) != true) return false;
    if (db_handle->get_short(3, data.level) != true) return false;
    if (db_handle->get_byte(4, data.grow_type) != true) return false;
    if (db_handle->get_uint(5, stats.max_hp) != true) return false;
    if (db_handle->get_uint(6, stats.max_mp) != true) return false;
    if (db_handle->get_ushort(7, stats.physical_attack) != true) return false;
    if (db_handle->get_ushort(8, stats.physical_defense) != true) return false;
    if (db_handle->get_ushort(9, stats.magical_attack) != true) return false;
    if (db_handle->get_ushort(10, stats.magical_defense) != true) return false;
    if (db_handle->get_int(11, stats.inventory_weight) != true) return false;
    if (db_handle->get_short(12, stats.hp_regeneration) != true) return false;
    if (db_handle->get_short(13, stats.mp_regeneration) != true) return false;
    if (db_handle->get_uint(14, stats.move_speed) != true) return false;
    if (db_handle->get_ushort(15, stats.attack_speed) != true) return false;
    if (db_handle->get_ushort(16, stats.cast_speed) != true) return false;
    if (db_handle->get_short(17, stats.hit_recovery) != true) return false;
    if (db_handle->get_short(18, stats.jump) != true) return false;
    if (db_handle->get_int(19, stats.character_weight) != true) return false;
    if (db_handle->get_ushort(20, data.maximum_fatigue) != true) return false;
    if (db_handle->get_str(21, data.create_time, 24) != true) return false;
    if (db_handle->get_uint(22, data.guild_id) != true) return false;
    if (db_handle->get_ubyte(23, data.guild_right) != true) return false;
    if (db_handle->get_ushort(24, data.maximum_premium_fatigue) != true) return false;
    if (db_handle->get_ubyte(25, data.member_flag) != true) return false;
    if (db_handle->get_binary(26, stats.element_resist,
                              sizeof(stats.element_resist)) != true) return false;
    if (db_handle->get_binary(27, stats.spec_property,
                              sizeof(stats.spec_property)) != true) return false;
    if (db_handle->get_uint(28, data.member_id) != true) return false;
    if (db_handle->get_byte(29, data.expert_job) != true) return false;
    if (db_handle->get_byte(30, data.skill_tree_index) != true) return false;
    if (db_handle->get_byte(31, data.event_character_grow_type) != true) return false;
    if (db_handle->get_byte(32, data.guild_secede) != true) return false;
    if (!data.tag_character_no) {
        return true;
    }
    return _GetTagCharacInfo(data.tag_character_no,
                             &payload->tag_charac_data);
}

// IDA_EA: 0x84093F6
// IDA_NAME: _ZN13DB_LoadCharac14_getCharacStatEP15SIG_LOAD_CHARAC
// DEMANGLED: DB_LoadCharac::_getCharacStat(SIG_LOAD_CHARAC *)
bool DB_LoadCharac::_getCharacStat(SIG_LOAD_CHARAC* payload) {
    CHARAC_DATA& data = payload->charac_data;
    MySQL* db_handle = _getGameDB();
    db_handle->set_query(
        "seLect village,exp,HP,fatigue,premium_fatigue,unix_timestamp(last_play_time),used_fatigue,tutorial_flag,trade_gold_total,trade_gold_daily,open_flag,chaos_point,chaos_exp,chaos_mode_count,chaos_kill_count,chaos_die_count,unix_timestamp(chaos_die_time),assault_count,luck_point,dungeon_play_count,unix_timestamp(chaos_kill_time),help_abuse_ratio,help_abuse_exp,expert_job_exp,fatigue_battery_charging,escalade_tutorial_flag,power_war_point,village_prev,power_war_assault_count,power_war_assault_victory_count,unix_timestamp(last_play_time_powerwar),fatigue_grownup_buff,emotion,add_slot_flag,member_dungeon_flag,member_bonus_fatigue,last_play_dungeon_index, add_equipslot_flag, channel_equipslot_switch, expand_equipslot_switch, visible_flags, growth_power_reward, unix_timestamp(chaos_respon_time) from charac_stat where charac_no=%u",
        payload->character_no);
    if (db_handle->exec(true) != true || db_handle->fetch() != true) {
        return false;
    }
    if (db_handle->get_byte(0, data.village) != true) return false;
    if (db_handle->get_uint(1, data.experience) != true) return false;
    if (db_handle->get_ubyte(2, data.hp) != true) return false;
    if (db_handle->get_ushort(3, data.fatigue) != true) return false;
    if (db_handle->get_ushort(4, data.premium_fatigue) != true) return false;
    if (db_handle->get_uint(5, data.last_play_time) != true) return false;
    if (db_handle->get_ushort(6, data.used_fatigue) != true) return false;
    if (db_handle->get_uint(7, data.tutorial_flag) != true) return false;
    if (db_handle->get_uint(8, data.trade_gold_total) != true) return false;
    if (db_handle->get_uint(9, data.trade_gold_daily) != true) return false;
    data.trade_gold_total_billion = 0;
    if (db_handle->get_byte(10, data.open_flag) != true) return false;
    if (db_handle->get_int(11, data.chaos_point) != true) return false;
    if (db_handle->get_int(12, data.chaos_experience) != true) return false;
    if (db_handle->get_int(13, data.chaos_mode_count) != true) return false;
    if (db_handle->get_int(14, data.chaos_kill_count) != true) return false;
    if (db_handle->get_int(15, data.chaos_die_count) != true) return false;
    if (db_handle->get_int(16, data.chaos_die_time) != true) return false;
    if (db_handle->get_int(17, data.assault_count) != true) return false;
    if (db_handle->get_int(18, data.luck_point) != true) return false;
    if (db_handle->get_int(19, data.dungeon_play_count) != true) return false;
    if (db_handle->get_int(20, data.chaos_kill_time) != true) return false;
    if (db_handle->get_uint(21, data.help_abuse_ratio) != true) return false;
    if (db_handle->get_uint(22, data.help_abuse_experience) != true) return false;
    if (db_handle->get_int(23, data.expert_job_experience) != true) return false;
    if (db_handle->get_short(24, data.fatigue_battery_charging) != true) return false;
    if (db_handle->get_binary(25, data.escalade_tutorial_flag,
                              sizeof(data.escalade_tutorial_flag)) != true) return false;
    if (db_handle->get_short(26, data.power_war_point) != true) return false;
    if (db_handle->get_byte(27, data.previous_village) != true) return false;
    if (db_handle->get_uint(28, data.power_war_assault_count) != true) return false;
    if (db_handle->get_uint(29, data.power_war_victory_count) != true) return false;
    if (db_handle->get_uint(30, data.power_war_last_play_time) != true) return false;
    if (db_handle->get_ushort(31, data.fatigue_grownup_buff) != true) return false;
    if (db_handle->get_short(32, data.emotion) != true) return false;
    if (db_handle->get_byte(33, data.add_slot_flag) != true) return false;
    if (db_handle->get_byte(34, data.member_dungeon_flag) != true) return false;
    if (db_handle->get_short(35, data.member_bonus_fatigue) != true) return false;
    if (db_handle->get_uint(36, data.last_play_dungeon_index) != true) return false;
    if (db_handle->get_byte(37, data.add_equipslot_flag) != true) return false;
    if (db_handle->get_byte(38, data.channel_equipslot_switch) != true) return false;
    if (db_handle->get_byte(39, data.expand_equipslot_switch) != true) return false;
    if (db_handle->get_byte(40, data.visible_flags) != true) return false;
    if (db_handle->get_ubyte(41, data.growth_power_reward) != true) return false;
    return db_handle->get_int(42, data.chaos_response_time) == true;
}

// IDA_EA: 0x8409D2E
// IDA_NAME: _ZN13DB_LoadCharac21_LoadEventBringUpInfoEP15SIG_LOAD_CHARAC
// DEMANGLED: DB_LoadCharac::_LoadEventBringUpInfo(SIG_LOAD_CHARAC *)
bool DB_LoadCharac::_LoadEventBringUpInfo(SIG_LOAD_CHARAC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    CDataManager* data_manager = G_CDataManager();
    EventClassify::CEventScriptMng* event_script_mng =
        data_manager ? data_manager->get_event_script_mng() : 0;
    const std::string* table_name = event_script_mng
        ? event_script_mng->get_db_table_name(
                                            kEventBringUpEventEntityType)
        : 0;
    if (!table_name) {
        return false;
    }

    const char* account_id = NumberToString(payload->member_id, 0);
    const char* table_name_text = ::toTString(*table_name);
    db_handle->set_query(
        "seLect use_material_cnt, give_reward_flag from %s where charac_no=%u and m_id=%s",
        table_name_text,
        payload->character_no,
        account_id);
    if (db_handle->exec(true) != true) {
        cMyTrace trace("bool DB_LoadCharac::_LoadEventBringUpInfo(SIG_LOAD_CHARAC*)",
                       8795,
                       5);
        trace("DB_LoadCharac::_LoadEventBringUpInfo, exec() ERROR charac_no=%u",
              payload->character_no);
        return false;
    }
    if (!db_handle->get_n_rows()) {
        return true;
    }
    if (db_handle->fetch() != true) {
        return false;
    }
    unsigned char use_material_cnt = 0;
    unsigned char give_reward_flag = 0;
    if (db_handle->get_ubyte(0, use_material_cnt) != true) {
        cMyTrace trace("bool DB_LoadCharac::_LoadEventBringUpInfo(SIG_LOAD_CHARAC*)",
                       8811,
                       5);
        trace("DB_LoadCharac::_LoadEventBringUpInfo, get_ubyte(1) ERROR charac_no=%u",
              payload->character_no);
        return false;
    }
    if (db_handle->get_ubyte(1, give_reward_flag) != true) {
        cMyTrace trace("bool DB_LoadCharac::_LoadEventBringUpInfo(SIG_LOAD_CHARAC*)",
                       8818,
                       5);
        trace("DB_LoadCharac::_LoadEventBringUpInfo, get_ubyte(2) ERROR charac_no=%u",
              payload->character_no);
        return false;
    }
    payload->charac_data.event_bring_up_material_count = use_material_cnt;
    payload->charac_data.event_bring_up_reward_flag =
        give_reward_flag != 0;
    return true;
}

// IDA_EA: 0x8409FA6
// IDA_NAME: _ZN13DB_LoadCharac17_GetTagCharacInfoEjP15TAG_CHARAC_DATA
// DEMANGLED: DB_LoadCharac::_GetTagCharacInfo(unsigned int,TAG_CHARAC_DATA *)
bool DB_LoadCharac::_GetTagCharacInfo(unsigned int charac_no,
                                      TAG_CHARAC_DATA* tag_data) {
    CREATE_CHARACTER_INITIAL_STATS& stats = tag_data->db_stats;
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect charac_no, charac_name, job, lev, grow_type, skill_tree_index, maxHP, maxMP, phy_attack, phy_defense, mag_attack, mag_defense, element_resist, spec_property, inven_weight, hp_regen, mp_regen, move_speed, attack_speed, cast_speed, hit_recovery, jump, charac_weight from charac_info where charac_no = %d and delete_flag != 1",
        charac_no);
    if (db_handle->exec(true) != true || db_handle->fetch() != true) {
        return false;
    }
    if (!db_handle->getAffectedRowCount()) {
        return false;
    }
    if (db_handle->get_int(0, tag_data->character_no) != true) return false;
    if (db_handle->get_str(1, tag_data->character_name, 29) != true) return false;
    if (db_handle->get_byte(2, tag_data->job) != true) return false;
    if (db_handle->get_short(3, tag_data->level) != true) return false;
    if (db_handle->get_byte(4, tag_data->grow_type) != true) return false;
    if (db_handle->get_byte(5, tag_data->skill_tree_index) != true) return false;
    if (db_handle->get_uint(6, stats.max_hp) != true) return false;
    if (db_handle->get_uint(7, stats.max_mp) != true) return false;
    if (db_handle->get_ushort(8, stats.physical_attack) != true) return false;
    if (db_handle->get_ushort(9, stats.physical_defense) != true) return false;
    if (db_handle->get_ushort(10, stats.magical_attack) != true) return false;
    if (db_handle->get_ushort(11, stats.magical_defense) != true) return false;
    if (db_handle->get_binary(12, stats.element_resist,
                              sizeof(stats.element_resist)) != true) return false;
    if (db_handle->get_binary(13, stats.spec_property,
                              sizeof(stats.spec_property)) != true) return false;
    if (db_handle->get_int(14, stats.inventory_weight) != true) return false;
    if (db_handle->get_short(15, stats.hp_regeneration) != true) return false;
    if (db_handle->get_short(16, stats.mp_regeneration) != true) return false;
    if (db_handle->get_uint(17, stats.move_speed) != true) return false;
    if (db_handle->get_ushort(18, stats.attack_speed) != true) return false;
    if (db_handle->get_ushort(19, stats.cast_speed) != true) return false;
    if (db_handle->get_short(20, stats.hit_recovery) != true) return false;
    if (db_handle->get_short(21, stats.jump) != true) return false;
    return db_handle->get_int(22, stats.character_weight) == true;
}

// IDA_EA: 0x840A4CE
// IDA_NAME: _ZN13DB_LoadCharac11_sendResultEiiP15SIG_LOAD_CHARAC
// DEMANGLED: DB_LoadCharac::_sendResult(int,int,SIG_LOAD_CHARAC *)
void DB_LoadCharac::_sendResult(int signal,
                                int user_idx,
                                SIG_LOAD_CHARAC* payload) {
    (void)signal;
    Stream* out_stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 9018);
    CStreamGuard guard(out_stream, true);
    *guard << 7;
    *guard << user_idx;
    SIG_LOAD_CHARAC* dest = guard->GetInBuffer<SIG_LOAD_CHARAC>();
    memcpy(dest, payload, sizeof(SIG_LOAD_CHARAC));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);

    cMyTrace trace("void DB_LoadCharac::_sendResult(int, int, SIG_LOAD_CHARAC*)",
                   9031,
                   0);
    trace("CharLastPlayTick char(%d) t(%d)",
          payload->character_no,
          payload->charac_data.last_play_time);
}
