#include "old_game/appearance/pet/creature.h"

#include "common/manager/os_api.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"

namespace user_creature {

// IDA_EA: 0x833EF20
// IDA_NAME: _ZN13user_creature13CCreatureItem3SetEPS0_
// DEMANGLED: user_creature::CCreatureItem::Set(user_creature::CCreatureItem *)
CCreatureItem* CCreatureItem::Set(CCreatureItem* source) {
    item_id_ = source->GetItemId();
    creature_item_type_ = source->GetCreatureItemType();
    uid_ = source->GetUid();
    slot_no_ = source->GetSlotNo();
    no_charge_ = static_cast<uint8_t>(source->GetNoCharge());
    stat_ = static_cast<uint8_t>(source->GetStat());
    expire_time_ = 0;
    return this;
}

// IDA_EA: 0x833EFE2
// IDA_NAME: _ZN13user_creature13CCreatureItem19SetCreatureItemTypeENS_18CREATURE_ITME_TYPEE
// DEMANGLED: user_creature::CCreatureItem::SetCreatureItemType(user_creature::CREATURE_ITME_TYPE)
CCreatureItem* CCreatureItem::SetCreatureItemType(CREATURE_ITME_TYPE type) {
    creature_item_type_ = static_cast<int>(type);
    return this;
}

// IDA_EA: 0x833912A
// IDA_NAME: _ZN13user_creature4CEgg5HatchEPi
// DEMANGLED: user_creature::CEgg::Hatch(int *)
int CEgg::Hatch(int* hatch_item_id) {
    CItem* const item = G_CDataManager()->find_item(GetItemId());
    if (!item || item->is_stackable() != 0) {
        return 0;
    }

    *hatch_item_id = static_cast<const CEquipItem*>(item)->get_hatch_item_id();
    return 1;
}

// IDA_EA: 0x8339FA0
// IDA_NAME: _ZN13user_creature12CCreatureMgr5HatchEi
// DEMANGLED: user_creature::CCreatureMgr::Hatch(int)
int CCreatureMgr::Hatch(int add_info) {
    CPacketRespondent response(owner_, 1, ENUM_CMDPACKET_HATCH_CREATURE);

    CCreatureItemMap::iterator existing = creature_items_.find(add_info);
    if (existing == creature_items_.end()) {
        response.SetError(21);
        return 0;
    }

    CCreatureItem* const egg_item = existing->second;
    if (egg_item->GetCreatureItemType() != CREATURE_ITME_TYPE_EGG) {
        response.SetError(23);
        return 0;
    }

    int hatch_item_id = 0;
    if (static_cast<CEgg*>(egg_item)->Hatch(&hatch_item_id) != 1) {
        response.SetError(17);
        return 0;
    }

    CCreatureItem* const creature_item =
        GetInstanceCreatureItemFactory()->AcquireCreatureItem(
            CREATURE_ITME_TYPE_CREATURE);
    if (!creature_item) {
        response.SetError(22);
        return 0;
    }

    creature_item->Set(egg_item);
    CCreature* const creature = static_cast<CCreature*>(creature_item);
    creature->SetItemId(hatch_item_id);
    creature_item->SetCreatureItemType(CREATURE_ITME_TYPE_CREATURE);
    creature->SetStomach(100);

    CItem* const hatched_item = G_CDataManager()->find_item(hatch_item_id);
    int expire_time = 0;
    if (hatched_item->getUsablePeriod() == 0 &&
        hatched_item->getExpirationDate() != 0) {
        expire_time = hatched_item->getExpirationDate() -
                      OS_API::GetDateTimeTick();
    } else {
        expire_time = OS_API::GetDateTimeTick() +
                      86400 * hatched_item->getUsablePeriod();
    }
    if (hatched_item->getUsablePeriod() == 0 &&
        hatched_item->getExpirationDate() == 0) {
        expire_time = 0;
    }
    creature_item->SetExpireTime(expire_time);

    UnregisterCreatureItem(egg_item->GetUid());
    RegisterCreatureItem(creature_item, creature_item->GetUid());

    const int slot = creature_item->GetSlotNo();
    Inven_Item* const inventory_item =
        owner_->getCurCharacInvenW()->GetInvenRef(3, slot);
    inventory_item->seal_or_state_ = 0;
    inventory_item->item_id_ = creature_item->GetItemId();

    CDBMsgSender sender(static_cast<ENUM_INTERNALPACKET>(138), owner_);
    if (sender.HatchCreature(creature_item->GetUid(),
                             CREATURE_ITME_TYPE_CREATURE,
                             creature_item->GetItemId()) != 1) {
        cMyTrace trace("bool user_creature::CCreatureMgr::Hatch(int)",
                       2842,
                       5);
        trace("cannot write db uid(%d)\\n", owner_->get_unique_id());
        response.SetError(2);
        return 0;
    }

    owner_->LogHistory("Hatch,%d,%d",
                       creature_item->GetItemId(),
                       creature_item->GetUid());
    sender.Send();
    response.SetOk();
    response.Send();
    response.MakeNotipacketCreatureItemList(&creature_items_, slot);
    response.Send();
    owner_->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                               static_cast<ENUM_ITEMSPACE>(7),
                               slot);
    return 1;
}

}  // namespace user_creature
