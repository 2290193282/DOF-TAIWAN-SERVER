#include "infra/security/policy/motion_hack_dispatcher.h"

#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

namespace {

const char kReadSignature[] =
    "virtual int Dispatcher_MotionHack::read(PacketBuf&, MSG_BASE&)";

}  // namespace

// The raw CMD358 template uses a 0x16-byte unconstructed stack message.  It
// has no MSG_MOTION_HACK C2 identity.  MSG_BASE::C2 @0x812C076 is proven an
// empty body, so the implicit base construction performed by this C++ type
// writes no storage and does not add a named derived-message provider.
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

// IDA_EA: 0x82663DE
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_MOTION_HACK9ParamBaseL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_MOTION_HACK,ParamBase,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x826652E
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_MOTION_HACK9ParamBaseL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_MOTION_HACK,ParamBase,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8266538
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_MOTION_HACK9ParamBaseL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_MOTION_HACK,ParamBase,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x82665CA
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_MOTION_HACK9ParamBaseL8ch_state0EE4sendEP5CUserRS1_
// DEMANGLED: IPacketDispatcher<MSG_MOTION_HACK,ParamBase,(ch_state)0>::send(CUser *,ParamBase&)
// IDA_EA: 0x8266F24
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_MOTION_HACK9ParamBaseL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_MOTION_HACK,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x8266F54
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_MOTION_HACK9ParamBaseL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_MOTION_HACK,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x8598B02
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_MOTION_HACK9ParamBaseL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_MOTION_HACK,ParamBase,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_MOTION_HACK,
                                 ParamBase,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x859753E
// IDA_NAME: _ZN21Dispatcher_MotionHackC2Ev
// DEMANGLED: Dispatcher_MotionHack::Dispatcher_MotionHack(void)
Dispatcher_MotionHack::Dispatcher_MotionHack() {}

// IDA_EA: 0x82656C6
// IDA_NAME: _ZN21Dispatcher_MotionHackD2Ev
// DEMANGLED: Dispatcher_MotionHack::~Dispatcher_MotionHack()
// IDA_EA: 0x82656F6
// IDA_NAME: _ZN21Dispatcher_MotionHackD0Ev
// DEMANGLED: Dispatcher_MotionHack::~Dispatcher_MotionHack()
Dispatcher_MotionHack::~Dispatcher_MotionHack() {}

// IDA_EA: 0x8264574
// IDA_NAME: _ZN21Dispatcher_MotionHack4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_MotionHack::read(PacketBuf &,MSG_BASE &)
int Dispatcher_MotionHack::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_MOTION_HACK& input = static_cast<MSG_MOTION_HACK&>(message);
    if (packet.get_byte(input.enabled) != 1) {
        return LineFunc(1044, kReadSignature, 0, 0);
    }
    if (packet.get_int(input.value1) != 1) {
        return LineFunc(1046, kReadSignature, 0, 0);
    }
    if (packet.get_int(input.value2) == 1) {
        return 0;
    }
    return LineFunc(1048, kReadSignature, 0, 0);
}

// IDA_EA: 0x826464A
// IDA_NAME: _ZN21Dispatcher_MotionHack7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_MotionHack::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_MotionHack::process(CUser* user,
                                   MSG_BASE& message,
                                   ParamBase& param) {
    (void)param;

    const MSG_MOTION_HACK& input =
        static_cast<const MSG_MOTION_HACK&>(message);
    if (input.enabled) {
        user->getHackAnalyzer()->addServerHackCnt(
            user,
            static_cast<WongWork::ENUM_HACKTYPE>(0x83u),
            1u,
            input.value1,
            input.value2);
    }
    return 0;
}
