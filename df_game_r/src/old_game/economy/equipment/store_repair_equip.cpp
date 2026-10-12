#include "old_game/player/core/cuser.h"
#include "common/config.h"
#include "old_game/economy/inventory/inventory.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/equipment/item_amplifier_abi.h"
#include "old_game/economy/equipment/item_option.h"
#include "common/manager/user.h"
#include "old_game/economy/shop/npc_runtime.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/statistics_runtime.h"
#include "old_game/dungeon/tower/death_tower_value_statistic_runtime.h"

#include <stdint.h>

#include <vector>


struct tagRepairItemInfo {
    int inventory_type;
    int slot;
    CEquipItem* item;
};

extern int getEquipmentRepairPrice(int growth_repair_cost,
                                   int current_endurance,
                                   int maximum_endurance,
                                   int growth_grade,
                                   bool special_repair,
                                   float upgrade_rate)
    __asm__("_Z23getEquipmentRepairPriceiiiibf");

// IDA_EA: 0x8619CEC
// IDA_NAME: _ZN5Store12repair_equipEP5CUsercstRt
// DEMANGLED: Store::repair_equip(CUser *,char,short,unsigned short,unsigned short &)
int Store::repair_equip(CUser* user,
                        char inven_type,
                        short slot,
                        unsigned short repair_ticket_slot,
                        unsigned short& repair_cost) {

    if (static_cast<unsigned char>(user->CheckInTrade())) {
        return 19;
    }

    int total_repair_cost = 0;
    if (slot != -1) {
        Inven_Item target_item;
        if (inven_type == 2) {
            target_item = user->getCurCharacCargoR()->get_cargo_slot(slot);
        } else if (inven_type == 3) {
            target_item = reinterpret_cast<const CInventory*>(
                user->getCurCharacInvenR())->GetInvenSlot(
                    0, slot);
        } else {
            if (inven_type != 0) {
                return 17;
            }
            target_item = reinterpret_cast<const CInventory*>(
                user->getCurCharacInvenR())->GetInvenSlot(
                    1, slot);
        }

        if (target_item.item_id_ == 0 || target_item.item_category_ != 1) {
            return 17;
        }

        repair_cost = target_item.avatar_select_;
        CEquipItem* equip_item = reinterpret_cast<CEquipItem*>(
            G_CDataManager()->find_item(
                target_item.item_id_));
        if (!equip_item || !equip_item->get_endurance()) {
            return 17;
        }

        Inven_Item repair_ticket;
        if (repair_ticket_slot != 0xFFFFu) {
            repair_ticket = reinterpret_cast<const CInventory*>(
                user->getCurCharacInvenR())->GetInvenSlot(
                    1, repair_ticket_slot);
            if ((repair_ticket.item_id_ == 0 &&
                 repair_ticket.item_attr_ == 0 &&
                 (static_cast<uint32_t>(repair_ticket.add_info_) & 0xffu) ==
                     0u) ||
                repair_ticket.item_category_ != 2) {
                return 21;
            }
            const uint32_t repair_ticket_id =
                static_cast<uint32_t>(repair_ticket.item_id_);
            if (repair_ticket_id != 8055u && repair_ticket_id != 8223u) {
                return 21;
            }
        }

        const int charac_level = user->get_charac_level();
        const int growth_grade =
            equip_item->getGrowthGrade(charac_level);
        const int growth_repair_cost =
            equip_item->getGrowthRepairCost(charac_level);
        const bool has_amplify_ability =
            target_item.amplify_option_.hasAbility();
        const int upgrade = target_item.GetUpgrade();
        const float upgrade_repair_rate =
            (G_CDataManager())->getUpgradeItemRepairCostRate(upgrade, has_amplify_ability);
        total_repair_cost = getEquipmentRepairPrice(
            growth_repair_cost,
            target_item.avatar_select_,
            equip_item->get_endurance(),
            growth_grade,
            false,
            upgrade_repair_rate);

        if (repair_ticket_slot != 0xFFFFu) {
            const uint32_t repair_ticket_id =
                static_cast<uint32_t>(repair_ticket.item_id_);
            if (repair_ticket_id == 8055u || repair_ticket_id == 8223u) {
                if (static_cast<CInventory*>(user->getCurCharacInvenW())
                        ->use_item(repair_ticket_slot, 1, 0)) {
                    return 22;
                }
                user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                                     static_cast<ENUM_ITEMSPACE>(0),
                                     repair_ticket_slot);
                total_repair_cost = 0;
            }
        }

        if (reinterpret_cast<CInventory*>(
                user->getCurCharacInvenW())
                ->use_money(total_repair_cost,
                            static_cast<eMoneySubReason>(1),
                            true) != 1) {
            return 10;
        }

        GetInstanceValueStatistic()->AddValueStatistic(
            static_cast<VALUE_STATISTIC_FIELD>(0x14),
            user,
            static_cast<unsigned int>(total_repair_cost));
        if (user->get_state() == 10) {
            WongWork::CDeathTower* death_tower = user->getDeathTower();
            if (death_tower) {
                const CDungeon* dungeon =
                    death_tower->getCDungeonMGr()->getDungeon();
                GetInstanceDeathTowerValueStatistic()->SendDeathTowerValueStatistic(
                    static_cast<DEATHTOWER_VALUE_STATISTIC_FIELD>(9),
                    static_cast<char>(dungeon->get_index() != 11000),
                    static_cast<char>(user->get_charac_level()),
                    static_cast<unsigned int>(total_repair_cost));
            }
        }

        target_item.avatar_select_ =
            static_cast<uint16_t>(equip_item->get_endurance());
        switch (inven_type) {
        case 2:
            static_cast<CCargo*>(user->getCurCharacCargoW())
                ->update_item(slot, target_item);
            user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                                 static_cast<ENUM_ITEMSPACE>(2),
                                 slot);
            break;
        case 3:
            static_cast<CInventory*>(user->getCurCharacInvenW())
                ->update_item(static_cast<INVEN_TYPE>(0), slot, target_item);
            user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                                 static_cast<ENUM_ITEMSPACE>(3),
                                 slot);
            break;
        case 0:
            static_cast<CInventory*>(user->getCurCharacInvenW())
                ->update_item(static_cast<INVEN_TYPE>(1), slot, target_item);
            user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                                 static_cast<ENUM_ITEMSPACE>(0),
                                 slot);
            break;
        }
        return 0;
    }

    std::vector<tagRepairItemInfo> repair_items;
    for (int equip_slot = 3; equip_slot <= 8; ++equip_slot) {
        Inven_Item item;
        item = reinterpret_cast<const CInventory*>(
            user->getCurCharacInvenR())->GetInvenSlot(
                1, equip_slot);
        if ((item.item_id_ != 0 || item.item_attr_ != 0 ||
             (static_cast<uint32_t>(item.add_info_) & 0xffu) != 0u) &&
            item.item_category_ == 1) {
            CEquipItem* equip_item = reinterpret_cast<CEquipItem*>(
                G_CDataManager()->find_item(
                    item.item_id_));
            if (equip_item) {
                const int charac_level = user->get_charac_level();
                const int growth_grade =
                    equip_item->getGrowthGrade(charac_level);
                const int growth_repair_cost =
                    equip_item->getGrowthRepairCost(charac_level);
                const bool has_amplify_ability =
                    item.amplify_option_.hasAbility();
                const int upgrade = item.GetUpgrade();
                const float upgrade_repair_rate =
                    (G_CDataManager())->getUpgradeItemRepairCostRate(upgrade, has_amplify_ability);
                const int maximum_endurance = equip_item->get_endurance();
                total_repair_cost += getEquipmentRepairPrice(
                    growth_repair_cost,
                    item.avatar_select_,
                    maximum_endurance,
                    growth_grade,
                    false,
                    upgrade_repair_rate);
                if (maximum_endurance &&
                    item.avatar_select_ != maximum_endurance) {
                    tagRepairItemInfo repair_info = {
                        1, equip_slot, equip_item};
                    repair_items.push_back(repair_info);
                }
            }
        }
    }

    for (int equip_slot = oldsrv::config::kOrdinaryEquipmentTypeFirst;
         equip_slot <= oldsrv::config::kOrdinaryEquipmentTypeLast;
         ++equip_slot) {
        Inven_Item item;
        item = reinterpret_cast<const CInventory*>(
            user->getCurCharacInvenR())->GetInvenSlot(
                0, equip_slot);
        if ((item.item_id_ != 0 || item.item_attr_ != 0 ||
             (static_cast<uint32_t>(item.add_info_) & 0xffu) != 0u) &&
            item.item_category_ == 1) {
            CEquipItem* equip_item = reinterpret_cast<CEquipItem*>(
                G_CDataManager()->find_item(
                    item.item_id_));
            if (equip_item) {
                const int charac_level = user->get_charac_level();
                const int growth_grade =
                    equip_item->getGrowthGrade(charac_level);
                const int growth_repair_cost =
                    equip_item->getGrowthRepairCost(charac_level);
                const bool has_amplify_ability =
                    item.amplify_option_.hasAbility();
                const int upgrade = item.GetUpgrade();
                const float upgrade_repair_rate =
                    (G_CDataManager())->getUpgradeItemRepairCostRate(upgrade, has_amplify_ability);
                const int maximum_endurance = equip_item->get_endurance();
                total_repair_cost += getEquipmentRepairPrice(
                    growth_repair_cost,
                    item.avatar_select_,
                    maximum_endurance,
                    growth_grade,
                    false,
                    upgrade_repair_rate);
                if (maximum_endurance &&
                    item.avatar_select_ != maximum_endurance) {
                    tagRepairItemInfo repair_info = {
                        0, equip_slot, equip_item};
                    repair_items.push_back(repair_info);
                }
            }
        }
    }

    if (reinterpret_cast<CInventory*>(user->getCurCharacInvenW())
            ->use_money(total_repair_cost,
                        static_cast<eMoneySubReason>(1),
                        true) != 1) {
        return 10;
    }

    GetInstanceValueStatistic()->AddValueStatistic(
                                      static_cast<VALUE_STATISTIC_FIELD>(0x14),
                                      user,
                                      static_cast<unsigned int>(
                                          total_repair_cost));
    if (user->get_state() == 10) {
        WongWork::CDeathTower* death_tower = user->getDeathTower();
        if (death_tower) {
            const CDungeon* dungeon =
                death_tower->getCDungeonMGr()->getDungeon();
            GetInstanceDeathTowerValueStatistic()->SendDeathTowerValueStatistic(
                static_cast<DEATHTOWER_VALUE_STATISTIC_FIELD>(9),
                static_cast<char>(dungeon->get_index() != 11000),
                static_cast<char>(user->get_charac_level()),
                static_cast<unsigned int>(total_repair_cost));
        }
    }

    for (std::vector<tagRepairItemInfo>::const_iterator it =
             repair_items.begin();
         it != repair_items.end();
         ++it) {
        Inven_Item item;
        item = reinterpret_cast<const CInventory*>(
            user->getCurCharacInvenR())->GetInvenSlot(
                it->inventory_type, it->slot);
        if ((item.item_id_ != 0 || item.item_attr_ != 0 ||
             (static_cast<uint32_t>(item.add_info_) & 0xffu) != 0u) &&
            item.item_category_ == 1) {
            item.avatar_select_ =
                static_cast<uint16_t>(it->item->get_endurance());
            static_cast<CInventory*>(user->getCurCharacInvenW())
                ->update_item(static_cast<INVEN_TYPE>(it->inventory_type),
                              it->slot,
                              item);
        }
    }
    for (int equip_slot = oldsrv::config::kOrdinaryEquipmentTypeFirst;
         equip_slot <= oldsrv::config::kOrdinaryEquipmentTypeLast;
         ++equip_slot) {
        user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(3),
                             equip_slot);
    }
    return 0;
}
