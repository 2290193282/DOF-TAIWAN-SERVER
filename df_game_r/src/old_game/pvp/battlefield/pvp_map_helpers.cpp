#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_game/pvp/battlefield/pvp_map_public_abi.h"


// IDA_EA: 0x85D4690
// IDA_NAME: _Z15IsDeathMatchMapiRb
// DEMANGLED: IsDeathMatchMap(int,bool &)
int IsDeathMatchMap(int map_no, bool& changed_to_individual) {
    PvPMapInfo info;
    if (!CMapList::GetPvpMapInfo(map_no, info)) {
        return 0;
    }

    changed_to_individual = info.enabled != 0;
    if (static_cast<unsigned int>(map_no) < 20u) {
        return PVPMAP_DEATHMATCH_MODE[map_no];
    }
    return 0;
}
