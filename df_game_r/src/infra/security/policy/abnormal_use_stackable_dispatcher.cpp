#include "infra/security/policy/abnormal_use_stackable_dispatcher.h"

#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

namespace {

const char kReadSignature[] =
    "virtual int Dispatcher_AbnormalUseStackable::read(PacketBuf&, MSG_BASE&)";

}  // namespace

// IDA_EA: 0x8265448
// IDA_NAME: _ZN20AbnormalUseStackableC2Ev
// DEMANGLED: AbnormalUseStackable::AbnormalUseStackable(void)
AbnormalUseStackable::AbnormalUseStackable() : ParamBase(), raw_04(0) {}

// IDA_EA: 0x826589A
// IDA_NAME: _ZN20AbnormalUseStackableD2Ev
// DEMANGLED: AbnormalUseStackable::~AbnormalUseStackable()
// IDA_EA: 0x82658CA
// IDA_NAME: _ZN20AbnormalUseStackableD0Ev
// DEMANGLED: AbnormalUseStackable::~AbnormalUseStackable()
AbnormalUseStackable::~AbnormalUseStackable() {}

// The old specialization has no separate derived-message constructor symbol.
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

// IDA_EA: 0x8265E0E
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_ABNORMAL_USE_STACKABLE20AbnormalUseStackableL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_ABNORMAL_USE_STACKABLE,AbnormalUseStackable,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8265F5E
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_ABNORMAL_USE_STACKABLE20AbnormalUseStackableL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_ABNORMAL_USE_STACKABLE,AbnormalUseStackable,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8265F68
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_ABNORMAL_USE_STACKABLE20AbnormalUseStackableL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_ABNORMAL_USE_STACKABLE,AbnormalUseStackable,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x8265FFA
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_ABNORMAL_USE_STACKABLE20AbnormalUseStackableL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_ABNORMAL_USE_STACKABLE,AbnormalUseStackable,(ch_state)0>::send(CUser *,ParamBase &)
// IDA_EA: 0x8266E3A
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_ABNORMAL_USE_STACKABLE20AbnormalUseStackableL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_ABNORMAL_USE_STACKABLE,AbnormalUseStackable,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x8266E6A
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_ABNORMAL_USE_STACKABLE20AbnormalUseStackableL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_ABNORMAL_USE_STACKABLE,AbnormalUseStackable,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x8598DBE
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_ABNORMAL_USE_STACKABLE20AbnormalUseStackableL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_ABNORMAL_USE_STACKABLE,AbnormalUseStackable,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_ABNORMAL_USE_STACKABLE,
                                 AbnormalUseStackable,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x859792E
// IDA_NAME: _ZN31Dispatcher_AbnormalUseStackableC2Ev
// DEMANGLED: Dispatcher_AbnormalUseStackable::Dispatcher_AbnormalUseStackable(void)
Dispatcher_AbnormalUseStackable::Dispatcher_AbnormalUseStackable() {}

// IDA_EA: 0x82655DC
// IDA_NAME: _ZN31Dispatcher_AbnormalUseStackableD2Ev
// DEMANGLED: Dispatcher_AbnormalUseStackable::~Dispatcher_AbnormalUseStackable()
// IDA_EA: 0x826560C
// IDA_NAME: _ZN31Dispatcher_AbnormalUseStackableD0Ev
// DEMANGLED: Dispatcher_AbnormalUseStackable::~Dispatcher_AbnormalUseStackable()
Dispatcher_AbnormalUseStackable::~Dispatcher_AbnormalUseStackable() {}

// IDA_EA: 0x826491C
// IDA_NAME: _ZN31Dispatcher_AbnormalUseStackable4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_AbnormalUseStackable::read(PacketBuf &,MSG_BASE &)
int Dispatcher_AbnormalUseStackable::read(PacketBuf& packet,
                                          MSG_BASE& message) {
    MSG_ABNORMAL_USE_STACKABLE& input =
        static_cast<MSG_ABNORMAL_USE_STACKABLE&>(message);
    if (packet.get_int(input.first_value) != 1) {
        return LineFunc(1328, kReadSignature, 0, 0);
    }
    if (packet.get_int(input.second_value) == 1) {
        return 0;
    }
    return LineFunc(1332, kReadSignature, 0, 0);
}

// IDA_EA: 0x82649AE
// IDA_NAME: _ZN31Dispatcher_AbnormalUseStackable7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_AbnormalUseStackable::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_AbnormalUseStackable::process(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& param) {
    (void)param;

    MSG_ABNORMAL_USE_STACKABLE& input =
        static_cast<MSG_ABNORMAL_USE_STACKABLE&>(message);
    if (check_error(user, message) == 0) {
        user->getHackAnalyzer()->addServerHackCnt(
            user,
            static_cast<WongWork::ENUM_HACKTYPE>(0xFA0u),
            1u,
            input.first_value,
            input.second_value);
    }
    return 0;
}

// IDA_EA: 0x8264A30
// IDA_NAME: _ZN31Dispatcher_AbnormalUseStackable11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_AbnormalUseStackable::check_error(CUser *,MSG_BASE &)
int Dispatcher_AbnormalUseStackable::check_error(CUser* user,
                                                 MSG_BASE& message) {
    (void)user;
    (void)message;
    return 0;
}
