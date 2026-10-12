#include "old_game/pvp/battlefield/packet_dispatcher_sr08_pvp_unclaimed.h"
#include "infra/network/packet/PacketEnum.h"

#include <type_traits>

#include "common/manager/global_tools.h"
#include "common/manager/user_history_log_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/pvp/battlefield/pvp_assault.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"

// Declarations precede the concrete CMD443 constructor so this translation
// unit owns the exact old template specialization rather than implicitly
// instantiating the generic provider.
template <>
IPacketDispatcher<MsgMercenaryCompetition,
                  SendMercenaryCompetitionResult,
                  static_cast<ch_state>(0)>::IPacketDispatcher();
template <>
IPacketDispatcher<MsgMercenaryCompetition,
                  SendMercenaryCompetitionResult,
                  static_cast<ch_state>(0)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MsgMercenaryCompetition,
                      SendMercenaryCompetitionResult,
                      static_cast<ch_state>(0)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MsgMercenaryCompetition,
                      SendMercenaryCompetitionResult,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MsgMercenaryCompetition,
                      SendMercenaryCompetitionResult,
                      static_cast<ch_state>(0)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MsgMercenaryCompetition,
                       SendMercenaryCompetitionResult,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&);

// IDA_EA: 0x81CE4EE
// IDA_NAME: _ZN23Dispatcher_EvadeAssault4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_EvadeAssault::read(PacketBuf &,MSG_BASE &)
int Dispatcher_EvadeAssault::read(PacketBuf& packet, MSG_BASE& message) {
    // The old body is an unconditional zero return; packet and message are
    // purposefully not inspected for this no-payload command.
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81CE4F8
// IDA_NAME: _ZN23Dispatcher_EvadeAssault7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_EvadeAssault::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_EvadeAssault::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param) {
    (void)param;

    const int error = check_error(user, message);
    if (error > 0) {
        return error;
    }
    if (error < 0) {
        return -1;
    }

    if (static_cast<unsigned char>(
            pvp_assault::GetInstanceAssaultMgr()->OnEvadeAssaultPlace(user)) ==
        1u) {
        return 0;
    }
    return LineFunc(
        10168,
        "virtual int Dispatcher_EvadeAssault::process(CUser*, MSG_BASE&, ParamBase&)",
        0,
        user->get_acc_id());
}

// IDA_EA: 0x81CE582
// IDA_NAME: _ZN23Dispatcher_EvadeAssault11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_EvadeAssault::check_error(CUser *,MSG_BASE &)
int Dispatcher_EvadeAssault::check_error(CUser* user, MSG_BASE& message) {
    (void)message;
    if (!user) {
        return LineFunc(
            10178,
            "int Dispatcher_EvadeAssault::check_error(CUser*, MSG_BASE&)",
            0,
            0);
    }
    if (user->get_state() == 3) {
        return 0;
    }
    return LineFunc(
        10181,
        "int Dispatcher_EvadeAssault::check_error(CUser*, MSG_BASE&)",
        0,
        user->get_acc_id());
}

// IDA_EA: 0x8245CEE
// IDA_NAME: _ZN23Dispatcher_EvadeAssaultD2Ev
// DEMANGLED: Dispatcher_EvadeAssault::~Dispatcher_EvadeAssault()
// IDA_EA: 0x8245D1E
// IDA_NAME: _ZN23Dispatcher_EvadeAssaultD0Ev
// DEMANGLED: Dispatcher_EvadeAssault::~Dispatcher_EvadeAssault()
Dispatcher_EvadeAssault::~Dispatcher_EvadeAssault() {}

// IDA_EA: 0x8596524
// IDA_NAME: _ZN23Dispatcher_EvadeAssaultC2Ev
// DEMANGLED: Dispatcher_EvadeAssault::Dispatcher_EvadeAssault(void)
Dispatcher_EvadeAssault::Dispatcher_EvadeAssault() {}

// IDA_EA: 0x824D1E0
// IDA_NAME: _ZN30SendMercenaryCompetitionResultC2Ev
// DEMANGLED: SendMercenaryCompetitionResult::SendMercenaryCompetitionResult(void)
SendMercenaryCompetitionResult::SendMercenaryCompetitionResult() {}

// IDA_EA: 0x824D1FC
// IDA_NAME: _ZN30SendMercenaryCompetitionResultD2Ev
// IDA_EA: 0x824D22C
// IDA_NAME: _ZN30SendMercenaryCompetitionResultD0Ev
// DEMANGLED: SendMercenaryCompetitionResult::~SendMercenaryCompetitionResult()
SendMercenaryCompetitionResult::~SendMercenaryCompetitionResult() {}

// IDA_EA: 0x8598E66
// IDA_NAME: _ZN17IPacketDispatcherI23MsgMercenaryCompetition30SendMercenaryCompetitionResultL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MsgMercenaryCompetition,SendMercenaryCompetitionResult,(ch_state)0>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MsgMercenaryCompetition,
                  SendMercenaryCompetitionResult,
                  static_cast<ch_state>(0)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825BB00
// IDA_NAME: _ZN17IPacketDispatcherI23MsgMercenaryCompetition30SendMercenaryCompetitionResultL8ch_state0EED2Ev
// IDA_EA: 0x825BB30
// IDA_NAME: _ZN17IPacketDispatcherI23MsgMercenaryCompetition30SendMercenaryCompetitionResultL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MsgMercenaryCompetition,SendMercenaryCompetitionResult,(ch_state)0>::~IPacketDispatcher()
template <>
IPacketDispatcher<MsgMercenaryCompetition,
                  SendMercenaryCompetitionResult,
                  static_cast<ch_state>(0)>::~IPacketDispatcher() {}

// IDA_EA: 0x824D24A
// IDA_NAME: _ZN17IPacketDispatcherI23MsgMercenaryCompetition30SendMercenaryCompetitionResultL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MsgMercenaryCompetition,SendMercenaryCompetitionResult,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MsgMercenaryCompetition,
                      SendMercenaryCompetitionResult,
                      static_cast<ch_state>(0)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }

    // The old routine reserves the 24-byte message slot and invokes read()
    // directly; it does not run a MSG_BASE constructor on that raw stack
    // object.  Typed aligned storage preserves this observed lifetime.
    typename std::aligned_storage<sizeof(MsgMercenaryCompetition),
                                  alignof(MsgMercenaryCompetition)>::type
        message_storage;
    MSG_BASE& message = *reinterpret_cast<MSG_BASE*>(&message_storage);
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }

    SendMercenaryCompetitionResult param;
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

// IDA_EA: 0x824D39A
// IDA_NAME: _ZN17IPacketDispatcherI23MsgMercenaryCompetition30SendMercenaryCompetitionResultL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MsgMercenaryCompetition,SendMercenaryCompetitionResult,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MsgMercenaryCompetition,
                      SendMercenaryCompetitionResult,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                               PacketBuf&) {
    return 0;
}

// IDA_EA: 0x824D3A4
// IDA_NAME: _ZN17IPacketDispatcherI23MsgMercenaryCompetition30SendMercenaryCompetitionResultL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MsgMercenaryCompetition,SendMercenaryCompetitionResult,(ch_state)0>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MsgMercenaryCompetition,
                      SendMercenaryCompetitionResult,
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
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MsgMercenaryCompetition, PARAM_TYPE = SendMercenaryCompetitionResult, ch_state STATE = (ch_state)0u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825D636
// IDA_NAME: _ZN17IPacketDispatcherI23MsgMercenaryCompetition30SendMercenaryCompetitionResultL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MsgMercenaryCompetition,SendMercenaryCompetitionResult,(ch_state)0>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MsgMercenaryCompetition,
                       SendMercenaryCompetitionResult,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x81DDC24
// IDA_NAME: _ZN32Dispatcher_Mercenary_Competition4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Mercenary_Competition::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Mercenary_Competition::read(PacketBuf& packet,
                                           MSG_BASE& message) {
    MsgMercenaryCompetition& request =
        static_cast<MsgMercenaryCompetition&>(message);
    if (packet.get_int(request.charac_no) != 1) {
        return LineFunc(
            20186,
            "virtual int Dispatcher_Mercenary_Competition::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    if (packet.get_byte(request.competition_count) != 1) {
        return LineFunc(
            20188,
            "virtual int Dispatcher_Mercenary_Competition::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    if (packet.get_byte(request.competition_type) == 1) {
        return 0;
    }
    return LineFunc(
        20190,
        "virtual int Dispatcher_Mercenary_Competition::read(PacketBuf&, MSG_BASE&)",
        0,
        0);
}

// IDA_EA: 0x81DDD00
// IDA_NAME: _ZN32Dispatcher_Mercenary_Competition7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Mercenary_Competition::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Mercenary_Competition::process(CUser* user,
                                              MSG_BASE& message,
                                              ParamBase& param) {
    if (!user || user->get_state() != 3) {
        return -1;
    }

    MsgMercenaryCompetition& request =
        static_cast<MsgMercenaryCompetition&>(message);
    SendMercenaryCompetitionResult& result =
        static_cast<SendMercenaryCompetitionResult&>(param);
    if (user->GetParty() && user->getCurCharacNo() ==
                                static_cast<int>(request.charac_no)) {
        result.accepted = 0;
        return -1;
    }

    result.accepted = user->Competition_Mercenary_Charac(
        request.charac_no, request.competition_type, request.competition_count)
                          ? 1u
                          : 0u;
    result.charac_no = request.charac_no;
    result.competition_type = request.competition_type;
    result.competition_count = request.competition_count;
    CUser_history_log(user)->MercenaryCompetition(
        request.charac_no, request.competition_type, request.competition_count);
    return 0;
}

// IDA_EA: 0x81DDE20
// IDA_NAME: _ZN32Dispatcher_Mercenary_Competition4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_Mercenary_Competition::send(CUser *,ParamBase &)
void Dispatcher_Mercenary_Competition::send(CUser* user, ParamBase& param) {
    SendMercenaryCompetitionResult& result =
        static_cast<SendMercenaryCompetitionResult&>(param);
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_MERCENARY_COMPETITION);
    packet.put_byte(result.accepted != 0 ? 1 : 0);
    if (result.accepted) {
        packet.put_int(result.charac_no);
        packet.put_byte(result.competition_count);
        packet.put_byte(result.competition_type);
    } else {
        packet.put_byte(21);
    }
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x82446B0
// IDA_NAME: _ZN32Dispatcher_Mercenary_CompetitionD2Ev
// DEMANGLED: Dispatcher_Mercenary_Competition::~Dispatcher_Mercenary_Competition()
// IDA_EA: 0x82446E0
// IDA_NAME: _ZN32Dispatcher_Mercenary_CompetitionD0Ev
// DEMANGLED: Dispatcher_Mercenary_Competition::~Dispatcher_Mercenary_Competition()
Dispatcher_Mercenary_Competition::~Dispatcher_Mercenary_Competition() {}

// IDA_EA: 0x85979D6
// IDA_NAME: _ZN32Dispatcher_Mercenary_CompetitionC2Ev
// DEMANGLED: Dispatcher_Mercenary_Competition::Dispatcher_Mercenary_Competition(void)
Dispatcher_Mercenary_Competition::Dispatcher_Mercenary_Competition() {}
