#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"

#include <utility>
#include <vector>

#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/inventory.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x86475D8
// IDA_NAME: _ZN5CUser14RewardTutorialEj
// DEMANGLED: CUser::RewardTutorial(unsigned int)
void CUser::RewardTutorial(unsigned int tutorial_id) {
    std::vector<int> inserted_slots;
    std::vector<std::pair<int, int> >* const rewards =
        G_CDataManager()->get_server_parameter_script()
            ->GetEscaladeTutorialReward(tutorial_id);
    if (!rewards) {
        return;
    }

    // The original call passes the shared empty-string object for both
    // inventory messages.  The explicit empty literal preserves that data
    // contract without introducing a synthetic message identity.
    const int inserted = getCurCharacInvenW()->insert_event_items(
        *rewards,
        inserted_slots,
        static_cast<eMoneyAddReason>(3),
        static_cast<eItemAddReason>(8),
        true,
        "",
        "");

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_CHANGE_TUTORIAL_FLAG);
    packet.put_byte(1);
    packet.put_byte(inserted);
    if (inserted > 0) {
        std::vector<int>::const_iterator it = inserted_slots.begin();
        const std::vector<int>::const_iterator end = inserted_slots.end();
        while (it != end) {
            packet.put_short(*it++);
            packet.put_int(*it++);
            packet.put_int(*it++);
        }
    }
    packet.finalize(true);
    Send(packet);
}
