#include "old_data/PVF/equipment/item_upgrade.h"

#include <stdint.h>

#include "old_game/player/core/action_point_packet_dispatchers.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "core/config/env/environment.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/equipment/item_amplifier_abi.h"
#include "common/manager/statistics_runtime.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/event/event_upgrade_jar_script.h"
#include "old_data/db/equipment_upgrade/db_req_item_upgrade_log.h"
#include "old_game/activity/schedule/reduce_upgrade_item_pay.h"
#include "old_game/activity/schedule/freely_give_item.h"
#include "old_game/economy/equipment/upgrade_room.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/shop/npc_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/mt_rand.h"

namespace WongWork {

// IDA_EA: 0x854779C
// IDA_NAME: _ZN8WongWork12CItemUpgrade11ProcUpgradeEP5CUserP16MSG_ITEM_UPGRADEP11PacketGuard
// DEMANGLED: WongWork::CItemUpgrade::ProcUpgrade(CUser *,MSG_ITEM_UPGRADE *,PacketGuard *)
int CItemUpgrade::ProcUpgrade(CUser* user,
                              MSG_ITEM_UPGRADE* message,
                              PacketGuard* packet) {
    CInventory* const inventory = user->getCurCharacInvenW();
    Inven_Item target =
        inventory->GetInvenSlot(
            static_cast<INVEN_TYPE>(1), message->body_.target_slot);
    if (target.isEmpty() ||
        target.item_id_ != message->body_.target_item_id) {
        return 4;
    }

    Inven_Item material;
    if (!upgrade_room::ResolveUpgradeMaterial(
            user, message->body_.material_slot, material)) {
        material = inventory->GetInvenSlot(
            static_cast<INVEN_TYPE>(1), message->body_.material_slot);
    }
    if (material.isEmpty()) {
        return 22;
    }
    if (!GlobalData::s_pEvent_UpGrade_Jar->CheckMaxLuckyLevel(
            target, material.item_id_)) {
        return 1012;
    }
    GlobalData::s_pEvent_UpGrade_Jar->CheckMaterialIndex(
        material.item_id_);

    ENUM_ERROR error = static_cast<ENUM_ERROR>(0);
    message->body_.upgrade_count = static_cast<uint16_t>(
        _getUpgradeCountByUpgradeItem(target, material, error));
    if (static_cast<int>(error) != 0) {
        return static_cast<int>(error);
    }
    message->body_.upgrade_rate = static_cast<uint8_t>(
        _getUpgradeRateByUpgradeItem(target, material, error));
    if (static_cast<int>(error) != 0) {
        return static_cast<int>(error);
    }

    CItem* const item = G_CDataManager()->find_item(
        static_cast<int32_t>(target.item_id_));
    if (!item) {
        return 4;
    }

    Inven_Item ticket;
    char forced_penalty = 0;
    if (message->body_.ticket_slot != 0xffffu) {
        if (static_cast<uint8_t>(
                item->vtable_->is_possible_kiri_protection(item)) != 1u) {
            return 234;
        }
        ticket = inventory->GetInvenSlot(
            static_cast<INVEN_TYPE>(1), message->body_.ticket_slot);
        if (ticket.isEmpty()) {
            return 4;
        }

        UpgradeItemTicketMap& tickets =
            G_CDataManager()
                ->get_server_parameter_script()
                ->get_upgrade_item_tickets();
        UpgradeItemTicketMap::iterator found =
            tickets.find(ticket.item_id_);
        if (found == tickets.end()) {
            return 9;
        }
        if (!IsUpgradeProtectionPolicyModeAllowed(
                found->second, message->body_.mode)) {
            return 9;
        }
        const uint8_t upgrade =
            static_cast<uint8_t>(target.GetUpgrade());
        if (!IsUpgradeProtectionPolicyLevelAllowed(
                found->second, upgrade)) {
            return 8;
        }
        forced_penalty =
            static_cast<char>(found->second.penalty_protection);
    }

    upgrade_table_t* const table =
        target.amplify_option_.hasAbility()
            ? GlobalData::s_itemAmplifier_->getUpgradeInfoTable()
            : CItemUpgradeTable::getUpgradeInfoTable();
    upgrade_info_t info;
    if (_GetUpgradeInfo(
            &target,
            item,
            *table,
            &info,
            user->get_charac_level()) != 1) {
        return 13;
    }
    if (message->body_.upgrade_count != 0u) {
        info.calculated_cost = 0;
        info.event_material_item_count = 1;
        info.event_material_item_index =
            static_cast<int32_t>(material.item_id_);
    }

    if (!_CheckTargetUsableLv(material, target)) {
        return 19;
    }
    const int portable_error =
        _ProcPortableUpgradeItem(material, target, info);
    if (portable_error != 0) {
        return portable_error;
    }
    if (ticket.item_id_ == 2675470u ||
        ticket.item_id_ == 2675471u) {
        info.calculated_cost = 0;
    }

    GlobalData::s_pEvent_UpGrade_Jar->SettingRecipeNUpConunt(
        static_cast<int32_t>(material.item_id_),
        info,
        static_cast<uint8_t>(target.GetUpgrade()));

    if (message->body_.mode == 0u &&
        static_cast<uint8_t>(
            item->vtable_->is_impossible_upgrade(item)) != 0u) {
        return 19;
    }
    if (message->body_.mode == 1u &&
        static_cast<uint8_t>(
            item->vtable_->is_impossible_amplify(item)) != 0u) {
        return 19;
    }

    if (message->body_.ticket_slot != 0xffffu) {
        const Inven_Item current_ticket =
            inventory->GetInvenSlot(
                static_cast<INVEN_TYPE>(1), message->body_.ticket_slot);
        if (current_ticket.item_id_ == 2675471u) {
            info.event_material_item_count =
                message->body_.mode == 0u ? 50 : 10;
        } else if (current_ticket.item_id_ == 2675470u) {
            info.event_material_item_count =
                message->body_.mode == 0u ? 100 : 20;
        }
    }

    const int condition_error =
        _CheckCondition(user, target, item, material, *table, info);
    if (condition_error != 0) {
        return condition_error;
    }
    if (message->body_.mode == 0u &&
        target.amplify_option_.hasAbility()) {
        return 23;
    }
    if (message->body_.mode == 1u &&
        !target.amplify_option_.hasAbility()) {
        return 23;
    }

    packet->put_byte(1);
    const int old_upgrade =
        static_cast<uint8_t>(target.GetUpgrade());
    bool succeeded = false;
    bool fixed_ticket_succeeded = false;
    if (message->body_.upgrade_count != 0u) {
        succeeded = true;
        const unsigned long limit = 99u;
        if (random_->randInt(limit) <
            static_cast<unsigned int>(message->body_.upgrade_rate)) {
            target.SetUpgrade(
                static_cast<uint8_t>(message->body_.upgrade_count));
            fixed_ticket_succeeded = true;
        } else {
            target.SetUpgrade(static_cast<uint8_t>(old_upgrade));
        }
    } else {
        succeeded =
            static_cast<uint8_t>(_DoProcUpgrade(user, &target, &info)) != 0u;

        int charged_cost = 0;
        CEventBase* const event =
            GlobalData::s_event_manager->GetRepeatEvent(50);
        if (event->IsEventing(0) &&
            !target.amplify_option_.hasAbility()) {
            const int reduce_rate =
                static_cast<CReduceUpgradeItemPay*>(
                    GlobalData::s_event_manager->GetRepeatEvent(50))
                    ->GetReduceRate();
            if (reduce_rate > 0) {
                charged_cost =
                    info.calculated_cost -
                    reduce_rate * info.calculated_cost / 100;
                if (charged_cost < 0) {
                    charged_cost = 0;
                }
            }
        } else {
            charged_cost = info.calculated_cost;
        }

        const Inven_Item equipped =
            user->getCurCharacInvenR()->GetInvenSlot(
                static_cast<INVEN_TYPE>(0), 11);
        CItem* const equipped_item = G_CDataManager()->find_item(
            static_cast<int32_t>(equipped.item_id_));
        if (equipped_item) {
            const float current_cost =
                static_cast<float>(charged_cost);
            charged_cost = static_cast<int>(
                static_cast<CEquipItem*>(equipped_item)
                        ->getUpgradeCostDiscount() *
                    current_cost / -100.0f +
                current_cost);
        }

        const eMoneySubReason money_reason =
            static_cast<eMoneySubReason>(
                target.amplify_option_.hasAbility() ? 46 : 13);
        const int money_consumed =
            upgrade_room::IsOwnerUpgradeContextActive(user)
                ? upgrade_room::ConsumeUpgradeMoney(
                      user, charged_cost, static_cast<int>(money_reason))
                : user->getCurCharacInvenW()->use_money(
                      charged_cost, money_reason, true);
        if (static_cast<uint8_t>(money_consumed) != 1u) {
            return 10;
        }
    }

    const int material_consumed =
        upgrade_room::IsOwnerUpgradeContextActive(user)
            ? upgrade_room::ConsumeUpgradeMaterial(
                  user,
                  message->body_.material_slot,
                  static_cast<int>(material.item_id_),
                  info.event_material_item_count,
                  12)
            : user->getCurCharacInvenW()->delete_item(
                  static_cast<INVEN_TYPE>(1),
                  message->body_.material_slot,
                  info.event_material_item_count,
                  static_cast<eItemDelReason>(12),
                  true);
    if (static_cast<uint8_t>(material_consumed) != 1u) {
        return 17;
    }
    if (message->body_.upgrade_count != 0u) {
        user->history_log().apply_Item(
            static_cast<int32_t>(material.item_id_),
            static_cast<int32_t>(target.item_id_),
            static_cast<eApplyItemReason>(1));
    }

    GetInstanceValueStatistic()->AddValueStatistic(
        static_cast<VALUE_STATISTIC_FIELD>(14),
        user,
        static_cast<unsigned int>(info.calculated_cost));

    packet->put_byte(message->body_.mode);
    packet->put_short(message->body_.material_slot);
    const Inven_Item remaining_material =
        user->getCurCharacInvenW()->GetInvenSlot(
            static_cast<INVEN_TYPE>(1), message->body_.material_slot);
    packet->put_int(remaining_material.get_add_info());
    packet->put_short(
        succeeded ? static_cast<uint16_t>(0xffffu)
                  : message->body_.ticket_slot);
    if (message->body_.upgrade_count != 0u) {
        packet->put_byte(1);
    } else if (GlobalData::s_pEvent_UpGrade_Jar->isNewNPC()) {
        packet->put_byte(2);
    } else if (GlobalData::s_pEvent_UpGrade_Jar->isFortune()) {
        packet->put_byte(3);
    } else {
        packet->put_byte(0);
    }
    packet->put_byte(old_upgrade);

    const int current_upgrade =
        static_cast<uint8_t>(target.GetUpgrade());
    if (current_upgrade >= table->notice_upgrade_level &&
        (message->body_.upgrade_count == 0u ||
         fixed_ticket_succeeded)) {
        _NoticeUpgrade(user, target, succeeded);
    }
    if (current_upgrade > 12) {
        DB_ReqItemUpgradeLog::makeRequest(
            static_cast<unsigned int>(
                G_CEnvironment()->get_server_group()),
            target.item_id_,
            static_cast<unsigned int>(user->getCurCharacNo()),
            user->getCurCharacName(),
            static_cast<unsigned int>(current_upgrade),
            succeeded);
    }

    if (succeeded) {
        packet->put_byte(
            message->body_.upgrade_count == 0u ||
                    fixed_ticket_succeeded
                ? 0
                : 1);
        packet->put_byte(current_upgrade);
        packet->put_short(message->body_.target_slot);
        if (message->body_.upgrade_count == 0u) {
            APSystem::CUserProc::ClearActionAndSendtoUser(
                user,
                static_cast<APSystem::_ActionGroupIndex>(12),
                0,
                ENUM_PACKETCLASS_NOTIPACKET);
        }
    } else {
        int sell_price = 0;
        G_Store()->GetSellItemPrice(
            target,
            item,
            static_cast<int16_t>(target.add_info_),
            false,
            sell_price);
        GetInstanceValueStatistic()->AddValueStatistic(
            static_cast<VALUE_STATISTIC_FIELD>(15),
            user,
            static_cast<unsigned int>(sell_price));

        const int penalty_error =
            _ProcPenalty(
                user,
                &target,
                item,
                message->body_.target_slot,
                *table,
                &info,
                packet,
                &ticket,
                forced_penalty);
        if (penalty_error != 0) {
            if (penalty_error == 216) {
                user->SendUpdateItem(
                    static_cast<CUser::eSendTarget>(1),
                    static_cast<ENUM_ITEMSPACE>(0),
                    message->body_.material_slot);
                user->SendUpdateItemList(
                    static_cast<CUser::eSendTarget>(1),
                    static_cast<ENUM_ITEMSPACE>(0),
                    0);
            }
            return penalty_error;
        }

        // The selected protection ticket is a failure-side resource.  The
        // 0828 client sends its slot independently from the upgrade material;
        // consume it only after _ProcPenalty has actually replaced the native
        // failure penalty.  A successful roll reports 0xffff above so the
        // client does not mirror a deletion that never happened.
        if (message->body_.ticket_slot != 0xffffu) {
            if (static_cast<uint8_t>(
                    user->getCurCharacInvenW()->delete_item(
                        static_cast<INVEN_TYPE>(1),
                        message->body_.ticket_slot,
                        1,
                        static_cast<eItemDelReason>(3),
                        true)) != 1u) {
                return 17;
            }
            user->history_log().apply_Item(
                static_cast<int32_t>(ticket.item_id_),
                static_cast<int32_t>(target.item_id_),
                static_cast<eApplyItemReason>(0));
        }
    }

    user->history_log().UpgradeItem(
        item->get_index(),
        old_upgrade,
        static_cast<uint8_t>(target.GetUpgrade()),
        succeeded);
    const char* const guild_item_name =
        static_cast<uint8_t>(
            item->vtable_->is_random_option(item)) != 0u
            ? message->body_.guild_item_name
            : item->GetItemName();
    user->makeGuildItemUpgradMessage(
        guild_item_name,
        old_upgrade,
        static_cast<uint8_t>(target.GetUpgrade()),
        succeeded);

    if (target.item_id_ != 0u) {
        const int update_result = inventory->update_item(
            static_cast<INVEN_TYPE>(1),
            message->body_.target_slot,
            target);
        if (update_result == 1 && succeeded &&
            static_cast<int>(static_cast<uint8_t>(target.GetUpgrade())) >
                old_upgrade) {
            freely_give_item::OnNormalUpgradeResult(
                user,
                static_cast<uint32_t>(target.item_id_),
                static_cast<uint8_t>(target.GetUpgrade()),
                true);
        }
    }
    user->SendUpdateItemList(
        static_cast<CUser::eSendTarget>(1),
        static_cast<ENUM_ITEMSPACE>(0),
        0);
    return 0;
}

}  // namespace WongWork
