#include "old_game/dungeon/instance/revenge_dungeon_runtime.h"

namespace village_attacked {

// IDA_EA: 0x82AA6A4
// IDA_NAME: _ZN16village_attacked15CRevengeDungeonC2Ev
// DEMANGLED: village_attacked::CRevengeDungeon::CRevengeDungeon(void)
CRevengeDungeon::CRevengeDungeon()
    : is_open_(0),
      raw_01_(),
      close_time_(0) {}

// IDA_EA: 0x8234FF6
// IDA_NAME: _ZN16village_attacked15CRevengeDungeon20IsOpenRevengeDungeonEv
// DEMANGLED: village_attacked::CRevengeDungeon::IsOpenRevengeDungeon(void)
int CRevengeDungeon::IsOpenRevengeDungeon() {
    return is_open_;
}

// IDA_EA: 0x847DD1E
// IDA_NAME: _ZN16village_attacked15CRevengeDungeon18OpenRevengeDungeonEv
// DEMANGLED: village_attacked::CRevengeDungeon::OpenRevengeDungeon(void)
CRevengeDungeon* CRevengeDungeon::OpenRevengeDungeon() {
    is_open_ = 1;
    return this;
}

// IDA_EA: 0x847DD2A
// IDA_NAME: _ZN16village_attacked15CRevengeDungeon19CloseRevengeDungeonEv
// DEMANGLED: village_attacked::CRevengeDungeon::CloseRevengeDungeon(void)
CRevengeDungeon* CRevengeDungeon::CloseRevengeDungeon() {
    is_open_ = 0;
    return this;
}

// IDA_EA: 0x847DD36
// IDA_NAME: _ZN16village_attacked15CRevengeDungeon12SetCloseTimeEj
// DEMANGLED: village_attacked::CRevengeDungeon::SetCloseTime(unsigned int)
CRevengeDungeon* CRevengeDungeon::SetCloseTime(uint32_t close_time_value) {
    close_time_ = close_time_value;
    return this;
}

// IDA_EA: 0x85BFF32
// IDA_NAME: _ZN16village_attacked15CRevengeDungeon12GetCloseTimeEv
// DEMANGLED: village_attacked::CRevengeDungeon::GetCloseTime(void)
uint32_t CRevengeDungeon::GetCloseTime() {
    return close_time_;
}

}  // namespace village_attacked
