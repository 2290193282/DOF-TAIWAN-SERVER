#include "old_data/PVF/equipment/item_upgrade.h"

#include <algorithm>
#include <cmath>
#include <stdint.h>
#include <utility>
#include <vector>

#include "old_game/progression/career/disjoint_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/statistics_runtime.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "old_data/PVF/event/event_upgrade_jar_script.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/shop/npc_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

namespace {

bool IsApplicableProtectionTicket(const UpgradeItemTicketMap& tickets,
                                  int item_id,
                                  unsigned int upgrade,
                                  uint8_t upgrade_mode) {
    const UpgradeItemTicketMap::const_iterator found = tickets.find(item_id);
    if (found == tickets.end()) {
        return false;
    }

    return IsUpgradeProtectionPolicyApplicable(
        found->second, upgrade, upgrade_mode, true);
}

int FindApplicableProtectionTicketSlot(CUser* user,
                                       const UpgradeItemTicketMap& tickets,
                                       unsigned int upgrade,
                                       uint8_t upgrade_mode) {
    CInventory* const inventory = user->getCurCharacInvenR();
    for (UpgradeItemTicketMap::const_iterator it = tickets.begin();
         it != tickets.end(); ++it) {
        if (!IsApplicableProtectionTicket(
                tickets,
                static_cast<int>(it->first),
                upgrade,
                upgrade_mode)) {
            continue;
        }

        const int slot =
            inventory->check_item_exist(static_cast<int>(it->first));
        if (slot != -1) {
            return slot;
        }
    }
    return -1;
}

}  // namespace

namespace WongWork {

// IDA_EA: 0x8546938
// IDA_NAME: _ZN8WongWork12CItemUpgrade26_getAdditionalDisjointItemEjPK5CItemRSt6vectorISt4pairIiiESaIS6_EERK15upgrade_table_t
// DEMANGLED: WongWork::CItemUpgrade::_getAdditionalDisjointItem(unsigned int,CItem const*,std::vector<std::pair<int,int>> &,upgrade_table_t const&)
void CItemUpgrade::_getAdditionalDisjointItem(
    unsigned int upgrade,
    const CItem* item,
    std::vector<std::pair<int, int> >& result_items,
    const upgrade_table_t& table) {
    const int corrected_grade = std::max(
        0,
        item->get_grade() -
            table.disjoint_correction_grades[item->get_rarity()]);
    if (static_cast<unsigned int>(corrected_grade) <
        static_cast<unsigned int>(table.disjoint_equip_level_const)) {
        return;
    }
    if (static_cast<unsigned int>(table.disjoint_bonus_count_const) >=
        upgrade) {
        return;
    }

    const unsigned int delta =
        upgrade -
        static_cast<unsigned int>(table.disjoint_bonus_count_const);
    const int squared_delta =
        static_cast<int>(std::pow(static_cast<double>(delta), 2.0));
    const float weight =
        table.disjoint_failed_bonus_weights[item->get_rarity()];
    const int count = static_cast<int>(
        static_cast<long double>(squared_delta) * weight);
    if (count != 0) {
        result_items.push_back(
            std::make_pair(table.disjoint_bonus_item_index, count));
    }
}

// IDA_EA: 0x8546A96
// IDA_NAME: _ZN8WongWork12CItemUpgrade26_getAdditionalDisjointItemEjPK5CItemRSt6vectorISt4pairIiiESaIS6_EERK15upgrade_table_tb
// DEMANGLED: WongWork::CItemUpgrade::_getAdditionalDisjointItem(unsigned int,CItem const*,std::vector<std::pair<int,int>> &,upgrade_table_t const&,bool)
void CItemUpgrade::_getAdditionalDisjointItem(
    unsigned int upgrade,
    const CItem* item,
    std::vector<std::pair<int, int> >& result_items,
    const upgrade_table_t& table,
    bool has_ability) {
    if (upgrade >= table.destruction_result.result_items.size()) {
        return;
    }

    const STResultItemCnt& result =
        table.destruction_result.result_items.at(upgrade);
    if (result.item_index != upgrade) {
        return;
    }

    const int count = has_ability ? result.item_grade : result.item_count;
    if (count != 0) {
        result_items.push_back(std::make_pair(
            table.destruction_result.result_count, count));
    }
}

// IDA_EA: 0x8546BBA
// IDA_NAME: _ZN8WongWork12CItemUpgrade12_ProcPenaltyEP5CUserP10Inven_ItemPK5CItemtRK15upgrade_table_tPK14upgrade_info_tP11PacketGuardS4_c
// DEMANGLED: WongWork::CItemUpgrade::_ProcPenalty(CUser *,Inven_Item *,CItem const*,unsigned short,upgrade_table_t const&,upgrade_info_t const*,PacketGuard *,Inven_Item *,char)
int CItemUpgrade::_ProcPenalty(CUser* user,
                               Inven_Item* target,
                               const CItem* item,
                               uint16_t target_slot,
                               const upgrade_table_t& table,
                               const upgrade_info_t* info,
                               PacketGuard* packet,
                               Inven_Item* alternative_material,
                               char forced_penalty) {
    int penalty_type = 0;
    if (alternative_material->item_id_ != 0u) {
        penalty_type = forced_penalty ? 2 : 1;
    } else {
        penalty_type = table.GetPenaltyType(
            static_cast<uint8_t>(target->GetUpgrade()),
            static_cast<ENUM_RARITY>(item->get_rarity()),
            info);
        if (GlobalData::s_pEvent_UpGrade_Jar->isFortune()) {
            penalty_type = 2;
        }
    }

    packet->put_byte(static_cast<char>(penalty_type));
    if (penalty_type == 1) {
        packet->put_byte(static_cast<uint8_t>(target->GetUpgrade()));
        packet->put_short(target_slot);
        return 0;
    }

    if (penalty_type == 2) {
        if (GlobalData::s_pEvent_UpGrade_Jar->isFortune()) {
            const_cast<upgrade_info_t*>(info)->parsed_values_08[2] = 1;
        }

        const int decrease = info->parsed_values_08[2];
        if (decrease >= static_cast<uint8_t>(target->GetUpgrade()) ||
            forced_penalty < 0) {
            target->SetUpgrade(0);
        } else if (forced_penalty > 0) {
            target->SetUpgrade(static_cast<uint8_t>(
                target->GetUpgrade() - forced_penalty));
        } else {
            target->SetUpgrade(static_cast<uint8_t>(
                target->GetUpgrade() - decrease));
        }

        packet->put_byte(static_cast<uint8_t>(target->GetUpgrade()));
        packet->put_short(target_slot);
        return 0;
    }

    if (penalty_type != 3) {
        return 0;
    }

    const UpgradeItemTicketMap& protection_tickets =
        G_CDataManager()
            ->get_server_parameter_script()
            ->get_upgrade_item_tickets();
    const unsigned int failed_upgrade =
        static_cast<uint8_t>(target->GetUpgrade());
    const uint8_t upgrade_mode =
        target->amplify_option_.hasAbility() ? 1u : 0u;
    const int protection_slot = FindApplicableProtectionTicketSlot(
        user, protection_tickets, failed_upgrade, upgrade_mode);

    if (protection_slot != -1) {
        const Inven_Item protection =
            user->getCurCharacInvenW()->GetInvenSlot(1, protection_slot);
        const int protection_item_id =
            static_cast<int32_t>(protection.item_id_);
        if (!IsApplicableProtectionTicket(
                protection_tickets,
                protection_item_id,
                failed_upgrade,
                upgrade_mode)) {
            return 17;
        }
        if (static_cast<uint8_t>(
                user->getCurCharacInvenW()->delete_item(
                    static_cast<INVEN_TYPE>(1),
                    protection_slot,
                    1,
                    static_cast<eItemDelReason>(3),
                    true)) != 1u) {
            return 17;
        }

        user->history_log().apply_Item(
            protection_item_id,
            static_cast<int32_t>(target->item_id_),
            static_cast<eApplyItemReason>(0));
        user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(0),
                             protection_slot);
        target->SetUpgrade(0);
        user->getCurCharacInvenW()->update_item(
            static_cast<INVEN_TYPE>(1), target_slot, *target);
        user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(0),
                             target_slot);
        cMyTrace trace("UpgradeProtection0828", __LINE__, 5);
        trace("[UPGRADE_PROTECTION][APPLIED] account=%d charac=%d "
              "mode=%u upgrade=%u ticket=%d target=%u slot=%d",
              user->get_acc_id(),
              user->get_charac_no(-1),
              static_cast<unsigned int>(upgrade_mode),
              failed_upgrade,
              protection_item_id,
              static_cast<unsigned int>(target->item_id_),
              protection_slot);
        return 216;
    }

    cMyTrace trace("UpgradeProtection0828", __LINE__, 5);
    trace("[UPGRADE_PROTECTION][NO_APPLICABLE_TICKET] account=%d "
          "charac=%d mode=%u upgrade=%u configured=%u target=%u",
          user->get_acc_id(),
          user->get_charac_no(-1),
          static_cast<unsigned int>(upgrade_mode),
          failed_upgrade,
          static_cast<unsigned int>(protection_tickets.size()),
          static_cast<unsigned int>(target->item_id_));

    if (static_cast<uint8_t>(
            user->getCurCharacInvenW()->delete_item(
                static_cast<INVEN_TYPE>(1),
                target_slot,
                1,
                static_cast<eItemDelReason>(13),
                true)) != 1u) {
        return 17;
    }

    const unsigned int old_upgrade =
        static_cast<uint8_t>(target->GetUpgrade());
    target->SetUpgrade(0);
    packet->put_byte(0);
    packet->put_short(target_slot);

    DisJointResult disjoint_result;
    const int target_item_id =
        static_cast<int32_t>(target->item_id_);
    if (G_CDataManager()->get_disjoint()->GetResult(
            target_item_id, disjoint_result)) {
        packet->put_byte(0);
        return 0;
    }
    target->item_id_ = 0u;

    std::vector<std::pair<int, int> > result_items;
    result_items.reserve(6u);
    for (int index = 0; index <= 4; ++index) {
        if (disjoint_result.values[index + 5] != 0) {
            result_items.push_back(
                std::make_pair(disjoint_result.values[index + 5],
                               disjoint_result.values[index]));
        }
    }

    _getAdditionalDisjointItem(
        old_upgrade, item, result_items, table);
    if (target->amplify_option_.hasAbility()) {
        _getAdditionalDisjointItem(
            old_upgrade,
            item,
            result_items,
            *CItemUpgradeTable::getUpgradeInfoTable());
    }

    std::vector<int> inserted_slots;
    const int inserted =
        user->getCurCharacInvenW()->insert_event_items(
            result_items,
            inserted_slots,
            static_cast<eMoneyAddReason>(7),
            static_cast<eItemAddReason>(19),
            false,
            "",
            "");
    packet->put_byte(inserted);

    std::vector<int>::const_iterator inserted_item =
        inserted_slots.begin();
    while (inserted_item != inserted_slots.end()) {
        const int slot = *inserted_item++;
        const unsigned int item_id =
            static_cast<unsigned int>(*inserted_item++);
        const int count = *inserted_item++;
        packet->put_short(slot);
        packet->put_item_idx(item_id);
        packet->put_int(count);

        CItem* const result_item =
            G_CDataManager()->find_item(static_cast<int>(item_id));
        if (result_item) {
            Inven_Item generated;
            generated.item_id_ = item_id;
            generated.set_add_info(count);
            result_item->vtable_->make_item(result_item, generated);
            GetInstanceCubeStatistic()->collectCubeStatistics(
                result_item,
                count,
                user,
                static_cast<CUBE_STATISTIC_FIELD>(1));

            int sell_price = 0;
            G_Store()->GetSellItemPrice(
                generated,
                result_item,
                static_cast<short>(generated.add_info_),
                false,
                sell_price);
            GetInstanceValueStatistic()->AddValueStatistic(
                static_cast<VALUE_STATISTIC_FIELD>(9),
                user,
                static_cast<unsigned int>(sell_price));
        }
    }
    return 0;
}

}  // namespace WongWork
