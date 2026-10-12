#include "old_data/db/character/arad_create_query.h"

#include "common/config.h"
#include "old_data/db/core/db_conn_info.h"
#include "core/config/env/environment.h"
#include "common/types/compressed_blob.h"
#include "common/manager/reliable_person.h"
#include "infra/log/format/trace.h"
#include "old_data/db/core/mysql.h"
#include "old_game/activity/schedule/event_mgr_runtime.h"

#include <map>
#include <string.h>


namespace {

const char kAradDbQueryFile[] = "localjapan/Arad_DB_Query.cpp";

enum {
    kCreateCharacInventoryRawSize =
        oldsrv::config::kMainInventoryPrimaryItemCount *
        oldsrv::config::kInventoryItemRecordSize,
    // zlib compressBound(raw) plus compress_zip's four-byte length prefix.
    kCreateCharacInventoryZipSize =
        kCreateCharacInventoryRawSize +
        (kCreateCharacInventoryRawSize >> 12) +
        (kCreateCharacInventoryRawSize >> 14) +
        (kCreateCharacInventoryRawSize >> 25) + 17,
};

void LogQueryFailure(const char* function,
                     int line,
                     const char* message) {
    LogManager::logFormat(1,
                          kAradDbQueryFile,
                          function,
                          line,
                          "%s",
                          message);
}

}  // namespace

namespace ARAD {
namespace DATABASE {
namespace CREATE_QUERY {

// IDA_EA: 0x818AC54
// IDA_NAME: _ZN4ARAD8DATABASE12CREATE_QUERY27arad_sp_charater_name_checkEP5MySQLP17SIG_CREATE_CHARACPKc
// DEMANGLED: ARAD::DATABASE::CREATE_QUERY::arad_sp_charater_name_check(MySQL *,SIG_CREATE_CHARAC *,char const*)
int arad_sp_charater_name_check(MySQL* db_handle,
                                SIG_CREATE_CHARAC* payload,
                                const char* database_name) {
    if (db_handle->select(
            "seLect delete_flag from %s.charac_info where charac_name='%s'",
            database_name,
            payload->character_name)) {
        payload->result = 2;
        LogQueryFailure(
            "int ARAD::DATABASE::CREATE_QUERY::arad_sp_charater_name_check(MySQL*, SIG_CREATE_CHARAC*, const char*)",
            0x1F,
            "select error");
        return -1;
    }

    int delete_flag = 0;
    if (!db_handle->get_n_rows()) {
        return 0;
    }

    payload->result = 2;
    if (db_handle->get_int(0, delete_flag)) {
        payload->result = delete_flag ? 3u : 1u;
        return -1;
    }

    LogQueryFailure(
        "int ARAD::DATABASE::CREATE_QUERY::arad_sp_charater_name_check(MySQL*, SIG_CREATE_CHARAC*, const char*)",
        0x28,
        "Parsing Error of Selected Field");
    return -1;
}

// IDA_EA: 0x818AD8B
// IDA_NAME: _ZN4ARAD8DATABASE12CREATE_QUERY26arad_sp_get_characer_indexEP5MySQLP17SIG_CREATE_CHARAC
// DEMANGLED: ARAD::DATABASE::CREATE_QUERY::arad_sp_get_characer_index(MySQL *,SIG_CREATE_CHARAC *)
int arad_sp_get_characer_index(MySQL* db_handle,
                               SIG_CREATE_CHARAC* payload) {
    if (db_handle->select("seLect @@identity")) {
        LogQueryFailure(
            "int ARAD::DATABASE::CREATE_QUERY::arad_sp_get_characer_index(MySQL*, SIG_CREATE_CHARAC*)",
            0x47,
            "select @@identity error");
        return -1;
    }

    if (!db_handle->get_uint(0, payload->character_no)) {
        LogQueryFailure(
            "int ARAD::DATABASE::CREATE_QUERY::arad_sp_get_characer_index(MySQL*, SIG_CREATE_CHARAC*)",
            0x4C,
            "Parsing Error of Selected Field");
        return -1;
    }
    if (payload->character_no) {
        return 0;
    }

    LogQueryFailure(
        "int ARAD::DATABASE::CREATE_QUERY::arad_sp_get_characer_index(MySQL*, SIG_CREATE_CHARAC*)",
        0x51,
        "charac_no error");
    return -1;
}

// IDA_EA: 0x818AEA2
// IDA_NAME: _ZN4ARAD8DATABASE12CREATE_QUERY30arad_sp_create_character_checkEP5MySQLP17SIG_CREATE_CHARAC
// DEMANGLED: ARAD::DATABASE::CREATE_QUERY::arad_sp_create_character_check(MySQL *,SIG_CREATE_CHARAC *)
int arad_sp_create_character_check(MySQL* db_handle,
                                   SIG_CREATE_CHARAC* payload) {
    if (db_handle->select(
            "seLect count(*) from charac_info where m_id=%d and delete_flag=0",
            payload->account_id)) {
        LogQueryFailure(
            "int ARAD::DATABASE::CREATE_QUERY::arad_sp_create_character_check(MySQL*, SIG_CREATE_CHARAC*)",
            0x62,
            "select charac_info error");
        return -1;
    }

    int character_count = 0;
    if (!db_handle->get_int(0, character_count) || character_count > 35) {
        payload->result = 2;
        LogQueryFailure(
            "int ARAD::DATABASE::CREATE_QUERY::arad_sp_create_character_check(MySQL*, SIG_CREATE_CHARAC*)",
            0x69,
            "max charac slot limit");
        return -1;
    }
    return 0;
}

// IDA_EA: 0x818AF9A
// IDA_NAME: _ZN4ARAD8DATABASE12CREATE_QUERY24arad_sp_create_characterEP5MySQLP17SIG_CREATE_CHARAC
// DEMANGLED: ARAD::DATABASE::CREATE_QUERY::arad_sp_create_character(MySQL *,SIG_CREATE_CHARAC *)
int arad_sp_create_character(MySQL* db_handle,
                             SIG_CREATE_CHARAC* payload) {
    if (arad_sp_create_character_check(db_handle, payload)) {
        return -1;
    }

    DBConnInfoMap& db_connections =
        G_CEnvironment()->db_conn_info_map();
    const char* database_name = db_connections[DB_HANDLE_2].db_name;
    if (arad_sp_charater_name_check(db_handle, payload, database_name)) {
        return -1;
    }

    char escaped_name[60];
    memset(escaped_name, 0, sizeof(escaped_name));
    db_handle->escape_string(escaped_name, payload->character_name);

    char* spec_property = db_handle->blob_to_str(
        1, payload->initial_stats.spec_property,
        sizeof(payload->initial_stats.spec_property));
    char* element_resist = db_handle->blob_to_str(
        0, payload->initial_stats.element_resist,
        sizeof(payload->initial_stats.element_resist));

    return db_handle->insert(
               "inSert into charac_info ( m_id,charac_name,job,sex,maxHP,maxMP,phy_attack,phy_defense,mag_attack,mag_defense,element_resist,spec_property,inven_weight,hp_regen,mp_regen,move_speed,attack_speed,cast_speed,hit_recovery,jump,charac_weight,max_fatigue,create_time,expert_job) values(%u,'%s',%d,%d,%d,%u,%d,%d,%d,%d,'%s','%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,NOW(),%d)",
               payload->account_id,
               escaped_name,
               static_cast<int>(payload->job),
               static_cast<int>(payload->sex),
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
               static_cast<int>(payload->expert_job))
               ? -1
               : 0;
}

// IDA_EA: 0x818B26D
// IDA_NAME: _ZN4ARAD8DATABASE12CREATE_QUERY26arad_sp_create_charac_statEP5MySQLP17SIG_CREATE_CHARAC
// DEMANGLED: ARAD::DATABASE::CREATE_QUERY::arad_sp_create_charac_stat(MySQL *,SIG_CREATE_CHARAC *)
int arad_sp_create_charac_stat(MySQL* db_handle,
                               SIG_CREATE_CHARAC* payload) {
    if (!db_handle->insert(
            "inSert into charac_stat  (charac_no,village,HP, forbidden_to_play, forbidden_due_to)  values(%d,%d,100,0,0) ",
            payload->character_no,
            payload->initial_village ? payload->initial_village : 1)) {
        return 0;
    }
    payload->result = 2;
    return -1;
}

// IDA_EA: 0x818B2B7
// IDA_NAME: _ZN4ARAD8DATABASE12CREATE_QUERY20arad_sp_create_invenEP5MySQLP17SIG_CREATE_CHARAC
// DEMANGLED: ARAD::DATABASE::CREATE_QUERY::arad_sp_create_inven(MySQL *,SIG_CREATE_CHARAC *)
int arad_sp_create_inven(MySQL* db_handle, SIG_CREATE_CHARAC* payload) {
    WongWork::CEventMgr event_manager;
    char compressed[kCreateCharacInventoryZipSize];
    memset(compressed, 0, sizeof(compressed));
    unsigned int compressed_length = sizeof(compressed);

    if (payload->has_inventory) {
        char inventory_bytes[sizeof(payload->inventory)];
        memcpy(inventory_bytes,
               payload->inventory,
               sizeof(payload->inventory));
        if (compress_zip(compressed,
                         compressed_length,
                         inventory_bytes,
                         sizeof(inventory_bytes))) {
            return -1;
        }

        const int coin = event_manager.GetCoinCharacCreate();
        char* original_inventory = db_handle->blob_to_str(
            0, payload->inventory, sizeof(payload->inventory));
        char* compressed_inventory = db_handle->blob_to_str(
            0, compressed, compressed_length);

        // The old 0x818B38F call pushes four variadic arguments although the
        // format has three conversions.  Preserve that exact stack order;
        // the fourth coin argument is ignored by the formatter.
        if (db_handle->insert(
                "inSert into inventory (charac_no, inventory,coin) values(%u, '%s',%d)",
                payload->character_no,
                compressed_inventory,
                original_inventory,
                coin)) {
            return -1;
        }
    } else {
        // IDA 0x818B3CB supplies only character_no to this two-conversion
        // format and then tests GetCoinCharacCreate(), not insert().
        db_handle->insert(
            "inSert into inventory  (charac_no, coin, money)  values(%u, %d) ",
            payload->character_no);
        if (event_manager.GetCoinCharacCreate()) {
            return -1;
        }
    }
    return 0;
}

// IDA_EA: 0x818B406
// IDA_NAME: _ZN4ARAD8DATABASE12CREATE_QUERY27arad_sp_create_expand_invenEP5MySQLP17SIG_CREATE_CHARAC
// DEMANGLED: ARAD::DATABASE::CREATE_QUERY::arad_sp_create_expand_inven(MySQL *,SIG_CREATE_CHARAC *)
int arad_sp_create_expand_inven(MySQL* db_handle,
                                SIG_CREATE_CHARAC* payload) {
    return db_handle->insert(
               " inSert into charac_inven_expand  (charac_no,cargo,cargo_capacity, jewel)  values(%u,'',0,'')",
               payload->character_no)
               ? -1
               : 0;
}

// IDA_EA: 0x818B443
// IDA_NAME: _ZN4ARAD8DATABASE12CREATE_QUERY20arad_sp_create_equipEP5MySQLP17SIG_CREATE_CHARAC
// DEMANGLED: ARAD::DATABASE::CREATE_QUERY::arad_sp_create_equip(MySQL *,SIG_CREATE_CHARAC *)
int arad_sp_create_equip(MySQL* db_handle, SIG_CREATE_CHARAC* payload) {
    char compressed[0x333];
    memset(compressed, 0, sizeof(compressed));
    unsigned int compressed_length = sizeof(compressed);
    char equipment_bytes[sizeof(payload->equipment)];
    memcpy(equipment_bytes,
           payload->equipment,
           sizeof(payload->equipment));

    if (compress_zip(compressed,
                     compressed_length,
                     equipment_bytes,
                     sizeof(equipment_bytes)) != 1) {
        LogQueryFailure(
            "int ARAD::DATABASE::CREATE_QUERY::arad_sp_create_equip(MySQL*, SIG_CREATE_CHARAC*)",
            0x155,
            "equip compress zip fail");
        return -1;
    }

    char* equipment = db_handle->blob_to_str(
        0, compressed, compressed_length);
    return db_handle->update(
               " upDate inventory set equipslot='%s' where charac_no = %u ",
               equipment,
               payload->character_no)
               ? -1
               : 0;
}

// IDA_EA: 0x818B588
// IDA_NAME: _ZN4ARAD8DATABASE12CREATE_QUERY20arad_sp_create_skillEP5MySQLP17SIG_CREATE_CHARAC
// DEMANGLED: ARAD::DATABASE::CREATE_QUERY::arad_sp_create_skill(MySQL *,SIG_CREATE_CHARAC *)
int arad_sp_create_skill(MySQL* db_handle, SIG_CREATE_CHARAC* payload) {
    char* skill_slot = db_handle->blob_to_str(
        0, payload->skill_slot, sizeof(payload->skill_slot));
    return db_handle->insert(
               "inSert into skill (charac_no, skill_slot) values(%u, '%s')",
               payload->character_no,
               skill_slot)
               ? -1
               : 0;
}

// IDA_EA: 0x818B5F4
// IDA_NAME: _ZN4ARAD8DATABASE12CREATE_QUERY18arad_sp_create_pvpEP5MySQLP17SIG_CREATE_CHARAC
// DEMANGLED: ARAD::DATABASE::CREATE_QUERY::arad_sp_create_pvp(MySQL *,SIG_CREATE_CHARAC *)
int arad_sp_create_pvp(MySQL* db_handle, SIG_CREATE_CHARAC* payload) {
    return db_handle->insert(
               "inSert into pvp_result (charac_no) values(%u)",
               payload->character_no)
               ? -1
               : 0;
}

// IDA_EA: 0x818B638
// IDA_NAME: _ZN4ARAD8DATABASE12CREATE_QUERY28arad_sp_create_dungeon_pointEP5MySQLP17SIG_CREATE_CHARAC
// DEMANGLED: ARAD::DATABASE::CREATE_QUERY::arad_sp_create_dungeon_point(MySQL *,SIG_CREATE_CHARAC *)
int arad_sp_create_dungeon_point(MySQL*, SIG_CREATE_CHARAC*) {
    return -1;
}

// IDA_EA: 0x818B642
// IDA_NAME: _ZN4ARAD8DATABASE12CREATE_QUERY29arad_sp_create_charac_dungeonEP5MySQLP17SIG_CREATE_CHARAC
// DEMANGLED: ARAD::DATABASE::CREATE_QUERY::arad_sp_create_charac_dungeon(MySQL *,SIG_CREATE_CHARAC *)
int arad_sp_create_charac_dungeon(MySQL* db_handle,
                                  SIG_CREATE_CHARAC* payload) {
    return db_handle->insert(
               "inSert into charac_dungeon(charac_no, dungeon) values(%d, '')",
               payload->character_no)
               ? -1
               : 0;
}

// IDA_EA: 0x818B68C
// IDA_NAME: _ZN4ARAD8DATABASE12CREATE_QUERY27arad_sp_create_charac_questEP5MySQLP17SIG_CREATE_CHARAC
// DEMANGLED: ARAD::DATABASE::CREATE_QUERY::arad_sp_create_charac_quest(MySQL *,SIG_CREATE_CHARAC *)
int arad_sp_create_charac_quest(MySQL* db_handle,
                                SIG_CREATE_CHARAC* payload) {
    return db_handle->insert(
               "inSert into charac_quest(charac_no,quest_10,quest_15,quest_20,quest_30,quest_40,quest_50, quest_60,quest_70,quest_etc,play_1,play_1_trigger,play_2,play_2_trigger,play_3,play_3_trigger,play_4,play_4_trigger,play_5,play_5_trigger,play_6,play_6_trigger,play_7,play_7_trigger,play_8,play_8_trigger,play_9,play_9_trigger,play_10,play_10_trigger) values(%d,'','','','','','','','','',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0)",
               payload->character_no)
               ? -1
               : 0;
}

// IDA_EA: 0x818B6C9
// IDA_NAME: _ZN4ARAD8DATABASE12CREATE_QUERY31arad_sp_create_npc_relationshipEP5MySQLP17SIG_CREATE_CHARAC
// DEMANGLED: ARAD::DATABASE::CREATE_QUERY::arad_sp_create_npc_relationship(MySQL *,SIG_CREATE_CHARAC *)
int arad_sp_create_npc_relationship(MySQL* db_handle,
                                    SIG_CREATE_CHARAC* payload) {
    return db_handle->insert(
               "inSert into charac_npc(charac_no,npc_cnt,npc_data) values(%d,0,'')",
               payload->character_no)
               ? -1
               : 0;
}

}  // namespace CREATE_QUERY
}  // namespace DATABASE
}  // namespace ARAD
