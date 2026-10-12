#include "old_game/appearance/avatar/arad_avatar_runtime.h"

#include "common/manager/global_data.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/statistic_proxy_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/core/db_types.h"
#include "old_game/activity/schedule/event_packets.h"

// IDA_EA: 0x81931DC
// IDA_NAME: _ZN19AvatarConvertServer22StatisticsEmblemCreateERKSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: AvatarConvertServer::StatisticsEmblemCreate(std::vector<std::pair<int,int>> const&)
char AvatarConvertServer::StatisticsEmblemCreate(
    const std::vector<std::pair<int, int> >& items) {
    if (items.size() > 5u) {
        return 0;
    }

    Packet_Emblem_Create_Statistic packet;
    int emitted = 0;
    for (std::vector<std::pair<int, int> >::const_iterator it = items.begin();
         it != items.end();
         ++it) {
        CItem* const item = G_CDataManager()->find_item(it->first);
        if (!item) {
            return 0;
        }
        if (item->vtable_->get_item_type(item) == 20) {
            packet.emblem_grades_0e[emitted] = item->get_grade();
            packet.emblem_counts_22[emitted] = it->second;
            if (++emitted > 5) {
                break;
            }
        }
    }

    if (emitted != static_cast<int>(items.size())) {
        return 0;
    }
    packet.emblem_count_0a = emitted;
    return static_cast<char>(
        GlobalData::s_statistic_proxy_mgr
            ->GetServerProxy(SERVER_GROUP_DEFAULT)
            ->SendPacket(reinterpret_cast<char*>(&packet), sizeof(packet)));
}
