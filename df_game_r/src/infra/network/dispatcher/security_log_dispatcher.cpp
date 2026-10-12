#include "infra/network/dispatcher/security_log_dispatcher.h"

#include <cstring>

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

namespace {

const char kProcessSignature[] =
    "virtual int Dispatcher_SecurityLog::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// IDA_EA: 0x82602EC
// IDA_NAME: _ZN23MSG_SECURITY_LOG_PACKETC2Ev
// DEMANGLED: MSG_SECURITY_LOG_PACKET::MSG_SECURITY_LOG_PACKET(void)
MSG_SECURITY_LOG_PACKET::MSG_SECURITY_LOG_PACKET() : MSG_BASE() {
    // Raw C2 deliberately overwrites the just-constructed MSG_BASE as
    // `memset(this, 0, 0x25)`, including the 13-byte C2S header storage.
    std::memset(static_cast<void*>(this), 0, sizeof(*this));
}

// IDA_EA: 0x826057A
// IDA_NAME: _ZN18stSecurityLogParamC2Ev
// DEMANGLED: stSecurityLogParam::stSecurityLogParam(void)
stSecurityLogParam::stSecurityLogParam() : ParamBase() {}

// IDA_EA: 0x8260C14
// IDA_NAME: _ZN18stSecurityLogParamD2Ev
// DEMANGLED: stSecurityLogParam::~stSecurityLogParam()
// IDA_EA: 0x8260C44
// IDA_NAME: _ZN18stSecurityLogParamD0Ev
// DEMANGLED: stSecurityLogParam::~stSecurityLogParam()
stSecurityLogParam::~stSecurityLogParam() {}

// The exact old state-0 template is emitted from the shared canonical
// packet_dispatcher_msg_template.tpp.  Keep these seven markers beside this
// one explicit instantiation owner; there is no copied local template body.
// IDA_EA: 0x8260F3A
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_SECURITY_LOG_PACKET18stSecurityLogParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SECURITY_LOG_PACKET,stSecurityLogParam,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8261096
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_SECURITY_LOG_PACKET18stSecurityLogParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SECURITY_LOG_PACKET,stSecurityLogParam,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x82610A0
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_SECURITY_LOG_PACKET18stSecurityLogParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SECURITY_LOG_PACKET,stSecurityLogParam,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x8262344
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_SECURITY_LOG_PACKET18stSecurityLogParamL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_SECURITY_LOG_PACKET,stSecurityLogParam,(ch_state)0>::send(CUser *,ParamBase &)
// IDA_EA: 0x82606B0
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_SECURITY_LOG_PACKET18stSecurityLogParamL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_SECURITY_LOG_PACKET,stSecurityLogParam,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x82606E0
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_SECURITY_LOG_PACKET18stSecurityLogParamL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_SECURITY_LOG_PACKET,stSecurityLogParam,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x859825E
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_SECURITY_LOG_PACKET18stSecurityLogParamL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_SECURITY_LOG_PACKET,stSecurityLogParam,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_SECURITY_LOG_PACKET,
                                 stSecurityLogParam,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x85956B4
// IDA_NAME: _ZN22Dispatcher_SecurityLogC2Ev
// DEMANGLED: Dispatcher_SecurityLog::Dispatcher_SecurityLog(void)
Dispatcher_SecurityLog::Dispatcher_SecurityLog() {}

// IDA_EA: 0x8260596
// IDA_NAME: _ZN22Dispatcher_SecurityLogD2Ev
// DEMANGLED: Dispatcher_SecurityLog::~Dispatcher_SecurityLog()
// IDA_EA: 0x82605C6
// IDA_NAME: _ZN22Dispatcher_SecurityLogD0Ev
// DEMANGLED: Dispatcher_SecurityLog::~Dispatcher_SecurityLog()
Dispatcher_SecurityLog::~Dispatcher_SecurityLog() {}

// IDA_EA: 0x825FABC
// IDA_NAME: _ZN22Dispatcher_SecurityLog4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_SecurityLog::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SecurityLog::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_SECURITY_LOG_PACKET& input =
        static_cast<MSG_SECURITY_LOG_PACKET&>(message);

    int first_word;
    if (packet.get_int(first_word) != 1) {
        return 2458;
    }
    input.raw_0d = static_cast<uint32_t>(first_word);

    // The raw loop covers five contiguous 32-bit slots at +0x11 through
    // +0x21, returning the same 2466 line code at every failed iteration.
    if (packet.get_int(input.raw_11) != 1 ||
        packet.get_int(input.raw_15) != 1 ||
        packet.get_int(input.raw_19) != 1 ||
        packet.get_int(input.raw_1d) != 1 ||
        packet.get_int(input.raw_21) != 1) {
        return 2466;
    }
    return 0;
}

// IDA_EA: 0x825FB38
// IDA_NAME: _ZN22Dispatcher_SecurityLog7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SecurityLog::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SecurityLog::process(CUser* user,
                                    MSG_BASE& message,
                                    ParamBase& param) {
    (void)param;
    const unsigned int account_id =
        user ? static_cast<unsigned int>(user->get_acc_id()) : 0u;
    const MSG_SECURITY_LOG_PACKET& input =
        static_cast<const MSG_SECURITY_LOG_PACKET&>(message);

    if (input.raw_15 == 0u || input.raw_19 == 0u) {
        return 0;
    }

    LogManager::logFormat(
        1,
        "PacketDispatcher_Impl_2.cpp",
        kProcessSignature,
        0x9D0,
        "Delay check, mode:%d, value:%d, %d, %d, %d, %d, %d, m_id:%s",
        input.raw_0d,
        input.raw_11,
        input.raw_15,
        input.raw_19,
        input.raw_1d,
        input.raw_21,
        input.raw_1d - input.raw_19,
        NumberToString(account_id, 0));
    return 0;
}

// IDA_EA: 0x825FC34
// IDA_NAME: _ZN22Dispatcher_SecurityLog4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_SecurityLog::send(CUser *,ParamBase &)
void Dispatcher_SecurityLog::send(CUser* user, ParamBase& param) {
    (void)user;
    (void)param;
}
