#include "old_data/db/character/db_create_charac.h"

#include "common/config.h"
#include "old_data/db/legacy_dispatch/arad_dispatcher.h"
#include "old_data/db/core/db_conn_info.h"
#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/types/compressed_blob.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "common/manager/gm_accounts_abi.h"
#include "old_game/activity/schedule/event_mgr_runtime.h"
#include "common/manager/reliable_person.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <map>

class MySQL;
namespace WongWork {
class CGMAccounts;
}  // namespace WongWork

extern int GetIdentityFromDB(MySQL* mysql)
    __asm__("_Z17GetIdentityFromDBP5MySQL");

namespace {

enum {
    kCreateCharacNameEscapedSize = 60,
    kInventoryZipSize = 18238,
    kEquipZipSize = 890,
    kSkillZipSize = 501,
    kInventoryRawSize =
        oldsrv::config::kMainInventoryPrimaryItemCount *
        oldsrv::config::kInventoryItemRecordSize,
    // zlib's compressBound(raw) plus the four-byte raw-length prefix emitted
    // by compress_zip().
    kInventoryZipRequiredSize =
        kInventoryRawSize + (kInventoryRawSize >> 12) +
        (kInventoryRawSize >> 14) + (kInventoryRawSize >> 25) + 17,
    kEquipmentItemCount = 12,
    kServerGroupCount = 0x12,
};

static_assert(kInventoryZipSize >= kInventoryZipRequiredSize,
              "create-character inventory compression buffer is too small");

const char kDecideLevelupStateTable[] =
    "account_decide_levelup_state_0828";

// RECONSTRUCTED_0828: CMD561's reservation is account-scoped until CMD5
// returns the newly allocated character number.  This lookup replaces the
// expired 2009 calendar gate only when an active 0828 reservation exists.
// Return values: 1=pending reservation, 0=no reservation, -1=DB failure.
int QueryPendingDecideLevelupReservation(SIG_CREATE_CHARAC* payload) {
    CEnvironment* const environment = G_CEnvironment();
    const int server_group = environment ? environment->get_server_group() : 0;
    if (!payload || !GlobalData::s_db_mgr || server_group < 0 ||
        server_group >= kServerGroupCount) {
        return -1;
    }
    MySQL* const db = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, static_cast<ENUM_SERVER_GROUP>(server_group));
    if (!db ||
        !db->set_query(
            "seLect 1 from %s where account_id=%u and charac_no is null "
            "and main_reward_decided=0 and target_level in(20,30,40,50,60,70) "
            "limit 1",
            kDecideLevelupStateTable,
            static_cast<unsigned int>(payload->account_id)) ||
        !db->exec(true)) {
        return -1;
    }
    return db->get_n_rows() == 1 ? 1 : 0;
}

}  // namespace

// IDA_EA: 0x80EC2A0
// IDA_NAME: _ZN15DB_CreateCharac20MakeInsertCharacInfoEP5MySQLP17SIG_CREATE_CHARACPKc
// DEMANGLED: DB_CreateCharac::MakeInsertCharacInfo(MySQL *,SIG_CREATE_CHARAC *,char const*)
int DB_CreateCharac::MakeInsertCharacInfo(MySQL* db_handle,
                                          SIG_CREATE_CHARAC* payload,
                                          const char* escaped_name) {
    char* spec_property =
        db_handle->blob_to_str(1,
                               payload->initial_stats.spec_property,
                               sizeof(payload->initial_stats.spec_property));
    char* element_resist =
        db_handle->blob_to_str(0,
                               payload->initial_stats.element_resist,
                               sizeof(payload->initial_stats.element_resist));
    const char* account_id = NumberToString(payload->account_id, 0);
    return db_handle->set_query(
        "inSert into charac_info (m_id,charac_name,job,sex,maxHP,maxMP,phy_attack,phy_defense,mag_attack,mag_defense,element_resist,spec_property,inven_weight,hp_regen,mp_regen,move_speed,attack_speed,cast_speed,hit_recovery,jump,charac_weight,max_fatigue,create_time,expert_job,event_charac_level) values(%s,'%s',%d,%d,%d,%u,%d,%d,%d,%d,'%s','%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,NOW(),%d,%d)",
        account_id,
        escaped_name,
        payload->job,
        payload->sex,
        payload->initial_stats.max_hp,
        payload->initial_stats.max_mp,
        payload->initial_stats.physical_attack,
        payload->initial_stats.physical_defense,
        payload->initial_stats.magical_attack,
        payload->initial_stats.magical_defense,
        element_resist,
        spec_property,
        payload->initial_stats.inventory_weight,
        payload->initial_stats.hp_regeneration,
        payload->initial_stats.mp_regeneration,
        payload->initial_stats.move_speed,
        payload->initial_stats.attack_speed,
        payload->initial_stats.cast_speed,
        payload->initial_stats.hit_recovery,
        payload->initial_stats.jump,
        payload->initial_stats.character_weight,
        DEFAULT_MAX_FATIGUE,
        payload->expert_job,
        payload->event_character_grow_type);
}

// IDA_EA: 0x8400CC2
// IDA_NAME: _ZN15DB_CreateCharacC2Ev
// DEMANGLED: DB_CreateCharac::DB_CreateCharac(void)
DB_CreateCharac::DB_CreateCharac() {}

// IDA_EA: 0x8400CDE
// IDA_NAME: _ZN15DB_CreateCharacD2Ev
// DEMANGLED: DB_CreateCharac::~DB_CreateCharac()
// IDA_EA: 0x8400D0E
// IDA_NAME: _ZN15DB_CreateCharacD0Ev
// DEMANGLED: DB_CreateCharac::~DB_CreateCharac() deleting destructor
DB_CreateCharac::~DB_CreateCharac() {}

// IDA_EA: 0x8400D2C
// IDA_NAME: _ZN15DB_CreateCharac8dispatchEiiP6Stream
// DEMANGLED: DB_CreateCharac::dispatch(int,int,Stream *)
bool DB_CreateCharac::dispatch(int signal, int user_idx, Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }
    SIG_CREATE_CHARAC* payload = stream->GetOutBuffer<SIG_CREATE_CHARAC>();
    if (!payload) {
        return false;
    }
    payload->result = 0;

    // LOCAL_POLICY: character creation has no per-account/per-day throttle.
    // Keep the legacy limit helpers for ABI/reference compatibility, but do
    // not consult or mutate the legacy throttle table in the live create path.
    if (CreateNewCharac(payload) == true) {
        if (GetCharacNo(payload) == true) {
            if (_CreateNewCharacStat(payload) == true) {
                bool inventory_ok = true;
                if (payload->has_inventory) {
                    inventory_ok = CreateInventory(payload->character_no,
                                                   payload->inventory);
                } else {
                    inventory_ok = CreateInventory(payload->character_no, 0);
                }
                bool equipment_ok = true;
                if (payload->has_equipment) {
                    equipment_ok = CreateEquip(payload->character_no,
                                               payload->equipment);
                }
                if (!inventory_ok || !equipment_ok) {
                    payload->result = 2;
                }
                if (CreateSkill(payload) != true) {
                    payload->result = 2;
                }
                if (CreatePvP(payload->character_no) != true) {
                    payload->result = 2;
                }
                if (_createCharacDungeon(payload->account_id) == true) {
                    if (_createCharacQuest(payload->character_no) == true) {
                        if (_createCharacNPCRelationship(
                                payload->character_no) != true) {
                            payload->result = 2;
                        }
                    } else {
                        payload->result = 2;
                    }
                } else {
                    payload->result = 2;
                }
            }
        } else {
            payload->result = 2;
        }
    }
    return SendResult(signal, user_idx, payload) == true;
}

// IDA_EA: 0x840103E
// IDA_NAME: _ZN15DB_CreateCharac28_createCharacNPCRelationshipEj
// DEMANGLED: DB_CreateCharac::_createCharacNPCRelationship(unsigned int)
bool DB_CreateCharac::_createCharacNPCRelationship(unsigned int charac_no) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "inSert into charac_npc(charac_no,npc_cnt,npc_data) values(%d,0,'')",
        charac_no);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x8401094
// IDA_NAME: _ZN15DB_CreateCharac18_createCharacQuestEj
// DEMANGLED: DB_CreateCharac::_createCharacQuest(unsigned int)
bool DB_CreateCharac::_createCharacQuest(unsigned int charac_no) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "inSert into new_charac_quest(charac_no, clear_quest) values(%u,'')",
        charac_no);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x84010FC
// IDA_NAME: _ZN15DB_CreateCharac20_createCharacDungeonEj
// DEMANGLED: DB_CreateCharac::_createCharacDungeon(unsigned int)
bool DB_CreateCharac::_createCharacDungeon(unsigned int account_id) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    const char* account_text = NumberToString(account_id, 0);
    db_handle->set_query("seLect * from member_dungeon where m_id=%s",
                         account_text);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (!db_handle->get_n_rows()) {
        account_text = NumberToString(account_id, 0);
        db_handle->set_query(
            "inSert into member_dungeon (m_id,dungeon) values (%s,'')",
            account_text);
    }
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x84011D4
// IDA_NAME: _ZN15DB_CreateCharac16_checkCharacNameEP5MySQLPKcP17SIG_CREATE_CHARAC
// DEMANGLED: DB_CreateCharac::_checkCharacName(MySQL *,char const*,SIG_CREATE_CHARAC *)
bool DB_CreateCharac::_checkCharacName(MySQL* db_handle,
                                       const char* database_name,
                                       SIG_CREATE_CHARAC* payload) {
    db_handle->set_query(
        "seLect delete_flag from %s.charac_info where charac_name='%s'",
        database_name,
        payload->character_name);
    if (db_handle->exec(true) != true) {
        payload->result = 2;
        return false;
    }
    int delete_flag = 0;
    if (db_handle->get_n_rows()) {
        payload->result = 2;
        if (db_handle->fetch() != true ||
            db_handle->get_int(0, delete_flag) != true) {
            return false;
        }
        if (delete_flag) {
            payload->result = 3;
        } else {
            CEnvironment* env = G_CEnvironment();
            if (env && env->get_gc_no() == 1010) {
                cMyTrace trace(
                    "bool DB_CreateCharac::_checkCharacName(MySQL*, const char*, SIG_CREATE_CHARAC*)",
                    3434,
                    5);
                trace("already charac_name(%s) row(%d)",
                      payload->character_name,
                      db_handle->get_n_rows());
                if (!strcmp(payload->character_name, "")) {
                    cMyTrace query_trace(
                        "bool DB_CreateCharac::_checkCharacName(MySQL*, const char*, SIG_CREATE_CHARAC*)",
                        3437,
                        5);
                    query_trace("already charac_name query(%s)",
                                db_handle->GetQueryString());
                }
            }
            payload->result = 1;
        }
        return false;
    }
    return true;
}

// IDA_EA: 0x84013B2
// IDA_NAME: _ZN15DB_CreateCharac26_checkCreateEventCharacterEP17SIG_CREATE_CHARAC
// DEMANGLED: DB_CreateCharac::_checkCreateEventCharacter(SIG_CREATE_CHARAC *)
bool DB_CreateCharac::_checkCreateEventCharacter(SIG_CREATE_CHARAC* payload) {
    const int pending = QueryPendingDecideLevelupReservation(payload);
    if (pending > 0) {
        return true;
    }
    if (pending < 0) {
        payload->result = 2;
        return false;
    }
    CSystemTime* system_time = GlobalData::system_time();
    const int cur_sec = system_time ? system_time->getCurSec() : 0;
    if (cur_sec > 1258642799 && cur_sec <= 1262206800) {
        if (payload->user_event_character_flag) {
            payload->result = 4;
            return false;
        }
        return true;
    }
    payload->result = 4;
    return false;
}

// IDA_EA: 0x8401428
// IDA_NAME: _ZN15DB_CreateCharac27_updateCreateEventCharacterEP17SIG_CREATE_CHARAC
// DEMANGLED: DB_CreateCharac::_updateCreateEventCharacter(SIG_CREATE_CHARAC *)
bool DB_CreateCharac::_updateCreateEventCharacter(SIG_CREATE_CHARAC* payload) {
    const int pending = QueryPendingDecideLevelupReservation(payload);
    if (pending < 0) {
        payload->result = 2;
        return false;
    }
    CSystemTime* system_time = GlobalData::system_time();
    const int cur_sec = system_time ? system_time->getCurSec() : 0;
    if (pending > 0 || (cur_sec > 1258642799 && cur_sec <= 1262206800)) {
        MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(6),static_cast<ENUM_SERVER_GROUP>( 0));
        const char* account_text = NumberToString(payload->account_id, 0);
        db_handle->set_query(
            "upDate member_login set event_charac_flag = 1 where m_id = %s",
            account_text);
        if (db_handle->exec(true) == true) {
            payload->user_event_character_flag = 1;
            return true;
        }
        payload->result = 2;
        return false;
    }
    payload->result = 4;
    return false;
}

// IDA_EA: 0x8401504
// IDA_NAME: _ZN15DB_CreateCharac25CheckLimitCreateNewCharacEP17SIG_CREATE_CHARAC
// DEMANGLED: DB_CreateCharac::CheckLimitCreateNewCharac(SIG_CREATE_CHARAC *)
bool DB_CreateCharac::CheckLimitCreateNewCharac(SIG_CREATE_CHARAC* payload) {
    if (GlobalData::s_pGMAccounts_ &&
        (GlobalData::s_pGMAccounts_)->isGM(
                                  payload->account_id)) {
        return true;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1),static_cast<ENUM_SERVER_GROUP>( 0));
    CSystemTime* system_time = GlobalData::system_time();
    time_t timer =
        static_cast<time_t>(system_time ? system_time->getCurSec() : 0);
    struct tm tp;
    localtime_r(&timer, &tp);
    memset(&tp, 0, 12);
    timer = mktime(&tp);
    unsigned int create_time = 0;
    const char* account_text = NumberToString(payload->account_id, 0);
    db_handle->set_query(
        "seLect unix_timestamp(create_time) from limit_create_character where m_id=%s",
        account_text);
    if (db_handle->exec(true) != true) {
        payload->result = 2;
        return false;
    }
    const int row_count = db_handle->get_n_rows();
    if (!row_count) {
        return true;
    }
    int today_count = 0;
    for (int row = 0; row < row_count; ++row) {
        if (db_handle->fetch() != true ||
            db_handle->get_uint(0, create_time) != true) {
            break;
        }
        if (static_cast<unsigned int>(timer) <= create_time &&
            ++today_count > 1) {
            payload->result = 5;
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x84016AE
// IDA_NAME: _ZN15DB_CreateCharac26InsertLimitCreateNewCharacEP17SIG_CREATE_CHARAC
// DEMANGLED: DB_CreateCharac::InsertLimitCreateNewCharac(SIG_CREATE_CHARAC *)
bool DB_CreateCharac::InsertLimitCreateNewCharac(SIG_CREATE_CHARAC* payload) {
    if (GlobalData::s_pGMAccounts_ &&
        (GlobalData::s_pGMAccounts_)->isGM(
                                  payload->account_id)) {
        return true;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1),static_cast<ENUM_SERVER_GROUP>( 0));
    const char* account_text = NumberToString(payload->account_id, 0);
    db_handle->set_query(
        "inSert into limit_create_character(m_id, create_time) values(%s, now() )",
        account_text);
    if (db_handle->exec(true) == true) {
        CSystemTime* system_time = GlobalData::system_time();
        time_t timer =
            static_cast<time_t>(system_time ? system_time->getCurSec() : 0);
        struct tm tp;
        struct tm* local_time = localtime_r(&timer, &tp);
        char time_text[256];
        sprintf(time_text,
                "%04d-%02d-%02d %02d:%02d:%02d",
                local_time->tm_year + 1900,
                local_time->tm_mon + 1,
                local_time->tm_mday,
                local_time->tm_hour,
                local_time->tm_min,
                local_time->tm_sec);
        CEnvironment* env = G_CEnvironment();
        const int server_group = env ? env->get_server_group() : 0;
        account_text = NumberToString(payload->account_id, 0);
        cMyTrace trace(
            "bool DB_CreateCharac::InsertLimitCreateNewCharac(SIG_CREATE_CHARAC*)",
            3702,
            0);
        trace("[LIMIT NEW CHARAC] %s,%d,%s,%d",
              account_text,
              server_group,
              time_text,
              payload->character_no);
        return true;
    }
    payload->result = 5;
    return false;
}

// IDA_EA: 0x8401864
// IDA_NAME: _ZN15DB_CreateCharac30makeRequestResetLimitCreateMidEv
// DEMANGLED: DB_CreateCharac::makeRequestResetLimitCreateMid(void)
void DB_CreateCharac::makeRequestResetLimitCreateMid() {
    CEnvironment* env = G_CEnvironment();
    if (env && env->get_channel_no() == 1) {
        Stream* stream =
            GlobalData::stream_pool()->Acquire("DBThread.cpp", 3811);
        CStreamGuard guard(stream, true);
        ARAD::DISPATCHER::make_internal_stream_jpn(
            guard,
            ARAD::kInternalPacketResetLimitedCreateCharacPerMid,
            -1);
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    }
}

// IDA_EA: 0x8401922
// IDA_NAME: _ZN15DB_CreateCharac25CheckLimitCreateNewCharacEP17SIG_CREATE_CHARACb
// DEMANGLED: DB_CreateCharac::CheckLimitCreateNewCharac(SIG_CREATE_CHARAC *,bool)
bool DB_CreateCharac::CheckLimitCreateNewCharac(SIG_CREATE_CHARAC* payload,
                                                bool use_mid_limit) {
    if (!use_mid_limit) {
        return CheckLimitCreateNewCharac(payload);
    }
    if (GlobalData::s_pGMAccounts_ &&
        (GlobalData::s_pGMAccounts_)->isGM(
                                  payload->account_id)) {
        return true;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1),static_cast<ENUM_SERVER_GROUP>( 0));
    CSystemTime* system_time = GlobalData::system_time();
    time_t timer =
        static_cast<time_t>(system_time ? system_time->getCurSec() : 0);
    struct tm tp;
    localtime_r(&timer, &tp);
    memset(&tp, 0, 12);
    timer = mktime(&tp);
    unsigned int count = 0;
    unsigned int last_access_time = 0;
    const char* account_text = NumberToString(payload->account_id, 0);
    db_handle->set_query(
        "seLect count, unix_timestamp(last_access_time) from limit_create_character where m_id=%s",
        account_text);
    if (db_handle->exec(true) != true) {
        payload->result = 2;
        return false;
    }
    const int row_count = db_handle->get_n_rows();
    if (!row_count) {
        return true;
    }
    for (int row = 0; row < row_count; ++row) {
        if (db_handle->fetch() != true ||
            db_handle->get_uint(0, count) != true ||
            db_handle->get_uint(1, last_access_time) != true) {
            break;
        }
        if (count > 1) {
            payload->result = 5;
            CEnvironment* env = G_CEnvironment();
            const int server_group = env ? env->get_server_group() : 0;
            account_text = NumberToString(payload->account_id, 0);
            cMyTrace trace(
                "bool DB_CreateCharac::CheckLimitCreateNewCharac(SIG_CREATE_CHARAC*, bool)",
                3877,
                0);
            trace("[LIMIT NEW CHARAC MID] %s,%d,%d,%d",
                  account_text,
                  server_group,
                  count,
                  last_access_time);
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x8401B9A
// IDA_NAME: _ZN15DB_CreateCharac26InsertLimitCreateNewCharacEP17SIG_CREATE_CHARACb
// DEMANGLED: DB_CreateCharac::InsertLimitCreateNewCharac(SIG_CREATE_CHARAC *,bool)
bool DB_CreateCharac::InsertLimitCreateNewCharac(SIG_CREATE_CHARAC* payload,
                                                 bool use_mid_limit) {
    if (!use_mid_limit) {
        return InsertLimitCreateNewCharac(payload);
    }
    if (GlobalData::s_pGMAccounts_ &&
        (GlobalData::s_pGMAccounts_)->isGM(
                                  payload->account_id)) {
        return true;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1),static_cast<ENUM_SERVER_GROUP>( 0));
    const char* account_text = NumberToString(payload->account_id, 0);
    db_handle->set_query(
        "upDate limit_create_character set count=count+1, last_access_time=now() where m_id=%s",
        account_text);
    if (db_handle->exec(true) != true) {
        payload->result = 2;
        return false;
    }
    if (!db_handle->getAffectedRowCount()) {
        account_text = NumberToString(payload->account_id, 0);
        db_handle->set_query(
            "inSert into limit_create_character(m_id, count, last_access_time) values(%s, 1, now() )",
            account_text);
        if (db_handle->exec(true) != true) {
            payload->result = 2;
            return false;
        }
    }

    CSystemTime* system_time = GlobalData::system_time();
    time_t timer =
        static_cast<time_t>(system_time ? system_time->getCurSec() : 0);
    struct tm tp;
    struct tm* local_time = localtime_r(&timer, &tp);
    char time_text[256];
    sprintf(time_text,
            "%04d-%02d-%02d %02d:%02d:%02d",
            local_time->tm_year + 1900,
            local_time->tm_mon + 1,
            local_time->tm_mday,
            local_time->tm_hour,
            local_time->tm_min,
            local_time->tm_sec);
    CEnvironment* env = G_CEnvironment();
    const int server_group = env ? env->get_server_group() : 0;
    account_text = NumberToString(payload->account_id, 0);
    cMyTrace trace(
        "bool DB_CreateCharac::InsertLimitCreateNewCharac(SIG_CREATE_CHARAC*, bool)",
        3937,
        0);
    trace("[LIMIT NEW CHARAC] %s,%d,%s,%d",
          account_text,
          server_group,
          time_text,
          payload->character_no);
    return true;
}

// IDA_EA: 0x8401DCC
// IDA_NAME: _ZN15DB_CreateCharac15CreateNewCharacEP17SIG_CREATE_CHARAC
// DEMANGLED: DB_CreateCharac::CreateNewCharac(SIG_CREATE_CHARAC *)
bool DB_CreateCharac::CreateNewCharac(SIG_CREATE_CHARAC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    const char* account_text = NumberToString(payload->account_id, 0);
    db_handle->set_query(
        "seLect count(*) from charac_info where m_id=%s and delete_flag=0",
        account_text);
    if (db_handle->exec(true) != true) {
        payload->result = 2;
        return false;
    }
    int charac_count = 0;
    if (db_handle->fetch() != true ||
        db_handle->get_int(0, charac_count) != true || charac_count > 35) {
        payload->result = 2;
        return false;
    }
    CEnvironment* env = G_CEnvironment();
    const STDBConnInfo* game_db = 0;
    if (env) {
        DBConnInfoMap& db_map = env->db_conn_info_map();
        DBConnInfoMap::iterator it =
            db_map.find(static_cast<ENUM_DB_HANDLE_IDX>(2));
        if (it != db_map.end()) {
            game_db = &it->second;
        }
    }
    if (!game_db ||
        _checkCharacName(db_handle, game_db->db_name, payload) != true) {
        return false;
    }

    payload->first_character_flag = 0;
    CSystemTime* system_time = GlobalData::system_time();
    const unsigned int cur_sec =
        static_cast<unsigned int>(system_time ? system_time->getCurSec() : 0);
    if (env &&
        env->get_accept_handshake_flag_888() == 10 &&
        cur_sec <= 0x478E7050u) {
        account_text = NumberToString(payload->account_id, 0);
        db_handle->set_query("seLect count(*) from charac_info where m_id=%s",
                             account_text);
        if (db_handle->exec(true) != true) {
            payload->result = 2;
            return false;
        }
        int all_charac_count = 0;
        if (db_handle->fetch() != true ||
            db_handle->get_int(0, all_charac_count) != true) {
            payload->result = 2;
            return false;
        }
        if (!all_charac_count) {
            payload->first_character_flag = 1;
        }
    }

    char escaped_name[kCreateCharacNameEscapedSize];
    memset(escaped_name, 0, sizeof(escaped_name));
    db_handle->escape_string(escaped_name, payload->character_name);
    if (payload->event_character_grow_type > 0) {
        if (_checkCreateEventCharacter(payload) != true) {
            return false;
        }
        if (_updateCreateEventCharacter(payload) != true) {
            return false;
        }
    } else if (payload->event_character_grow_type < 0) {
        payload->result = 4;
        return false;
    }
    MakeInsertCharacInfo(db_handle, payload, escaped_name);
    if (db_handle->exec(true) == true) {
        return true;
    }
    account_text = NumberToString(payload->account_id, 0);
    cMyTrace trace("bool DB_CreateCharac::CreateNewCharac(SIG_CREATE_CHARAC*)",
                   4322,
                   5);
    trace("DB_CreateCharac::CreateNewCharac, exec() ERROR m_id=%s",
          account_text);
    payload->result = 1;
    return false;
}

// IDA_EA: 0x840215E
// IDA_NAME: _ZN15DB_CreateCharac20_CreateNewCharacStatEP17SIG_CREATE_CHARAC
// DEMANGLED: DB_CreateCharac::_CreateNewCharacStat(SIG_CREATE_CHARAC *)
bool DB_CreateCharac::_CreateNewCharacStat(SIG_CREATE_CHARAC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "inSert into charac_stat (charac_no,village,HP, forbidden_to_play, forbidden_due_to) values(%d,%d,100,0,0)",
        payload->character_no,
        payload->initial_village ? payload->initial_village : 1);
    if (db_handle->exec(true) == true) {
        return true;
    }
    const char* account_text = NumberToString(payload->account_id, 0);
    cMyTrace trace(
        "bool DB_CreateCharac::_CreateNewCharacStat(SIG_CREATE_CHARAC*)",
        4347,
        5);
    trace("DB_CreateCharac::_CreateNewCharacStat, exec() ERROR m_id=%s",
          account_text);
    payload->result = 2;
    return false;
}

// IDA_EA: 0x8402230
// IDA_NAME: _ZN15DB_CreateCharac15CreateInventoryEjP10Inven_Item
// DEMANGLED: DB_CreateCharac::CreateInventory(unsigned int,Inven_Item *)
bool DB_CreateCharac::CreateInventory(unsigned int charac_no,
                                      Inven_Item* inventory) {
    char compressed[kInventoryZipSize];
    memset(compressed, 0, sizeof(compressed));
    unsigned int compressed_len = sizeof(compressed);
    if (inventory) {
        char inventory_bytes[kInventoryRawSize];
        memcpy(inventory_bytes, inventory, sizeof(inventory_bytes));
        if (compress_zip(compressed,
                         compressed_len,
                         inventory_bytes,
                         sizeof(inventory_bytes)) != 1) {
            return false;
        }
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    WongWork::CEventMgr event_manager;
    const int coin = event_manager.GetCoinCharacCreate();
    if (inventory) {
        char* inventory_blob =
            db_handle->blob_to_str(0, compressed, compressed_len);
        db_handle->set_query(
            "inSert into inventory (charac_no, inventory,coin) values(%u, '%s',%d)",
            charac_no,
            inventory_blob,
            coin);
    } else {
        db_handle->set_query(
            "inSert into inventory (charac_no, coin) values(%u, %d)",
            charac_no,
            coin);
    }
    if (db_handle->exec(true) != true) {
        return false;
    }
    db_handle->set_query(
        "inSert into charac_inven_expand(charac_no,cargo,cargo_capacity,jewel) values(%u,'',0,'')",
        charac_no);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x84023AC
// IDA_NAME: _ZN15DB_CreateCharac11CreateEquipEjP10Inven_Item
// DEMANGLED: DB_CreateCharac::CreateEquip(unsigned int,Inven_Item *)
bool DB_CreateCharac::CreateEquip(unsigned int charac_no,
                                  Inven_Item* equip) {
    if (!equip) {
        return true;
    }
    char compressed[kEquipZipSize];
    memset(compressed, 0, sizeof(compressed));
    unsigned int compressed_len = sizeof(compressed);
    char equipment_bytes[sizeof(Inven_Item) * kEquipmentItemCount];
    memcpy(equipment_bytes, equip, sizeof(equipment_bytes));
    if (compress_zip(compressed,
                     compressed_len,
                     equipment_bytes,
                     sizeof(equipment_bytes)) != 1) {
        return false;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    char* equip_blob = db_handle->blob_to_str(0, compressed, compressed_len);
    db_handle->set_query(
        "upDate inventory set equipslot='%s' where charac_no = %u",
        equip_blob,
        charac_no);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x84024E6
// IDA_NAME: _ZN15DB_CreateCharac11CreateSkillEP17SIG_CREATE_CHARAC
// DEMANGLED: DB_CreateCharac::CreateSkill(SIG_CREATE_CHARAC *)
bool DB_CreateCharac::CreateSkill(SIG_CREATE_CHARAC* payload) {
    char skill_slot[kSkillZipSize];
    char skill_slot_2nd[kSkillZipSize];
    char skill_slot_bytes[sizeof(payload->skill_slot)];
    char skill_slot_2nd_bytes[sizeof(payload->skill_slot_second)];
    memset(skill_slot, 0, sizeof(skill_slot));
    memset(skill_slot_2nd, 0, sizeof(skill_slot_2nd));
    memcpy(skill_slot_bytes, payload->skill_slot, sizeof(skill_slot_bytes));
    memcpy(skill_slot_2nd_bytes,
           payload->skill_slot_second,
           sizeof(skill_slot_2nd_bytes));
    unsigned int skill_slot_len = sizeof(skill_slot);
    unsigned int skill_slot_2nd_len = sizeof(skill_slot_2nd);
    if (compress_zip(skill_slot,
                     skill_slot_len,
                     skill_slot_bytes,
                     sizeof(skill_slot_bytes)) != 1) {
        return false;
    }
    if (compress_zip(skill_slot_2nd,
                     skill_slot_2nd_len,
                     skill_slot_2nd_bytes,
                     sizeof(skill_slot_2nd_bytes)) != 1) {
        return false;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    char* skill_slot_2nd_blob =
        db_handle->blob_to_str(1, skill_slot_2nd, skill_slot_2nd_len);
    char* skill_slot_blob =
        db_handle->blob_to_str(0, skill_slot, skill_slot_len);
    db_handle->set_query(
        "inSert into skill (charac_no, remain_sp ,skill_slot, skill_slot_2nd) values(%u,%u, '%s', '%s')",
        payload->character_no,
        payload->remain_sp,
        skill_slot_blob,
        skill_slot_2nd_blob);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x84026EC
// IDA_NAME: _ZN15DB_CreateCharac9CreatePvPEj
// DEMANGLED: DB_CreateCharac::CreatePvP(unsigned int)
bool DB_CreateCharac::CreatePvP(unsigned int charac_no) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query("inSert into pvp_result (charac_no) values(%u)",
                         charac_no);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x840275C
// IDA_NAME: _ZN15DB_CreateCharac11GetCharacNoEP17SIG_CREATE_CHARAC
// DEMANGLED: DB_CreateCharac::GetCharacNo(SIG_CREATE_CHARAC *)
bool DB_CreateCharac::GetCharacNo(SIG_CREATE_CHARAC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    const unsigned int identity = GetIdentityFromDB(db_handle);
    if (!identity) {
        return false;
    }
    payload->character_no = identity;
    return true;
}

// IDA_EA: 0x84027B0
// IDA_NAME: _ZN15DB_CreateCharac10SendResultEiiP17SIG_CREATE_CHARAC
// DEMANGLED: DB_CreateCharac::SendResult(int,int,SIG_CREATE_CHARAC *)
bool DB_CreateCharac::SendResult(int signal,
                                 int user_idx,
                                 SIG_CREATE_CHARAC* payload) {
    Stream* stream = GlobalData::big_stream_pool()->Acquire("DBThread.cpp",
                                                            4571);
    CStreamGuard guard(stream, true);
    *guard << signal;
    *guard << user_idx;
    guard->put_binary(payload, sizeof(*payload));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}
