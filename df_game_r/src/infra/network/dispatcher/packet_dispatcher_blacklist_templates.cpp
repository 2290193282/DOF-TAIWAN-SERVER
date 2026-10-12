#include "old_game/progression/quest/packet_dispatcher_request_blacklist.h"

#include <type_traits>

#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

namespace {

// IDA 0x8255AB8 reserves an unconstructed 13-byte MSG_BASE stack extent;
// using storage here preserves the absence of a MSG_BASE::C2 call while the
// real message type remains the exact virtual-method parameter type.
typedef std::aligned_storage<sizeof(MSG_BASE), alignof(MSG_BASE)>::type
    RequestBlackListMessageStorage;

static_assert(sizeof(RequestBlackListMessageStorage) == 0x0du,
              "CMD123 dispatch-template request extent changed");

}  // namespace

// IDA_EA: 0x85986BE
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE23stRequestBlackListParamL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stRequestBlackListParam,(ch_state)3>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_BASE,
                  stRequestBlackListParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825CF6A
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE23stRequestBlackListParamL8ch_state3EED2Ev
// IDA_EA: 0x825CF9A
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE23stRequestBlackListParamL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stRequestBlackListParam,(ch_state)3>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_BASE,
                  stRequestBlackListParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher() {}

// IDA_EA: 0x8255AB8
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE23stRequestBlackListParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stRequestBlackListParam,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE,
                      stRequestBlackListParam,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    RequestBlackListMessageStorage message_storage;
    MSG_BASE& message = *reinterpret_cast<MSG_BASE*>(&message_storage);
    int result = read(packet, message);
    if (result != 0) {
        return result;
    }

    stRequestBlackListParam param;
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

// IDA_EA: 0x8255C08
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE23stRequestBlackListParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stRequestBlackListParam,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE,
                      stRequestBlackListParam,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x8255C12
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE23stRequestBlackListParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stRequestBlackListParam,(ch_state)3>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE,
                      stRequestBlackListParam,
                      static_cast<ch_state>(3)>::check_state(CUser* user,
                                                               PacketBuf& packet) {
    if (user->get_state() > 2) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_BASE, PARAM_TYPE = stRequestBlackListParam, ch_state STATE = (ch_state)3u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x8255CA6
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE23stRequestBlackListParamL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_BASE,stRequestBlackListParam,(ch_state)3>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_BASE,
                       stRequestBlackListParam,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&) {}
