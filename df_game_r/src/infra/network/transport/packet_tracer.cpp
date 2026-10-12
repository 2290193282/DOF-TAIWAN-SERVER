#include "infra/network/transport/packet_tracer.h"

#include "infra/log/format/trace.h"

#include <cstdio>
#include <cstring>


// IDA_EA: 0x8260628
// IDA_NAME: _ZN13CPacketTracerC2Ev
// DEMANGLED: CPacketTracer::CPacketTracer(void)
CPacketTracer::CPacketTracer() : tcp_write_count_(0), udp_write_count_(0) {
    // The original C2 writes only the two counters.  A function-local static
    // receives BSS zero-initialization before this constructor, just as in
    // the old singleton; a non-static C2 leaves the ring records untouched.
}

// IDA_EA: 0x859926C
// IDA_NAME: _ZN13CPacketTracer9AddUdpLogEii
// DEMANGLED: CPacketTracer::AddUdpLog(int,int)
CPacketTracer* CPacketTracer::AddUdpLog(int packet_class, int command) {
    const uint32_t index = udp_write_count_ % kRingSize;
    udp_records_[index].packet_class = static_cast<uint8_t>(packet_class);
    udp_records_[index].command = command;
    ++udp_write_count_;
    return this;
}

// IDA_EA: 0x85992C8
// IDA_NAME: _ZN13CPacketTracer9AddTcpLogEii
// DEMANGLED: CPacketTracer::AddTcpLog(int,int)
CPacketTracer* CPacketTracer::AddTcpLog(int packet_class, int command) {
    const uint32_t index = tcp_write_count_ % kRingSize;
    tcp_records_[index].packet_class = static_cast<uint8_t>(packet_class);
    tcp_records_[index].command = command;
    ++tcp_write_count_;
    return this;
}

// IDA_EA: 0x859931A
// IDA_NAME: _ZN13CPacketTracer19AbsoluteTcpWriteLogEv
// DEMANGLED: CPacketTracer::AbsoluteTcpWriteLog(void)
int CPacketTracer::AbsoluteTcpWriteLog() {
    char text[2048];
    std::memset(text, 0, sizeof(text));
    for (unsigned int index = 0; index < kRingSize; ++index) {
        // Preserve the original 0x859931A self-referential sprintf loop.
        std::sprintf(text, "%s,%d/%d", text,
                     static_cast<int>(tcp_records_[index].packet_class),
                     tcp_records_[index].command);
    }

    const uint32_t ring_index = --tcp_write_count_ % kRingSize;
    cMyTrace trace("AbsoluteTcpWriteLog", 65, 0);
    return trace("[TRACE_TCP_PACKET] (idx:%d)%s",
                 static_cast<int>(ring_index), text);
}

// IDA_EA: 0x859940E
// IDA_NAME: _ZN13CPacketTracer19AbsoluteUdpWriteLogEv
// DEMANGLED: CPacketTracer::AbsoluteUdpWriteLog(void)
int CPacketTracer::AbsoluteUdpWriteLog() {
    char text[2048];
    std::memset(text, 0, sizeof(text));
    for (unsigned int index = 0; index < kRingSize; ++index) {
        // Preserve the original 0x859940E self-referential sprintf loop.
        std::sprintf(text, "%s,%d/%d", text,
                     static_cast<int>(udp_records_[index].packet_class),
                     udp_records_[index].command);
    }

    const uint32_t ring_index = --udp_write_count_ % kRingSize;
    cMyTrace trace("AbsoluteUdpWriteLog", 90, 0);
    return trace("[TRACE_UDP_PACKET] (idx:%d)%s",
                 static_cast<int>(ring_index), text);
}

// IDA_EA: 0x8260640
// IDA_NAME: _Z21CPacketTracerInstancev
// DEMANGLED: CPacketTracerInstance(void)
CPacketTracer* CPacketTracerInstance() {
    static CPacketTracer instance;
    return &instance;
}
