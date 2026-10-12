#include "infra/network/dispatcher/p2p_hole_punching_statistics_dispatcher.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


namespace {

const char kReadSignature[] =
    "virtual int Dispatcher_P2PHolePunchingSuccessRateStatistics::read(PacketBuf&, MSG_BASE&)";

}  // namespace


// IDA_EA: 0x8260470
// IDA_NAME: _ZN34MSG_P2P_HOLE_PUNCHING_SUCCESS_RATEC2Ev
// DEMANGLED: MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE::MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE(void)
MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE::MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE()
    : MSG_BASE() {
    // The old C2 writes only +0x0d and +0x0e, leaving +0x12 untouched.
    *reinterpret_cast<volatile uint8_t*>(&raw_0d) = 0;
    *reinterpret_cast<volatile uint32_t*>(&raw_0e) = 0;
}

// IDA_EA: 0x8260844
// IDA_NAME: _ZN6Stream11GetInBufferI34MSG_P2P_HOLE_PUNCHING_SUCCESS_RATEEEPT_v
// DEMANGLED: MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE * Stream::GetInBuffer<MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE>(void)
template <>
MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE*
Stream::GetInBuffer<MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE>() {
    const int bytes = sizeof(MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE);
    while (enable_in(bytes) != true) {
        resize(size() * 2);
    }
    MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE* const result =
        reinterpret_cast<MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x826077E
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI34MSG_P2P_HOLE_PUNCHING_SUCCESS_RATEEEPT_v
// DEMANGLED: MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE * CStreamGuard::GetInBuffer<MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE>(void)
template <>
MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE*
CStreamGuard::GetInBuffer<MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE>() {
    return stream_->GetInBuffer<MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE>();
}


// The old CMD599 class owns this exact state-0 specialization. Keep the
// template definitions local to this provider so no unrelated TU emits its
// vtable or native symbols.
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

// IDA_EA: 0x8260D4C
// IDA_NAME: _ZN17IPacketDispatcherI34MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE9ParamBaseL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE,ParamBase,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8260EA8
// IDA_NAME: _ZN17IPacketDispatcherI34MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE9ParamBaseL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE,ParamBase,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x826234A
// IDA_NAME: _ZN17IPacketDispatcherI34MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE9ParamBaseL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x826237A
// IDA_NAME: _ZN17IPacketDispatcherI34MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE9ParamBaseL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE,ParamBase,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x8262608
// IDA_NAME: _ZN17IPacketDispatcherI34MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE9ParamBaseL8ch_state0EE4sendEP5CUserRS1_
// DEMANGLED: IPacketDispatcher<MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE,ParamBase,(ch_state)0>::send(CUser *,ParamBase&)
// IDA_EA: 0x826260E
// IDA_NAME: _ZN17IPacketDispatcherI34MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE9ParamBaseL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE,ParamBase,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x859923A
// IDA_NAME: _ZN17IPacketDispatcherI34MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE9ParamBaseL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE,ParamBase,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE,
                                 ParamBase,
                                 static_cast<ch_state>(0)>;


// IDA_EA: 0x8597F32
// IDA_NAME: _ZN47Dispatcher_P2PHolePunchingSuccessRateStatisticsC2Ev
// DEMANGLED: Dispatcher_P2PHolePunchingSuccessRateStatistics::Dispatcher_P2PHolePunchingSuccessRateStatistics(void)
Dispatcher_P2PHolePunchingSuccessRateStatistics::
    Dispatcher_P2PHolePunchingSuccessRateStatistics() {}

// IDA_EA: 0x8260908
// IDA_NAME: _ZN47Dispatcher_P2PHolePunchingSuccessRateStatisticsD2Ev
// DEMANGLED: Dispatcher_P2PHolePunchingSuccessRateStatistics::~Dispatcher_P2PHolePunchingSuccessRateStatistics()
// IDA_EA: 0x8260938
// IDA_NAME: _ZN47Dispatcher_P2PHolePunchingSuccessRateStatisticsD0Ev
// DEMANGLED: Dispatcher_P2PHolePunchingSuccessRateStatistics::~Dispatcher_P2PHolePunchingSuccessRateStatistics()
Dispatcher_P2PHolePunchingSuccessRateStatistics::
    ~Dispatcher_P2PHolePunchingSuccessRateStatistics() {}

// IDA_EA: 0x825FEDA
// IDA_NAME: _ZN47Dispatcher_P2PHolePunchingSuccessRateStatistics4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_P2PHolePunchingSuccessRateStatistics::read(PacketBuf &,MSG_BASE &)
int Dispatcher_P2PHolePunchingSuccessRateStatistics::read(
    PacketBuf& packet,
    MSG_BASE& message) {
    MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE& input =
        static_cast<MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE&>(message);
    if (packet.get_byte(input.raw_0d) != 1) {
        return LineFunc(2877, kReadSignature, 0, 0);
    }
    if (packet.get_int(input.raw_0e) != 1) {
        return LineFunc(2880, kReadSignature, 0, 0);
    }
    if (packet.get_int(input.raw_12) == 1) {
        return 0;
    }
    return LineFunc(2883, kReadSignature, 0, 0);
}

// IDA_EA: 0x825FFB6
// IDA_NAME: _ZN47Dispatcher_P2PHolePunchingSuccessRateStatistics7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_P2PHolePunchingSuccessRateStatistics::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_P2PHolePunchingSuccessRateStatistics::process(
    CUser* user,
    MSG_BASE& message,
    ParamBase& param) {
    (void)param;

    MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE& input =
        static_cast<MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE&>(message);
    Stream* const stream = GlobalData::s_stream_pool->Acquire(
        "PacketDispatcher_Impl_2.cpp", 2892);
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, 15, user->GetUID());
    MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE* const destination =
        guard.GetInBuffer<MSG_P2P_HOLE_PUNCHING_SUCCESS_RATE>();
    std::memcpy(destination, &input, sizeof(input));
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, guard);
    return 0;
}

// IDA_EA: 0x82605FE
// IDA_NAME: _ZN47Dispatcher_P2PHolePunchingSuccessRateStatistics12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_P2PHolePunchingSuccessRateStatistics::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_P2PHolePunchingSuccessRateStatistics::dispatch_sig(
    CUser* user,
    PacketBuf& packet) {
    (void)user;
    (void)packet;
    return 0;
}

// IDA_EA: 0x8260608
// IDA_NAME: _ZN47Dispatcher_P2PHolePunchingSuccessRateStatistics4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_P2PHolePunchingSuccessRateStatistics::send(CUser *,ParamBase &)
void Dispatcher_P2PHolePunchingSuccessRateStatistics::send(CUser* user,
                                                            ParamBase& param) {
    (void)user;
    (void)param;
}
