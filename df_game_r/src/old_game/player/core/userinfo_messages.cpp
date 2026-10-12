#include "old_game/player/core/userinfo_messages.h"

#include <string.h>


typedef char MSG_USERINFO_REQ_Size_Must_Be_0x10[
    (sizeof(MSG_USERINFO_REQ) == 0x10) ? 1 : -1];
typedef char MSG_DECREASE_DURABILITY_Size_Must_Be_0x0e[
    (sizeof(MSG_DECREASE_DURABILITY) == 0x0e) ? 1 : -1];
typedef char MSG_REPAIR_EQUIP_Size_Must_Be_0x12[
    (sizeof(MSG_REPAIR_EQUIP) == 0x12) ? 1 : -1];
typedef char MSG_RENAME_CREATURE_Size_Must_Be_0x21[
    (sizeof(MSG_RENAME_CREATURE) == 0x21) ? 1 : -1];
typedef char MSG_GROWTH_EQUIPMENT_Size_Must_Be_0x15[
    (sizeof(MSG_GROWTH_EQUIPMENT) == 0x15) ? 1 : -1];
typedef char MSG_CHANGE_AVATAR_OPTION_Size_Must_Be_0x1a[
    (sizeof(MSG_CHANGE_AVATAR_OPTION) == 0x1a) ? 1 : -1];

// IDA_EA: 0x822DF1C
// IDA_NAME: _ZN19MSG_RENAME_CREATUREC2Ev
// DEMANGLED: MSG_RENAME_CREATURE::MSG_RENAME_CREATURE(void)
MSG_RENAME_CREATURE::MSG_RENAME_CREATURE()
    : MSG_BASE() {
    memset(name, 0, sizeof(name));
}

// IDA_EA: 0x822E468
// IDA_NAME: _ZN20MSG_GROWTH_EQUIPMENTC2Ev
// DEMANGLED: MSG_GROWTH_EQUIPMENT::MSG_GROWTH_EQUIPMENT(void)
MSG_GROWTH_EQUIPMENT::MSG_GROWTH_EQUIPMENT()
    : MSG_BASE(),
      item_space(0),
      slot(0) {}

// IDA_EA: 0x822E0CC
// IDA_NAME: _ZN24MSG_CHANGE_AVATAR_OPTIONC2Ev
// DEMANGLED: MSG_CHANGE_AVATAR_OPTION::MSG_CHANGE_AVATAR_OPTION(void)
MSG_CHANGE_AVATAR_OPTION::MSG_CHANGE_AVATAR_OPTION()
    : MSG_BASE(),
      material_slot(0xffffu),
      material_item_id(0xffffffffu),
      avatar_slot(0xffffu),
      avatar_item_id(0xffffffffu),
      option(0xffu) {}
