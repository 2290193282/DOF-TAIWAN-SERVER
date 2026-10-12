#include "old_game/appearance/pet/creature.h"
#include "old_game/appearance/pet/creature_fillpoint_runtime.h"

#include "common/config.h"
#include "old_data/db/inventory/load_inventory_payload.h"
#include "old_data/PVF/creature/creature_script.h"
#include "old_game/player/core/cuser.h"

#include <cstring>

namespace user_creature {
// IDA_EA: 0x83370B0
// IDA_NAME: _ZN13user_creature9CCreature5ResetEv
// DEMANGLED: user_creature::CCreature::Reset(void)
void CCreature::Reset() {
    creature_item_type_ = CREATURE_ITME_TYPE_CREATURE;
    uid_ = 0;
    slot_no_ = 0;
    state_ = CREATURE_STATE_ABI_2;
    state_dirty_ = 0;
    creature_script_ = 0;
    creature_id_ = 0;
    level_ = 1;
    experience_ = 0;
    no_charge_ = 0;
    item_grow_state_ = 1;
    item_grow_balance_ = 5;
    std::memset(name_, 0, 13u);
    stomach_.SetStomachValue(0.0f);
}

// IDA_EA: 0x8337330
// IDA_NAME: _ZN13user_creature9CCreature5EquipEP5CUser
// DEMANGLED: user_creature::CCreature::Equip(CUser *)
int CCreature::Equip(CUser* user) {
    SetState(user, user && user->is_fighting() ? CREATURE_STATE_ABI_1
                                               : CREATURE_STATE_ABI_3);
    return 1;
}

// IDA_EA: 0x8337382
// IDA_NAME: _ZN13user_creature9CCreature9DismantleEP5CUser
// DEMANGLED: user_creature::CCreature::Dismantle(CUser *)
int CCreature::Dismantle(CUser* user) {
    SetState(user, CREATURE_STATE_ABI_2);
    return 1;
}

// IDA_EA: 0x83373AA
// IDA_NAME: _ZN13user_creature9CCreature14RenameCreatureEPKc
// DEMANGLED: user_creature::CCreature::RenameCreature(char const*)
int CCreature::RenameCreature(const char* name) {
    std::strncpy(name_, name, 12u);
    return 1;
}

// IDA_EA: 0x83384B8
// IDA_NAME: _ZN13user_creature9CCreature17CheckDiedCreatureEP5CUser
// DEMANGLED: user_creature::CCreature::CheckDiedCreature(CUser *)
int CCreature::CheckDiedCreature(CUser* user) {
    if (!IsDieCreature()) return 0;
    Die(user);
    return 1;
}

// IDA_EA: 0x83388EC
// IDA_NAME: _ZN13user_creature9CCreature19IsEvolutionCreatureEi
// DEMANGLED: user_creature::CCreature::IsEvolutionCreature(int)
bool CCreature::IsEvolutionCreature(int creature_id) {
    CCreatureScript* const script = creature_script_;
    return script && script->GetEvolutionCreatureId() == creature_id;
}

// IDA_EA: 0x8338BFA
// IDA_NAME: _ZN13user_creature9CCreature24IsEventEvolutionCreatureEi
// DEMANGLED: user_creature::CCreature::IsEventEvolutionCreature(int)
int CCreature::IsEventEvolutionCreature(int creature_id) {
    CCreatureScript* const script = creature_script_;
    return script ? script->validEventEvlouteCreature(creature_id) : 0;
}

// IDA_EA: 0x822D3A8
// IDA_NAME: _ZNK13user_creature12CCreatureMgr18IsEquippedCreatureEv
// DEMANGLED: user_creature::CCreatureMgr::IsEquippedCreature(void)const
bool CCreatureMgr::IsEquippedCreature() const {
    return active_creature_ != 0;
}

// IDA_EA: 0x83392F6
// IDA_NAME: _ZN13user_creature12CCreatureMgr5EquipEPNS_9CCreatureE
// DEMANGLED: user_creature::CCreatureMgr::Equip(user_creature::CCreature *)
int CCreatureMgr::Equip(CCreature* creature) {
    if (!creature ||
        reinterpret_cast<CCreatureItem*>(creature)->GetCreatureItemType() !=
            1)
        return 0;
    CUser* const owner = owner_;
    if (creature->Equip(owner) != 1) return 0;
    CreatureFillPoint::ClearCooldown(owner);
    active_creature_ = creature;
    return 1;
}

// IDA_EA: 0x833934C
// IDA_NAME: _ZN13user_creature12CCreatureMgr9DismantleEPNS_9CCreatureE
// DEMANGLED: user_creature::CCreatureMgr::Dismantle(user_creature::CCreature *)
int CCreatureMgr::Dismantle(CCreature* creature) {
    if (!creature ||
        reinterpret_cast<CCreatureItem*>(creature)->GetCreatureItemType() !=
            1)
        return 0;
    CUser* const owner = owner_;
    if (creature->Dismantle(owner) != 1) return 0;
    CreatureFillPoint::ClearCooldown(owner);
    active_creature_ = 0;
    return 1;
}

// IDA_EA: 0x83393A2
// IDA_NAME: _ZN13user_creature12CCreatureMgr17EquipCreatureItemEPNS_9CCreatureES2_
// DEMANGLED: user_creature::CCreatureMgr::EquipCreatureItem(user_creature::CCreature *,user_creature::CCreature *)
bool CCreatureMgr::EquipCreatureItem(CCreature* old_creature,
                                     CCreature* new_creature) {
    if (old_creature && Dismantle(old_creature) != 1) return false;
    return !new_creature || Equip(new_creature) == 1;
}

// IDA_EA: 0x83393FC
// IDA_NAME: _ZN13user_creature12CCreatureMgr19IsArtifactEquipSlotEi
// DEMANGLED: user_creature::CCreatureMgr::IsArtifactEquipSlot(int)
bool CCreatureMgr::IsArtifactEquipSlot(int slot) {
    return slot >= oldsrv::config::kEquipmentTypeArtifactRed &&
           slot <= oldsrv::config::kEquipmentTypeArtifactGreen;
}

// IDA_EA: 0x83396C8
// IDA_NAME: _ZN13user_creature12CCreatureMgr13EquipArtifactEiP10Inven_Item
// DEMANGLED: user_creature::CCreatureMgr::EquipArtifact(int,Inven_Item *)
bool CCreatureMgr::EquipArtifact(int slot, Inven_Item* item) {
    CArtifact* const artifact = GetArtifact(slot);
    return artifact && artifact->EquipArtifact(item) == 1;
}

// IDA_EA: 0x8339718
// IDA_NAME: _ZN13user_creature12CCreatureMgr17DismantleArtifactEiP10Inven_Item
// DEMANGLED: user_creature::CCreatureMgr::DismantleArtifact(int,Inven_Item *)
int CCreatureMgr::DismantleArtifact(int slot, Inven_Item*) {
    CArtifact* const artifact = GetArtifact(slot);
    if (!artifact) return 0;
    artifact->Reset();
    return 1;
}

// IDA_EA: 0x8339420
// IDA_NAME: _ZN13user_creature12CCreatureMgr12SwapCreatureEP10Inven_ItemiiS2_ii
// DEMANGLED: CCreatureMgr::SwapCreature(Inven_Item *,int,int,Inven_Item *,int,int)
int CCreatureMgr::SwapCreature(Inven_Item* left,
                               int left_slot,
                               int left_space,
                               Inven_Item* right,
                               int right_slot,
                               int right_space) {
    if (!left || !right || !owner_) return 0;

    CCreatureItem* const left_creature =
        left->item_category_ == 5
            ? const_cast<CCreatureItem*>(FindCreatureItem(left->get_add_info()))
            : 0;
    CCreatureItem* const right_creature =
        right->item_category_ == 5
            ? const_cast<CCreatureItem*>(FindCreatureItem(right->get_add_info()))
            : 0;

    if (!left_creature && !right_creature) return 1;
    if (!IsCreatureEquipmentScope(left_slot) ||
        !IsCreatureEquipmentScope(right_slot)) {
        return 0;
    }

    // Move the active creature before the database swap is queued so runtime
    // state and persistence agree.
    if (left_slot == oldsrv::config::kEquipmentTypeCreature && !left_space) {
        if (!EquipCreatureItem(reinterpret_cast<CCreature*>(left_creature),
                               reinterpret_cast<CCreature*>(right_creature))) {
            return 0;
        }
    } else if (right_slot == oldsrv::config::kEquipmentTypeCreature &&
               !right_space) {
        if (!EquipCreatureItem(reinterpret_cast<CCreature*>(right_creature),
                               reinterpret_cast<CCreature*>(left_creature))) {
            return 0;
        }
    }

    // The 0828 client added weapon-avatar at physical equipment slot 10,
    // shifting creature/artifact equipment to 23..26.  The DB/login ABI did
    // not shift: its canonical equipment tail remains 238..241.  Project
    // between those namespaces instead of retaining the 0627 +216 offset
    // (which incorrectly persisted the active creature as slot 239).
    const int persisted_left_slot =
        left_slot +
        (left_space ? 0
                    : oldsrv::config::kCreatureEquipmentPersistenceOffset);
    const int persisted_right_slot =
        right_slot +
        (right_space ? 0
                     : oldsrv::config::kCreatureEquipmentPersistenceOffset);
    if (left_creature) left_creature->SetSlotNo(persisted_right_slot);
    if (right_creature) right_creature->SetSlotNo(persisted_left_slot);

    CDBMsgSender sender(static_cast<ENUM_INTERNALPACKET>(140), owner_);
    if (sender.SwapCreatureItem(owner_,
                                left,
                                persisted_left_slot,
                                right,
                                persisted_right_slot) != 1) {
        return 0;
    }
    return sender.Send();
}

// IDA_EA: 0x83399FA
// IDA_NAME: _ZN13user_creature12CCreatureMgr16SwapCreatureItemEP10Inven_ItemiiS2_ii
// DEMANGLED: CCreatureMgr::SwapCreatureItem(Inven_Item *,int,int,Inven_Item *,int,int)
int CCreatureMgr::SwapCreatureItem(Inven_Item* left,
                                   int left_slot,
                                   int left_space,
                                   Inven_Item* right,
                                   int right_slot,
                                   int right_space) {
    if (!left || !right) return 0;

    if (left->item_category_ == 5 || right->item_category_ == 5) {
        if (SwapCreature(left,
                         left_slot,
                         left_space,
                         right,
                         right_slot,
                         right_space) != 1) {
            return 0;
        }
    } else if ((left->item_category_ == 6 || right->item_category_ == 6) &&
               SwapArtifact(left,
                            left_slot,
                            left_space,
                            right,
                            right_slot,
                            right_space) != 1) {
        return 0;
    }
    return 1;
}

// IDA_EA: 0x8339ACC
// IDA_NAME: _ZN13user_creature12CCreatureMgr17CheckDiedCreatureEv
// DEMANGLED: user_creature::CCreatureMgr::CheckDiedCreature(void)
bool CCreatureMgr::CheckDiedCreature() {
    CCreature* const creature = active_creature_;
    CUser* const owner = owner_;
    return creature && creature->CheckDiedCreature(owner);
}

// IDA_EA: 0x833F618
// IDA_NAME: _ZN13user_creature20CCreatureItemFactory16FreeCreatureItemEPNS_13CCreatureItemE
// DEMANGLED: user_creature::CCreatureItemFactory::FreeCreatureItem(user_creature::CCreatureItem *)
int CCreatureItemFactory::FreeCreatureItem(CCreatureItem* item) {
    if (!item) return 0;
    const CREATURE_ITME_TYPE type =
        static_cast<CREATURE_ITME_TYPE>(item->GetCreatureItemType());
    if (type == CREATURE_ITME_TYPE_CREATURE) {
        return creature_pool_.Free(static_cast<CCreature*>(item));
    }
    if (type != CREATURE_ITME_TYPE_EGG) return 0;
    return egg_pool_.Free(static_cast<CEgg*>(item));
}

// IDA_EA: 0x8339752
// IDA_NAME: _ZN13user_creature12CCreatureMgr12SwapArtifactEP10Inven_ItemiiS2_ii
// DEMANGLED: user_creature::CCreatureMgr::SwapArtifact(Inven_Item *,int,int,Inven_Item *,int,int)
int CCreatureMgr::SwapArtifact(Inven_Item* left,
                               int left_slot,
                               int left_space,
                               Inven_Item* right,
                               int right_slot,
                               int right_space) {
    if (left_space && right_space) return 1;

    int equip_slot = -1;
    Inven_Item* item_to_equip = 0;
    int dismantle_slot = -1;
    Inven_Item* item_to_dismantle = 0;

    if (IsArtifactEquipSlot(left_slot)) {
        if (!right->isEmpty()) {
            equip_slot = left_slot;
            item_to_equip = right;
        }
        if (!left->isEmpty()) {
            dismantle_slot = left_slot;
            item_to_dismantle = left;
        }
    }
    if (IsArtifactEquipSlot(right_slot)) {
        if (!left->isEmpty()) {
            equip_slot = right_slot;
            item_to_equip = left;
        }
        if (!right->isEmpty()) {
            dismantle_slot = right_slot;
            item_to_dismantle = right;
        }
    }
    if (dismantle_slot != -1 && item_to_dismantle &&
        DismantleArtifact(dismantle_slot, item_to_dismantle) != 1) {
        return 0;
    }
    if (equip_slot != -1 && item_to_equip) {
        if (EquipArtifact(equip_slot, item_to_equip) != 1) return 0;
        item_to_equip->seal_or_state_ = 0;
    }
    return 1;
}

// IDA_EA: 0x833989C
// IDA_NAME: _ZN13user_creature12CCreatureMgr16DenySwapArtifactEP10Inven_ItemiiS2_ii
// DEMANGLED: user_creature::CCreatureMgr::DenySwapArtifact(Inven_Item *,int,int,Inven_Item *,int,int)
int CCreatureMgr::DenySwapArtifact(Inven_Item* left,
                                   int left_slot,
                                   int left_space,
                                   Inven_Item* right,
                                   int right_slot,
                                   int right_space) {
    (void)left_space;
    (void)right_space;

    if (active_creature_) {
        const bool rejected_active_slot =
            IsArtifactEquipSlot(left_slot) && !right->isEmpty();
        if (!rejected_active_slot ||
            active_creature_->IsAvailableArtifact(left_slot) == 1) {
            const bool rejected_other_slot =
                IsArtifactEquipSlot(right_slot) && !left->isEmpty();
            return rejected_other_slot &&
                   active_creature_->IsAvailableArtifact(left_slot) != 1;
        }
        return 1;
    }

    if (IsArtifactEquipSlot(left_slot) && right->isEmpty()) {
        return 0;
    }

    const bool rejected_other_slot =
        IsArtifactEquipSlot(right_slot) && left->isEmpty();
    return !rejected_other_slot;
}

// IDA_EA: 0x8339EE0
// IDA_NAME: _ZN13user_creature12CCreatureMgr10SetStomachEi
// DEMANGLED: user_creature::CCreatureMgr::SetStomach(int)
int CCreatureMgr::SetStomach(int value) {
    if (active_creature_) {
        active_creature_->SetStomach(value);
        CPacketRespondent respondent(owner_, 0, 0);
        const int stomach = active_creature_->GetStomach();
        const int uid = active_creature_->GetUid();
        respondent.MakeNotipacketCreatureState(uid, stomach);
        respondent.Send();
    }
    return 1;
}

// IDA_EA: 0x833BAEE
// IDA_NAME: _ZNK13user_creature12CCreatureMgr21checkMaxCreatureLevelEib
// DEMANGLED: user_creature::CCreatureMgr::checkMaxCreatureLevel(int,bool)const
bool CCreatureMgr::checkMaxCreatureLevel(int item_id,
                                         bool grow_creature) const {
    for (CCreatureItemMap::const_iterator it = creature_items_.begin();
         it != creature_items_.end(); ++it) {
        CCreatureItem* const item = it->second;
        if (!item || item->GetItemId() != item_id) continue;
        CCreature* const creature = reinterpret_cast<CCreature*>(item);
        if (static_cast<bool>(creature->IsGrowCreature()) == grow_creature)
            return creature->isMaxLevel() != 0;
    }
    return false;
}

// IDA_EA: 0x833BBB2
// IDA_NAME: _ZNK13user_creature12CCreatureMgr22isGrowCreatureMaxLevelEv
// DEMANGLED: user_creature::CCreatureMgr::isGrowCreatureMaxLevel(void)const
int CCreatureMgr::isGrowCreatureMaxLevel() const {
    return save_state_;
}

// IDA_EA: 0x833BE7C
// IDA_NAME: _ZN13user_creature12CCreatureMgr12LoadArtifactEiP10Inven_Item
// DEMANGLED: user_creature::CCreatureMgr::LoadArtifact(int,Inven_Item *)
bool CCreatureMgr::LoadArtifact(int slot, Inven_Item* item) {
    return item->isEmpty() || EquipArtifact(slot, item);
}

// IDA_EA: 0x833C31A
// IDA_NAME: _ZNK13user_creature12CCreatureMgr12GetExtraGoldEi
// DEMANGLED: user_creature::CCreatureMgr::GetExtraGold(int)const
int CCreatureMgr::GetExtraGold(int gold) const {
    CCreature* const creature = active_creature_;
    if (!creature || creature->IsDieCreature()) return 0;
    switch (creature->GetCreatureId()) {
    case 0x16:
    case 0x17:
    case 0x18:
    case 0x40:
        return 5 * gold / 100;
    case 0x2A:
    case 0x3F:
    case 0x36:
    case 0x37:
    case 0x38:
        return 3 * gold / 100;
    default:
        return 0;
    }
}

// IDA_EA: 0x833C6F6
// IDA_NAME: _ZNK13user_creature12CCreatureMgr21GetOverSkillExtraGoldEi
// DEMANGLED: user_creature::CCreatureMgr::GetOverSkillExtraGold(int)const
int CCreatureMgr::GetOverSkillExtraGold(int gold) const {
    CCreature* const creature = active_creature_;
    if (!creature || creature->IsDieCreature()) return 0;
    if (reinterpret_cast<CCreatureItem*>(creature)->GetItemId() != 63114)
        return 0;
    COverSkillTime* const over_skill_time = creature->GetOverSkillTime();
    return over_skill_time && over_skill_time->checkTime() ?
           5 * gold / 100 : 0;
}

// IDA_EA: 0x833D6C2
// IDA_NAME: _ZN13user_creature12CCreatureMgr11ItemEvoluteEi
// DEMANGLED: user_creature::CCreatureMgr::ItemEvolute(int)
int CCreatureMgr::ItemEvolute(int member_count) {
    CCreature* const creature = active_creature_;
    CUser* const owner = owner_;
    return creature ? creature->ItemEvolute(owner, member_count) : 0;
}

// IDA_EA: 0x833D6FA
// IDA_NAME: _ZNK13user_creature12CCreatureMgr26VerifyEquipedCreatureQuestEv
// DEMANGLED: user_creature::CCreatureMgr::VerifyEquipedCreatureQuest(void)const
int CCreatureMgr::VerifyEquipedCreatureQuest() const {
    CCreature* const creature = active_creature_;
    if (!creature) return 0;
    CUser* const owner = owner_;
    return creature->IsGrowCreature()
               ? creature->IsAbleSelectEvolute(owner)
               : creature->IsAbleEvolute(owner);
}

// IDA_EA: 0x833D7DE
// IDA_NAME: _ZN13user_creature12CCreatureMgr16RegistTempPeriodEii
// DEMANGLED: user_creature::CCreatureMgr::RegistTempPeriod(int,int)
int CCreatureMgr::RegistTempPeriod(int key, int period_value) {
    if (temporary_periods_.find(key) != temporary_periods_.end()) {
        return 0;
    }
    temporary_periods_.insert(std::make_pair(key, period_value));
    return 1;
}

// IDA_EA: 0x833D88C
// IDA_NAME: _ZNK13user_creature12CCreatureMgr19getRemainTempPeriodEii
// DEMANGLED: user_creature::CCreatureMgr::getRemainTempPeriod(int,int)const
int CCreatureMgr::getRemainTempPeriod(int key, int current_tick) const {
    CCreatureTempPeriodMap::const_iterator period =
        temporary_periods_.find(key);
    if (period != temporary_periods_.end()) {
        if (period->second <= 0) return period->second;
        const int remain = period->second - current_tick;
        return remain ? remain : 1;
    }
    const CCreatureItem* const item = FindCreatureItem(key);
    return item ? item->GetRemainDate(current_tick) : -1;
}

// IDA_EA: 0x833D962
// IDA_NAME: _ZN13user_creature12CCreatureMgr18UnRegistTempPeriodEi
// DEMANGLED: user_creature::CCreatureMgr::UnRegistTempPeriod(int)
int CCreatureMgr::UnRegistTempPeriod(int key) {
    CCreatureTempPeriodMap::iterator period = temporary_periods_.find(key);
    if (period == temporary_periods_.end()) return 0;
    temporary_periods_.erase(period);
    return 1;
}

// IDA_EA: 0x833D9D8
// IDA_NAME: _ZN13user_creature12CCreatureMgr22LoadCreatureTempPeriodEP31SIG_POSTAL_CREATURE_TEMP_PERIOD
// DEMANGLED: user_creature::CCreatureMgr::LoadCreatureTempPeriod(SIG_POSTAL_CREATURE_TEMP_PERIOD *)
int CCreatureMgr::LoadCreatureTempPeriod(SIG_POSTAL_CREATURE_TEMP_PERIOD* period) {
    if (period->expiration_time == -1) {
        return 0;
    }
    RegistTempPeriod(period->creature_id, period->expiration_time);
    return 1;
}

// IDA_EA: 0x833DEBC
// IDA_NAME: _ZNK13user_creature12CCreatureMgr32IsGrowCreature_Equipped_CreatureEv
// DEMANGLED: user_creature::CCreatureMgr::IsGrowCreature_Equipped_Creature(void)const
bool CCreatureMgr::IsGrowCreature_Equipped_Creature() const {
    CCreature* const creature = active_creature_;
    return creature && creature->IsGrowCreature() != 0;
}

}  // namespace user_creature
