#include "old_game/appearance/pet/creature.h"

#include "common/config.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"

namespace user_creature {

// IDA_EA: 0x833BA30
// IDA_NAME: _ZN13user_creature12CCreatureMgr18IsSaveCreatureItemEv
// DEMANGLED: user_creature::CCreatureMgr::IsSaveCreatureItem(void)
int CCreatureMgr::IsSaveCreatureItem() {
    if (creature_items_.size() == 0u) {
        return 0;
    }

    for (CCreatureItemMap::iterator it = creature_items_.begin();
         it != creature_items_.end(); ++it) {
        CCreatureItem* const item = it->second;
        if (item != 0 && item->IsSave() != 0) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8339B06
// IDA_NAME: _ZN13user_creature12CCreatureMgr7GainExpEi
// DEMANGLED: user_creature::CCreatureMgr::GainExp(int)
int CCreatureMgr::GainExp(int item_id) {
    CCreature* const creature = active_creature_;
    CUser* const user = owner_;
    if (!creature || !creature->GainExp(item_id, user)) {
        return 0;
    }
    if (creature->IsGrowCreature() != 0) {
        save_state_ = static_cast<unsigned char>(creature->isMaxLevel());
    }
    return 1;
}

// IDA_EA: 0x8339DBA
// IDA_NAME: _ZN13user_creature12CCreatureMgr7UseFeedEv
// DEMANGLED: user_creature::CCreatureMgr::UseFeed(void)
int CCreatureMgr::UseFeed() {
    CCreature* const creature = active_creature_;
    if (!creature) {
        return 0;
    }

    CUser* const user = owner_;
    int stomach = 0;
    if (!creature->UseFeed(user, stomach)) {
        return 0;
    }

    CDBMsgSender sender(static_cast<ENUM_INTERNALPACKET>(138), user);
    const int creature_uid =
        reinterpret_cast<CCreatureItem*>(creature)->GetUid();
    int result = 0;
    if (sender.UseFeedCreature(creature_uid, stomach) == 1) {
        sender.Send();
        result = 1;
    } else {
        cMyTrace trace("bool user_creature::CCreatureMgr::UseFeed()", 2741, 5);
        trace("cannot write db uid(%d)\n", user->get_unique_id());
    }
    return result;
}

// IDA_EA: 0x833BE56
// IDA_NAME: _ZN13user_creature12CCreatureMgr11GetArtifactEi
// DEMANGLED: user_creature::CCreatureMgr::GetArtifact(int)
CArtifact* CCreatureMgr::GetArtifact(int slot) {
    if (slot >= oldsrv::config::kEquipmentTypeArtifactRed &&
        slot <= oldsrv::config::kEquipmentTypeArtifactGreen) {
        return &artifacts_[
            slot - oldsrv::config::kEquipmentTypeArtifactRed];
    }
    return 0;
}

// IDA_EA: 0x833A5BC
// IDA_NAME: _ZN13user_creature12CCreatureMgr7UseItemEP10Inven_Itemi
// DEMANGLED: user_creature::CCreatureMgr::UseItem(Inven_Item *,int)
int CCreatureMgr::UseItem(Inven_Item* item, int) {
    const int item_id = item->item_id_;
    if (item_id == 24 && UseFeed() != 1) {
        return 133;
    }

    CCreature* const creature = active_creature_;
    if (creature && creature->vaildItemExp(item_id) != 0 &&
        GainExp(item_id) != 1) {
        return 17;
    }
    return 0;
}

}  // namespace user_creature
