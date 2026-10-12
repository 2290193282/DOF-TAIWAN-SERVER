#include "old_data/PVF/map/datamanager/map_runtime.h"

#include <cstring>

// IDA_EA: 0x834F5C8
// IDA_NAME: _ZN8CMapList13GetPvpMapInfoEiR10PvPMapInfo
// DEMANGLED: CMapList::GetPvpMapInfo(int,PvPMapInfo &)
int CMapList::GetPvpMapInfo(int map_no, PvPMapInfo& info) {
    std::map<int, PvPMapInfo>::const_iterator found =
        mapPvpMapInfo_.find(map_no);
    if (found == mapPvpMapInfo_.end()) {
        return 0;
    }

    const PvPMapInfo* const source = &found->second;
    if (&info != source) {
        std::memcpy(&info, source, 5u);
    }
    return 1;
}
