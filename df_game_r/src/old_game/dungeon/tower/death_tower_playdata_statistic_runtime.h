#ifndef OLDSRV_09_DUNGEON_PVP_PARTY_DEATH_TOWER_PLAYDATA_STATISTIC_RUNTIME_H_
#define OLDSRV_09_DUNGEON_PVP_PARTY_DEATH_TOWER_PLAYDATA_STATISTIC_RUNTIME_H_

#include <stdint.h>

struct stDeathTowerRecordMemberInfo_t;

class CDeathTowerPlayDataStatistic {
public:
    // IDA_EA: 0x860F45C
    // IDA_NAME: _ZN28CDeathTowerPlayDataStatistic22SendDeathTowerPlayInfoEicPK30stDeathTowerRecordMemberInfo_tjj
    // DEMANGLED: CDeathTowerPlayDataStatistic::SendDeathTowerPlayInfo(int,char,stDeathTowerRecordMemberInfo_t const*,unsigned int,unsigned int)
    bool SendDeathTowerPlayInfo(
        int member_index,
        char death_tower_type,
        const stDeathTowerRecordMemberInfo_t* start_member_info,
        uint32_t clear_stage,
        uint32_t start_member_count);
};

// IDA_EA: 0x860F451
// IDA_NAME: _Z38GetInstanceDeathTowerPlaydataStatisticv
// DEMANGLED: GetInstanceDeathTowerPlaydataStatistic(void)
CDeathTowerPlayDataStatistic* GetInstanceDeathTowerPlaydataStatistic();

#endif  // OLDSRV_09_DUNGEON_PVP_PARTY_DEATH_TOWER_PLAYDATA_STATISTIC_RUNTIME_H_
