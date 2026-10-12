#include "old_game/player/core/cuser.h"

#include "common/manager/global_data.h"
#include "common/manager/statistic_proxy_abi.h"
#include "old_game/activity/schedule/event_packets.h"
#include "old_game/economy/inventory/inventory.h"

// IDA_EA: 0x868A782
// IDA_NAME: _ZN5CUser32SendEventJarOutputItemStatisticsERKSt6vectorI10Inven_ItemSaIS1_EE
// DEMANGLED: CUser::SendEventJarOutputItemStatistics(std::vector<Inven_Item,std::allocator<Inven_Item>> const&)
unsigned char CUser::SendEventJarOutputItemStatistics(
    const std::vector<Inven_Item>& output_items) {
    Packet_Event_UpgradeJar_Output_Statistic_GTS packet;
    if (output_items.size() != 2u) {
        return 1;
    }

    unsigned int event_item_position = 0;
    if (output_items[0].item_id_ != 2670203) {
        if (output_items[1].item_id_ != 2670203) {
            return 0;
        }
        event_item_position = 1;
    }

    packet.dword_0a = output_items[event_item_position].add_info_;
    packet.dword_0e =
        output_items[1u - event_item_position].item_id_;
    return static_cast<unsigned char>(
        GlobalData::s_statistic_proxy_mgr
            ->GetServerProxy(static_cast<ENUM_SERVER_GROUP>(0))
            ->SendPacket(reinterpret_cast<char*>(&packet), 0x12));
}
