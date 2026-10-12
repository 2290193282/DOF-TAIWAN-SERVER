#include "old_game/appearance/pet/creature.h"

#include "common/config.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"

#include <cstring>

namespace user_creature {

// IDA_EA: 0x8340C82
// IDA_NAME: _ZN13user_creature13CCreatureItemC2Ev
// DEMANGLED: user_creature::CCreatureItem::CCreatureItem(void)
CCreatureItem::CCreatureItem() {}

// IDA_EA: 0x833705E
// IDA_NAME: _ZN13user_creature9CCreatureC2Ev
// DEMANGLED: user_creature::CCreature::CCreature(void)
CCreature::CCreature() {
    Reset();
}

// IDA_EA: 0x83370A2
// IDA_NAME: _ZN13user_creature9CCreatureD2Ev
// DEMANGLED: user_creature::CCreature::~CCreature(void)
CCreature::~CCreature() {}

// IDA_EA: 0x83385AA
// IDA_NAME: _ZN13user_creature9CCreature16LoadCreatureItemEPNS_12CCreatureMgrEP10Inven_ItemP29SIG_CREATURE_ITEM_DETAIL_INFOP5CUser
// DEMANGLED: user_creature::CCreature::LoadCreatureItem(user_creature::CCreatureMgr *,Inven_Item *,SIG_CREATURE_ITEM_DETAIL_INFO *,CUser *)
int CCreature::LoadCreatureItem(CCreatureMgr* manager,
                                Inven_Item* item,
                                SIG_CREATURE_ITEM_DETAIL_INFO* detail,
                                CUser* user) {
    if (detail->state) {
        item->reset();
        return 0;
    }

    experience_ = detail->experience;
    SetItemId(item->item_id_);
    uid_ = detail->ui_id;
    slot_no_ = detail->slot;
    creature_item_type_ = static_cast<int8_t>(detail->creature_type);
    stomach_.SetStomachValue(static_cast<float>(detail->stomach));
    std::strncpy(name_, detail->name, 12u);
    if (item_grow_state_ == 1u) {
        level_ = GetExpLevel(experience_);
        detail->grow_creature = 1;
    } else {
        level_ = GetExpLevel(static_cast<short>(experience_ >> 16));
        detail->grow_creature = 0;
    }
    detail->max_level = static_cast<unsigned char>(isMaxLevel());
    no_charge_ = detail->no_charge != 0u;
    stat_ = detail->state;
    expire_time_ = detail->expiration_value;
    if (item_grow_state_) {
        stomach_.Resume();
    }
    SetState(user, CREATURE_STATE_ABI_2);
    state_dirty_ = 1;
    if (detail->slot ==
            oldsrv::config::kCreatureInventoryEquippedCreatureSlot &&
        manager->Equip(this) != 1) {
        return 0;
    }
    if (manager->RegisterCreatureItem(this, detail->ui_id) != 1) {
        return 0;
    }

    Inven_Item* const inventory_item =
        user->getCurCharacInvenW()->GetInvenRef(3, GetSlotNo());
    inventory_item->set_add_info(uid_);
    state_dirty_ =
        static_cast<unsigned char>(stomach_.IsSaveStomach(state_));
    return 1;
}

// IDA_EA: 0x8338F4C
// IDA_NAME: _ZN13user_creature4CEggC2Ev
// DEMANGLED: user_creature::CEgg::CEgg(void)
CEgg::CEgg() {
    Reset();
}

// IDA_EA: 0x8338F74
// IDA_NAME: _ZN13user_creature4CEggD2Ev
// DEMANGLED: user_creature::CEgg::~CEgg(void)
CEgg::~CEgg() {}

// IDA_EA: 0x8338F82
// IDA_NAME: _ZN13user_creature4CEgg5ResetEv
// DEMANGLED: user_creature::CEgg::Reset(void)
void CEgg::Reset() {
    creature_item_type_ = CREATURE_ITME_TYPE_EGG;
    uid_ = 0;
    slot_no_ = 0;
    no_charge_ = 0;
}

// IDA_EA: 0x8338FAC
// IDA_NAME: _ZN13user_creature4CEgg16LoadCreatureItemEPNS_12CCreatureMgrEP10Inven_ItemP29SIG_CREATURE_ITEM_DETAIL_INFOP5CUser
// DEMANGLED: user_creature::CEgg::LoadCreatureItem(user_creature::CCreatureMgr *,Inven_Item *,SIG_CREATURE_ITEM_DETAIL_INFO *,CUser *)
int CEgg::LoadCreatureItem(CCreatureMgr* manager,
                           Inven_Item* item,
                           SIG_CREATURE_ITEM_DETAIL_INFO* detail,
                           CUser* user) {
    if (detail->state) {
        item->reset();
        return 0;
    }

    SetItemId(item->item_id_);
    uid_ = detail->ui_id;
    slot_no_ = detail->slot;
    creature_item_type_ = static_cast<int8_t>(detail->creature_type);
    expire_time_ = detail->expiration_value;
    item->seal_or_state_ = 1;
    no_charge_ = detail->no_charge != 0u;
    stat_ = detail->state;
    if (manager->RegisterCreatureItem(this, detail->ui_id) != 1) {
        return 0;
    }

    Inven_Item* const inventory_item =
        user->getCurCharacInvenW()->GetInvenRef(3, GetSlotNo());
    inventory_item->seal_or_state_ = 1;
    inventory_item->set_add_info(uid_);
    return 1;
}

// IDA_EA: 0x833F67A
// IDA_NAME: _ZN13user_creature20CCreatureItemFactoryC2Ev
// DEMANGLED: user_creature::CCreatureItemFactory::CCreatureItemFactory(void)
CCreatureItemFactory::CCreatureItemFactory()
    : egg_pool_(1000), creature_pool_(1000) {}

// IDA_EA: 0x833F6D0
// IDA_NAME: _ZN13user_creature20CCreatureItemFactoryD2Ev
// DEMANGLED: user_creature::CCreatureItemFactory::~CCreatureItemFactory(void)
CCreatureItemFactory::~CCreatureItemFactory() {}

// IDA_EA: 0x833F510
// IDA_NAME: _ZN13user_creature20CCreatureItemFactory13GetRemainSizeENS_18CREATURE_ITME_TYPEE
// DEMANGLED: user_creature::CCreatureItemFactory::GetRemainSize(user_creature::CREATURE_ITME_TYPE)
int CCreatureItemFactory::GetRemainSize(CREATURE_ITME_TYPE type) {
    if (type == CREATURE_ITME_TYPE_CREATURE) return creature_pool_.Remain();
    if (type != CREATURE_ITME_TYPE_EGG) return 0;
    return egg_pool_.Remain();
}

// IDA_EA: 0x833F546
// IDA_NAME: _ZN13user_creature20CCreatureItemFactory7GetSizeENS_18CREATURE_ITME_TYPEE
// DEMANGLED: user_creature::CCreatureItemFactory::GetSize(user_creature::CREATURE_ITME_TYPE)
int CCreatureItemFactory::GetSize(CREATURE_ITME_TYPE type) {
    if (type == CREATURE_ITME_TYPE_CREATURE) return creature_pool_.Size();
    if (type != CREATURE_ITME_TYPE_EGG) return 0;
    return egg_pool_.Size();
}

// IDA_EA: 0x833F57C
// IDA_NAME: _ZN13user_creature20CCreatureItemFactory19AcquireCreatureItemENS_18CREATURE_ITME_TYPEE
// DEMANGLED: user_creature::CCreatureItemFactory::AcquireCreatureItem(user_creature::CREATURE_ITME_TYPE)
CCreatureItem* CCreatureItemFactory::AcquireCreatureItem(
    CREATURE_ITME_TYPE type) {
    CCreatureItem* item = 0;
    if (type == CREATURE_ITME_TYPE_CREATURE) {
        item = creature_pool_.Acquire();
    } else if (type == CREATURE_ITME_TYPE_EGG) {
        item = egg_pool_.Acquire();
    }
    if (item) {
        item->Reset();
    } else {
        cMyTrace trace(
            "user_creature::CCreatureItem* user_creature::CCreatureItemFactory::AcquireCreatureItem(user_creature::CREATURE_ITME_TYPE)",
            104,
            5);
        trace("[%s][%d]",
              "user_creature::CCreatureItem* user_creature::CCreatureItemFactory::AcquireCreatureItem(user_creature::CREATURE_ITME_TYPE)");
    }
    return item;
}

// IDA_EA: 0x833F715
// IDA_NAME: _ZN13user_creature30GetInstanceCreatureItemFactoryEv
// DEMANGLED: user_creature::GetInstanceCreatureItemFactory(void)
CCreatureItemFactory* GetInstanceCreatureItemFactory() {
    static CCreatureItemFactory instance;
    return &instance;
}

// IDA_EA: 0x8336C63
// IDA_NAME: _ZN13user_creature22GetCreatureEggPoolSizeEv
// DEMANGLED: user_creature::GetCreatureEggPoolSize(void)
int GetCreatureEggPoolSize() {
    return GetInstanceCreatureItemFactory()->GetSize(CREATURE_ITME_TYPE_EGG);
}

// IDA_EA: 0x8336C80
// IDA_NAME: _ZN13user_creature19GetCreaturePoolSizeEv
// DEMANGLED: user_creature::GetCreaturePoolSize(void)
int GetCreaturePoolSize() {
    return GetInstanceCreatureItemFactory()->GetSize(
        CREATURE_ITME_TYPE_CREATURE);
}

// IDA_EA: 0x8336C9D
// IDA_NAME: _ZN13user_creature28GetCreatureEggPoolRemainSizeEv
// DEMANGLED: user_creature::GetCreatureEggPoolRemainSize(void)
int GetCreatureEggPoolRemainSize() {
    return GetInstanceCreatureItemFactory()->GetRemainSize(
        CREATURE_ITME_TYPE_EGG);
}

// IDA_EA: 0x8336CBA
// IDA_NAME: _ZN13user_creature25GetCreaturePoolRemainSizeEv
// DEMANGLED: user_creature::GetCreaturePoolRemainSize(void)
int GetCreaturePoolRemainSize() {
    return GetInstanceCreatureItemFactory()->GetRemainSize(
        CREATURE_ITME_TYPE_CREATURE);
}

}  // namespace user_creature
