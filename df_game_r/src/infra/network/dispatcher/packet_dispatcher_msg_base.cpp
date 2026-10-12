#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

#include <stdint.h>

#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"


// IDA_EA: 0x8236A56
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)0>::IPacketDispatcher(void)
IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>::
    IPacketDispatcher() {}

// IDA_EA: 0x8125696
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x81256C6
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)0>::~IPacketDispatcher()
IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>::
    ~IPacketDispatcher() {}

// IDA_EA: 0x81258B6
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
int IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>::
    dispatch_template(CUser* user, PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    MSG_BASE message;
    int result = read(packet, message);
    if (result != 0) {
        return result;
    }

    ParamBase param;
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

// IDA_EA: 0x8125A06
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)0>::check_state(CUser *,PacketBuf &)
int IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>::
    check_state(CUser* user, PacketBuf& packet) {
    if (user->get_state() >= 0) {
        return 0;
    }

    packet.set_index(1);
    uint16_t code = 0;
    packet.get_short(code);
    LogManager::logFormat(
        1,
        "localchina/../PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_BASE, PARAM_TYPE = ParamBase, ch_state STATE = (ch_state)0u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x8125A98
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state0EE4sendEP5CUserRS1_
// DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)0>::send(CUser *,ParamBase&)
void IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>::send(
    CUser* user, ParamBase& param) {
    (void)user;
    (void)param;
}

// IDA_EA: 0x8125A9E
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
int IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>::
    dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)user;
    (void)packet;
    return 0;
}
