#include "old_game/progression/quest/quest_runtime.h"
#include "infra/network/packet/PacketEnum.h"

#include "common/config.h"
#include "old_game/progression/career/expert_job_disjoint_types.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/instance_rental_system_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/progression/career/expert_job_statistic.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

#include <stdint.h>
#include <utility>
#include <vector>

namespace expert_job {

typedef std::vector<std::pair<int, int> > IntPairVector;


// IDA_EA: 0x849B734
// IDA_NAME: _ZN10expert_job19GetRecipeInputItemsEP5CItemRSt6vectorISt4pairIiiESaIS4_EE
// DEMANGLED: expert_job::GetRecipeInputItems(
//             CItem*,std::vector<std::pair<int,int> >&)
bool GetRecipeInputItems(CItem* recipe, IntPairVector& materials) {
    const IntPairVector& recipe_materials =
        static_cast<const CStackableItem*>(recipe)->recipe_input_items();
    for (IntPairVector::const_iterator it = recipe_materials.begin();
         it != recipe_materials.end();
         ++it) {
        CItem* const item = G_CDataManager()->find_item(it->first);
        if (item && item->is_stackable() &&
            static_cast<CStackableItem*>(item)->get_sub_type() != 1) {
            materials.push_back(*it);
        }
    }
    return false;
}

// IDA_EA: 0x849B820
// IDA_NAME: _ZN10expert_job17IsExistRecipeItemEP5CUserP5CItemb
// DEMANGLED: expert_job::IsExistRecipeItem(CUser*,CItem*,bool)
int IsExistRecipeItem(CUser* user,
                      CItem* recipe,
                      bool use_other_user_monster_card) {
    IntPairVector materials;
    if (!use_other_user_monster_card) {
        materials.clear();
        if (!recipe) {
            return 0;
        }
        materials =
            static_cast<const CStackableItem*>(recipe)->recipe_input_items();
        user->CalculateExpertJobMaterials(materials);
        return user->getCurCharacInvenR()->CheckNeedItemList(materials) == 1;
    }

    materials.clear();
    GetRecipeInputItems(recipe, materials);
    user->CalculateExpertJobMaterials(materials);
    if (materials.empty()) {
        return 0;
    }
    return user->getCurCharacInvenR()->CheckNeedItemList(materials) == 1;
}

// IDA_EA: 0x849B957
// IDA_NAME: _ZN10expert_job19DelMaterialOfRecipeEP5CUserRSt6vectorISt4pairIiiESaIS4_EEiP11PacketGuardRbi
// DEMANGLED: expert_job::DelMaterialOfRecipe(
//             CUser*,std::vector<std::pair<int,int> >&,int,
//             PacketGuard*,bool&,int)
bool DelMaterialOfRecipe(CUser* user,
                         IntPairVector& materials,
                         int recipe_slot,
                         PacketGuard* packet,
                         bool& equipment_changed,
                         int delete_recipe_item) {
    IntPairVector adjusted_materials;
    adjusted_materials.clear();
    adjusted_materials = materials;
    user->CalculateExpertJobMaterials(adjusted_materials);

    std::vector<int> deleted_materials;
    const int deleted_count =
        user->getCurCharacInvenW()->delete_event_items(
            adjusted_materials,
            deleted_materials,
            equipment_changed,
            static_cast<eMoneySubReason>(12),
            static_cast<eItemDelReason>(8),
            false);
    if (!deleted_count) {
        cMyTrace trace(
            "bool expert_job::DelMaterialOfRecipe(CUser*, IntPairVector&, "
            "int, PacketGuard*, bool&, int)",
            448,
            5);
        trace("ExpertJobMaterial::DelMaterialOfRecipe, "
              "getCurCharacInvenR()->delete_event_items , "
              "size does not match! %d %d!=%d",
              user->getCurCharacNo(),
              deleted_count,
              static_cast<int>(adjusted_materials.size()));
        return false;
    }

    if (delete_recipe_item == 1) {
        packet->put_byte(deleted_count + 1);
        if (user->getCurCharacInvenW()->delete_item(
                static_cast<INVEN_TYPE>(1),
                recipe_slot,
                1,
                static_cast<eItemDelReason>(8),
                true) != 1) {
            cMyTrace trace(
                "bool expert_job::DelMaterialOfRecipe(CUser*, "
                "IntPairVector&, int, PacketGuard*, bool&, int)",
                463,
                5);
            trace("CUser::compound_item, "
                  "getCurCharacInvenW()->delete_item failed , %d %d",
                  user->getCurCharacNo(),
                  recipe_slot);
            return false;
        }
        packet->put_byte(0);
        packet->put_short(recipe_slot);
        packet->put_int(1);
    } else {
        packet->put_byte(deleted_count);
    }

    if (deleted_count > 0) {
        std::vector<int>::const_iterator it = deleted_materials.begin();
        while (it != deleted_materials.end()) {
            packet->put_byte(*it++);
            packet->put_short(*it++);
            packet->put_int(*it++);
        }
    }
    return true;
}

// IDA_EA: 0x849BCFB
// IDA_NAME: _ZN10expert_job19UseMaterialOfRecipeEP5CUserP5CItemP11PacketGuardb
// DEMANGLED: expert_job::UseMaterialOfRecipe(
//             CUser*,CItem*,PacketGuard*,bool)
int UseMaterialOfRecipe(CUser* user,
                        CItem* recipe,
                        PacketGuard* packet,
                        bool use_other_user_monster_card) {
    bool equipment_changed = false;
    if (use_other_user_monster_card) {
        IntPairVector materials;
        GetRecipeInputItems(recipe, materials);
        if (materials.empty()) {
            return 17;
        }
        if (!DelMaterialOfRecipe(user,
                                 materials,
                                 0,
                                 packet,
                                 equipment_changed,
                                 0)) {
            return 17;
        }
    } else {
        IntPairVector materials =
            static_cast<const CStackableItem*>(recipe)->recipe_input_items();
        if (!DelMaterialOfRecipe(user,
                                 materials,
                                 0,
                                 packet,
                                 equipment_changed,
                                 0)) {
            return 17;
        }
    }

    if (equipment_changed) {
        user->send_equip(oldsrv::config::kEquipmentTypeCreature);
    }
    return 0;
}

// IDA_EA: 0x849BE32
// IDA_NAME: _ZN10expert_job15FindMonsterCardEP5CItem
// DEMANGLED: expert_job::FindMonsterCard(CItem*)
CItem* FindMonsterCard(CItem* recipe) {
    const IntPairVector& materials =
        static_cast<const CStackableItem*>(recipe)->recipe_input_items();
    for (IntPairVector::const_iterator it = materials.begin();
         it != materials.end();
         ++it) {
        CItem* const item = G_CDataManager()->find_item(it->first);
        if (item && item->is_stackable() &&
            static_cast<CStackableItem*>(item)->get_sub_type() == 1) {
            return item;
        }
    }
    return 0;
}

// IDA_EA: 0x849F23E
// IDA_NAME: _ZN10expert_job17ExpertJobMaterialC2Ev
// DEMANGLED: expert_job::ExpertJobMaterial::ExpertJobMaterial(void)
ExpertJobMaterial::ExpertJobMaterial() {
    Reset();
}

// IDA_EA: 0x849F252
// IDA_NAME: _ZN10expert_job17ExpertJobMaterial5ResetEv
// DEMANGLED: expert_job::ExpertJobMaterial::Reset(void)
ExpertJobMaterial* ExpertJobMaterial::Reset() {
    slot_ = 0;
    item_space_ = 0;
    user_ = 0;
    item_ = 0;
    item_id_ = 0;
    return this;
}

// IDA_EA: 0x849F286
// IDA_NAME: _ZN10expert_job17ExpertJobMaterial5EmptyEv
// DEMANGLED: expert_job::ExpertJobMaterial::Empty(void)
bool ExpertJobMaterial::Empty() {
    return !item_ || !item_id_ || !user_;
}

// IDA_EA: 0x849BF08
// IDA_NAME: _ZN10expert_job17ExpertJobMaterial7SetDataEP5CUsertt
// DEMANGLED: expert_job::ExpertJobMaterial::SetData(
//             CUser*,unsigned short,unsigned short)
int ExpertJobMaterial::SetData(CUser* user,
                               unsigned short item_space,
                               unsigned short slot) {
    if (item_space) {
        return 21;
    }

    const Inven_Item* const inventory_item =
        user->GetCurCharacInventoryRef(0, slot);
    if (!inventory_item || !inventory_item->item_id_) {
        return 21;
    }

    item_id_ = static_cast<int>(inventory_item->item_id_);
    item_ = G_CDataManager()->find_item(item_id_);
    if (!item_) {
        return 21;
    }

    user_ = user;
    slot_ = slot;
    item_space_ = 0;
    return 0;
}

// IDA_EA: 0x849BFD0
// IDA_NAME: _ZN10expert_job17ExpertJobMaterial9CheckTypeEbi
// DEMANGLED: expert_job::ExpertJobMaterial::CheckType(bool,int)
int ExpertJobMaterial::CheckType(bool stackable, int item_type) {
    if (Empty()) {
        return 17;
    }
    if (static_cast<bool>(item_->is_stackable()) != stackable) {
        return 23;
    }
    if (stackable) {
        if (static_cast<CStackableItem*>(item_)->GetItemType() != item_type) {
            return 23;
        }
    } else if (static_cast<CEquipItem*>(item_)->IsEnchantalbe() != 1) {
        return 23;
    }
    return 0;
}

// IDA_EA: 0x849C06E
// IDA_NAME: _ZN10expert_job17ExpertJobMaterial7IsRightEv
// DEMANGLED: expert_job::ExpertJobMaterial::IsRight(void)
int ExpertJobMaterial::IsRight() {
    if (Empty()) {
        return 17;
    }
    if (!user_->getCurCharacR()) {
        return 3;
    }
    if (item_space_) {
        return 17;
    }

    const Inven_Item* const inventory_item =
        user_->GetCurCharacInventoryRef(item_space_, slot_);
    if (!inventory_item) {
        return 17;
    }
    return static_cast<int>(inventory_item->item_id_) == item_id_
               ? 0
               : 17;
}

// IDA_EA: 0x849C114
// IDA_NAME: _ZN10expert_job17ExpertJobMaterial8IsRecipeEv
// DEMANGLED: expert_job::ExpertJobMaterial::IsRecipe(void)
int ExpertJobMaterial::IsRecipe() {
    if (item_->is_stackable() != 1) {
        return 23;
    }
    return static_cast<CStackableItem*>(item_)->GetItemType() == 3 ? 0 : 23;
}

// IDA_EA: 0x849C166
// IDA_NAME: _ZN10expert_job17ExpertJobMaterial23IsExistMaterialOfRecipeEv
// DEMANGLED: expert_job::ExpertJobMaterial::IsExistMaterialOfRecipe(void)
int ExpertJobMaterial::IsExistMaterialOfRecipe() {
    if (Empty()) {
        return 17;
    }
    const int recipe_error = IsRecipe();
    if (recipe_error) {
        return recipe_error;
    }
    if (!user_->getCurCharacR()) {
        return 3;
    }
    return user_->getCurCharacInvenR()->CheckNeedItemList(
               static_cast<const CStackableItem*>(item_)
                   ->recipe_input_items()) == 1
               ? 0
               : 21;
}

// IDA_EA: 0x849C204
// IDA_NAME: _ZN10expert_job17ExpertJobMaterial19DelMaterialOfRecipeEP5CUserRKSt6vectorISt4pairIiiESaIS5_EEiP11PacketGuardRb
// DEMANGLED: expert_job::ExpertJobMaterial::DelMaterialOfRecipe(
//             CUser*,std::vector<std::pair<int,int> > const&,int,
//             PacketGuard*,bool&)
bool ExpertJobMaterial::DelMaterialOfRecipe(
    CUser* user,
    const IntPairVector& materials,
    int recipe_slot,
    PacketGuard* packet,
    bool& equipment_changed) {
    std::vector<int> deleted_materials;
    const int deleted_count =
        user->getCurCharacInvenW()->delete_event_items(
            materials,
            deleted_materials,
            equipment_changed,
            static_cast<eMoneySubReason>(12),
            static_cast<eItemDelReason>(8),
            false);
    if (!deleted_count) {
        cMyTrace trace(
            "bool expert_job::ExpertJobMaterial::DelMaterialOfRecipe("
            "CUser*, const IntPairVector&, int, PacketGuard*, bool&)",
            648,
            5);
        trace("ExpertJobMaterial::DelMaterialOfRecipe, "
              "getCurCharacInvenR()->delete_event_items , "
              "size does not match! %d %d!=%d",
              user->getCurCharacNo(),
              deleted_count,
              static_cast<int>(materials.size()));
        return false;
    }

    packet->put_byte(deleted_count + 1);
    if (user->getCurCharacInvenW()->delete_item(
            static_cast<INVEN_TYPE>(1),
            recipe_slot,
            1,
            static_cast<eItemDelReason>(8),
            true) != 1) {
        cMyTrace trace(
            "bool expert_job::ExpertJobMaterial::DelMaterialOfRecipe("
            "CUser*, const IntPairVector&, int, PacketGuard*, bool&)",
            658,
            5);
        trace("CUser::compound_item, "
              "getCurCharacInvenW()->delete_item failed , %d %d",
              user->getCurCharacNo(),
              recipe_slot);
        return false;
    }

    packet->put_byte(0);
    packet->put_short(recipe_slot);
    packet->put_int(1);
    if (deleted_count > 0) {
        std::vector<int>::const_iterator it = deleted_materials.begin();
        while (it != deleted_materials.end()) {
            packet->put_byte(*it++);
            packet->put_short(*it++);
            packet->put_int(*it++);
        }
    }
    return true;
}

// IDA_EA: 0x849C536
// IDA_NAME: _ZN10expert_job17ExpertJobMaterial19UseMaterialOfRecipeEP11PacketGuard
// DEMANGLED: expert_job::ExpertJobMaterial::UseMaterialOfRecipe(PacketGuard*)
int ExpertJobMaterial::UseMaterialOfRecipe(PacketGuard* packet) {
    if (Empty()) {
        return 17;
    }
    const int recipe_error = IsRecipe();
    if (recipe_error) {
        return recipe_error;
    }
    if (!user_->getCurCharacR()) {
        return 3;
    }

    bool equipment_changed = false;
    if (!DelMaterialOfRecipe(user_,
                             static_cast<const CStackableItem*>(item_)
                                 ->recipe_input_items(),
                             slot_,
                             packet,
                             equipment_changed)) {
        return 17;
    }
    if (equipment_changed) {
        user_->send_equip(oldsrv::config::kEquipmentTypeCreature);
    }
    return 0;
}

// IDA_EA: 0x849C612
// IDA_NAME: _ZN10expert_job17ExpertJobMaterial15FindMonsterCardERKSt6vectorISt4pairIiiESaIS3_EE
// DEMANGLED: expert_job::ExpertJobMaterial::FindMonsterCard(
//             std::vector<std::pair<int,int> > const&)
CItem* ExpertJobMaterial::FindMonsterCard(const IntPairVector& materials) {
    for (IntPairVector::const_iterator it = materials.begin();
         it != materials.end();
         ++it) {
        CItem* const item = G_CDataManager()->find_item(it->first);
        if (item && item->is_stackable() &&
            static_cast<CStackableItem*>(item)->get_sub_type() == 1) {
            return item;
        }
    }
    return 0;
}

// IDA_EA: 0x849C6CA
// IDA_NAME: _ZN10expert_job17ExpertJobMaterial14GetEnchantItemEv
// DEMANGLED: expert_job::ExpertJobMaterial::GetEnchantItem(void)
CItem* ExpertJobMaterial::GetEnchantItem() {
    if (Empty() || IsRecipe()) {
        return 0;
    }
    return FindMonsterCard(
        static_cast<const CStackableItem*>(item_)->recipe_input_items());
}

// IDA_EA: 0x849F2C2
// IDA_NAME: _ZN10expert_job10CEnchanterC2Ev
// DEMANGLED: expert_job::CEnchanter::CEnchanter(void)
CEnchanter::CEnchanter()
    : CExpertJob(),
      materials_() {}

// IDA_EA: 0x849FD86 / 0x849FDB6
// IDA_NAME: _ZN10expert_job10CEnchanterD2Ev /
//           _ZN10expert_job10CEnchanterD0Ev
// DEMANGLED: expert_job::CEnchanter::~CEnchanter()
CEnchanter::~CEnchanter() {}

// IDA_EA: 0x849C71E
// IDA_NAME: _ZN10expert_job10CEnchanter5ResetEv
// DEMANGLED: expert_job::CEnchanter::Reset(void)
void CEnchanter::Reset() {
    for (int index = 0; index <= 1; ++index) {
        materials_[index].Reset();
    }
    mission_user_ = 0;
    peer_user_ = 0;
    mission_unique_id_ = 0;
    peer_unique_id_ = 0;
    fee_ = 0;
    SetType(static_cast<ENUM_EXPERT_JOB_TYPE>(1));
    enchant_item_ = 0;
    peer_request_type_ = static_cast<ENUM_PEER_REQUEST_TYPE>(0);
    recipe_item_ = 0;
}

// IDA_EA: 0x849C822
// IDA_NAME: _ZN10expert_job10CEnchanter14OnCloseMissionEP5CUserNS_23ENUM_CLOSE_MISSION_TYPEE
// DEMANGLED: expert_job::CEnchanter::OnCloseMission(
//             CUser*,expert_job::ENUM_CLOSE_MISSION_TYPE)
void CEnchanter::OnCloseMission(
    CUser* user,
    ENUM_CLOSE_MISSION_TYPE close_type) {
    CUser* other_user = 0;
    if (static_cast<uint16_t>(user->get_unique_id()) ==
        mission_unique_id_) {
        mission_user_->SetCurCharacExpertJob(0);
        peer_user_->SetCurCharacExpertJob(0);
        other_user = peer_user_;
    } else if (static_cast<uint16_t>(user->get_unique_id()) ==
               peer_unique_id_) {
        mission_user_->SetCurCharacExpertJob(0);
        peer_user_->SetCurCharacExpertJob(0);
        other_user = mission_user_;
    } else {
        return;
    }

    const int close_value = static_cast<int>(close_type);
    if ((close_value == 1 || close_value == 2) &&
        other_user &&
        other_user->getCurCharacR() &&
        mission_user_ != peer_user_) {
        PacketGuard packet;
        packet.put_header(0, 158);
        packet.finalize(true);
        other_user->Send(packet);
    }
    GlobalData::s_ExpertJobMgr->FreeExpertJob(this);
}

// IDA_EA: 0x849C9B2
// IDA_NAME: _ZN10expert_job10CEnchanter15IsRightMaterialEv
// DEMANGLED: expert_job::CEnchanter::IsRightMaterial(void)
int CEnchanter::IsRightMaterial() {
    for (int index = 0; index <= 1; ++index) {
        const int error = materials_[index].IsRight();
        if (error) {
            return error;
        }
    }
    return 0;
}

// IDA_EA: 0x849CA06
// IDA_NAME: _ZN10expert_job10CEnchanter13IsRecipeRightEv
// DEMANGLED: expert_job::CEnchanter::IsRecipeRight(void)
bool CEnchanter::IsRecipeRight() {
    return recipe_item_ &&
           recipe_item_->is_stackable() == 1 &&
           recipe_item_->GetItemType() == 3;
}

// IDA_EA: 0x849CA6A
// IDA_NAME: _ZN10expert_job10CEnchanter17IsEqualMonsterCadEv
// DEMANGLED: expert_job::CEnchanter::IsEqualMonsterCad(void)
bool CEnchanter::IsEqualMonsterCad() {
    CItem* const monster_card = FindMonsterCard(recipe_item_);
    return monster_card &&
           monster_card->get_index() == enchant_item_->get_index();
}

// IDA_EA: 0x849CAC6
// IDA_NAME: _ZN10expert_job10CEnchanter12_DrawEnchantEv
// DEMANGLED: expert_job::CEnchanter::_DrawEnchant(void)
bool CEnchanter::_DrawEnchant() {
    int expert_job_exp = 0;
    if (!CExpertJob::IsSuccess(
            mission_user_, recipe_item_, expert_job_exp)) {
        return false;
    }
    CExpertJob::IncreaseExpertJobExp(
        mission_user_,
        mission_user_->GetCurCharacExpertJobExp() + expert_job_exp);
    return true;
}

// IDA_EA: 0x849CB2C
// IDA_NAME: _ZN10expert_job10CEnchanter10_DoEnchantE21ENUM_TRY_ENCHANT_TYPE
// DEMANGLED: expert_job::CEnchanter::_DoEnchant(ENUM_TRY_ENCHANT_TYPE)
int CEnchanter::_DoEnchant(ENUM_TRY_ENCHANT_TYPE try_type) {
    const bool has_distinct_peer =
        peer_user_ &&
        peer_user_->getCurCharacR() &&
        mission_user_ != peer_user_;
    if (has_distinct_peer && fee_ > 0) {
        const bool mission_and_peer_ready =
            mission_user_ && peer_user_->getCurCharacR();
        if (mission_and_peer_ready &&
            !mission_user_->CheckMoney(fee_)) {
            return 17;
        }
        if (peer_user_->getCurCharacInvenR()->get_money() < fee_) {
            return 17;
        }
    }

    int error = IsRightMaterial();
    if (error) {
        return error;
    }
    if (IsExistRecipeItem(
            mission_user_,
            recipe_item_,
            IsUseOtherUserMonsterCard()) != 1) {
        return 21;
    }

    bool enchant_succeeded = false;
    PacketGuard packet;
    const int inventory_type = GetInvenTypeFromItemSpace(
        static_cast<ENUM_ITEMSPACE>(materials_[0].item_space_));
    Inven_Item* const inventory_item =
        materials_[0].user_->getCurCharacInvenW()->GetInvenRef(
            inventory_type,
            materials_[0].slot_);
    if (!inventory_item) {
        return 17;
    }
    if (GlobalData::s_rentalSystem->CheckRentableItem(
            1, inventory_item->item_id_)) {
        return 17;
    }
    if (GlobalData::s_rentalSystem->CheckRentableItem(
            2, inventory_item->item_id_)) {
        return 17;
    }

    const int target_item_id = materials_[0].item_id_;
    const int old_enchant_item_id = inventory_item->extend_info_;
    CItem* applied_enchant_item = 0;

    if (_DrawEnchant()) {
        applied_enchant_item = enchant_item_;
        if (applied_enchant_item) {
            inventory_item->extend_info_ =
                applied_enchant_item->get_index();
        }

        packet.clear();
        packet.put_header(0, 14);
        packet.put_byte(materials_[0].item_space_);
        packet.put_short(1);
        if (peer_user_->getCurCharacInvenR()->MakeItemPacket(
                static_cast<INVEN_TYPE>(inventory_type),
                materials_[0].slot_,
                packet) != 1) {
            return 0;
        }
        packet.finalize(true);
        peer_user_->Send(packet);
        enchant_succeeded = true;
    }

    if (applied_enchant_item && mission_user_) {
        GetInstanceExpertJobStatistic()->increaseItemCount(
            static_cast<ENUM_EXPERT_JOB_TYPE>(
                mission_user_->GetCurCharacExpertJobType()),
            static_cast<unsigned long>(
                applied_enchant_item->get_index()),
            1);
    }

    if (mission_user_ && mission_user_->getCurCharacR()) {
        if (fee_ > 0) {
            mission_user_->getCurCharacInvenW()->gain_money(
                fee_,
                static_cast<eMoneyAddReason>(16),
                true,
                0);
            mission_user_->SendUpdateItemList(
                static_cast<CUser::eSendTarget>(1),
                static_cast<ENUM_ITEMSPACE>(0),
                0);
        }

        packet.clear();
        packet.put_header(1, ENUM_CMDPACKET_TRY_ENCHANT);
        packet.put_byte(1);
        packet.put_byte(static_cast<int>(try_type));
        packet.put_byte(enchant_succeeded);
        packet.put_int(mission_user_->GetCurCharacExpertJobExp());
        packet.put_int(mission_user_->getCurCharacInvenR()->get_money());
        error = UseMaterialOfRecipe(
            mission_user_,
            recipe_item_,
            &packet,
            IsUseOtherUserMonsterCard());
        if (error) {
            inventory_item->extend_info_ = 0;
            return error;
        }
        packet.finalize(true);
        mission_user_->Send(packet);
    }

    if (peer_user_ &&
        peer_user_->getCurCharacR() &&
        mission_user_ != peer_user_) {
        if (fee_ > 0) {
            peer_user_->getCurCharacInvenW()->use_money(
                fee_,
                static_cast<eMoneySubReason>(23),
                true);
            peer_user_->SendUpdateItemList(
                static_cast<CUser::eSendTarget>(1),
                static_cast<ENUM_ITEMSPACE>(0),
                0);
        }

        if (IsUseOtherUserMonsterCard()) {
            const int monster_card_slot = materials_[1].slot_;
            if (peer_user_->getCurCharacInvenW()->delete_item(
                    static_cast<INVEN_TYPE>(1),
                    monster_card_slot,
                    1,
                    static_cast<eItemDelReason>(8),
                    true) != 1) {
                cMyTrace trace(
                    "int expert_job::CEnchanter::_DoEnchant("
                    "ENUM_TRY_ENCHANT_TYPE)",
                    985,
                    5);
                trace("CUser::compound_item, "
                      "getCurCharacInvenW()->delete_item failed , %d %d",
                      peer_user_->getCurCharacNo(),
                      monster_card_slot);
            }

            packet.clear();
            packet.put_header(0, 14);
            packet.put_byte(0);
            packet.put_short(1);
            peer_user_->getCurCharacInvenR()->MakeItemPacket(
                static_cast<INVEN_TYPE>(1),
                monster_card_slot,
                packet);
            packet.finalize(true);
            peer_user_->Send(packet);
        }

        packet.clear();
        packet.put_header(1, ENUM_CMDPACKET_AGREE_ENCHANT);
        packet.put_byte(1);
        packet.put_byte(enchant_succeeded);
        packet.finalize(true);
        peer_user_->Send(packet);
    }

    peer_user_->history_log().EnchantItem(
        target_item_id,
        old_enchant_item_id,
        inventory_item->extend_info_,
        enchant_succeeded,
        peer_user_->get_acc_name(),
        peer_user_->getCurCharacName());
    return 0;
}

// IDA_EA: 0x849D344
// IDA_NAME: _ZN10expert_job10CEnchanter18_PutItemForEnchantEP5CUseriP11stItemPos_ti
// DEMANGLED: expert_job::CEnchanter::_PutItemForEnchant(
//             CUser*,int,stItemPos_t*,int)
int CEnchanter::_PutItemForEnchant(CUser* user,
                                   int put_type,
                                   stItemPos_t* item_position,
                                   int item_id) {
    if (!user->getCurCharacR()) {
        return 3;
    }

    if (static_cast<uint16_t>(user->get_unique_id()) !=
        static_cast<uint16_t>(peer_user_->get_unique_id())) {
        if (put_type != 2 || IsUseOtherUserMonsterCard()) {
            return 3;
        }
        if (static_cast<uint16_t>(user->get_unique_id()) !=
            static_cast<uint16_t>(mission_user_->get_unique_id())) {
            return 3;
        }
    }

    switch (put_type) {
    case 1: {
        if (!materials_[0].Empty()) {
            return 20;
        }
        int error = materials_[0].SetData(
            user, item_position->item_space, item_position->slot);
        if (error) {
            return error;
        }
        error = materials_[0].CheckType(false, 0);
        if (error) {
            return error;
        }
        break;
    }
    case 2: {
        if (!materials_[1].Empty()) {
            return 20;
        }
        int error = materials_[1].SetData(
            user, item_position->item_space, item_position->slot);
        if (error) {
            return error;
        }
        error = materials_[1].CheckType(true, 2);
        if (error) {
            return error;
        }
        break;
    }
    case 0:
        if (item_id < 0) {
            return 95;
        }
        if (fee_ > 0) {
            return 20;
        }
        if (mission_user_->CheckMoney(item_id) != 1) {
            return 95;
        }
        if (user->getCurCharacInvenR()->get_money() < item_id) {
            return 22;
        }
        fee_ = item_id;
        break;
    }

    if (!mission_user_ || !mission_user_->getCurCharacR()) {
        return 0;
    }

    PacketGuard packet;
    packet.put_header(0, 157);
    packet.put_byte(put_type);
    switch (put_type) {
    case 1:
        packet.put_int(materials_[0].item_id_);
        break;
    case 2:
        packet.put_int(materials_[1].item_id_);
        break;
    case 0:
        packet.put_int(fee_);
        break;
    }
    packet.finalize(true);

    if (static_cast<uint16_t>(user->get_unique_id()) ==
        static_cast<uint16_t>(mission_user_->get_unique_id())) {
        if (!peer_user_ || !peer_user_->getCurCharacR()) {
            return 3;
        }
        peer_user_->Send(packet);
    } else {
        mission_user_->Send(packet);
    }
    return 0;
}

// IDA_EA: 0x849D7B6
// IDA_NAME: _ZN10expert_job10CEnchanter15_TryEnchantMineEP5CUserP11stItemPos_t
// DEMANGLED: expert_job::CEnchanter::_TryEnchantMine(
//             CUser*,stItemPos_t*)
int CEnchanter::_TryEnchantMine(CUser* user,
                                stItemPos_t* item_position) {
    int error = materials_[0].SetData(
        user, item_position->item_space, item_position->slot);
    if (error) {
        return error;
    }

    CItem* const monster_card = FindMonsterCard(recipe_item_);
    if (!monster_card) {
        return 17;
    }

    Inven_Item inventory_item;
    const int monster_card_slot =
        user->getCurCharacInvenR()->GetInvenData(
            monster_card->get_index(), inventory_item);
    if (monster_card_slot == -1) {
        return 17;
    }

    error = materials_[1].SetData(user, 0, monster_card_slot);
    if (error) {
        return error;
    }
    if (!IsRecipeRight()) {
        return 17;
    }
    if (CExpertJob::IsLearnRecipe(user, recipe_item_) != 1) {
        return 17;
    }
    error = materials_[0].CheckType(false, 0);
    if (error) {
        return error;
    }
    error = materials_[1].CheckType(true, 2);
    if (error) {
        return error;
    }

    enchant_item_ = materials_[1].item_;
    if (!enchant_item_) {
        return 17;
    }
    if (!IsEqualMonsterCad()) {
        return 17;
    }
    if (!EnableWorld(materials_[0].item_, recipe_item_)) {
        return 17;
    }
    if (static_cast<CStackableItem*>(enchant_item_)
            ->getUsableItemType(
                static_cast<CEquipItem*>(materials_[0].item_)
                    ->GetItemType()) != 1) {
        return 23;
    }

    error = _DoEnchant(static_cast<ENUM_TRY_ENCHANT_TYPE>(0));
    return error ? error : 0;
}

// IDA_EA: 0x849DA3E
// IDA_NAME: _ZN10expert_job10CEnchanter16_TryEnchantOtherEP5CUserP11stItemPos_t
// DEMANGLED: expert_job::CEnchanter::_TryEnchantOther(
//             CUser*,stItemPos_t*)
int CEnchanter::_TryEnchantOther(CUser* user,
                                 stItemPos_t* item_position) {
    (void)item_position;
    if (materials_[0].Empty()) {
        return 21;
    }
    if (!IsRecipeRight()) {
        return 17;
    }
    if (CExpertJob::IsLearnRecipe(user, recipe_item_) != 1) {
        return 17;
    }
    if (materials_[1].Empty()) {
        return 20;
    }

    enchant_item_ = materials_[1].item_;
    if (!enchant_item_) {
        return 17;
    }
    if (!IsEqualMonsterCad()) {
        return 17;
    }
    if (!EnableWorld(materials_[0].item_, recipe_item_)) {
        return 17;
    }
    if (static_cast<CStackableItem*>(enchant_item_)
            ->getUsableItemType(
                static_cast<CEquipItem*>(materials_[0].item_)
                    ->GetItemType()) != 1) {
        return 23;
    }

    int rate_variation = 0;
    stCompoundRate* const success_rate =
        CExpertJob::GetSuccessRate(user, recipe_item_, rate_variation);
    if (!success_rate) {
        return 23;
    }
    int rate = success_rate->rates[0] + rate_variation;
    if (rate > 100) {
        rate = 100;
    }

    if (peer_user_ && peer_user_->getCurCharacR()) {
        PacketGuard packet;
        packet.put_header(0, 156);
        packet.put_byte(1);
        packet.put_short(rate);
        packet.put_byte(1);
        packet.put_int(recipe_item_->get_index());
        packet.finalize(true);
        peer_user_->Send(packet);
    }
    return 0;
}

// IDA_EA: 0x849DCC4
// IDA_NAME: _ZN10expert_job10CEnchanter11_TryEnchantEP5CUserP11stItemPos_t21ENUM_TRY_ENCHANT_TYPE
// DEMANGLED: expert_job::CEnchanter::_TryEnchant(
//             CUser*,stItemPos_t*,ENUM_TRY_ENCHANT_TYPE)
int CEnchanter::_TryEnchant(CUser* user,
                            stItemPos_t* item_position,
                            ENUM_TRY_ENCHANT_TYPE try_type) {
    int error = 0;
    switch (static_cast<int>(try_type)) {
    case 1:
        error = _TryEnchantOther(user, item_position);
        if (error) {
            materials_[1].Reset();
            recipe_item_ = 0;
        }
        break;
    case 2:
        OnCloseMission(user, static_cast<ENUM_CLOSE_MISSION_TYPE>(2));
        break;
    case 0:
        error = _TryEnchantMine(user, item_position);
        OnCloseMission(user, static_cast<ENUM_CLOSE_MISSION_TYPE>(0));
        break;
    }
    return error;
}

// IDA_EA: 0x849DD94
// IDA_NAME: _ZN10expert_job10CEnchanter13_AgreeEnchantEP5CUser23ENUM_AGREE_ENCHANT_TYPE
// DEMANGLED: expert_job::CEnchanter::_AgreeEnchant(
//             CUser*,ENUM_AGREE_ENCHANT_TYPE)
int CEnchanter::_AgreeEnchant(CUser* user,
                              ENUM_AGREE_ENCHANT_TYPE agree_type) {
    if (static_cast<uint16_t>(user->get_unique_id()) !=
        static_cast<uint16_t>(peer_user_->get_unique_id())) {
        return 3;
    }

    switch (static_cast<int>(agree_type)) {
    case 1: {
        const int error =
            _DoEnchant(static_cast<ENUM_TRY_ENCHANT_TYPE>(1));
        OnCloseMission(user, static_cast<ENUM_CLOSE_MISSION_TYPE>(0));
        return error;
    }
    case 2:
        OnCloseMission(user, static_cast<ENUM_CLOSE_MISSION_TYPE>(2));
        break;
    case 0:
        if (materials_[1].Empty()) {
            return 21;
        }
        materials_[1].Reset();
        recipe_item_ = 0;
        if (mission_user_ && mission_user_->getCurCharacR()) {
            PacketGuard packet;
            packet.put_header(1, ENUM_CMDPACKET_TRY_ENCHANT);
            packet.put_byte(1);
            packet.put_byte(3);
            packet.finalize(true);
            mission_user_->Send(packet);
        }
        break;
    }
    return 0;
}

// IDA_EA: 0x849F30C
// IDA_NAME: _ZN10expert_job10CEnchanter14IsAvailableJobEv
// DEMANGLED: expert_job::CEnchanter::IsAvailableJob(void)
int CEnchanter::IsAvailableJob() {
    return 1;
}

// IDA_EA: 0x849F316
// IDA_NAME: _ZN10expert_job10CEnchanter18SetPeerRequestTypeE22ENUM_PEER_REQUEST_TYPE
// DEMANGLED: expert_job::CEnchanter::SetPeerRequestType(
//             ENUM_PEER_REQUEST_TYPE)
void CEnchanter::SetPeerRequestType(
    ENUM_PEER_REQUEST_TYPE request_type) {
    peer_request_type_ = request_type;
}

// IDA_EA: 0x849F324
// IDA_NAME: _ZN10expert_job10CEnchanter25IsUseOtherUserMonsterCardEv
// DEMANGLED: expert_job::CEnchanter::IsUseOtherUserMonsterCard(void)
bool CEnchanter::IsUseOtherUserMonsterCard() {
    const int request_type = static_cast<int>(peer_request_type_);
    return request_type == 7 || request_type == 8;
}

// IDA_EA: 0x849ED1A
// IDA_NAME: _ZN10expert_job13CExpertJobMgr15OnEnchantByBeadEP5CUseriiii
// DEMANGLED: expert_job::CExpertJobMgr::OnEnchantByBead(CUser *,int,int,int,int)
int CExpertJobMgr::OnEnchantByBead(CUser* user,
                                   int source_item_space,
                                   int bead_slot,
                                   int target_item_space,
                                   int target_slot) {
    Inven_Item* const bead = user->getCurCharacInvenW()->GetInvenRef(
        static_cast<INVEN_TYPE>(1), bead_slot);
    Inven_Item* const target = user->getCurCharacInvenW()->GetInvenRef(
        static_cast<INVEN_TYPE>(1), target_slot);
    if (!bead || !target) return 17;

    if (GlobalData::s_rentalSystem->CheckRentableItem(1, target->item_id_) ||
        GlobalData::s_rentalSystem->CheckRentableItem(2, target->item_id_)) {
        return 17;
    }

    CItem* const bead_item = G_CDataManager()->find_item(bead->item_id_);
    CItem* const target_item = G_CDataManager()->find_item(target->item_id_);
    if (!bead_item || !target_item || !bead_item->is_stackable()) return 17;

    CItem* const monster_card = G_CDataManager()->find_item(
        static_cast<CStackableItem*>(bead_item)->GetMonsterCardId());
    if (!monster_card || !EnableWorld(target_item, bead_item)) return 17;
    if (static_cast<CStackableItem*>(monster_card)->getUsableItemType(
            target_item->vtable_->get_item_type(target_item)) != 1) {
        return 23;
    }

    if (user->getCurCharacInvenW()->delete_item(
            static_cast<INVEN_TYPE>(1),
            bead_slot,
            1,
            static_cast<eItemDelReason>(8),
            true) != 1) {
        cMyTrace trace(
            "int expert_job::CExpertJobMgr::OnEnchantByBead(CUser*, int, "
            "int, int, int)",
            1741,
            5);
        trace("CExpertJobMgr::OnEnchantByBead, "
              "getCurCharacInvenW()->delete_item failed , %d %d",
              user->getCurCharacNo(),
              bead_slot);
        return 17;
    }

    const int previous_enchant = target->extend_info_;
    target->extend_info_ = monster_card->get_index();

    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 14);
    packet.put_byte(0);
    packet.put_short(2);
    user->getCurCharacInvenR()->MakeItemPacket(
        static_cast<INVEN_TYPE>(1), target_slot, packet);
    user->getCurCharacInvenR()->MakeItemPacket(
        static_cast<INVEN_TYPE>(1), bead_slot, packet);
    packet.finalize(true);
    user->Send(packet);

    GetInstanceExpertJobStatistic()->increaseItemCount(
        static_cast<ENUM_EXPERT_JOB_TYPE>(1),
        static_cast<unsigned long>(monster_card->get_index()),
        1);
    user->history_log().EnchantItem(target->item_id_,
                                    previous_enchant,
                                    target->extend_info_,
                                    true,
                                    user->get_acc_name(),
                                    user->getCurCharacName());

    packet.clear();
    packet.put_header(1, ENUM_CMDPACKET_ENCHANT_BY_BEAD);
    packet.put_byte(1);
    packet.finalize(true);
    user->Send(packet);
    (void)source_item_space;
    (void)target_item_space;
    return 0;
}

}  // namespace expert_job
