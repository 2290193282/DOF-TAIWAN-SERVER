#ifndef OLDSRV_OLD_DATA_DB_CHARACTER_ARAD_CREATE_QUERY_H_
#define OLDSRV_OLD_DATA_DB_CHARACTER_ARAD_CREATE_QUERY_H_

#include "old_data/db/character/create_charac_payload.h"


class MySQL;

namespace ARAD {
namespace DATABASE {
namespace CREATE_QUERY {

int arad_sp_charater_name_check(MySQL* db_handle,
                                SIG_CREATE_CHARAC* payload,
                                const char* database_name);
int arad_sp_get_characer_index(MySQL* db_handle,
                               SIG_CREATE_CHARAC* payload);
int arad_sp_create_character_check(MySQL* db_handle,
                                   SIG_CREATE_CHARAC* payload);
int arad_sp_create_character(MySQL* db_handle,
                             SIG_CREATE_CHARAC* payload);
int arad_sp_create_charac_stat(MySQL* db_handle,
                               SIG_CREATE_CHARAC* payload);
int arad_sp_create_inven(MySQL* db_handle, SIG_CREATE_CHARAC* payload);
int arad_sp_create_expand_inven(MySQL* db_handle,
                                SIG_CREATE_CHARAC* payload);
int arad_sp_create_equip(MySQL* db_handle, SIG_CREATE_CHARAC* payload);
int arad_sp_create_skill(MySQL* db_handle, SIG_CREATE_CHARAC* payload);
int arad_sp_create_pvp(MySQL* db_handle, SIG_CREATE_CHARAC* payload);
int arad_sp_create_dungeon_point(MySQL* db_handle,
                                 SIG_CREATE_CHARAC* payload);
int arad_sp_create_charac_dungeon(MySQL* db_handle,
                                  SIG_CREATE_CHARAC* payload);
int arad_sp_create_charac_quest(MySQL* db_handle,
                                SIG_CREATE_CHARAC* payload);
int arad_sp_create_npc_relationship(MySQL* db_handle,
                                    SIG_CREATE_CHARAC* payload);

}  // namespace CREATE_QUERY
}  // namespace DATABASE
}  // namespace ARAD

#endif  // OLDSRV_OLD_DATA_DB_CHARACTER_ARAD_CREATE_QUERY_H_
