#include "infra/network/dispatcher/p2p_statistics_dispatcher.h"

#include "common/manager/global_data.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "common/manager/statistic_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

namespace {

const char kReadSignature[] =
    "virtual int Dispatcher_P2P_Statistics::read(PacketBuf&, MSG_BASE&)";
const char kProcessSignature[] =
    "virtual int Dispatcher_P2P_Statistics::process(CUser*, MSG_BASE&, ParamBase&)";
const char kCheckErrorSignature[] =
    "int Dispatcher_P2P_Statistics::check_error(CUser*, MSG_BASE&)";

}  // namespace

// IDA_EA: 0x822E1D2
// IDA_NAME: _ZN18MSG_P2P_STATISTICSC2Ev
// DEMANGLED: MSG_P2P_STATISTICS::MSG_P2P_STATISTICS(void)
MSG_P2P_STATISTICS::MSG_P2P_STATISTICS()
    : MSG_BASE() {
    raw_0d = 0;
    raw_0e = 0;
    raw_10 = 0;
    raw_12 = 0;
    raw_14 = 0;
    raw_18 = 0;
    raw_1c = 0;
    raw_20 = 0;
    raw_24 = 0;
    raw_28 = 0;
    raw_2a = 0;
    raw_2c = 0;
    raw_30 = 0;
    raw_34 = 0;
    raw_38 = 0;
    raw_3c = 0;
}

// IDA_EA: 0x822C696
// IDA_NAME: _ZN21Packet_P2P_StatisticsC2Ev
// DEMANGLED: Packet_P2P_Statistics::Packet_P2P_Statistics(void)
Packet_P2P_Statistics::Packet_P2P_Statistics()
    : header(0x27E1u, 0x003Fu),
      raw_0a(0),
      raw_0e(0),
      raw_12(0),
      raw_13(0),
      raw_15(0),
      raw_17(0),
      raw_19(0),
      raw_1d(0),
      raw_21(0),
      raw_25(0),
      raw_29(0),
      raw_2b(0),
      raw_2d(0),
      raw_2f(0),
      raw_33(0),
      raw_37(0),
      raw_3b(0) {}

// The raw CMD479 class owns this one exact generic state-0 specialization.
// The common implementation is intentionally instantiated in this .cpp only
// after MSG_P2P_STATISTICS and ParamBase are complete.
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

// IDA_EA: 0x824B074
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_P2P_STATISTICS9ParamBaseL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_P2P_STATISTICS,ParamBase,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x824B1D0
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_P2P_STATISTICS9ParamBaseL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_P2P_STATISTICS,ParamBase,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x824B1DA
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_P2P_STATISTICS9ParamBaseL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_P2P_STATISTICS,ParamBase,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x824B26C
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_P2P_STATISTICS9ParamBaseL8ch_state0EE4sendEP5CUserRS1_
// DEMANGLED: IPacketDispatcher<MSG_P2P_STATISTICS,ParamBase,(ch_state)0>::send(CUser *,ParamBase&)
// IDA_EA: 0x825B620
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_P2P_STATISTICS9ParamBaseL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_P2P_STATISTICS,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825B650
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_P2P_STATISTICS9ParamBaseL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_P2P_STATISTICS,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x8599042
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_P2P_STATISTICS9ParamBaseL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_P2P_STATISTICS,ParamBase,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_P2P_STATISTICS,
                                 ParamBase,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x8597CCA
// IDA_NAME: _ZN25Dispatcher_P2P_StatisticsC2Ev
// DEMANGLED: Dispatcher_P2P_Statistics::Dispatcher_P2P_Statistics(void)
Dispatcher_P2P_Statistics::Dispatcher_P2P_Statistics() {}

// IDA_EA: 0x82440E6
// IDA_NAME: _ZN25Dispatcher_P2P_StatisticsD2Ev
// DEMANGLED: Dispatcher_P2P_Statistics::~Dispatcher_P2P_Statistics()
// IDA_EA: 0x8244116
// IDA_NAME: _ZN25Dispatcher_P2P_StatisticsD0Ev
// DEMANGLED: Dispatcher_P2P_Statistics::~Dispatcher_P2P_Statistics()
Dispatcher_P2P_Statistics::~Dispatcher_P2P_Statistics() {}

// IDA_EA: 0x81E2780
// IDA_NAME: _ZN25Dispatcher_P2P_Statistics4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_P2P_Statistics::read(PacketBuf &,MSG_BASE &)
int Dispatcher_P2P_Statistics::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_P2P_STATISTICS& input = static_cast<MSG_P2P_STATISTICS&>(message);
    if (packet.get_byte(input.raw_0d) != 1) {
        return LineFunc(22810, kReadSignature, 0, 0);
    }
    if (packet.get_byte(input.raw_0e) != 1) {
        return LineFunc(22811, kReadSignature, 0, 0);
    }
    if (packet.get_short(input.raw_10) != 1) {
        return LineFunc(22813, kReadSignature, 0, 0);
    }
    if (packet.get_short(input.raw_12) != 1) {
        return LineFunc(22814, kReadSignature, 0, 0);
    }
    if (packet.get_short(input.raw_14) != 1) {
        return LineFunc(22815, kReadSignature, 0, 0);
    }
    if (packet.get_int(input.raw_18) != 1) {
        return LineFunc(22817, kReadSignature, 0, 0);
    }
    if (packet.get_int(input.raw_1c) != 1) {
        return LineFunc(22818, kReadSignature, 0, 0);
    }
    if (packet.get_int(input.raw_20) != 1) {
        return LineFunc(22819, kReadSignature, 0, 0);
    }
    if (packet.get_int(input.raw_24) != 1) {
        return LineFunc(22820, kReadSignature, 0, 0);
    }
    if (packet.get_short(input.raw_28) != 1) {
        return LineFunc(22822, kReadSignature, 0, 0);
    }
    if (packet.get_short(input.raw_2a) != 1) {
        return LineFunc(22823, kReadSignature, 0, 0);
    }
    if (packet.get_short(input.raw_2c) != 1) {
        return LineFunc(22824, kReadSignature, 0, 0);
    }
    if (packet.get_int(input.raw_30) != 1) {
        return LineFunc(22826, kReadSignature, 0, 0);
    }
    if (packet.get_int(input.raw_34) != 1) {
        return LineFunc(22827, kReadSignature, 0, 0);
    }
    if (packet.get_int(input.raw_38) != 1) {
        return LineFunc(22828, kReadSignature, 0, 0);
    }
    if (packet.get_int(input.raw_3c) == 1) {
        return 0;
    }
    return LineFunc(22829, kReadSignature, 0, 0);
}

// IDA_EA: 0x81E2EF6
// IDA_NAME: _ZN25Dispatcher_P2P_Statistics11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_P2P_Statistics::check_error(CUser *,MSG_BASE &)
int Dispatcher_P2P_Statistics::check_error(CUser* user, MSG_BASE& message) {
    (void)user;
    (void)message;
    return LineFunc(22896, kCheckErrorSignature, 0, 0);
}

// IDA_EA: 0x81E2BDE
// IDA_NAME: _ZN25Dispatcher_P2P_Statistics7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_P2P_Statistics::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_P2P_Statistics::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    (void)param;

    const MSG_P2P_STATISTICS& input =
        static_cast<const MSG_P2P_STATISTICS&>(message);
    Packet_P2P_Statistics statistic;
    statistic.raw_0a =
        static_cast<int>(static_cast<signed char>(input.raw_0d)) -
        static_cast<int>(static_cast<signed char>(input.raw_0e));
    statistic.raw_0e = static_cast<int>(static_cast<signed char>(input.raw_0e));
    statistic.raw_12 = static_cast<char>(user->GetServerGroup());
    statistic.raw_13 = input.raw_10;
    statistic.raw_15 = input.raw_12;
    statistic.raw_17 = input.raw_14;
    statistic.raw_19 = input.raw_18;
    statistic.raw_1d = input.raw_1c;
    statistic.raw_21 = input.raw_20;
    statistic.raw_25 = input.raw_24;
    statistic.raw_29 = input.raw_28;
    statistic.raw_2b = input.raw_2a;
    statistic.raw_2d = input.raw_2c;
    statistic.raw_2f = input.raw_30;
    statistic.raw_33 = input.raw_34;
    statistic.raw_37 = input.raw_38;
    statistic.raw_3b = input.raw_3c;

    cMyTrace p2p_trace(kProcessSignature, 22867, 0);
    p2p_trace("p2p statistics");
    // Raw 0x81E2BDE copies v10[0] into char v3 before this call; its first
    // %d is therefore the signed low byte of the stored dword, not raw_0a.
    const int p2p_trace_count =
        static_cast<int>(static_cast<signed char>(statistic.raw_0a));
    cMyTrace p2p_detail_trace(kProcessSignature, 22868, 0);
    p2p_detail_trace(
        "num(%d) min(%d) max(%d), avg(%d), 100(%u), 200(%u), 300(%u), 400(%u)",
        p2p_trace_count,
        statistic.raw_13,
        statistic.raw_15,
        statistic.raw_17,
        static_cast<unsigned int>(statistic.raw_19),
        static_cast<unsigned int>(statistic.raw_1d),
        static_cast<unsigned int>(statistic.raw_21),
        static_cast<unsigned int>(statistic.raw_25));
    cMyTrace relay_trace(kProcessSignature, 22878, 0);
    relay_trace("relay statistics");
    // Raw 0x81E2BDE likewise copies v11[0] into char v4 before the relay
    // record; preserve that signed low-byte vararg promotion exactly.
    const int relay_trace_count =
        static_cast<int>(static_cast<signed char>(statistic.raw_0e));
    cMyTrace relay_detail_trace(kProcessSignature, 22879, 0);
    relay_detail_trace(
        "num(%d) min(%d) max(%d), avg(%d), 100(%u), 200(%u), 300(%u), 400(%u)",
        relay_trace_count,
        statistic.raw_29,
        statistic.raw_2b,
        statistic.raw_2d,
        static_cast<unsigned int>(statistic.raw_2f),
        static_cast<unsigned int>(statistic.raw_33),
        static_cast<unsigned int>(statistic.raw_37),
        static_cast<unsigned int>(statistic.raw_3b));

    // The raw body carries CUser's group inside the record but deliberately
    // dispatches it through the fixed statistic-manager group zero.
    GlobalData::s_statistic_proxy_mgr
        ->GetServerProxy(static_cast<ENUM_SERVER_GROUP>(0))
        ->SendPacket(reinterpret_cast<char*>(&statistic), 0x3F);
    return 0;
}
