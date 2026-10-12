#include "infra/network/dispatcher/client_spec_statistic_dispatcher.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/statistic_proxy_abi.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

// This inclusion is intentionally translation-unit local.  The following
// explicit instantiation gives this CMD174 source the sole owner of the exact
// old state-2 IPacketDispatcher vtable and its seven native symbols.
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

namespace {

const char kCheckErrorSignature[] =
    "int Dispatcher_Client_Spec_Statistic::check_error(CUser*, MSG_BASE&)";

}  // namespace

// IDA_EA: 0x822B890
// IDA_NAME: _ZN6HWSpecC2Ev
// DEMANGLED: HWSpec::HWSpec(void)
HWSpec::HWSpec() {
    // The original C2 has one byte store at +0 followed by dword stores at
    // +4/+8.  Use volatile-width stores so a compiler cannot coalesce the
    // first assignment into the three padding bytes that must stay unwritten.
    *reinterpret_cast<volatile uint8_t*>(&raw_00) = 0xffu;
    *reinterpret_cast<volatile uint32_t*>(&raw_04) = 0xffffffffu;
    *reinterpret_cast<volatile uint32_t*>(&raw_08) = 0xffffffffu;
}

// IDA_EA: 0x822B8B0
// IDA_NAME: _ZN11Client_SpecC2Ev
// DEMANGLED: Client_Spec::Client_Spec(void)
Client_Spec::Client_Spec() {}

// IDA_EA: 0x82556A0
// IDA_NAME: _ZN25MSG_CLIENT_SPEC_STATISTICC2Ev
// DEMANGLED: MSG_CLIENT_SPEC_STATISTIC::MSG_CLIENT_SPEC_STATISTIC(void)
MSG_CLIENT_SPEC_STATISTIC::MSG_CLIENT_SPEC_STATISTIC()
    : MSG_BASE(),
      client_spec() {}

// IDA_EA: 0x8231218
// IDA_NAME: _ZN26stClientSpecStatisticParamC2Ev
// DEMANGLED: stClientSpecStatisticParam::stClientSpecStatisticParam(void)
stClientSpecStatisticParam::stClientSpecStatisticParam() : ParamBase() {
    // Raw C2 writes the parameter byte alone after ParamBase has installed
    // its vptr; retaining a byte-width store also protects neighboring stack
    // storage in the packed five-byte object.
    *reinterpret_cast<volatile uint8_t*>(&raw_04) = 0;
}

// IDA_EA: 0x824773E
// IDA_NAME: _ZN26stClientSpecStatisticParamD2Ev
// DEMANGLED: stClientSpecStatisticParam::~stClientSpecStatisticParam()
// IDA_EA: 0x824776E
// IDA_NAME: _ZN26stClientSpecStatisticParamD0Ev
// DEMANGLED: stClientSpecStatisticParam::~stClientSpecStatisticParam()
stClientSpecStatisticParam::~stClientSpecStatisticParam() {}

// IDA_EA: 0x822BC18
// IDA_NAME: _ZN28Packet_Client_Spec_StatisticC2Ev
// DEMANGLED: Packet_Client_Spec_Statistic::Packet_Client_Spec_Statistic(void)
Packet_Client_Spec_Statistic::Packet_Client_Spec_Statistic()
    : header(0x09ceu, 0x0c06u),
      raw_0b(0) {}

// IDA_EA: 0x82556C2
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_CLIENT_SPEC_STATISTIC26stClientSpecStatisticParamL8ch_state2EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CLIENT_SPEC_STATISTIC,stClientSpecStatisticParam,(ch_state)2>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x825582C
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_CLIENT_SPEC_STATISTIC26stClientSpecStatisticParamL8ch_state2EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CLIENT_SPEC_STATISTIC,stClientSpecStatisticParam,(ch_state)2>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8255836
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_CLIENT_SPEC_STATISTIC26stClientSpecStatisticParamL8ch_state2EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CLIENT_SPEC_STATISTIC,stClientSpecStatisticParam,(ch_state)2>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825CECE
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_CLIENT_SPEC_STATISTIC26stClientSpecStatisticParamL8ch_state2EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_CLIENT_SPEC_STATISTIC,stClientSpecStatisticParam,(ch_state)2>::~IPacketDispatcher()
// IDA_EA: 0x825CEFE
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_CLIENT_SPEC_STATISTIC26stClientSpecStatisticParamL8ch_state2EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_CLIENT_SPEC_STATISTIC,stClientSpecStatisticParam,(ch_state)2>::~IPacketDispatcher()
// IDA_EA: 0x825D966
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_CLIENT_SPEC_STATISTIC26stClientSpecStatisticParamL8ch_state2EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_CLIENT_SPEC_STATISTIC,stClientSpecStatisticParam,(ch_state)2>::send(CUser *,ParamBase &)
// IDA_EA: 0x859872E
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_CLIENT_SPEC_STATISTIC26stClientSpecStatisticParamL8ch_state2EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_CLIENT_SPEC_STATISTIC,stClientSpecStatisticParam,(ch_state)2>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_CLIENT_SPEC_STATISTIC,
                                stClientSpecStatisticParam,
                                static_cast<ch_state>(2)>;

// IDA_EA: 0x8596594
// IDA_NAME: _ZN32Dispatcher_Client_Spec_StatisticC2Ev
// DEMANGLED: Dispatcher_Client_Spec_Statistic::Dispatcher_Client_Spec_Statistic(void)
Dispatcher_Client_Spec_Statistic::Dispatcher_Client_Spec_Statistic() {}

// IDA_EA: 0x8245CA0
// IDA_NAME: _ZN32Dispatcher_Client_Spec_StatisticD2Ev
// DEMANGLED: Dispatcher_Client_Spec_Statistic::~Dispatcher_Client_Spec_Statistic()
// IDA_EA: 0x8245CD0
// IDA_NAME: _ZN32Dispatcher_Client_Spec_StatisticD0Ev
// DEMANGLED: Dispatcher_Client_Spec_Statistic::~Dispatcher_Client_Spec_Statistic()
Dispatcher_Client_Spec_Statistic::~Dispatcher_Client_Spec_Statistic() {}

// IDA_EA: 0x81CE5FE
// IDA_NAME: _ZN32Dispatcher_Client_Spec_Statistic4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Client_Spec_Statistic::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Client_Spec_Statistic::read(PacketBuf& packet,
                                            MSG_BASE& message) {
    MSG_CLIENT_SPEC_STATISTIC& input =
        static_cast<MSG_CLIENT_SPEC_STATISTIC&>(message);

    // The original intentionally ignores every PacketBuf return value here.
    // Do not turn a short packet into a new validation/rejection path.
    packet.get_byte(input.raw_0d);
    packet.get_byte(input.client_spec.raw_00);
    packet.get_byte(input.client_spec.spec_count);
    for (int index = 0; input.client_spec.spec_count > index; ++index) {
        HWSpec& spec = input.client_spec.specs[index];
        packet.get_byte(spec.raw_00);
        packet.get_int(spec.raw_04);
        packet.get_int(spec.raw_08);
    }
    return 0;
}

// IDA_EA: 0x81CE6D4
// IDA_NAME: _ZN32Dispatcher_Client_Spec_Statistic7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Client_Spec_Statistic::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Client_Spec_Statistic::process(CUser* user,
                                               MSG_BASE& message,
                                               ParamBase& param) {
    const int error = check_error(user, message);
    if (error > 0) {
        return error;
    }
    if (error < 0) {
        return -1;
    }

    MSG_CLIENT_SPEC_STATISTIC& input =
        static_cast<MSG_CLIENT_SPEC_STATISTIC&>(message);
    stClientSpecStatisticParam& result =
        static_cast<stClientSpecStatisticParam&>(param);
    result.raw_04 = input.raw_0d;
    std::memcpy(user->getClientHWSpec(),
                &input.client_spec,
                sizeof(input.client_spec));
    return 0;
}

// IDA_EA: 0x81CE75A
// IDA_NAME: _ZN32Dispatcher_Client_Spec_Statistic11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_Client_Spec_Statistic::check_error(CUser *,MSG_BASE &)
int Dispatcher_Client_Spec_Statistic::check_error(CUser* user,
                                                   MSG_BASE& message) {
    (void)message;
    if (user) {
        return 0;
    }
    return LineFunc(10237, kCheckErrorSignature, 0, 0);
}

// IDA_EA: 0x81CE794
// IDA_NAME: _ZN32Dispatcher_Client_Spec_Statistic4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_Client_Spec_Statistic::send(CUser *,ParamBase &)
void Dispatcher_Client_Spec_Statistic::send(CUser* user, ParamBase& param) {
    if (!user) {
        return;
    }

    const stClientSpecStatisticParam& result =
        static_cast<const stClientSpecStatisticParam&>(param);
    const char* const client_spec = user->getClientHWSpec();
    if (!client_spec[0] || result.raw_04 == 1u || result.raw_04 == 0x81u) {
        Packet_Client_Spec_Statistic statistic;
        statistic.raw_0a =
            (result.raw_04 == 1u || result.raw_04 == 0x81u) ? 2u : 0u;
        statistic.raw_0b = static_cast<uint8_t>(client_spec[1]);
        std::memcpy(statistic.specs,
                    client_spec + 4,
                    12u * statistic.raw_0b);

        // Raw stack variable n_1 aliases PacketHeader::size at output +2:
        // the ctor first writes 0x0c06, then send replaces it with the exact
        // dynamic length used for both the header and SendPacket call.
        const uint16_t send_size = static_cast<uint16_t>(
            12u * static_cast<unsigned int>(statistic.raw_0b) + 18u);
        statistic.header.size = send_size;

        // Do not touch statistic.raw_0c (+12..+17): the old CMD174 sender
        // transmits those six stack bytes without initializing them.
        GlobalData::s_statistic_proxy_mgr
            ->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))
            ->SendPacket(reinterpret_cast<char*>(&statistic),
                         static_cast<int>(send_size));
    }
}
