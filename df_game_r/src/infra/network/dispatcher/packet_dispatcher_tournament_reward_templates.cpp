#include "old_game/activity/reward/packet_dispatcher_tournament_reward_select_state.h"
#include "old_game/activity/reward/packet_dispatcher_tournament_reward_select.h"

#include <type_traits>

#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

namespace {

// IDA 0x824C162 reserves a physical unconstructed MSG_BASE extent.  Keeping
// storage rather than a constructed MSG_BASE preserves the original missing
// MSG_BASE::C2 call edge without replacing its named virtual ABI type.
typedef std::aligned_storage<sizeof(MSG_BASE), alignof(MSG_BASE)>::type
    TournamentRewardSelectStateMessageStorage;

static_assert(sizeof(TournamentRewardSelectStateMessageStorage) == 0x0du,
              "CMD458 dispatch-template request extent changed");

}  // namespace

// IDA_EA: 0x8598F7E
// IDA_NAME: _ZN17IPacketDispatcherI25MsgTournamentRewardSelect24stTournamentRewardSelectL8ch_state0EEC2Ev
template <>
IPacketDispatcher<MsgTournamentRewardSelect,
                  stTournamentRewardSelect,
                  static_cast<ch_state>(0)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825B842 / 0x825B872
// IDA_NAME: _ZN17IPacketDispatcherI25MsgTournamentRewardSelect24stTournamentRewardSelectL8ch_state0EED2Ev / D0Ev
template <>
IPacketDispatcher<MsgTournamentRewardSelect,
                  stTournamentRewardSelect,
                  static_cast<ch_state>(0)>::~IPacketDispatcher() {}

// IDA_EA: 0x824BF00
// IDA_NAME: _ZN17IPacketDispatcherI25MsgTournamentRewardSelect24stTournamentRewardSelectL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
template <>
int IPacketDispatcher<MsgTournamentRewardSelect,
                      stTournamentRewardSelect,
                      static_cast<ch_state>(0)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    MsgTournamentRewardSelect message;
    int result = read(packet, message);
    if (result != 0) {
        return result;
    }

    stTournamentRewardSelect param;
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

// IDA_EA: 0x824C05C
// IDA_NAME: _ZN17IPacketDispatcherI25MsgTournamentRewardSelect24stTournamentRewardSelectL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
template <>
int IPacketDispatcher<MsgTournamentRewardSelect,
                      stTournamentRewardSelect,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x824C066
// IDA_NAME: _ZN17IPacketDispatcherI25MsgTournamentRewardSelect24stTournamentRewardSelectL8ch_state0EE11check_stateEP5CUserR9PacketBuf
template <>
int IPacketDispatcher<MsgTournamentRewardSelect,
                      stTournamentRewardSelect,
                      static_cast<ch_state>(0)>::check_state(CUser* user,
                                                               PacketBuf& packet) {
    if (user->get_state() >= 0) {
        return 0;
    }

    packet.set_index(1);
    short code = 0;
    packet.get_short(code);
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MsgTournamentRewardSelect, PARAM_TYPE = stTournamentRewardSelect, ch_state STATE = (ch_state)0u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825D600
// IDA_NAME: _ZN17IPacketDispatcherI25MsgTournamentRewardSelect24stTournamentRewardSelectL8ch_state0EE4sendEP5CUserR9ParamBase
template <>
void IPacketDispatcher<MsgTournamentRewardSelect,
                       stTournamentRewardSelect,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x8598F62
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE29stTournamentRewardSelectStateL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stTournamentRewardSelectState,(ch_state)0>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_BASE,
                  stTournamentRewardSelectState,
                  static_cast<ch_state>(0)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825B890
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE29stTournamentRewardSelectStateL8ch_state0EED2Ev
// IDA_EA: 0x825B8C0
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE29stTournamentRewardSelectStateL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stTournamentRewardSelectState,(ch_state)0>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_BASE,
                  stTournamentRewardSelectState,
                  static_cast<ch_state>(0)>::~IPacketDispatcher() {}

// IDA_EA: 0x824C162
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE29stTournamentRewardSelectStateL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stTournamentRewardSelectState,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE,
                      stTournamentRewardSelectState,
                      static_cast<ch_state>(0)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    TournamentRewardSelectStateMessageStorage message_storage;
    MSG_BASE& message = *reinterpret_cast<MSG_BASE*>(&message_storage);
    int result = read(packet, message);
    if (result != 0) {
        return result;
    }

    stTournamentRewardSelectState param;
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

// IDA_EA: 0x824C2B2
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE29stTournamentRewardSelectStateL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stTournamentRewardSelectState,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE,
                      stTournamentRewardSelectState,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x824C2BC
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE29stTournamentRewardSelectStateL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stTournamentRewardSelectState,(ch_state)0>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE,
                      stTournamentRewardSelectState,
                      static_cast<ch_state>(0)>::check_state(CUser* user,
                                                               PacketBuf& packet) {
    if (user->get_state() >= 0) {
        return 0;
    }

    packet.set_index(1);
    short code = 0;
    packet.get_short(code);
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_BASE, PARAM_TYPE = stTournamentRewardSelectState, ch_state STATE = (ch_state)0u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825D606
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE29stTournamentRewardSelectStateL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_BASE,stTournamentRewardSelectState,(ch_state)0>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_BASE,
                       stTournamentRewardSelectState,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&) {}
