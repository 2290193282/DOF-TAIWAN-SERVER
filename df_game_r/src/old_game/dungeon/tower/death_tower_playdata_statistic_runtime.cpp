#include "old_game/dungeon/tower/death_tower_playdata_statistic_runtime.h"

#include "common/manager/global_data.h"
#include "common/manager/statistic_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/db/dungeon/db_get_death_tower_best_record.h"
#include "old_game/dungeon/instance/dungeon_pvp_packets.h"

namespace {

const char kDeathTowerPlaydataStatisticFunction[] =
    "bool CDeathTowerPlayDataStatistic::SendDeathTowerPlayInfo("
    "int, char, const stDeathTowerRecordMemberInfo_t*, unsigned int, "
    "unsigned int)";
const char kDeathTowerPlaydataStatisticError[] =
    "CDeathTowerPlayDataStatistic ERROR!! deathTower_type (%d) level (%d) "
    "job_ (%d) grow_type_ (%d) clearStage (%d)";

}  // namespace

// IDA_EA: 0x860F451
// IDA_NAME: _Z38GetInstanceDeathTowerPlaydataStatisticv
// DEMANGLED: GetInstanceDeathTowerPlaydataStatistic(void)
CDeathTowerPlayDataStatistic* GetInstanceDeathTowerPlaydataStatistic() {
    static CDeathTowerPlayDataStatistic instance;
    return &instance;
}

// IDA_EA: 0x860F45C
// IDA_NAME: _ZN28CDeathTowerPlayDataStatistic22SendDeathTowerPlayInfoEicPK30stDeathTowerRecordMemberInfo_tjj
// DEMANGLED: CDeathTowerPlayDataStatistic::SendDeathTowerPlayInfo(int,char,stDeathTowerRecordMemberInfo_t const*,unsigned int,unsigned int)
bool CDeathTowerPlayDataStatistic::SendDeathTowerPlayInfo(
    int member_index,
    char death_tower_type,
    const stDeathTowerRecordMemberInfo_t* start_member_info,
    uint32_t clear_stage,
    uint32_t start_member_count) {
    const stDeathTowerRecordMemberInfo_t& member =
        start_member_info[member_index];

    Packet_DeathTower_Statistic_Playdata_Job job_packet;
    job_packet.death_tower_type = static_cast<uint8_t>(death_tower_type);
    job_packet.character_level = member.level;
    job_packet.character_job = member.job;
    job_packet.grow_type = member.grow_type;
    job_packet.clear_stage = clear_stage;

    if (job_packet.character_level != 0u) {
        GlobalData::s_statistic_proxy_mgr
            ->GetServerProxy(SERVER_GROUP_DEFAULT)
            ->SendPacket(reinterpret_cast<char*>(&job_packet),
                         sizeof(job_packet));
    } else {
        LogManager::logFormat(
            1,
            "Statistics.cpp",
            kDeathTowerPlaydataStatisticFunction,
            0x5FE,
            kDeathTowerPlaydataStatisticError,
            static_cast<int>(job_packet.death_tower_type),
            static_cast<int>(job_packet.character_level),
            static_cast<int>(job_packet.character_job),
            static_cast<int>(job_packet.grow_type),
            static_cast<int>(job_packet.clear_stage));
    }

    Packet_DeathTower_Statistic_Playdata_Party party_packet;
    party_packet.death_tower_type = static_cast<uint8_t>(death_tower_type);
    party_packet.start_member_count =
        static_cast<uint8_t>(start_member_count);
    party_packet.clear_stage = clear_stage;
    GlobalData::s_statistic_proxy_mgr
        ->GetServerProxy(SERVER_GROUP_DEFAULT)
        ->SendPacket(reinterpret_cast<char*>(&party_packet),
                     sizeof(party_packet));
    return true;
}
