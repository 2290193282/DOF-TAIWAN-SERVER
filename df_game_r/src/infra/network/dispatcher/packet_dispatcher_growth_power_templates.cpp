#include <type_traits>

#include "infra/log/format/trace.h"
#include "old_game/progression/quest/quest_skill_params.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

namespace {

// IDA 0x824E336 reserves the 13-byte MSG_BASE extent but has no
// MSG_BASE::C2 call edge.  This storage preserves that real lifetime while
// retaining MSG_BASE as the exact virtual-method protocol type.
typedef std::aligned_storage<sizeof(MSG_BASE), alignof(MSG_BASE)>::type
    UnconstructedGrowthPowerMessageStorage;

static_assert(sizeof(UnconstructedGrowthPowerMessageStorage) ==
                  sizeof(MSG_BASE),
              "CMD433 must retain the unconstructed 13-byte MSG_BASE local");

}  // namespace

// This translation unit is the sole owner of the CMD433 template vtable
// family.  Every member is declared before its explicit definition so no
// primary-template COMDAT provider can be emitted elsewhere.
template <>
IPacketDispatcher<MSG_BASE, stGrowthPowerParam,
                  static_cast<ch_state>(0)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_BASE, stGrowthPowerParam,
                  static_cast<ch_state>(0)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_BASE, stGrowthPowerParam,
                      static_cast<ch_state>(0)>::dispatch_template(
    CUser*, PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE, stGrowthPowerParam,
                      static_cast<ch_state>(0)>::dispatch_sig(
    CUser*, PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE, stGrowthPowerParam,
                      static_cast<ch_state>(0)>::check_state(
    CUser*, PacketBuf&);
template <>
void IPacketDispatcher<MSG_BASE, stGrowthPowerParam,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&);

// IDA_EA: 0x8598DA2
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE18stGrowthPowerParamL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stGrowthPowerParam,(ch_state)0>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_BASE, stGrowthPowerParam,
                  static_cast<ch_state>(0)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825BD70
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE18stGrowthPowerParamL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stGrowthPowerParam,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825BDA0
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE18stGrowthPowerParamL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stGrowthPowerParam,(ch_state)0>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_BASE, stGrowthPowerParam,
                  static_cast<ch_state>(0)>::~IPacketDispatcher() {}

// IDA_EA: 0x824E336
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE18stGrowthPowerParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stGrowthPowerParam,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE, stGrowthPowerParam,
                      static_cast<ch_state>(0)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    UnconstructedGrowthPowerMessageStorage message_storage;
    MSG_BASE& message = *reinterpret_cast<MSG_BASE*>(&message_storage);
    int result = read(packet, message);
    if (result != 0) {
        return result;
    }

    stGrowthPowerParam param;
    result = process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    send(user, param);
    return dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x824E486
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE18stGrowthPowerParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stGrowthPowerParam,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE, stGrowthPowerParam,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                 PacketBuf&) {
    return 0;
}

// IDA_EA: 0x824E490
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE18stGrowthPowerParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stGrowthPowerParam,(ch_state)0>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE, stGrowthPowerParam,
                      static_cast<ch_state>(0)>::check_state(
    CUser* user,
    PacketBuf& packet) {
    if (user->get_state() >= 0) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_BASE, PARAM_TYPE = stGrowthPowerParam, ch_state STATE = (ch_state)0u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825D65A
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE18stGrowthPowerParamL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_BASE,stGrowthPowerParam,(ch_state)0>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_BASE, stGrowthPowerParam,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&) {}
