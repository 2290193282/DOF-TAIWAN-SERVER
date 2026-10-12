#include "old_game/progression/quest/pvp_mission_combo_clear_dispatcher.h"

#include <cstring>
#include <type_traits>

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "common/manager/os_api.h"

namespace {

const char kPvpMissionComboClearReadSignature[] =
    "virtual int Dispatcher_PvpMissionComboClear::read(PacketBuf&, MSG_BASE&)";
const char kPvpMissionComboClearProcessSignature[] =
    "virtual int Dispatcher_PvpMissionComboClear::process(CUser*, MSG_BASE&, ParamBase&)";
const char kPvpMissionComboClearTemplateCheckStateSignature[] =
    "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_PVP_MISSION_COMBO_CLEAR, PARAM_TYPE = ParamBase, ch_state STATE = (ch_state)3u]";

// Raw dispatch_template @0x8250A88 reserves this local without invoking
// MSG_PVP_MISSION_COMBO_CLEAR or MSG_BASE C2.  It is populated only by read.
typedef typename std::aligned_storage<sizeof(MSG_PVP_MISSION_COMBO_CLEAR),
                                      alignof(MSG_PVP_MISSION_COMBO_CLEAR)>::type
    UnconstructedPvpMissionComboClear;

}  // namespace

// Keep every declaration ahead of the raw dispatch_template definition so
// this translation unit cannot instantiate a generic state-3 provider.
template <>
IPacketDispatcher<MSG_PVP_MISSION_COMBO_CLEAR,
                  ParamBase,
                  static_cast<ch_state>(3)>::IPacketDispatcher();

template <>
IPacketDispatcher<MSG_PVP_MISSION_COMBO_CLEAR,
                  ParamBase,
                  static_cast<ch_state>(3)>::~IPacketDispatcher();

template <>
int IPacketDispatcher<MSG_PVP_MISSION_COMBO_CLEAR,
                      ParamBase,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet);

template <>
int IPacketDispatcher<MSG_PVP_MISSION_COMBO_CLEAR,
                      ParamBase,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser* user,
                                                               PacketBuf& packet);

template <>
int IPacketDispatcher<MSG_PVP_MISSION_COMBO_CLEAR,
                      ParamBase,
                      static_cast<ch_state>(3)>::check_state(CUser* user,
                                                              PacketBuf& packet);

template <>
void IPacketDispatcher<MSG_PVP_MISSION_COMBO_CLEAR,
                       ParamBase,
                       static_cast<ch_state>(3)>::send(CUser* user,
                                                        ParamBase& param);

// IDA_EA: 0x8250A88
// IDA_NAME: _ZN17IPacketDispatcherI27MSG_PVP_MISSION_COMBO_CLEAR9ParamBaseL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_PVP_MISSION_COMBO_CLEAR,ParamBase,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_PVP_MISSION_COMBO_CLEAR,
                      ParamBase,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    UnconstructedPvpMissionComboClear message_storage;
    MSG_PVP_MISSION_COMBO_CLEAR& message =
        *reinterpret_cast<MSG_PVP_MISSION_COMBO_CLEAR*>(&message_storage);
    int result = read(packet, static_cast<MSG_BASE&>(message));
    if (result != 0) {
        return result;
    }

    ParamBase param;
    result = process(user, static_cast<MSG_BASE&>(message), param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    send(user, param);
    return dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x8250BD8
// IDA_NAME: _ZN17IPacketDispatcherI27MSG_PVP_MISSION_COMBO_CLEAR9ParamBaseL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_PVP_MISSION_COMBO_CLEAR,ParamBase,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_PVP_MISSION_COMBO_CLEAR,
                      ParamBase,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                               PacketBuf&) {
    return 0;
}

// IDA_EA: 0x8250BE2
// IDA_NAME: _ZN17IPacketDispatcherI27MSG_PVP_MISSION_COMBO_CLEAR9ParamBaseL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_PVP_MISSION_COMBO_CLEAR,ParamBase,(ch_state)3>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_PVP_MISSION_COMBO_CLEAR,
                      ParamBase,
                      static_cast<ch_state>(3)>::check_state(CUser* user,
                                                              PacketBuf& packet) {
    if (user->get_state() > 2) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(1,
                           "PacketDispatcher.h",
                           kPvpMissionComboClearTemplateCheckStateSignature,
                           148,
                           "return __LINE__: code(%d) size(%d)",
                           code,
                           packet.get_len());
    return 149;
}

// IDA_EA: 0x825C33A
// IDA_NAME: _ZN17IPacketDispatcherI27MSG_PVP_MISSION_COMBO_CLEAR9ParamBaseL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_PVP_MISSION_COMBO_CLEAR,ParamBase,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825C36A
// IDA_NAME: _ZN17IPacketDispatcherI27MSG_PVP_MISSION_COMBO_CLEAR9ParamBaseL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_PVP_MISSION_COMBO_CLEAR,ParamBase,(ch_state)3>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_PVP_MISSION_COMBO_CLEAR,
                  ParamBase,
                  static_cast<ch_state>(3)>::~IPacketDispatcher() {}

// IDA_EA: 0x8250C76
// IDA_NAME: _ZN17IPacketDispatcherI27MSG_PVP_MISSION_COMBO_CLEAR9ParamBaseL8ch_state3EE4sendEP5CUserRS1_
// DEMANGLED: IPacketDispatcher<MSG_PVP_MISSION_COMBO_CLEAR,ParamBase,(ch_state)3>::send(CUser *,ParamBase&)
template <>
void IPacketDispatcher<MSG_PVP_MISSION_COMBO_CLEAR,
                       ParamBase,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x8598BE2
// IDA_NAME: _ZN17IPacketDispatcherI27MSG_PVP_MISSION_COMBO_CLEAR9ParamBaseL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_PVP_MISSION_COMBO_CLEAR,ParamBase,(ch_state)3>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_PVP_MISSION_COMBO_CLEAR,
                  ParamBase,
                  static_cast<ch_state>(3)>::IPacketDispatcher() {}

// IDA_EA: 0x85976AA
// IDA_NAME: _ZN31Dispatcher_PvpMissionComboClearC2Ev
// DEMANGLED: Dispatcher_PvpMissionComboClear::Dispatcher_PvpMissionComboClear(void)
Dispatcher_PvpMissionComboClear::Dispatcher_PvpMissionComboClear() {}

// IDA_EA: 0x8244F38
// IDA_NAME: _ZN31Dispatcher_PvpMissionComboClearD2Ev
// DEMANGLED: Dispatcher_PvpMissionComboClear::~Dispatcher_PvpMissionComboClear()
// IDA_EA: 0x8244F68
// IDA_NAME: _ZN31Dispatcher_PvpMissionComboClearD0Ev
// DEMANGLED: Dispatcher_PvpMissionComboClear::~Dispatcher_PvpMissionComboClear()
Dispatcher_PvpMissionComboClear::~Dispatcher_PvpMissionComboClear() {}

// IDA_EA: 0x81D59C6
// IDA_NAME: _ZN31Dispatcher_PvpMissionComboClear4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_PvpMissionComboClear::read(PacketBuf &,MSG_BASE &)
int Dispatcher_PvpMissionComboClear::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_PVP_MISSION_COMBO_CLEAR& request =
        static_cast<MSG_PVP_MISSION_COMBO_CLEAR&>(message);
    if (packet.get_byte(request.mission_index) != 1) {
        return LineFunc(14670, kPvpMissionComboClearReadSignature, 0, 0);
    }
    if (packet.get_byte(request.combo_value) == 1) {
        return 0;
    }
    return LineFunc(14672, kPvpMissionComboClearReadSignature, 0, 0);
}

// IDA_EA: 0x81D5A5E
// IDA_NAME: _ZN31Dispatcher_PvpMissionComboClear7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_PvpMissionComboClear::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_PvpMissionComboClear::process(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase&) {
    if (!user->getCurCharacR()) {
        return LineFunc(14680, kPvpMissionComboClearProcessSignature, 0, 0);
    }

    const MSG_PVP_MISSION_COMBO_CLEAR& request =
        static_cast<const MSG_PVP_MISSION_COMBO_CLEAR&>(message);
    if (request.mission_index <= 8u) {
        user->Update_ComboMission(static_cast<int>(request.combo_value),
                                  static_cast<short>(request.mission_index));
        return 0;
    }

    char format[256];
    std::memset(format, 0, sizeof(format));
    OS_API::snprintf(format,
                     0x100u,
                     "[@missionSystem] invalid Combo Mission index:%d",
                     static_cast<int>(request.mission_index));
    cMyTrace trace(kPvpMissionComboClearProcessSignature, 14691, 5);
    trace(format);
    if (user->isGMUser()) {
        user->SendNotiPacketMessage(
            format, static_cast<ENUM_MESSAGE_TYPE>(0));
        return 0;
    }
    return LineFunc(14698,
                    kPvpMissionComboClearProcessSignature,
                    0,
                    user->get_acc_id());
}
