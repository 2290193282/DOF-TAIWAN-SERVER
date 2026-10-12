#include "old_game/economy/equipment/packet_dispatcher_enchant_3rd_chronicle.h"
#include "infra/network/packet/PacketEnum.h"

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kReadSignature[] =
    "virtual int Dispatcher_Enchant_3rdChronicleItem::read(PacketBuf&, MSG_BASE&)";
const char kStateSignature[] =
    "virtual int Dispatcher_Enchant_3rdChronicleItem::check_state(CUser*, PacketBuf&)";
const char kTemplateStateSignature[] =
    "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, "
    "PacketBuf&) [with MSG_TYPE = MSG_ENCHANT, PARAM_TYPE = stEnchantParam, "
    "ch_state STATE = (ch_state)0u]";

}  // namespace

// Keep the state-0 template's historical diagnostic identity, rather than
// using the host compiler's __PRETTY_FUNCTION__ spelling.
template <>
int IPacketDispatcher<MSG_ENCHANT,
                      stEnchantParam,
                      static_cast<ch_state>(0)>::check_state(
    CUser* user,
    PacketBuf& packet) {
    if (user->get_state() >= 0) return 0;
    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(1, "PacketDispatcher.h", kTemplateStateSignature,
                           148, "return __LINE__: code(%d) size(%d)", code,
                           packet.get_len());
    return 149;
}

// IDA_EA: 0x82523A0 / 0x82524FC / 0x825D758 / 0x825D7EA / 0x825C6E2 /
// 0x825C712 / 0x8598B72.
template class IPacketDispatcher<MSG_ENCHANT,
                                 stEnchantParam,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x859761E
// IDA_NAME: _ZN35Dispatcher_Enchant_3rdChronicleItemC2Ev
// DEMANGLED: Dispatcher_Enchant_3rdChronicleItem::Dispatcher_Enchant_3rdChronicleItem(void)
Dispatcher_Enchant_3rdChronicleItem::Dispatcher_Enchant_3rdChronicleItem()
    : IPacketDispatcher<MSG_ENCHANT, stEnchantParam,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x82453CA
// IDA_NAME: _ZN35Dispatcher_Enchant_3rdChronicleItemD2Ev
// DEMANGLED: Dispatcher_Enchant_3rdChronicleItem::~Dispatcher_Enchant_3rdChronicleItem()
// IDA_EA: 0x82453FA
// IDA_NAME: _ZN35Dispatcher_Enchant_3rdChronicleItemD0Ev
// DEMANGLED: Dispatcher_Enchant_3rdChronicleItem::~Dispatcher_Enchant_3rdChronicleItem()
Dispatcher_Enchant_3rdChronicleItem::~Dispatcher_Enchant_3rdChronicleItem() {}

// IDA_EA: 0x81D343A
// IDA_NAME: _ZN35Dispatcher_Enchant_3rdChronicleItem11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_Enchant_3rdChronicleItem::check_error(CUser *,MSG_BASE &)
int Dispatcher_Enchant_3rdChronicleItem::check_error(CUser* user,
                                                       MSG_BASE& base_message) {
    MSG_ENCHANT& message = static_cast<MSG_ENCHANT&>(base_message);
    if (user->get_state() != 3 || !user->getCurCharacR()) return 7;
    const int security_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(39));
    if (security_error != 0) return security_error;
    if (user->CheckItemLock(1, message.target_slot)) return 213;

    CItem* const item = G_CDataManager()->find_item(
        static_cast<int>(message.material_item_id));
    if (!item) return 4;
    CInventory* const inventory = user->getCurCharacInvenW();
    if (!inventory) return 4;
    return inventory->GetInvenSlot(static_cast<INVEN_TYPE>(1),
                                   message.material_slot)
                       .item_id_ == item->get_index()
               ? 0
               : 23;
}

// IDA_EA: 0x81D35FE
// IDA_NAME: _ZN35Dispatcher_Enchant_3rdChronicleItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Enchant_3rdChronicleItem::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Enchant_3rdChronicleItem::read(PacketBuf& packet,
                                                MSG_BASE& base_message) {
    MSG_ENCHANT& message = static_cast<MSG_ENCHANT&>(base_message);
    if (packet.get_short(message.material_slot) != 1)
        return LineFunc(12632, kReadSignature, 0, 0);
    if (packet.get_int(message.material_item_id) != 1)
        return LineFunc(12634, kReadSignature, 0, 0);
    if (packet.get_short(message.target_slot) != 1)
        return LineFunc(12637, kReadSignature, 0, 0);
    if (packet.get_int(message.target_item_id) != 1)
        return LineFunc(12639, kReadSignature, 0, 0);
    if (packet.get_byte(message.enchant_type) != 1)
        return LineFunc(12642, kReadSignature, 0, 0);
    return 0;
}

// IDA_EA: 0x81D375E
// IDA_NAME: _ZN35Dispatcher_Enchant_3rdChronicleItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Enchant_3rdChronicleItem::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Enchant_3rdChronicleItem::process(CUser* user,
                                                   MSG_BASE& base_message,
                                                   ParamBase& base_param) {
    const int error = check_error(user, base_message);
    if (error > 0) {
        const MSG_ENCHANT& message = static_cast<const MSG_ENCHANT&>(base_message);
        LogManager::logFormat(1, "PacketDispatcher.cpp", __FUNCTION__, __LINE__,
            "[CHRONICLE_ENCHANT][REQUEST_REJECT] char=%d material=%u slot=%u "
            "target=%u slot=%u skill=%u error=%d",
            user->getCurCharacNo(), message.material_item_id,
            static_cast<unsigned int>(message.material_slot), message.target_item_id,
            static_cast<unsigned int>(message.target_slot),
            static_cast<unsigned int>(message.enchant_type), error);
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ENCHANT_3RD_CHRONICLE_ITEM,
                                 static_cast<uint8_t>(error));
        return -1;
    }

    stEnchantParam& param = static_cast<stEnchantParam&>(base_param);
    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_ENCHANT_3RD_CHRONICLE_ITEM);
    param.result = G_CDataManager()->get_item_upgrade()->ProcUpgrade(
        user, static_cast<MSG_ENCHANT*>(&base_message), &response);
    const MSG_ENCHANT& message = static_cast<const MSG_ENCHANT&>(base_message);
    LogManager::logFormat(1, "PacketDispatcher.cpp", __FUNCTION__, __LINE__,
        "[CHRONICLE_ENCHANT][RESULT] char=%d material=%u slot=%u target=%u "
        "slot=%u skill=%u error=%d",
        user->getCurCharacNo(), message.material_item_id,
        static_cast<unsigned int>(message.material_slot), message.target_item_id,
        static_cast<unsigned int>(message.target_slot),
        static_cast<unsigned int>(message.enchant_type), param.result);
    if (param.result == 0) {
        response.finalize(true);
        user->Send(response);
    }
    return 0;
}

// IDA_EA: 0x81D387E
// IDA_NAME: _ZN35Dispatcher_Enchant_3rdChronicleItem4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_Enchant_3rdChronicleItem::send(CUser *,ParamBase &)
void Dispatcher_Enchant_3rdChronicleItem::send(CUser* user,
                                                ParamBase& base_param) {
    const stEnchantParam& param = static_cast<const stEnchantParam&>(base_param);
    if (param.result != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ENCHANT_3RD_CHRONICLE_ITEM,
                                 static_cast<uint8_t>(param.result));
    }
}

// IDA_EA: 0x82328EC
// IDA_NAME: _ZN35Dispatcher_Enchant_3rdChronicleItem11check_stateEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_Enchant_3rdChronicleItem::check_state(CUser *,PacketBuf &)
int Dispatcher_Enchant_3rdChronicleItem::check_state(CUser* user,
                                                       PacketBuf& packet) {
    (void)packet;
    if (user->get_state() != 3 || !user->getCurCharacR()) {
        return LineFunc(1526, kStateSignature, 0, 0);
    }
    if (user->CheckInTrade()) return LineFunc(1529, kStateSignature, 0, 0);
    return 0;
}
