#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

#include "infra/log/format/trace.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

namespace {

#if defined(__i386__)
static_assert(sizeof(stBossDieCheckParam) == 0x10u,
              "CMD120 parameter must retain its original 16-byte extent");
#endif
static_assert(sizeof(MSG_BOSS_DIE_CHECK) == 0x18u,
              "CMD120 request must retain its original 24-byte extent");

}  // namespace

// This exact state-5 specialization is A-owned.  Keep each member declared
// before its body, so C-owned Dispatcher_BossDieCheck can only consume this
// original template family rather than instantiate the primary template.
template <>
IPacketDispatcher<MSG_BOSS_DIE_CHECK,
                  stBossDieCheckParam,
                  static_cast<ch_state>(5)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_BOSS_DIE_CHECK,
                  stBossDieCheckParam,
                  static_cast<ch_state>(5)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_BOSS_DIE_CHECK,
                      stBossDieCheckParam,
                      static_cast<ch_state>(5)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_BOSS_DIE_CHECK,
                      stBossDieCheckParam,
                      static_cast<ch_state>(5)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_BOSS_DIE_CHECK,
                      stBossDieCheckParam,
                      static_cast<ch_state>(5)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_BOSS_DIE_CHECK,
                       stBossDieCheckParam,
                       static_cast<ch_state>(5)>::send(CUser*, ParamBase&);

// IDA_EA: 0x859866A
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_BOSS_DIE_CHECK19stBossDieCheckParamL8ch_state5EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_BOSS_DIE_CHECK,stBossDieCheckParam,(ch_state)5>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_BOSS_DIE_CHECK,
                  stBossDieCheckParam,
                  static_cast<ch_state>(5)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825D054
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_BOSS_DIE_CHECK19stBossDieCheckParamL8ch_state5EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_BOSS_DIE_CHECK,stBossDieCheckParam,(ch_state)5>::~IPacketDispatcher()
// IDA_EA: 0x825D084
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_BOSS_DIE_CHECK19stBossDieCheckParamL8ch_state5EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_BOSS_DIE_CHECK,stBossDieCheckParam,(ch_state)5>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_BOSS_DIE_CHECK,
                  stBossDieCheckParam,
                  static_cast<ch_state>(5)>::~IPacketDispatcher() {}

// IDA_EA: 0x8256094
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_BOSS_DIE_CHECK19stBossDieCheckParamL8ch_state5EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BOSS_DIE_CHECK,stBossDieCheckParam,(ch_state)5>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BOSS_DIE_CHECK,
                      stBossDieCheckParam,
                      static_cast<ch_state>(5)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    // MSG_BASE::MSG_BASE @ 0x812C076 is empty; this typed automatic object
    // preserves the original 0x18 stack extent used at 0x82560F6.
    MSG_BOSS_DIE_CHECK message;
    int result = read(packet, message);
    if (result != 0) {
        return result;
    }

    stBossDieCheckParam param;
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

// IDA_EA: 0x82561E4
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_BOSS_DIE_CHECK19stBossDieCheckParamL8ch_state5EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BOSS_DIE_CHECK,stBossDieCheckParam,(ch_state)5>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BOSS_DIE_CHECK,
                      stBossDieCheckParam,
                      static_cast<ch_state>(5)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x82561EE
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_BOSS_DIE_CHECK19stBossDieCheckParamL8ch_state5EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BOSS_DIE_CHECK,stBossDieCheckParam,(ch_state)5>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BOSS_DIE_CHECK,
                      stBossDieCheckParam,
                      static_cast<ch_state>(5)>::check_state(CUser* user,
                                                               PacketBuf& packet) {
    if (user->get_state() > 4) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_BOSS_DIE_CHECK, PARAM_TYPE = stBossDieCheckParam, ch_state STATE = (ch_state)5u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825D972
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_BOSS_DIE_CHECK19stBossDieCheckParamL8ch_state5EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_BOSS_DIE_CHECK,stBossDieCheckParam,(ch_state)5>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_BOSS_DIE_CHECK,
                       stBossDieCheckParam,
                       static_cast<ch_state>(5)>::send(CUser*, ParamBase&) {}
