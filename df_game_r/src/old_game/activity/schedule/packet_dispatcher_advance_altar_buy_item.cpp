#include "old_game/activity/schedule/packet_dispatcher_advance_altar_buy_item.h"

#include "old_game/activity/reward/advance_altar.h"
#include "old_game/activity/reward/advance_altar_shop_parameter.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/activity/schedule/advance_altar_data.h"
#include "old_game/activity/schedule/advance_altar_stage_control_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

#include <cstring>

namespace CMDPacketStruct {

// IDA_EA: 0x8141CC8
// IDA_NAME: _ZN15CMDPacketStruct23_ReqAdvanceAltarBuyItemC2Ev
// DEMANGLED: CMDPacketStruct::_ReqAdvanceAltarBuyItem::_ReqAdvanceAltarBuyItem(void)
_ReqAdvanceAltarBuyItem::_ReqAdvanceAltarBuyItem() : STBaseRequest() {
    std::memset(this, 0, 0x20u);
}

// IDA_EA: 0x8141CF6
// IDA_NAME: _ZN15CMDPacketStruct23_ResAdvanceAltarBuyItemC2Ev
// DEMANGLED: CMDPacketStruct::_ResAdvanceAltarBuyItem::_ResAdvanceAltarBuyItem(void)
_ResAdvanceAltarBuyItem::_ResAdvanceAltarBuyItem() : STBaseResponse() {
    std::memset(this, 0, 0x1cu);
}

// IDA_EA: 0x814251C / 0x814254C
// IDA_NAME: _ZN15CMDPacketStruct23_ResAdvanceAltarBuyItemD2Ev / _ZN15CMDPacketStruct23_ResAdvanceAltarBuyItemD0Ev
// DEMANGLED: CMDPacketStruct::_ResAdvanceAltarBuyItem::~_ResAdvanceAltarBuyItem()
_ResAdvanceAltarBuyItem::~_ResAdvanceAltarBuyItem() {}

}  // namespace CMDPacketStruct

namespace advancealtar {

// IDA_EA: 0x813FAD0
// IDA_NAME: _ZN12advancealtar18Dispatcher_BuyItem12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: advancealtar::Dispatcher_BuyItem::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_BuyItem::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (!user || user->get_state() <= 2 || !user->getCurCharacR()) {
        return 896;
    }

    CMDPacketStruct::_ReqAdvanceAltarBuyItem request;
    CMDPacketStruct::_ResAdvanceAltarBuyItem response;
    read_packet(packet, request);
    const int result = process_packet(user, request, response);
    (void)result;
    send_packet(user, response);
    return 0;
}

// IDA_EA: 0x813FBB8
// IDA_NAME: _ZNK12advancealtar18Dispatcher_BuyItem11read_packetER9PacketBufRN15CMDPacketStruct13STBaseRequestE
// DEMANGLED: advancealtar::Dispatcher_BuyItem::read_packet(PacketBuf &,CMDPacketStruct::STBaseRequest &)const
int Dispatcher_BuyItem::read_packet(
    PacketBuf& packet,
    CMDPacketStruct::STBaseRequest& request) const {
    packet.get_binary(reinterpret_cast<char*>(&request), 0x20);
    return 0;
}

// IDA_EA: 0x813FBE6
// IDA_NAME: _ZNK12advancealtar18Dispatcher_BuyItem11check_errorEP5CUserRN15CMDPacketStruct13STBaseRequestE
// DEMANGLED: advancealtar::Dispatcher_BuyItem::check_error(CUser *,CMDPacketStruct::STBaseRequest &)const
int Dispatcher_BuyItem::check_error(
    CUser* user,
    CMDPacketStruct::STBaseRequest& request) const {
    (void)user;
    (void)request;
    return !isOpenAdvanceAltar();
}

// IDA_EA: 0x813FC06
// IDA_NAME: _ZNK12advancealtar18Dispatcher_BuyItem14process_packetEP5CUserRN15CMDPacketStruct13STBaseRequestERNS3_14STBaseResponseE
// DEMANGLED: advancealtar::Dispatcher_BuyItem::process_packet(CUser *,CMDPacketStruct::STBaseRequest &,CMDPacketStruct::STBaseResponse &)const
int Dispatcher_BuyItem::process_packet(
    CUser* user,
    CMDPacketStruct::STBaseRequest& request,
    CMDPacketStruct::STBaseResponse& response) const {
    CMDPacketStruct::_ReqAdvanceAltarBuyItem& req =
        static_cast<CMDPacketStruct::_ReqAdvanceAltarBuyItem&>(request);
    CMDPacketStruct::_ResAdvanceAltarBuyItem& res =
        static_cast<CMDPacketStruct::_ResAdvanceAltarBuyItem&>(response);

    res.ridable_id_08 = req.ridable_id_10;
    res.shop_type_0c = req.shop_type_14;
    res.upgrade_id_10 = req.upgrade_id_18;
    res.first_level_14 = req.first_level_1c;
    res.last_level_16 = req.last_level_1e;
    res.result_04 = check_error(user, request);
    if (res.result_04) {
        return 0;
    }

    CDataManager* const data_manager = G_CDataManager();
    std::vector<BuyUpgradeData>* const upgrades =
        data_manager->get_advance_altar_shop_parameter()->getBuyUpgradeDataVec(
            static_cast<int>(req.ridable_id_10),
            static_cast<AdvanceAltarShopType::T>(req.shop_type_14));
    if (!upgrades) {
        res.result_04 = 3;
        return 0;
    }

    BuyUpgradeData* selected_upgrade = 0;
    for (std::vector<BuyUpgradeData>::iterator it = upgrades->begin();
         it != upgrades->end(); ++it) {
        if (it->id == static_cast<int32_t>(req.upgrade_id_18)) {
            selected_upgrade = &*it;
            break;
        }
    }
    if (!selected_upgrade) {
        res.result_04 = 3;
        return 0;
    }

    int total_star = 0;
    for (int level = req.first_level_1c; level <= req.last_level_1e;
         ++level) {
        FieldData* const price =
            selected_upgrade->getFieldDataPoint(level, FieldType::PRICE);
        if (!price) {
            res.result_04 = 3;
            return 0;
        }
        total_star += price->int_value;
    }

    CharacAdvanceAltarManager* const manager =
        &user->advance_altar_manager_;
    int remaining_star = 0;
    res.result_04 = manager->buyItem(
        static_cast<AdvanceAltarShopType::T>(req.shop_type_14),
        static_cast<int>(req.upgrade_id_18), req.first_level_1c,
        req.last_level_1e, total_star, remaining_star);
    res.remaining_star_18 = remaining_star;
    if (!res.result_04) {
        HistoryLog::starSub(*user, SubStarType::BUY_ITEM, total_star);
    }
    return 0;
}

// IDA_EA: 0x813FE82
// IDA_NAME: _ZN12advancealtar18Dispatcher_BuyItem11send_packetEP5CUserRN15CMDPacketStruct14STBaseResponseE
// DEMANGLED: advancealtar::Dispatcher_BuyItem::send_packet(CUser *,CMDPacketStruct::STBaseResponse &)
void Dispatcher_BuyItem::send_packet(
    CUser* user,
    CMDPacketStruct::STBaseResponse& response) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_ADVANCE_ALTAR_BUY_ITEM);
    packet.put_byte(1);
    packet.put_binary(reinterpret_cast<char*>(&response), 0x1c);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x81421D8
// IDA_NAME: _ZN12advancealtar18Dispatcher_BuyItemD2Ev
// DEMANGLED: advancealtar::Dispatcher_BuyItem::~Dispatcher_BuyItem()
// IDA_EA: 0x8142208
// IDA_NAME: _ZN12advancealtar18Dispatcher_BuyItemD0Ev
// DEMANGLED: advancealtar::Dispatcher_BuyItem::~Dispatcher_BuyItem()
Dispatcher_BuyItem::~Dispatcher_BuyItem() {}

// IDA_EA: 0x8142226
// IDA_NAME: _ZN12advancealtar18Dispatcher_BuyItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: advancealtar::Dispatcher_BuyItem::read(PacketBuf &,MSG_BASE &)
int Dispatcher_BuyItem::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8142230
// IDA_NAME: _ZN12advancealtar18Dispatcher_BuyItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: advancealtar::Dispatcher_BuyItem::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_BuyItem::process(CUser* user,
                                MSG_BASE& message,
                                ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x85956EC
// IDA_NAME: _ZN12advancealtar18Dispatcher_BuyItemC2Ev
// DEMANGLED: advancealtar::Dispatcher_BuyItem::Dispatcher_BuyItem(void)
Dispatcher_BuyItem::Dispatcher_BuyItem()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

}  // namespace advancealtar
