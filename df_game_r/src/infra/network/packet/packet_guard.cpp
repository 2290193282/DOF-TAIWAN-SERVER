#include "infra/network/packet/packet_guard.h"

#include "common/manager/global_data.h"
#include "infra/network/packet/packet_pool.h"

#include <pthread.h>


// IDA_EA: 0x858DD4C
// IDA_NAME: _ZN11PacketGuardC2Ev
// DEMANGLED: PacketGuard::PacketGuard(void)
PacketGuard::PacketGuard()
    : InterfacePacketBuf(), pool_(0), external_packet_(false) {
    for (int index = GlobalData::THREAD_DISPATCH;
         index <= GlobalData::THREAD_FEATURE_DB; ++index) {
        PacketPool* candidate =
            GlobalData::packet_pool(static_cast<GlobalData::THREAD_TYPE>(index));
        if (candidate != 0 &&
            pthread_equal(candidate->getThreadID(), pthread_self())) {
            pool_ = candidate;
            break;
        }
    }
    if (pool_ != 0) {
        AcquirePacketBuf(pool_->Acquire());
    }
}

// IDA_EA: 0x858DE4A
// IDA_NAME: _ZN11PacketGuardC2EP9PacketBuf
// DEMANGLED: PacketGuard::PacketGuard(PacketBuf*)
PacketGuard::PacketGuard(PacketBuf* packet)
    : InterfacePacketBuf(), pool_(0), external_packet_(true) {
    AcquirePacketBuf(packet);
}

// IDA_EA: 0x858DE80
// IDA_NAME: _ZN11PacketGuardD2Ev
// DEMANGLED: PacketGuard::~PacketGuard()
PacketGuard::~PacketGuard() {
    if (pool_ != 0) {
        pool_->Free(packet_);
    } else if (!external_packet_) {
        (void)(packet_ ? packet_->get_index() : 0);
    }
}
