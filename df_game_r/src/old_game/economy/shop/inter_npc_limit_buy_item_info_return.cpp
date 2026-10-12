#include "old_game/economy/shop/inter_npc_limit_buy_item_info_return.h"

#include <cstring>
#include <string>

#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "common/manager/server_xml_runtime.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/mail/mail_trade_params.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

void SendLimitNpcBuyItemUpdate(const LimitNpcBuyItemUpdate& update,
                               int server_group) {
    CServerProxyMgr<CMonitorServerProxy>* const manager =
        GlobalData::s_monitor_proxy_mgr;
    if (!manager) {
        return;
    }

    CMonitorServerProxy* const proxy = manager->GetServerProxy(
        static_cast<ENUM_SERVER_GROUP>(server_group));
    if (proxy) {
        proxy->SendPacket(
            const_cast<char*>(reinterpret_cast<const char*>(&update)),
            static_cast<int>(sizeof(update)));
    }
}

void FillLimitNpcBuyItemUpdate(LimitNpcBuyItemUpdate& update,
                               int item_id,
                               int charac_no,
                               uint32_t response_tag,
                               int result) {
    update.item_id = static_cast<uint32_t>(item_id);
    update.charac_no = static_cast<uint32_t>(charac_no);
    update.response_tag = response_tag;
    update.result = static_cast<uint32_t>(result);
}

}  // namespace

// IDA_EA: 0x84E6502
// IDA_NAME: _ZN25Inter_NpcLimitBuyItemInfoC2Ev
// DEMANGLED: Inter_NpcLimitBuyItemInfo::Inter_NpcLimitBuyItemInfo(void)
Inter_NpcLimitBuyItemInfo::Inter_NpcLimitBuyItemInfo()
    : ISigDispatcher(),
      target_item_id_(0),
      need_material_(),
      inserted_slot_(-1),
      material_slot_(-1),
      target_prototype_(0),
      mailed_(false),
      target_item_(),
      material_item_() {
    reset();
}

// IDA_EA: 0x84E6554 / 0x84E6584
// IDA_NAME: _ZN25Inter_NpcLimitBuyItemInfoD2Ev / D0Ev
// DEMANGLED: Inter_NpcLimitBuyItemInfo::~Inter_NpcLimitBuyItemInfo()
Inter_NpcLimitBuyItemInfo::~Inter_NpcLimitBuyItemInfo() {}

// IDA_EA: 0x84E65A2
// IDA_NAME: _ZN25Inter_NpcLimitBuyItemInfo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_NpcLimitBuyItemInfo::dispatch_sig(CUser *,char *,int)
int Inter_NpcLimitBuyItemInfo::dispatch_sig(CUser*,
                                            char* payload,
                                            int payload_size) {
    reset();
    if (!payload || payload_size < static_cast<int>(sizeof(LimitNpcBuyItemInfo))) {
        return 0;
    }

    LimitNpcBuyItemInfo response;
    std::memcpy(&response, payload, sizeof(response));

    LimitNpcBuyItemUpdate update;
    CGameManager* const game_manager = G_CGameManager();
    CUser* const target = game_manager
                              ? game_manager->getUser(
                                    static_cast<int>(response.user_id), 0)
                              : 0;
    if (!target) {
        // The raw body asks the monitor manager for group zero when the UID
        // no longer maps to a live CUser.  Keep that acknowledgement so the
        // limit-purchase row can be released rather than left pending.
        FillLimitNpcBuyItemUpdate(update,
                                  static_cast<int>(response.item_id),
                                  static_cast<int>(response.charac_no),
                                  response.response_tag,
                                  3);
        SendLimitNpcBuyItemUpdate(update, 0);
        return 0;
    }

    if (response.error_code != 0u) {
        target->SendCmdErrorPacket(
            ENUM_CMDPACKET_LIMIT_NPC_BUY_ITEM,
            static_cast<unsigned char>(response.error_code));
        return 0;
    }

    const int error = checkError(target, &response);
    if (error != 0) {
        FillLimitNpcBuyItemUpdate(update,
                                  target_item_id_,
                                  target->get_charac_no(-1),
                                  response.response_tag,
                                  error);
        SendLimitNpcBuyItemUpdate(update, target->GetServerGroup());
        target->SendCmdErrorPacket(ENUM_CMDPACKET_LIMIT_NPC_BUY_ITEM,
                                   static_cast<unsigned char>(error));
        return 0;
    }

    target_item_.item_id_ = target_item_id_;
    G_CDataManager()->get_item_list()->create_item(
        ENUM_ITEM_CREATE_TYPE_DEFAULT, target_item_, 1);

    inserted_slot_ = target->getCurCharacInvenW()->insertItemIntoInventory(
        target_item_, static_cast<eItemAddReason>(0), true, false);
    mailed_ = false;
    if (inserted_slot_ < 0) {
        char title[21] = {};
        char text[256] = {};
        const std::string title_text = g_ServerString_.GetServerString(0x436, 0);
        const std::string body_text = g_ServerString_.GetServerString(0x437, 0);
        std::strncpy(title, title_text.c_str(), 0x14u);
        std::strncpy(text, body_text.c_str(), 0xffu);

        if (!WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
                title,
                target_item_,
                0,
                static_cast<unsigned int>(target->get_charac_no(-1)),
                text,
                static_cast<int>(std::strlen(text)),
                30,
                static_cast<ENUM_SERVER_GROUP>(target->GetServerGroup()),
                false,
                false)) {
            MSG_MAILBOX_SEND message = {};
            std::strncpy(message.receiver_name, title, 0x14u);
            message.item_id = target_item_.getKey();
            message.item_add_info = target_item_.get_add_info();
            target->history_log().SendMail(&message, 0);
        }
        mailed_ = true;
    }

    target->getCurCharacInvenW()->delete_item(
        static_cast<INVEN_TYPE>(1),
        material_slot_,
        need_material_.second,
        static_cast<eItemDelReason>(26),
        true);

    FillLimitNpcBuyItemUpdate(update,
                              target_item_id_,
                              target->get_charac_no(-1),
                              response.response_tag,
                              0);
    SendLimitNpcBuyItemUpdate(update, target->GetServerGroup());
    sendPacket(target);
    return 0;
}

// IDA_EA: 0x84E6ACC
// IDA_NAME: _ZN25Inter_NpcLimitBuyItemInfo10checkErrorEP5CUserP19LimitNpcBuyItemInfo
// DEMANGLED: Inter_NpcLimitBuyItemInfo::checkError(CUser *,LimitNpcBuyItemInfo *)
int Inter_NpcLimitBuyItemInfo::checkError(CUser* user,
                                          LimitNpcBuyItemInfo* response) {
    if (!user || !response || user->get_state() <= 2 ||
        user->get_charac_no(-1) != static_cast<int>(response->charac_no)) {
        return 3;
    }

    target_item_id_ = static_cast<int>(response->item_id);
    if (target_item_id_ == 0) {
        return 17;
    }

    target_prototype_ = G_CDataManager()->find_item(target_item_id_);
    if (!target_prototype_) {
        return 17;
    }

    // OFFICIAL_IDA_0828: Inter_NpcLimitBuyItemInfo::checkError @0x84E6ACC
    // shares CItem::GetNeedMaterial with CMD21; keep its override/fallback
    // semantics identical so limited and ordinary NPC shops charge the same
    // PVF-owned material price.
    const std::pair<int, int>* const material =
        target_prototype_->GetNeedMaterial();
    need_material_ = *material;
    if (need_material_.first == 0 || need_material_.first == -1) {
        return 23;
    }

    material_slot_ = user->getCurCharacInvenRefR()->GetInvenData(
        need_material_.first, material_item_);
    if (material_slot_ < 0) {
        return 21;
    }
    if (need_material_.second <= 0 ||
        need_material_.second > material_item_.get_add_info()) {
        return 22;
    }
    return 0;
}

// IDA_EA: 0x84E6C42
// IDA_NAME: _ZN25Inter_NpcLimitBuyItemInfo10sendPacketEP5CUser
// DEMANGLED: Inter_NpcLimitBuyItemInfo::sendPacket(CUser *)
void Inter_NpcLimitBuyItemInfo::sendPacket(CUser* user) {
    if (!user) {
        return;
    }

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_LIMIT_NPC_BUY_ITEM);
    packet.put_byte(1);
    packet.put_byte(mailed_ ? 1 : 0);
    if (!mailed_) {
        packet.put_short(static_cast<unsigned short>(inserted_slot_));
        packet.put_int(target_item_.item_id_);
        packet.put_int(target_item_.add_info_);
        packet.put_short(target_item_.avatar_select_);
        packet.put_byte(target_item_.amplify_option_.getAbilityType());
        packet.put_short(target_item_.amplify_option_.getAbilityValue());
        packet.put_packet(target_item_);
    }

    packet.put_int(material_item_.item_id_);
    const Inven_Item* const material = user->getCurCharacInvenR()->GetInvenRef(
        static_cast<INVEN_TYPE>(1), material_slot_);
    packet.put_int(material ? material->add_info_ : 0);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x84E6E1A
// IDA_NAME: _ZN25Inter_NpcLimitBuyItemInfo5resetEv
// DEMANGLED: Inter_NpcLimitBuyItemInfo::reset(void)
void Inter_NpcLimitBuyItemInfo::reset() {
    target_item_id_ = 0;
    need_material_.first = 0;
    need_material_.second = 0;
    inserted_slot_ = -1;
    material_slot_ = -1;
    target_prototype_ = 0;
    mailed_ = false;
    target_item_.reset();
    material_item_.reset();
}
