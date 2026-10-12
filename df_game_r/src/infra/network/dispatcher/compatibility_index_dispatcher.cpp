#include "infra/network/dispatcher/compatibility_index_dispatcher.h"

#include "common/manager/global_data.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "common/manager/statistic_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

namespace {

const char kReadSignature[] =
    "virtual int Dispatcher_CompatibilityIndex::read(PacketBuf&, MSG_BASE&)";
const char kProcessSignature[] =
    "virtual int Dispatcher_CompatibilityIndex::process(CUser*, MSG_BASE&, ParamBase&)";
}  // namespace

// IDA_EA: 0x822E174
// IDA_NAME: _ZN23MSG_COMPATIBILITY_INDEXC2Ev
// DEMANGLED: MSG_COMPATIBILITY_INDEX::MSG_COMPATIBILITY_INDEX(void)
MSG_COMPATIBILITY_INDEX::MSG_COMPATIBILITY_INDEX()
    : MSG_BASE(),
      raw_0d(0),
      raw_0e(0),
      raw_10(0),
      raw_14(0),
      raw_16(0),
      raw_18(0),
      raw_1a(0),
      raw_1c(0),
      raw_1d(0) {}

// IDA_EA: 0x822C61E
// IDA_NAME: _ZN31Packet_Stat_Compatibility_IndexC2Ev
// DEMANGLED: Packet_Stat_Compatibility_Index::Packet_Stat_Compatibility_Index(void)
Packet_Stat_Compatibility_Index::Packet_Stat_Compatibility_Index()
    : header(0x27E0u, 0x001Eu),
      raw_0a(0),
      raw_0e(0),
      raw_0f(0),
      raw_10(0),
      raw_14(0),
      raw_16(0),
      raw_18(0),
      raw_1a(0),
      raw_1c(0),
      raw_1d(0) {}

// The exact old state-0 template is emitted from the shared canonical
// packet_dispatcher_msg_template.tpp.  Keep these seven markers beside the
// single explicit instantiation owner; no local copied template body exists.
// IDA_EA: 0x824B272
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_COMPATIBILITY_INDEX9ParamBaseL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_COMPATIBILITY_INDEX,ParamBase,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x824B3CE
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_COMPATIBILITY_INDEX9ParamBaseL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_COMPATIBILITY_INDEX,ParamBase,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x824B3D8
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_COMPATIBILITY_INDEX9ParamBaseL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_COMPATIBILITY_INDEX,ParamBase,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x824B46A
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_COMPATIBILITY_INDEX9ParamBaseL8ch_state0EE4sendEP5CUserRS1_
// DEMANGLED: IPacketDispatcher<MSG_COMPATIBILITY_INDEX,ParamBase,(ch_state)0>::send(CUser *,ParamBase&)
// IDA_EA: 0x825B66E
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_COMPATIBILITY_INDEX9ParamBaseL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_COMPATIBILITY_INDEX,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825B69E
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_COMPATIBILITY_INDEX9ParamBaseL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_COMPATIBILITY_INDEX,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x8599026
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_COMPATIBILITY_INDEX9ParamBaseL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_COMPATIBILITY_INDEX,ParamBase,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_COMPATIBILITY_INDEX,
                                 ParamBase,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x8597CAE
// IDA_NAME: _ZN29Dispatcher_CompatibilityIndexC2Ev
// DEMANGLED: Dispatcher_CompatibilityIndex::Dispatcher_CompatibilityIndex(void)
Dispatcher_CompatibilityIndex::Dispatcher_CompatibilityIndex() {}

// IDA_EA: 0x8244134
// IDA_NAME: _ZN29Dispatcher_CompatibilityIndexD2Ev
// DEMANGLED: Dispatcher_CompatibilityIndex::~Dispatcher_CompatibilityIndex()
// IDA_EA: 0x8244164
// IDA_NAME: _ZN29Dispatcher_CompatibilityIndexD0Ev
// DEMANGLED: Dispatcher_CompatibilityIndex::~Dispatcher_CompatibilityIndex()
Dispatcher_CompatibilityIndex::~Dispatcher_CompatibilityIndex() {}

// IDA_EA: 0x81E23E0
// IDA_NAME: _ZN29Dispatcher_CompatibilityIndex11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CompatibilityIndex::check_error(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CompatibilityIndex::check_error(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x81E23EA
// IDA_NAME: _ZN29Dispatcher_CompatibilityIndex4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CompatibilityIndex::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CompatibilityIndex::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_COMPATIBILITY_INDEX& input =
        static_cast<MSG_COMPATIBILITY_INDEX&>(message);
    if (packet.get_byte(input.raw_0d) != 1) {
        return LineFunc(22751, kReadSignature, 0, 0);
    }
    if (packet.get_byte(input.raw_0e) != 1) {
        return LineFunc(22752, kReadSignature, 0, 0);
    }
    if (packet.get_int(input.raw_10) != 1) {
        return LineFunc(22753, kReadSignature, 0, 0);
    }
    if (packet.get_short(input.raw_14) != 1) {
        return LineFunc(22754, kReadSignature, 0, 0);
    }
    if (packet.get_short(input.raw_16) != 1) {
        return LineFunc(22755, kReadSignature, 0, 0);
    }
    if (packet.get_short(input.raw_18) != 1) {
        return LineFunc(22756, kReadSignature, 0, 0);
    }
    if (packet.get_short(input.raw_1a) != 1) {
        return LineFunc(22757, kReadSignature, 0, 0);
    }
    if (packet.get_byte(input.raw_1c) != 1) {
        return LineFunc(22758, kReadSignature, 0, 0);
    }
    if (packet.get_byte(input.raw_1d) == 1) {
        return 0;
    }
    return LineFunc(22759, kReadSignature, 0, 0);
}

// IDA_EA: 0x81E2664
// IDA_NAME: _ZN29Dispatcher_CompatibilityIndex7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CompatibilityIndex::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CompatibilityIndex::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    const int error = check_error(user, message, param);
    if (error > 0) {
        return 0;
    }
    if (error < 0) {
        return LineFunc(22776, kProcessSignature, error, 0);
    }

    const MSG_COMPATIBILITY_INDEX& input =
        static_cast<const MSG_COMPATIBILITY_INDEX&>(message);
    Packet_Stat_Compatibility_Index statistic;
    statistic.raw_0a = user->get_acc_id();
    statistic.raw_10 = static_cast<int32_t>(input.raw_10);
    statistic.raw_0f = input.raw_0e;
    statistic.raw_0e = input.raw_0d;
    statistic.raw_14 = input.raw_14;
    statistic.raw_16 = input.raw_16;
    statistic.raw_1a = input.raw_1a;
    statistic.raw_18 = input.raw_18;
    statistic.raw_1c = input.raw_1c;
    statistic.raw_1d = input.raw_1d;

    // The raw body passes literal group zero, not CUser::GetServerGroup().
    GlobalData::s_statistic_proxy_mgr
        ->GetServerProxy(static_cast<ENUM_SERVER_GROUP>(0))
        ->SendPacket(reinterpret_cast<char*>(&statistic), 0x1E);
    return 0;
}
