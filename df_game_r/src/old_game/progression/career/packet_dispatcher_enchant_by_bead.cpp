#include "old_game/progression/career/packet_dispatcher_enchant_by_bead.h"

#include <set>
#include <type_traits>

#include "common/manager/global_tools.h"
#include "common/manager/global_data.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/progression/career/packet_dispatcher_expert_job_abi.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

namespace {

const char kReadSignature[] =
    "virtual int Dispatcher_EnchantByBead::read(PacketBuf&, MSG_BASE&)";
const char kProcessSignature[] =
    "virtual int Dispatcher_EnchantByBead::process(CUser*, MSG_BASE&, ParamBase&)";
const char kStateSignature[] =
    "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, "
    "PacketBuf&) [with MSG_TYPE = MSG_ENCHANT_BY_BEAD, "
    "PARAM_TYPE = stEnchantByBeadParam, ch_state STATE = (ch_state)3u]";

}  // namespace

// IDA_EA: 0x823148E
// IDA_NAME: _ZN20stEnchantByBeadParamC2Ev
// DEMANGLED: stEnchantByBeadParam::stEnchantByBeadParam(void)
stEnchantByBeadParam::stEnchantByBeadParam() : ParamBase(), result(0) {}

// IDA_EA: 0x824725E / 0x824728E
// IDA_NAME: _ZN20stEnchantByBeadParamD2Ev / D0Ev
// DEMANGLED: stEnchantByBeadParam::~stEnchantByBeadParam()
stEnchantByBeadParam::~stEnchantByBeadParam() {}

// The old template keeps the incoming 20-byte request as unconstructed
// stack storage.  Declaring this specialization first prevents an implicit
// generic body from adding a MSG_BASE constructor edge before read().
template <>
int IPacketDispatcher<MSG_ENCHANT_BY_BEAD,
                      stEnchantByBeadParam,
                      static_cast<ch_state>(3)>::check_state(
    CUser* user,
    PacketBuf& packet);

// IDA_EA: 0x8252E54
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_ENCHANT_BY_BEAD20stEnchantByBeadParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
template <>
int IPacketDispatcher<MSG_ENCHANT_BY_BEAD,
                      stEnchantByBeadParam,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (check_state(user, packet) > 0) return 0;

    typedef typename std::aligned_storage<sizeof(MSG_ENCHANT_BY_BEAD),
                                          alignof(MSG_ENCHANT_BY_BEAD)>::type
        RawMessageStorage;
    RawMessageStorage storage;
    MSG_ENCHANT_BY_BEAD& message =
        *reinterpret_cast<MSG_ENCHANT_BY_BEAD*>(&storage);

    int result = read(packet, static_cast<MSG_BASE&>(message));
    if (result != 0) return result;

    stEnchantByBeadParam param;
    result = process(user, static_cast<MSG_BASE&>(message),
                     static_cast<ParamBase&>(param));
    if (result > 0) return result;
    if (result < 0) return 0;

    send(user, static_cast<ParamBase&>(param));
    return dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x8252FAE
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_ENCHANT_BY_BEAD20stEnchantByBeadParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
template <>
int IPacketDispatcher<MSG_ENCHANT_BY_BEAD,
                      stEnchantByBeadParam,
                      static_cast<ch_state>(3)>::check_state(
    CUser* user,
    PacketBuf& packet) {
    if (user->get_state() > 2) return 0;

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(1,
                           "PacketDispatcher.h",
                           kStateSignature,
                           148,
                           "return __LINE__: code(%d) size(%d)",
                           code,
                           packet.get_len());
    return 149;
}

// IDA_EA: 0x8252FA4 / 0x825D894 / 0x825C8B6 / 0x825C8E6 / 0x8598942
// IDA_NAME: IPacketDispatcher<MSG_ENCHANT_BY_BEAD,stEnchantByBeadParam,(ch_state)3>
template class IPacketDispatcher<MSG_ENCHANT_BY_BEAD,
                                 stEnchantByBeadParam,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x8596EE0
// IDA_NAME: _ZN24Dispatcher_EnchantByBeadC2Ev
// DEMANGLED: Dispatcher_EnchantByBead::Dispatcher_EnchantByBead(void)
Dispatcher_EnchantByBead::Dispatcher_EnchantByBead()
    : IPacketDispatcher<MSG_ENCHANT_BY_BEAD,
                        stEnchantByBeadParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x8245688
// IDA_NAME: _ZN24Dispatcher_EnchantByBeadD2Ev
// DEMANGLED: Dispatcher_EnchantByBead::~Dispatcher_EnchantByBead()
// IDA_EA: 0x82456B8
// IDA_NAME: _ZN24Dispatcher_EnchantByBeadD0Ev
// DEMANGLED: Dispatcher_EnchantByBead::~Dispatcher_EnchantByBead()
Dispatcher_EnchantByBead::~Dispatcher_EnchantByBead() {}

// IDA_EA: 0x81D2218
// IDA_NAME: _ZN24Dispatcher_EnchantByBead4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_EnchantByBead::read(PacketBuf &,MSG_BASE &)
int Dispatcher_EnchantByBead::read(PacketBuf& packet, MSG_BASE& base_message) {
    MSG_ENCHANT_BY_BEAD& message =
        static_cast<MSG_ENCHANT_BY_BEAD&>(base_message);
    if (packet.get_byte(message.source_item_space) != 1) {
        return LineFunc(12060, kReadSignature, 0, 0);
    }
    if (packet.get_short(message.bead_slot) != 1) {
        return LineFunc(12061, kReadSignature, 0, 0);
    }
    if (packet.get_byte(message.target_item_space) != 1) {
        return LineFunc(12063, kReadSignature, 0, 0);
    }
    if (packet.get_short(message.target_slot) != 1) {
        return LineFunc(12064, kReadSignature, 0, 0);
    }
    return 0;
}

// IDA_EA: 0x81D233A
// IDA_NAME: _ZN24Dispatcher_EnchantByBead11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_EnchantByBead::check_error(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_EnchantByBead::check_error(CUser* user, MSG_BASE& base_message) {
    const MSG_ENCHANT_BY_BEAD& message =
        static_cast<const MSG_ENCHANT_BY_BEAD&>(base_message);
    if (user->get_state() <= 2) return 0x7fffffff;

    const int protection_result = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(43));
    if (protection_result != 0) return protection_result;

    CInventory* const inventory = user->getCurCharacInvenW();
    Inven_Item* const bead = inventory->GetInvenRef(1, message.bead_slot);
    if (!bead) return 17;
    Inven_Item* const target = inventory->GetInvenRef(1, message.target_slot);
    if (!target) return 17;

    CItem* const bead_item = G_CDataManager()->find_item(bead->item_id_);
    if (!bead_item) return 17;

    const std::set<int>& allowed_targets =
        static_cast<const CStackableItem*>(bead_item)->bead_target_item_ids();
    if (allowed_targets.empty()) return 0;
    return allowed_targets.find(target->item_id_) != allowed_targets.end()
               ? 0
               : 19;
}

// IDA_EA: 0x81D24B8
// IDA_NAME: _ZN24Dispatcher_EnchantByBead7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_EnchantByBead::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_EnchantByBead::process(CUser* user,
                                      MSG_BASE& base_message,
                                      ParamBase& base_param) {
    const MSG_ENCHANT_BY_BEAD& message =
        static_cast<const MSG_ENCHANT_BY_BEAD&>(base_message);
    stEnchantByBeadParam& param =
        static_cast<stEnchantByBeadParam&>(base_param);
    param.result = check_error(user, base_message);
    if (param.result > 0) return 0;
    if (param.result < 0) {
        return LineFunc(12129, kProcessSignature, param.result, 0);
    }

    const int result = GlobalData::s_ExpertJobMgr
                           ->OnEnchantByBead(user,
                                             message.source_item_space,
                                             message.bead_slot,
                                             message.target_item_space,
                                             message.target_slot);
    if (result != 0) param.result = result;
    return 0;
}

// IDA_EA: 0x81D25B0
// IDA_NAME: _ZN24Dispatcher_EnchantByBead4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_EnchantByBead::send(CUser *,ParamBase &)
void Dispatcher_EnchantByBead::send(CUser* user, ParamBase& base_param) {
    const stEnchantByBeadParam& param =
        static_cast<const stEnchantByBeadParam&>(base_param);
    if (param.result != 0x7fffffff && param.result != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ENCHANT_BY_BEAD,
                                 static_cast<unsigned char>(param.result));
    }
}
