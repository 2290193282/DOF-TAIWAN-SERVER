#include "old_game/social/party/hell_party_item_statistic_runtime.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "common/manager/statistic_proxy_abi.h"
#include "old_game/dungeon/instance/dungeon_pvp_packets.h"

// IDA_EA: 0x860FCC0
// IDA_NAME: _ZN23CHellPartyItemStatistic26SendHellPartyItemStatisticEbiiicPi
// DEMANGLED: CHellPartyItemStatistic::SendHellPartyItemStatistic(
//     bool,int,int,int,char,int *)
int CHellPartyItemStatistic::SendHellPartyItemStatistic(
    bool active_flag,
    int dungeon_index,
    int dungeon_difficulty,
    int kill_data_value,
    char statistics_enabled,
    int* item_rarity_counts) {
    if (dungeon_difficulty > 4) {
        return 0;
    }
    if (statistics_enabled == 0) {
        return 0;
    }

    Packet_HellParty_Statistic_Item packet;
    packet.active_flag = static_cast<uint8_t>(active_flag);
    packet.dungeon_index = dungeon_index;
    packet.dungeon_difficulty = static_cast<uint8_t>(dungeon_difficulty);
    packet.kill_data_byte = static_cast<uint8_t>(kill_data_value);
    packet.statistics_enabled = static_cast<uint8_t>(statistics_enabled);
    std::memcpy(packet.item_rarity_counts,
                item_rarity_counts,
                sizeof(packet.item_rarity_counts));

    GlobalData::s_statistic_proxy_mgr
        ->GetServerProxy(SERVER_GROUP_DEFAULT)
        ->SendPacket(reinterpret_cast<char*>(&packet), sizeof(packet));
    return 1;
}

// IDA_EA: 0x860FCB6
// IDA_NAME: _Z33GetInstanceHellPartyItemStatisticv
// DEMANGLED: GetInstanceHellPartyItemStatistic(void)
CHellPartyItemStatistic* GetInstanceHellPartyItemStatistic() {
    static CHellPartyItemStatistic instance;
    return &instance;
}
