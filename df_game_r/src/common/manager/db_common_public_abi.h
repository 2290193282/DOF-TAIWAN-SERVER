#ifndef OLDSRV_GLOBAL_DB_COMMON_PUBLIC_ABI_H_
#define OLDSRV_GLOBAL_DB_COMMON_PUBLIC_ABI_H_

class MySQL;

namespace WongWork {
namespace DBCommon {

unsigned int GetIdentity(MySQL* db);

}  // namespace DBCommon
}  // namespace WongWork

unsigned int GetIdentityFromDB(MySQL* db)
    __asm__("_Z17GetIdentityFromDBP5MySQL");

const char* get_skill_slot_name(char skill_tree_index)
    __asm__("_Z19get_skill_slot_namec");
const char* get_skill_table_name(char skill_tree_index)
    __asm__("_Z20get_skill_table_nameb");
const char* get_member_game_option()
    __asm__("_Z22get_member_game_optionv");
const char* get_item_count_table_name()
    __asm__("_Z25get_item_count_table_namev");
const char* getComboSkillTableName(int skill_tree_kind)
    __asm__("_Z22getComboSkillTableName20ENUM_SKILL_TREE_KIND");

#endif  // OLDSRV_GLOBAL_DB_COMMON_PUBLIC_ABI_H_
