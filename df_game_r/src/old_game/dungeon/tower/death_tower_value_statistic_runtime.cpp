#include "old_game/dungeon/tower/death_tower_value_statistic_runtime.h"

#include "common/manager/global_data.h"
#include "common/manager/statistic_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/dungeon/instance/dungeon_pvp_packets.h"

namespace {

// df_game_r .rodata @0x08CE0390. The source byte sequence is retained rather
// than transcoded because the original executable stores this message in CP949.
const char kInvalidDeathTowerValueStatisticName[] =
    "name(%d), "
    "\xC1\xB8\xC0\xE7\xC7\xCF\xC1\xF6 "
    "\xBE\xCA\xB4\xC2 "
    "\xBE\xC6\xC0\xCC\xC5\xDB "
    "\xB5\xEE\xB1\xDE";

}  // namespace

// IDA_EA: 0x860F342
// IDA_NAME: _ZN25CDeathTowerValueStatistic28SendDeathTowerValueStatisticE32DEATHTOWER_VALUE_STATISTIC_FIELDccj
// DEMANGLED: CDeathTowerValueStatistic::SendDeathTowerValueStatistic(DEATHTOWER_VALUE_STATISTIC_FIELD,char,char,unsigned int)
bool CDeathTowerValueStatistic::SendDeathTowerValueStatistic(
    DEATHTOWER_VALUE_STATISTIC_FIELD field,
    char non_tutorial_dungeon,
    char charac_level,
    uint32_t value) {
    Packet_DeathTower_Statistic_Value packet;
    packet.non_tutorial_dungeon =
        static_cast<uint8_t>(non_tutorial_dungeon);
    packet.charac_level = static_cast<uint16_t>(charac_level);

    uint32_t statistic_field = static_cast<uint32_t>(field);
    uint32_t statistic_value = value;
    if (statistic_field == 10u) {
        switch (value) {
        case 0u:
            statistic_field = 3u;
            statistic_value = 1u;
            break;
        case 1u:
            statistic_field = 4u;
            statistic_value = 1u;
            break;
        case 2u:
            statistic_field = 5u;
            statistic_value = 1u;
            break;
        case 3u:
            statistic_field = 6u;
            statistic_value = 1u;
            break;
        default:
            LogManager::logFormat(
                1,
                "Statistics.cpp",
                "bool CDeathTowerValueStatistic::SendDeathTowerValueStatistic(DEATHTOWER_VALUE_STATISTIC_FIELD, char, char, unsigned int)",
                0x5D5,
                kInvalidDeathTowerValueStatisticName,
                static_cast<int>(field));
            return false;
        }
    }

    packet.statistic_field = statistic_field;
    packet.value = statistic_value;
    GlobalData::s_statistic_proxy_mgr
        ->GetServerProxy(SERVER_GROUP_DEFAULT)
        ->SendPacket(reinterpret_cast<char*>(&packet), sizeof(packet));
    return true;
}
