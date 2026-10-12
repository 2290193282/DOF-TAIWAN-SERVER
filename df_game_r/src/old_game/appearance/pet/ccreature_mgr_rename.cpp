#include "old_game/appearance/pet/creature.h"

#include <cstring>

#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"

namespace user_creature {

// IDA_EA: 0x8339B6E
// IDA_NAME: _ZN13user_creature12CCreatureMgr14RenameCreatureEPKcii
// DEMANGLED: user_creature::CCreatureMgr::RenameCreature(char const*,int,int)
int CCreatureMgr::RenameCreature(const char* name, int slot, int rename_mode) {
    CPacketRespondent response(owner_, 1, ENUM_CMDPACKET_RENAME_CREATURE);
    if (!active_creature_) {
        response.SetError(4);
        return 0;
    }

    CInventory* const inventory = owner_->getCurCharacInvenW();
    const int use_result = inventory->use_item(slot, 1, rename_mode);
    if (use_result != 0) {
        response.SetError(use_result);
        return 0;
    }

    active_creature_->RenameCreature(name);
    response.MakeNotipacketRenameCreature(
        name, static_cast<int>(std::strlen(name)));
    response.Broadcast(owner_->getCurCharacVill(), owner_->get_area(false));

    CDBMsgSender sender(static_cast<ENUM_INTERNALPACKET>(138), owner_);
    if (sender.RenameCreatureItem(active_creature_->GetUid(), name) == 1) {
        sender.Send();
        response.MakeCmdpacketRenameCreature(slot, rename_mode);
        return 1;
    }

    cMyTrace trace(
        "bool user_creature::CCreatureMgr::RenameCreature(const char*, int, int)",
        2720,
        5);
    trace("cannot write db uid(%d)\n", owner_->get_unique_id());
    response.SetError(2);
    return 0;
}

}  // namespace user_creature
