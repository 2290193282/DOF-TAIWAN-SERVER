#include "old_game/appearance/pet/creature.h"

#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"

namespace user_creature {

// IDA_EA: 0x833C00C
// IDA_NAME: _ZN13user_creature12CCreatureMgr16SetKeepRightItemEP5CUserP10Inven_Item
// DEMANGLED: user_creature::CCreatureMgr::SetKeepRightItem(CUser *,Inven_Item *)
bool CCreatureMgr::SetKeepRightItem(CUser* user, Inven_Item* item) {
    for (CCreatureItemMap::iterator it = creature_items_.begin();
         it != creature_items_.end(); ++it) {
        CCreatureItem* const creature_item = it->second;
        if (!creature_item ||
            creature_item->GetCreatureItemType() !=
                CREATURE_ITME_TYPE_CREATURE) {
            continue;
        }

        CCreature* const creature = static_cast<CCreature*>(creature_item);
        if (!creature->IsNoCharge()) {
            continue;
        }

        creature->SetNoCharge(false);
        CDBMsgSender sender(static_cast<ENUM_INTERNALPACKET>(138), user);
        const int no_charge = creature->IsNoCharge();
        const int creature_uid = creature_item->GetUid();
        if (sender.UseKeepRight(creature_uid, no_charge) != 1) {
            const unsigned short unique_id =
                static_cast<unsigned short>(user->get_unique_id());
            cMyTrace trace(
                "bool user_creature::CCreatureMgr::SetKeepRightItem(CUser*, Inven_Item*)",
                3568,
                5);
            trace("cannot write db uid(%d)\n", unique_id);
            return false;
        }

        sender.Send();
        CPacketRespondent respondent(owner_, 0, 0);
        respondent.MakeNotipacketCreatureItemList(&creature_items_,
                                                  creature_item->GetSlotNo());
    }

    item->reset();
    return true;
}

// IDA_EA: 0x833C240
// IDA_NAME: _ZN13user_creature12CCreatureMgr9CheckItemEP5CUserP10Inven_ItemRb
// DEMANGLED: user_creature::CCreatureMgr::CheckItem(CUser *,Inven_Item *,bool &)
bool CCreatureMgr::CheckItem(CUser* user, Inven_Item* item, bool& valid) {
    valid = true;
    if (item->item_category_ != 7 || item->item_id_ != 26) {
        return true;
    }

    valid = false;
    return SetKeepRightItem(user, item);
}

}  // namespace user_creature
