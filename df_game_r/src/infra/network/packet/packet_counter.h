#ifndef OLDSRV_RUNTIME_PACKET_COUNTER_H_
#define OLDSRV_RUNTIME_PACKET_COUNTER_H_

#include <stdint.h>

// IDA unit: 99_misc_global/CPacketCounter<1000,10240>.
// This template instance is the function-local UDP counter at
// UdpThread::_processUDPData. Its storage layout is fixed by the four old
// member bodies, including the otherwise-unused reset arrays.
template <int FirstPacket, int PastLastPacket>
class CPacketCounter {
public:
    CPacketCounter(char* file_name, char* counter_name);
    ~CPacketCounter();

    unsigned int IncrementPacketCount(int packet_id);
    char* Reset();

private:
    enum {
        kBucketCount = PastLastPacket - FirstPacket
    };

    uint32_t reset_state_;
    int32_t reset_time_;
    uint32_t packet_counts_[kBucketCount];
    uint32_t reset_counts_a_[kBucketCount];
    uint8_t reset_flags_[kBucketCount];
    uint32_t reset_counts_b_[kBucketCount];
    char log_path_[256];
    uint8_t enabled_;
    uint8_t reset_marker_;
};

#if defined(__i386__)
static_assert(sizeof(CPacketCounter<1000, 10240>) == 0x1D644u,
              "CPacketCounter<1000,10240> must retain the old UDP ABI");
#endif

#endif  // OLDSRV_RUNTIME_PACKET_COUNTER_H_
