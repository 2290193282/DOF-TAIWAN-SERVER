#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

#include "infra/log/format/trace.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

namespace {

static_assert(sizeof(MSG_PARTY_TELEPORT) == 0x14u,
              "CMD435 request must retain its original 20-byte extent");
static_assert(sizeof(stPartyTeleport) == 0x10u,
              "CMD435 parameter must retain its original 16-byte extent");
static_assert(sizeof(MSG_PARTY_TELEPORT_CONFIRM) == 0x0eu,
              "CMD436 request must retain its original 14-byte extent");
static_assert(sizeof(stPartyTeleportConfirm) == 0x0cu,
              "CMD436 parameter must retain its original 12-byte extent");

}  // namespace

// Declare the whole specialization before its C2 is emitted.  Otherwise the
// concrete dispatcher can cause the primary virtual members to become an
// unintended COMDAT provider.
template <>
IPacketDispatcher<MSG_PARTY_TELEPORT,
                  stPartyTeleport,
                  static_cast<ch_state>(0)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_PARTY_TELEPORT,
                  stPartyTeleport,
                  static_cast<ch_state>(0)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_PARTY_TELEPORT,
                      stPartyTeleport,
                      static_cast<ch_state>(0)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_PARTY_TELEPORT,
                      stPartyTeleport,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_PARTY_TELEPORT,
                      stPartyTeleport,
                      static_cast<ch_state>(0)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_PARTY_TELEPORT,
                       stPartyTeleport,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&);

// CMD436 is the neighbouring, distinct state-0 specialization in the old
// IPacketDispatcher<MSG> unit.  Keep its complete specialization here with
// CMD435, its sole A-domain owner, so the C-domain derived vtable cannot emit
// primary-template COMDAT replacements.
template <>
IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,
                  stPartyTeleportConfirm,
                  static_cast<ch_state>(0)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,
                  stPartyTeleportConfirm,
                  static_cast<ch_state>(0)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,
                      stPartyTeleportConfirm,
                      static_cast<ch_state>(0)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,
                      stPartyTeleportConfirm,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,
                      stPartyTeleportConfirm,
                      static_cast<ch_state>(0)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,
                       stPartyTeleportConfirm,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&);

// IDA_EA: 0x8598402
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_PARTY_TELEPORT15stPartyTeleportL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_PARTY_TELEPORT,stPartyTeleport,(ch_state)0>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_PARTY_TELEPORT,
                  stPartyTeleport,
                  static_cast<ch_state>(0)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825BD22
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_PARTY_TELEPORT15stPartyTeleportL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_PARTY_TELEPORT,stPartyTeleport,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825BD52
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_PARTY_TELEPORT15stPartyTeleportL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_PARTY_TELEPORT,stPartyTeleport,(ch_state)0>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_PARTY_TELEPORT,
                  stPartyTeleport,
                  static_cast<ch_state>(0)>::~IPacketDispatcher() {}

// IDA_EA: 0x824E13E
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_PARTY_TELEPORT15stPartyTeleportL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_PARTY_TELEPORT,stPartyTeleport,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_PARTY_TELEPORT,
                      stPartyTeleport,
                      static_cast<ch_state>(0)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    MSG_PARTY_TELEPORT message;
    int result = read(packet, message);
    if (result != 0) {
        return result;
    }

    stPartyTeleport param;
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

// IDA_EA: 0x824E29A
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_PARTY_TELEPORT15stPartyTeleportL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_PARTY_TELEPORT,stPartyTeleport,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_PARTY_TELEPORT,
                      stPartyTeleport,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x824E2A4
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_PARTY_TELEPORT15stPartyTeleportL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_PARTY_TELEPORT,stPartyTeleport,(ch_state)0>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_PARTY_TELEPORT,
                      stPartyTeleport,
                      static_cast<ch_state>(0)>::check_state(CUser* user,
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
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_PARTY_TELEPORT, PARAM_TYPE = stPartyTeleport, ch_state STATE = (ch_state)0u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825D654
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_PARTY_TELEPORT15stPartyTeleportL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_PARTY_TELEPORT,stPartyTeleport,(ch_state)0>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_PARTY_TELEPORT,
                       stPartyTeleport,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x85983E6
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_PARTY_TELEPORT_CONFIRM22stPartyTeleportConfirmL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,stPartyTeleportConfirm,(ch_state)0>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,
                  stPartyTeleportConfirm,
                  static_cast<ch_state>(0)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825BCD4
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_PARTY_TELEPORT_CONFIRM22stPartyTeleportConfirmL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,stPartyTeleportConfirm,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825BD04
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_PARTY_TELEPORT_CONFIRM22stPartyTeleportConfirmL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,stPartyTeleportConfirm,(ch_state)0>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,
                  stPartyTeleportConfirm,
                  static_cast<ch_state>(0)>::~IPacketDispatcher() {}

// IDA_EA: 0x824DEDC
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_PARTY_TELEPORT_CONFIRM22stPartyTeleportConfirmL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,stPartyTeleportConfirm,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,
                      stPartyTeleportConfirm,
                      static_cast<ch_state>(0)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    MSG_PARTY_TELEPORT_CONFIRM message;
    int result = read(packet, message);
    if (result != 0) {
        return result;
    }

    stPartyTeleportConfirm param;
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

// IDA_EA: 0x824E038
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_PARTY_TELEPORT_CONFIRM22stPartyTeleportConfirmL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,stPartyTeleportConfirm,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,
                      stPartyTeleportConfirm,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x824E042
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_PARTY_TELEPORT_CONFIRM22stPartyTeleportConfirmL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,stPartyTeleportConfirm,(ch_state)0>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,
                      stPartyTeleportConfirm,
                      static_cast<ch_state>(0)>::check_state(CUser* user,
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
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_PARTY_TELEPORT_CONFIRM, PARAM_TYPE = stPartyTeleportConfirm, ch_state STATE = (ch_state)0u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825D64E
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_PARTY_TELEPORT_CONFIRM22stPartyTeleportConfirmL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,stPartyTeleportConfirm,(ch_state)0>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,
                       stPartyTeleportConfirm,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&) {}
