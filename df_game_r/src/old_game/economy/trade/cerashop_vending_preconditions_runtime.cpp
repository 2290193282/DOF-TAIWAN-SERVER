#include "old_game/economy/shop/npc_runtime.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <utility>
#include <vector>

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "core/config/env/environment.h"
#include "common/manager/data_manager.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "common/manager/ipg_runtime.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/premium_public_abi.h"
#include "common/manager/user_history_log_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/stackable/stackable_script_types.h"
#include "old_data/db/event/db_req_save_crazy_levelup_event.h"
#include "old_data/db/event/db_update_chuseok_package_purchase.h"
#include "old_data/db/inventory/db_update_inventory_capacity.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/activity/schedule/event_mgr_runtime.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/gen_unique_no.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/economy/mall/cerashop_special_item_handler.h"
#include "old_game/progression/career/premium_lethe_manager.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/premium_runtime.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"
#include "old_data/PVF/equipment/random_option_item_handle.h"

namespace {

// The DB dispatcher consumes the six values after the signal/user-id stream
// prefix as the exact 24-byte member_premium transaction payload.
void SavePremiumInfo(CUser* user,
                     int premium_type,
                     long start_time,
                     long end_time,
                     int restrict_code,
                     int target_server) {
    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire("SpecialItemHandler.cpp", 661),
        true);
    *guard << 84;
    *guard << user->GetUID();
    *guard << user->get_acc_id();
    *guard << premium_type;
    *guard << static_cast<int>(start_time);
    *guard << static_cast<int>(end_time);
    *guard << restrict_code;
    *guard << target_server;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}

int PremiumTargetServer(int premium_type) {
    if (premium_type > 0 && premium_type <= 105 &&
        g_SPremiumInfo[premium_type].apply_target_server == 1) {
        return G_CEnvironment()->get_server_group();
    }
    return 0;
}

long PremiumStartTime(CUser* user, int premium_type, long fallback) {
    const std::vector<WongWork::SUserPremiumInfo> entries =
        user->GetPremiumInfo()->GetPremiumInfoList(premium_type);
    return entries.empty() ? fallback : entries.front().start_time;
}

long PremiumEndTime(CUser* user,
                    int premium_type,
                    long start_time,
                    bool vending_machine,
                    int term_days) {
    long end_time = start_time + 86400L * term_days;
    if (vending_machine) {
        const std::vector<WongWork::SUserPremiumInfo> entries =
            user->GetPremiumInfo()->GetPremiumInfoList(premium_type);
        if (!entries.empty()) {
            end_time = entries.front().end_time + 86400L * term_days;
        }
    }
    if (premium_type > 0 && premium_type <= 105 &&
        g_SPremiumInfo[premium_type].expire_time > start_time) {
        end_time = g_SPremiumInfo[premium_type].expire_time;
    }
    return end_time;
}

}  // namespace

// IDA_EA: 0x8234EC2
// IDA_NAME: _ZN19STSpecailItem_ParamC2ERSt6vectorI10Inven_ItemSaIS1_EERN8WongWork9CCeraShop21stCeraShopItemParam_tE
// DEMANGLED: STSpecailItem_Param::STSpecailItem_Param(std::vector<Inven_Item,std::allocator<Inven_Item>> &,WongWork::CCeraShop::stCeraShopItemParam_t &)
STSpecailItem_Param::STSpecailItem_Param(
    std::vector<Inven_Item>& output_items_arg,
    WongWork::CCeraShop::stCeraShopItemParam_t& item_param_arg)
    : user(0),
      item_id(0),
      special_value(0),
      output_items(&output_items_arg),
      item_param(&item_param_arg),
      restrict_code(0),
      current_seconds(0),
      is_vending_machine(false),
      first_flag(0),
      second_flag(0),
      third_flag(false),
      final_flag(1) {}

// IDA_EA: 0x8234F36
// IDA_NAME: _ZN20STSpecailItem_ResultC2Ev
// DEMANGLED: STSpecailItem_Result::STSpecailItem_Result(void)
STSpecailItem_Result::STSpecailItem_Result() : result(0) {}

// IDA_EA: 0x827BCEC
// IDA_NAME: _ZN18SpecialItemHandlerC2EPN8WongWork9CCeraShopE
// DEMANGLED: SpecialItemHandler::SpecialItemHandler(WongWork::CCeraShop *)
SpecialItemHandler::SpecialItemHandler(WongWork::CCeraShop* cera_shop)
    : cera_shop_(cera_shop), response_slots_() {
    response_slots_[1].response = &SpecialItemHandler::respond_special_item_cargo;
    response_slots_[7].response =
        &SpecialItemHandler::respond_special_item_cargo_abs;
    response_slots_[11].response =
        &SpecialItemHandler::respond_special_item_inventory_upgrade;
    response_slots_[8].response =
        &SpecialItemHandler::respond_special_item_charac_slot_extend;
    response_slots_[5].response =
        &SpecialItemHandler::respond_special_item_package;
    response_slots_[6].response =
        &SpecialItemHandler::respond_special_item_booster;
    response_slots_[12].response =
        &SpecialItemHandler::respond_special_item_booster_selection;
    response_slots_[9].response =
        &SpecialItemHandler::respond_special_item_skill_confirm;
    response_slots_[10].response =
        &SpecialItemHandler::respond_special_item_skill_tree_style;
    response_slots_[2].response =
        &SpecialItemHandler::respond_special_item_premium;
    response_slots_[13].response =
        &SpecialItemHandler::respond_special_item_cerapoint_giftcard;
}

// IDA_EA: 0x8328284
// IDA_NAME: _ZN18SpecialItemHandlerD2Ev
// DEMANGLED: SpecialItemHandler::~SpecialItemHandler()
SpecialItemHandler::~SpecialItemHandler() {}

// IDA_EA: 0x827BE0A
// IDA_NAME: _ZN18SpecialItemHandler7executeEiRK19STSpecailItem_ParamR20STSpecailItem_Result
// DEMANGLED: SpecialItemHandler::execute(int,STSpecailItem_Param const&,STSpecailItem_Result&)
int SpecialItemHandler::execute(int special_item_type,
                                const STSpecailItem_Param& param,
                                STSpecailItem_Result& result) {
    if (special_item_type > 13) {
        return 0;
    }

    (this->*response_slots_[special_item_type].response)(special_item_type,
                                                          param,
                                                          result);
    return 1;
}

// IDA_EA: 0x827D3DC
// IDA_NAME: _ZN18SpecialItemHandler28respond_special_item_premiumEiRK19STSpecailItem_ParamR20STSpecailItem_Result
// DEMANGLED: SpecialItemHandler::respond_special_item_premium(int,STSpecailItem_Param const&,STSpecailItem_Result &)
int SpecialItemHandler::respond_special_item_premium(
    int special_item_type,
    const STSpecailItem_Param& param,
    STSpecailItem_Result& result) {
    (void)result;
    std::map<unsigned long, STPremiumItemData>::const_iterator item_it =
        g_map_premium_item_data.find(
            static_cast<unsigned long>(static_cast<unsigned int>(param.item_id)));
    if (item_it == g_map_premium_item_data.end()) {
        LogManager::logFormat(
            1,
            "SpecialItemHandler.cpp",
            "int SpecialItemHandler::respond_special_item_premium(int, const STSpecailItem_Param&, STSpecailItem_Result&)",
            767,
            "PREMIUM_ERROR : NO PREMIUM ITEM!! : ITEM_ID (%u)",
            static_cast<unsigned int>(param.item_id));
        return 0;
    }

    CUser* const user = param.user;
    const STPremiumItemData& item_data = item_it->second;
    int premium_type = static_cast<int>(item_data.premium_type);
    int restrict_code = param.restrict_code;
    bool vending_machine = param.is_vending_machine;
    long time_seed = param.current_seconds;
    int term_days = static_cast<int>(item_data.term) * param.final_flag;
    if (param.third_flag) {
        term_days *= G_CDataManager()->GetCeraShopEventMultiplier();
    }

    switch (premium_type) {
    case 1:
    case 3: {
        WongWork::CEventMgr event_mgr;
        term_days += event_mgr.GetPremiumServiceBonus() + 1;
        break;
    }
    case 8:
        vending_machine = true;
        break;
    case 13:
        if (!user->isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(13)) &&
            user->isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(14))) {
            const std::vector<WongWork::SUserPremiumInfo> entries =
                user->GetPremiumInfo()->GetPremiumInfoList(14);
            if (!entries.empty()) {
                time_seed = entries.front().end_time;
            }
        }
        break;
    case 14:
        if (user->isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(13))) {
            premium_type = 13;
            restrict_code = G_CDataManager()->get_item_list()->GetRestrictCode(
                param.item_id);
        }
        break;
    case 15: {
        switch (param.item_id) {
        case 200:
            term_days = 30;
            break;
        case 203:
            term_days = 3;
            break;
        case 204:
            term_days = 7;
            break;
        case 205:
            term_days = 14;
            break;
        default:
            break;
        }

        CInventory* const inventory = user->getCurCharacInvenR();
        if (static_cast<unsigned int>(inventory->GetCoin()) <= 2u) {
            const int total_coin = inventory->GetCoin() +
                                   inventory->GetPayCoin() +
                                   inventory->GetEventCoin();
            const int add_count = std::min(3, 100000 - total_coin);
            if (add_count > 0) {
                user->getCurCharacInvenW()->UpdateDailyCoin(add_count);
                CUser_history_log(user)->CoinAdd(
                    user->getCurCharacInvenR()->GetCoin(),
                    add_count,
                    static_cast<eCoinAddReason>(4));
            }
        }
        break;
    }
    case 22:
    case 27:
    case 29:
        restrict_code = G_CDataManager()->get_item_list()->GetRestrictCode(
            static_cast<int>(item_data.target_item_id));
        vending_machine = true;
        break;
    case 33: {
        if (!item_data.restrict_flag && restrict_code != 0 &&
            !user->IsRestrictedGoods(restrict_code)) {
            user->SetRestrictedGoods(restrict_code);
            cera_shop_->SaveFeaturedIdx(user,
                                         restrict_code,
                                         param.current_seconds,
                                         PremiumTargetServer(premium_type));
        }
        restrict_code = G_CDataManager()->get_item_list()->GetRestrictCode(
            static_cast<int>(item_data.target_item_id));
        vending_machine = true;

        CPremiumLetheManager* const lethe_manager =
            G_CGameManager()->GetPremiumLetheManager();
        if (lethe_manager && !user->IsEndLetheAtLogin()) {
            lethe_manager->BackupSkill(user, ENUM_SKILL_TREE_0);
            lethe_manager->BackupSkill(user, ENUM_SKILL_TREE_1);
        }
        break;
    }
    case 79:
    case 83:
    case 87:
    case 92:
        restrict_code = G_CDataManager()->get_item_list()->GetRestrictCode(
            static_cast<int>(item_data.target_item_id));
        vending_machine = true;
        if (premium_type == 83 &&
            user->isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(87))) {
            cera_shop_->_ClosePremium(user, 87);
        } else if (premium_type == 87 &&
                   user->isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(83))) {
            cera_shop_->_ClosePremium(user, 83);
        }
        break;
    default:
        if (restrict_code == 0) {
            restrict_code = G_CDataManager()->get_item_list()->GetRestrictCode(
                param.item_id);
        }
        break;
    }

    if (restrict_code != 0 && !user->IsRestrictedGoods(restrict_code)) {
        user->SetRestrictedGoods(restrict_code);
        cera_shop_->SaveFeaturedIdx(user,
                                     restrict_code,
                                     param.current_seconds,
                                     PremiumTargetServer(premium_type));
    }

    long start_time = time_seed;
    if (vending_machine) {
        start_time = PremiumStartTime(user, premium_type, time_seed);
    }
    const long end_time = PremiumEndTime(user,
                                         premium_type,
                                         start_time,
                                         vending_machine,
                                         term_days);
    SavePremiumInfo(user,
                    premium_type,
                    start_time,
                    end_time,
                    restrict_code,
                    PremiumTargetServer(premium_type));
    WongWork::CHandlePremium::handleSetUserPremium(
        user,
        static_cast<ENUM_PREMIUM_TYPE>(premium_type),
        start_time,
        end_time,
        restrict_code,
        true);

    if (premium_type == 83 || premium_type == 87) {
        user->ReCalcChattingEmoticon();
        user->SendChattingEmoticon();
    } else if (premium_type == 8) {
        PacketGuard packet;
        packet.put_header(0, 2);
        packet.put_byte(0);
        packet.put_short(1);
        user->make_basic_info(packet, 0);
        packet.finalize(true);
        G_GameWorld()->send_all(packet, user);
    }

    const std::vector<WongWork::SUserPremiumInfo> entries =
        user->GetPremiumInfo()->GetPremiumInfoList(premium_type);
    bool already_notified = false;
    for (std::vector<WongWork::SUserPremiumInfo>::const_iterator it =
             entries.begin();
         it != entries.end();
         ++it) {
        if (it->state == 2) {
            already_notified = true;
            break;
        }
    }
    if (!already_notified) {
        cera_shop_->_sendPremiumPacket(user,
                                       special_item_type,
                                       premium_type,
                                       end_time - time_seed);
    }
    return 0;
}

// IDA_EA: 0x83202B8
// IDA_NAME: _ZN8WongWork9CCeraShopC2Ev
// DEMANGLED: WongWork::CCeraShop::CCeraShop(void)
WongWork::CCeraShop::CCeraShop()
    : special_item_handler_(new SpecialItemHandler(this)), original_item(0) {}

// IDA_EA: 0x8320314
// IDA_NAME: _ZN8WongWork9CCeraShopD2Ev
// DEMANGLED: WongWork::CCeraShop::~CCeraShop()
WongWork::CCeraShop::~CCeraShop() {
    delete special_item_handler_;
}

// IDA_EA: 0x8325854
// IDA_NAME: _ZN8WongWork9CCeraShop22CallSpecialItemHandlerEiRK19STSpecailItem_ParamR20STSpecailItem_Result
// DEMANGLED: WongWork::CCeraShop::CallSpecialItemHandler(int,STSpecailItem_Param const&,STSpecailItem_Result&)
int WongWork::CCeraShop::CallSpecialItemHandler(
    int special_item_type,
    const STSpecailItem_Param& param,
    STSpecailItem_Result& result) {
    return special_item_handler_->execute(special_item_type, param, result);
}

// IDA_EA: 0x827BE8A
// IDA_NAME: _ZN18SpecialItemHandler26respond_special_item_cargoEiRK19STSpecailItem_ParamR20STSpecailItem_Result
// DEMANGLED: SpecialItemHandler::respond_special_item_cargo(int,STSpecailItem_Param const&,STSpecailItem_Result&)
int SpecialItemHandler::respond_special_item_cargo(
    int special_item_type,
    const STSpecailItem_Param& param,
    STSpecailItem_Result& result) {
    CCargo* const cargo = param.user->getCurCharacCargoW();
    if (cargo->UpgradeCargo(param.special_value) == 1) {
        cera_shop_->_sendCargoUpgradePacket(param.user, special_item_type);
    } else {
        result.result = 23;
    }
    return 0;
}

// IDA_EA: 0x827BEE6
// IDA_NAME: _ZN18SpecialItemHandler30respond_special_item_cargo_absEiRK19STSpecailItem_ParamR20STSpecailItem_Result
// DEMANGLED: SpecialItemHandler::respond_special_item_cargo_abs(int,STSpecailItem_Param const&,STSpecailItem_Result&)
int SpecialItemHandler::respond_special_item_cargo_abs(
    int special_item_type,
    const STSpecailItem_Param& param,
    STSpecailItem_Result& result) {
    const int requested_capacity =
        param.user->getCurCharacCargoR()->GetCapacity() + param.special_value;
    if (param.user->getCurCharacCargoR()->isUpgradable(requested_capacity) ==
        1) {
        param.user->getCurCharacCargoW()->UpgradeCargo(requested_capacity);
        cera_shop_->_sendCargoUpgradePacket(param.user, special_item_type);
    } else {
        result.result = 18;
    }
    return 0;
}

// IDA_EA: 0x827BF90
// IDA_NAME: _ZN18SpecialItemHandler38respond_special_item_inventory_upgradeEiRK19STSpecailItem_ParamR20STSpecailItem_Result
// DEMANGLED: SpecialItemHandler::respond_special_item_inventory_upgrade(int,STSpecailItem_Param const&,STSpecailItem_Result&)
int SpecialItemHandler::respond_special_item_inventory_upgrade(
    int special_item_type,
    const STSpecailItem_Param& param,
    STSpecailItem_Result& result) {
    CInventory* const inventory = param.user->getCurCharacInvenR();
    const int current_capacity = inventory->get_inventory_capacity();
    if (current_capacity > 15) {
        result.result = 18;
        return 0;
    }

    const int new_capacity = current_capacity + param.special_value;
    CUser_history_log(param.user)->ExpandInventory(current_capacity,
                                                   new_capacity,
                                                   0);
    inventory->set_inventory_capacity(new_capacity);
    DB_UpdateInventoryCapacity::makeRequest(
        static_cast<ENUM_SERVER_GROUP>(param.user->GetServerGroup()),
        param.user->GetUID(),
        static_cast<unsigned int>(param.user->getCurCharacNo()),
        inventory->get_inventory_capacity());
    cera_shop_->_sendInventoryUpgradePacket(param.user, special_item_type);
    return 0;
}

// IDA_EA: 0x827C0C2
// IDA_NAME: _ZN18SpecialItemHandler39respond_special_item_charac_slot_extendEiRK19STSpecailItem_ParamR20STSpecailItem_Result
// DEMANGLED: SpecialItemHandler::respond_special_item_charac_slot_extend(int,STSpecailItem_Param const&,STSpecailItem_Result&)
int SpecialItemHandler::respond_special_item_charac_slot_extend(
    int special_item_type,
    const STSpecailItem_Param& param,
    STSpecailItem_Result& result) {
    (void)result;
    param.user->incCharacSlotLimit(
        static_cast<unsigned char>(param.special_value));

    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire("SpecialItemHandler.cpp", 141),
        true);
    *guard << 650;
    *guard << -1;
    SIG_EXTEND_CHARAC_SLOT* const request =
        guard->GetInBuffer<SIG_EXTEND_CHARAC_SLOT>();
    request->account_id = static_cast<unsigned int>(param.user->get_acc_id());
    request->slot_effect_count =
        static_cast<unsigned char>(param.user->getSlotEffectCount());
    request->charac_slot_limit =
        static_cast<unsigned char>(param.user->getCharacSlotLimit());
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    cera_shop_->_sendCharacSlotExtendPacket(param.user, special_item_type);
    return 0;
}

// IDA_EA: 0x827C204
// IDA_NAME: _ZN18SpecialItemHandler28respond_special_item_packageEiRK19STSpecailItem_ParamR20STSpecailItem_Result
// DEMANGLED: SpecialItemHandler::respond_special_item_package(int,STSpecailItem_Param const&,STSpecailItem_Result &)
int SpecialItemHandler::respond_special_item_package(
    int,
    const STSpecailItem_Param& param,
    STSpecailItem_Result&) {
    const unsigned int package_item_id = static_cast<unsigned int>(param.item_id);
    CUser* const user = param.user;

    if (package_item_id == 27u) {
        CStreamGuard guard(
            GlobalData::s_stream_pool->Acquire("SpecialItemHandler.cpp", 195),
            true);
        *guard << 180;
        *guard << -1;
        SIG_SAVE_CRAZY_LEVELUP_EVENT* const request =
            guard->GetInBuffer<SIG_SAVE_CRAZY_LEVELUP_EVENT>();
        std::memcpy(request->event_id, "0612", sizeof(request->event_id));
        request->m_id = static_cast<uint32_t>(user->get_acc_id());
        request->charac_no = static_cast<uint32_t>(user->get_charac_no(-1));
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, guard);
    }

    cera_shop_->_onBuySpecialItem(
        user, static_cast<ENUM_CERA_SPECIALITEM>(5), package_item_id);

    const CStackableItem* const package_item =
        static_cast<const CStackableItem*>(
            G_CDataManager()->find_item(static_cast<int>(package_item_id)));
    std::vector<STStackablePackageData> rewards(package_item->package_data());

    std::vector<tagPackageItemSelection>& selections =
        param.item_param->package_item_selections;
    const std::vector<std::vector<STStackablePackageData> >& package_choices =
        package_item->package_selection_data();
    if (!selections.empty()) {
        if (selections.size() > 1u) {
            for (std::vector<tagPackageItemSelection>::const_iterator it =
                     selections.begin();
                 it != selections.end();
                 ++it) {
                LogManager::logFormat(
                    1,
                    "SpecialItemHandler.cpp",
                    "void SpecialItemHandler::respond_special_item_package(int, const STSpecailItem_Param&, STSpecailItem_Result&)",
                    219,
                    "kdd372418Log::Package Selection item error (%s,%d,%d,%d,%d)",
                    NumberToString(static_cast<unsigned int>(user->get_acc_id()),
                                   0),
                    user->get_charac_no(-1),
                    static_cast<int>(it->item_index),
                    static_cast<int>(it->package_index),
                    static_cast<int>(it->selection_index));
            }
        }

        for (std::vector<tagPackageItemSelection>::iterator it =
                 selections.begin();
             it != selections.end();) {
            if (it->item_index != package_item_id) {
                ++it;
                continue;
            }

            if (it->package_index >= package_choices.size() ||
                it->selection_index >=
                    package_choices[it->package_index].size()) {
                LogManager::logFormat(
                    1,
                    "SpecialItemHandler.cpp",
                    "void SpecialItemHandler::respond_special_item_package(int, const STSpecailItem_Param&, STSpecailItem_Result&)",
                    231,
                    "Package Selection error (%s,%d,%d,%d,%d)",
                    NumberToString(static_cast<unsigned int>(user->get_acc_id()),
                                   0),
                    user->get_charac_no(-1),
                    static_cast<int>(it->item_index),
                    static_cast<int>(it->package_index),
                    static_cast<int>(it->selection_index));
                break;
            }

            rewards.push_back(
                package_choices[it->package_index].at(it->selection_index));
            it = selections.erase(it);
        }
    }

    for (std::vector<STStackablePackageData>::const_iterator reward =
             rewards.begin();
         reward != rewards.end();
         ++reward) {
        unsigned char avatar_selection = 0;
        for (std::vector<std::pair<int, int> >::iterator selected =
                 param.item_param->selected_items.begin();
             selected != param.item_param->selected_items.end();
             ++selected) {
            if (selected->first == reward->item_index) {
                avatar_selection = static_cast<unsigned char>(selected->second);
                param.item_param->selected_items.erase(selected);
                break;
            }
        }

        CItem* const reward_item =
            G_CDataManager()->find_item(reward->item_index);
        if (reward_item) {
            reward_item->vtable_->set_parent_booster_item_id(
                reward_item, package_item_id);
        }
        cera_shop_->_processAdditionalItem(
            user,
            static_cast<unsigned long>(reward->item_index),
            static_cast<unsigned short>(reward->item_count),
            avatar_selection,
            0xffu,
            *param.output_items,
            *param.item_param,
            0);
        if (reward_item) {
            reward_item->vtable_->set_parent_booster_item_id(reward_item, 0);
        }

        switch (reward->item_index) {
        case 896:
            CUser::SendRandomBoxStatistic(
                0, static_cast<ENUM_RANDOMBOX_ACTION_KIND>(0));
            break;
        case 2660397:
            CUser::SendRandomBoxStatistic(
                1, static_cast<ENUM_RANDOMBOX_ACTION_KIND>(0));
            break;
        case 2660169:
            CUser::SendRandomBoxStatistic(
                2, static_cast<ENUM_RANDOMBOX_ACTION_KIND>(0));
            break;
        default:
            break;
        }
    }

    const int restrict_code =
        G_CDataManager()->get_item_list()->GetRestrictCode(package_item_id);
    if (restrict_code != 0 && user->IsRestrictedGoods(restrict_code) != 1) {
        user->SetRestrictedGoods(restrict_code);
        cera_shop_->SaveFeaturedIdx(
            user, restrict_code, param.current_seconds, 0);
    }
    user->SaveInventory();
    return 0;
}

// IDA_EA: 0x827CA4A
// IDA_NAME: _ZN18SpecialItemHandler28respond_special_item_boosterEiRK19STSpecailItem_ParamR20STSpecailItem_Result
// DEMANGLED: SpecialItemHandler::respond_special_item_booster(int,STSpecailItem_Param const&,STSpecailItem_Result &)
int SpecialItemHandler::respond_special_item_booster(
    int,
    const STSpecailItem_Param& param,
    STSpecailItem_Result&) {
    const unsigned int booster_item_id = static_cast<unsigned int>(param.item_id);
    CUser* const user = param.user;
    cera_shop_->setOriginalItem(booster_item_id);
    cera_shop_->_onBuySpecialItem(
        user, static_cast<ENUM_CERA_SPECIALITEM>(6), booster_item_id);

    const CStackableItem* const booster_item =
        static_cast<const CStackableItem*>(
            G_CDataManager()->find_item(static_cast<int>(booster_item_id)));
    std::vector<stStackableBoosterItemInfo_t> rewards;
    booster_item->getBoosterItem(rewards);
    for (std::vector<stStackableBoosterItemInfo_t>::const_iterator reward =
             rewards.begin();
         reward != rewards.end();
         ++reward) {
        CItem* const reward_item =
            G_CDataManager()->find_item(reward->item_index);
        if (reward_item) {
            reward_item->vtable_->set_parent_booster_item_id(
                reward_item, booster_item_id);
        }
        cera_shop_->_processAdditionalItem(
            user,
            static_cast<unsigned long>(reward->item_index),
            reward->item_count,
            reward->avatar_option_0e,
            reward->avatar_option_0d,
            *param.output_items,
            *param.item_param,
            0);
        if (reward_item) {
            reward_item->vtable_->set_parent_booster_item_id(reward_item, 0);
        }
    }

    const int restrict_code =
        G_CDataManager()->get_item_list()->GetRestrictCode(booster_item_id);
    if (restrict_code != 0 && user->IsRestrictedGoods(restrict_code) != 1) {
        user->SetRestrictedGoods(restrict_code);
        cera_shop_->SaveFeaturedIdx(
            user, restrict_code, param.current_seconds, 0);
    }
    user->SaveInventory();
    return 0;
}

// IDA_EA: 0x827CCFC
// IDA_NAME: _ZN18SpecialItemHandler38respond_special_item_booster_selectionEiRK19STSpecailItem_ParamR20STSpecailItem_Result
// DEMANGLED: SpecialItemHandler::respond_special_item_booster_selection(int,STSpecailItem_Param const&,STSpecailItem_Result&)
int SpecialItemHandler::respond_special_item_booster_selection(
    int,
    const STSpecailItem_Param& param,
    STSpecailItem_Result& result) {
    return cera_shop_->_processBoosterSelect(param.user,
                                             param.item_id,
                                             param.restrict_code,
                                             result.result,
                                             param.current_seconds,
                                             param.is_vending_machine,
                                             *param.output_items,
                                             *param.item_param,
                                             param.first_flag,
                                             param.second_flag);
}

// IDA_EA: 0x8327894
// IDA_NAME: _ZN8WongWork9CCeraShop21_processBoosterSelectEP5CUseriiRilbRSt6vectorI10Inven_ItemSaIS5_EERNS0_21stCeraShopItemParam_tEcc
// DEMANGLED: WongWork::CCeraShop::_processBoosterSelect(CUser *,int,int,int &,long,bool,std::vector<Inven_Item,std::allocator<Inven_Item>> &,WongWork::CCeraShop::stCeraShopItemParam_t &,char,char)
int WongWork::CCeraShop::_processBoosterSelect(
    CUser* user,
    int item_id,
    int restrict_code,
    int& processing_result,
    long current_seconds,
    bool is_vending_machine,
    std::vector<Inven_Item>& output_items,
    stCeraShopItemParam_t& item_param,
    char first_flag,
    char second_flag) {
    CItem* const item = G_CDataManager()->find_item(item_id);
    if (!item) {
        return 1;
    }

    const CStackableItem* const stackable_item =
        static_cast<const CStackableItem*>(item);
    const stStackableBooster_t& booster = stackable_item->booster_data();
    const std::pair<char, char> category(first_flag, second_flag);
    const std::map<std::pair<char, char>,
                   std::vector<stStackableBoosterItemInfo_t> >::const_iterator
        category_items = booster.category_items.find(category);
    if (category_items == booster.category_items.end()) {
        return 0;
    }

    std::vector<stStackableBoosterItemInfo_t> selected_items;
    const std::vector<stStackableBoosterItemInfo_t>& candidates =
        category_items->second;
    for (std::vector<Inven_Item>::const_iterator output = output_items.begin();
         output != output_items.end();
         ++output) {
        for (std::vector<stStackableBoosterItemInfo_t>::const_iterator candidate =
                 candidates.begin();
             candidate != candidates.end();
             ++candidate) {
            if (candidate->item_index == output->item_id_) {
                selected_items.push_back(*candidate);
            }
        }
    }

    output_items.clear();
    for (std::vector<stStackableBoosterItemInfo_t>::const_iterator selected =
             selected_items.begin();
         selected != selected_items.end();
         ++selected) {
        _processAdditionalItem(user,
                               static_cast<unsigned long>(selected->item_index),
                               selected->item_count,
                               selected->avatar_option_0e,
                               selected->avatar_option_0d,
                               output_items,
                               item_param,
                               selected->equipment_upgrade);
    }

    const int featured_index =
        G_CDataManager()->get_item_list()->GetRestrictCode(item_id);
    if (featured_index != 0 && user->IsRestrictedGoods(featured_index) != 1) {
        user->SetRestrictedGoods(featured_index);
        SaveFeaturedIdx(user, featured_index, current_seconds, 0);
    }
    user->SaveInventory();
    return 1;
}

int WongWork::CCeraShop::_processBoosterSelectWithAvatarOptions(
    CUser* user,
    int item_id,
    int restrict_code,
    int& processing_result,
    long current_seconds,
    bool is_vending_machine,
    std::vector<Inven_Item>& output_items,
    stCeraShopItemParam_t& item_param,
    char first_flag,
    char second_flag,
    const std::map<int, uint8_t>& avatar_options) {
    (void)restrict_code;
    (void)is_vending_machine;
    processing_result = 0;

    CItem* const item = G_CDataManager()->find_item(item_id);
    if (!item || !user || !user->getCurCharacR()) {
        return 0;
    }

    const CStackableItem* const stackable_item =
        static_cast<const CStackableItem*>(item);
    const stStackableBooster_t& booster = stackable_item->booster_data();
    const std::pair<char, char> category(first_flag, second_flag);
    const std::map<std::pair<char, char>,
                   std::vector<stStackableBoosterItemInfo_t> >::const_iterator
        category_items = booster.category_items.find(category);
    if (category_items == booster.category_items.end()) {
        return 0;
    }

    std::vector<stStackableBoosterItemInfo_t> selected_items;
    const std::vector<stStackableBoosterItemInfo_t>& candidates =
        category_items->second;
    for (std::vector<Inven_Item>::const_iterator output = output_items.begin();
         output != output_items.end();
         ++output) {
        std::vector<stStackableBoosterItemInfo_t>::const_iterator match =
            candidates.end();
        for (std::vector<stStackableBoosterItemInfo_t>::const_iterator candidate =
                 candidates.begin();
             candidate != candidates.end();
             ++candidate) {
            if (candidate->item_index != output->item_id_) {
                continue;
            }
            if (match != candidates.end()) {
                return 0;
            }
            match = candidate;
        }
        if (match == candidates.end()) {
            return 0;
        }
        selected_items.push_back(*match);
    }

    output_items.clear();
    for (std::vector<stStackableBoosterItemInfo_t>::const_iterator selected =
             selected_items.begin();
         selected != selected_items.end();
         ++selected) {
        unsigned char avatar_option = selected->avatar_option_0e;
        CItem* const reward_item =
            G_CDataManager()->find_item(selected->item_index);
        if (!reward_item) {
            return 0;
        }
        if (reward_item->vtable_->is_avatar_item(reward_item)) {
            if (!avatar_options.empty()) {
                const std::map<int, uint8_t>::const_iterator option =
                    avatar_options.find(selected->item_index);
                if (option == avatar_options.end()) {
                    return 0;
                }
                avatar_option = option->second;
            }
        }

        _processAdditionalItem(user,
                               static_cast<unsigned long>(selected->item_index),
                               selected->item_count,
                               avatar_option,
                               selected->avatar_option_0d,
                               output_items,
                               item_param,
                               selected->equipment_upgrade);
    }

    const int featured_index =
        G_CDataManager()->get_item_list()->GetRestrictCode(item_id);
    if (featured_index != 0 && user->IsRestrictedGoods(featured_index) != 1) {
        user->SetRestrictedGoods(featured_index);
        SaveFeaturedIdx(user, featured_index, current_seconds, 0);
    }
    user->SaveInventory();
    return 1;
}

// IDA_EA: 0x827CDD0
// IDA_NAME: _ZN18SpecialItemHandler34respond_special_item_skill_confirmEiRK19STSpecailItem_ParamR20STSpecailItem_Result
// DEMANGLED: SpecialItemHandler::respond_special_item_skill_confirm(int,STSpecailItem_Param const&,STSpecailItem_Result &)
int SpecialItemHandler::respond_special_item_skill_confirm(
    int special_item_type,
    const STSpecailItem_Param& param,
    STSpecailItem_Result& result) {
    (void)special_item_type;
    (void)result;

    CPremiumLetheManager* const premium_lethe_manager =
        G_CGameManager()->GetPremiumLetheManager();
    CUser* const user = param.user;
    const int skill_tree_index = user->GetCurCharacSkillTreeIndex();
    const int charac_no = user->getCurCharacNo();
    const char* const charac_name = user->getCurCharacName();

    cMyTrace trace(
        "void SpecialItemHandler::respond_special_item_skill_confirm(int, const STSpecailItem_Param&, STSpecailItem_Result&)",
        481,
        0);
    const int trace_result = trace(
        "ONE_DAY_LETHE : BUY_SKILL_CONFIRM_TICKET, char(%s), char_no(%d), style(%d)",
        charac_name,
        charac_no,
        skill_tree_index);
    if (premium_lethe_manager) {
        return premium_lethe_manager->ConfirmSkillReq(user);
    }
    return trace_result;
}

// IDA_EA: 0x827CE7E
// IDA_NAME: _ZN18SpecialItemHandler37respond_special_item_skill_tree_styleEiRK19STSpecailItem_ParamR20STSpecailItem_Result
// DEMANGLED: SpecialItemHandler::respond_special_item_skill_tree_style(int,STSpecailItem_Param const&,STSpecailItem_Result&)
int SpecialItemHandler::respond_special_item_skill_tree_style(
    int special_item_type,
    const STSpecailItem_Param& param,
    STSpecailItem_Result& result) {
    (void)special_item_type;
    (void)result;

    CPremiumLetheManager* const premium_lethe_manager =
        G_CGameManager()->GetPremiumLetheManager();
    CUser* const user = param.user;
    const int charac_no = user->getCurCharacNo();
    const char* const charac_name = user->getCurCharacName();

    cMyTrace trace(
        "void SpecialItemHandler::respond_special_item_skill_tree_style(int, const STSpecailItem_Param&, STSpecailItem_Result&)",
        499,
        0);
    trace("SKILL_TREE_STYLE : BUY_SKILL_TREE_STYLE, char(%s), char_no(%d)",
          charac_name,
          charac_no);

    user->SetCurCharacSkillTreeIndex(0);
    user->UpdateData();
    premium_lethe_manager->InitSkill(user, true, ENUM_SKILL_TREE_1);

    PacketGuard packet;
    packet.put_header(0, 2);
    packet.put_byte(0);
    packet.put_short(1);
    user->make_basic_info(packet, 0);
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x827DE4E
// IDA_NAME: _ZN18SpecialItemHandler39respond_special_item_cerapoint_giftcardEiRK19STSpecailItem_ParamR20STSpecailItem_Result
// DEMANGLED: SpecialItemHandler::respond_special_item_cerapoint_giftcard(int,STSpecailItem_Param const&,STSpecailItem_Result &)
int SpecialItemHandler::respond_special_item_cerapoint_giftcard(
    int special_item_type,
    const STSpecailItem_Param& param,
    STSpecailItem_Result& result) {
    (void)special_item_type;
    (void)result;

    return GlobalData::s_pIPGHelper->IPGInputPoint(
        param.user,
        static_cast<unsigned int>(param.special_value),
        2,
        0,
        0);
}

// IDA_EA: 0x8327454
// IDA_NAME: _ZN8WongWork9CCeraShop17_onBuySpecialItemEP5CUser21ENUM_CERA_SPECIALITEMm
// DEMANGLED: WongWork::CCeraShop::_onBuySpecialItem(CUser *,ENUM_CERA_SPECIALITEM,unsigned long)
int WongWork::CCeraShop::_onBuySpecialItem(
    CUser* user,
    ENUM_CERA_SPECIALITEM special_item_type,
    unsigned long item_id) {
    (void)special_item_type;

    const unsigned int packet_id = static_cast<unsigned int>(item_id);
    if (packet_id - 63u <= 0x0Au &&
        ((1u << (packet_id - 63u)) & 0x7DFu) != 0u) {
        CStreamGuard guard(
            GlobalData::s_stream_pool->Acquire("CeraShop.cpp", 6513), true);
        *guard << 180;
        *guard << -1;

        SIG_SAVE_CRAZY_LEVELUP_EVENT* const request =
            guard->GetInBuffer<SIG_SAVE_CRAZY_LEVELUP_EVENT>();
        std::memcpy(request->event_id,
                    "0707",
                    sizeof(request->event_id));
        request->m_id = static_cast<uint32_t>(user->get_acc_id());
        request->charac_no =
            static_cast<uint32_t>(user->get_charac_no(-1));
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, guard);
    }

    const int is_chuseok_packet = Is2010ChuseokPacket(packet_id);
    if (is_chuseok_packet) {
        DB_UpdateChuseokPackagePurchase::makeRequest(
            static_cast<unsigned int>(user->get_acc_id()));
    }
    return is_chuseok_packet;
}

// IDA_EA: 0x83275BE
// IDA_NAME: _ZN8WongWork9CCeraShop27_sendInventoryUpgradePacketEP5CUseri
// DEMANGLED: WongWork::CCeraShop::_sendInventoryUpgradePacket(CUser *,int)
void WongWork::CCeraShop::_sendInventoryUpgradePacket(CUser* user,
                                                       int special_item_type) {
    PacketGuard packet;
    packet.put_header(0, 66);
    packet.put_short(special_item_type);
    packet.put_short(user->getCurCharacInvenR()->get_inventory_capacity());
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x8327674
// IDA_NAME: _ZN8WongWork9CCeraShop27_sendCharacSlotExtendPacketEP5CUseri
// DEMANGLED: WongWork::CCeraShop::_sendCharacSlotExtendPacket(CUser *,int)
void WongWork::CCeraShop::_sendCharacSlotExtendPacket(
    CUser* user,
    int special_item_type) {
    PacketGuard packet;
    packet.put_header(0, 66);
    packet.put_short(special_item_type);
    packet.put_byte(user->getCharacSlotLimit());
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x8327726
// IDA_NAME: _ZN8WongWork9CCeraShop23_sendCargoUpgradePacketEP5CUseri
// DEMANGLED: WongWork::CCeraShop::_sendCargoUpgradePacket(CUser *,int)
void WongWork::CCeraShop::_sendCargoUpgradePacket(CUser* user,
                                                   int special_item_type) {
    PacketGuard packet;
    packet.put_header(0, 66);
    packet.put_short(special_item_type);
    packet.put_short(user->getCurCharacCargoR()->GetCapacity());
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x854C3E4
// IDA_NAME: _ZN18ItemVendingMachineC2Ev
// DEMANGLED: ItemVendingMachine::ItemVendingMachine(void)
ItemVendingMachine::ItemVendingMachine()
    : cera_shop_(), purchase_count_bonus_() {}

// IDA_EA: 0x854C42A
// IDA_NAME: _ZN18ItemVendingMachineD2Ev
// DEMANGLED: ItemVendingMachine::~ItemVendingMachine()
ItemVendingMachine::~ItemVendingMachine() {}

// IDA_EA: 0x8323AF4
// IDA_NAME: _ZN8WongWork9CCeraShop16_CheckValidCountEPPK14CCeraShopGoods
// DEMANGLED: WongWork::CCeraShop::_CheckValidCount(CCeraShopGoods const **)
int WongWork::CCeraShop::_CheckValidCount(const CCeraShopGoods** goods) {
    int special_item_counts[15] = {};
    int special_type = 0;
    int special_value = 0;

    for (int first = 0; first <= 34 && goods[first]; ++first) {
        for (int second = first + 1; second <= 34 && goods[second]; ++second) {
            if (goods[first]->GetItemIdx() == goods[second]->GetItemIdx()) {
                const int group_index = goods[first]->GetGroupIdx();
                if (group_index == 3) {
                    CItem* const item = G_CDataManager()->find_item(
                        static_cast<unsigned int>(goods[first]->GetItemIdx()));
                    if (static_cast<unsigned char>(item->is_stackable()) != 1u) {
                        return 0;
                    }
                } else if (group_index == 7 || group_index == 0) {
                    return 0;
                }
            }
        }

        bool unique = false;
        if (static_cast<unsigned char>(GlobalData::s_pCeraShop->IsSpecialItem(
                goods[first]->GetItemIdx(),
                special_type,
                special_value,
                unique)) == 1u) {
            ++special_item_counts[special_type];
            if (unique) {
                const int previous_count = special_item_counts[special_type] - 1;
                special_item_counts[special_type] = std::min(
                    1,
                    std::max(special_item_counts[special_type], previous_count));
            }
        }
    }

    for (int special_index = 1; special_index <= 14; ++special_index) {
        if (special_item_counts[special_index] > 1) {
            return 0;
        }
    }

    typedef std::pair<const CCeraShopGoods*, unsigned int> GoodsCount;
    std::vector<GoodsCount> non_stackable_goods;
    for (int index = 0; index <= 34; ++index) {
        const CCeraShopGoods* const current_goods = goods[index];
        if (!current_goods) {
            continue;
        }

        if (static_cast<unsigned char>(current_goods->isStackBuyable()) != 1u ||
            G_CDataManager()->get_item_list()->GetRestrictCode(
                current_goods->GetItemIdx())) {
            std::vector<GoodsCount>::iterator found = non_stackable_goods.begin();
            const std::vector<GoodsCount>::iterator end = non_stackable_goods.end();
            while (found != end) {
                if (found->first == current_goods) {
                    ++found->second;
                    break;
                }
                ++found;
            }
            if (found == end) {
                non_stackable_goods.push_back(
                    std::make_pair(current_goods, 1u));
            }
        }
    }

    for (std::vector<GoodsCount>::const_iterator it = non_stackable_goods.begin();
         it != non_stackable_goods.end();
         ++it) {
        if (it->second > 1u) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x8324810
// IDA_NAME: _ZN8WongWork9CCeraShop21_CheckBuyingConditionEP5CUserPK14CCeraShopGoodsjhtcRi
// DEMANGLED: WongWork::CCeraShop::_CheckBuyingCondition(CUser *,CCeraShopGoods const *,unsigned int,unsigned char,unsigned short,char,int &)
int WongWork::CCeraShop::_CheckBuyingCondition(
    CUser* user,
    const CCeraShopGoods* goods,
    unsigned int ipg_no,
    unsigned char raw_select,
    unsigned short avatar_type_select,
    char skip_coin_cap_check,
    int& error) {
    if (goods->GetGroupIdx() == 1 && !skip_coin_cap_check) {
        CInventory* const inventory = user->getCurCharacInvenR();
        const int current_coin = inventory->GetCoin();
        const int total_coin = inventory->GetPayCoin() + current_coin +
                               inventory->GetEventCoin();
        if (total_coin + goods->GetStackCount() <= 100000) {
            error = 0;
            return 1;
        }

        error = 79;
        return 0;
    }

    const unsigned short expiry_date = static_cast<unsigned short>(
        goods->GetExpiryDate());
    const unsigned int related_item_count = static_cast<unsigned int>(
        goods->GetStackCount());
    const unsigned long related_item = static_cast<unsigned long>(
        goods->GetItemIdx());
    const int allowed = _CheckBuyingCondition(user,
                                               ipg_no,
                                               related_item,
                                               related_item_count,
                                               expiry_date,
                                               raw_select,
                                               avatar_type_select);
    if (allowed != 1) {
        error = 8;
    }
    return allowed;
}

// IDA_EA: 0x8325A88
// IDA_NAME: _ZN8WongWork9CCeraShop12_processCoinEP5CUsertb
// DEMANGLED: WongWork::CCeraShop::_processCoin(CUser *,unsigned short,bool)
void WongWork::CCeraShop::_processCoin(CUser* user,
                                       unsigned short coin_count,
                                       bool) {
    const unsigned int total = static_cast<unsigned int>(
                                   user->getCurCharacInvenR()->GetPayCoin()) +
                               coin_count;
    user->getCurCharacInvenW()->SetPayCoin(total);

    CUser_history_log(user)->PayCoinAdd(
        user->getCurCharacInvenR()->GetPayCoin(),
        coin_count,
        static_cast<eCoinAddReason>(3));
    user->SaveMoneyCoin();
    user->SendCoin();
}

// IDA_EA: 0x832587E
// IDA_NAME: _ZN8WongWork9CCeraShop18ProcessSpecialItemEP5CUseriiRilbRSt6vectorI10Inven_ItemSaIS5_EERNS0_21stCeraShopItemParam_tEccbi
// DEMANGLED: WongWork::CCeraShop::ProcessSpecialItem(CUser *,int,int,int &,long,bool,std::vector<Inven_Item,std::allocator<Inven_Item>> &,WongWork::CCeraShop::stCeraShopItemParam_t &,char,char,bool,int)
int WongWork::CCeraShop::ProcessSpecialItem(
    CUser* user,
    int item_id,
    int restrict_code,
    int& processing_result,
    long current_seconds,
    bool is_vending_machine,
    std::vector<Inven_Item>& output_items,
    stCeraShopItemParam_t& item_param,
    char first_flag,
    char second_flag,
    bool third_flag,
    int final_flag) {
    if (!user->getCurCharacR()) {
        return 0;
    }

    int special_item_type = 0;
    int special_value = 0;
    bool unique = false;
    if (g_map_premium_item_data.find(static_cast<unsigned long>(item_id)) ==
        g_map_premium_item_data.end()) {
        if (IsSpecialItem(item_id, special_item_type, special_value, unique) !=
            1) {
            return 0;
        }
    } else {
        special_item_type = 2;
    }

    if (restrict_code == 0 && (item_id == 16 || item_id == 2675057)) {
        restrict_code = 50000;
    }

    // The original calls this accessor before forming the special-item
    // payload, even though the returned manager is not consumed in this leaf.
    (void)G_CGameManager()->GetPremiumLetheManager();

    STSpecailItem_Param param(output_items, item_param);
    param.user = user;
    param.item_id = item_id;
    param.special_value = special_value;
    param.restrict_code = restrict_code;
    param.current_seconds = current_seconds;
    param.is_vending_machine = is_vending_machine;
    param.first_flag = first_flag;
    param.second_flag = second_flag;
    param.third_flag = third_flag;
    param.final_flag = final_flag;

    STSpecailItem_Result result;
    if (special_item_handler_->execute(special_item_type, param, result) != 1) {
        return 0;
    }

    processing_result = result.result;
    return 1;
}

// IDA_EA: 0x8325B22
// IDA_NAME: _ZN8WongWork9CCeraShop12_processItemEP5CUser10Inven_ItemiRilbbRSt6vectorIS3_SaIS3_EERNS0_21stCeraShopItemParam_tE
// DEMANGLED: WongWork::CCeraShop::_processItem(CUser *,Inven_Item,int,int &,long,bool,bool,std::vector<Inven_Item,std::allocator<Inven_Item>> &,WongWork::CCeraShop::stCeraShopItemParam_t &)
unsigned int WongWork::CCeraShop::_processItem(
    CUser* user,
    Inven_Item item,
    int special_item_id,
    int& processing_result,
    long current_seconds,
    bool is_vending_machine,
    bool save_inventory,
    std::vector<Inven_Item>& output_items,
    stCeraShopItemParam_t& item_param) {
    if (ProcessSpecialItem(user,
                           item.item_id_,
                           special_item_id,
                           processing_result,
                           current_seconds,
                           is_vending_machine,
                           output_items,
                           item_param,
                           0,
                           0,
                           false,
                           item.add_info_) == 1) {
        return 0;
    }

    if (AvatarCoin::BuyProcess(user, item) != 0) {
        return 0;
    }

    const int slot = user->getCurCharacInvenW()->insertItemIntoInventory(
        item, static_cast<eItemAddReason>(15), true, false);
    if (slot < 0) {
        const char* const mail_text =
            g_scriptStringManager_.findString(4, "game_server_msg_104", 0);
        WongWork::CMailBoxHelper::ReqDBSendNewMailCashShop(
            user,
            item,
            0u,
            static_cast<unsigned int>(user->getCurCharacNo()),
            true,
            mail_text,
            static_cast<int>(std::strlen(mail_text)));
        return 0;
    }

    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(
                                 item.item_category_ == 6 ? 7 : 0),
                             slot);
    if (save_inventory) {
        user->SaveInventory();
    }
    return static_cast<unsigned int>(slot);
}

// IDA_EA: 0x8325D66
// IDA_NAME: _ZN8WongWork9CCeraShop16_processCreatureEP5CUser10Inven_ItemRiS4_bii
// DEMANGLED: WongWork::CCeraShop::_processCreature(CUser *,Inven_Item,int &,int &,bool,int,int)
int WongWork::CCeraShop::_processCreature(CUser* user,
                                          Inven_Item item,
                                          int& item_id,
                                          int& error,
                                          bool,
                                          int usable_period,
                                          int expiration_date) {
    int inserted_slot = -1;
    bool valid = true;

    user_creature::CCreatureMgr* const creature_manager =
        user->getCurCharacInvenW()->GetCreatureMgrW();
    if (!creature_manager->CheckItem(user, &item, valid)) {
        error = 1;
    }
    if (!valid) {
        return inserted_slot;
    }

    if (item.item_category_ == 5 &&
        !user_creature::CCreatureItemConverter::SetEggItem(&item)) {
        LogManager::logFormat(
            1,
            "CeraShop.cpp",
            "int WongWork::CCeraShop::_processCreature(CUser*, Inven_Item, int&, int&, bool, int, int)",
            5919,
            "History ERROR, CCreatureItemConverter::SetEggItem");
        error = 1;
    }

    inserted_slot = user->getCurCharacInvenW()->insertItemIntoInventory(
        item, static_cast<eItemAddReason>(25), true, false);
    if (inserted_slot < 0) {
        const char* const mail_text =
            g_scriptStringManager_.findString(4, "game_server_msg_104", 0);
        if (item.item_category_ == 5) {
            WongWork::CMailBoxHelper::ReqDBSendNewCreatureMail(
                user,
                user->getCurCharacNo(),
                item,
                mail_text,
                static_cast<int>(std::strlen(mail_text)));
        } else {
            WongWork::CMailBoxHelper::ReqDBSendNewMailCashShop(
                user,
                item,
                0u,
                static_cast<unsigned int>(user->getCurCharacNo()),
                true,
                mail_text,
                static_cast<int>(std::strlen(mail_text)));
        }
        item_id = item.item_id_;
        return 0;
    }

    user->getCurCharacInvenW()->GetCreatureMgrW()->InsertCreatureItem(
        &item, inserted_slot, 25, usable_period, expiration_date);
    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(7),
                             inserted_slot);
    user->SaveInventory();
    item_id = user->getCurCharacInvenR()
                  ->GetInvenSlot(3, inserted_slot)
                  .item_id_;
    return inserted_slot;
}

// IDA_EA: 0x83267A2
// IDA_NAME: _ZN8WongWork9CCeraShop22_processAdditionalItemEP5CUsermthhRSt6vectorI10Inven_ItemSaIS4_EERNS0_21stCeraShopItemParam_tEh
// DEMANGLED: WongWork::CCeraShop::_processAdditionalItem(CUser *,unsigned long,unsigned short,unsigned char,unsigned char,std::vector<Inven_Item,std::allocator<Inven_Item>> &,WongWork::CCeraShop::stCeraShopItemParam_t &,unsigned char)
void WongWork::CCeraShop::_processAdditionalItem(
    CUser* user,
    unsigned long item_id,
    unsigned short item_count,
    unsigned char avatar_option_0e,
    unsigned char avatar_option_0d,
    std::vector<Inven_Item>& output_items,
    stCeraShopItemParam_t& item_param,
    unsigned char equipment_upgrade) {
    if (!user->getCurCharacR()) {
        return;
    }

    if (item_id == 1) {
        _processCoin(user, item_count, false);
        Inven_Item coin;
        coin.item_id_ = 1;
        coin.add_info_ = item_count;
        output_items.push_back(coin);
        return;
    }

    CItem* const item = G_CDataManager()->find_item(static_cast<int>(item_id));
    if (!item) {
        return;
    }

    const int item_type = item->vtable_->get_item_type(item);
    if (item->is_stackable() != 0 && (item_type == 16 || item_type == 34)) {
        Inven_Item stackable_item;
        stackable_item.item_id_ = static_cast<int>(item_id);
        stackable_item.add_info_ = item_count;
        item->vtable_->make_item(item, stackable_item);

        const int slot = user->getCurCharacInvenW()->insertItemIntoInventory(
            stackable_item, static_cast<eItemAddReason>(15), true, false);
        if (slot < 0) {
            WongWork::CMailBoxHelper::ReqDBSendNewMailCashShop(
                user,
                stackable_item,
                0u,
                static_cast<unsigned int>(user->getCurCharacNo()),
                true,
                0,
                0);
        } else {
            user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                                     static_cast<ENUM_ITEMSPACE>(0),
                                     slot);
        }
        output_items.push_back(stackable_item);
        return;
    }

    const int grant_count = item->is_stackable() != 0 ? 1 : item_count;
    char ipg_no[24] = {};
    if (item->vtable_->is_avatar_item(item)) {
        WongWork::CGenUniqueNo::genIPGNo(
            static_cast<WongWork::CGenUniqueNo::ENUM_IPGNO_TYPE>(3),
            static_cast<unsigned int>(user->getCurCharacNo()),
            ipg_no);
    }

    Inven_Item generated_item;
    generated_item.item_id_ = static_cast<int>(item_id);
    generated_item.add_info_ = item_count;
    item->vtable_->make_item(item, generated_item);
    if (equipment_upgrade != 0) {
        generated_item.SetUpgrade(equipment_upgrade);
    }
    if (generated_item.item_id_ == 29147 || generated_item.item_id_ == 29392 ||
        generated_item.item_id_ == 29741 || generated_item.item_id_ == 30038 ||
        generated_item.item_id_ == 30305) {
        generated_item.SetUpgrade(12);
    }
    autoUnsealItem(item, generated_item);

    int processing_result = 0;
    for (int index = 0; index < grant_count; ++index) {
        if (item->vtable_->is_avatar_item(item)) {
            const int avatar_period =
                static_cast<CEquipItem*>(item)->getAvatarPeriod(avatar_option_0d);
            const int slot = user->getCurCharacInvenW()->AddAvatarItem(
                static_cast<int>(item_id),
                avatar_period,
                false,
                static_cast<char>(avatar_option_0e),
                static_cast<char>(avatar_option_0d),
                ipg_no,
                static_cast<eAvatarItemAddReason>(0),
                false,
                0);
            if (slot < 0) {
                int expire_time = 0;
                if (avatar_period != 0 && avatar_period != 365) {
                    expire_time = avatar_period * 86400 +
                                  GlobalData::s_systemTime_.getCurSec();
                }
                const char* const mail_text = g_scriptStringManager_.findString(
                    4, "game_server_msg_104", 0);
                WongWork::CMailBoxHelper::ReqDBSendNewAvatarMail(
                    user,
                    user->getCurCharacNo(),
                    static_cast<int>(item_id),
                    expire_time,
                    static_cast<char>(avatar_option_0e),
                    static_cast<char>(avatar_option_0d),
                    ipg_no,
                    mail_text,
                    static_cast<int>(std::strlen(mail_text)));
            } else {
                user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                                         static_cast<ENUM_ITEMSPACE>(1),
                                         slot);
            }
            user->SendAradEventItemLog(item_id, 1);
        } else if (item->vtable_->is_creature_item(item)) {
            int creature_item_id = 0;
            _processCreature(user,
                             generated_item,
                             creature_item_id,
                             processing_result,
                             false,
                             item->getUsablePeriod(),
                             item->getExpirationDate());
        } else {
            _processItem(user,
                         generated_item,
                         0,
                         processing_result,
                         GlobalData::s_systemTime_.getCurSec(),
                         true,
                         false,
                         output_items,
                         item_param);

            if (generated_item.item_id_ == 193 &&
                G_CDataManager()->find_item(generated_item.item_id_)) {
                CItem* const bonus_item = G_CDataManager()->find_item(16);
                Inven_Item bonus;
                bonus_item->vtable_->make_item(bonus_item, bonus);
                bonus.item_id_ = 16;
                bonus.add_info_ = 1;
                int bonus_processing_result = 0;
                std::vector<Inven_Item> ignored_output;
                stCeraShopItemParam_t ignored_param;
                _processItem(user,
                             bonus,
                             50002,
                             bonus_processing_result,
                             GlobalData::s_systemTime_.getCurSec(),
                             false,
                             true,
                             ignored_output,
                             ignored_param);
            }
        }

        if (item->is_stackable() == 0 || _isPackageItem(item) != 1) {
            output_items.push_back(generated_item);
        }
    }
}

// IDA_EA: 0x8327F10
// IDA_NAME: _ZN8WongWork9CCeraShop14autoUnsealItemEPK5CItemR10Inven_Item
// DEMANGLED: WongWork::CCeraShop::autoUnsealItem(CItem const*,Inven_Item &)
void WongWork::CCeraShop::autoUnsealItem(const CItem* item,
                                         Inven_Item& inventory_item) {
    if (!item || item->IsRandomOption() != 1) {
        return;
    }

    const int original_item_index = getOriginalItem();
    if (!original_item_index) {
        return;
    }

    CItem* const original_item = G_CDataManager()->find_item(original_item_index);
    if (!original_item || original_item->IsRandomOptionUnseal() != 1) {
        return;
    }

    if (GlobalData::g_randomOptionHandle->give_option(
            static_cast<unsigned long>(inventory_item.item_id_),
            static_cast<ENUM_RARITY>(item->get_rarity()),
            item->getUsableLevel(),
            item->getItemGroupName(),
            item->GetRandomOptionGrade(),
            &inventory_item.chronicle_random_option_.random_option_) != 1) {
        cMyTrace trace(
            "void WongWork::CCeraShop::autoUnsealItem(const CItem*, Inven_Item&)",
            0x2131,
            0);
        trace("autoUnsealItem : %d", inventory_item.item_id_);
    }
}

// IDA_EA: 0x8326739
// IDA_NAME: _Z14_isPackageItemPK5CItem
// DEMANGLED: _isPackageItem(CItem const*)
int _isPackageItem(const CItem* item) {
    if (item->vtable_->get_item_type(item) == 15) {
        return 1;
    }
    if (item->vtable_->get_item_type(item) == 14) {
        return 1;
    }
    return item->vtable_->get_item_type(item) == 16;
}

// IDA_EA: 0x83237F6
// IDA_NAME: _ZN8WongWork9CCeraShop16_ProcessIPGErrorEP5CUserPNS_3IPG8SIPGDataES5_hijiic
// DEMANGLED: WongWork::CCeraShop::_ProcessIPGError(CUser *,WongWork::IPG::SIPGData *,WongWork::IPG::SIPGData *,unsigned char,int,unsigned int,int,int,char)
void WongWork::CCeraShop::_ProcessIPGError(
    CUser* user,
    IPG::SIPGData* request,
    IPG::SIPGData* result,
    unsigned char error,
    int group_index,
    unsigned int goods_no,
    int item_order,
    int item_count,
    char gift) {
    if (result->data_type == IPG::IPG_OUTPUT) {
        if (std::atoi(result->wire.output.result_code_decimal) == 1) {
            if (!user) {
                GlobalData::s_pIPGHelper->IPGOutputCancel(request, result);
                return;
            }
            // 0x83238BC uses the output response-reserved field (absolute
            // result +0x159), followed by the five request output fields.
            // This is distinct from the null-user SIPGData overload, whose
            // own provider copies the legacy +0x16e cancel-code span.
            GlobalData::s_pIPGHelper->IPGOutputCancel(
                user,
                result->wire.output.response_reserved,
                request->wire.output.product_id_decimal,
                request->wire.output.price_decimal,
                request->wire.output.order_id,
                request->wire.output.product_name,
                request->wire.output.product_description,
                0,
                0);
        } else {
            LogManager::logFormat(
                1,
                "CeraShop.cpp",
                "void WongWork::CCeraShop::_ProcessIPGError(CUser*, WongWork::IPG::SIPGData*, WongWork::IPG::SIPGData*, unsigned char, int, unsigned int, int, int, char)",
                0xB47,
                "IPG ERROR (Goods No: %d)(%s)",
                goods_no,
                result->wire.output.result_message);
        }
    }

    if (user) {
        ProcessError(user,
                     static_cast<unsigned short>(error),
                     group_index,
                     goods_no,
                     item_order,
                     item_count,
                     gift);
    }
}

// IDA_EA: 0x8326036
// IDA_NAME: _ZN8WongWork9CCeraShop18_processAvatarGiftEP5CUseriiccPKcji
// DEMANGLED: WongWork::CCeraShop::_processAvatarGift(CUser *,int,int,char,char,char const*,unsigned int,int)
void WongWork::CCeraShop::_processAvatarGift(CUser* user,
                                             int item_id,
                                             int usable_period,
                                             char ability_no,
                                             char avatar_select,
                                             const char* order_id,
                                             unsigned int receiver_member_id,
                                             int receiver_charac_no) {
    int expire_time = 0;
    if (usable_period != 0 && usable_period != 365) {
        expire_time = 86400 * usable_period +
                      GlobalData::s_systemTime_.getCurSec();
    }

    char mail_text[256];
    std::memset(mail_text, 0, sizeof(mail_text));
    const char* const format = g_scriptStringManager_.findString(
        4, "game_server_msg_02", 0);
    std::sprintf(mail_text, format, user->getCurCharacName());

    user->history_log().SendMailForCeraShopGift(
        static_cast<unsigned int>(receiver_charac_no),
        2,
        static_cast<unsigned int>(item_id),
        1u);
    user->SendCeraShopGiftStatisticLog(receiver_member_id,
                                       static_cast<unsigned int>(receiver_charac_no),
                                       static_cast<unsigned long>(item_id),
                                       1u);
    WongWork::CMailBoxHelper::ReqDBSendNewAvatarMailCashShop(
        user,
        receiver_charac_no,
        item_id,
        expire_time,
        ability_no,
        avatar_select,
        order_id,
        mail_text,
        static_cast<int>(std::strlen(mail_text)));
}

// IDA_EA: 0x832619C
// IDA_NAME: _ZN8WongWork9CCeraShop16_processCoinGiftEP5CUsertji
// DEMANGLED: WongWork::CCeraShop::_processCoinGift(CUser *,unsigned short,unsigned int,int)
int WongWork::CCeraShop::_processCoinGift(CUser* user,
                                           unsigned short coin_count,
                                           unsigned int receiver_member_id,
                                           int receiver_charac_no) {
    Inven_Item coin;
    coin.item_category_ = 2;
    coin.item_id_ = 1;
    coin.add_info_ = static_cast<int32_t>(coin_count);
    coin.avatar_select_ = 2;
    coin.ResetItemAttr();

    char mail_text[256];
    std::memset(mail_text, 0, sizeof(mail_text));
    const char* const format = g_scriptStringManager_.findString(
        4, "game_server_msg_02", 0);
    std::sprintf(mail_text, format, user->getCurCharacName());

    user->history_log().SendMailForCeraShopGift(
        static_cast<unsigned int>(receiver_charac_no), 1, 1u, coin_count);
    user->SendCeraShopGiftStatisticLog(receiver_member_id,
                                       static_cast<unsigned int>(receiver_charac_no),
                                       1ul,
                                       coin_count);
    return WongWork::CMailBoxHelper::ReqDBSendNewMailCashShop(
        user,
        coin,
        0u,
        static_cast<unsigned int>(receiver_charac_no),
        true,
        mail_text,
        static_cast<int>(std::strlen(mail_text)));
}

// IDA_EA: 0x8326332
// IDA_NAME: _ZN8WongWork9CCeraShop16_processCoinGiftEP5CUsertjii
// DEMANGLED: WongWork::CCeraShop::_processCoinGift(CUser *,unsigned short,unsigned int,int,int)
int WongWork::CCeraShop::_processCoinGift(CUser* user,
                                           unsigned short coin_count,
                                           unsigned int receiver_member_id,
                                           int receiver_charac_no,
                                           int coin_item_id) {
    Inven_Item coin;
    coin.item_category_ = 2;
    coin.item_id_ = coin_item_id;
    coin.add_info_ = static_cast<int32_t>(coin_count);
    coin.avatar_select_ = 2;
    coin.ResetItemAttr();

    char mail_text[256];
    std::memset(mail_text, 0, sizeof(mail_text));
    const char* const format = g_scriptStringManager_.findString(
        4, "game_server_msg_02", 0);
    std::sprintf(mail_text, format, user->getCurCharacName());

    char gift_kind = 0;
    if (coin_item_id == 236) {
        gift_kind = 4;
    } else if (coin_item_id == 202) {
        gift_kind = 5;
    }
    user->history_log().SendMailForCeraShopGift(
        static_cast<unsigned int>(receiver_charac_no),
        gift_kind,
        static_cast<unsigned int>(coin_item_id),
        coin_count);
    user->SendCeraShopGiftStatisticLog(
        receiver_member_id,
        static_cast<unsigned int>(receiver_charac_no),
        static_cast<unsigned long>(coin_item_id),
        coin_count);
    return WongWork::CMailBoxHelper::ReqDBSendNewMailCashShop(
        user,
        coin,
        0u,
        static_cast<unsigned int>(receiver_charac_no),
        true,
        mail_text,
        static_cast<int>(std::strlen(mail_text)));
}

// IDA_EA: 0x83264D6
// IDA_NAME: _ZN8WongWork9CCeraShop16_processItemGiftEP5CUser10Inven_Itemji
// DEMANGLED: WongWork::CCeraShop::_processItemGift(CUser *,Inven_Item,unsigned int,int)
int WongWork::CCeraShop::_processItemGift(CUser* user,
                                           Inven_Item item,
                                           unsigned int receiver_member_id,
                                           int receiver_charac_no) {
    char mail_text[256];
    std::memset(mail_text, 0, sizeof(mail_text));
    const char* const format = g_scriptStringManager_.findString(
        4, "game_server_msg_02", 0);
    std::sprintf(mail_text, format, user->getCurCharacName());

    user->history_log().SendMailForCeraShopGift(
        static_cast<unsigned int>(receiver_charac_no),
        0,
        static_cast<unsigned int>(item.item_id_),
        static_cast<unsigned int>(item.add_info_));
    user->SendCeraShopGiftStatisticLog(
        receiver_member_id,
        static_cast<unsigned int>(receiver_charac_no),
        static_cast<unsigned long>(item.item_id_),
        static_cast<unsigned int>(item.add_info_));
    return WongWork::CMailBoxHelper::ReqDBSendNewMailCashShop(
        user,
        item,
        0u,
        static_cast<unsigned int>(receiver_charac_no),
        true,
        mail_text,
        static_cast<int>(std::strlen(mail_text)));
}

// IDA_EA: 0x83265E8
// IDA_NAME: _ZN8WongWork9CCeraShop20_processCreatureGiftEP5CUser10Inven_Itemjiii
// DEMANGLED: WongWork::CCeraShop::_processCreatureGift(CUser *,Inven_Item,unsigned int,int,int,int)
void WongWork::CCeraShop::_processCreatureGift(CUser* user,
                                                Inven_Item item,
                                                unsigned int receiver_member_id,
                                                int receiver_charac_no,
                                                int,
                                                int) {
    char mail_text[256];
    std::memset(mail_text, 0, sizeof(mail_text));
    const char* const format = g_scriptStringManager_.findString(
        4, "game_server_msg_02", 0);
    std::sprintf(mail_text, format, user->getCurCharacName());

    user->history_log().SendMailForCeraShopGift(
        static_cast<unsigned int>(receiver_charac_no),
        3,
        static_cast<unsigned int>(item.item_id_),
        static_cast<unsigned int>(item.add_info_));
    user->SendCeraShopGiftStatisticLog(
        receiver_member_id,
        static_cast<unsigned int>(receiver_charac_no),
        static_cast<unsigned long>(item.item_id_),
        static_cast<unsigned int>(item.add_info_));
    if (item.item_category_ == 5) {
        WongWork::CMailBoxHelper::ReqDBSendNewCreatureMail(
            user,
            receiver_charac_no,
            item,
            mail_text,
            static_cast<int>(std::strlen(mail_text)));
        return;
    }
    WongWork::CMailBoxHelper::ReqDBSendNewMailCashShop(
        user,
        item,
        0u,
        static_cast<unsigned int>(receiver_charac_no),
        true,
        mail_text,
        static_cast<int>(std::strlen(mail_text)));
}

// IDA_EA: 0x817A2BA
// IDA_NAME: _ZN15Arad_IPGProcessC2EP18ItemVendingMachine
// DEMANGLED: Arad_IPGProcess::Arad_IPGProcess(ItemVendingMachine *)
Arad_IPGProcess::Arad_IPGProcess(ItemVendingMachine* vending_machine)
    : vending_machine_(vending_machine) {}

// IDA_EA: 0x817A2C8
// IDA_NAME: _ZN15Arad_IPGProcessD2Ev
// DEMANGLED: Arad_IPGProcess::~Arad_IPGProcess()
Arad_IPGProcess::~Arad_IPGProcess() {}

// IDA_EA: 0x819C2B4
// IDA_NAME: _ZN15Arad_IPGProcess18CashShopModeAvatarEP5CUsermiiccPKcRijic
// DEMANGLED: Arad_IPGProcess::CashShopModeAvatar(CUser *,unsigned long,int,int,char,char,char const*,int &,unsigned int,int,char)
void Arad_IPGProcess::CashShopModeAvatar(CUser* user,
                                          unsigned long item_id,
                                          int goods_no,
                                          int usable_period,
                                          char ability_no,
                                          char avatar_select,
                                          const char* order_id,
                                          int& processing_result,
                                          unsigned int receiver_member_id,
                                          int receiver_charac_no,
                                          char gift) {
    // 0x819C2B4 never reads its +0x7c goods number nor its +0x90 result
    // reference.  AddAvatarItem failure is deliberately converted to the
    // CashShop avatar-mail path at 0x819C3AA..0x819C403, rather than a local
    // ProcessIPG error; the caller subsequently keeps processing_result == 0.
    (void)goods_no;
    (void)processing_result;
    if (gift == 1) {
        vending_machine_->cera_shop_._processAvatarGift(
            user,
            static_cast<int>(item_id),
            usable_period,
            ability_no,
            avatar_select,
            order_id,
            receiver_member_id,
            receiver_charac_no);
        return;
    }

    const int slot = user->getCurCharacInvenW()->AddAvatarItem(
        static_cast<int>(item_id),
        usable_period,
        false,
        ability_no,
        avatar_select,
        order_id,
        static_cast<eAvatarItemAddReason>(0),
        false,
        0);
    if (slot >= 0) {
        user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                                 static_cast<ENUM_ITEMSPACE>(1),
                                 slot);
        return;
    }

    const char* const mail_text = g_scriptStringManager_.findString(
        4, "game_server_msg_104", 0);
    WongWork::CMailBoxHelper::ReqDBSendNewAvatarMailCashShop(
        user,
        user->getCurCharacNo(),
        static_cast<int>(item_id),
        usable_period,
        ability_no,
        avatar_select,
        order_id,
        mail_text,
        static_cast<int>(std::strlen(mail_text)));
}

// IDA_EA: 0x819C434
// IDA_NAME: _ZN15Arad_IPGProcess16CashShopModeCoinEP5CUsertjic
// DEMANGLED: Arad_IPGProcess::CashShopModeCoin(CUser *,unsigned short,unsigned int,int,char)
int Arad_IPGProcess::CashShopModeCoin(CUser* user,
                                      unsigned short coin_count,
                                      unsigned int receiver_member_id,
                                      int receiver_charac_no,
                                      char gift) {
    if (gift == 1) {
        return vending_machine_->cera_shop_._processCoinGift(
            user,
            coin_count,
            receiver_member_id,
            receiver_charac_no);
    }
    vending_machine_->cera_shop_._processCoin(user, coin_count, false);
    return 0;
}

// IDA_EA: 0x819C4A0
// IDA_NAME: _ZN15Arad_IPGProcess16CashShopModeItemEP5CUsermtilRSt6vectorI10Inven_ItemSaIS3_EERN8WongWork9CCeraShop21stCeraShopItemParam_tERijic
// DEMANGLED: Arad_IPGProcess::CashShopModeItem(CUser *,unsigned long,unsigned short,int,long,std::vector<Inven_Item,std::allocator<Inven_Item>> &,WongWork::CCeraShop::stCeraShopItemParam_t &,int &,unsigned int,int,char)
unsigned int Arad_IPGProcess::CashShopModeItem(
    CUser* user,
    unsigned long item_id,
    unsigned short item_count,
    int special_item_id,
    long current_seconds,
    std::vector<Inven_Item>& output_items,
    WongWork::CCeraShop::stCeraShopItemParam_t& item_param,
    int& processing_result,
    unsigned int receiver_member_id,
    int receiver_charac_no,
    char gift) {
    CItem* const prototype = G_CDataManager()->find_item(
        static_cast<int>(item_id));
    if (!prototype) {
        return 0;
    }

    Inven_Item item;
    prototype->vtable_->make_item(prototype, item);
    item.item_id_ = static_cast<int32_t>(item_id);
    item.add_info_ = static_cast<int32_t>(item_count);
    if (gift == 1) {
        return static_cast<unsigned int>(
            vending_machine_->cera_shop_._processItemGift(
                user, item, receiver_member_id, receiver_charac_no));
    }
    return vending_machine_->cera_shop_._processItem(user,
                                                       item,
                                                       special_item_id,
                                                       processing_result,
                                                       current_seconds,
                                                       false,
                                                       true,
                                                       output_items,
                                                       item_param);
}

// IDA_EA: 0x819C670
// IDA_NAME: _ZN15Arad_IPGProcess20CashShopModeCreatureEP5CUsermtRiS2_jic
// DEMANGLED: Arad_IPGProcess::CashShopModeCreature(CUser *,unsigned long,unsigned short,int &,int &,unsigned int,int,char)
void Arad_IPGProcess::CashShopModeCreature(CUser* user,
                                           unsigned long item_id,
                                           unsigned short item_count,
                                           int& output_item_id,
                                           int& processing_result,
                                           unsigned int receiver_member_id,
                                           int receiver_charac_no,
                                           char gift) {
    CItem* const prototype = G_CDataManager()->find_item(
        static_cast<int>(item_id));
    if (!prototype) {
        return;
    }

    Inven_Item item;
    prototype->vtable_->make_item(prototype, item);
    item.item_id_ = static_cast<int32_t>(item_id);
    item.add_info_ = static_cast<int32_t>(item_count);
    if (gift == 1) {
        vending_machine_->cera_shop_._processCreatureGift(
            user,
            item,
            receiver_member_id,
            receiver_charac_no,
            prototype->getUsablePeriod(),
            prototype->getExpirationDate());
        return;
    }

    vending_machine_->cera_shop_._processCreature(
        user,
        item,
        output_item_id,
        processing_result,
        true,
        prototype->getUsablePeriod(),
        prototype->getExpirationDate());
    // 0x819C851 leaves the child call's EAX untouched, but 0x8178BE5 (the
    // sole xref) jumps directly to its result handling and never consumes it.
    // The meaningful output is the two references above; preserving a
    // pointer-shaped EAX value would invent an ABI that the old program had
    // no semantic caller for.
}
