#include "old_game/economy/shop/npc_runtime.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <utility>
#include <vector>

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/cash_shop_gift_only_0828.h"
#include "core/config/env/environment.h"
#include "common/manager/data_manager.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/ipg_runtime.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_limit_edition.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/server_utility.h"
#include "common/manager/statistics_runtime.h"
#include "infra/log/format/trace.h"
#include "old_data/Login/inter_returns/break_away_prevent_runtime.h"
#include "old_data/db/auction_cashshop/db_update_gift_count.h"
#include "old_data/db/legacy_dispatch/db_sr08_leaf_dispatch_payloads.h"
#include "old_game/activity/schedule/event_login_item_runtime.h"
#include "old_game/activity/schedule/event_mgr_runtime.h"
#include "old_game/economy/inventory/gen_unique_no.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/avatar_type_select_runtime.h"
#include "old_game/economy/mail/mail_trade_params.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/premium_runtime.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

namespace {

const unsigned int kCeraShopMaxBuyItemCount = 35u;

const CItem* GetRelatedItem(const CCeraShopGoods* goods) {
    return goods
               ? reinterpret_cast<const CItem*>(
                     static_cast<uintptr_t>(goods->GetRelatedItem()))
               : 0;
}

bool IsPvpRestrictedCeraItem(int item_index) {
    switch (item_index) {
    case 194:
    case 196:
    case 197:
    case 720:
    case 743:
    case 744:
    case 820:
    case 920:
    case 2660290:
    case 2660358:
    case 2660401:
    case 2660402:
    case 2670266:
    case 2675028:
        return true;
    default:
        return false;
    }
}

bool ResolveAvatarPrice(const CCeraShopGoods* goods,
                        unsigned char avatar_select,
                        unsigned char select_type,
                        stAvatarTypeSelect_t& selection,
                        int& cera_price,
                        int& gold_price,
                        int& win_point) {
    const CItem* const related_item = GetRelatedItem(goods);
    if (!related_item || !related_item->vtable_ ||
        !related_item->vtable_->is_avatar_item ||
        !related_item->vtable_->is_avatar_item(related_item) ||
        select_type > 0x63u) {
        return false;
    }

    const CEquipItem* const equipment =
        static_cast<const CEquipItem*>(related_item);
    const std::vector<stAvatarTypeSelect_t>& selections =
        equipment->avatar_type_selects_;
    if (selections.empty() || avatar_select >= selections.size()) {
        return false;
    }

    selection = selections.at(avatar_select);
    cera_price = selection.cera_price;
    gold_price = selection.gold_price;
    win_point = selection.win_point_price;
    return true;
}

void MakePurchaseDescription(const CCeraShopGoods* goods,
                             char* destination,
                             size_t destination_size) {
    if (!destination || destination_size == 0u) {
        return;
    }

    destination[0] = '\0';
    if (!goods) {
        return;
    }

    const int item_index = goods->GetItemIdx();
    const int stack_count = goods->GetStackCount();
    if (item_index == 1) {
        std::snprintf(destination, destination_size, "Continue Coin %d",
                      stack_count);
        return;
    }

    const CItem* const related_item = GetRelatedItem(goods);
    if (!related_item) {
        return;
    }

    const char* const item_name = related_item->GetItemName();
    if (!item_name) {
        return;
    }
    if (related_item->is_stackable() && stack_count > 1) {
        std::snprintf(destination, destination_size, "%s Count %d", item_name,
                      stack_count);
    } else {
        std::strncpy(destination, item_name, destination_size - 1u);
        destination[destination_size - 1u] = '\0';
    }
}

void FillRequestOptions(stCeraShopIPGRequest_t& request,
                        const CeraShopBuyItem& item) {
    request.option_count = item.option_count;
    for (unsigned int index = 0;
         index < item.option_count && index < kCeraShopMaxBuyItemCount;
         ++index) {
        request.options[index].item_id = item.options[index].item_id;
        request.options[index].selected = item.options[index].select;
    }

    request.additional_item_count = item.additional_item_count;
    for (unsigned int index = 0;
         index < item.additional_item_count && index < kCeraShopMaxBuyItemCount;
         ++index) {
        request.additional_items[index].item_id =
            item.additional_items[index].item_id;
        request.additional_items[index].first_option =
            item.additional_items[index].first_option;
        request.additional_items[index].second_option =
            item.additional_items[index].second_option;
    }
}

void BuildItemParam(const stCeraShopIPGRequest_t& request,
                    WongWork::CCeraShop::stCeraShopItemParam_t& item_param) {
    const unsigned int option_count =
        request.option_count < kCeraShopMaxBuyItemCount
            ? request.option_count
            : kCeraShopMaxBuyItemCount;
    for (unsigned int index = 0; index < option_count; ++index) {
        item_param.selected_items.push_back(
            std::make_pair(static_cast<int>(request.options[index].item_id),
                           static_cast<int>(request.options[index].selected)));
    }

    const unsigned int additional_count =
        request.additional_item_count < kCeraShopMaxBuyItemCount
            ? request.additional_item_count
            : kCeraShopMaxBuyItemCount;
    for (unsigned int index = 0; index < additional_count; ++index) {
        tagPackageItemSelection selection;
        selection.item_index = request.additional_items[index].item_id;
        selection.package_index = request.additional_items[index].first_option;
        selection.selection_index =
            request.additional_items[index].second_option;
        item_param.package_item_selections.push_back(selection);
    }
}

const char* GetOutputOrderId(const WongWork::IPG::SIPGData* request,
                             const WongWork::IPG::SIPGData* result) {
    if (request) {
        return request->wire.output.order_id;
    }
    return result ? result->wire.output.order_id : "";
}

void SendChildrensDayReward(WongWork::CCeraShop* cera_shop,
                            CUser* user,
                            WongWork::CCeraShop::stCeraShopItemParam_t& item_param) {
    WongWork::CEventMgr event_mgr;
    if (!event_mgr.IsChildrensDayEventing() ||
        !user->IsOKChildrensDayEventCondition()) {
        return;
    }

    WongWork::CWebEvent::ReqSaveEventItem(
        static_cast<unsigned int>(user->get_acc_id()),
        static_cast<unsigned int>(user->getCurCharacNo()),
        0x65aeu,
        1u,
        0x30d47u,
        0u,
        true);

    PacketGuard packet;
    packet.put_header(0, 14);
    packet.put_byte(1);
    int item_count_index = packet.get_index();
    packet.put_short(10);

    int added_item_count = 0;
    for (int index = 0; index <= 9; ++index) {
        const int source_slot = user->GetChildrensDayEventTargetItemSlot(index);
        const int item_period = user->GetChildrensDayEventTargetItemPeriod(index);
        if (source_slot < 0 || item_period == -1) {
            break;
        }

        const Inven_Item source_item =
            user->getCurCharacInvenR()->GetInvenSlot(2, source_slot);
        const int inserted_slot = user->getCurCharacInvenW()->AddAvatarItem(
            source_item.item_id_,
            item_period,
            false,
            static_cast<char>(source_item.avatar_select_),
            static_cast<char>(-1),
            "E_CHILDRENS_DAY",
            static_cast<eAvatarItemAddReason>(2),
            false,
            0);
        if (inserted_slot >= 0) {
            user->getCurCharacInvenR()->MakeItemPacket(
                static_cast<INVEN_TYPE>(2), inserted_slot, packet);
            ++added_item_count;
        }
    }
    packet.put_short(item_count_index, added_item_count);
    packet.finalize(true);
    user->Send(packet);
    (void)cera_shop;
    (void)item_param;
}

void AppendBonusItem(WongWork::CCeraShop* cera_shop,
                     CUser* user,
                     WongWork::CCeraShop::stCeraShopItemParam_t& item_param,
                     unsigned long& bonus_item_index,
                     unsigned int& bonus_item_count) {
    bonus_item_index = static_cast<unsigned long>(-1);
    bonus_item_count = 0u;

    if (!GlobalData::s_event_manager) {
        return;
    }
    CEventBase* const event = GlobalData::s_event_manager->GetRepeatEvent(17);
    if (!event || !event->IsEventing(0)) {
        return;
    }

    CCeraShopBonusItemEvent* const bonus_event =
        static_cast<CCeraShopBonusItemEvent*>(event);
    if (bonus_event->getBonusRate() <
        static_cast<int>(get_rand_int(1000))) {
        return;
    }

    G_CDataManager()->get_item_list()->getCeraShopBonusItem(
        user->getUsedCera(), bonus_item_index, bonus_item_count);
    if (bonus_item_index == static_cast<unsigned long>(-1)) {
        return;
    }

    std::vector<Inven_Item> ignored_output_items;
    cera_shop->_processAdditionalItem(
        user,
        bonus_item_index,
        static_cast<unsigned short>(bonus_item_count),
        0,
        255,
        ignored_output_items,
        item_param,
        0);
}

}  // namespace

// IDA_EA: 0x832033A
// IDA_NAME: _ZNK8WongWork9CCeraShop8LogErrorERKNS0_17stErrorLogInput_tE
// DEMANGLED: WongWork::CCeraShop::LogError(WongWork::CCeraShop::stErrorLogInput_t const&)const
int WongWork::CCeraShop::LogError(const stErrorLogInput_t& input) const {
    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire("CeraShop.cpp", 122), true);
    guard << 814;
    guard << input.member_id;

    SIG_CERASHOP_ERROR_LOG record;
    record.member_id = input.member_id;
    record.charac_no = input.charac_no;
    record.server_id = static_cast<uint32_t>(
        G_CEnvironment()->get_server_group());
    record.channel_no = static_cast<uint32_t>(
        G_CEnvironment()->get_channel_no());
    record.type = input.type;
    record.error_code = input.error_code;
    record.goods_id = input.goods_id;
    guard.put_binary(&record, static_cast<int>(sizeof(record)));
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    return 1;
}

// IDA_EA: 0x832047A
// IDA_NAME: _ZN8WongWork9CCeraShop17ProcessBuyRequestEP5CUserRK21MSG_BUY_CERASHOP_ITEM
// DEMANGLED: WongWork::CCeraShop::ProcessBuyRequest(CUser *,MSG_BUY_CERASHOP_ITEM const&)
int WongWork::CCeraShop::ProcessBuyRequest(
    CUser* user,
    const MSG_BUY_CERASHOP_ITEM& message) {
    if (!user) {
        return 0;
    }

    const unsigned int item_count = message.item_count;
    if (item_count == 0u || item_count > kCeraShopMaxBuyItemCount) {
        ProcessError(user,
                     0x15u,
                     -1,
                     0u,
                     1,
                     1,
                     static_cast<char>(message.gift));
        return 1;
    }

    const CCeraShopGoods* first_goods =
        G_CDataManager()->FindGoods(static_cast<int>(message.items[0].goods_no));
    if (!first_goods) {
        ProcessError(user,
                     0x15u,
                     0,
                     message.items[0].goods_no,
                     1,
                     1,
                     static_cast<char>(message.gift));
        return 1;
    }

    WongWork::CEventMgr event_mgr;
    if (event_mgr.IsChildrensDayEventing()) {
        user->ClearChildrensDayEvent();
    }

    const CCeraShopGoods* goods[kCeraShopMaxBuyItemCount + 1] = {};
    for (unsigned int index = 0; index < item_count; ++index) {
        const CeraShopBuyItem& item = message.items[index];
        goods[index] = G_CDataManager()->FindGoods(
            static_cast<int>(item.goods_no));
        if (!goods[index]) {
            ProcessError(user,
                         0x15u,
                         -1,
                         item.goods_no,
                         static_cast<int>(index + 1u),
                         static_cast<int>(item_count),
                         static_cast<char>(message.gift));
            continue;
        }

        const int group_index = goods[index]->GetGroupIdx();
        if (!user->IsGameMasterMode() &&
            (goods[index]->GetSticker() == 99 ||
             (message.gift == 1 && goods[index]->isGiftDisallowance_()) ||
             (message.gift != 1 &&
              cash_shop_0828::is_gift_only_goods(item.goods_no)))) {
            ProcessError(user,
                         8u,
                         group_index,
                         item.goods_no,
                         static_cast<int>(index + 1u),
                         static_cast<int>(item_count),
                         static_cast<char>(message.gift));
            goods[index] = 0;
            continue;
        }

        if (group_index != 1 && goods[index]->GetRelatedItem() == 0) {
            ProcessError(user,
                         0x11u,
                         group_index,
                         item.goods_no,
                         static_cast<int>(index + 1u),
                         static_cast<int>(item_count),
                         static_cast<char>(message.gift));
            goods[index] = 0;
            continue;
        }

        if (goods[index]->GetItemIdx() == 195) {
            user->isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(33));
        } else if (IsPvpRestrictedCeraItem(goods[index]->GetItemIdx())) {
            GameWorld* const game_world = G_GameWorld();
            if (game_world && game_world->IsPVPChannel() &&
                (!game_world->IsPvPSkilTreeChannel() ||
                 user->get_pvp_grade() <= 9)) {
                ProcessError(user,
                             7u,
                             group_index,
                             item.goods_no,
                             static_cast<int>(index + 1u),
                             static_cast<int>(item_count),
                             static_cast<char>(message.gift));
                goods[index] = 0;
                continue;
            }
        }

        if (!_CheckBuyingCondition(
                user,
                item.goods_no,
                static_cast<unsigned long>(goods[index]->GetItemIdx()),
                static_cast<unsigned int>(goods[index]->GetStackCount()),
                static_cast<unsigned short>(goods[index]->GetExpiryDate()),
                item.select_type,
                static_cast<unsigned short>(item.avatar_select))) {
            ProcessError(user,
                         0x4fu,
                         group_index,
                         item.goods_no,
                         static_cast<int>(index + 1u),
                         static_cast<int>(item_count),
                         static_cast<char>(message.gift));
            goods[index] = 0;
        }
    }

    if (!_CheckValidCount(goods)) {
        ProcessError(user,
                     1u,
                     -1,
                     message.items[0].goods_no,
                     1,
                     1,
                     static_cast<char>(message.gift));
        return 1;
    }

    user->resetUsedCera();
    int available_gold = user->getCurCharacMoney();
    int available_win_point = user->GetWinPoint();
    const int current_seconds = GlobalData::s_systemTime_.getCurSec();

    for (unsigned int index = 0; index < item_count; ++index) {
        if (!goods[index]) {
            continue;
        }

        const CeraShopBuyItem& item = message.items[index];
        if (item.option_count > kCeraShopMaxBuyItemCount ||
            item.additional_item_count > kCeraShopMaxBuyItemCount) {
            ProcessError(user,
                         0x4fu,
                         goods[index]->GetGroupIdx(),
                         item.goods_no,
                         static_cast<int>(index + 1u),
                         static_cast<int>(item_count),
                         static_cast<char>(message.gift));
            continue;
        }

        int cera_price = 0;
        int gold_price = 0;
        int win_point = 0;
        int avatar_selection_value = 0;
        const int group_index = goods[index]->GetGroupIdx();
        if (group_index != 0) {
            cera_price = goods[index]->GetCeraPrice();
            gold_price = goods[index]->GetGoldPrice();
            win_point = goods[index]->GetWinPoint();
        } else {
            stAvatarTypeSelect_t selection;
            if (!ResolveAvatarPrice(goods[index],
                                    item.avatar_select,
                                    item.select_type,
                                    selection,
                                    cera_price,
                                    gold_price,
                                    win_point)) {
                ProcessError(user,
                             0x17u,
                             group_index,
                             item.goods_no,
                             static_cast<int>(index + 1u),
                             static_cast<int>(item_count),
                             static_cast<char>(message.gift));
                continue;
            }
            avatar_selection_value = selection.usable_period;
        }

        const WongWork::CUserPremium* const premium = user->GetPremiumInfo();
        const int discount_rate = premium ? premium->getCeraShopDiscountRate() : 0;
        if (discount_rate != 0) {
            cera_price -= cera_price * discount_rate / 100;
            gold_price -= gold_price * discount_rate / 100;
            win_point -= win_point * discount_rate / 100;
        }

        int error = 0;
        if (user->GetCera() < cera_price) {
            error = 11;
        } else if (user->getCurCharacInvenR()->get_money() < gold_price) {
            error = 10;
        } else if (user->GetWinPoint() < win_point) {
            error = 12;
        }
        if (gold_price > available_gold) {
            error = 10;
        }
        if (win_point > available_win_point) {
            error = 12;
        }
        if (error != 0) {
            ProcessError(user,
                         static_cast<unsigned short>(error),
                         group_index,
                         item.goods_no,
                         static_cast<int>(index + 1u),
                         static_cast<int>(item_count),
                         static_cast<char>(message.gift));
            continue;
        }

        if (user->IsRestrictedGoods(goods[index]->GetFeaturedIdx())) {
            ProcessError(user,
                         0x14u,
                         group_index,
                         item.goods_no,
                         static_cast<int>(index + 1u),
                         static_cast<int>(item_count),
                         static_cast<char>(message.gift));
            continue;
        }

        available_gold -= gold_price;
        available_win_point -= win_point;

        CStreamGuard guard(
            GlobalData::s_stream_pool->Acquire("CeraShop.cpp", 891), false);
        stCeraShopIPGRequest_t& request =
            *guard.GetInBuffer<stCeraShopIPGRequest_t>();
        request.goods_no = item.goods_no;
        request.group_index = static_cast<uint32_t>(group_index);
        request.item_index = static_cast<uint32_t>(goods[index]->GetItemIdx());
        request.stack_count = static_cast<uint16_t>(goods[index]->GetStackCount());
        request.cera_price = cera_price;
        request.win_point = win_point;
        request.gold_price = gold_price;
        request.avatar_selection_value = avatar_selection_value;
        request.item_count = item_count;
        request.item_order = index + 1u;
        request.featured_index =
            static_cast<uint32_t>(goods[index]->GetFeaturedIdx());
        request.select_type = item.select_type;
        request.avatar_select = item.avatar_select;
        request.current_seconds = current_seconds;
        request.gift = message.gift;
        request.post_recipient_wire_word = message.post_recipient_wire_word;
        request.buy_type = message.buy_type;
        std::memcpy(request.recipient_name_suffix,
                    message.receiver_name + 26,
                    sizeof(request.recipient_name_suffix));
        FillRequestOptions(request, item);

        char product_id[20] = {};
        char price[36] = {};
        char description[100] = {};
        char unique_no[21] = {};
        std::snprintf(product_id, sizeof(product_id), "%u", item.goods_no);
        std::snprintf(price, sizeof(price), "%d", cera_price);
        MakePurchaseDescription(goods[index], description, sizeof(description));
        WongWork::CGenUniqueNo::GenUniqueNo(unique_no);

        if (cera_price != 0) {
            GlobalData::s_pIPGHelper->IPGOutput(user,
                                                 product_id,
                                                 price,
                                                 unique_no,
                                                 description,
                                                 user->getCurCharacName(),
                                                 guard.Get());
        } else {
            GlobalData::s_pIPGHelper->IPGOutputSimulation(
                user,
                product_id,
                price,
                unique_no,
                description,
                user->getCurCharacName(),
                guard.Get());
        }
    }
    return 1;
}

// IDA_EA: 0x8321D2E
// IDA_NAME: _ZN8WongWork9CCeraShop18ProcessIPGUserDataEP5CUserPNS_3IPG8SIPGDataES5_P6Stream
// DEMANGLED: WongWork::CCeraShop::ProcessIPGUserData(CUser *,WongWork::IPG::SIPGData *,WongWork::IPG::SIPGData *,Stream *)
void WongWork::CCeraShop::ProcessIPGUserData(
    CUser* user,
    IPG::SIPGData* request,
    IPG::SIPGData* result,
    Stream* callback_stream) {
    PacketGuard reply;
    if (!result) {
        return;
    }

    // The query reply is the original six twelve-byte balance fields in the
    // IPG_QUERY union, followed by the cash-data response packet (0, 53).
    if (result->data_type == IPG::IPG_QUERY && user) {
        const IPG::SIPGQueryWire& query = result->wire.query;
        const unsigned int cera = static_cast<unsigned int>(
            std::atoi(query.balance_decimal[5]) +
            std::atoi(query.balance_decimal[4]) +
            std::atoi(query.balance_decimal[3]) +
            std::atoi(query.balance_decimal[2]) +
            std::atoi(query.balance_decimal[1]) +
            std::atoi(query.balance_decimal[0]));
        user->SetCera(cera);
        reply.put_header(0, 53);
        reply.put_byte(1);
        reply.put_int(user->GetCera());
    }

    if (result->data_type != IPG::IPG_OUTPUT) {
        if (result->data_type == IPG::IPG_INPUT) {
            const int input_result =
                std::atoi(result->wire.input.charge_result_decimal);
            if (input_result != 1) {
                reply.put_header(0, 53);
                reply.put_byte(0);
                reply.put_int(0);
            } else {
                int ignored_first = 0;
                int ignored_second = 0;
                if (callback_stream) {
                    *callback_stream >> ignored_first;
                    *callback_stream >> ignored_second;
                }
                if (user && GlobalData::s_pIPGHelper) {
                    GlobalData::s_pIPGHelper->IPGOutputConfirm(request, result);
                }
                return;
            }
        }

        // The original finalizes the initially empty guard for every other
        // non-output data type (including IPG_QUERY after it filled reply).
        if (user) {
            reply.finalize(true);
            user->Send(reply);
        }
        return;
    }

    // The original caller guarantees a live user and callback stream here.
    // Preserve that business gate without dereferencing a stale asynchronous
    // result after logout.
    if (!user || !user->getCurCharacR() || !callback_stream) {
        return;
    }

    stCeraShopIPGRequest_t* const request_record =
        callback_stream->GetOutBuffer<stCeraShopIPGRequest_t>();
    if (!request_record) {
        return;
    }

    const int group_index = static_cast<int>(request_record->group_index);
    const unsigned int goods_no = request_record->goods_no;
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

    uint32_t receiver_member_id = 0;
    std::memcpy(&receiver_member_id,
                request_record->recipient_name_suffix,
                sizeof(receiver_member_id));

    std::vector<Inven_Item> output_items;
    stCeraShopItemParam_t item_param;
    BuildItemParam(*request_record, item_param);

    // CeraShop's original provider uses success code 1.  This differs from
    // the later Japanese vending-machine bridge, whose result convention is
    // zero-success, so do not delegate this owner to that bridge.
    const int billing_result =
        std::atoi(result->wire.output.result_code_decimal);
    if (billing_result != 1) {
        _ProcessIPGError(user,
                         request,
                         result,
                         static_cast<unsigned char>(
                             billing_result == -12 ? 26 : 25),
                         group_index,
                         goods_no,
                         item_order,
                         item_count,
                         gift);
        return;
    }

    int processing_result = 0;
    int creature_item_id = -1;
    int response_group_index = group_index;
    const char* const order_id = GetOutputOrderId(request, result);

    switch (group_index) {
    case 0: {
        if (gift == 1) {
            _processAvatarGift(user,
                                static_cast<int>(item_index),
                                avatar_selection_value,
                                static_cast<char>(select_type),
                                static_cast<char>(avatar_select),
                                order_id,
                                receiver_member_id,
                                receiver_charac_no);
            break;
        }

        const int slot = user->getCurCharacInvenW()->AddAvatarItem(
            static_cast<int>(item_index),
            avatar_selection_value,
            false,
            static_cast<char>(select_type),
            static_cast<char>(avatar_select),
            order_id,
            static_cast<eAvatarItemAddReason>(0),
            false,
            0);
        if (slot >= 0) {
            user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                                     static_cast<ENUM_ITEMSPACE>(1),
                                     slot);

            WongWork::CEventMgr event_mgr;
            if (event_mgr.IsChildrensDayEventing()) {
                CCeraShopGoods* const goods = G_CDataManager()->FindGoods(
                    static_cast<int>(goods_no));
                CItem* const item = G_CDataManager()->find_item(
                    static_cast<int>(item_index));
                if (goods && item && item->vtable_ &&
                    item->vtable_->get_item_type &&
                    goods->GetSticker() == 22) {
                    user->SetChildrensDayEventValue(
                        item->vtable_->get_item_type(item),
                        slot,
                        avatar_selection_value);
                }
            }
        } else {
            const char* const mail_text = g_scriptStringManager_.findString(
                4, "game_server_msg_104", 0);
            WongWork::CMailBoxHelper::ReqDBSendNewAvatarMail(
                user,
                user->getCurCharacNo(),
                static_cast<int>(item_index),
                avatar_selection_value,
                static_cast<char>(select_type),
                static_cast<char>(avatar_select),
                order_id,
                mail_text,
                static_cast<int>(std::strlen(mail_text)));
        }
        break;
    }

    case 1:
        if (gift == 1) {
            _processCoinGift(user,
                             stack_count,
                             receiver_member_id,
                             receiver_charac_no);
        } else {
            _processCoin(user, stack_count, true);
        }
        break;

    case 2:
    case 7:
    case 9: {
        CItem* const item = G_CDataManager()->find_item(
            static_cast<int>(item_index));
        if (!item || !item->vtable_ || !item->vtable_->make_item) {
            processing_result = 17;
            break;
        }

        Inven_Item inventory_item;
        item->vtable_->make_item(item, inventory_item);
        inventory_item.item_id_ = static_cast<int32_t>(item_index);
        inventory_item.add_info_ = static_cast<int32_t>(stack_count);
        if (gift == 1) {
            _processItemGift(user,
                             inventory_item,
                             receiver_member_id,
                             receiver_charac_no);
        } else {
            _processItem(user,
                         inventory_item,
                         featured_index,
                         processing_result,
                         current_seconds,
                         false,
                         true,
                         output_items,
                         item_param);
        }
        break;
    }

    case 4: {
        CItem* const item = G_CDataManager()->find_item(
            static_cast<int>(item_index));
        if (!item || !item->vtable_ || !item->vtable_->make_item) {
            processing_result = 17;
            break;
        }

        Inven_Item inventory_item;
        item->vtable_->make_item(item, inventory_item);
        inventory_item.item_id_ = static_cast<int32_t>(item_index);
        inventory_item.add_info_ = static_cast<int32_t>(stack_count);
        if (ProcessSpecialItem(user,
                               static_cast<int>(item_index),
                               featured_index,
                               processing_result,
                               current_seconds,
                               false,
                               output_items,
                               item_param,
                               0,
                               0,
                               false,
                               1) != 1) {
            const int slot = user->getCurCharacInvenW()->insertItemIntoInventory(
                inventory_item,
                static_cast<eItemAddReason>(30),
                true,
                false);
            if (slot < 0) {
                processing_result = 4;
            } else {
                user->SendUpdateItemList(
                    static_cast<CUser::eSendTarget>(1),
                    static_cast<ENUM_ITEMSPACE>(0),
                    slot);
                user->SaveInventory();
            }
        }
        break;
    }

    case 3: {
        CItem* const item = G_CDataManager()->find_item(
            static_cast<int>(item_index));
        if (!item || !item->vtable_ || !item->vtable_->make_item) {
            processing_result = 17;
            break;
        }

        Inven_Item creature_item;
        item->vtable_->make_item(item, creature_item);
        creature_item.item_id_ = static_cast<int32_t>(item_index);
        creature_item.add_info_ = static_cast<int32_t>(stack_count);
        if (gift == 1) {
            _processCreatureGift(user,
                                 creature_item,
                                 receiver_member_id,
                                 receiver_charac_no,
                                 item->getUsablePeriod(),
                                 item->getExpirationDate());
        } else {
            _processCreature(user,
                             creature_item,
                             creature_item_id,
                             processing_result,
                             true,
                             item->getUsablePeriod(),
                             item->getExpirationDate());
        }
        break;
    }

    case 10: {
        _processAdditionalItem(user,
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

        CItemLimitEditionMgr* const limit_manager =
            G_CDataManager()->getItemLimitEditionMgr();
        CItemLimitEdition* const limit_item = limit_manager
            ? limit_manager->getItemInfo(goods_no)
            : 0;
        if (!limit_item) {
            processing_result = 17;
            break;
        }

        int effective_featured_index = limit_item->getFeaturedIdx();
        if (effective_featured_index == 0) {
            int special_type = 0;
            int special_value = 0;
            bool unique = false;
            if (IsSpecialItem(limit_item->getRelateditem(),
                              special_type,
                              special_value,
                              unique)) {
                effective_featured_index =
                    G_CDataManager()->get_item_list()->GetRestrictCode(
                        limit_item->getRelateditem());
            }
        }
        if (effective_featured_index != 0) {
            // The original gates on the derived value but persists the
            // CItemLimitEdition's native featured index field.
            SaveFeaturedIdx(user,
                            limit_item->getFeaturedIdx(),
                            GlobalData::s_systemTime_.getCurSec(),
                            0);
        }
        break;
    }

    default:
        // The data manager should never expose an unsupported group.  Do not
        // settle a successful billing reply without delivering its item.
        processing_result = 17;
        break;
    }

    if (processing_result != 0) {
        _ProcessIPGError(user,
                         request,
                         result,
                         static_cast<unsigned char>(processing_result),
                         response_group_index,
                         goods_no,
                         item_order,
                         item_count,
                         gift);
        cMyTrace trace(
            "void WongWork::CCeraShop::ProcessIPGUserData(CUser*, WongWork::IPG::SIPGData*, WongWork::IPG::SIPGData*, Stream*)",
            2389,
            0);
        trace("[%s][%d]",
              "void WongWork::CCeraShop::ProcessIPGUserData(CUser*, WongWork::IPG::SIPGData*, WongWork::IPG::SIPGData*, Stream*)",
              processing_result);
        return;
    }

    if (request_record->cera_price != 0 && GlobalData::s_BreakAwaySys) {
        GlobalData::s_BreakAwaySys->updateUsedCera(
            static_cast<unsigned int>(user->get_acc_id()),
            static_cast<unsigned int>(request_record->cera_price));
    }
    if (request_record->gold_price != 0) {
        CCirculationStatistic* const statistic =
            GetInstanceCirculationStatistic();
        if (statistic) {
            statistic->AddCirculationStatistic(
                static_cast<CIRCULATION_STATISTIC_FIELD>(38),
                user,
                static_cast<unsigned int>(request_record->gold_price));
        }
    }
    user->getCurCharacInvenW()->use_money(
        request_record->gold_price,
        static_cast<eMoneySubReason>(8),
        true);
    user->UseWinPoint(request_record->win_point,
                      static_cast<eWPSubReason>(2));
    user->UseCera(static_cast<unsigned int>(request_record->cera_price));
    user->addUsedCera(static_cast<unsigned int>(request_record->cera_price));

    if (featured_index != 0 && !user->IsRestrictedGoods(featured_index)) {
        user->SetRestrictedGoods(featured_index);
        SaveFeaturedIdx(user, featured_index, current_seconds, 0);
    }

    reply.put_header(1, ENUM_CMDPACKET_BUY_CERASHOP_ITEM);
    reply.put_byte(1);
    reply.put_byte(gift);
    reply.put_int(response_group_index);
    reply.put_int(static_cast<int>(goods_no));
    reply.put_int(item_order);
    reply.put_int(item_count);
    reply.put_int(creature_item_id);
    reply.put_short(static_cast<int>(output_items.size()));
    for (std::vector<Inven_Item>::const_iterator item = output_items.begin();
         item != output_items.end();
         ++item) {
        reply.put_int(item->item_id_);
        reply.put_int(item->add_info_);
    }
    if (gift == 1 && receiver_member_id != 0u) {
        DB_UpdateGiftCount::makeRequest(receiver_member_id, 1);
    }

    if (item_count == item_order) {
        SendChildrensDayReward(this, user, item_param);

        unsigned long bonus_item_index = static_cast<unsigned long>(-1);
        unsigned int bonus_item_count = 0u;
        AppendBonusItem(this,
                        user,
                        item_param,
                        bonus_item_index,
                        bonus_item_count);
        reply.put_int(static_cast<int>(bonus_item_index));
        reply.put_int(static_cast<int>(bonus_item_count));

        PacketGuard cash_packet;
        cash_packet.put_header(0, 53);
        cash_packet.put_byte(1);
        cash_packet.put_int(user->GetCera());
        cash_packet.finalize(true);
        user->Send(cash_packet);

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
    }

    reply.finalize(true);
    user->Send(reply);
}

// IDA_EA: 0x83277DC
// IDA_NAME: _ZN8WongWork9CCeraShop18_sendPremiumPacketEP5CUseriii
// DEMANGLED: WongWork::CCeraShop::_sendPremiumPacket(CUser *,int,int,int)
void WongWork::CCeraShop::_sendPremiumPacket(CUser* user,
                                             int special_item_type,
                                             int premium_type,
                                             int interval_seconds) {
    if (!user) {
        return;
    }
    PacketGuard packet;
    packet.put_header(0, 66);
    packet.put_short(static_cast<short>(special_item_type));
    packet.put_byte(static_cast<char>(premium_type));
    packet.put_int(interval_seconds);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x8327BB6
// IDA_NAME: _ZN8WongWork9CCeraShop13_ClosePremiumEP5CUseri
// DEMANGLED: WongWork::CCeraShop::_ClosePremium(CUser *,int)
void WongWork::CCeraShop::_ClosePremium(CUser* user, int premium_type) {
    if (!user || premium_type <= 0 || premium_type > 105) {
        return;
    }

    WongWork::CUserPremium* const premium_info = user->GetPremiumInfo();
    const WongWork::SUserPremiumInfo* const entry =
        premium_info ? premium_info->GetPremiumInfo(premium_type) : 0;
    if (!entry || entry->premium_type == 0) {
        return;
    }

    const long now = GlobalData::s_systemTime_.getCurSec();
    const long end_time = now + 1;
    int target_server = 0;
    if (entry->premium_type > 0 && entry->premium_type <= 105 &&
        g_SPremiumInfo[entry->premium_type].apply_target_server == 1) {
        target_server = G_CEnvironment()->get_server_group();
    }

    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire("CeraShop.cpp", 8423), true);
    guard << 84;
    guard << user->GetUID();
    guard << user->get_acc_id();
    guard << entry->premium_type;
    guard << static_cast<int>(now);
    guard << static_cast<int>(end_time);
    guard << entry->param;
    guard << target_server;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);

    WongWork::CHandlePremium::handleSetUserPremium(
        user,
        static_cast<ENUM_PREMIUM_TYPE>(entry->premium_type),
        now,
        end_time,
        entry->param,
        false);
    WongWork::CHandlePremium::handlePremiumEffect(
        user, static_cast<ENUM_PREMIUM_TYPE>(entry->premium_type));
    WongWork::CHandlePremium::handleNotifyPremiumInfo(
        user, entry->premium_type, 0);
}
