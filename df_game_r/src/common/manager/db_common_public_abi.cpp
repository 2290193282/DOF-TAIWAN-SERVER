#include "common/manager/db_common_public_abi.h"

#include "old_data/db/core/mysql.h"
#include "old_game/world/region/game_world.h"

namespace WongWork {
namespace DBCommon {

// IDA_EA: 0x83F9AD4
// IDA_NAME: _ZN8WongWork8DBCommon11GetIdentityEP5MySQL
// DEMANGLED: WongWork::DBCommon::GetIdentity(MySQL *)
unsigned int GetIdentity(MySQL* db) {
    unsigned int identity = 0;
    db->set_query("seLect @@identity");
    if (db->exec(true) != true) {
        return 0;
    }
    if (db->fetch() != true) {
        return 0;
    }
    if (db->get_uint(0, identity) == true) {
        return identity;
    }
    return 0;
}

}  // namespace DBCommon
}  // namespace WongWork

// IDA_EA: 0x83FBC66
// IDA_NAME: _Z17GetIdentityFromDBP5MySQL
// DEMANGLED: GetIdentityFromDB(MySQL *)
unsigned int GetIdentityFromDB(MySQL* db) {
    return WongWork::DBCommon::GetIdentity(db);
}

// IDA_EA: 0x83FBD93
// IDA_NAME: _Z19get_skill_slot_namec
// DEMANGLED: get_skill_slot_name(char)
const char* get_skill_slot_name(char skill_tree_index) {
    if (skill_tree_index == -1 || skill_tree_index == 0) {
        return "skill_slot";
    }
    if (skill_tree_index == 1) {
        return "skill_slot_2nd";
    }
    return "skill_slot";
}

// IDA_EA: 0x83FBDC6
// IDA_NAME: _Z20get_skill_table_nameb
// DEMANGLED: get_skill_table_name(bool)
const char* get_skill_table_name(char skill_tree_index) {
    GameWorld* world = G_GameWorld();
    if (world->IsPvPSkilTreeChannel() || skill_tree_index) {
        return "skill_fair_pvp";
    }
    return "skill";
}

// IDA_EA: 0x83FBE25
// IDA_NAME: _Z22get_member_game_optionv
// DEMANGLED: get_member_game_option(void)
const char* get_member_game_option() {
    return "member_game_option";
}

// IDA_EA: 0x83FBE07
// IDA_NAME: _Z25get_item_count_table_namev
// DEMANGLED: get_item_count_table_name(void)
const char* get_item_count_table_name() {
    return "event_item_count_value";
}

// IDA_EA: 0x844A38B
// IDA_NAME: _Z22getComboSkillTableName20ENUM_SKILL_TREE_KIND
// DEMANGLED: getComboSkillTableName(ENUM_SKILL_TREE_KIND)
const char* getComboSkillTableName(int skill_tree_kind) {
    if (skill_tree_kind == 0) {
        return "combo_skill";
    }
    if (skill_tree_kind == 1) {
        return "combo_skill_2nd";
    }
    return "combo_skill";
}

// Current restored DB sources declared these two small helpers with unsigned
// char parameters. The old binary exports signed-char symbols. Keep the
// compatibility names bound to the same IDA-backed bodies.
__asm__(
    ".globl _Z19get_skill_slot_nameh\n"
    ".set _Z19get_skill_slot_nameh,_Z19get_skill_slot_namec\n"
    ".globl _Z20get_skill_table_nameh\n"
    ".set _Z20get_skill_table_nameh,_Z20get_skill_table_nameb");
