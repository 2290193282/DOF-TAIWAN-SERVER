#include "infra/network/packet/packet_counter.h"

#include <cstdio>
#include <ctime>

// The constructor calls Reset, so declare the exact specialization before
// defining the constructor. This keeps G++4.9 from instantiating the primary
// template body at that call site.
template <>
char* CPacketCounter<1000, 10240>::Reset();

// IDA_EA: 0x857F4DC
// IDA_NAME: _ZN14CPacketCounterILi1000ELi10240EEC2EPcS1_
// DEMANGLED: CPacketCounter<1000,10240>::CPacketCounter(char *,char *)
template <>
CPacketCounter<1000, 10240>::CPacketCounter(char* file_name,
                                             char* counter_name) {
    Reset();
    reset_time_ = static_cast<int32_t>(std::time(0));
    if (file_name) {
        std::sprintf(log_path_, "./log/%s/%s", file_name, counter_name);
    } else {
        std::sprintf(log_path_, "./log/%s", counter_name);
    }
    enabled_ = 1;
}

// IDA_EA: 0x857F55A
// IDA_NAME: _ZN14CPacketCounterILi1000ELi10240EED2Ev
// DEMANGLED: CPacketCounter<1000,10240>::~CPacketCounter()
template <>
CPacketCounter<1000, 10240>::~CPacketCounter() {
}

// IDA_EA: 0x857F560
// IDA_NAME: _ZN14CPacketCounterILi1000ELi10240EE20IncrementPacketCountEi
// DEMANGLED: CPacketCounter<1000,10240>::IncrementPacketCount(int)
template <>
unsigned int CPacketCounter<1000, 10240>::IncrementPacketCount(int packet_id) {
    unsigned int result;
    if (packet_id >= 1000 && packet_id <= 10239) {
        const unsigned int index = static_cast<unsigned int>(packet_id - 1000);
        if (enabled_ == 1 || packet_counts_[index] <= 10u) {
            result = index;
            ++packet_counts_[index];
        }
    }
    return result;
}

// IDA_EA: 0x857F92E
// IDA_NAME: _ZN14CPacketCounterILi1000ELi10240EE5ResetEv
// DEMANGLED: CPacketCounter<1000,10240>::Reset()
template <>
char* CPacketCounter<1000, 10240>::Reset() {
    for (int index = 0; index < 9240; ++index) {
        packet_counts_[index] = 0;
        reset_counts_a_[index] = 0;
        reset_flags_[index] = 0;
        reset_counts_b_[index] = 0;
    }
    reset_state_ = 0;
    reset_marker_ = 0;
    return reinterpret_cast<char*>(this);
}
