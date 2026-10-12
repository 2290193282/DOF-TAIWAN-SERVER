#include "infra/network/packet/dynamic_pool_packet_buf.h"


namespace {

const unsigned int kDynamicPoolDummy = 36641;

}  // namespace

// IDA_EA: 0x82ACD02
// IDA_NAME: _ZN11DynamicPoolI9PacketBufE5CNodeD2Ev
// DEMANGLED: DynamicPool<PacketBuf>::CNode::~CNode()
DynamicPoolPacketBuf::CNode::~CNode() {}

// IDA_EA: 0x82C12CA
// IDA_NAME: _ZN11DynamicPoolI9PacketBufE5CNodeC2Ev
// DEMANGLED: DynamicPool<PacketBuf>::CNode::CNode(void)
DynamicPoolPacketBuf::CNode::CNode()
    : PacketBuf(), pool_dummy_(kDynamicPoolDummy), pool_ref_count_(0) {}

unsigned int DynamicPoolPacketBuf::CNode::pool_dummy_value() const {
    return pool_dummy_;
}

unsigned char DynamicPoolPacketBuf::CNode::pool_ref_count_value() const {
    return pool_ref_count_;
}

void DynamicPoolPacketBuf::CNode::increment_pool_ref_count() {
    ++pool_ref_count_;
}

void DynamicPoolPacketBuf::CNode::decrement_pool_ref_count() {
    --pool_ref_count_;
}

// IDA_EA: 0x82ACBB4
// IDA_NAME: _ZN11DynamicPoolI9PacketBufEC2Ei
// DEMANGLED: DynamicPool<PacketBuf>::DynamicPool(int)
DynamicPoolPacketBuf::DynamicPoolPacketBuf(int initial_count)
    : free_nodes_(),
      chunks_(),
      pointer_index_(),
      total_size_(0),
      initial_count_(static_cast<unsigned short>(initial_count)),
      grow_count_(static_cast<unsigned short>(initial_count)),
      error_code_(0) {
    MoreAlloc(initial_count);
}

// IDA_EA: 0x82ACCD6
// IDA_NAME: _ZN11DynamicPoolI9PacketBufE11SetPoolSizeEtt
// DEMANGLED: DynamicPool<PacketBuf>::SetPoolSize(unsigned short,unsigned short)
DynamicPoolPacketBuf* DynamicPoolPacketBuf::SetPoolSize(
    unsigned short initial_count, unsigned short grow_count) {
    initial_count_ = initial_count;
    grow_count_ = grow_count;
    return this;
}

// IDA_EA: 0x82ACD16
// IDA_NAME: _ZN11DynamicPoolI9PacketBufED2Ev
// DEMANGLED: DynamicPool<PacketBuf>::~DynamicPool()
DynamicPoolPacketBuf::~DynamicPoolPacketBuf() {
    for (std::vector<CNode*>::iterator it = chunks_.begin();
         it != chunks_.end(); ++it) {
        delete[] *it;
    }
    chunks_.clear();
    while (!free_nodes_.empty()) {
        free_nodes_.pop();
    }
    pointer_index_.clear();
    total_size_ = 0;
}

// IDA_EA: 0x82ACE5A
// IDA_NAME: _ZN11DynamicPoolI9PacketBufE7AcquireEv
// DEMANGLED: DynamicPool<PacketBuf>::Acquire(void)
PacketBuf* DynamicPoolPacketBuf::Acquire() {
    if (free_nodes_.empty()) {
        MoreAlloc(grow_count_);
    }
    if (free_nodes_.empty()) {
        return 0;
    }

    CNode* node = free_nodes_.front();
    free_nodes_.pop();
    if (node->pool_dummy_value() != kDynamicPoolDummy) {
        error_code_ = 3;
    }
    if (node->pool_ref_count_value() != 0) {
        error_code_ = 4;
    }
    node->increment_pool_ref_count();
    return node;
}

// IDA_EA: 0x82AD0B0
// IDA_NAME: _ZN11DynamicPoolI9PacketBufE6RemainEv
// DEMANGLED: DynamicPool<PacketBuf>::Remain(void)
int DynamicPoolPacketBuf::Remain() const {
    return static_cast<int>(free_nodes_.size());
}

// IDA_EA: 0x82AD0C6
// IDA_NAME: _ZN11DynamicPoolI9PacketBufE4SizeEv
// DEMANGLED: DynamicPool<PacketBuf>::Size(void)
int DynamicPoolPacketBuf::Size() const {
    return total_size_;
}

// IDA_EA: 0x82B7E14
// IDA_NAME: _ZN11DynamicPoolI9PacketBufE9MoreAllocEi
// DEMANGLED: DynamicPool<PacketBuf>::MoreAlloc(int)
int DynamicPoolPacketBuf::MoreAlloc(int count) {
    if (count <= 0) {
        return 0;
    }
    CNode* chunk = new CNode[count];
    if (chunk == 0) {
        return 0;
    }
    chunks_.push_back(chunk);
    for (int index = 0; index < count; ++index) {
        CNode* node = &chunk[index];
        free_nodes_.push(node);
        pointer_index_[node] = 1;
    }
    total_size_ += count;
    return 1;
}

// IDA_EA: 0x858E2CA
// IDA_NAME: _ZN11DynamicPoolI9PacketBufE4FreeEPS0_
// DEMANGLED: DynamicPool<PacketBuf>::Free(PacketBuf*)
int DynamicPoolPacketBuf::Free(PacketBuf* packet) {
    if (packet == 0) {
        return 0;
    }
    CNode* node = static_cast<CNode*>(packet);
    free_nodes_.push(node);
    if (node->pool_dummy_value() != kDynamicPoolDummy) {
        error_code_ = 5;
    }
    if (node->pool_ref_count_value() != 1) {
        error_code_ = 6;
    }
    node->decrement_pool_ref_count();
    return 1;
}
