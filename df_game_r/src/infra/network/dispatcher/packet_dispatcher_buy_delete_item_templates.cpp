#include "old_game/economy/inventory/packet_dispatcher_delete_item.h"
#include "old_game/economy/shop/packet_dispatcher_buy_item.h"

#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

namespace {

static_assert(sizeof(MSG_BUY_ITEM) == 0x19u,
              "CMD21 message must retain the old 25-byte stack extent");
static_assert(sizeof(BuyItemParam) == 0x0b7u,
              "CMD21 parameter must retain the old 183-byte stack extent");
static_assert(sizeof(MSG_DELETE_ITEM) == 0x4c3u,
              "CMD18 message must retain the old 1219-byte stack extent");
static_assert(sizeof(stDeleteItemParam) == 0x32cu,
              "CMD18 parameter must retain the old 812-byte stack extent");

const char kBuyStateSignature[] =
    "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_BUY_ITEM, PARAM_TYPE = BuyItemParam, ch_state STATE = (ch_state)3u]";
const char kDeleteStateSignature[] =
    "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_DELETE_ITEM, PARAM_TYPE = stDeleteItemParam, ch_state STATE = (ch_state)3u]";

}  // namespace

// IDA_EA: 0x8597FA2
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_BUY_ITEM12BuyItemParamL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_BUY_ITEM,BuyItemParam,(ch_state)3>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_BUY_ITEM,
                  BuyItemParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x82362B8
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_BUY_ITEM12BuyItemParamL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_BUY_ITEM,BuyItemParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x82362E8
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_BUY_ITEM12BuyItemParamL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_BUY_ITEM,BuyItemParam,(ch_state)3>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_BUY_ITEM,
                  BuyItemParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher() {}

// IDA_EA: 0x825AAE2
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_BUY_ITEM12BuyItemParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BUY_ITEM,BuyItemParam,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BUY_ITEM,
                      BuyItemParam,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }

    MSG_BUY_ITEM message;
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }

    BuyItemParam param;
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

// IDA_EA: 0x825AC4A
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_BUY_ITEM12BuyItemParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BUY_ITEM,BuyItemParam,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BUY_ITEM,
                      BuyItemParam,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x825AC54
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_BUY_ITEM12BuyItemParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BUY_ITEM,BuyItemParam,(ch_state)3>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BUY_ITEM,
                      BuyItemParam,
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
                          kBuyStateSignature,
                          148,
                          "return __LINE__: code(%d) size(%d)",
                          code,
                          packet.get_len());
    return 149;
}

// IDA_EA: 0x825AFB4
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_BUY_ITEM12BuyItemParamL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_BUY_ITEM,BuyItemParam,(ch_state)3>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_BUY_ITEM,
                       BuyItemParam,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x859804A
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_DELETE_ITEM17stDeleteItemParamL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_DELETE_ITEM,stDeleteItemParam,(ch_state)3>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_DELETE_ITEM,
                  stDeleteItemParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x823648C
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_DELETE_ITEM17stDeleteItemParamL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_DELETE_ITEM,stDeleteItemParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x82364BC
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_DELETE_ITEM17stDeleteItemParamL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_DELETE_ITEM,stDeleteItemParam,(ch_state)3>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_DELETE_ITEM,
                  stDeleteItemParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher() {}

// IDA_EA: 0x825A0C2
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_DELETE_ITEM17stDeleteItemParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_DELETE_ITEM,stDeleteItemParam,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_DELETE_ITEM,
                      stDeleteItemParam,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }

    MSG_DELETE_ITEM message;
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }

    stDeleteItemParam param;
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

// IDA_EA: 0x825A230
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_DELETE_ITEM17stDeleteItemParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_DELETE_ITEM,stDeleteItemParam,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_DELETE_ITEM,
                      stDeleteItemParam,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x825A23A
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_DELETE_ITEM17stDeleteItemParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_DELETE_ITEM,stDeleteItemParam,(ch_state)3>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_DELETE_ITEM,
                      stDeleteItemParam,
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
                          kDeleteStateSignature,
                          148,
                          "return __LINE__: code(%d) size(%d)",
                          code,
                          packet.get_len());
    return 149;
}

// IDA_EA: 0x825ADD4
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_DELETE_ITEM17stDeleteItemParamL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_DELETE_ITEM,stDeleteItemParam,(ch_state)3>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_DELETE_ITEM,
                       stDeleteItemParam,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&) {}

template class IPacketDispatcher<MSG_BUY_ITEM,
                                 BuyItemParam,
                                 static_cast<ch_state>(3)>;
template class IPacketDispatcher<MSG_DELETE_ITEM,
                                 stDeleteItemParam,
                                 static_cast<ch_state>(3)>;
