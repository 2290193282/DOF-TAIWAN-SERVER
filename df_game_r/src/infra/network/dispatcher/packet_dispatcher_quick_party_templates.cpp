#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

#include "infra/log/format/trace.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

namespace {

static_assert(sizeof(stQuickPartyParam) == 0x08u,
              "CMD445 parameter must retain its original 8-byte extent");

}  // namespace

// This exact state-5 specialization is A-owned.  Keep every declaration
// ahead of its body so no concrete dispatcher can instantiate a primary
// template member before the original specialization is visible.
template <>
IPacketDispatcher<MSG_BASE,
                  stQuickPartyParam,
                  static_cast<ch_state>(5)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_BASE,
                  stQuickPartyParam,
                  static_cast<ch_state>(5)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_BASE,
                      stQuickPartyParam,
                      static_cast<ch_state>(5)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE,
                      stQuickPartyParam,
                      static_cast<ch_state>(5)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE,
                      stQuickPartyParam,
                      static_cast<ch_state>(5)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_BASE,
                       stQuickPartyParam,
                       static_cast<ch_state>(5)>::send(CUser*, ParamBase&);

// IDA_EA: 0x8598E9E
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE17stQuickPartyParamL8ch_state5EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stQuickPartyParam,(ch_state)5>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_BASE,
                  stQuickPartyParam,
                  static_cast<ch_state>(5)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825BA64
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE17stQuickPartyParamL8ch_state5EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stQuickPartyParam,(ch_state)5>::~IPacketDispatcher()
// IDA_EA: 0x825BA94
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE17stQuickPartyParamL8ch_state5EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stQuickPartyParam,(ch_state)5>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_BASE,
                  stQuickPartyParam,
                  static_cast<ch_state>(5)>::~IPacketDispatcher() {}

// IDA_EA: 0x824CDF8
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE17stQuickPartyParamL8ch_state5EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stQuickPartyParam,(ch_state)5>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE,
                      stQuickPartyParam,
                      static_cast<ch_state>(5)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    MSG_BASE message;
    int result = read(packet, message);
    if (result != 0) {
        return result;
    }

    stQuickPartyParam param;
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

// IDA_EA: 0x824CF48
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE17stQuickPartyParamL8ch_state5EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stQuickPartyParam,(ch_state)5>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE,
                      stQuickPartyParam,
                      static_cast<ch_state>(5)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x824CF52
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE17stQuickPartyParamL8ch_state5EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stQuickPartyParam,(ch_state)5>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE,
                      stQuickPartyParam,
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
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_BASE, PARAM_TYPE = stQuickPartyParam, ch_state STATE = (ch_state)5u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825D62A
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE17stQuickPartyParamL8ch_state5EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_BASE,stQuickPartyParam,(ch_state)5>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_BASE,
                       stQuickPartyParam,
                       static_cast<ch_state>(5)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x8598E82
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_QUICK_PARTY17stQuickPartyParamL8ch_state5EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_QUICK_PARTY,stQuickPartyParam,(ch_state)5>::IPacketDispatcher(void)
// IDA_EA: 0x824CFE6
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_QUICK_PARTY17stQuickPartyParamL8ch_state5EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_QUICK_PARTY,stQuickPartyParam,(ch_state)5>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x824D142
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_QUICK_PARTY17stQuickPartyParamL8ch_state5EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_QUICK_PARTY,stQuickPartyParam,(ch_state)5>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x824D14C
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_QUICK_PARTY17stQuickPartyParamL8ch_state5EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_QUICK_PARTY,stQuickPartyParam,(ch_state)5>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825BAB2
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_QUICK_PARTY17stQuickPartyParamL8ch_state5EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_QUICK_PARTY,stQuickPartyParam,(ch_state)5>::~IPacketDispatcher()
// IDA_EA: 0x825BAE2
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_QUICK_PARTY17stQuickPartyParamL8ch_state5EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_QUICK_PARTY,stQuickPartyParam,(ch_state)5>::~IPacketDispatcher()
// IDA_EA: 0x825D630
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_QUICK_PARTY17stQuickPartyParamL8ch_state5EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_QUICK_PARTY,stQuickPartyParam,(ch_state)5>::send(CUser *,ParamBase &)
template class IPacketDispatcher<MSG_QUICK_PARTY,
                                 stQuickPartyParam,
                                 static_cast<ch_state>(5)>;
