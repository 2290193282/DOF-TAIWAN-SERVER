#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

#include "infra/log/format/trace.h"
#include "old_data/PVF/equipment/item_upgrade.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

// This is the sole vtable/function owner for the original item-separate
// dispatcher template family.  Declare every member before a definition so
// no translation unit can emit a primary-template COMDAT provider.
template <>
IPacketDispatcher<MSG_ITEM_UPGRADE, STUdateItemParam,
                  static_cast<ch_state>(0)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_ITEM_UPGRADE, STUdateItemParam,
                  static_cast<ch_state>(0)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_ITEM_UPGRADE, STUdateItemParam,
                      static_cast<ch_state>(0)>::dispatch_template(
    CUser*, PacketBuf&);
template <>
int IPacketDispatcher<MSG_ITEM_UPGRADE, STUdateItemParam,
                      static_cast<ch_state>(0)>::dispatch_sig(
    CUser*, PacketBuf&);
template <>
int IPacketDispatcher<MSG_ITEM_UPGRADE, STUdateItemParam,
                      static_cast<ch_state>(0)>::check_state(
    CUser*, PacketBuf&);
template <>
void IPacketDispatcher<MSG_ITEM_UPGRADE, STUdateItemParam,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&);

// IDA_EA: 0x85985A6
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_ITEM_UPGRADE16STUdateItemParamL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_ITEM_UPGRADE,STUdateItemParam,(ch_state)0>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_ITEM_UPGRADE, STUdateItemParam,
                  static_cast<ch_state>(0)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825D228
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_ITEM_UPGRADE16STUdateItemParamL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_ITEM_UPGRADE,STUdateItemParam,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825D258
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_ITEM_UPGRADE16STUdateItemParamL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_ITEM_UPGRADE,STUdateItemParam,(ch_state)0>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_ITEM_UPGRADE, STUdateItemParam,
                  static_cast<ch_state>(0)>::~IPacketDispatcher() {}

// IDA_EA: 0x8256C38
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_ITEM_UPGRADE16STUdateItemParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_ITEM_UPGRADE,STUdateItemParam,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_ITEM_UPGRADE, STUdateItemParam,
                      static_cast<ch_state>(0)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }

    MSG_ITEM_UPGRADE message;
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }

    STUdateItemParam param;
    result = this->process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    this->send(user, param);
    return this->dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x8256DA2
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_ITEM_UPGRADE16STUdateItemParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_ITEM_UPGRADE,STUdateItemParam,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_ITEM_UPGRADE, STUdateItemParam,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x8256DAC
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_ITEM_UPGRADE16STUdateItemParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_ITEM_UPGRADE,STUdateItemParam,(ch_state)0>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_ITEM_UPGRADE, STUdateItemParam,
                      static_cast<ch_state>(0)>::check_state(
    CUser* user,
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
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_ITEM_UPGRADE, PARAM_TYPE = STUdateItemParam, ch_state STATE = (ch_state)0u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x8256E3E
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_ITEM_UPGRADE16STUdateItemParamL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_ITEM_UPGRADE,STUdateItemParam,(ch_state)0>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_ITEM_UPGRADE, STUdateItemParam,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&) {}
