#include "infra/network/packet/packet_pool.h"


// IDA_EA: 0x828FC76
// IDA_NAME: _ZN10PacketPoolC2Ev
// DEMANGLED: PacketPool::PacketPool(void)
PacketPool::PacketPool() : pool_(new DynamicPoolPacketBuf(10)), thread_id_(0) {
    pool_->SetPoolSize(10, 10);
}

// IDA_EA: 0x828FCFA
// IDA_NAME: _ZN10PacketPoolC2Eii
// DEMANGLED: PacketPool::PacketPool(int,int)
PacketPool::PacketPool(int initial_count, int grow_count)
    : pool_(new DynamicPoolPacketBuf(initial_count)), thread_id_(0) {
    pool_->SetPoolSize(static_cast<unsigned short>(initial_count),
                       static_cast<unsigned short>(grow_count));
}

// IDA_EA: 0x828FD82
// IDA_NAME: _ZN10PacketPoolD2Ev
// DEMANGLED: PacketPool::~PacketPool()
PacketPool::~PacketPool() {
    delete pool_;
    pool_ = 0;
}

// IDA_EA: 0x828FDBA
// IDA_NAME: _ZN10PacketPool7AcquireEv
// DEMANGLED: PacketPool::Acquire(void)
PacketBuf* PacketPool::Acquire() {
    PacketBuf* packet = pool_->Acquire();
    if (packet != 0) {
        packet->clear();
    }
    return packet;
}

// IDA_EA: 0x82A6EF2
// IDA_NAME: _ZN10PacketPool11setThreadIDEm
// DEMANGLED: PacketPool::setThreadID(unsigned long)
PacketPool* PacketPool::setThreadID(unsigned long thread_id) {
    thread_id_ = static_cast<pthread_t>(thread_id);
    return this;
}

// IDA_EA: 0x82A6F00
// IDA_NAME: _ZN10PacketPool11getThreadIDEv
// DEMANGLED: PacketPool::getThreadID(void)
pthread_t PacketPool::getThreadID() const {
    return thread_id_;
}

// IDA_EA: 0x858E2AE
// IDA_NAME: _ZN10PacketPool4FreeEP9PacketBuf
// DEMANGLED: PacketPool::Free(PacketBuf*)
int PacketPool::Free(PacketBuf* packet) {
    return pool_->Free(packet);
}

int PacketPool::Size() const {
    return pool_->Size();
}

int PacketPool::Remain() const {
    return pool_->Remain();
}
