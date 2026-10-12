#include "old_game/economy/inventory/packet_dispatcher_clear_used_qp.h"

#include <type_traits>

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mall/quest_shop_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

typedef std::aligned_storage<sizeof(MSG_BASE), alignof(MSG_BASE)>::type
    RawMsgClearUsedQpStorage;

static_assert(sizeof(RawMsgClearUsedQpStorage) == sizeof(MSG_BASE),
              "CMD400 must retain the old unconstructed 13-byte MSG_BASE local");

const char kClearUsedQpProcessSignature[] =
    "virtual int Dispatcher_Clear_Used_QP::process(CUser*, MSG_BASE&, ParamBase&)";
const char kClearUsedQpTemplateCheckStateSignature[] =
    "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_BASE, PARAM_TYPE = STQuestShopParam, ch_state STATE = (ch_state)0u]";

}  // namespace

// These declarations precede the dispatcher constructor so G++ cannot emit a
// primary-template vtable.  CMD400 has a distinct old-IDB template triplet.
template <>
IPacketDispatcher<MSG_BASE,
                  STQuestShopParam,
                  static_cast<ch_state>(0)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_BASE,
                  STQuestShopParam,
                  static_cast<ch_state>(0)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_BASE,
                      STQuestShopParam,
                      static_cast<ch_state>(0)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE,
                      STQuestShopParam,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE,
                      STQuestShopParam,
                      static_cast<ch_state>(0)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_BASE,
                       STQuestShopParam,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&);

// IDA_EA: 0x8598C6E
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE16STQuestShopParamL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,STQuestShopParam,(ch_state)0>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_BASE,
                  STQuestShopParam,
                  static_cast<ch_state>(0)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825C1B4
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE16STQuestShopParamL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,STQuestShopParam,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825C1E4
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE16STQuestShopParamL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,STQuestShopParam,(ch_state)0>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_BASE,
                  STQuestShopParam,
                  static_cast<ch_state>(0)>::~IPacketDispatcher() {}

// IDA_EA: 0x82500DC
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE16STQuestShopParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,STQuestShopParam,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE,
                      STQuestShopParam,
                      static_cast<ch_state>(0)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }

    // The old body reserves an unconstructed 13-byte MSG_BASE stack extent.
    // MSG_BASE::C2 is deliberately not called on this path.
    RawMsgClearUsedQpStorage message_storage;
    MSG_BASE& message = *reinterpret_cast<MSG_BASE*>(&message_storage);
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }

    STQuestShopParam param;
    result = this->process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    this->send(user, param);
    return this->dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x825022C
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE16STQuestShopParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,STQuestShopParam,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE,
                      STQuestShopParam,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x8250236
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE16STQuestShopParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,STQuestShopParam,(ch_state)0>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE,
                      STQuestShopParam,
                      static_cast<ch_state>(0)>::check_state(CUser* user,
                                                               PacketBuf& packet) {
    if (user->get_state() >= 0) {
        return 0;
    }

    packet.set_index(1);
    // The old diagnostic path supplies an uninitialized local when get_short
    // fails; preserve that path rather than inventing a packet code.
    short code;
    packet.get_short(code);
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        kClearUsedQpTemplateCheckStateSignature,
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825D722
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE16STQuestShopParamL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_BASE,STQuestShopParam,(ch_state)0>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_BASE,
                       STQuestShopParam,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x859776E
// IDA_NAME: _ZN24Dispatcher_Clear_Used_QPC2Ev
// DEMANGLED: Dispatcher_Clear_Used_QP::Dispatcher_Clear_Used_QP(void)
Dispatcher_Clear_Used_QP::Dispatcher_Clear_Used_QP()
    : IPacketDispatcher<MSG_BASE,
                        STQuestShopParam,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8244DB2
// IDA_NAME: _ZN24Dispatcher_Clear_Used_QPD2Ev
// DEMANGLED: Dispatcher_Clear_Used_QP::~Dispatcher_Clear_Used_QP()
// IDA_EA: 0x8244DE2
// IDA_NAME: _ZN24Dispatcher_Clear_Used_QPD0Ev
// DEMANGLED: Dispatcher_Clear_Used_QP::~Dispatcher_Clear_Used_QP()
Dispatcher_Clear_Used_QP::~Dispatcher_Clear_Used_QP() {}

// IDA_EA: 0x81D6746
// IDA_NAME: _ZN24Dispatcher_Clear_Used_QP11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_Clear_Used_QP::check_error(CUser *,MSG_BASE &)
int Dispatcher_Clear_Used_QP::check_error(CUser* user, MSG_BASE& message) {
    (void)message;
    if (!user) {
        return -1;
    }
    if (user->get_state() == 3) {
        return 0;
    }
    return 99;
}

// IDA_EA: 0x81D677C
// IDA_NAME: _ZN24Dispatcher_Clear_Used_QP4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Clear_Used_QP::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Clear_Used_QP::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81D6786
// IDA_NAME: _ZN24Dispatcher_Clear_Used_QP7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Clear_Used_QP::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Clear_Used_QP::process(CUser* user,
                                      MSG_BASE& message,
                                      ParamBase& param) {
    const int check_result = check_error(user, message);
    if (check_result > 0) {
        return -1;
    }
    if (check_result < 0) {
        return LineFunc(16433, kClearUsedQpProcessSignature, 0, 0);
    }

    STQuestShopParam& result = static_cast<STQuestShopParam&>(param);
    CQuestShop* const quest_shop = static_cast<CQuestShop*>(
        user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(11)));
    if (!quest_shop) {
        result.result = 1;
        return 0;
    }

    const int quest_shop_result = quest_shop->clearUsedQp(user);
    if (quest_shop_result) {
        result.result = quest_shop_result;
    }
    return 0;
}

// IDA_EA: 0x81D6850
// IDA_NAME: _ZN24Dispatcher_Clear_Used_QP4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_Clear_Used_QP::send(CUser *,ParamBase &)
void Dispatcher_Clear_Used_QP::send(CUser* user, ParamBase& param) {
    const STQuestShopParam& result = static_cast<const STQuestShopParam&>(param);
    if (result.result) {
        PacketGuard packet;
        packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                          ENUM_CMDPACKET_CLEAR_USED_QP);
        packet.put_byte(0);
        packet.put_short(result.result);
        packet.finalize(true);
        user->Send(packet);
        return;
    }

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_CLEAR_USED_QP);
    packet.put_byte(1);
    CInventory* const inventory = user->getCurCharacInvenR();
    packet.put_int(inventory->get_money());
    packet.finalize(true);
    user->Send(packet);
    user->sendCharacQp();
    user->sendCharacQuestPiece();
}
