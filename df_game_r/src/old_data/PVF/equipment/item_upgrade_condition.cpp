#include "old_data/PVF/equipment/item_upgrade.h"

#include "common/config.h"
#include <stdint.h>

#include "old_game/progression/career/disjoint_abi.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/activity/schedule/reduce_upgrade_item_pay.h"
#include "old_game/economy/equipment/upgrade_room.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"

namespace WongWork {

// IDA_EA: 0x8546342
// IDA_NAME: _ZN8WongWork12CItemUpgrade15_CheckConditionEPK5CUserRK10Inven_ItemPK5CItemS6_RK15upgrade_table_tRK14upgrade_info_t
// DEMANGLED: WongWork::CItemUpgrade::_CheckCondition(CUser const*,Inven_Item const&,CItem const*,Inven_Item const&,upgrade_table_t const&,upgrade_info_t const&)
int CItemUpgrade::_CheckCondition(const CUser* user,
                                  const Inven_Item& target,
                                  const CItem* item,
                                  const Inven_Item& material,
                                  const upgrade_table_t& table,
                                  const upgrade_info_t& info) {
    if (target.amplify_option_.isIdentified() != 1) {
        return 174;
    }
    if (static_cast<int32_t>(material.item_id_) !=
        info.event_material_item_index) {
        return 22;
    }
    if (material.add_info_ < info.event_material_item_count) {
        return 22;
    }

    int money = 0;
    CEventBase* const event =
        GlobalData::s_event_manager->GetRepeatEvent(50);
    if (event->IsEventing(0) &&
        !target.amplify_option_.hasAbility()) {
        const int reduce_rate =
            static_cast<CReduceUpgradeItemPay*>(
                GlobalData::s_event_manager->GetRepeatEvent(50))
                ->GetReduceRate();
        if (reduce_rate > 0) {
            money = info.calculated_cost -
                    reduce_rate * info.calculated_cost / 100;
            if (money < 0) {
                money = 0;
            }
        }
    } else {
        money = info.calculated_cost;
    }

    CInventory* const inventory = user->getCurCharacInvenR();
    const Inven_Item equipped = inventory->GetInvenSlot(
        0, oldsrv::config::kEquipmentTypeTitle);
    CItem* const equipped_item = G_CDataManager()->find_item(
        static_cast<int>(equipped.item_id_));
    if (equipped_item) {
        const float current_money = static_cast<float>(money);
        money = static_cast<int>(
            static_cast<CEquipItem*>(equipped_item)
                    ->getUpgradeCostDiscount() *
                current_money / -100.0f +
            current_money);
    }

    if (upgrade_room::EffectiveMoney(
            user, user->getCurCharacInvenR()->get_money()) < money) {
        return 10;
    }
    if (table.upgrade_infos.size() <=
        static_cast<uint8_t>(target.GetUpgrade())) {
        return 13;
    }
    if (target.item_category_ != 1u) {
        return 19;
    }
    if (item->vtable_->get_item_type(item) <
            oldsrv::config::kOrdinaryEquipmentTypeFirst ||
        item->vtable_->get_item_type(item) >
            oldsrv::config::kOrdinaryEquipmentTypeLast) {
        return 19;
    }
    if (item->vtable_->get_item_type(item) ==
        oldsrv::config::kEquipmentTypeTitle) {
        return 19;
    }
    if (static_cast<int>(target.avatar_select_) !=
        static_cast<const CEquipItem*>(item)->get_endurance()) {
        return 7;
    }

    if (table.max_upgrade_levels_by_rarity[item->get_rarity()] != -1 &&
        static_cast<uint8_t>(target.GetUpgrade()) >=
            table.max_upgrade_levels_by_rarity[item->get_rarity()]) {
        return 95;
    }

    if (table.GetPenaltyType(
            static_cast<uint8_t>(target.GetUpgrade()),
            static_cast<ENUM_RARITY>(item->get_rarity()),
            &info) == 3) {
        unsigned int required_slots = static_cast<unsigned int>(
            G_CDataManager()
                ->get_disjoint()
                ->getPredictResultItemTypeCount(item->get_rarity()) +
            1);
        if (target.amplify_option_.hasAbility()) {
            ++required_slots;
        }
        if (required_slots != 0u) {
            const int inventory_empty =
                user->getCurCharacInvenR()->InvenEmptycount(
                    static_cast<Inven_Item::ITEM_TYPE>(3));
            const int quick_empty =
                user->getCurCharacInvenR()->QuickEmptyCount();
            if (static_cast<unsigned int>(inventory_empty + quick_empty) <
                required_slots) {
                return 21;
            }
        }
    }

    const ItemUpgradeType::T upgrade_type =
        static_cast<ItemUpgradeType::T>(
            target.amplify_option_.hasAbility() ? 2 : 1);
    CEquipItem* const equipment =
        const_cast<CEquipItem*>(
            static_cast<const CEquipItem*>(item));
    return equipment->isUpgradableLevel(
               upgrade_type, user->get_charac_level())
               ? 0
               : 19;
}

}  // namespace WongWork
