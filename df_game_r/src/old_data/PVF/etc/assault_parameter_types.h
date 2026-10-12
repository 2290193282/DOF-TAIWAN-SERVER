#ifndef OLDSRV_OLD_DATA_PVF_ETC_ASSAULT_PARAMETER_TYPES_H_
#define OLDSRV_OLD_DATA_PVF_ETC_ASSAULT_PARAMETER_TYPES_H_

#include <stddef.h>


struct stAssaultChaosPointWeight {
    stAssaultChaosPointWeight();
    ~stAssaultChaosPointWeight();

    int min;
    int max;
    float from_weight;
    float to_weight;
};

struct stAssaultLevelGhostPanalty {
    stAssaultLevelGhostPanalty();
    ~stAssaultLevelGhostPanalty();

    int min_level;
    int max_level;
    int min_time;
    int max_time;
};

struct stAssaultChaosPointGhostPanalty {
    stAssaultChaosPointGhostPanalty();
    ~stAssaultChaosPointGhostPanalty();

    int min;
    int max;
    float from_weight;
    float to_weight;
};

static_assert(sizeof(stAssaultChaosPointWeight) == 0x10u,
              "stAssaultChaosPointWeight IDA ABI size must stay 0x10");
static_assert(offsetof(stAssaultChaosPointWeight, min) == 0x00u,
              "stAssaultChaosPointWeight::min must stay at +0x00");
static_assert(offsetof(stAssaultChaosPointWeight, max) == 0x04u,
              "stAssaultChaosPointWeight::max must stay at +0x04");
static_assert(offsetof(stAssaultChaosPointWeight, from_weight) == 0x08u,
              "stAssaultChaosPointWeight::from_weight must stay at +0x08");
static_assert(offsetof(stAssaultChaosPointWeight, to_weight) == 0x0Cu,
              "stAssaultChaosPointWeight::to_weight must stay at +0x0c");

static_assert(sizeof(stAssaultLevelGhostPanalty) == 0x10u,
              "stAssaultLevelGhostPanalty IDA ABI size must stay 0x10");
static_assert(offsetof(stAssaultLevelGhostPanalty, min_level) == 0x00u,
              "stAssaultLevelGhostPanalty::min_level must stay at +0x00");
static_assert(offsetof(stAssaultLevelGhostPanalty, max_level) == 0x04u,
              "stAssaultLevelGhostPanalty::max_level must stay at +0x04");
static_assert(offsetof(stAssaultLevelGhostPanalty, min_time) == 0x08u,
              "stAssaultLevelGhostPanalty::min_time must stay at +0x08");
static_assert(offsetof(stAssaultLevelGhostPanalty, max_time) == 0x0Cu,
              "stAssaultLevelGhostPanalty::max_time must stay at +0x0c");

static_assert(sizeof(stAssaultChaosPointGhostPanalty) == 0x10u,
              "stAssaultChaosPointGhostPanalty IDA ABI size must stay 0x10");
static_assert(offsetof(stAssaultChaosPointGhostPanalty, min) == 0x00u,
              "stAssaultChaosPointGhostPanalty::min must stay at +0x00");
static_assert(offsetof(stAssaultChaosPointGhostPanalty, max) == 0x04u,
              "stAssaultChaosPointGhostPanalty::max must stay at +0x04");
static_assert(offsetof(stAssaultChaosPointGhostPanalty, from_weight) == 0x08u,
              "stAssaultChaosPointGhostPanalty::from_weight must stay at +0x08");
static_assert(offsetof(stAssaultChaosPointGhostPanalty, to_weight) == 0x0Cu,
              "stAssaultChaosPointGhostPanalty::to_weight must stay at +0x0c");

#endif  // OLDSRV_OLD_DATA_PVF_ETC_ASSAULT_PARAMETER_TYPES_H_
