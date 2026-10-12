#include "old_data/PVF/equipment/item_upgrade.h"

#include <utility>
#include <vector>

#include "common/manager/server_utility.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/etc/equipment_grade_sample.h"
#include "old_data/PVF/etc/equipment_growth.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"

namespace WongWork {
namespace {

bool ResolveEmancipatePreservation(int ticket_mode, bool* preserve) {
    if (!preserve) return false;
    switch (ticket_mode) {
        case 0:  // legacy lower ticket
        case 2:  // 0828 lower growth ticket
            *preserve = false;
            return true;
        case 1:  // legacy upper ticket
        case 3:  // 0828 upper growth ticket
            *preserve = true;
            return true;
        default:
            return false;
    }
}

bool IsEmancipateGrowTargetAllowed(const CStackableItem* ticket,
                                   int target_item_id) {
    if (!ticket) return false;
    const std::vector<StackableEmancipateGrowGroup>& groups =
        ticket->emancipate_grow_groups();
    if (groups.empty()) {
        return true;
    }

    equipment_growth::PrototypeIdentity identity;
    if (!equipment_growth::FindPrototype(target_item_id, &identity)) {
        return false;
    }

    bool allowed = false;
    for (std::vector<StackableEmancipateGrowGroup>::const_iterator group =
             groups.begin();
         group != groups.end(); ++group) {
        // The two official 0828 tickets use an exact positive declaration:
        // [group] 1 4 3 4 5 6.  The client parser accepts -1/0 sentinels, but
        // no server-side meaning for those values has been recovered, so the
        // transaction fails closed instead of treating them as a wildcard.
        if (group->declared_count <= 0 ||
            static_cast<size_t>(group->declared_count) !=
                group->grow_types.size()) {
            return false;
        }
        if (group->group != static_cast<int>(identity.type)) {
            continue;
        }
        for (std::vector<int>::const_iterator stage =
                 group->grow_types.begin();
             stage != group->grow_types.end(); ++stage) {
            if (*stage == static_cast<int>(identity.stage)) {
                allowed = true;
                break;
            }
        }
    }
    return allowed;
}

}  // namespace

// IDA_EA: 0x85491D6
// IDA_NAME: _ZN8WongWork12CItemUpgrade20ProcUpgradeChronicleEP5CUsersjsjcRSt6vectorIiSaIiEE
// DEMANGLED: WongWork::CItemUpgrade::ProcUpgradeChronicle(CUser *,short,unsigned int,short,unsigned int,char,std::vector<int,std::allocator<int>> &)
int CItemUpgrade::ProcUpgradeChronicle(CUser* user,
                                       int16_t material_slot,
                                       uint32_t material_item_id,
                                       int16_t target_slot,
                                       uint32_t target_item_id,
                                       uint8_t grade_sample,
                                       std::vector<int>& changed_slots) {
    (void)material_item_id;
    (void)target_item_id;
    CInventory* const write_inventory = user->getCurCharacInvenW();
    const Inven_Item material = write_inventory->GetInvenSlot(
        static_cast<INVEN_TYPE>(1), material_slot);
    const Inven_Item target = write_inventory->GetInvenSlot(
        static_cast<INVEN_TYPE>(1), target_slot);
    CStackableItem* const material_prototype =
        static_cast<CStackableItem*>(G_CDataManager()->find_item(
            static_cast<int>(material.item_id_)));
    CEquipItem* const target_prototype =
        static_cast<CEquipItem*>(G_CDataManager()->find_item(
            static_cast<int>(target.item_id_)));

    const int emancipate = material_prototype->GetEmancipate();
    bool preserve_condition = false;
    if (!ResolveEmancipatePreservation(emancipate,
                                       &preserve_condition) ||
        !IsEmancipateGrowTargetAllowed(material_prototype,
                                       target.item_id_)) {
        return 17;
    }

    // The dispatcher has already checked both IDs and concrete prototype
    // kinds.  This body deliberately starts at the same direct PVF list
    // access as 0x85491D6 instead of introducing another validation layer.
    const std::vector<std::pair<int, int> >& required_items =
        *reinterpret_cast<const std::vector<std::pair<int, int> >*>(
            target_prototype->GetEmancipateMaterialList());
    if (user->getCurCharacInvenR()->CheckNeedItemList(required_items) != 1) {
        return 22;
    }

    CInventory trial_inventory;
    InventoryMemory trial_memory;
    trial_inventory.SetInventoryMemory(&trial_memory);
    trial_inventory.setCopy(*user->getCurCharacInvenRefR());
    if (trial_inventory.tryDeleteEventItems(required_items) != 1) {
        return 17;
    }

    bool equipment_changed = false;
    write_inventory->delete_event_items(
        required_items,
        changed_slots,
        equipment_changed,
        static_cast<eMoneySubReason>(12),
        static_cast<eItemDelReason>(12),
        false);

    int sampled_grade = 0;
    const unsigned int random_index = get_rand_int(100);
    if (random_index < 100u && grade_sample <= 4u) {
        // The old body indexes the imported vector directly.  The PVF parser
        // supplies the 100 entries per grade required by this transaction.
        sampled_grade = static_cast<int>(
            G_CDataManager()->get_equipment_grade_sample()
                ->grades[grade_sample][random_index]);
    }

    stEquipConditionForChronicle preserved;
    int result = 0;
    if (preserve_condition &&
        (result = user->getCurCharacInvenR()->CheckEquipmentItemCondition(
             static_cast<INVEN_TYPE>(1), target_slot, preserved)) != 0) {
        return result;
    }

    const std::vector<std::pair<int, int> >& output_items =
        *reinterpret_cast<const std::vector<std::pair<int, int> >*>(
            target_prototype->GetEmancipateResultList());
    if (output_items.size() > 1u) {
        return 17;
    }
    const std::pair<int, int>& output = *output_items.begin();
    if (output.second > 1 || write_inventory->GetItemType(output.first) != 1) {
        return 17;
    }

    Inven_Item generated;
    generated.item_id_ = output.first;
    target_prototype->vtable_->make_item(target_prototype, generated);
    if (preserve_condition) {
        if (preserved.upgrade <= 30u) {
            generated.SetUpgrade(preserved.upgrade);
        }
        if (preserved.has_ability != 0 && preserved.ability_type != 0) {
            generated.amplify_option_.assign(preserved.ability_type,
                                             preserved.ability_value);
        }
        generated.extend_info_ = preserved.extend_info;
    }
    generated.set_add_info(sampled_grade);

    write_inventory->delete_item(static_cast<INVEN_TYPE>(1), target_slot, 1,
                                 static_cast<eItemDelReason>(12), true);
    user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                         static_cast<ENUM_ITEMSPACE>(0), target_slot);
    write_inventory->delete_item(static_cast<INVEN_TYPE>(1), material_slot, 1,
                                 static_cast<eItemDelReason>(12), true);
    write_inventory->insert_item_special_slot(
        generated,
        1,
        target_slot,
        static_cast<eMoneyAddReason>(7),
        static_cast<eItemAddReason>(9));
    user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                         static_cast<ENUM_ITEMSPACE>(0), target_slot);
    return 0;
}

}  // namespace WongWork
