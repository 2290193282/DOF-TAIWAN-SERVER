#include "old_game/player/core/cuser.h"

#include "old_game/activity/reward/event_base.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x868AAEA
// IDA_NAME: _ZN5CUser24SendOneADayItemShopIndexERKSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: CUser::SendOneADayItemShopIndex(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&)
void CUser::SendOneADayItemShopIndex(
    const std::vector<std::pair<int, int> >& selected_items) {
    CEventBase* const event = GlobalData::s_event_manager->GetRepeatEvent(109);
    if (event->IsEventing(0) != 1) {
        return;
    }

    PacketGuard packet;
    packet.put_header(0, 341);
    const int selected_count = static_cast<int>(selected_items.size());
    packet.put_byte(selected_count);
    for (int index = 0; index < selected_count; ++index) {
        packet.put_int(selected_items[index].first);
        packet.put_int(selected_items[index].second);
    }
    packet.finalize(true);
    Send(packet);
}
