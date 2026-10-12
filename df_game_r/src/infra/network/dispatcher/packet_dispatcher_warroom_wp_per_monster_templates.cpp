#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

#include "infra/log/format/trace.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

namespace {

static_assert(sizeof(MSG_WARROOM_WP_PER_MONSTER) == 0x19u,
              "CMD374 native request must retain its 25-byte extent");
#if defined(__i386__)
static_assert(sizeof(stWarRoomWp) == 0x08u,
              "CMD374 parameter must retain its original 8-byte extent");
#endif

}  // namespace

template <>
IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,
                  stWarRoomWp,
                  static_cast<ch_state>(0)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,
                  stWarRoomWp,
                  static_cast<ch_state>(0)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,
                      stWarRoomWp,
                      static_cast<ch_state>(0)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,
                      stWarRoomWp,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,
                      stWarRoomWp,
                      static_cast<ch_state>(0)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,
                       stWarRoomWp,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&);

// IDA_EA: 0x8598BFE
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_WARROOM_WP_PER_MONSTER11stWarRoomWpL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,stWarRoomWp,(ch_state)0>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,
                  stWarRoomWp,
                  static_cast<ch_state>(0)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825C2EC
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_WARROOM_WP_PER_MONSTER11stWarRoomWpL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,stWarRoomWp,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825C31C
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_WARROOM_WP_PER_MONSTER11stWarRoomWpL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,stWarRoomWp,(ch_state)0>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,
                  stWarRoomWp,
                  static_cast<ch_state>(0)>::~IPacketDispatcher() {}

// IDA_EA: 0x825089C
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_WARROOM_WP_PER_MONSTER11stWarRoomWpL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,stWarRoomWp,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,
                      stWarRoomWp,
                      static_cast<ch_state>(0)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    MSG_WARROOM_WP_PER_MONSTER message;
    int result = read(packet, static_cast<MSG_BASE&>(message));
    if (result != 0) {
        return result;
    }

    stWarRoomWp param;
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

// IDA_EA: 0x82509EC
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_WARROOM_WP_PER_MONSTER11stWarRoomWpL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,stWarRoomWp,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,
                      stWarRoomWp,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x82509F6
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_WARROOM_WP_PER_MONSTER11stWarRoomWpL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,stWarRoomWp,(ch_state)0>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,
                      stWarRoomWp,
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
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_WARROOM_WP_PER_MONSTER, PARAM_TYPE = stWarRoomWp, ch_state STATE = (ch_state)0u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825D73A
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_WARROOM_WP_PER_MONSTER11stWarRoomWpL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,stWarRoomWp,(ch_state)0>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,
                       stWarRoomWp,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&) {}
