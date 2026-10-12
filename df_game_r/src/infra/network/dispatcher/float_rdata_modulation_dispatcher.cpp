#include "infra/network/dispatcher/float_rdata_modulation_dispatcher.h"

#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

// This inclusion is intentionally translation-unit local.  The explicit
// instantiation below makes this CMD453 source the sole owner of the seven
// old IPacketDispatcher specialization symbols; it does not synthesize a
// MSG_FLOAT_RDATA_MODULATION constructor.
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

namespace {

const char kReadSignature[] =
    "virtual int Dispatcher_Float_RDATA_Modulation::read(PacketBuf&, MSG_BASE&)";

}  // namespace

// The exact old state-0 template is emitted from the shared canonical
// packet_dispatcher_msg_template.tpp.  Keep these seven markers beside the
// single explicit instantiation owner; no local copied template body exists.
// IDA_EA: 0x8265C1C
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_FLOAT_RDATA_MODULATION9ParamBaseL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_FLOAT_RDATA_MODULATION,ParamBase,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8265D6C
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_FLOAT_RDATA_MODULATION9ParamBaseL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_FLOAT_RDATA_MODULATION,ParamBase,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8265D76
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_FLOAT_RDATA_MODULATION9ParamBaseL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_FLOAT_RDATA_MODULATION,ParamBase,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x8265E08
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_FLOAT_RDATA_MODULATION9ParamBaseL8ch_state0EE4sendEP5CUserRS1_
// DEMANGLED: IPacketDispatcher<MSG_FLOAT_RDATA_MODULATION,ParamBase,(ch_state)0>::send(CUser *,ParamBase&)
// IDA_EA: 0x8266DEC
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_FLOAT_RDATA_MODULATION9ParamBaseL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_FLOAT_RDATA_MODULATION,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x8266E1C
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_FLOAT_RDATA_MODULATION9ParamBaseL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_FLOAT_RDATA_MODULATION,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x8598F0E
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_FLOAT_RDATA_MODULATION9ParamBaseL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_FLOAT_RDATA_MODULATION,ParamBase,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_FLOAT_RDATA_MODULATION,
                                 ParamBase,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x8597AB6
// IDA_NAME: _ZN33Dispatcher_Float_RDATA_ModulationC2Ev
// DEMANGLED: Dispatcher_Float_RDATA_Modulation::Dispatcher_Float_RDATA_Modulation(void)
Dispatcher_Float_RDATA_Modulation::Dispatcher_Float_RDATA_Modulation() {}

// IDA_EA: 0x826558E
// IDA_NAME: _ZN33Dispatcher_Float_RDATA_ModulationD2Ev
// DEMANGLED: Dispatcher_Float_RDATA_Modulation::~Dispatcher_Float_RDATA_Modulation()
// IDA_EA: 0x82655BE
// IDA_NAME: _ZN33Dispatcher_Float_RDATA_ModulationD0Ev
// DEMANGLED: Dispatcher_Float_RDATA_Modulation::~Dispatcher_Float_RDATA_Modulation()
Dispatcher_Float_RDATA_Modulation::~Dispatcher_Float_RDATA_Modulation() {}

// IDA_EA: 0x8264A3A
// IDA_NAME: _ZN33Dispatcher_Float_RDATA_Modulation4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Float_RDATA_Modulation::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Float_RDATA_Modulation::read(PacketBuf& packet,
                                             MSG_BASE& message) {
    MSG_FLOAT_RDATA_MODULATION& input =
        static_cast<MSG_FLOAT_RDATA_MODULATION&>(message);
    if (packet.get_int(input.raw_10) != 1) {
        return LineFunc(1410, kReadSignature, 0, 0);
    }
    if (packet.get_int(input.raw_14) == 1) {
        return 0;
    }
    return LineFunc(1414, kReadSignature, 0, 0);
}

// IDA_EA: 0x8264ACC
// IDA_NAME: _ZN33Dispatcher_Float_RDATA_Modulation7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Float_RDATA_Modulation::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Float_RDATA_Modulation::process(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase& param) {
    (void)param;
    if (!check_error(user, message)) {
        const MSG_FLOAT_RDATA_MODULATION& input =
            static_cast<const MSG_FLOAT_RDATA_MODULATION&>(message);
        const unsigned int value1 = input.raw_10 ^ 0x59AB991Fu;
        const unsigned int value2 = input.raw_14 ^ 0x1F93712Bu;
        user->getHackAnalyzer()->addServerHackCnt(
            user,
            static_cast<WongWork::ENUM_HACKTYPE>(0x349u),
            1u,
            value1,
            value2);
    }
    return 0;
}

// IDA_EA: 0x8264B5C
// IDA_NAME: _ZN33Dispatcher_Float_RDATA_Modulation11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_Float_RDATA_Modulation::check_error(CUser *,MSG_BASE &)
int Dispatcher_Float_RDATA_Modulation::check_error(CUser* user,
                                                    MSG_BASE& message) {
    (void)user;
    (void)message;
    return 0;
}
