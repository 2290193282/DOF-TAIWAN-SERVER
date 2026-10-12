#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

#include <type_traits>

#include "infra/log/format/trace.h"
#include "old_game/social/party/packet_dispatcher_group_business_043.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

// Declarations precede every constructor definition so the compiler cannot
// emit a primary-template member or a second vtable owner while forming these
// four exact old-server specializations.
template <>
IPacketDispatcher<MSG_REQUEST_MEMBER_ENTER,
                  stRequestMemberEnterParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_REQUEST_MEMBER_ENTER,
                  stRequestMemberEnterParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_REQUEST_MEMBER_ENTER,
                      stRequestMemberEnterParam,
                      static_cast<ch_state>(3)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_REQUEST_MEMBER_ENTER,
                      stRequestMemberEnterParam,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_REQUEST_MEMBER_ENTER,
                      stRequestMemberEnterParam,
                      static_cast<ch_state>(3)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_REQUEST_MEMBER_ENTER,
                       stRequestMemberEnterParam,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&);

template <>
IPacketDispatcher<MSG_MEMBER_ENTER_REPLY,
                  stMemberEnterReplyParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_MEMBER_ENTER_REPLY,
                  stMemberEnterReplyParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_MEMBER_ENTER_REPLY,
                      stMemberEnterReplyParam,
                      static_cast<ch_state>(3)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_MEMBER_ENTER_REPLY,
                      stMemberEnterReplyParam,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_MEMBER_ENTER_REPLY,
                      stMemberEnterReplyParam,
                      static_cast<ch_state>(3)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_MEMBER_ENTER_REPLY,
                       stMemberEnterReplyParam,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&);

template <>
IPacketDispatcher<MSG_MEMBER_SECEDE,
                  stMemberSecedeParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_MEMBER_SECEDE,
                  stMemberSecedeParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_MEMBER_SECEDE,
                      stMemberSecedeParam,
                      static_cast<ch_state>(3)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_MEMBER_SECEDE,
                      stMemberSecedeParam,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_MEMBER_SECEDE,
                      stMemberSecedeParam,
                      static_cast<ch_state>(3)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_MEMBER_SECEDE,
                       stMemberSecedeParam,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&);

template <>
IPacketDispatcher<MSG_BASE,
                  stCallMemberListParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_BASE,
                  stCallMemberListParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_BASE,
                      stCallMemberListParam,
                      static_cast<ch_state>(3)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE,
                      stCallMemberListParam,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE,
                      stCallMemberListParam,
                      static_cast<ch_state>(3)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_BASE,
                       stCallMemberListParam,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&);

// IDA_EA: 0x8598536
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_REQUEST_MEMBER_ENTER25stRequestMemberEnterParamL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_REQUEST_MEMBER_ENTER,stRequestMemberEnterParam,(ch_state)3>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_REQUEST_MEMBER_ENTER,
                  stRequestMemberEnterParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825D360
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_REQUEST_MEMBER_ENTER25stRequestMemberEnterParamL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_REQUEST_MEMBER_ENTER,stRequestMemberEnterParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825D390
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_REQUEST_MEMBER_ENTER25stRequestMemberEnterParamL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_REQUEST_MEMBER_ENTER,stRequestMemberEnterParam,(ch_state)3>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_REQUEST_MEMBER_ENTER,
                  stRequestMemberEnterParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher() {}

// IDA_EA: 0x8257420
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_REQUEST_MEMBER_ENTER25stRequestMemberEnterParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_REQUEST_MEMBER_ENTER,stRequestMemberEnterParam,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_REQUEST_MEMBER_ENTER,
                      stRequestMemberEnterParam,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }

    std::aligned_storage<sizeof(MSG_REQUEST_MEMBER_ENTER),
                         alignof(MSG_REQUEST_MEMBER_ENTER)>::type
        message_storage;
    MSG_BASE& message = *reinterpret_cast<MSG_BASE*>(&message_storage);
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }

    stRequestMemberEnterParam param;
    result = this->process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    this->send(user, param);
    if (this->dispatch_sig(user, packet) > 0) {
        return 133;
    }
    return 0;
}

// IDA_EA: 0x8257570
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_REQUEST_MEMBER_ENTER25stRequestMemberEnterParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_REQUEST_MEMBER_ENTER,stRequestMemberEnterParam,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_REQUEST_MEMBER_ENTER,
                      stRequestMemberEnterParam,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x825757A
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_REQUEST_MEMBER_ENTER25stRequestMemberEnterParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_REQUEST_MEMBER_ENTER,stRequestMemberEnterParam,(ch_state)3>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_REQUEST_MEMBER_ENTER,
                      stRequestMemberEnterParam,
                      static_cast<ch_state>(3)>::check_state(CUser* user,
                                                               PacketBuf& packet) {
    if (user->get_state() > 2) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_REQUEST_MEMBER_ENTER, PARAM_TYPE = stRequestMemberEnterParam, ch_state STATE = (ch_state)3u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825D996
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_REQUEST_MEMBER_ENTER25stRequestMemberEnterParamL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_REQUEST_MEMBER_ENTER,stRequestMemberEnterParam,(ch_state)3>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_REQUEST_MEMBER_ENTER,
                       stRequestMemberEnterParam,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x8598552
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_MEMBER_ENTER_REPLY23stMemberEnterReplyParamL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_MEMBER_ENTER_REPLY,stMemberEnterReplyParam,(ch_state)3>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_MEMBER_ENTER_REPLY,
                  stMemberEnterReplyParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825D312
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_MEMBER_ENTER_REPLY23stMemberEnterReplyParamL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_MEMBER_ENTER_REPLY,stMemberEnterReplyParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825D342
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_MEMBER_ENTER_REPLY23stMemberEnterReplyParamL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_MEMBER_ENTER_REPLY,stMemberEnterReplyParam,(ch_state)3>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_MEMBER_ENTER_REPLY,
                  stMemberEnterReplyParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher() {}

// IDA_EA: 0x825722C
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_MEMBER_ENTER_REPLY23stMemberEnterReplyParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_MEMBER_ENTER_REPLY,stMemberEnterReplyParam,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_MEMBER_ENTER_REPLY,
                      stMemberEnterReplyParam,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }

    std::aligned_storage<sizeof(MSG_MEMBER_ENTER_REPLY),
                         alignof(MSG_MEMBER_ENTER_REPLY)>::type
        message_storage;
    MSG_BASE& message = *reinterpret_cast<MSG_BASE*>(&message_storage);
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }

    stMemberEnterReplyParam param;
    result = this->process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    this->send(user, param);
    if (this->dispatch_sig(user, packet) > 0) {
        return 133;
    }
    return 0;
}

// IDA_EA: 0x825737C
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_MEMBER_ENTER_REPLY23stMemberEnterReplyParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_MEMBER_ENTER_REPLY,stMemberEnterReplyParam,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_MEMBER_ENTER_REPLY,
                      stMemberEnterReplyParam,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x8257386
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_MEMBER_ENTER_REPLY23stMemberEnterReplyParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_MEMBER_ENTER_REPLY,stMemberEnterReplyParam,(ch_state)3>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_MEMBER_ENTER_REPLY,
                      stMemberEnterReplyParam,
                      static_cast<ch_state>(3)>::check_state(CUser* user,
                                                               PacketBuf& packet) {
    if (user->get_state() > 2) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_MEMBER_ENTER_REPLY, PARAM_TYPE = stMemberEnterReplyParam, ch_state STATE = (ch_state)3u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825741A
// IDA_NAME: _ZN17IPacketDispatcherI22MSG_MEMBER_ENTER_REPLY23stMemberEnterReplyParamL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_MEMBER_ENTER_REPLY,stMemberEnterReplyParam,(ch_state)3>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_MEMBER_ENTER_REPLY,
                       stMemberEnterReplyParam,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x859856E
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_MEMBER_SECEDE19stMemberSecedeParamL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_MEMBER_SECEDE,stMemberSecedeParam,(ch_state)3>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_MEMBER_SECEDE,
                  stMemberSecedeParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825D2C4
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_MEMBER_SECEDE19stMemberSecedeParamL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_MEMBER_SECEDE,stMemberSecedeParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825D2F4
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_MEMBER_SECEDE19stMemberSecedeParamL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_MEMBER_SECEDE,stMemberSecedeParam,(ch_state)3>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_MEMBER_SECEDE,
                  stMemberSecedeParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher() {}

// IDA_EA: 0x8257038
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_MEMBER_SECEDE19stMemberSecedeParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_MEMBER_SECEDE,stMemberSecedeParam,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_MEMBER_SECEDE,
                      stMemberSecedeParam,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }

    std::aligned_storage<sizeof(MSG_MEMBER_SECEDE),
                         alignof(MSG_MEMBER_SECEDE)>::type
        message_storage;
    MSG_BASE& message = *reinterpret_cast<MSG_BASE*>(&message_storage);
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }

    stMemberSecedeParam param;
    result = this->process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    this->send(user, param);
    if (this->dispatch_sig(user, packet) > 0) {
        return 133;
    }
    return 0;
}

// IDA_EA: 0x8257188
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_MEMBER_SECEDE19stMemberSecedeParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_MEMBER_SECEDE,stMemberSecedeParam,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_MEMBER_SECEDE,
                      stMemberSecedeParam,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x8257192
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_MEMBER_SECEDE19stMemberSecedeParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_MEMBER_SECEDE,stMemberSecedeParam,(ch_state)3>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_MEMBER_SECEDE,
                      stMemberSecedeParam,
                      static_cast<ch_state>(3)>::check_state(CUser* user,
                                                               PacketBuf& packet) {
    if (user->get_state() > 2) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_MEMBER_SECEDE, PARAM_TYPE = stMemberSecedeParam, ch_state STATE = (ch_state)3u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x8257226
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_MEMBER_SECEDE19stMemberSecedeParamL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_MEMBER_SECEDE,stMemberSecedeParam,(ch_state)3>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_MEMBER_SECEDE,
                       stMemberSecedeParam,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x859858A
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE21stCallMemberListParamL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stCallMemberListParam,(ch_state)3>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_BASE,
                  stCallMemberListParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825D276
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE21stCallMemberListParamL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stCallMemberListParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825D2A6
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE21stCallMemberListParamL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stCallMemberListParam,(ch_state)3>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_BASE,
                  stCallMemberListParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher() {}

// IDA_EA: 0x8256E44
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE21stCallMemberListParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stCallMemberListParam,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE,
                      stCallMemberListParam,
                      static_cast<ch_state>(3)>::dispatch_template(CUser* user,
                                                                     PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }

    std::aligned_storage<sizeof(MSG_BASE), alignof(MSG_BASE)>::type
        message_storage;
    MSG_BASE& message = *reinterpret_cast<MSG_BASE*>(&message_storage);
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }

    stCallMemberListParam param;
    result = this->process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    this->send(user, param);
    if (this->dispatch_sig(user, packet) > 0) {
        return 133;
    }
    return 0;
}

// IDA_EA: 0x8256F94
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE21stCallMemberListParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stCallMemberListParam,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE,
                      stCallMemberListParam,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x8256F9E
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE21stCallMemberListParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stCallMemberListParam,(ch_state)3>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE,
                      stCallMemberListParam,
                      static_cast<ch_state>(3)>::check_state(CUser* user,
                                                               PacketBuf& packet) {
    if (user->get_state() > 2) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_BASE, PARAM_TYPE = stCallMemberListParam, ch_state STATE = (ch_state)3u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x8257032
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE21stCallMemberListParamL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_BASE,stCallMemberListParam,(ch_state)3>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_BASE,
                       stCallMemberListParam,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&) {}
