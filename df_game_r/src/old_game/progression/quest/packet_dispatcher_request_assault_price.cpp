#include "old_game/progression/quest/packet_dispatcher_request_assault_price.h"

#include "common/config.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/gm_accounts_abi.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/pvp/battlefield/pvp_assault.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const int kAssaultQueryCommand = 4;
const int kAssaultProtectionOperation = 55;
const int kNoResponseResult = 0x7fffffff;

int ApplyEquipmentUpgradeDiscount(CUser* user, int price) {
    if (price <= 0) return price;

    const Inven_Item head_equipment =
        user->getCurCharacInvenR()->GetInvenSlot(
            0, oldsrv::config::kEquipmentTypeTitle);
    CItem* const item = G_CDataManager()->find_item(head_equipment.item_id_);
    if (!item) return price;

    return static_cast<int>(
        static_cast<CEquipItem*>(item)->getUpgradeCostDiscount() *
            static_cast<float>(price) / -100.0f +
        static_cast<float>(price));
}

}  // namespace

// IDA_EA: 0x8597A7E
// IDA_NAME: _ZN30Dispatcher_RequestAssaultPriceC2Ev
// DEMANGLED: Dispatcher_RequestAssaultPrice::Dispatcher_RequestAssaultPrice(void)
Dispatcher_RequestAssaultPrice::Dispatcher_RequestAssaultPrice()
    : IPacketDispatcher<MsgRequestAssaultPrice, stRequestAssaultPrice,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x82444DC
// IDA_NAME: _ZN30Dispatcher_RequestAssaultPriceD2Ev
// DEMANGLED: Dispatcher_RequestAssaultPrice::~Dispatcher_RequestAssaultPrice()
// IDA_EA: 0x824450C
// IDA_NAME: _ZN30Dispatcher_RequestAssaultPriceD0Ev
// DEMANGLED: Dispatcher_RequestAssaultPrice::~Dispatcher_RequestAssaultPrice()
Dispatcher_RequestAssaultPrice::~Dispatcher_RequestAssaultPrice() {}

// IDA_EA: 0x81DEE2A
// IDA_NAME: _ZN30Dispatcher_RequestAssaultPrice4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_RequestAssaultPrice::read(PacketBuf &,MSG_BASE &)
int Dispatcher_RequestAssaultPrice::read(PacketBuf& packet, MSG_BASE& base) {
    MsgRequestAssaultPrice& message =
        static_cast<MsgRequestAssaultPrice&>(base);
    return packet.get_short(message.item_unique_id) == 1
               ? 0
               : LineFunc(
                     20669,
                     "virtual int Dispatcher_RequestAssaultPrice::read(PacketBuf&, MSG_BASE&)",
                     0, 0);
}

// IDA_EA: 0x81DF084
// IDA_NAME: _ZN30Dispatcher_RequestAssaultPrice11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_RequestAssaultPrice::check_error(CUser *,MSG_BASE &)
int Dispatcher_RequestAssaultPrice::check_error(
    CUser* user, const MsgRequestAssaultPrice& message) {
    if (!user || user->get_state() <= 2) return -1;

    CUser* const target = G_GameWorld()->find_from_world(message.item_unique_id);
    if (private_store::GetInstancePrivateStoreMgr()->IsBusyPrivateStore(user))
        return 60;
    if (user->IsBusyDisjointStore()) return 191;
    if (user->IsCurCharacGhost()) return 19;
    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) return 19;
    if (!target) return 3;
    if (target->IsBusyDisjointStore()) return 192;

    const int party_error = pvp_assault::GetInstanceAssaultMgr()
                                ->check_party_teleport_user(
                                    static_cast<char>(kAssaultQueryCommand),
                                    user, target, false);
    if (party_error > 0) return party_error;

    const int protection_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(kAssaultProtectionOperation));
    if (protection_error) return protection_error;
    if (target->get_state() > 4) return kNoResponseResult;

    if (!GlobalData::s_pGMAccounts_->isGM(user->get_acc_id()) &&
        GlobalData::s_pGMAccounts_->isGM(target->get_acc_id()))
        return kNoResponseResult;

    int preflight_result = 0;
    pvp_assault::GetInstanceAssaultMgr()->OnRequestAssault(
        user, target, &preflight_result);
    return preflight_result;
}

// IDA_EA: 0x81DEE80
// IDA_NAME: _ZN30Dispatcher_RequestAssaultPrice7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RequestAssaultPrice::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RequestAssaultPrice::process(CUser* user,
                                            MSG_BASE& base,
                                            ParamBase& raw) {
    const MsgRequestAssaultPrice& message =
        static_cast<const MsgRequestAssaultPrice&>(base);
    stRequestAssaultPrice& param =
        static_cast<stRequestAssaultPrice&>(raw);
    CUser* const target = G_GameWorld()->find_from_world(message.item_unique_id);
    if (!target || target->get_state() <= 2 || !target->getCurCharacR()) {
        param.result_code = 3;
        return 0;
    }

    param.requested_money = pvp_assault::GetInstanceAssaultMgr()
                                ->GetRequestAssaultNeedMoney(user, target);
    param.requested_money = ApplyEquipmentUpgradeDiscount(
        user, param.requested_money);
    user->SetCurCharacAssaultMoney(param.requested_money);
    user->SetCurCharacAssaultedUserId(
        static_cast<uint16_t>(target->get_unique_id()));

    param.result_code = check_error(user, message);
    if (param.result_code < 0) {
        return LineFunc(
            20724,
            "virtual int Dispatcher_RequestAssaultPrice::process(CUser*, MSG_BASE&, ParamBase&)",
            param.result_code, 0);
    }
    return 0;
}

// IDA_EA: 0x81DF294
// IDA_NAME: _ZN30Dispatcher_RequestAssaultPrice4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_RequestAssaultPrice::send(CUser *,ParamBase &)
void Dispatcher_RequestAssaultPrice::send(CUser* user, ParamBase& raw) {
    const stRequestAssaultPrice& param =
        static_cast<const stRequestAssaultPrice&>(raw);
    PacketGuard response;
    response.clear();
    response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                        ENUM_CMDPACKET_REQUEST_ASSAULT_PRICE);
    if (param.result_code == 0) {
        response.put_byte(1);
        response.put_int(param.requested_money);
    } else if (param.result_code != kNoResponseResult) {
        response.put_byte(0);
        response.put_byte(param.result_code);
        if (param.result_code == 10)
            response.put_int(param.requested_money - user->getCurCharacMoney());
    } else {
        return;
    }
    response.finalize(true);
    user->Send(response);
}

// The original CMD121 state-3 dispatcher exports the generic base slots from
// this unit; the concrete class alone does not instantiate those template
// bodies.
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
template class IPacketDispatcher<MsgRequestAssaultPrice,
                                 stRequestAssaultPrice,
                                 static_cast<ch_state>(3)>;
