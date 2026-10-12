#include "old_game/social/friend/friend_dispatcher.h"

#include <type_traits>

#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

namespace {

// The IDA dispatch template reserves an unconstructed 0x2f-byte request
// extent (there is no MSG_BASE::C2 edge at 0x825760E).  This storage models
// that observed automatic-object ABI while MSG_REMOVE_FRIEND supplies the
// complete named field layout used by the virtual read/process methods.
typedef std::aligned_storage<sizeof(MSG_REMOVE_FRIEND),
                             alignof(MSG_REMOVE_FRIEND)>::type
    RemoveFriendMessageStorage;

static_assert(sizeof(RemoveFriendMessageStorage) == 0x2fu,
              "CMD64 dispatch-template request extent changed");

typedef std::aligned_storage<sizeof(MSG_ADD_FRIEND),
                             alignof(MSG_ADD_FRIEND)>::type
    AddFriendMessageStorage;

static_assert(sizeof(AddFriendMessageStorage) == 0x2fu,
              "CMD63 dispatch-template request extent changed");

}  // namespace

// IDA_EA: 0x859851A
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_REMOVE_FRIEND19stRemoveFriendParamL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_REMOVE_FRIEND,stRemoveFriendParam,(ch_state)3>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_REMOVE_FRIEND,
                  stRemoveFriendParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825D3AE
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_REMOVE_FRIEND19stRemoveFriendParamL8ch_state3EED2Ev
// IDA_EA: 0x825D3DE
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_REMOVE_FRIEND19stRemoveFriendParamL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_REMOVE_FRIEND,stRemoveFriendParam,(ch_state)3>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_REMOVE_FRIEND,
                  stRemoveFriendParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher() {}

// IDA_EA: 0x825760E
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_REMOVE_FRIEND19stRemoveFriendParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_REMOVE_FRIEND,stRemoveFriendParam,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_REMOVE_FRIEND,
                      stRemoveFriendParam,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    RemoveFriendMessageStorage message_storage;
    MSG_REMOVE_FRIEND& message =
        *reinterpret_cast<MSG_REMOVE_FRIEND*>(&message_storage);
    int result = read(packet, message);
    if (result != 0) {
        return result;
    }

    stRemoveFriendParam param;
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

// IDA_EA: 0x825775E
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_REMOVE_FRIEND19stRemoveFriendParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_REMOVE_FRIEND,stRemoveFriendParam,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_REMOVE_FRIEND,
                      stRemoveFriendParam,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x8257768
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_REMOVE_FRIEND19stRemoveFriendParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_REMOVE_FRIEND,stRemoveFriendParam,(ch_state)3>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_REMOVE_FRIEND,
                      stRemoveFriendParam,
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
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_REMOVE_FRIEND, PARAM_TYPE = stRemoveFriendParam, ch_state STATE = (ch_state)3u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825D99C
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_REMOVE_FRIEND19stRemoveFriendParamL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_REMOVE_FRIEND,stRemoveFriendParam,(ch_state)3>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_REMOVE_FRIEND,
                       stRemoveFriendParam,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x85984FE
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_ADD_FRIEND16stAddFriendParamL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_ADD_FRIEND,stAddFriendParam,(ch_state)3>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_ADD_FRIEND,
                  stAddFriendParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825D3FC
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_ADD_FRIEND16stAddFriendParamL8ch_state3EED2Ev
// IDA_EA: 0x825D42C
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_ADD_FRIEND16stAddFriendParamL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_ADD_FRIEND,stAddFriendParam,(ch_state)3>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_ADD_FRIEND,
                  stAddFriendParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher() {}

// IDA_EA: 0x82577FC
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_ADD_FRIEND16stAddFriendParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_ADD_FRIEND,stAddFriendParam,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_ADD_FRIEND,
                      stAddFriendParam,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    AddFriendMessageStorage message_storage;
    MSG_ADD_FRIEND& message =
        *reinterpret_cast<MSG_ADD_FRIEND*>(&message_storage);
    int result = read(packet, message);
    if (result != 0) {
        return result;
    }

    stAddFriendParam param;
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

// IDA_EA: 0x825794C
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_ADD_FRIEND16stAddFriendParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_ADD_FRIEND,stAddFriendParam,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_ADD_FRIEND,
                      stAddFriendParam,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x8257956
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_ADD_FRIEND16stAddFriendParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_ADD_FRIEND,stAddFriendParam,(ch_state)3>::check_state(CUser* user,
//                                                                                    PacketBuf &)
template <>
int IPacketDispatcher<MSG_ADD_FRIEND,
                      stAddFriendParam,
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
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_ADD_FRIEND, PARAM_TYPE = stAddFriendParam, ch_state STATE = (ch_state)3u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825D9A2
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_ADD_FRIEND16stAddFriendParamL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_ADD_FRIEND,stAddFriendParam,(ch_state)3>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_ADD_FRIEND,
                       stAddFriendParam,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&) {}
