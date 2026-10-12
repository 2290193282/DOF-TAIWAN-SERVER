#include "old_game/pvp/battlefield/pvp_map_public_abi.h"


// IDA_EA: 0x890A427
// IDA_NAME: _Z25loadPvpMapBasicDataScriptPKcPSt6vectorI11PVPMAP_DATASaIS2_EE
// The old loader stores up to 20 [pvp map basic data] rows into these public
// globals; zero-initialized BSS preserves the pre-import default state.
int32_t PVPMAP_BIG_IMAGE[20] __asm__("PVPMAP_BIG_IMAGE");
int32_t PVPMAP_SMALL_IMAGE[20] __asm__("PVPMAP_SMALL_IMAGE");
uint8_t PVPMAP_DEATHMATCH_MODE[20] __asm__("PVPMAP_DEATHMATCH_MODE");
uint8_t PVPMAP_ONLY_TOURNAMENT_EXPOSURE[20]
    __asm__("PVPMAP_ONLY_TOURNAMENT_EXPOSURE");
uint8_t PVPMAP_ONLY_NORMAL_EXPOSURE[20]
    __asm__("PVPMAP_ONLY_NORMAL_EXPOSURE");
uint8_t PVPMAP_ONLY_TOURNAMENT_RANDOM[20]
    __asm__("PVPMAP_ONLY_TOURNAMENT_RANDOM");

typedef char PVPMAP_BIG_IMAGE_Count_Must_Be_20[
    (sizeof(PVPMAP_BIG_IMAGE) == 20u * sizeof(int32_t)) ? 1 : -1];
typedef char PVPMAP_SMALL_IMAGE_Count_Must_Be_20[
    (sizeof(PVPMAP_SMALL_IMAGE) == 20u * sizeof(int32_t)) ? 1 : -1];
typedef char PVPMAP_DEATHMATCH_MODE_Count_Must_Be_20[
    (sizeof(PVPMAP_DEATHMATCH_MODE) == 20u) ? 1 : -1];
typedef char PVPMAP_ONLY_TOURNAMENT_EXPOSURE_Count_Must_Be_20[
    (sizeof(PVPMAP_ONLY_TOURNAMENT_EXPOSURE) == 20u) ? 1 : -1];
typedef char PVPMAP_ONLY_NORMAL_EXPOSURE_Count_Must_Be_20[
    (sizeof(PVPMAP_ONLY_NORMAL_EXPOSURE) == 20u) ? 1 : -1];
typedef char PVPMAP_ONLY_TOURNAMENT_RANDOM_Count_Must_Be_20[
    (sizeof(PVPMAP_ONLY_TOURNAMENT_RANDOM) == 20u) ? 1 : -1];
