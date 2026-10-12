#include "old_game/player/core/arad_db_charac_dispatcher.h"

#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_data/db/character/arad_create_query.h"
#include "infra/network/packet/stream.h"

#include <stdint.h>

namespace ARAD {

// IDA_EA: 0x8184114
// IDA_NAME: _ZN4ARAD25Arad_DB_Charac_Dispatcher8dispatchEiiP6Stream
// DEMANGLED: ARAD::Arad_DB_Charac_Dispatcher::dispatch(int,int,Stream *)
int Arad_DB_Charac_Dispatcher::dispatch(int, int, Stream* stream) {
    SIG_CREATE_CHARAC* payload = stream->GetOutBuffer<SIG_CREATE_CHARAC>();
    if (!payload) {
        LogManager::logFormat(
            1, "localjapan/Arad_Character.cpp",
            "virtual int ARAD::Arad_DB_Charac_Dispatcher::dispatch(int, int, Stream*)",
            0x18, "%s", "Context Null");
        return -1;
    }

    if (create_charac(payload)) {
        return -1;
    }
    if (create_charac_stat(payload)) {
        return -1;
    }
    if (create_inventory(payload)) {
        return -1;
    }
    if (create_equip(payload)) {
        return -1;
    }
    if (create_skill(payload)) {
        return -1;
    }
    if (create_pvp(payload)) {
        return -1;
    }
    if (create_charac_dungeon(payload)) {
        return -1;
    }
    if (create_charac_quest(payload)) {
        return -1;
    }
    return 0;
}

// IDA_EA: 0x8184290
// IDA_NAME: _ZN4ARAD25Arad_DB_Charac_Dispatcher13create_characEP17SIG_CREATE_CHARAC
// DEMANGLED: ARAD::Arad_DB_Charac_Dispatcher::create_charac(SIG_CREATE_CHARAC *)
int Arad_DB_Charac_Dispatcher::create_charac(SIG_CREATE_CHARAC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, SERVER_GROUP_DEFAULT);
    if (!db_handle) {
        LogManager::logFormat(
            1, "localjapan/Arad_Character.cpp",
            "int ARAD::Arad_DB_Charac_Dispatcher::create_charac(SIG_CREATE_CHARAC*)",
            0x4E, "%s", "Database null!!");
        return -1;
    }
    if (DATABASE::CREATE_QUERY::arad_sp_create_character(db_handle, payload)) {
        return -1;
    }
    if (DATABASE::CREATE_QUERY::arad_sp_get_characer_index(db_handle,
                                                            payload)) {
        return 89;
    }
    return 0;
}

// IDA_EA: 0x8184342
// IDA_NAME: _ZN4ARAD25Arad_DB_Charac_Dispatcher18create_charac_statEP17SIG_CREATE_CHARAC
// DEMANGLED: ARAD::Arad_DB_Charac_Dispatcher::create_charac_stat(SIG_CREATE_CHARAC *)
int Arad_DB_Charac_Dispatcher::create_charac_stat(SIG_CREATE_CHARAC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, SERVER_GROUP_DEFAULT);
    if (!db_handle) {
        LogManager::logFormat(
            1, "localjapan/Arad_Character.cpp",
            "int ARAD::Arad_DB_Charac_Dispatcher::create_charac_stat(SIG_CREATE_CHARAC*)",
            0x67, "%s", "Database null!!");
        return -1;
    }
    return DATABASE::CREATE_QUERY::arad_sp_create_charac_stat(db_handle,
                                                                payload)
               ? -1
               : 0;
}

// IDA_EA: 0x81843D2
// IDA_NAME: _ZN4ARAD25Arad_DB_Charac_Dispatcher16create_inventoryEP17SIG_CREATE_CHARAC
// DEMANGLED: ARAD::Arad_DB_Charac_Dispatcher::create_inventory(SIG_CREATE_CHARAC *)
int Arad_DB_Charac_Dispatcher::create_inventory(SIG_CREATE_CHARAC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_3, SERVER_GROUP_DEFAULT);
    if (!db_handle) {
        LogManager::logFormat(
            1, "localjapan/Arad_Character.cpp",
            "int ARAD::Arad_DB_Charac_Dispatcher::create_inventory(SIG_CREATE_CHARAC*)",
            0x7B, "%s", "Database null!!");
        return -1;
    }
    if (DATABASE::CREATE_QUERY::arad_sp_create_inven(db_handle, payload)) {
        return -1;
    }
    return DATABASE::CREATE_QUERY::arad_sp_create_expand_inven(db_handle,
                                                                 payload)
               ? -1
               : 0;
}

// IDA_EA: 0x8184484
// IDA_NAME: _ZN4ARAD25Arad_DB_Charac_Dispatcher12create_equipEP17SIG_CREATE_CHARAC
// DEMANGLED: ARAD::Arad_DB_Charac_Dispatcher::create_equip(SIG_CREATE_CHARAC *)
int Arad_DB_Charac_Dispatcher::create_equip(SIG_CREATE_CHARAC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_3, SERVER_GROUP_DEFAULT);
    if (!db_handle) {
        LogManager::logFormat(
            1, "localjapan/Arad_Character.cpp",
            "int ARAD::Arad_DB_Charac_Dispatcher::create_equip(SIG_CREATE_CHARAC*)",
            0x95, "%s", "Database null!!");
        return -1;
    }
    if (reinterpret_cast<intptr_t>(payload) == -19969) {
        return 0;
    }
    return DATABASE::CREATE_QUERY::arad_sp_create_equip(db_handle, payload)
               ? -1
               : 0;
}

// IDA_EA: 0x8184528
// IDA_NAME: _ZN4ARAD25Arad_DB_Charac_Dispatcher12create_skillEP17SIG_CREATE_CHARAC
// DEMANGLED: ARAD::Arad_DB_Charac_Dispatcher::create_skill(SIG_CREATE_CHARAC *)
int Arad_DB_Charac_Dispatcher::create_skill(SIG_CREATE_CHARAC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_3, SERVER_GROUP_DEFAULT);
    if (!db_handle) {
        LogManager::logFormat(
            1, "localjapan/Arad_Character.cpp",
            "int ARAD::Arad_DB_Charac_Dispatcher::create_skill(SIG_CREATE_CHARAC*)",
            0xB0, "%s", "Database null!!");
        return -1;
    }
    return DATABASE::CREATE_QUERY::arad_sp_create_skill(db_handle, payload)
               ? -1
               : 0;
}

// IDA_EA: 0x81845B8
// IDA_NAME: _ZN4ARAD25Arad_DB_Charac_Dispatcher10create_pvpEP17SIG_CREATE_CHARAC
// DEMANGLED: ARAD::Arad_DB_Charac_Dispatcher::create_pvp(SIG_CREATE_CHARAC *)
int Arad_DB_Charac_Dispatcher::create_pvp(SIG_CREATE_CHARAC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, SERVER_GROUP_DEFAULT);
    if (!db_handle) {
        LogManager::logFormat(
            1, "localjapan/Arad_Character.cpp",
            "int ARAD::Arad_DB_Charac_Dispatcher::create_pvp(SIG_CREATE_CHARAC*)",
            0xBF, "%s", "Database null!!");
        return -1;
    }
    return DATABASE::CREATE_QUERY::arad_sp_create_pvp(db_handle, payload)
               ? -1
               : 0;
}

// IDA_EA: 0x8184648
// IDA_NAME: _ZN4ARAD25Arad_DB_Charac_Dispatcher21create_charac_dungeonEP17SIG_CREATE_CHARAC
// DEMANGLED: ARAD::Arad_DB_Charac_Dispatcher::create_charac_dungeon(SIG_CREATE_CHARAC *)
int Arad_DB_Charac_Dispatcher::create_charac_dungeon(
    SIG_CREATE_CHARAC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, SERVER_GROUP_DEFAULT);
    if (!db_handle) {
        LogManager::logFormat(
            1, "localjapan/Arad_Character.cpp",
            "int ARAD::Arad_DB_Charac_Dispatcher::create_charac_dungeon(SIG_CREATE_CHARAC*)",
            0xCD, "%s", "Database null!!");
        return -1;
    }
    return DATABASE::CREATE_QUERY::arad_sp_create_charac_dungeon(db_handle,
                                                                   payload)
               ? -1
               : 0;
}

// IDA_EA: 0x81846D8
// IDA_NAME: _ZN4ARAD25Arad_DB_Charac_Dispatcher19create_charac_questEP17SIG_CREATE_CHARAC
// DEMANGLED: ARAD::Arad_DB_Charac_Dispatcher::create_charac_quest(SIG_CREATE_CHARAC *)
int Arad_DB_Charac_Dispatcher::create_charac_quest(SIG_CREATE_CHARAC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, SERVER_GROUP_DEFAULT);
    if (!db_handle) {
        LogManager::logFormat(
            1, "localjapan/Arad_Character.cpp",
            "int ARAD::Arad_DB_Charac_Dispatcher::create_charac_quest(SIG_CREATE_CHARAC*)",
            0xDB, "%s", "Database null!!");
        return -1;
    }
    return DATABASE::CREATE_QUERY::arad_sp_create_charac_quest(db_handle,
                                                                 payload)
               ? -1
               : 0;
}

// IDA_EA: 0x8184768
// IDA_NAME: _ZN4ARAD25Arad_DB_Charac_Dispatcher30create_charac_npc_relationshipEP17SIG_CREATE_CHARAC
// DEMANGLED: ARAD::Arad_DB_Charac_Dispatcher::create_charac_npc_relationship(SIG_CREATE_CHARAC *)
int Arad_DB_Charac_Dispatcher::create_charac_npc_relationship(
    SIG_CREATE_CHARAC* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, SERVER_GROUP_DEFAULT);
    if (!db_handle) {
        LogManager::logFormat(
            1, "localjapan/Arad_Character.cpp",
            "int ARAD::Arad_DB_Charac_Dispatcher::create_charac_npc_relationship(SIG_CREATE_CHARAC*)",
            0xE9, "%s", "Database null!!");
        return -1;
    }
    return DATABASE::CREATE_QUERY::arad_sp_create_npc_relationship(db_handle,
                                                                     payload)
               ? -1
               : 0;
}

}  // namespace ARAD
