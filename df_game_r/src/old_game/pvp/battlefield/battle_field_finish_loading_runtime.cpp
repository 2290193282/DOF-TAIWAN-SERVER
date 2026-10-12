#include "old_game/pvp/battlefield/battle_field.h"

#include "common/manager/global_data.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x830DC74
// IDA_NAME: _ZN13CBattle_Field10onStartMapEv
// DEMANGLED: CBattle_Field::onStartMap(void)
int CBattle_Field::onStartMap() {
    MapInfo* const map_info = GetCurrentMapInfo();
    const int tick = GlobalData::s_systemTime_.getCurTickCount();
    map_info->play_info_.start_tick = static_cast<uint32_t>(tick);
    return tick;
}
