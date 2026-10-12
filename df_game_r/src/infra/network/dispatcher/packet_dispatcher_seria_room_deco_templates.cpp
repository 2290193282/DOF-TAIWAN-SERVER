#include <type_traits>

#include "infra/log/format/trace.h"
#include "old_game/dungeon/room/packet_dispatcher_seria_room_deco.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

namespace {

// The original state-0 dispatcher at 0x82496C4 reserves a physical,
// unconstructed request extent: there is no MSG_SERIAROOM_DECO_EVENT C2 in
// its call graph.  Keep that exact lifetime while giving the derived callback
// a named protocol layout.
typedef std::aligned_storage<sizeof(MSG_SERIAROOM_DECO_EVENT),
                             alignof(MSG_SERIAROOM_DECO_EVENT)>::type
    RawSeriaRoomDecoMessageStorage;

static_assert(sizeof(RawSeriaRoomDecoMessageStorage) ==
                  sizeof(MSG_SERIAROOM_DECO_EVENT),
              "CMD495 raw request extent must stay 16 bytes");

}  // namespace

// IDA_EA: 0x85990EA
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_SERIAROOM_DECO_EVENT20stSeriaRoomDecoEventL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,stSeriaRoomDecoEvent,(ch_state)0>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                  stSeriaRoomDecoEvent,
                  static_cast<ch_state>(0)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825B278
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_SERIAROOM_DECO_EVENT20stSeriaRoomDecoEventL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,stSeriaRoomDecoEvent,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825B2A8
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_SERIAROOM_DECO_EVENT20stSeriaRoomDecoEventL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,stSeriaRoomDecoEvent,(ch_state)0>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                  stSeriaRoomDecoEvent,
                  static_cast<ch_state>(0)>::~IPacketDispatcher() {}

// IDA_EA: 0x82496C4
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_SERIAROOM_DECO_EVENT20stSeriaRoomDecoEventL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,stSeriaRoomDecoEvent,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                      stSeriaRoomDecoEvent,
                      static_cast<ch_state>(0)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    RawSeriaRoomDecoMessageStorage message_storage;
    MSG_SERIAROOM_DECO_EVENT& message =
        *reinterpret_cast<MSG_SERIAROOM_DECO_EVENT*>(&message_storage);
    int result = read(packet, message);
    if (result != 0) {
        return result;
    }

    stSeriaRoomDecoEvent param;
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

// IDA_EA: 0x8249814
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_SERIAROOM_DECO_EVENT20stSeriaRoomDecoEventL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,stSeriaRoomDecoEvent,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                      stSeriaRoomDecoEvent,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                 PacketBuf&) {
    return 0;
}

// IDA_EA: 0x824981E
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_SERIAROOM_DECO_EVENT20stSeriaRoomDecoEventL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,stSeriaRoomDecoEvent,(ch_state)0>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                      stSeriaRoomDecoEvent,
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
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_SERIAROOM_DECO_EVENT, PARAM_TYPE = stSeriaRoomDecoEvent, ch_state STATE = (ch_state)0u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825D5A0
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_SERIAROOM_DECO_EVENT20stSeriaRoomDecoEventL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,stSeriaRoomDecoEvent,(ch_state)0>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_SERIAROOM_DECO_EVENT,
                       stSeriaRoomDecoEvent,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&) {}
