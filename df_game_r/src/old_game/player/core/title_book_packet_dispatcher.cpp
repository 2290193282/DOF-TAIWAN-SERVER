#include "old_game/player/core/title_book_packet_dispatcher.h"

#include <type_traits>

#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/title_book_runtime.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kTitleBookOtherUserReadSignature[] =
    "virtual int Dispatcher_TitleBookOtherUser::read(PacketBuf&, MSG_BASE&)";
const char kTitleBookPutReadSignature[] =
    "virtual int Dispatcher_TitleBookPut::read(PacketBuf&, MSG_BASE&)";
const char kTitleBookGetReadSignature[] =
    "virtual int Dispatcher_TitleBookGet::read(PacketBuf&, MSG_BASE&)";
const char kTitleBookPutTemplateCheckStateSignature[] =
    "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_TITLEBOOK_PUT, PARAM_TYPE = ParamBase, ch_state STATE = (ch_state)3u]";
const char kTitleBookGetTemplateCheckStateSignature[] =
    "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_TITLEBOOK_GET, PARAM_TYPE = ParamBase, ch_state STATE = (ch_state)3u]";
const char kTitleBookOtherUserTemplateCheckStateSignature[] =
    "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_TITLEBOOK_OTHERUSER, PARAM_TYPE = ParamBase, ch_state STATE = (ch_state)3u]";

// Raw CMD412/CMD413 templates reserve their 33-byte requests without calling
// either the derived-message or MSG_BASE constructor.
typedef typename std::aligned_storage<sizeof(MSG_TITLEBOOK_PUT),
                                      alignof(MSG_TITLEBOOK_PUT)>::type
    UnconstructedTitleBookPut;
typedef typename std::aligned_storage<sizeof(MSG_TITLEBOOK_GET),
                                      alignof(MSG_TITLEBOOK_GET)>::type
    UnconstructedTitleBookGet;

// Raw dispatch_template @0x824F7BA does not call either
// MSG_TITLEBOOK_OTHERUSER or MSG_BASE C2.  This storage deliberately mirrors
// the unconstructed 15-byte local passed to read/process in the old body.
typedef typename std::aligned_storage<sizeof(MSG_TITLEBOOK_OTHERUSER),
                                      alignof(MSG_TITLEBOOK_OTHERUSER)>::type
    UnconstructedTitleBookOtherUser;

}  // namespace

// CMD412 declarations precede every call site so no generic state-3 member
// can be instantiated before its exact old-binary specialization.
template <>
IPacketDispatcher<MSG_TITLEBOOK_PUT,
                  ParamBase,
                  static_cast<ch_state>(3)>::IPacketDispatcher();

template <>
IPacketDispatcher<MSG_TITLEBOOK_PUT,
                  ParamBase,
                  static_cast<ch_state>(3)>::~IPacketDispatcher();

template <>
int IPacketDispatcher<MSG_TITLEBOOK_PUT,
                      ParamBase,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet);

template <>
int IPacketDispatcher<MSG_TITLEBOOK_PUT,
                      ParamBase,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser* user,
                                                               PacketBuf& packet);

template <>
int IPacketDispatcher<MSG_TITLEBOOK_PUT,
                      ParamBase,
                      static_cast<ch_state>(3)>::check_state(CUser* user,
                                                              PacketBuf& packet);

template <>
void IPacketDispatcher<MSG_TITLEBOOK_PUT,
                       ParamBase,
                       static_cast<ch_state>(3)>::send(CUser* user,
                                                        ParamBase& param);

// IDA_EA: 0x824FBA2
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_TITLEBOOK_PUT9ParamBaseL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_TITLEBOOK_PUT,ParamBase,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_TITLEBOOK_PUT,
                      ParamBase,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    UnconstructedTitleBookPut message_storage;
    MSG_TITLEBOOK_PUT& message =
        *reinterpret_cast<MSG_TITLEBOOK_PUT*>(&message_storage);
    const int read_result = read(packet, static_cast<MSG_BASE&>(message));
    if (read_result != 0) {
        return read_result;
    }

    ParamBase param;
    const int process_result =
        process(user, static_cast<MSG_BASE&>(message), param);
    if (process_result > 0) {
        return process_result;
    }
    if (process_result < 0) {
        return 0;
    }

    send(user, param);
    return dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x824FCF2
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_TITLEBOOK_PUT9ParamBaseL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_TITLEBOOK_PUT,ParamBase,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_TITLEBOOK_PUT,
                      ParamBase,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                               PacketBuf&) {
    return 0;
}

// IDA_EA: 0x824FCFC
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_TITLEBOOK_PUT9ParamBaseL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_TITLEBOOK_PUT,ParamBase,(ch_state)3>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_TITLEBOOK_PUT,
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
                           kTitleBookPutTemplateCheckStateSignature,
                           148,
                           "return __LINE__: code(%d) size(%d)",
                           code,
                           packet.get_len());
    return 149;
}

// IDA_EA: 0x825C0CA
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_TITLEBOOK_PUT9ParamBaseL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_TITLEBOOK_PUT,ParamBase,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825C0FA
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_TITLEBOOK_PUT9ParamBaseL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_TITLEBOOK_PUT,ParamBase,(ch_state)3>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_TITLEBOOK_PUT,
                  ParamBase,
                  static_cast<ch_state>(3)>::~IPacketDispatcher() {}

// IDA_EA: 0x824FD90
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_TITLEBOOK_PUT9ParamBaseL8ch_state3EE4sendEP5CUserRS1_
// DEMANGLED: IPacketDispatcher<MSG_TITLEBOOK_PUT,ParamBase,(ch_state)3>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_TITLEBOOK_PUT,
                       ParamBase,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x8598CC2
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_TITLEBOOK_PUT9ParamBaseL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_TITLEBOOK_PUT,ParamBase,(ch_state)3>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_TITLEBOOK_PUT,
                  ParamBase,
                  static_cast<ch_state>(3)>::IPacketDispatcher() {}

// IDA_EA: 0x8597816
// IDA_NAME: _ZN23Dispatcher_TitleBookPutC2Ev
// DEMANGLED: Dispatcher_TitleBookPut::Dispatcher_TitleBookPut(void)
Dispatcher_TitleBookPut::Dispatcher_TitleBookPut() {}

// IDA_EA: 0x8244CC8
// IDA_NAME: _ZN23Dispatcher_TitleBookPutD2Ev
// DEMANGLED: Dispatcher_TitleBookPut::~Dispatcher_TitleBookPut()
// IDA_EA: 0x8244CF8
// IDA_NAME: _ZN23Dispatcher_TitleBookPutD0Ev
// DEMANGLED: Dispatcher_TitleBookPut::~Dispatcher_TitleBookPut()
Dispatcher_TitleBookPut::~Dispatcher_TitleBookPut() {}

// IDA_EA: 0x81D75E8
// IDA_NAME: _ZN23Dispatcher_TitleBookPut4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_TitleBookPut::read(PacketBuf &,MSG_BASE &)
int Dispatcher_TitleBookPut::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_TITLEBOOK_PUT& request = static_cast<MSG_TITLEBOOK_PUT&>(message);
    if (packet.get_int(request.item_space) != 1) {
        return LineFunc(16852, kTitleBookPutReadSignature, 0, 0);
    }
    if (packet.get_int(request.source_slot) != 1) {
        return LineFunc(16854, kTitleBookPutReadSignature, 0, 0);
    }
    if (packet.get_int(request.item_id) != 1) {
        return LineFunc(16856, kTitleBookPutReadSignature, 0, 0);
    }
    if (packet.get_int(request.category) != 1) {
        return LineFunc(16858, kTitleBookPutReadSignature, 0, 0);
    }
    if (packet.get_int(request.title_slot) == 1) {
        return 0;
    }
    return LineFunc(16860, kTitleBookPutReadSignature, 0, 0);
}

// IDA_EA: 0x81D774E
// IDA_NAME: _ZN23Dispatcher_TitleBookPut7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_TitleBookPut::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_TitleBookPut::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase&) {
    if (user->get_state() <= 2) {
        return 0;
    }

    const MSG_TITLEBOOK_PUT& request =
        static_cast<const MSG_TITLEBOOK_PUT&>(message);
    const int result = CTitleBook::putTitle(
        user,
        static_cast<ENUM_ITEMSPACE>(request.item_space),
        request.source_slot,
        request.item_id,
        static_cast<ENUM_TITLE_BOOK_CATEGORY>(request.category),
        request.title_slot);

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_TITLE_BOOK_PUT);
    if (result) {
        packet.put_byte(0);
        packet.put_byte(result);
        packet.put_int(request.item_space);
        packet.put_int(request.category);
    } else {
        packet.put_byte(1);
        packet.put_int(request.item_space);
        packet.put_int(request.source_slot);
        packet.put_int(request.category);
        packet.put_int(request.title_slot);
    }
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// CMD413 declarations precede every call site so no generic state-3 member
// can be instantiated before its exact old-binary specialization.
template <>
IPacketDispatcher<MSG_TITLEBOOK_GET,
                  ParamBase,
                  static_cast<ch_state>(3)>::IPacketDispatcher();

template <>
IPacketDispatcher<MSG_TITLEBOOK_GET,
                  ParamBase,
                  static_cast<ch_state>(3)>::~IPacketDispatcher();

template <>
int IPacketDispatcher<MSG_TITLEBOOK_GET,
                      ParamBase,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet);

template <>
int IPacketDispatcher<MSG_TITLEBOOK_GET,
                      ParamBase,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser* user,
                                                               PacketBuf& packet);

template <>
int IPacketDispatcher<MSG_TITLEBOOK_GET,
                      ParamBase,
                      static_cast<ch_state>(3)>::check_state(CUser* user,
                                                              PacketBuf& packet);

template <>
void IPacketDispatcher<MSG_TITLEBOOK_GET,
                       ParamBase,
                       static_cast<ch_state>(3)>::send(CUser* user,
                                                        ParamBase& param);

// IDA_EA: 0x824F9AE
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_TITLEBOOK_GET9ParamBaseL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_TITLEBOOK_GET,ParamBase,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_TITLEBOOK_GET,
                      ParamBase,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    UnconstructedTitleBookGet message_storage;
    MSG_TITLEBOOK_GET& message =
        *reinterpret_cast<MSG_TITLEBOOK_GET*>(&message_storage);
    const int read_result = read(packet, static_cast<MSG_BASE&>(message));
    if (read_result != 0) {
        return read_result;
    }

    ParamBase param;
    const int process_result =
        process(user, static_cast<MSG_BASE&>(message), param);
    if (process_result > 0) {
        return process_result;
    }
    if (process_result < 0) {
        return 0;
    }

    send(user, param);
    return dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x824FAFE
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_TITLEBOOK_GET9ParamBaseL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_TITLEBOOK_GET,ParamBase,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_TITLEBOOK_GET,
                      ParamBase,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                               PacketBuf&) {
    return 0;
}

// IDA_EA: 0x824FB08
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_TITLEBOOK_GET9ParamBaseL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_TITLEBOOK_GET,ParamBase,(ch_state)3>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_TITLEBOOK_GET,
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
                           kTitleBookGetTemplateCheckStateSignature,
                           148,
                           "return __LINE__: code(%d) size(%d)",
                           code,
                           packet.get_len());
    return 149;
}

// IDA_EA: 0x825C07C
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_TITLEBOOK_GET9ParamBaseL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_TITLEBOOK_GET,ParamBase,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825C0AC
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_TITLEBOOK_GET9ParamBaseL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_TITLEBOOK_GET,ParamBase,(ch_state)3>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_TITLEBOOK_GET,
                  ParamBase,
                  static_cast<ch_state>(3)>::~IPacketDispatcher() {}

// IDA_EA: 0x824FB9C
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_TITLEBOOK_GET9ParamBaseL8ch_state3EE4sendEP5CUserRS1_
// DEMANGLED: IPacketDispatcher<MSG_TITLEBOOK_GET,ParamBase,(ch_state)3>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_TITLEBOOK_GET,
                       ParamBase,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x8598CDE
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_TITLEBOOK_GET9ParamBaseL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_TITLEBOOK_GET,ParamBase,(ch_state)3>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_TITLEBOOK_GET,
                  ParamBase,
                  static_cast<ch_state>(3)>::IPacketDispatcher() {}

// IDA_EA: 0x8597832
// IDA_NAME: _ZN23Dispatcher_TitleBookGetC2Ev
// DEMANGLED: Dispatcher_TitleBookGet::Dispatcher_TitleBookGet(void)
Dispatcher_TitleBookGet::Dispatcher_TitleBookGet() {}

// IDA_EA: 0x8244C7A
// IDA_NAME: _ZN23Dispatcher_TitleBookGetD2Ev
// DEMANGLED: Dispatcher_TitleBookGet::~Dispatcher_TitleBookGet()
// IDA_EA: 0x8244CAA
// IDA_NAME: _ZN23Dispatcher_TitleBookGetD0Ev
// DEMANGLED: Dispatcher_TitleBookGet::~Dispatcher_TitleBookGet()
Dispatcher_TitleBookGet::~Dispatcher_TitleBookGet() {}

// IDA_EA: 0x81D7974
// IDA_NAME: _ZN23Dispatcher_TitleBookGet4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_TitleBookGet::read(PacketBuf &,MSG_BASE &)
int Dispatcher_TitleBookGet::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_TITLEBOOK_GET& request = static_cast<MSG_TITLEBOOK_GET&>(message);
    if (packet.get_int(request.item_space) != 1) {
        return LineFunc(16908, kTitleBookGetReadSignature, 0, 0);
    }
    if (packet.get_int(request.output_slot) != 1) {
        return LineFunc(16910, kTitleBookGetReadSignature, 0, 0);
    }
    if (packet.get_int(request.item_id) != 1) {
        return LineFunc(16912, kTitleBookGetReadSignature, 0, 0);
    }
    if (packet.get_int(request.category) != 1) {
        return LineFunc(16914, kTitleBookGetReadSignature, 0, 0);
    }
    if (packet.get_int(request.title_slot) == 1) {
        return 0;
    }
    return LineFunc(16916, kTitleBookGetReadSignature, 0, 0);
}

// IDA_EA: 0x81D7ADA
// IDA_NAME: _ZN23Dispatcher_TitleBookGet7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_TitleBookGet::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_TitleBookGet::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase&) {
    if (user->get_state() <= 2) {
        return 0;
    }

    MSG_TITLEBOOK_GET& request = static_cast<MSG_TITLEBOOK_GET&>(message);
    const int result = CTitleBook::getTitle(
        user,
        static_cast<ENUM_TITLE_BOOK_CATEGORY>(request.category),
        request.title_slot,
        request.item_id,
        static_cast<ENUM_ITEMSPACE>(request.item_space),
        request.output_slot);

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_TITLE_BOOK_GET);
    if (result) {
        packet.put_byte(0);
        packet.put_byte(result);
        packet.put_int(request.item_space);
        packet.put_int(request.category);
    } else {
        packet.put_byte(1);
        packet.put_int(request.item_space);
        packet.put_int(request.output_slot);
        packet.put_int(request.category);
        packet.put_int(request.title_slot);
    }
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// Every declaration precedes dispatch_template so G++4.9 cannot instantiate a
// generic state-3 member before its explicit old-binary specialization.
template <>
IPacketDispatcher<MSG_TITLEBOOK_OTHERUSER,
                  ParamBase,
                  static_cast<ch_state>(3)>::IPacketDispatcher();

template <>
IPacketDispatcher<MSG_TITLEBOOK_OTHERUSER,
                  ParamBase,
                  static_cast<ch_state>(3)>::~IPacketDispatcher();

template <>
int IPacketDispatcher<MSG_TITLEBOOK_OTHERUSER,
                      ParamBase,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet);

template <>
int IPacketDispatcher<MSG_TITLEBOOK_OTHERUSER,
                      ParamBase,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser* user,
                                                               PacketBuf& packet);

template <>
int IPacketDispatcher<MSG_TITLEBOOK_OTHERUSER,
                      ParamBase,
                      static_cast<ch_state>(3)>::check_state(CUser* user,
                                                              PacketBuf& packet);

template <>
void IPacketDispatcher<MSG_TITLEBOOK_OTHERUSER,
                       ParamBase,
                       static_cast<ch_state>(3)>::send(CUser* user,
                                                        ParamBase& param);

// IDA_EA: 0x824F7BA
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_TITLEBOOK_OTHERUSER9ParamBaseL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_TITLEBOOK_OTHERUSER,ParamBase,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_TITLEBOOK_OTHERUSER,
                      ParamBase,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    int result = 0;
    UnconstructedTitleBookOtherUser message_storage;
    MSG_TITLEBOOK_OTHERUSER& message =
        *reinterpret_cast<MSG_TITLEBOOK_OTHERUSER*>(&message_storage);
    result = read(packet, static_cast<MSG_BASE&>(message));
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

// IDA_EA: 0x824F90A
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_TITLEBOOK_OTHERUSER9ParamBaseL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_TITLEBOOK_OTHERUSER,ParamBase,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_TITLEBOOK_OTHERUSER,
                      ParamBase,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                               PacketBuf&) {
    return 0;
}

// IDA_EA: 0x824F914
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_TITLEBOOK_OTHERUSER9ParamBaseL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_TITLEBOOK_OTHERUSER,ParamBase,(ch_state)3>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_TITLEBOOK_OTHERUSER,
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
                           kTitleBookOtherUserTemplateCheckStateSignature,
                           148,
                           "return __LINE__: code(%d) size(%d)",
                           code,
                           packet.get_len());
    return 149;
}

// IDA_EA: 0x825C02E
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_TITLEBOOK_OTHERUSER9ParamBaseL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_TITLEBOOK_OTHERUSER,ParamBase,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825C05E
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_TITLEBOOK_OTHERUSER9ParamBaseL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_TITLEBOOK_OTHERUSER,ParamBase,(ch_state)3>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_TITLEBOOK_OTHERUSER,
                  ParamBase,
                  static_cast<ch_state>(3)>::~IPacketDispatcher() {}

// IDA_EA: 0x824F9A8
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_TITLEBOOK_OTHERUSER9ParamBaseL8ch_state3EE4sendEP5CUserRS1_
// DEMANGLED: IPacketDispatcher<MSG_TITLEBOOK_OTHERUSER,ParamBase,(ch_state)3>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_TITLEBOOK_OTHERUSER,
                       ParamBase,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x8598CFA
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_TITLEBOOK_OTHERUSER9ParamBaseL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_TITLEBOOK_OTHERUSER,ParamBase,(ch_state)3>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_TITLEBOOK_OTHERUSER,
                  ParamBase,
                  static_cast<ch_state>(3)>::IPacketDispatcher() {}

// IDA_EA: 0x859784E
// IDA_NAME: _ZN29Dispatcher_TitleBookOtherUserC2Ev
// DEMANGLED: Dispatcher_TitleBookOtherUser::Dispatcher_TitleBookOtherUser(void)
Dispatcher_TitleBookOtherUser::Dispatcher_TitleBookOtherUser() {}

// IDA_EA: 0x8244C2C
// IDA_NAME: _ZN29Dispatcher_TitleBookOtherUserD2Ev
// DEMANGLED: Dispatcher_TitleBookOtherUser::~Dispatcher_TitleBookOtherUser()
// IDA_EA: 0x8244C5C
// IDA_NAME: _ZN29Dispatcher_TitleBookOtherUserD0Ev
// DEMANGLED: Dispatcher_TitleBookOtherUser::~Dispatcher_TitleBookOtherUser()
Dispatcher_TitleBookOtherUser::~Dispatcher_TitleBookOtherUser() {}

// IDA_EA: 0x81D7D00
// IDA_NAME: _ZN29Dispatcher_TitleBookOtherUser4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_TitleBookOtherUser::read(PacketBuf &,MSG_BASE &)
int Dispatcher_TitleBookOtherUser::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_TITLEBOOK_OTHERUSER& request =
        static_cast<MSG_TITLEBOOK_OTHERUSER&>(message);
    if (packet.get_short(request.target_uid) == 1) {
        return 0;
    }
    return LineFunc(16964, kTitleBookOtherUserReadSignature, 0, 0);
}

// IDA_EA: 0x81D7D56
// IDA_NAME: _ZN29Dispatcher_TitleBookOtherUser7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_TitleBookOtherUser::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_TitleBookOtherUser::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase&) {
    if (user->get_state() <= 2) {
        return 0;
    }

    const MSG_TITLEBOOK_OTHERUSER& request =
        static_cast<const MSG_TITLEBOOK_OTHERUSER&>(message);
    CUser* const target = G_GameWorld()->find_from_world(request.target_uid);
    if (!target) {
        return 0;
    }
    if (target->get_state() > 2) {
        CTitleBook* const title_book = static_cast<CTitleBook*>(
            target->GetCharacExpandData(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(14)));
        title_book->sendListOtherUser(
            user, static_cast<ENUM_TITLE_BOOK_INFO_TYPE>(1));
    }
    return 0;
}
