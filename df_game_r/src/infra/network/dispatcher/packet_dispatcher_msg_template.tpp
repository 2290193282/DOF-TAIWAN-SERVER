#ifndef OLDSRV_PROTOCOL_PACKET_DISPATCHER_MSG_TEMPLATE_TPP_
#define OLDSRV_PROTOCOL_PACKET_DISPATCHER_MSG_TEMPLATE_TPP_

// This file is deliberately an instantiation-only include.  Include it in
// precisely one .cpp after MSG_TYPE and PARAM_TYPE are complete, then use an
// explicit template instantiation.  Do not include it from the public
// declaration header: each raw IPacketDispatcher triplet has one object/vtable
// owner and must not become a compiler-selected COMDAT provider.

#include <stdint.h>

#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

template <typename MSG_TYPE, typename PARAM_TYPE, ch_state STATE>
IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::IPacketDispatcher() {}

template <typename MSG_TYPE, typename PARAM_TYPE, ch_state STATE>
IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::~IPacketDispatcher() {}

template <typename MSG_TYPE, typename PARAM_TYPE, ch_state STATE>
int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::dispatch_template(
    CUser* user, PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    MSG_TYPE message;
    int result = read(packet, static_cast<MSG_BASE&>(message));
    if (result != 0) {
        return result;
    }

    PARAM_TYPE param;
    result = process(user,
                     static_cast<MSG_BASE&>(message),
                     static_cast<ParamBase&>(param));
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    send(user, static_cast<ParamBase&>(param));
    return dispatch_sig(user, packet) > 0 ? 133 : 0;
}

template <typename MSG_TYPE, typename PARAM_TYPE, ch_state STATE>
int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(
    CUser* user, PacketBuf& packet) {
    if (user->get_state() >= static_cast<int>(STATE)) {
        return 0;
    }

    packet.set_index(1);
    // The old i386 implementation passes an uninitialized stack short to
    // LogManager when get_short() fails.  Preserve that raw diagnostic-path
    // storage behavior instead of fabricating a zero packet code.
    short code;
    packet.get_short(code);
    LogManager::logFormat(1,
                           "PacketDispatcher.h",
                           __PRETTY_FUNCTION__,
                           148,
                           "return __LINE__: code(%d) size(%d)",
                           code,
                           packet.get_len());
    return 149;
}

template <typename MSG_TYPE, typename PARAM_TYPE, ch_state STATE>
void IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::send(
    CUser* user, ParamBase& param) {
    (void)user;
    (void)param;
}

template <typename MSG_TYPE, typename PARAM_TYPE, ch_state STATE>
int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::dispatch_sig(
    CUser* user, PacketBuf& packet) {
    (void)user;
    (void)packet;
    return 0;
}

#endif  // OLDSRV_PROTOCOL_PACKET_DISPATCHER_MSG_TEMPLATE_TPP_
