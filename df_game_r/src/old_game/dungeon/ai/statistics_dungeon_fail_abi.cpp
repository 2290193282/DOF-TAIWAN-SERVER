#include "common/manager/statistics_abi.h"
#include "common/manager/statistics_shusia_service_abi.h"

#include "old_game/pvp/battlefield/statistics_pvp_runtime.h"
#include "old_game/world/region/game_world.h"

// IDA_EA: 0x860B8B4
// IDA_NAME: _ZN10Statistics11DungeonFailEi
// DEMANGLED: Statistics::DungeonFail(int)
int Statistics::DungeonFail(int character_job) {
    StatisticsShusiaService* const service =
        G_GameWorld()->getStatisticsShusiaService();
    return service->incDungeonFailure(static_cast<char>(character_job));
}

// IDA_EA: 0x860B90A
// IDA_NAME: _ZN10Statistics16UseShusiaServiceEi
// DEMANGLED: Statistics::UseShusiaService(int)
int Statistics::UseShusiaService(int character_level) {
    StatisticsShusiaService* const service =
        G_GameWorld()->getStatisticsShusiaService();
    return service->incServiceUsage(static_cast<char>(character_level));
}

// IDA_EA: 0x860B8D8
// IDA_NAME: _ZN10Statistics8PVPPlay2Eiii
// DEMANGLED: Statistics::PVPPlay2(int,int,int)
int Statistics::PVPPlay2(int room_number,
                         int play_type,
                         int participant_count) {
    return G_GameWorld()->getStatisticsPvP()->incPvPRoomResult(
        room_number,
        play_type,
        participant_count);
}
