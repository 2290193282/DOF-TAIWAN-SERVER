#include "old_game/dungeon/tower/death_tower_value_statistic_runtime.h"


// IDA_EA: 0x860F337
// IDA_NAME: _Z35GetInstanceDeathTowerValueStatisticv
// DEMANGLED: GetInstanceDeathTowerValueStatistic(void)
CDeathTowerValueStatistic* GetInstanceDeathTowerValueStatistic() {
    static CDeathTowerValueStatistic instance;
    return &instance;
}
