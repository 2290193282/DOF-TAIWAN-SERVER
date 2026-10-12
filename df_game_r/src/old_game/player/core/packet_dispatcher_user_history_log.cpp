#include "old_game/player/core/packet_dispatcher_user_history_log.h"

#include <cstring>

#include "infra/security/policy/packet_dispatcher_guard.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

namespace {

const char kReadSignature[] =
    "virtual int Dispatcher_UserHistoryLog::read(PacketBuf&, MSG_BASE&)";

}  // namespace

// IDA_EA: 0x822DEA8
// IDA_NAME: _ZN20MSG_USER_HISTORY_LOGC2Ev
// DEMANGLED: MSG_USER_HISTORY_LOG::MSG_USER_HISTORY_LOG(void)
MSG_USER_HISTORY_LOG::MSG_USER_HISTORY_LOG()
    : MSG_BASE(),
      action(0),
      history_length(0) {
    std::memset(history_text, 0, sizeof(history_text));
}

// The raw CMD426 class owns this one exact generic state-0 specialization.
// Include the shared definition only after the proved message layout is
// complete, then emit it once from this concrete provider TU.
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

// IDA_EA: 0x824EBE0
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_USER_HISTORY_LOG9ParamBaseL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_USER_HISTORY_LOG,ParamBase,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x824ED4A
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_USER_HISTORY_LOG9ParamBaseL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_USER_HISTORY_LOG,ParamBase,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x824ED54
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_USER_HISTORY_LOG9ParamBaseL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_USER_HISTORY_LOG,ParamBase,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x824EDE6
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_USER_HISTORY_LOG9ParamBaseL8ch_state0EE4sendEP5CUserRS1_
// DEMANGLED: IPacketDispatcher<MSG_USER_HISTORY_LOG,ParamBase,(ch_state)0>::send(CUser *,ParamBase&)
// IDA_EA: 0x825BEA8
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_USER_HISTORY_LOG9ParamBaseL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_USER_HISTORY_LOG,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825BED8
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_USER_HISTORY_LOG9ParamBaseL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_USER_HISTORY_LOG,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x8598D6A
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_USER_HISTORY_LOG9ParamBaseL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_USER_HISTORY_LOG,ParamBase,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_USER_HISTORY_LOG,
                                 ParamBase,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x85978BE
// IDA_NAME: _ZN25Dispatcher_UserHistoryLogC2Ev
// DEMANGLED: Dispatcher_UserHistoryLog::Dispatcher_UserHistoryLog(void)
Dispatcher_UserHistoryLog::Dispatcher_UserHistoryLog() {}

// IDA_EA: 0x8244A58
// IDA_NAME: _ZN25Dispatcher_UserHistoryLogD2Ev
// DEMANGLED: Dispatcher_UserHistoryLog::~Dispatcher_UserHistoryLog()
// IDA_EA: 0x8244A88
// IDA_NAME: _ZN25Dispatcher_UserHistoryLogD0Ev
// DEMANGLED: Dispatcher_UserHistoryLog::~Dispatcher_UserHistoryLog()
Dispatcher_UserHistoryLog::~Dispatcher_UserHistoryLog() {}

// IDA_EA: 0x81DA5D2
// IDA_NAME: _ZN25Dispatcher_UserHistoryLog4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_UserHistoryLog::read(PacketBuf &,MSG_BASE &)
int Dispatcher_UserHistoryLog::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_USER_HISTORY_LOG& input = static_cast<MSG_USER_HISTORY_LOG&>(message);
    if (packet.get_byte(input.action) != 1) {
        return LineFunc(18601, kReadSignature, 0, 0);
    }
    if (packet.get_int(input.history_length) != 1) {
        return LineFunc(18604, kReadSignature, 0, 0);
    }
    if (packet.get_str(input.history_text, 256, input.history_length) == 1) {
        return 0;
    }
    return LineFunc(18607, kReadSignature, 0, 0);
}

// IDA_EA: 0x81DA6C0
// IDA_NAME: _ZN25Dispatcher_UserHistoryLog7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_UserHistoryLog::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_UserHistoryLog::process(CUser* user,
                                       MSG_BASE& message,
                                       ParamBase& param) {
    (void)param;
    const MSG_USER_HISTORY_LOG& input =
        static_cast<const MSG_USER_HISTORY_LOG&>(message);
    user->LogHistory("act,%d,%s", static_cast<int>(input.action), input.history_text);
    return 0;
}
