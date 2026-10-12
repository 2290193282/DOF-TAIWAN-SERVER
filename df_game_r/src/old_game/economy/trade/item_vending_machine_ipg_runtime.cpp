#include "old_game/economy/shop/npc_runtime.h"

#include <cstdlib>
#include <cstring>
#include <utility>
#include <vector>

#include "core/config/env/environment.h"
#include "common/manager/data_manager.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/ipg_runtime.h"
#include "common/manager/global_data.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_limit_edition.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"
#include "old_data/db/account/db_save_pu_user_check.h"
#include "old_data/db/auction_cashshop/db_billing_record.h"
#include "old_data/db/auction_cashshop/db_update_cerashop_purchase_count.h"
#include "old_data/db/auction_cashshop/db_update_gift_count.h"
#include "old_data/Login/inter_returns/break_away_prevent_runtime.h"
#include "old_game/activity/schedule/hero_mission_runtime.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/economy/mall/cerashop_add_restrict_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x854C4F6
// IDA_NAME: _ZN18ItemVendingMachine18ProcessIPGUserDataEP5CUserPN8WongWork3IPG8SIPGDataES5_P6Stream
// DEMANGLED: ItemVendingMachine::ProcessIPGUserData(CUser *,WongWork::IPG::SIPGData *,WongWork::IPG::SIPGData *,Stream *)
int ItemVendingMachine::ProcessIPGUserData(
    CUser* user,
    WongWork::IPG::SIPGData* request,
    WongWork::IPG::SIPGData* result,
    Stream* callback_stream) {
    ProcessIPGUserDataJpnEx(user, request, result, callback_stream);
    return 0;
}

// IDA_EA: 0x81783EA
// IDA_NAME: _ZN18ItemVendingMachine23ProcessIPGUserDataJpnExEP5CUserPN8WongWork3IPG8SIPGDataES5_P6Stream
// DEMANGLED: ItemVendingMachine::ProcessIPGUserDataJpnEx(CUser *,WongWork::IPG::SIPGData *,WongWork::IPG::SIPGData *,Stream *)
void ItemVendingMachine::ProcessIPGUserDataJpnEx(
    CUser* user,
    WongWork::IPG::SIPGData* request,
    WongWork::IPG::SIPGData* result,
    Stream* callback_stream) {
    if (!user) {
        return;
    }

    switch (result->data_type) {
    case WongWork::IPG::IPG_QUERY:
        ProcessIPG_ResultQuery(user, request, result, callback_stream);
        return;
    case WongWork::IPG::IPG_INPUT:
        ProcessIPG_ResultInput(user, request, result, callback_stream);
        return;
    case WongWork::IPG::IPG_OUTPUT:
        ProcessIPG_ResultOutput(user, request, result, callback_stream);
        return;
    default:
        LogManager::logFormat(
            1,
            "localjapan/AppendVendingMachine.cpp",
            "void ItemVendingMachine::ProcessIPGUserDataJpnEx(CUser*, WongWork::IPG::SIPGData*, WongWork::IPG::SIPGData*, Stream*)",
            0x482,
            "Unhandled reply from BModule is %d",
            static_cast<int>(result->data_type));
        return;
    }
}

// IDA_EA: 0x81784D4
// IDA_NAME: _ZN18ItemVendingMachine22ProcessIPG_ResultQueryEP5CUserPN8WongWork3IPG8SIPGDataES5_P6Stream
// DEMANGLED: ItemVendingMachine::ProcessIPG_ResultQuery(CUser *,WongWork::IPG::SIPGData *,WongWork::IPG::SIPGData *,Stream *)
void ItemVendingMachine::ProcessIPG_ResultQuery(
    CUser* user,
    WongWork::IPG::SIPGData*,
    WongWork::IPG::SIPGData* result,
    Stream*) {
    if (!user) {
        return;
    }

    // The original creates this guard even though the query branch does not
    // build a packet.  It preserves the dispatcher-thread packet-pool scope.
    PacketGuard guard;
    const WongWork::IPG::SIPGQueryWire& query = result->wire.query;
    const int result_code = std::atoi(query.result_code_decimal);
    if (result_code) {
        user->SendCashData(true);
        LogManager::logFormat(
            1,
            "localjapan/AppendVendingMachine.cpp",
            "void ItemVendingMachine::ProcessIPG_ResultQuery(CUser*, WongWork::IPG::SIPGData*, WongWork::IPG::SIPGData*, Stream*)",
            0x4DD,
            "IPG_QUERY %d %d",
            user->get_acc_id(),
            result_code);
        return;
    }

    const unsigned int cera = static_cast<unsigned int>(
        std::atoi(query.balance_decimal[4]) +
        std::atoi(query.balance_decimal[5]) +
        std::atoi(query.balance_decimal[2]) +
        std::atoi(query.balance_decimal[1]) +
        std::atoi(query.balance_decimal[3]) +
        std::atoi(query.balance_decimal[0]));
    user->SetCera(cera);
    user->SetCeraPoint(
        static_cast<unsigned int>(std::atoi(query.point_decimal)));
    user->history_log().CeraPoint(
        static_cast<ENUM_CERAPOINT_LOG_TYPE>(0), cera);
    user->SendCashData(false);
}

// IDA_EA: 0x817866C
// IDA_NAME: _ZN18ItemVendingMachine22ProcessIPG_ResultInputEP5CUserPN8WongWork3IPG8SIPGDataES5_P6Stream
// DEMANGLED: ItemVendingMachine::ProcessIPG_ResultInput(CUser *,WongWork::IPG::SIPGData *,WongWork::IPG::SIPGData *,Stream *)
void ItemVendingMachine::ProcessIPG_ResultInput(
    CUser*,
    WongWork::IPG::SIPGData*,
    WongWork::IPG::SIPGData*,
    Stream*) {
    // 0x817866C is the original nine-byte empty body.
}

// IDA_EA: 0x8178676
// IDA_NAME: _ZN18ItemVendingMachine23ProcessIPG_ResultOutputEP5CUserPN8WongWork3IPG8SIPGDataES5_P6Stream
// DEMANGLED: ItemVendingMachine::ProcessIPG_ResultOutput(CUser *,WongWork::IPG::SIPGData *,WongWork::IPG::SIPGData *,Stream *)
int ItemVendingMachine::ProcessIPG_ResultOutput(
    CUser* user,
    WongWork::IPG::SIPGData* request,
    WongWork::IPG::SIPGData* result,
    Stream* callback_stream) {
    // These two process-lifetime helpers are static locals in the old body.
    // Their first construction receives the actual member `this`; later
    // invocations retain that original process-lifetime owner exactly as GCC
    // static-local initialization does.
    static Arad_IPGProcess ipg_process(this);
    static Arad_MileageProcess mileage_process(this);

    stCeraShopIPGRequest_t* const request_record =
        callback_stream->GetOutBuffer<stCeraShopIPGRequest_t>();
    const int group_index = static_cast<int>(request_record->group_index);
    const int goods_no = static_cast<int>(request_record->goods_no);
    const unsigned long item_index =
        static_cast<unsigned long>(request_record->item_index);
    const unsigned short stack_count = request_record->stack_count;
    const int item_count = static_cast<int>(request_record->item_count);
    const int item_order = static_cast<int>(request_record->item_order);
    const int featured_index = static_cast<int>(request_record->featured_index);
    const int avatar_selection_value = request_record->avatar_selection_value;
    const unsigned char select_type = request_record->select_type;
    const unsigned char avatar_select = request_record->avatar_select;
    const int current_seconds = request_record->current_seconds;
    const char gift = static_cast<char>(request_record->gift);
    const int receiver_charac_no =
        static_cast<int>(request_record->post_recipient_wire_word);
    const uint16_t buy_type = request_record->buy_type;

    // The four bytes at +0x40 are an account-id wire word.  They are not a
    // character string; the old code loads the word directly from the Stream
    // record before forwarding it to every gift path.
    uint32_t receiver_member_id = 0;
    std::memcpy(&receiver_member_id,
                request_record->recipient_name_suffix,
                sizeof(receiver_member_id));

    std::vector<Inven_Item> output_items;
    WongWork::CCeraShop::stCeraShopItemParam_t item_param;
    for (int index = 0; index < static_cast<int>(request_record->option_count);
         ++index) {
        const stCeraShopIPGRequestOption_t& option =
            request_record->options[index];
        item_param.selected_items.push_back(
            std::make_pair(static_cast<int>(option.item_id),
                           static_cast<int>(option.selected)));
    }
    for (int index = 0;
         index < static_cast<int>(request_record->additional_item_count);
         ++index) {
        const stCeraShopIPGRequestAdditionalItem_t& additional =
            request_record->additional_items[index];
        tagPackageItemSelection selection;
        selection.item_index = additional.item_id;
        selection.package_index = additional.first_option;
        selection.selection_index = additional.second_option;
        item_param.package_item_selections.push_back(selection);
    }

    int callback_cera_price = 0;
    *callback_stream >> callback_cera_price;
    int processing_result = std::atoi(result->wire.output.result_code_decimal);
    if (processing_result != 0 || !user || !user->getCurCharacR()) {
        if (!user || !user->getCurCharacR()) {
            const int wire_member_id =
                std::atoi(result->wire.output.account_id);
            const int wire_goods_no =
                std::atoi(result->wire.output.product_id_decimal);
            cMyTrace trace(
                "void ItemVendingMachine::ProcessIPG_ResultOutput(CUser*, WongWork::IPG::SIPGData*, WongWork::IPG::SIPGData*, Stream*)",
                1437,
                5);
            trace(
                "There's abnormal transaction that the client doesn't exist, client would be %s(m_id: %d) and the Goodis is %d, ret_code = %d ret_msg = %s",
                result->wire.output.account_name,
                wire_member_id,
                wire_goods_no,
                processing_result,
                result->wire.output.result_message);
        }
        cera_shop_._ProcessIPGError(user,
                                    request,
                                    result,
                                    0x19u,
                                    group_index,
                                    static_cast<unsigned int>(goods_no),
                                    item_order,
                                    item_count,
                                    gift);
        return 0;
    }

    int creature_item_id = -1;
    int response_group_index = group_index;
    switch (static_cast<unsigned int>(group_index)) {
    case 0:
        ipg_process.CashShopModeAvatar(user,
                                       item_index,
                                       goods_no,
                                       avatar_selection_value,
                                       static_cast<char>(select_type),
                                       static_cast<char>(avatar_select),
                                       request->wire.output.order_id,
                                       processing_result,
                                       receiver_member_id,
                                       receiver_charac_no,
                                       gift);
        break;
    case 1:
        ipg_process.CashShopModeCoin(user,
                                     stack_count,
                                     receiver_member_id,
                                     receiver_charac_no,
                                     gift);
        break;
    case 2:
    case 7:
    case 9:
        ipg_process.CashShopModeItem(user,
                                     item_index,
                                     stack_count,
                                     featured_index,
                                     current_seconds,
                                     output_items,
                                     item_param,
                                     processing_result,
                                     receiver_member_id,
                                     receiver_charac_no,
                                     gift);
        break;
    case 3:
        ipg_process.CashShopModeCreature(user,
                                         item_index,
                                         stack_count,
                                         creature_item_id,
                                         processing_result,
                                         receiver_member_id,
                                         receiver_charac_no,
                                         gift);
        break;
    case 10: {
        cera_shop_._processAdditionalItem(user,
                                           item_index,
                                           stack_count,
                                           select_type,
                                           avatar_select,
                                           output_items,
                                           item_param,
                                           0);
        response_group_index = -1;
        CItem* const item = G_CDataManager()->find_item(
            static_cast<int>(item_index));
        if (item && item->isCerashopPackageItem()) {
            response_group_index = 7;
        }
        if (response_group_index != 7) {
            output_items.clear();
        }

        CItemLimitEdition* const limit_item =
            G_CDataManager()->getItemLimitEditionMgr()->getItemInfo(
                static_cast<unsigned int>(goods_no));
        if (!limit_item) {
            throw static_cast<ENUM_ERROR>(17);
        }

        int effective_featured_index = limit_item->getFeaturedIdx();
        if (effective_featured_index == 0) {
            int special_type = 0;
            int special_value = 0;
            bool unique = false;
            const int related_item = limit_item->getRelateditem();
            if (cera_shop_.IsSpecialItem(related_item,
                                         special_type,
                                         special_value,
                                         unique)) {
                effective_featured_index =
                    G_CDataManager()->get_item_list()->GetRestrictCode(
                        limit_item->getRelateditem());
            }
        }
        if (effective_featured_index != 0) {
            // Preserve the old body exactly: it gates on the derived value,
            // but saves CItemLimitEdition's own FeaturedIdx field.
            cera_shop_.SaveFeaturedIdx(user,
                                       limit_item->getFeaturedIdx(),
                                       GlobalData::s_systemTime_.getCurSec(),
                                       0);
        }
        LogManager::logFormat(
            1,
            "localjapan/AppendVendingMachine.cpp",
            "void ItemVendingMachine::ProcessIPG_ResultOutput(CUser*, WongWork::IPG::SIPGData*, WongWork::IPG::SIPGData*, Stream*)",
            0x653,
            "Invalid Group Index [%d]",
            response_group_index);
        break;
    }
    case 11: {
        const int mileage_error = mileage_process.BuyMileageItem(
            user,
            goods_no,
            item_index,
            stack_count,
            featured_index,
            current_seconds,
            avatar_selection_value,
            static_cast<char>(select_type),
            static_cast<char>(avatar_select),
            callback_cera_price,
            item_count,
            item_order,
            request->wire.output.order_id,
            item_param);
        if (mileage_error != 0) {
            mileage_process.BuyFail(user,
                                    static_cast<short>(mileage_error),
                                    goods_no,
                                    item_order,
                                    item_count);
        }
        return 0;
    }
    default:
        LogManager::logFormat(
            1,
            "localjapan/AppendVendingMachine.cpp",
            "void ItemVendingMachine::ProcessIPG_ResultOutput(CUser*, WongWork::IPG::SIPGData*, WongWork::IPG::SIPGData*, Stream*)",
            0x653,
            "Invalid Group Index [%d]",
            response_group_index);
        break;
    }

    if (processing_result != 0) {
        cera_shop_._ProcessIPGError(user,
                                    request,
                                    result,
                                    static_cast<unsigned char>(processing_result),
                                    response_group_index,
                                    static_cast<unsigned int>(goods_no),
                                    item_order,
                                    item_count,
                                    gift);
        cMyTrace trace(
            "void ItemVendingMachine::ProcessIPG_ResultOutput(CUser*, WongWork::IPG::SIPGData*, WongWork::IPG::SIPGData*, Stream*)",
            1627,
            0);
        trace("[%s][%d]",
              "void ItemVendingMachine::ProcessIPG_ResultOutput(CUser*, WongWork::IPG::SIPGData*, WongWork::IPG::SIPGData*, Stream*)",
              processing_result);
        return 0;
    }

    if (response_group_index == 7) {
        ARAD::Singleton<HeroMissionEvent>::Get()->setMissionValue(
            user,
            HeroMissionCondition::MissionNo::CLEAR_DUNGEON,
            7);
    }

    if (buy_type == 1) {
        user->getCurCharacInvenW()->use_money(
            request_record->gold_price,
            static_cast<eMoneySubReason>(0),
            true);
        user->UseWinPoint(request_record->win_point,
                          static_cast<eWPSubReason>(2));
        user->UseCeraPoint(static_cast<unsigned int>(request_record->cera_price));
        user->history_log().CeraPoint(
            static_cast<ENUM_CERAPOINT_LOG_TYPE>(3),
            static_cast<unsigned int>(request_record->cera_price));
    } else if (buy_type == 0) {
        GlobalData::s_pIPGHelper->IPGOutputConfirm(request, result);
        user->getCurCharacInvenW()->use_money(
            request_record->gold_price,
            static_cast<eMoneySubReason>(0),
            true);
        user->UseWinPoint(request_record->win_point,
                          static_cast<eWPSubReason>(2));
        user->UseCera(static_cast<unsigned int>(request_record->cera_price));
        user->addUsedCera(static_cast<unsigned int>(request_record->cera_price));
    }

    mileage_process.GiveMileage(user, goods_no, request_record->cera_price);
    if (CheckPurcahseBonusItem(static_cast<unsigned long>(goods_no))) {
        DB_UpdateCeraShopPurchaseCount::makeRequest(
            static_cast<unsigned int>(user->get_acc_id()),
            item_index,
            goods_no,
            user->GetUID());
    }
    CerashopAddRestrict::Manager::UpdateBuyableRestrictItem(
        user,
        static_cast<unsigned int>(goods_no),
        1u);
    if (featured_index != 0 && user->IsRestrictedGoods(featured_index) != 1) {
        user->SetRestrictedGoods(featured_index);
        cera_shop_.SaveFeaturedIdx(user, featured_index, current_seconds, 0);
    }

    PacketGuard result_packet;
    result_packet.put_header(1, ENUM_CMDPACKET_BUY_CERASHOP_ITEM);
    result_packet.put_byte(1);
    result_packet.put_byte(gift);
    result_packet.put_int(response_group_index);
    result_packet.put_int(goods_no);
    result_packet.put_int(item_order);
    result_packet.put_int(item_count);
    result_packet.put_int(creature_item_id);
    result_packet.put_short(static_cast<int>(output_items.size()));
    for (std::vector<Inven_Item>::const_iterator item = output_items.begin();
         item != output_items.end();
         ++item) {
        result_packet.put_int(item->item_id_);
        result_packet.put_int(item->add_info_);
    }
    result_packet.put_short(0);
    if (gift == 1 && receiver_member_id != 0) {
        DB_UpdateGiftCount::makeRequest(receiver_member_id, 1);
    }

    unsigned long bonus_item_index = static_cast<unsigned long>(-1);
    unsigned int bonus_item_count = 0;
    if (item_count == item_order) {
        user->SendCashData(false);
        PacketGuard inventory_packet;
        inventory_packet.put_header(0, 14);
        inventory_packet.put_byte(0);
        inventory_packet.put_short(2);
        user->getCurCharacInvenR()->MakeItemPacket(
            static_cast<INVEN_TYPE>(1), 0, inventory_packet);
        user->getCurCharacInvenR()->MakeItemPacket(
            static_cast<INVEN_TYPE>(1), 2, inventory_packet);
        inventory_packet.finalize(true);
        user->Send(inventory_packet);

        CEventBase* const repeat_event =
            GlobalData::s_event_manager->GetRepeatEvent(17);
        if (repeat_event->IsEventing(0)) {
            CCeraShopBonusItemEvent* const bonus_event =
                static_cast<CCeraShopBonusItemEvent*>(
                    GlobalData::s_event_manager->GetRepeatEvent(17));
            if (bonus_event->getBonusRate() >=
                static_cast<int>(get_rand_int(1000))) {
                G_CDataManager()->get_item_list()->getCeraShopBonusItem(
                    user->getUsedCera(), bonus_item_index, bonus_item_count);
                if (bonus_item_index != static_cast<unsigned long>(-1)) {
                    std::vector<Inven_Item> ignored_bonus_items;
                    cera_shop_._processAdditionalItem(
                        user,
                        bonus_item_index,
                        static_cast<unsigned short>(bonus_item_count),
                        0,
                        255,
                        ignored_bonus_items,
                        item_param,
                        0);
                }
            }
        }
    }
    result_packet.put_int(static_cast<int>(bonus_item_index));
    result_packet.put_int(static_cast<int>(bonus_item_count));
    result_packet.finalize(true);
    user->Send(result_packet);

    GlobalData::s_BreakAwaySys->updateUsedCera(
        static_cast<unsigned int>(user->get_acc_id()),
        static_cast<unsigned int>(request_record->cera_price));

    if (buy_type == 1) {
        BillingRecord(user,
                      goods_no,
                      -1,
                      request_record->cera_price,
                      result->wire.output.order_id,
                      result->wire.output.result_message,
                      stack_count,
                      result->wire.output.transaction.transaction_id,
                      gift);
        user->history_log().BuyCashShopItem(
            static_cast<unsigned int>(goods_no),
            item_index,
            stack_count,
            result->wire.output.order_id,
            gift,
            static_cast<CeraShopPaymentType::T>(1),
            result->wire.output.transaction.transaction_id);
    } else {
        BillingRecord(user,
                      goods_no,
                      -1,
                      request_record->cera_price,
                      request->wire.output.order_id,
                      request->wire.output.result_message,
                      stack_count,
                      result->wire.output.transaction.transaction_id,
                      gift);
        user->history_log().BuyCashShopItem(
            static_cast<unsigned int>(goods_no),
            item_index,
            stack_count,
            request->wire.output.order_id,
            gift,
            static_cast<CeraShopPaymentType::T>(0),
            result->wire.output.transaction.transaction_id);
    }
    return 0;
}

// IDA_EA: 0x817991E
// IDA_NAME: _ZN18ItemVendingMachine13BillingRecordEP5CUseriiiPKcS3_iyc
// DEMANGLED: ItemVendingMachine::BillingRecord(CUser *,int,int,int,char const*,char const*,int,unsigned long long,char)
void ItemVendingMachine::BillingRecord(CUser* user,
                                       int goods_no,
                                       int group_index,
                                       int cera_price,
                                       const char* order_id,
                                       const char* product_name,
                                       int stack_count,
                                       unsigned long long transaction_id,
                                       char buy_type) {
    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire(
            "localjapan/AppendVendingMachine.cpp", 1840),
        true);
    guard << 176;
    guard << user->GetUID();

    SIG_BILLING_RECORD record;
    std::memset(&record, 0, sizeof(record));
    std::strcpy(record.agency_no, order_id);
    std::strcpy(record.user_id, user->get_acc_name());
    record.seal_flag = 1;
    record.price = cera_price;
    record.slot_part_type = 0;
    record.slot_part_code = 1;
    record.ipg_no = goods_no;
    record.bonus_ipg_no = group_index;
    record.m_id = static_cast<uint32_t>(user->get_acc_id());
    record.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
    record.job = user->get_charac_job();

    char ssn[14] = {};
    std::strncpy(ssn, user->GetSsnString(), sizeof(ssn));
    char sex_code_text[2] = {ssn[0], 0};
    char year[3] = {ssn[1], ssn[2], 0};
    char month[3] = {ssn[3], ssn[4], 0};
    char day[3] = {ssn[5], ssn[6], 0};
    const int sex_code = std::atoi(sex_code_text);
    if (sex_code == 1) {
        std::sprintf(record.birthday, "19%s-%s-%s", year, month, day);
        record.sex = 1;
    } else if (sex_code == 2) {
        std::sprintf(record.birthday, "19%s-%s-%s", year, month, day);
        record.sex = 2;
    } else if (sex_code == 3) {
        std::sprintf(record.birthday, "20%s-%s-%s", year, month, day);
        record.sex = 1;
    } else if (sex_code == 4) {
        std::sprintf(record.birthday, "20%s-%s-%s", year, month, day);
        record.sex = 2;
    }

    record.prize_flag = 1;
    record.result_code = 0;
    std::strcpy(record.result_message, product_name);
    record.server_id = G_CEnvironment()->get_server_group();
    record.grow_type = static_cast<char>(user->getCurCharacGrowType());
    record.level = user->get_charac_level();
    record.gift_server_info = record.server_id;
    record.gift_get_charac_no = 0;
    record.charge_price = cera_price;
    record.free_price = 0;
    record.count_no = stack_count;
    record.each_price = cera_price;
    if (std::strcmp(product_name, "mileage item") == 0) {
        record.price_type = 2;
    } else if (std::strcmp(product_name, "cera point") == 0) {
        record.price_type = 3;
    } else {
        record.price_type = 1;
    }

    if (!user->network_.GetPeerIP2(record.user_ip, sizeof(record.user_ip))) {
        std::memcpy(record.user_ip, "NONE", 5);
    }
    record.tran_id = static_cast<uint32_t>(transaction_id);
    record.unused_265 = static_cast<uint32_t>(transaction_id >> 32);
    record.buy_type = static_cast<uint8_t>(buy_type);

    guard.put_binary(&record, static_cast<int>(sizeof(record)));
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    if (user->GetPuUser() != 1) {
        DB_SavePuUserCheck::makeRequest(
            static_cast<unsigned int>(user->get_acc_id()));
    }
}
