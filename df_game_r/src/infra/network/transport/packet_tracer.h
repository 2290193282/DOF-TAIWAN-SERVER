#ifndef OLDSRV_GLOBAL_PACKET_TRACER_H_
#define OLDSRV_GLOBAL_PACKET_TRACER_H_

#include <stdint.h>


// IDA class: CPacketTracer.  It contains two independent ten-entry rings:
// TCP at +0x08 and UDP at +0x58, each as { byte packet_class; int command }.
class CPacketTracer {
public:
    struct PacketRecord {
        uint8_t packet_class;
        uint8_t reserved_[3];
        int command;
    };

    static const unsigned int kRingSize = 10;

    // IDA_EA: 0x8260628
    // IDA_NAME: _ZN13CPacketTracerC2Ev
    // DEMANGLED: CPacketTracer::CPacketTracer(void)
    CPacketTracer();

    // IDA_EA: 0x859926C
    // IDA_NAME: _ZN13CPacketTracer9AddUdpLogEii
    // DEMANGLED: CPacketTracer::AddUdpLog(int,int)
    CPacketTracer* AddUdpLog(int packet_class, int command);

    // IDA_EA: 0x85992C8
    // IDA_NAME: _ZN13CPacketTracer9AddTcpLogEii
    // DEMANGLED: CPacketTracer::AddTcpLog(int,int)
    CPacketTracer* AddTcpLog(int packet_class, int command);

    // IDA_EA: 0x859931A
    // IDA_NAME: _ZN13CPacketTracer19AbsoluteTcpWriteLogEv
    // DEMANGLED: CPacketTracer::AbsoluteTcpWriteLog(void)
    int AbsoluteTcpWriteLog();

    // IDA_EA: 0x859940E
    // IDA_NAME: _ZN13CPacketTracer19AbsoluteUdpWriteLogEv
    // DEMANGLED: CPacketTracer::AbsoluteUdpWriteLog(void)
    int AbsoluteUdpWriteLog();

private:
    uint32_t tcp_write_count_;
    uint32_t udp_write_count_;
    PacketRecord tcp_records_[kRingSize];
    PacketRecord udp_records_[kRingSize];
};

static_assert(sizeof(CPacketTracer::PacketRecord) == 8u,
              "CPacketTracer ring-record ABI must remain 8 bytes");
static_assert(sizeof(CPacketTracer) == 168u,
              "CPacketTracer ABI must remain 0xA8 bytes");

// IDA_EA: 0x8260640
// IDA_NAME: _Z21CPacketTracerInstancev
// DEMANGLED: CPacketTracerInstance(void)
CPacketTracer* CPacketTracerInstance();


#endif  // OLDSRV_GLOBAL_PACKET_TRACER_H_
