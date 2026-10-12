#ifndef OLDSRV_09_DUNGEON_PVP_PARTY_DEATH_TOWER_VALUE_STATISTIC_RUNTIME_H_
#define OLDSRV_09_DUNGEON_PVP_PARTY_DEATH_TOWER_VALUE_STATISTIC_RUNTIME_H_

#include <stdint.h>

enum DEATHTOWER_VALUE_STATISTIC_FIELD : int;

class CDeathTowerValueStatistic {
public:
    // IDA_EA: 0x860F342
    // IDA_NAME: _ZN25CDeathTowerValueStatistic28SendDeathTowerValueStatisticE32DEATHTOWER_VALUE_STATISTIC_FIELDccj
    // DEMANGLED: CDeathTowerValueStatistic::SendDeathTowerValueStatistic(DEATHTOWER_VALUE_STATISTIC_FIELD,char,char,unsigned int)
    bool SendDeathTowerValueStatistic(
        DEATHTOWER_VALUE_STATISTIC_FIELD field,
        char non_tutorial_dungeon,
        char charac_level,
        uint32_t value);
};

// IDA_EA: 0x860F337
// IDA_NAME: _Z35GetInstanceDeathTowerValueStatisticv
// DEMANGLED: GetInstanceDeathTowerValueStatistic(void)
CDeathTowerValueStatistic* GetInstanceDeathTowerValueStatistic();

#endif  // OLDSRV_09_DUNGEON_PVP_PARTY_DEATH_TOWER_VALUE_STATISTIC_RUNTIME_H_
