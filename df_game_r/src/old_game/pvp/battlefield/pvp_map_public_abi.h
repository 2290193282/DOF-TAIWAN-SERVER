#ifndef OLDSRV_GLOBAL_PVP_MAP_PUBLIC_ABI_H_
#define OLDSRV_GLOBAL_PVP_MAP_PUBLIC_ABI_H_

#include <stdint.h>


// IDA unit: 05_world_enter_village/CMapList.
// IDA_EA: CMapList::LoadPvPSinglePlayableMap @0x834F4BA.
// The old class-scoped symbol consumes the caller-provided filename as its
// first and only effective stack argument. C/05 owns the map-runtime body; A
// exposes only the old-symbol boundary consumed by the CDataManager/PVF init
// path after creature script loading.
// IDA unit: 99_misc_global/Global PVP map basic-data arrays.
// IDA_EA: loadPvpMapBasicDataScript @0x890A427 writes these arrays from
// Etc/pvpmapbasicdata.etc; CDataManager::init consumes the result at
// @0x835BB09..0x835BB85 before the CMapList/load_map_files branch.
extern int32_t PVPMAP_BIG_IMAGE[20] __asm__("PVPMAP_BIG_IMAGE");
extern int32_t PVPMAP_SMALL_IMAGE[20] __asm__("PVPMAP_SMALL_IMAGE");
extern uint8_t PVPMAP_DEATHMATCH_MODE[20] __asm__("PVPMAP_DEATHMATCH_MODE");
extern uint8_t PVPMAP_ONLY_TOURNAMENT_EXPOSURE[20]
    __asm__("PVPMAP_ONLY_TOURNAMENT_EXPOSURE");
extern uint8_t PVPMAP_ONLY_NORMAL_EXPOSURE[20]
    __asm__("PVPMAP_ONLY_NORMAL_EXPOSURE");
extern uint8_t PVPMAP_ONLY_TOURNAMENT_RANDOM[20]
    __asm__("PVPMAP_ONLY_TOURNAMENT_RANDOM");


#endif  // OLDSRV_GLOBAL_PVP_MAP_PUBLIC_ABI_H_
