#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_REVENGE_DUNGEON_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_REVENGE_DUNGEON_RUNTIME_H_

#include <stdint.h>

// Canonical C-owned runtime state for the village-attacked revenge dungeon.
// The storage itself remains the GlobalData process global; callers use this
// real member surface instead of CRevengeDungeon_* free-function aliases.
namespace village_attacked {

class CRevengeDungeon {
public:
    // IDA_EA: 0x82AA6A4
    // IDA_NAME: _ZN16village_attacked15CRevengeDungeonC2Ev
    // DEMANGLED: village_attacked::CRevengeDungeon::CRevengeDungeon(void)
    CRevengeDungeon();

    // IDA_EA: 0x8234FF6
    // IDA_NAME: _ZN16village_attacked15CRevengeDungeon20IsOpenRevengeDungeonEv
    // DEMANGLED: village_attacked::CRevengeDungeon::IsOpenRevengeDungeon(void)
    int IsOpenRevengeDungeon();

    // IDA_EA: 0x847DD1E
    // IDA_NAME: _ZN16village_attacked15CRevengeDungeon18OpenRevengeDungeonEv
    // DEMANGLED: village_attacked::CRevengeDungeon::OpenRevengeDungeon(void)
    CRevengeDungeon* OpenRevengeDungeon();

    // IDA_EA: 0x847DD2A
    // IDA_NAME: _ZN16village_attacked15CRevengeDungeon19CloseRevengeDungeonEv
    // DEMANGLED: village_attacked::CRevengeDungeon::CloseRevengeDungeon(void)
    CRevengeDungeon* CloseRevengeDungeon();

    // IDA_EA: 0x847DD36
    // IDA_NAME: _ZN16village_attacked15CRevengeDungeon12SetCloseTimeEj
    // DEMANGLED: village_attacked::CRevengeDungeon::SetCloseTime(unsigned int)
    CRevengeDungeon* SetCloseTime(uint32_t close_time_value);

    // IDA_EA: 0x85BFF32
    // IDA_NAME: _ZN16village_attacked15CRevengeDungeon12GetCloseTimeEv
    // DEMANGLED: village_attacked::CRevengeDungeon::GetCloseTime(void)
    uint32_t GetCloseTime();

private:
    uint8_t is_open_;        // IDA +0x00
    uint8_t raw_01_[0x03];   // IDA +0x01
    uint32_t close_time_;    // IDA +0x04
};

static_assert(sizeof(CRevengeDungeon) == 0x08u,
              "CRevengeDungeon IDA ABI size must stay 0x08");

}  // namespace village_attacked

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_REVENGE_DUNGEON_RUNTIME_H_
