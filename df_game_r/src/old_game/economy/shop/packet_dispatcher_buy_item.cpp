#include "old_game/economy/shop/packet_dispatcher_buy_item.h"

#include <algorithm>
#include <stdint.h>

#include "common/config.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "common/manager/history_log_abi.h"
#include "old_game/economy/inventory/item.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "common/manager/statistics_runtime.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/economy/shop/npc_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const ENUM_CMDPACKET kCmdBuyItem = ENUM_CMDPACKET_BUY_ITEM;

enum {
    kInventoryType = 1,
    kAuthMobileRestrictId = 3,
    kShopSecurityProtection = 34,
    kOneADayEventId = 109,
    kInvalidShopHackType = 0x3c9,
    kBossTowerSpecialMaterialId = 2749095,
};

const char kReadSignature[] =
    "virtual int DisPatcher_BuyItem::read(PacketBuf&, MSG_BASE&)";
const char kProcessSignature[] =
    "virtual int DisPatcher_BuyItem::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// IDA_EA: 0x85950A8
// IDA_NAME: _ZN18DisPatcher_BuyItemC2Ev
// DEMANGLED: DisPatcher_BuyItem::DisPatcher_BuyItem(void)
DisPatcher_BuyItem::DisPatcher_BuyItem()
    : IPacketDispatcher<MSG_BUY_ITEM,
                        BuyItemParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x8231F48
// IDA_NAME: _ZN18DisPatcher_BuyItemD2Ev
// DEMANGLED: DisPatcher_BuyItem::~DisPatcher_BuyItem()
// IDA_EA: 0x8231F78
// IDA_NAME: _ZN18DisPatcher_BuyItemD0Ev
// DEMANGLED: DisPatcher_BuyItem::~DisPatcher_BuyItem()
DisPatcher_BuyItem::~DisPatcher_BuyItem() {}

// IDA_EA: 0x81BDBA8
// IDA_NAME: _ZN18DisPatcher_BuyItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_BuyItem::read(PacketBuf &,MSG_BASE &)
int DisPatcher_BuyItem::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_BUY_ITEM& input = static_cast<MSG_BUY_ITEM&>(message);
    uint32_t value = 0;
    if (packet.get_int(value) != 1) {
        return LineFunc(392, kReadSignature, 0, 0);
    }
    input.item_id = value;
    if (packet.get_int(value) != 1) {
        return LineFunc(395, kReadSignature, 0, 0);
    }
    input.count_or_seed = value;
    if (packet.get_int(value) == 1) {
        input.npc_id = value;
        return 0;
    }
    return LineFunc(398, kReadSignature, 0, 0);
}

// IDA_EA: 0x81BE46A
// IDA_NAME: _ZN18DisPatcher_BuyItem11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_BuyItem::check_error(CUser *,MSG_BASE &)
int DisPatcher_BuyItem::check_error(CUser* user, MSG_BASE& message) {
    MSG_BUY_ITEM& input = static_cast<MSG_BUY_ITEM&>(message);
    CDataManager* const data_manager = G_CDataManager();
    if (data_manager->checkValidShopItem(input.npc_id, input.item_id) != 1) {
        if (data_manager->is_one_a_day_shop(static_cast<int>(input.npc_id))) {
            return 17;
        }
        user->getHackAnalyzer()->addServerHackCnt(
            user,
            static_cast<WongWork::ENUM_HACKTYPE>(kInvalidShopHackType),
            1u,
            0u,
            0u);
        return -1;
    }
    if (user->CheckInTrade() || !user->getCurCharacR() || user->isHackUser()) {
        return 19;
    }

    ServiceRestrictManager* const restrict_manager =
        ARAD::Singleton<ServiceRestrictManager>::Get();
    if (restrict_manager->isRestricted(
            user, RestrictCategory::AuthMobile, kAuthMobileRestrictId)) {
        return 209;
    }
    if (user->get_state() != 8) {
        const int security_error = GlobalData::s_pSecuProtectionField->Check(
            user, static_cast<SECURITY_PROTCTION>(kShopSecurityProtection));
        if (security_error != 0) {
            return security_error;
        }
    }
    if (private_store::GetInstancePrivateStoreMgr()->IsBusyPrivateStore(user)) {
        return 60;
    }
    if (!data_manager->is_one_a_day_shop(static_cast<int>(input.npc_id))) {
        return 0;
    }

    CEventBase* const event = GlobalData::s_event_manager->GetRepeatEvent(
        kOneADayEventId);
    return event->IsEventing(0) == 1 ? 0 : 17;
}

// IDA_EA: 0x81BEAB0
// IDA_NAME: _ZN18DisPatcher_BuyItem20get_nMaxBuyableCountEP5CUserR12BuyItemParam
// DEMANGLED: DisPatcher_BuyItem::get_nMaxBuyableCount(CUser *,BuyItemParam &)
int DisPatcher_BuyItem::get_nMaxBuyableCount(CUser* user,
                                              BuyItemParam& param) {
    int maximum = 0x7fffffff;
    if (param.price_ != 0) {
        maximum = user->getCurCharacMoney() / param.price_;
    }
    if (param.cera_price_ != 0) {
        maximum = std::min(maximum, user->GetCera() / param.cera_price_);
    }
    if (param.win_point_price_ != 0) {
        maximum = std::min(maximum,
                           user->GetWinPoint() / param.win_point_price_);
    }
    if (param.finish_point_price_ != 0) {
        maximum = std::min(maximum,
                           user->GetFinishPoint() / param.finish_point_price_);
    }
    if (param.guild_power_cost_ != 0) {
        maximum = std::min(maximum,
                           user->GetGuildPowerWarPoint() /
                               param.guild_power_cost_);
    }
    if (param.user_power_cost_ != 0) {
        const int user_power = static_cast<int>(
            static_cast<int16_t>(user->GetUserPowerWarPoint()));
        maximum = std::min(maximum, user_power / param.user_power_cost_);
    }
    return maximum;
}

// IDA_EA: 0x81BE658
// IDA_NAME: _ZN18DisPatcher_BuyItem8get_dataEP5CUserR12MSG_BUY_ITEMR12BuyItemParam
// DEMANGLED: DisPatcher_BuyItem::get_data(CUser *,MSG_BUY_ITEM &,BuyItemParam &)
int DisPatcher_BuyItem::get_data(CUser* user,
                                  MSG_BUY_ITEM& message,
                                  BuyItemParam& param) {
    param.prototype_ = G_CDataManager()->find_item(message.item_id);
    if (!param.prototype_) {
        return 17;
    }

    // OFFICIAL_IDA_0828: get_data @0x81BE658 calls
    // CItem::GetNeedMaterial @0x850D6F4.  That accessor is essential: the
    // manager normally returns (-1,0), then the prototype's PVF-owned +0x24
    // pair supplies [need material].  Reading the manager directly silently
    // converted every ordinary material-priced shop item into gold-only.
    const std::pair<int, int>* const need_material =
        param.prototype_->GetNeedMaterial();
    param.material_requirement_ = *need_material;
    if (param.material_requirement_.first != -1) {
        CInventory* const inventory = user->getCurCharacInvenR();
        param.material_slot_ = inventory->GetInvenData(
            param.material_requirement_.first, param.material_);
        param.material_prototype_ = G_CDataManager()->find_item(
            param.material_requirement_.first);
        if (!param.material_prototype_ ||
            param.material_prototype_->is_stackable() != 1) {
            return 1;
        }
    }

    param.price_ = param.prototype_->get_price();
    param.cera_price_ = param.prototype_->GetCashPrice();
    param.win_point_price_ = param.prototype_->GetMedalPrice();
    param.finish_point_price_ = param.prototype_->GetFinishPointPrice();
    param.guild_power_cost_ =
        param.prototype_->vtable_->get_guild_power_war_point(param.prototype_);
    param.user_power_cost_ =
        param.prototype_->vtable_->get_user_power_war_point(param.prototype_);

    if (param.prototype_->is_stackable() == 1) {
        if (message.count_or_seed == 0) {
            return 1;
        }
        if (message.count_or_seed > 1) {
            if (get_nMaxBuyableCount(user, param) <
                static_cast<int>(message.count_or_seed)) {
                return 10;
            }
            param.price_ *= static_cast<int>(message.count_or_seed);
            param.cera_price_ *= static_cast<int>(message.count_or_seed);
            param.win_point_price_ *= static_cast<int>(message.count_or_seed);
            param.finish_point_price_ *= static_cast<int>(message.count_or_seed);
            param.guild_power_cost_ *= static_cast<int>(message.count_or_seed);
            param.user_power_cost_ *= static_cast<int>(message.count_or_seed);
            param.material_requirement_.second *=
                static_cast<int>(message.count_or_seed);
        }
    } else if (static_cast<int>(message.count_or_seed) !=
               user->GetSeedFromDate()) {
        return 1;
    }

    if (user->getCurCharacMoney() < param.price_ ||
        static_cast<uint32_t>(user->GetCera()) <
            static_cast<uint32_t>(param.cera_price_) ||
        static_cast<uint32_t>(user->GetWinPoint()) <
            static_cast<uint32_t>(param.win_point_price_) ||
        user->GetFinishPoint() < param.finish_point_price_ ||
        (param.material_requirement_.first != -1 &&
         param.material_.add_info_ < param.material_requirement_.second) ||
        user->GetGuildPowerWarPoint() < param.guild_power_cost_ ||
        static_cast<int16_t>(user->GetUserPowerWarPoint()) <
            static_cast<int16_t>(param.user_power_cost_)) {
        return 10;
    }
    if (param.guild_power_cost_ != 0 && !user->IsGuildMaster()) {
        return 36;
    }
    return 0;
}

// IDA_EA: 0x81BEC5E
// IDA_NAME: _ZN18DisPatcher_BuyItem17statistic_processEP5CUserR12BuyItemParam
// DEMANGLED: DisPatcher_BuyItem::statistic_process(CUser *,BuyItemParam &)
int DisPatcher_BuyItem::statistic_process(CUser* user, BuyItemParam& param) {
    G_Store()->GetSellItemPrice(param.item_,
                                param.prototype_,
                                static_cast<short>(param.item_.add_info_),
                                false,
                                param.sell_price_);
    GetInstanceValueStatistic()->AddValueStatistic(
        static_cast<VALUE_STATISTIC_FIELD>(6), user, param.sell_price_);
    GetInstanceValueStatistic()->AddValueStatistic(
        static_cast<VALUE_STATISTIC_FIELD>(13), user, param.price_);
    return GetInstanceCubeStatistic()->collectCubeStatistics(
        param.material_requirement_.first,
        param.material_requirement_.second,
        user,
        static_cast<CUBE_STATISTIC_FIELD>(106));
}

// IDA_EA: 0x81BED44
// IDA_NAME: _ZN18DisPatcher_BuyItem15_onBuyItemEventERK12BuyItemParamP5CUser
// DEMANGLED: DisPatcher_BuyItem::_onBuyItemEvent(BuyItemParam const&,CUser *)
void DisPatcher_BuyItem::_onBuyItemEvent(const BuyItemParam&, CUser*) {}

// IDA_EA: 0x81BDC7E
// IDA_NAME: _ZN18DisPatcher_BuyItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_BuyItem::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_BuyItem::process(CUser* user,
                                MSG_BASE& message,
                                ParamBase& result) {
    MSG_BUY_ITEM& input = static_cast<MSG_BUY_ITEM&>(message);
    BuyItemParam& param = static_cast<BuyItemParam&>(result);

    int error = check_error(user, message);
    if (error > 0) {
        user->SendCmdErrorPacket(kCmdBuyItem,
                                 static_cast<unsigned char>(error));
        return -1;
    }
    if (error < 0) {
        user->SendCmdErrorPacket(kCmdBuyItem, 21);
        return LineFunc(417, kProcessSignature, 0, user->get_acc_id());
    }

    error = get_data(user, input, param);
    if (error != 0) {
        user->SendCmdErrorPacket(kCmdBuyItem,
                                 static_cast<unsigned char>(error));
        return -1;
    }
    if (param.material_requirement_.first == kBossTowerSpecialMaterialId) {
        if (user->get_state() == 3) {
            return LineFunc(436, kProcessSignature, 0, 0);
        }
        if (!G_CGameManager()->getBossTower(user->getBossTowerIndex())) {
            return LineFunc(440, kProcessSignature, 0, 0);
        }
    }

    param.item_.item_id_ = static_cast<int32_t>(input.item_id);
    param.prototype_->vtable_->make_item(param.prototype_, param.item_);
    if (!param.prototype_->vtable_->is_creature_item(param.prototype_) &&
        !param.prototype_->vtable_->is_avatar_item(param.prototype_)) {
        param.item_.set_add_info(static_cast<int>(input.count_or_seed));
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    param.inserted_slot_ = inventory->insertItemIntoInventory(
        param.item_, static_cast<eItemAddReason>(0), true, false);
    if (param.inserted_slot_ < 0) {
        user->SendCmdErrorPacket(kCmdBuyItem, 4);
        return -1;
    }

    CItem* const purchased_item = G_CDataManager()->find_item(input.item_id);
    if (!purchased_item) {
        user->SendCmdErrorPacket(kCmdBuyItem, 19);
        return -1;
    }
    if (purchased_item->vtable_->is_creature_item(purchased_item) &&
        purchased_item->vtable_->get_item_type(purchased_item) ==
            oldsrv::config::kEquipmentTypeCreature) {
        inventory->GetCreatureMgrW()->InsertCreatureItem(
            &param.item_,
            param.inserted_slot_,
            0,
            purchased_item->getUsablePeriod(),
            purchased_item->getExpirationDate());
    }

    GlobalData::s_guild_proxy_mgr
        ->GetServerProxy(static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))
        ->OnBuyItem(user,
                    param.prototype_,
                    static_cast<unsigned int>(param.guild_power_cost_),
                    static_cast<unsigned int>(param.user_power_cost_));
    inventory->use_money(param.price_, static_cast<eMoneySubReason>(0), true);
    if (param.material_requirement_.first != -1) {
        inventory->delete_item(static_cast<INVEN_TYPE>(kInventoryType),
                               param.material_slot_,
                               param.material_requirement_.second,
                               static_cast<eItemDelReason>(26),
                               true);
    }

    _onBuyItemEvent(param, user);
    statistic_process(user, param);
    HistoryLog::WriteBuyItem(user->history_stream(),
                             param.item_.item_id_,
                             param.item_.add_info_);
    user->SendAradEventItemLog(param.item_.item_id_, param.item_.add_info_);
    user->eventDeliveryBuyItem(static_cast<int>(input.npc_id), param.item_);
    return 0;
}

// IDA_EA: 0x81BE20A
// IDA_NAME: _ZN18DisPatcher_BuyItem4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_BuyItem::send(CUser *,ParamBase &)
void DisPatcher_BuyItem::send(CUser* user, ParamBase& result) {
    BuyItemParam& param = static_cast<BuyItemParam&>(result);
    PacketGuard response;
    response.put_header(1, kCmdBuyItem);
    response.put_byte(1);
    response.put_int(user->getCurCharacInvenR()->get_money());
    response.put_int(user->UseWinPoint(
        param.win_point_price_, static_cast<eWPSubReason>(0)));
    response.put_int(user->UseFinishPoint(param.finish_point_price_));
    response.put_int(user->UseCera(static_cast<unsigned int>(param.cera_price_)));
    response.put_short(static_cast<uint16_t>(param.inserted_slot_));
    response.put_int(param.item_.item_id_);
    response.put_int(param.item_.add_info_);
    response.put_short(param.item_.avatar_select_);
    response.put_byte(param.item_.amplify_option_.getAbilityType());
    response.put_short(param.item_.amplify_option_.getAbilityValue());
    response.put_packet(param.item_);
    response.put_int(param.material_requirement_.first);

    const Inven_Item* const material = user->getCurCharacInvenR()->GetInvenRef(
        static_cast<INVEN_TYPE>(kInventoryType), param.material_slot_);
    response.put_int(material ? material->add_info_ : 0);
    response.finalize(1);
    user->Send(response);
}
