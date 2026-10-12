#include "old_game/appearance/avatar/creature_script_public_abi.h"

// IDA_DATA_EA: CreatureScript::ExpTable_ consumed by initCreatureScript
// @0x8A35D37. Zero-initialized BSS is the old pre-import state.
int32_t CreatureScript::ExpTable_[99];

typedef char ExpTable_Count_Must_Be_99[
    (sizeof(CreatureScript::ExpTable_) == 99u * sizeof(int32_t)) ? 1 : -1];
