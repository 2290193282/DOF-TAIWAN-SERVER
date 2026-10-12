#include "old_game/appearance/pet/creature.h"

#include "common/config.h"
#include "old_data/PVF/creature/datamanager/creature_script.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"

namespace user_creature {

// IDA_EA: 0x8338CA0
// IDA_NAME: _ZN13user_creature9CCreature11ItemEvoluteEP5CUseri
// DEMANGLED: user_creature::CCreature::ItemEvolute(CUser *,int)
int CCreature::ItemEvolute(CUser* user, int member_count) {
    // The CCreature script pointer is the IDA-proven +0x44 member.  The
    // canonical CCreature state provider already reads and writes this exact
    // member while implementing SetItemId and the other evolution paths.
    CCreatureScript* const source_script = creature_script_;
    if (!source_script) {
        cMyTrace trace(
            "bool user_creature::CCreature::ItemEvolute(CUser*, int)",
            2203,
            5);
        trace("cannot evolute because it haven't creature script uid(%d), creature id(%d) \n",
              user->get_unique_id(),
              GetItemId());
        return 0;
    }

    const int creature_id =
        source_script->GetItemEvolutionCreatureId(member_count);
    if (creature_id == 0) {
        return 0;
    }

    CCreatureScript* const target_script =
        GetInstanceCreatureScriptMgr()->FindCreatureScript(creature_id);
    if (!target_script) {
        return 0;
    }

    SetItemId(target_script->GetItemId());
    CInventory* const inventory =
        static_cast<CInventory*>(user->getCurCharacInvenW());
    Inven_Item* const equipped = inventory->GetInvenRef(
        0, oldsrv::config::kEquipmentTypeCreature);
    if (!equipped) {
        return 0;
    }

    // The original writes the equipped creature record's physical item-id
    // member at Inven_Item +0x02.
    equipped->item_id_ = GetItemId();

    CDBMsgSender sender(static_cast<ENUM_INTERNALPACKET>(138), user);
    const int item_id = GetItemId();
    const int uid = GetUid();
    if (sender.EvoluteCreature(uid, item_id) != 1) {
        cMyTrace trace(
            "bool user_creature::CCreature::ItemEvolute(CUser*, int)",
            2235,
            5);
        trace("cannot write db uid(%d)\n", user->get_unique_id());
        return 0;
    }

    sender.Send();
    CPacketRespondent respondent(user, 0, 0);
    respondent.MakeNotipacketEvoluteCreature(creature_id);
    respondent.SendAccordingToPlace();
    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(3),
                             oldsrv::config::kEquipmentTypeCreature);
    return 1;
}

// IDA_EA: 0x833DA1E
// IDA_NAME: _ZN13user_creature12CCreatureMgr20EventEvoluteCreatureEiii
// DEMANGLED: user_creature::CCreatureMgr::EventEvoluteCreature(int,int,int)
int CCreatureMgr::EventEvoluteCreature(int item_id,
                                       int required_level,
                                       int client_creature_index) {
    if (!active_creature_) {
        return 0;
    }
    if (active_creature_->GetLevel() < required_level) {
        cMyTrace trace(
            "bool user_creature::CCreatureMgr::EventEvoluteCreature(int, int, int)",
            4290,
            0);
        trace("event evolute creature error : low creature level.");
        return 0;
    }
    if (!active_creature_->EventEvolute(owner_, item_id,
                                        client_creature_index)) {
        return 0;
    }

    CPacketRespondent respondent(owner_, 0, 0);
    respondent.MakeNotipacketCreatureItemList(
        &creature_items_,
        oldsrv::config::kCreatureInventoryEquippedCreatureSlot);
    return 1;
}

// IDA_EA: 0x833DB2E
// IDA_NAME: _ZN13user_creature9CCreature12EventEvoluteEP5CUserii
// DEMANGLED: user_creature::CCreature::EventEvolute(CUser *,int,int)
int CCreature::EventEvolute(CUser* user,
                            int item_id,
                            int client_creature_index) {
    if (!creature_script_) {
        cMyTrace trace("bool user_creature::CCreature::EventEvolute(CUser*, int, int)",
                       4300,
                       5);
        trace("cannot event evolute because it haven't creature script uid(%d), creature id(%d) \n",
              user->get_unique_id(),
              GetItemId());
        return 0;
    }

    const int creature_id = GetCreatureId(item_id);
    CCreatureScript* const target_script =
        GetInstanceCreatureScriptMgr()->FindCreatureScript(creature_id);
    if (!target_script) {
        return 0;
    }

    const int target_item_id = target_script->GetItemId();
    if (target_script->IsItemGrowCreature() != 1 && item_grow_state_ != 1) {
        int count_per_level = creature_script_->GetGrowItemCountforLevelUp();
        if (count_per_level == 0) {
            count_per_level = 25;
        }
        experience_ = G_CDataManager()->get_creature_level_exp(
            ((experience_ >> 16) / count_per_level) + 1);
    }

    SetItemId(target_item_id);
    CItem* const item = G_CDataManager()->find_item(target_item_id);
    if (!item) {
        return 0;
    }

    const int usable_period = item->getUsablePeriod();
    const int expiration_date = item->getExpirationDate();
    if (usable_period == 0) {
        SetExpireTime(expiration_date);
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    Inven_Item* const equipped = inventory->GetInvenRef(
        0, oldsrv::config::kEquipmentTypeCreature);
    if (!equipped) {
        return 0;
    }
    equipped->item_id_ = GetItemId();

    CDBMsgSender sender(static_cast<ENUM_INTERNALPACKET>(138), user);
    if (sender.EventEvoluteCreature(GetUid(), GetItemId(), experience_) != 1) {
        cMyTrace trace("bool user_creature::CCreature::EventEvolute(CUser*, int, int)",
                       4356,
                       5);
        trace("cannot write db uid(%d)\n", user->get_unique_id());
        return 0;
    }

    sender.Send();
    CPacketRespondent respondent(user, 0, 0);
    respondent.MakeNotipacketEvoluteCreature(client_creature_index);
    respondent.SendAccordingToPlace();
    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(3),
                             oldsrv::config::kEquipmentTypeCreature);
    return 1;
}

// IDA_EA: 0x833BEB4
// IDA_NAME: _ZN13user_creature12CCreatureMgr7EvoluteEiib
// DEMANGLED: user_creature::CCreatureMgr::Evolute(int,int,bool)
int CCreatureMgr::Evolute(int creature_id,
                          int required_level,
                          bool force) {
    if (!active_creature_) {
        return 0;
    }
    if (active_creature_->GetLevel() < required_level && !force) {
        cMyTrace trace("bool user_creature::CCreatureMgr::Evolute(int, int, bool)",
                       3547,
                       0);
        trace("evolute error : low creature level.");
        return 0;
    }
    if (active_creature_->IsEvolutionCreature(creature_id) || force) {
        return active_creature_->Evolute(owner_);
    }
    if (active_creature_->IsEvolutionCreatureMulty(creature_id) || force) {
        return active_creature_->EvoluteMulty(owner_, creature_id);
    }

    cMyTrace trace("bool user_creature::CCreatureMgr::Evolute(int, int, bool)",
                   3542,
                   0);
    trace("evolute error : different creature kind.");
    return 0;
}

// IDA_EA: 0x83381EA
// IDA_NAME: _ZN13user_creature9CCreature7EvoluteEP5CUser
// DEMANGLED: user_creature::CCreature::Evolute(CUser *)
int CCreature::Evolute(CUser* user) {
    if (!creature_script_) {
        cMyTrace trace("bool user_creature::CCreature::Evolute(CUser*)", 1859, 5);
        trace("cannot evolute because it haven't creature script uid(%d), creature id(%d) \n",
              user->get_unique_id(),
              GetItemId());
        return 0;
    }

    const int creature_id = creature_script_->GetEvolutionCreatureId();
    if (creature_id == 0) {
        return 0;
    }
    CCreatureScript* const target_script =
        GetInstanceCreatureScriptMgr()->FindCreatureScript(creature_id);
    if (!target_script) {
        return 0;
    }

    SetItemId(target_script->GetItemId());
    CInventory* const inventory = user->getCurCharacInvenW();
    Inven_Item* const equipped = inventory->GetInvenRef(
        0, oldsrv::config::kEquipmentTypeCreature);
    if (!equipped) {
        return 0;
    }
    equipped->item_id_ = GetItemId();

    CDBMsgSender sender(static_cast<ENUM_INTERNALPACKET>(138), user);
    if (sender.EvoluteCreature(GetUid(), GetItemId()) != 1) {
        cMyTrace trace("bool user_creature::CCreature::Evolute(CUser*)", 1891, 5);
        trace("cannot write db uid(%d)\n", user->get_unique_id());
        return 0;
    }

    sender.Send();
    CPacketRespondent respondent(user, 0, 0);
    respondent.MakeNotipacketEvoluteCreature(creature_id);
    respondent.SendAccordingToPlace();
    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(3),
                             oldsrv::config::kEquipmentTypeCreature);
    return 1;
}

// IDA_EA: 0x8338922
// IDA_NAME: _ZN13user_creature9CCreature24IsEvolutionCreatureMultyEi
// DEMANGLED: user_creature::CCreature::IsEvolutionCreatureMulty(int)
int CCreature::IsEvolutionCreatureMulty(int creature_id) {
    if (!creature_script_) {
        return 0;
    }
    return creature_script_->isEvoluationCreature(creature_id);
}

// IDA_EA: 0x8338950
// IDA_NAME: _ZN13user_creature9CCreature12EvoluteMultyEP5CUseri
// DEMANGLED: user_creature::CCreature::EvoluteMulty(CUser *,int)
int CCreature::EvoluteMulty(CUser* user, int creature_id) {
    if (!creature_script_) {
        cMyTrace trace("bool user_creature::CCreature::EvoluteMulty(CUser*, int)",
                       2115,
                       5);
        trace("cannot evolute because it haven't creature script uid(%d), creature id(%d) \n",
              user->get_unique_id(),
              GetItemId());
        return 0;
    }
    if (creature_script_->isEvoluationCreature(creature_id) != 1 ||
        creature_id == 0) {
        return 0;
    }

    CCreatureScript* const target_script =
        GetInstanceCreatureScriptMgr()->FindCreatureScript(creature_id);
    if (!target_script) {
        return 0;
    }

    SetItemId(target_script->GetItemId());
    CInventory* const inventory = user->getCurCharacInvenW();
    Inven_Item* const equipped = inventory->GetInvenRef(
        0, oldsrv::config::kEquipmentTypeCreature);
    if (!equipped) {
        return 0;
    }
    equipped->item_id_ = GetItemId();

    CDBMsgSender sender(static_cast<ENUM_INTERNALPACKET>(138), user);
    if (sender.EvoluteCreature(GetUid(), GetItemId()) != 1) {
        cMyTrace trace("bool user_creature::CCreature::EvoluteMulty(CUser*, int)",
                       2150,
                       5);
        trace("cannot write db uid(%d)\n", user->get_unique_id());
        return 0;
    }

    sender.Send();
    CPacketRespondent respondent(user, 0, 0);
    respondent.MakeNotipacketEvoluteCreature(creature_id);
    respondent.SendAccordingToPlace();
    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(3),
                             oldsrv::config::kEquipmentTypeCreature);
    return 1;
}

}  // namespace user_creature
