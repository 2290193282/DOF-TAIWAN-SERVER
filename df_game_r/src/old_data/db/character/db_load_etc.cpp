#include "old_data/db/character/db_load_etc.h"
#include "old_data/db/character/db_create_charac.h"
#include "old_data/db/npc/quest_shop_persistence_layout.h"

#include "common/config.h"
#include "old_game/player/setting/character_option.h"
#include "old_game/player/core/character_tag_data.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/economy/shop/npc_dialog_value_types.h"
#include "old_game/economy/mall/quest_shop_runtime.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/activity/schedule/event_messages.h"
#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/data_manager.h"
#include "common/manager/db_mgr.h"
#include "old_game/activity/reward/event_base.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <time.h>

extern int uncompress_zip(char* dest, unsigned int& dest_len,
                          const char* source, unsigned int source_len);


namespace EventClassify {
class CEventEntity;
class CUsedFatigueGiveItem;
}  // namespace EventClassify


extern unsigned char get_compressed_blob_data(MySQL* db_handle,
                                             int column,
                                             unsigned char* dest,
                                             int dest_len,
                                             int unit_size)
    __asm__("_Z24get_compressed_blob_dataP5MySQLiPcii");
extern unsigned char get_compressed_best_clear_time(
    MySQL* db_handle,
    int column,
    SIG_BEST_CLEAR_TIME_DATA* dest,
    int dest_len,
    int unit_size)
    __asm__("_Z24get_compressed_blob_dataP5MySQLiPcii");
extern const char* get_skill_table_name(unsigned char table_index)
    __asm__("_Z20get_skill_table_nameh");
extern const char* getComboSkillTableName(ENUM_SKILL_TREE_KIND skill_tree_kind)
    __asm__("_Z22getComboSkillTableName20ENUM_SKILL_TREE_KIND");
extern long getDailyInitializeTime(int hour, int minute, int second)
    __asm__("_Z22getDailyInitializeTimeiii");

namespace WongWork {
namespace EventCommon {
void LoadCharacEventItem(unsigned int account_id,
                         unsigned int charac_no,
                         unsigned int event_item_id);
}  // namespace EventCommon
}  // namespace WongWork

namespace {

}  // namespace

// IDA_EA: 0x8403B84
// IDA_NAME: _ZN10DB_LoadEtcC2Ev
// DEMANGLED: DB_LoadEtc::DB_LoadEtc(void)
DB_LoadEtc::DB_LoadEtc() {}

// IDA_EA: 0x8403BA0
// IDA_NAME: _ZN10DB_LoadEtcD2Ev
// DEMANGLED: DB_LoadEtc::~DB_LoadEtc()
// IDA_EA: 0x8403BD0
// IDA_NAME: _ZN10DB_LoadEtcD0Ev
// DEMANGLED: DB_LoadEtc::~DB_LoadEtc() deleting destructor
DB_LoadEtc::~DB_LoadEtc() {}

// IDA_EA: 0x8403BEE
// IDA_NAME: _ZN10DB_LoadEtc8dispatchEiiP6Stream
// DEMANGLED: DB_LoadEtc::dispatch(int,int,Stream *)
bool DB_LoadEtc::dispatch(int signal, int user_idx, Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }
    SIG_LOAD_ETC* payload = stream->GetOutBuffer<SIG_LOAD_ETC>();
    if (!payload) {
        return false;
    }

    GetUserRecord(payload);
    const int pvp_result = GetUserPvp(payload);
    if (pvp_result == 0) {
        const int pvp_base = GetUserPvpBase(payload);
        if (pvp_base == -1) {
            payload->pvp_result.Clear();
        } else if (pvp_base == 1) {
            payload->pvp_result.values[4] = -1;
        }
    } else if (pvp_result == 1) {
        DB_CreateCharac create_charac;
        create_charac.CreatePvP(payload->charac_no);
    } else if (pvp_result == -1) {
        payload->pvp_result.Clear();
    }

    if (LoadCharacItemStat(payload) != true) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "virtual bool DB_LoadEtc::dispatch(int, int, Stream*)",
            0x14C6,
            "DB_LoadETC ERROR : LoadCharacItemStat Fail");
        return false;
    }
    if (LoadLitmitItem(payload) != true) {
        cMyTrace trace("virtual bool DB_LoadEtc::dispatch(int, int, Stream*)",
                       5339,
                       0);
        trace("LoadLimitItem middle fetch <charac:%d>",
              payload->charac_no);
        return false;
    }

    if (GetUserSkill(payload) != true) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "virtual bool DB_LoadEtc::dispatch(int, int, Stream*)",
            0x14E3,
            "DB_LoadETC ERROR : GetUserSkill Fail");
        payload->result = 1;
    }

    CEventBase* repeat_event = GlobalData::s_event_manager->GetRepeatEvent(113);
    if (repeat_event->IsEventing(0)) {
        LoadConditionEventInfo(payload);
    }
    LoadProperDungeonClearCount(payload);
    LoadRedeemItem(payload);
    LoadPvPMissionList(payload);
    LoadTODUserState(payload);
    LoadVillageAttackInfo(payload);
    LoadAccountFirstLogin(payload, 145);
    LoadAccountFirstLogin(payload, 146);
    LoadAccountFirstLoginDaily(payload, 156);
    LoadFairPvPScore(payload);
    if (LoadItemMakingSkill(payload) != true) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "virtual bool DB_LoadEtc::dispatch(int, int, Stream*)",
            0x1566,
            "DB_LoadETC ERROR : LoadItemMakingSkill Fail");
        return false;
    }

    LoadQuestShop(payload);
    LoadTitleBook(payload);
    LoadAchievement(payload);
    LoadOnceGiveAvengerTitle(payload);
    LoadAuraAvatarOption(payload);
    LoadCharacOption(signal, user_idx, payload);
    LoadBoosterGage(payload);
    LoadPcRoomPlayTimeInfo(payload);
    LoadUsedGiftFatigueInfo(payload);
    LoadAttendanceInfo(&payload->attendance, payload->account_id);
    LoadComboSkillInfo(payload);
    LoadSeriaRoomAniDeco(payload);
    if (_loadNPCRelasionship(payload) != 0) {
        payload->result = 1;
    }

    const bool sent = SendResult(signal, user_idx, payload);
    if (sent == true) {
        WongWork::EventCommon::LoadCharacEventItem(payload->account_id,
                                                   payload->charac_no,
                                                   0);
        return true;
    }
    return false;
}

// IDA_EA: 0x84040B4
// IDA_NAME: _ZN10DB_LoadEtc19LoadItemMakingSkillEP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::LoadItemMakingSkill(SIG_LOAD_ETC *)
bool DB_LoadEtc::LoadItemMakingSkill(SIG_LOAD_ETC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, SERVER_GROUP_DEFAULT);
    db_handle->set_query(
        "seLect weapon, cloth, leather, light_armor, heavy_armor, plate, amulet, wrist, ring, support, magic_stone from item_making_skill_info where charac_no=%u",
        payload->charac_no);
    if (db_handle->exec(true) != true) {
        cMyTrace trace("bool DB_LoadEtc::LoadItemMakingSkill(SIG_LOAD_ETC*)",
                       5597,
                       5);
        trace("DB_LoadEtc::LoadItemMakingSkill, exec() ERROR charac_no=%u",
              payload->charac_no);
        return false;
    }
    if (!db_handle->get_n_rows()) {
        return true;
    }
    if (db_handle->fetch() != true) {
        cMyTrace trace("bool DB_LoadEtc::LoadItemMakingSkill(SIG_LOAD_ETC*)",
                       5608,
                       5);
        trace("DB_LoadEtc::LoadItemMakingSkill fetch, exec() ERROR charac_no=%u",
              payload->charac_no);
        return false;
    }
    SIG_ITEM_MAKING_SKILL& item_making = payload->item_making_skill;
    int16_t* skill_levels[11] = {
        &item_making.weapon,
        &item_making.cloth,
        &item_making.leather,
        &item_making.light_armor,
        &item_making.heavy_armor,
        &item_making.plate,
        &item_making.amulet,
        &item_making.wrist,
        &item_making.ring,
        &item_making.support,
        &item_making.magic_stone,
    };
    for (int index = 0; index <= 10; ++index) {
        if (db_handle->get_short(index, *skill_levels[index]) != true) {
            cMyTrace trace(
                "bool DB_LoadEtc::LoadItemMakingSkill(SIG_LOAD_ETC*)",
                5618,
                5);
            trace("DB_LoadEtc::LoadItemMakingSkill get(%d) ERROR charac_no=%u",
                  index,
                  payload->charac_no);
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x8404290
// IDA_NAME: _ZN10DB_LoadEtc16LoadFairPvPScoreEP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::LoadFairPvPScore(SIG_LOAD_ETC *)
bool DB_LoadEtc::LoadFairPvPScore(SIG_LOAD_ETC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect private_win,private_lose,private_draw,relay_battle_win, relay_battle_lose, relay_battle_draw,relay_battle_2kill, relay_battle_3kill, successive_win, max_successive_win, job_score,daily_play_count, unix_timestamp(last_play_time), give_item from fair_pvp_score where charac_no=%u",
        payload->charac_no);
    if (db_handle->exec(true) != true) {
        cMyTrace trace("bool DB_LoadEtc::LoadFairPvPScore(SIG_LOAD_ETC*)",
                       5656,
                       5);
        trace("DB_LoadSkill::LoadFairPvPScore, exec() ERROR charac_no=%u",
              payload->charac_no);
        return false;
    }
    if (!db_handle->get_n_rows()) {
        db_handle->set_query(
            "inSert into fair_pvp_score (charac_no) values(%u)",
            payload->charac_no);
        if (db_handle->exec(true) == true) {
            return true;
        }
        cMyTrace trace("bool DB_LoadEtc::LoadFairPvPScore(SIG_LOAD_ETC*)",
                       5669,
                       5);
        trace("DB_LoadEtc::LoadFairPvPScore insert, exec() ERROR charac_no=%u",
              payload->charac_no);
        return false;
    }
    if (db_handle->fetch() != true) {
        cMyTrace trace("bool DB_LoadEtc::LoadFairPvPScore(SIG_LOAD_ETC*)",
                       5679,
                       5);
        trace("DB_LoadEtc::LoadFairPvPScore, fetch() ERROR charac_no=%u",
              payload->charac_no);
        return false;
    }

    SIG_FAIR_PVP_SCORE& fair_pvp = payload->fair_pvp;
    int column = 0;
    for (int index = 0; index <= 1; ++index) {
        unsigned int& win =
            reinterpret_cast<unsigned int&>(fair_pvp.mode_totals[index].win);
        if (db_handle->get_uint(column++, win) != true) {
            return false;
        }
        unsigned int& lose =
            reinterpret_cast<unsigned int&>(fair_pvp.mode_totals[index].lose);
        if (db_handle->get_uint(column++, lose) != true) {
            return false;
        }
        unsigned int& draw =
            reinterpret_cast<unsigned int&>(fair_pvp.mode_totals[index].draw);
        if (db_handle->get_uint(column++, draw) != true) {
            return false;
        }
    }
    if (db_handle->get_uint(column++, fair_pvp.relay_two_kill) != true)
        return false;
    if (db_handle->get_uint(column++, fair_pvp.relay_three_kill) != true)
        return false;
    if (db_handle->get_uint(column++, fair_pvp.successive_win) != true)
        return false;
    if (db_handle->get_uint(column++, fair_pvp.maximum_successive_win) != true)
        return false;
    if (get_compressed_blob_data(
            db_handle,
            column++,
            reinterpret_cast<unsigned char*>(&fair_pvp.job_score),
            sizeof(fair_pvp.job_score),
            0) != 1) {
        return false;
    }
    if (db_handle->get_uint(column++, fair_pvp.daily_play_count) != true)
        return false;
    if (db_handle->get_int(column++, fair_pvp.last_play_time) != true)
        return false;
    if (fair_pvp.last_play_time < getDailyInitializeTime(6, 0, 0)) {
        fair_pvp.daily_play_count = 0;
    }
    return db_handle->get_ubyte(column++, fair_pvp.give_item) == true;
}

// IDA_EA: 0x8404A68
// IDA_NAME: _ZN10DB_LoadEtc13LoadQuestShopEP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::LoadQuestShop(SIG_LOAD_ETC *)
bool DB_LoadEtc::LoadQuestShop(SIG_LOAD_ETC* payload) {
    if (_LoadQuestShop(payload->charac_no, &payload->quest_shop.current) !=
        true) {
        cMyTrace trace("bool DB_LoadEtc::LoadQuestShop(SIG_LOAD_ETC*)",
                       5828,
                       5);
        trace("DB_LoadInventory::_LoadQuestShop ERROR::charac_no(%d)",
              payload->charac_no);
        return false;
    }
    if (payload->tag_charac_no) {
        if (_LoadQuestShop(payload->tag_charac_no,
                           &payload->quest_shop.tagged) != true) {
            cMyTrace trace("bool DB_LoadEtc::LoadQuestShop(SIG_LOAD_ETC*)",
                           5845,
                           5);
            trace("DB_LoadInventory::_LoadQuestShop ERROR::tag_charac_no(%d)",
                  payload->tag_charac_no);
            return false;
        }
        payload->quest_shop.has_tagged = 1;
    }
    return true;
}

// IDA_EA: 0x8404B72
// IDA_NAME: _ZN10DB_LoadEtc14_LoadQuestShopEjP22CHARAC_QUEST_SHOP_DATA
// DEMANGLED: DB_LoadEtc::_LoadQuestShop(unsigned int,CHARAC_QUEST_SHOP_DATA *)
bool DB_LoadEtc::_LoadQuestShop(unsigned int charac_no,
                                CHARAC_QUEST_SHOP_DATA* quest_shop) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect qp, init_count, max_hp, max_mp, psy_attack, psy_defense, mag_attack, mag_defence, move_speed, attack_speed, hp_regen, mp_regen, all_element_resist, fire_element_resist, water_element_resist, light_element_resist, dark_element_resist, all_element_attack,fire_element_attack, water_element_attack, light_element_attack, dark_element_attack, psy_critical, mag_critical, good_hit , evasion , hit_recovery , separate_psy_mag_attack , quest_piece from charac_quest_shop where charac_no=%u",
        charac_no);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->get_n_rows() == 0) {
        return true;
    }
    if (db_handle->fetch() != true) {
        return false;
    }
    unsigned int quest_point = 0;
    unsigned int init_count = 0;
    if (db_handle->get_uint(0, quest_point) != true)
        return false;
    if (db_handle->get_uint(1, init_count) != true)
        return false;
    quest_shop->quest_point = quest_point;
    quest_shop->init_count = init_count;
    for (int index = 0;
         index < quest_shop_persistence::kBuyingStatusColumnCount;
         ++index) {
        unsigned short buying_count = 0;
        if (db_handle->get_ushort(index + 2, buying_count) != true) {
            cMyTrace trace(
                "static bool DB_LoadEtc::_LoadQuestShop(unsigned int, CHARAC_QUEST_SHOP_DATA*)",
                6129 + index,
                5);
            trace("DB_LoadEtc::LoadQuestShope, get(%d) ERROR charac_no=%u",
                  index + 2,
                  charac_no);
            return false;
        }
        quest_shop_persistence::WriteBuyingStatusColumn(
            quest_shop, index, buying_count);
    }
    unsigned int quest_piece = 0;
    if (db_handle->get_uint(28, quest_piece) != true) {
        return false;
    }
    quest_shop->quest_piece = quest_piece;
    return true;
}

// IDA_EA: 0x8405C60
// IDA_NAME: _ZN10DB_LoadEtc14LoadLitmitItemEP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::LoadLitmitItem(SIG_LOAD_ETC *)
bool DB_LoadEtc::LoadLitmitItem(SIG_LOAD_ETC* payload) {
    (void)payload;
    return true;
}

// IDA_EA: 0x8405C6E
// IDA_NAME: _ZN10DB_LoadEtc21LoadAccountFirstLoginEP12SIG_LOAD_ETCi
// DEMANGLED: DB_LoadEtc::LoadAccountFirstLogin(SIG_LOAD_ETC *,int)
bool DB_LoadEtc::LoadAccountFirstLogin(SIG_LOAD_ETC* payload,
                                       int event_type) {
    EventClassify::CEventScriptMng* event_script_mng =
        G_CDataManager()->get_event_script_mng();
    if (event_script_mng->is_eventing(event_type) != 1) {
        return false;
    }

    EventClassify::CEventEntity* event_entity =
        event_script_mng->get_event_entity(event_type);
    if (!event_entity) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0));
    const std::string& table_name =
        event_entity->get_db_table_name();
    const char* table_name_text = ::toTString(table_name);
    const char* account_id = NumberToString(payload->account_id, 0);
    db_handle->set_query("seLect m_id from %s where m_id=%s",
                         table_name_text,
                         account_id);

    const int bit_index =
        event_script_mng->get_convert_to_user_confirmflag(
                                                        event_type);
    if (db_handle->exec(true) == true) {
        if (db_handle->get_n_rows()) {
            payload->user_confirm_flags.set(
                static_cast<size_t>(bit_index), true);
        } else {
            payload->user_confirm_flags.reset(static_cast<size_t>(bit_index));
        }
        return true;
    }
    payload->user_confirm_flags.set(static_cast<size_t>(bit_index), true);
    return false;
}

// IDA_EA: 0x8405E0A
// IDA_NAME: _ZN10DB_LoadEtc26LoadAccountFirstLoginDailyEP12SIG_LOAD_ETCi
// DEMANGLED: DB_LoadEtc::LoadAccountFirstLoginDaily(SIG_LOAD_ETC *,int)
bool DB_LoadEtc::LoadAccountFirstLoginDaily(SIG_LOAD_ETC* payload,
                                            int event_type) {
    EventClassify::CEventScriptMng* event_script_mng =
        G_CDataManager()->get_event_script_mng();
    if (event_script_mng->is_eventing(event_type) != 1) {
        return false;
    }
    EventClassify::CEventEntity* event_entity =
        event_script_mng->get_event_entity(event_type);
    if (!event_entity) {
        return false;
    }
    const int today_time = event_script_mng->get_today_time();
    if (event_entity->get_event_start_time() > today_time ||
        event_entity->get_event_end_time() < today_time) {
        return false;
    }

    time_t timer = time(0);
    tm current_tm;
    localtime_r(&timer, &current_tm);
    tm reset_tm = current_tm;
    reset_tm.tm_hour = G_CEnvironment()->get_fatigue_time();
    reset_tm.tm_min = 0;
    reset_tm.tm_sec = 0;
    time_t reset_time = mktime(&reset_tm);
    if (current_tm.tm_hour < reset_tm.tm_hour) {
        reset_time -= 86400;
    }

    const std::string& table_name =
        event_entity->get_db_table_name();
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect m_id from %s where m_id=%u and occ_date > FROM_UNIXTIME(%d)",
        ::toTString(table_name),
        payload->account_id,
        static_cast<int>(reset_time));

    const int bit_index =
        event_script_mng->get_convert_to_user_confirmflag(
                                                        event_type);
    if (db_handle->exec(true) == true) {
        if (db_handle->get_n_rows()) {
            payload->user_confirm_flags.set(
                static_cast<size_t>(bit_index), true);
        } else {
            payload->user_confirm_flags.reset(static_cast<size_t>(bit_index));
        }
        return true;
    }
    payload->user_confirm_flags.set(static_cast<size_t>(bit_index), true);
    return false;
}

// IDA_EA: 0x8406092
// IDA_NAME: _ZN10DB_LoadEtc13LoadTitleBookEP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::LoadTitleBook(SIG_LOAD_ETC *)
bool DB_LoadEtc::LoadTitleBook(SIG_LOAD_ETC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect general_section, specific_section, despair, event from charac_titlebook where charac_no=%u",
        payload->charac_no);
    if (db_handle->exec(true) != true) {
        cMyTrace trace("bool DB_LoadEtc::LoadTitleBook(SIG_LOAD_ETC*)",
                       6592,
                       5);
        trace("LoadTitleBook(), exec() ERROR charac_no=%u",
              payload->charac_no);
        return false;
    }
    if (!db_handle->get_n_rows() || db_handle->fetch() != true) {
        return false;
    }

    char blob[14652];
    memset(blob, 0, sizeof(blob));
    if (db_handle->get_binary(0, blob, 5136) != true) return false;
    unsigned int raw_len = 4270;
    if (uncompress_zip(reinterpret_cast<char*>(payload->title_book.category_0),
                       raw_len,
                       blob,
                       db_handle->get_binary_length(0)) != 1) {
        return false;
    }
    memset(blob, 0, sizeof(blob));
    if (db_handle->get_binary(1, blob, 14652) != true) return false;
    raw_len = 12200;
    if (uncompress_zip(reinterpret_cast<char*>(payload->title_book.category_1),
                       raw_len,
                       blob,
                       db_handle->get_binary_length(1)) != 1) {
        return false;
    }
    memset(blob, 0, sizeof(blob));
    if (db_handle->get_binary(2, blob, 8796) != true) return false;
    raw_len = 7320;
    if (uncompress_zip(reinterpret_cast<char*>(payload->title_book.category_2),
                       raw_len,
                       blob,
                       db_handle->get_binary_length(2)) != 1) {
        return false;
    }
    memset(blob, 0, sizeof(blob));
    if (db_handle->get_binary(3, blob, 4404) != true) return false;
    raw_len = 3660;
    return uncompress_zip(reinterpret_cast<char*>(payload->title_book.category_3),
                          raw_len,
                          blob,
                          db_handle->get_binary_length(3)) == 1;
}

// IDA_EA: 0x8406422
// IDA_NAME: _ZN10DB_LoadEtc15LoadAchievementEP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::LoadAchievement(SIG_LOAD_ETC *)
bool DB_LoadEtc::LoadAchievement(SIG_LOAD_ETC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect achievement, unix_timestamp(last_update_time) from charac_achievement where charac_no=%u",
        payload->charac_no);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (!db_handle->get_n_rows()) {
        return true;
    }
    if (db_handle->fetch() != true) {
        return false;
    }
    char blob[7384];
    memset(blob, 0, sizeof(blob));
    if (db_handle->get_binary(0, blob, sizeof(blob)) != true) {
        return false;
    }
    unsigned int raw_len = 6144;
    if (uncompress_zip(reinterpret_cast<char*>(payload->achievement.achievements),
                       raw_len,
                       blob,
                       db_handle->get_binary_length(0)) != 1) {
        return false;
    }
    int count = 0;
    for (int index = 0;
         index <= 511 &&
             payload->achievement.achievements[index].achievement_id;
         ++index) {
        ++count;
    }
    payload->achievement.count = static_cast<unsigned int>(count);
    return db_handle->get_uint(1, payload->achievement.last_update_time) ==
           true;
}

// IDA_EA: 0x8406600
// IDA_NAME: _ZN10DB_LoadEtc24LoadOnceGiveAvengerTitleEP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::LoadOnceGiveAvengerTitle(SIG_LOAD_ETC *)
bool DB_LoadEtc::LoadOnceGiveAvengerTitle(SIG_LOAD_ETC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0));
    const char* account_id = NumberToString(payload->account_id, 0);
    db_handle->set_query("seLect m_id from event_1107_avenger_plan where m_id=%s",
                         account_id);
    if (db_handle->exec(true) == true) {
        payload->avenger_title_received =
            db_handle->get_n_rows() ? 1 : 0;
        return true;
    }
    payload->avenger_title_received = 1;
    return false;
}

// IDA_EA: 0x84066AE
// IDA_NAME: _ZN10DB_LoadEtc22LoadPcRoomPlayTimeInfoEP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::LoadPcRoomPlayTimeInfo(SIG_LOAD_ETC *)
bool DB_LoadEtc::LoadPcRoomPlayTimeInfo(SIG_LOAD_ETC* payload) {
    CEventBase* repeat_event = GlobalData::s_event_manager
        ? GlobalData::s_event_manager->GetRepeatEvent(92)
        : 0;
    if (!repeat_event || repeat_event->IsEventing(0) != 1) {
        return true;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0));
    const char* account_id = NumberToString(payload->account_id, 0);
    db_handle->set_query(
        "seLect play_time, reward_cnt from event_1204_pcroom_play_time where m_id = %s and occ_date > curdate()",
        account_id);
    if (db_handle->exec(true) != true) return false;
    if (!db_handle->get_n_rows()) return true;
    if (db_handle->fetch() != true) return false;
    unsigned int play_time = 0;
    unsigned char reward_count = 0;
    if (db_handle->get_uint(0, play_time) != true) return false;
    if (db_handle->get_ubyte(1, reward_count) != true) return false;
    payload->pc_room_play_time.play_time = play_time;
    payload->pc_room_play_time.reward_index = reward_count;
    return true;
}

// IDA_EA: 0x8406810
// IDA_NAME: _ZN10DB_LoadEtc23LoadUsedGiftFatigueInfoEP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::LoadUsedGiftFatigueInfo(SIG_LOAD_ETC *)
bool DB_LoadEtc::LoadUsedGiftFatigueInfo(SIG_LOAD_ETC* payload) {
    payload->used_gift_fatigue = 0;
    CDataManager* data_manager = G_CDataManager();
    EventClassify::CEventScriptMng* event_script_mng =
        data_manager ? data_manager->get_event_script_mng() : 0;
    if (!event_script_mng ||
        event_script_mng->is_eventing(101) != 1) {
        return true;
    }
    EventClassify::CUsedFatigueGiveItem* event_entity =
        static_cast<EventClassify::CUsedFatigueGiveItem*>(
            event_script_mng->get_event_entity(101));
    bool is_charac_event = false;
    if (!event_entity ||
        event_entity->getIsCharacEvent(
                                              is_charac_event) != 1) {
        return true;
    }

    const std::string table_name(
        event_entity->get_db_table_name());
    if (table_name.find_first_not_of(" \t\r\n") == std::string::npos) {
        // Event 101 is optional login state.  An active event without its DB
        // owner must not generate `from  where ...`; keep the initialized
        // zero value and disable only this event read for the session.
        LogManager::logFormat(
            1,
            "db_load_etc.cpp",
            "bool DB_LoadEtc::LoadUsedGiftFatigueInfo(SIG_LOAD_ETC*)",
            __LINE__,
            "[USED_FATIGUE_EVENT][EMPTY_DB_TABLE] phase=load event_id=101 account_id=%u charac_no=%u is_charac_event=%d",
            payload->account_id,
            payload->charac_no,
            is_charac_event ? 1 : 0);
        return true;
    }
    MySQL* db_handle = 0;
    if (is_charac_event) {
        db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
        db_handle->set_query("seLect fatigue_quantity from %s where charac_no = %u",
                             ::toTString(table_name),
                             payload->charac_no);
    } else {
        db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0));
        const char* account_id = NumberToString(payload->account_id, 0);
        db_handle->set_query("seLect fatigue_quantity from %s where m_id = %s",
                             ::toTString(table_name),
                             account_id);
    }
    if (db_handle->exec(true) != true) return false;
    if (!db_handle->get_n_rows()) return true;
    if (db_handle->fetch() != true) return false;
    unsigned int fatigue_quantity = 0;
    if (db_handle->get_uint(0, fatigue_quantity) != true) return false;
    payload->used_gift_fatigue = fatigue_quantity;
    return true;
}

// IDA_EA: 0x8406A6A
// IDA_NAME: _ZN10DB_LoadEtc14LoadRedeemItemEP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::LoadRedeemItem(SIG_LOAD_ETC *)
bool DB_LoadEtc::LoadRedeemItem(SIG_LOAD_ETC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query("seLect redeem_info from charac_inven_expand where charac_no=%u",
                         payload->charac_no);
    if (db_handle->exec(true) != true) return false;
    if (!db_handle->get_n_rows()) return false;
    if (db_handle->fetch() != true) return false;
    char blob[1735];
    memset(blob, 0, sizeof(blob));
    if (db_handle->get_binary(0, blob, sizeof(blob)) != true) return false;
    unsigned int raw_len = 1735;
    return uncompress_zip(reinterpret_cast<char*>(&payload->redeem_items),
                          raw_len,
                          blob,
                          db_handle->get_binary_length(0)) == 1;
}

// IDA_EA: 0x8406CCE
// IDA_NAME: _ZN10DB_LoadEtc18LoadPvPMissionListEP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::LoadPvPMissionList(SIG_LOAD_ETC *)
bool DB_LoadEtc::LoadPvPMissionList(SIG_LOAD_ETC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query("seLect pvp_mission_info from fair_pvp_score where charac_no=%u",
                         payload->charac_no);
    if (db_handle->exec(true) != true) return false;
    if (!db_handle->get_n_rows()) return false;
    if (db_handle->fetch() != true) return false;
    char blob[408];
    memset(blob, 0, sizeof(blob));
    if (db_handle->get_binary(0, blob, sizeof(blob)) != true) return false;
    unsigned int raw_len = 408;
    return uncompress_zip(reinterpret_cast<char*>(&payload->pvp_missions),
                          raw_len,
                          blob,
                          db_handle->get_binary_length(0)) == 1;
}

// IDA_EA: 0x8406EEC
// IDA_NAME: _ZN10DB_LoadEtc16LoadTODUserStateEP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::LoadTODUserState(SIG_LOAD_ETC *)
bool DB_LoadEtc::LoadTODUserState(SIG_LOAD_ETC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect unix_timestamp(first_layer_start_date),today_enter_count,last_clear_layer,enter_count_by_week,unix_timestamp(m_date),unix_timestamp(last_clear_date) from charac_tower_despair where charac_no = %u",
        payload->charac_no);
    if (db_handle->exec(true) != true) return true;
    if (db_handle->fetch() != true) return true;
    if (db_handle->getAffectedRowCount() != 1) return true;
    if (db_handle->get_int(0,
                           payload->tod_user_state.first_layer_start_time) !=
        true)
        return false;
    if (db_handle->get_int(1, payload->tod_user_state.today_enter_count) !=
        true)
        return false;
    unsigned short layer = 0;
    if (db_handle->get_ushort(2, layer) != true) return false;
    payload->tod_user_state.last_clear_layer.layer = layer > 100 ? 1 : layer;
    if (db_handle->get_short(3,
                             payload->tod_user_state.enter_count_by_week) !=
        true)
        return false;
    if (db_handle->get_int(4, payload->tod_user_state.modified_time) != true)
        return false;
    return db_handle->get_int(5,
                              payload->tod_user_state.last_clear_time) == true;
}

// IDA_EA: 0x8407296
// IDA_NAME: _ZN10DB_LoadEtc22LoadConditionEventInfoEP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::LoadConditionEventInfo(SIG_LOAD_ETC *)
bool DB_LoadEtc::LoadConditionEventInfo(SIG_LOAD_ETC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect current_step, reward_step from event_conditionable_info where charac_no=%u",
        payload->charac_no);
    if (db_handle->exec(true) != true) return false;
    if (db_handle->get_n_rows() == 1) {
        short current_step = 0;
        short reward_step = 0;
        if (db_handle->fetch() != true ||
            db_handle->get_short(0, current_step) != true ||
            db_handle->get_short(1, reward_step) != true) {
            return false;
        }
        payload->condition_event_step =
            static_cast<uint16_t>(current_step);
        payload->condition_event_reward_step =
            static_cast<uint16_t>(reward_step);
        return true;
    }
    payload->condition_event_step = 0;
    payload->condition_event_reward_step = 0;
    return true;
}

// IDA_EA: 0x84073AA
// IDA_NAME: _ZN10DB_LoadEtc27LoadProperDungeonClearCountEP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::LoadProperDungeonClearCount(SIG_LOAD_ETC *)
bool DB_LoadEtc::LoadProperDungeonClearCount(SIG_LOAD_ETC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect clear_cnt from event_dungeon_clear where charac_no=%u",
        payload->charac_no);
    if (db_handle->exec(true) != true) return false;
    if (db_handle->get_n_rows() == 1) {
        short clear_count = 0;
        if (db_handle->fetch() != true ||
            db_handle->get_short(0, clear_count) != true) {
            return false;
        }
        payload->proper_dungeon_clear_count =
            static_cast<uint16_t>(clear_count);
        return true;
    }
    payload->proper_dungeon_clear_count = 0;
    return true;
}

// IDA_EA: 0x8407482
// IDA_NAME: _ZN10DB_LoadEtc20_loadNPCRelasionshipEP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::_loadNPCRelasionship(SIG_LOAD_ETC *)
int DB_LoadEtc::_loadNPCRelasionship(SIG_LOAD_ETC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query("seLect npc_cnt,npc_data from charac_npc where charac_no=%d",
                         payload->charac_no);
    if (db_handle->exec(true) != true) return 7296;
    if (db_handle->get_n_rows() != 1) return 7300;
    if (db_handle->fetch() != true) return 7303;
    if (db_handle->get_uint(0, payload->npc_relationship_count) != true)
        return 7306;
    if (payload->npc_relationship_count > 50u) return 7308;
    if (db_handle->get_binary_length(1) !=
        static_cast<int>(sizeof(payload->npc_relationships[0]) *
                         payload->npc_relationship_count)) {
        return 7309;
    }
    if (db_handle->get_binary(1, payload->npc_relationships, 600) == true) {
        return 0;
    }
    return 7312;
}

// IDA_EA: 0x84075C4
// IDA_NAME: _ZN10DB_LoadEtc13GetUserRecordEP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::GetUserRecord(SIG_LOAD_ETC *)
bool DB_LoadEtc::GetUserRecord(SIG_LOAD_ETC* payload) {
    (void)payload;
    return true;
}

// IDA_EA: 0x84075D2
// IDA_NAME: _ZN10DB_LoadEtc12GetUserSkillEP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::GetUserSkill(SIG_LOAD_ETC *)
bool DB_LoadEtc::GetUserSkill(SIG_LOAD_ETC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    const char* skill_table = get_skill_table_name(0);
    db_handle->set_query(
        "seLect remain_sp,skill_slot,remain_sp_2nd, skill_slot_2nd, remain_sfp_1st, remain_sfp_2nd, skill_command, skill_slot_lethe, lethe_flag, skill_slot_lethe_2nd, lethe_flag_2nd,script_version from %s where charac_no=%u",
        skill_table,
        payload->charac_no);
    if (db_handle->exec(true) != true) {
        cMyTrace trace("bool DB_LoadEtc::GetUserSkill(SIG_LOAD_ETC*)",
                       7504,
                       5);
        trace("DB_LoadSkill::GetUserSkill, exec() ERROR charac_no=%u",
              payload->charac_no);
        return false;
    }
    if (!db_handle->get_n_rows() &&
        G_GameWorld()->IsPvPSkilTreeChannel()) {
        db_handle->set_query("inSert into %s (charac_no) values(%u)",
                             skill_table,
                             payload->charac_no);
        if (db_handle->exec(true) == true) {
            payload->create_pvp_skill_row = 1;
            return true;
        }
        cMyTrace trace("bool DB_LoadEtc::GetUserSkill(SIG_LOAD_ETC*)",
                       7517,
                       5);
        trace("DB_LoadSkill::GetUserSkill insert, exec() ERROR charac_no=%u",
              payload->charac_no);
        return false;
    }

    if (db_handle->fetch() != true) {
        cMyTrace trace("bool DB_LoadEtc::GetUserSkill(SIG_LOAD_ETC*)",
                       7529,
                       5);
        trace("DB_LoadSkill::GetUserSkill, fetch() ERROR charac_no=%u",
              payload->charac_no);
        return false;
    }
    int column = 0;
    if (db_handle->get_uint(column++, payload->skill.remain_sp) != true)
        return false;
    if (get_compressed_blob_data(db_handle,
                                 column++,
                                 reinterpret_cast<unsigned char*>(
                                     payload->skill.first_tree),
                                 408,
                                 2) != 1) {
        return false;
    }
    if (db_handle->get_uint(column++, payload->skill.remain_sp_second) != true)
        return false;
    if (get_compressed_blob_data(db_handle,
                                 column++,
                                 reinterpret_cast<unsigned char*>(
                                     payload->skill.second_tree),
                                 408,
                                 2) != 1) {
        return false;
    }
    if (db_handle->get_uint(column++, payload->skill.remain_sfp_first) != true)
        return false;
    if (db_handle->get_uint(column++, payload->skill.remain_sfp_second) != true)
        return false;
    if (get_compressed_blob_data(db_handle,
                                 column++,
                                 reinterpret_cast<unsigned char*>(
                                     payload->skill.skill_commands),
                                 200,
                                 4) != 1) {
        return false;
    }
    if (get_compressed_blob_data(db_handle,
                                 column++,
                                 reinterpret_cast<unsigned char*>(
                                     payload->premium_lethe.mastered_skills),
                                 408,
                                 2) != 1) {
        return false;
    }
    int lethe_flag = 0;
    if (db_handle->get_int(column++, lethe_flag) != true) return false;
    if (get_compressed_blob_data(db_handle,
                                 column++,
                                 reinterpret_cast<unsigned char*>(
                                     payload->premium_lethe_second.mastered_skills),
                                 408,
                                 2) != 1) {
        return false;
    }
    int lethe_flag_2nd = 0;
    if (db_handle->get_int(column++, lethe_flag_2nd) != true) return false;
    if (db_handle->get_ubyte(column++, payload->skill.script_version) != true)
        return false;

    payload->premium_lethe.lethe_state =
        static_cast<unsigned char>(lethe_flag);
    payload->one_day_lethe_state = static_cast<unsigned char>(lethe_flag);
    payload->premium_lethe_second.lethe_state =
        static_cast<unsigned char>(lethe_flag_2nd);
    payload->one_day_lethe_second_state =
        static_cast<unsigned char>(lethe_flag_2nd);

    if (!payload->tag_charac_no) {
        return true;
    }
    db_handle->set_query("seLect skill_slot, skill_slot_2nd from skill where charac_no=%u",
                         payload->tag_charac_no);
    if (db_handle->exec(true) != true) {
        cMyTrace trace("bool DB_LoadEtc::GetUserSkill(SIG_LOAD_ETC*)",
                       7658,
                       5);
        trace("DB_LoadSkill::GetUserSkill, exec() ERROR tagCharacNo=%u",
              payload->tag_charac_no);
        return false;
    }
    if (db_handle->fetch() != true) {
        cMyTrace trace("bool DB_LoadEtc::GetUserSkill(SIG_LOAD_ETC*)",
                       7665,
                       5);
        trace("DB_LoadSkill::GetUserSkill, fetch() ERROR tagCharacNo=%u",
              payload->tag_charac_no);
        return false;
    }
    if (get_compressed_blob_data(db_handle,
                                 0,
                                 reinterpret_cast<unsigned char*>(
                                     payload->tag_charac_etc.skill_trees[0]),
                                 408,
                                 2) != 1) {
        return false;
    }
    return get_compressed_blob_data(db_handle,
                                    1,
                                    reinterpret_cast<unsigned char*>(
                                        payload->tag_charac_etc.skill_trees[1]),
                                    408,
                                    2) == 1;
}

// IDA_EA: 0x8407DB4
// IDA_NAME: _ZN10DB_LoadEtc14GetUserPvpBaseEP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::GetUserPvpBase(SIG_LOAD_ETC *)
int DB_LoadEtc::GetUserPvpBase(SIG_LOAD_ETC* payload) {
    (void)payload;
    return 1;
}

// IDA_EA: 0x8407DC2
// IDA_NAME: _ZN10DB_LoadEtc10GetUserPvpEP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::GetUserPvp(SIG_LOAD_ETC *)
int DB_LoadEtc::GetUserPvp(SIG_LOAD_ETC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect win,lose,pvp_point,pvp_grade,avg_kill_count,avg_buf_count,avg_debuf_count,avg_heal_count,avg_counter_count,avg_back_atk_count,avg_union_hit_count,avg_overkill_count,avg_combo_count,avg_aerial_count,avg_attacked_count,avg_deal_damage,avg_technic,avg_style,avg_hit_penalty,pvp_count,win_point,pvp_grade_ext,unix_timestamp(last_play_time),play_count,play_time,unix_timestamp(pvp_grade_ext_update_time) from pvp_result where charac_no=%u",
        payload->charac_no);
    if (db_handle->exec(true) != true) {
        return -1;
    }
    if (db_handle->fetch() != true) {
        return 1;
    }
    int32_t* pvp = payload->pvp_result.values;
    if (db_handle->get_int(0, pvp[0]) != true) return -1;
    if (db_handle->get_int(1, pvp[1]) != true) return -1;
    if (db_handle->get_int(2, pvp[2]) != true) return -1;
    for (int column = 3; column <= 21; ++column) {
        if (db_handle->get_int(column, pvp[column + 2]) != true) {
            return -1;
        }
    }
    int last_play_time = 0;
    if (db_handle->get_int(22, last_play_time) != true) return -1;
    unsigned int play_count = 0;
    if (db_handle->get_uint(23, play_count) != true) {
        return -1;
    }
    pvp[25] = static_cast<int32_t>(play_count);
    unsigned int play_time = 0;
    if (db_handle->get_uint(24, play_time) != true) {
        return -1;
    }
    pvp[24] = static_cast<int32_t>(play_time);

    time_t cur_sec = GlobalData::system_time()
        ? GlobalData::system_time()->getCurSec()
        : time(0);
    tm day_tm;
    tm* today = localtime_r(&cur_sec, &day_tm);
    today->tm_hour = 0;
    today->tm_min = 0;
    today->tm_sec = 0;
    pvp[26] = static_cast<int>(mktime(today));
    if (pvp[26] > last_play_time) {
        pvp[24] = 0;
    }

    int grade_ext_update_time = 0;
    if (db_handle->get_int(25, grade_ext_update_time) != true) return -1;
    const int current_sec = GlobalData::system_time()
        ? GlobalData::system_time()->getCurSec()
        : static_cast<int>(time(0));
    if (grade_ext_update_time + 86400 < current_sec) {
        pvp[23] = 0;
    }
    return 0;
}

// IDA_EA: 0x840838E
// IDA_NAME: _ZN10DB_LoadEtc20LoadAuraAvatarOptionEP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::LoadAuraAvatarOption(SIG_LOAD_ETC *)
bool DB_LoadEtc::LoadAuraAvatarOption(SIG_LOAD_ETC* payload) {
    payload->aura_avatar_option.reset();
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect option_type, value_1 from aura_avatar_option where charac_no=%u",
        payload->charac_no);
    if (db_handle->exec(true) != true) return false;
    const int rows = db_handle->get_n_rows();
    if (!rows) return true;
    for (int row = 0; row < rows; ++row) {
        if (db_handle->fetch() != true) break;
        int option_type = 0;
        int value = 0;
        if (db_handle->get_int(0, option_type) != true) return false;
        if (db_handle->get_int(1, value) != true) return false;
        if (static_cast<unsigned int>(option_type) <= 2) {
            payload->aura_avatar_option.values[option_type] = value;
        }
    }
    return true;
}

// IDA_EA: 0x84084E0
// IDA_NAME: _ZN10DB_LoadEtc16LoadCharacOptionEiiP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::LoadCharacOption(int,int,SIG_LOAD_ETC *)
bool DB_LoadEtc::LoadCharacOption(int signal,
                                  int user_idx,
                                  SIG_LOAD_ETC* payload) {
    (void)signal;
    (void)user_idx;
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect options , best_clear_time, blue_marble_enter_count, charac_inform_notice from charac_option where charac_no=%u",
        payload->charac_no);
    if (db_handle->exec(true) != true) return false;
    if (!db_handle->get_n_rows()) return true;
    if (db_handle->fetch() != true) return false;
    if (db_handle->get_binary(0,
                              &payload->character_option,
                              sizeof(payload->character_option)) != true) {
        return false;
    }
    if (get_compressed_best_clear_time(db_handle,
                                       1,
                                       payload->best_clear_time.records,
                                       sizeof(payload->best_clear_time.records),
                                       0) != 1) {
        cMyTrace trace("bool DB_LoadEtc::LoadCharacOption(int, int, SIG_LOAD_ETC*)",
                       8029,
                       5);
        trace("DB_LoadEtc::LoadCharacOption best_clear_time, get(%d) ERROR charac_no=%u",
              1,
              payload->charac_no);
        return false;
    }
    for (int index = 0; index <= 4; ++index) {
        if (payload->best_clear_time.records[0]
                .difficulty.clear_time[index] < 0) {
            payload->best_clear_time.records[0]
                .difficulty.clear_time[index] = 0;
        }
    }
    payload->blue_marble_enter_count = 0;
    if (db_handle->get_ubyte(2, payload->blue_marble_enter_count) != true)
        return false;
    if (db_handle->get_str(3, payload->charac_inform_notice, 16) !=
        true) {
        return false;
    }
    int count = 0;
    for (int index = 0;
         index <= 999 && payload->best_clear_time.records[index].dungeon_id;
         ++index) {
        ++count;
    }
    payload->best_clear_time.count = count;
    return true;
}

// IDA_EA: 0x8408766
// IDA_NAME: _ZN10DB_LoadEtc10SendResultEiiP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::SendResult(int,int,SIG_LOAD_ETC *)
bool DB_LoadEtc::SendResult(int signal, int user_idx, SIG_LOAD_ETC* payload) {
    Stream* out_stream =
        GlobalData::big_stream_pool()->Acquire("DBThread.cpp", 8082);
    CStreamGuard guard(out_stream, true);
    *guard << signal;
    *guard << user_idx;
    guard->put_binary(payload, sizeof(*payload));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x84356A2
// IDA_NAME: _ZN10DB_LoadEtc26LoadEventServerMessageInfoEP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::LoadEventServerMessageInfo(SIG_LOAD_ETC *)
bool DB_LoadEtc::LoadEventServerMessageInfo(SIG_LOAD_ETC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect kind, message_index, charac_name, message, unix_timestamp(update_time) from event_server_message where server_info=%d and (channel_no=%d or channel_no=0)",
        payload->server_info,
        payload->channel_no);
    if (db_handle->exec(true) != true) return false;
    const int rows = db_handle->get_n_rows();
    char message[50];
    char charac_name[20];
    for (int row = 0; row < rows; ++row) {
        if (db_handle->fetch() != true) return false;
        EventServerMessagePayload& server_message =
            payload->server_message_info.messages[row];
        char kind = 0;
        char message_index = 0;
        if (db_handle->get_byte(0, kind) != true) {
            return false;
        }
        if (db_handle->get_byte(1, message_index) != true) {
            return false;
        }
        server_message.kind = static_cast<int8_t>(kind);
        server_message.message_index = static_cast<int8_t>(message_index);
        memset(charac_name, 0, sizeof(charac_name));
        memset(message, 0, sizeof(message));
        if (db_handle->get_str(2, charac_name, 20) != true) return false;
        if (db_handle->get_str(3, message, 50) != true) return false;
        if (db_handle->get_int(4, server_message.update_time) != true) {
            return false;
        }
        server_message.message_length =
            static_cast<unsigned int>(strlen(message));
        strncpy(server_message.message, message, strlen(message));
        server_message.charac_name_length =
            static_cast<unsigned int>(strlen(charac_name));
        strncpy(server_message.charac_name,
                charac_name,
                strlen(charac_name));
    }
    payload->server_message_info.count = static_cast<unsigned char>(rows);
    return true;
}

// IDA_EA: 0x843F038
// IDA_NAME: _ZN10DB_LoadEtc21LoadVillageAttackInfoEP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::LoadVillageAttackInfo(SIG_LOAD_ETC *)
bool DB_LoadEtc::LoadVillageAttackInfo(SIG_LOAD_ETC* payload) {
    if (!payload->charac_no) {
        return false;
    }
    time_t timer = GlobalData::system_time()
        ? GlobalData::system_time()->getCurSec()
        : time(0);
    tm current_tm;
    tm* date_tm = localtime_r(&timer, &current_tm);
    CEnvironment* environment = G_CEnvironment();
    const int reset_hour = environment ? environment->get_fatigue_time() : 0;
    if (date_tm->tm_hour < reset_hour) {
        timer -= 86400;
        date_tm = localtime_r(&timer, &current_tm);
    }
    char date_text[12];
    memset(date_text, 0, sizeof(date_text));
    snprintf(date_text,
             sizeof(date_text),
             "%04d-%02d-%02d",
             date_tm->tm_year + 1900,
             date_tm->tm_mon + 1,
             date_tm->tm_mday);
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    if (!db_handle) return false;
    db_handle->set_query(
        "seLect attack_count, revenge_dungeon from village_attack_dungeon \t\t\t\t\t\t   where occ_date = '%s' and charac_no = %u",
        date_text,
        payload->charac_no);
    if (db_handle->exec(true) != true) return false;
    if (db_handle->get_n_rows()) {
        if (db_handle->fetch() != true) return false;
        return db_handle->get_ubyte(0, payload->village_attack_count) ==
                   true &&
               db_handle->get_ubyte(1, payload->revenge_dungeon_count) ==
                   true;
    }
    payload->village_attack_count = 0;
    payload->revenge_dungeon_count = 0;
    return false;
}

// IDA_EA: 0x8445660
// IDA_NAME: _ZN10DB_LoadEtc18LoadCharacItemStatEP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::LoadCharacItemStat(SIG_LOAD_ETC *)
bool DB_LoadEtc::LoadCharacItemStat(SIG_LOAD_ETC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect cooltime_item, effect_item, check_flag from charac_item_stat where charac_no=%u",
        payload->charac_no);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (!db_handle->get_n_rows()) {
        return true;
    }
    if (db_handle->fetch() != true) {
        return false;
    }
    if (db_handle->get_binary(
            0,
            reinterpret_cast<unsigned char*>(&payload->cooltime_items),
            sizeof(payload->cooltime_items)) != true) {
        cMyTrace trace("bool DB_LoadEtc::LoadCharacItemStat(SIG_LOAD_ETC*)",
                       44170,
                       5);
        trace("DB_LoadEtc::LoadCharacItemStat cooltime, get(%d) ERROR charac_no=%u",
              0,
              payload->charac_no);
        return false;
    }
    if (db_handle->get_binary(
            1,
            reinterpret_cast<unsigned char*>(&payload->effect_items),
            sizeof(payload->effect_items)) != true) {
        cMyTrace trace("bool DB_LoadEtc::LoadCharacItemStat(SIG_LOAD_ETC*)",
                       44177,
                       5);
        trace("DB_LoadEtc::LoadCharacItemStat effect_item, get(%d) ERROR charac_no=%u",
              1,
              payload->charac_no);
        return false;
    }
    if (get_compressed_blob_data(db_handle,
                                 2,
                                 payload->item_dictionary.flags,
                                 5000,
                                 0) != 1) {
        cMyTrace trace("bool DB_LoadEtc::LoadCharacItemStat(SIG_LOAD_ETC*)",
                       44184,
                       5);
        trace("DB_LoadEtc::LoadCharacItemStat item_dictionary, get(%d) ERROR charac_no=%u",
              2,
              payload->charac_no);
        return false;
    }
    return true;
}

// IDA_EA: 0x8446D9E
// IDA_NAME: _ZN10DB_LoadEtc15LoadBoosterGageEP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::LoadBoosterGage(SIG_LOAD_ETC *)
bool DB_LoadEtc::LoadBoosterGage(SIG_LOAD_ETC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query("seLect gage from member_booster_gage where m_id=%u",
                         payload->account_id);
    if (db_handle->exec(true) != true) return false;
    if (db_handle->get_n_rows() != 1) return false;
    if (db_handle->fetch() != true) return false;
    if (db_handle->get_int(0, payload->booster_gage.gage) == true) {
        return true;
    }
    cMyTrace trace("bool DB_LoadEtc::LoadBoosterGage(SIG_LOAD_ETC*)",
                   44996,
                   5);
    trace("DB_LoadEtc::LoadBoosterGage, get(%d) ERROR m_id=%u",
          0,
          payload->account_id);
    return false;
}

// IDA_EA: 0x8448944
// IDA_NAME: _ZN10DB_LoadEtc18LoadAttendanceInfoEP20SIG_ATTENDANCE_EVENTj
// DEMANGLED: DB_LoadEtc::LoadAttendanceInfo(SIG_ATTENDANCE_EVENT *,unsigned int)
bool DB_LoadEtc::LoadAttendanceInfo(SIG_ATTENDANCE_EVENT* attendance_payload,
                                    unsigned int account_id) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0));
    attendance_payload->can_attend = 0;
    CEventBase* repeat_event = GlobalData::s_event_manager
        ? GlobalData::s_event_manager->GetRepeatEvent(96)
        : 0;
    if (!repeat_event || repeat_event->IsEventing(0) != 1) {
        return false;
    }
    CEnvironment* environment = G_CEnvironment();
    const int reset_hour = environment ? environment->get_fatigue_time() : 0;
    const time_t cur_sec = GlobalData::system_time()
        ? GlobalData::system_time()->getCurSec()
        : time(0);
    const time_t reset_time = GetCurrentResetBaseTime(cur_sec, reset_hour);
    const char* account_text = NumberToString(account_id, 0);
    db_handle->set_query(
        "seLect occ_date from event_1207_attendance where m_id = %s and occ_date > from_unixtime(%u)",
        account_text,
        static_cast<unsigned int>(reset_time));
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (!db_handle->get_n_rows()) {
        attendance_payload->can_attend = 1;
    }
    return true;
}

// IDA_EA: 0x844A20A
// IDA_NAME: _ZN10DB_LoadEtc20LoadSeriaRoomAniDecoEP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::LoadSeriaRoomAniDeco(SIG_LOAD_ETC *)
bool DB_LoadEtc::LoadSeriaRoomAniDeco(SIG_LOAD_ETC* payload) {
    CEventBase* repeat_event = GlobalData::s_event_manager
        ? GlobalData::s_event_manager->GetRepeatEvent(105)
        : 0;
    if (!repeat_event || repeat_event->IsEventing(0) != 1) {
        return false;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect receive_flow, deco_flow from event_1208_seriaroom_anideco where m_id = %d and server_id = %d and charac_no = %d",
        payload->account_id,
        payload->server_info,
        payload->charac_no);
    if (db_handle->exec(true) != true) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "bool DB_LoadEtc::LoadSeriaRoomAniDeco(SIG_LOAD_ETC*)",
            0xBA8F,
            "LoadSeriaRoomAniDeco Query Error : m_id(%d)",
            payload->account_id);
        return false;
    }
    if (!db_handle->get_n_rows()) {
        return true;
    }
    if (db_handle->fetch() != true) return false;
    if (db_handle->get_ushort(
            0, payload->seria_room_ani_deco.receive_flow) != true)
        return false;
    return db_handle->get_ushort(
               1, payload->seria_room_ani_deco.deco_flow) == true;
}

// IDA_EA: 0x844A3B2
// IDA_NAME: _ZN10DB_LoadEtc18LoadComboSkillInfoEP12SIG_LOAD_ETC
// DEMANGLED: DB_LoadEtc::LoadComboSkillInfo(SIG_LOAD_ETC *)
void DB_LoadEtc::LoadComboSkillInfo(SIG_LOAD_ETC* payload) {
    if (payload->combo_skill.charac_job !=
        oldsrv::config::kCharacterJobDemonicSwordman) {
        return;
    }
    if (LoadComboSkill(payload, ENUM_SKILL_TREE_0) != true) {
        LogManager::logFormat(1,
                              "DBThread.cpp",
                              "void DB_LoadEtc::LoadComboSkillInfo(SIG_LOAD_ETC*)",
                              0xBAB4,
                              "DB_LoadETC ERROR : LoadComboSkill Fail");
        payload->result = 1;
    }
    if (LoadComboSkill(payload, ENUM_SKILL_TREE_1) != true) {
        LogManager::logFormat(1,
                              "DBThread.cpp",
                              "void DB_LoadEtc::LoadComboSkillInfo(SIG_LOAD_ETC*)",
                              0xBABA,
                              "DB_LoadETC ERROR : LoadComboSkill Second Fail");
        payload->result = 1;
    }
}

// IDA_EA: 0x844A47A
// IDA_NAME: _ZN10DB_LoadEtc14LoadComboSkillEP12SIG_LOAD_ETC20ENUM_SKILL_TREE_KIND
// DEMANGLED: DB_LoadEtc::LoadComboSkill(SIG_LOAD_ETC *,ENUM_SKILL_TREE_KIND)
bool DB_LoadEtc::LoadComboSkill(SIG_LOAD_ETC* payload,
                                ENUM_SKILL_TREE_KIND skill_tree_kind) {
    if (skill_tree_kind >= 2) {
        cMyTrace trace(
            "bool DB_LoadEtc::LoadComboSkill(SIG_LOAD_ETC*, ENUM_SKILL_TREE_KIND)",
            47812,
            5);
        trace("DB_LoadEtc::LoadComboSkill kind(%d) error characNo(%u)",
              skill_tree_kind,
              payload->charac_no);
        return false;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    if (!db_handle) return false;
    const char* table_name = getComboSkillTableName(skill_tree_kind);
    db_handle->set_query(
        "seLect combo_idx, value1, value2, value3, value4, value5, value6 from %s where charac_no=%u",
        table_name,
        payload->charac_no);
    if (db_handle->exec(true) != true) {
        cMyTrace trace(
            "bool DB_LoadEtc::LoadComboSkill(SIG_LOAD_ETC*, ENUM_SKILL_TREE_KIND)",
            47828,
            5);
        trace("DB_LoadEtc::LoadComboSkill, exec() ERROR charac_no=%u",
              payload->charac_no);
        return false;
    }
    const int rows = db_handle->get_n_rows();
    if (!rows) {
        return true;
    }
    if (skill_tree_kind) {
        payload->combo_skill.second_tree_count =
            static_cast<unsigned char>(rows);
        if (getComboSkill(db_handle,
                          payload->charac_no,
                          rows,
                          payload->combo_skill.second_tree_combos) != true) {
            cMyTrace trace(
                "bool DB_LoadEtc::LoadComboSkill(SIG_LOAD_ETC*, ENUM_SKILL_TREE_KIND)",
                47855,
                5);
            trace("DB_LoadEtc::LoadComboSkill, getComboSkill ERROR charac_no=%u",
                  payload->charac_no);
        }
    } else {
        payload->combo_skill.first_tree_count =
            static_cast<unsigned char>(rows);
        if (getComboSkill(db_handle,
                          payload->charac_no,
                          rows,
                          payload->combo_skill.first_tree_combos) != true) {
            cMyTrace trace(
                "bool DB_LoadEtc::LoadComboSkill(SIG_LOAD_ETC*, ENUM_SKILL_TREE_KIND)",
                47847,
                5);
            trace("DB_LoadEtc::LoadComboSkill, getComboSkill ERROR charac_no=%u",
                  payload->charac_no);
        }
    }
    return true;
}

// IDA_EA: 0x844A704
// IDA_NAME: _ZN10DB_LoadEtc13getComboSkillEP5MySQLjiP15comoboSkillData
// DEMANGLED: DB_LoadEtc::getComboSkill(MySQL *,unsigned int,int,comoboSkillData *)
bool DB_LoadEtc::getComboSkill(MySQL* db_handle,
                               unsigned int charac_no,
                               int row_count,
                               comoboSkillData* combo_skill_data) {
    if (!db_handle || !combo_skill_data) {
        return false;
    }
    for (int row = 0; row < row_count && row <= 5; ++row) {
        if (db_handle->fetch() != true) {
            cMyTrace trace(
                "bool DB_LoadEtc::getComboSkill(MySQL*, unsigned int, int, comoboSkillData*)",
                47873,
                5);
            trace("DB_LoadEtc::getComboSkill fetch, exec() ERROR charac_no=%u",
                  charac_no);
            return false;
        }
        short skill_id = 0;
        if (db_handle->get_short(0, skill_id) != true) {
            cMyTrace trace(
                "bool DB_LoadEtc::getComboSkill(MySQL*, unsigned int, int, comoboSkillData*)",
                47882,
                5);
            trace("DB_LoadEtc::getComboSkill get(%d) ERROR charac_no=%u",
                  1,
                  charac_no);
            return false;
        }
        combo_skill_data[row].skill_id = static_cast<uint16_t>(skill_id);
        for (int index = 0; index <= 5; ++index) {
            short command = 0;
            if (db_handle->get_short(index + 1, command) != true) {
                cMyTrace trace(
                    "bool DB_LoadEtc::getComboSkill(MySQL*, unsigned int, int, comoboSkillData*)",
                    47892,
                    5);
                trace("DB_LoadEtc::getComboSkill get(%d) ERROR charac_no=%u",
                      index + 1,
                      charac_no);
                return false;
            }
            combo_skill_data[row].command[index] =
                static_cast<uint16_t>(command);
        }
    }
    return true;
}
