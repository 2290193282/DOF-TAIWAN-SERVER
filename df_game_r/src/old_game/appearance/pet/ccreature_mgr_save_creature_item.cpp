#include "old_game/appearance/pet/creature.h"

#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"

#include <cstring>

namespace user_creature {

// IDA_EA: 0x833BBBE
// IDA_NAME: _ZN13user_creature12CCreatureMgr16SaveCreatureItemEv
// DEMANGLED: user_creature::CCreatureMgr::SaveCreatureItem(void)
int CCreatureMgr::SaveCreatureItem() {
    CDBMsgSender sender(
        static_cast<ENUM_INTERNALPACKET>(137),
        owner_);
    SIG_CREATURE_ITEM_LIST* const payload =
        sender.GetInBuffer<SIG_CREATURE_ITEM_LIST>();
    if (!payload) {
        cMyTrace trace(
            "bool user_creature::CCreatureMgr::SaveCreatureItem()", 3472, 5);
        trace("cannot write db uid(%d)\\n", owner_->get_unique_id());
        return 0;
    }

    std::memset(payload, 0, sizeof(*payload));
    for (CCreatureItemMap::iterator it = creature_items_.begin();
         it != creature_items_.end();
        ++it) {
        CCreatureItem* const item = it->second;
        if (item->IsSave() != 0) {
            const unsigned int index = payload->count;
            item->SaveCreatureItem(&payload->inventory_items[index],
                                   &payload->details[index],
                                   owner_);
            if (payload->details[index].ui_id != 0) {
                ++payload->count;
            }
        }
    }

    if (payload->count != 0u) {
        sender.Send();
    } else {
        sender.Cancel();
    }
    return 1;
}

}  // namespace user_creature
