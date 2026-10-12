#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"

#include <map>

typedef std::map<int, STDungeonInoutCnt> CDungeonEntranceLogStorage;

void* GetInstanceDungeonEntranceLog()
    __asm__("_Z29GetInstanceDungeonEntranceLogv");

// IDA_EA: 0x860E15B
// IDA_NAME: _Z29GetInstanceDungeonEntranceLogv
// DEMANGLED: GetInstanceDungeonEntranceLog(void)
void* GetInstanceDungeonEntranceLog() {
    static CDungeonEntranceLogStorage instance;
    return &instance;
}
